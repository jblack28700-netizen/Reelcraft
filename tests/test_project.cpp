#include <QtTest>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QColor>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtMath>
#include <QWheelEvent>
#include <cmath>
#include <cstring>

#include "application/Application.h"
#include "core/MediaItem.h"
#include "core/Project.h"
#include "media/FfmpegFrameSource.h"
#include "media/FrameExtractor.h"
#include "media/FramePump.h"
#include "media/FrameSource.h"
#include "playback/Clock.h"
#include "playback/DefaultPacingPolicy.h"
#include "playback/PacingPolicy.h"
#include "playback/Playhead.h"
#include "playback/Player.h"
#include "reframe/CameraKeyframe.h"
#include "reframe/CameraPath.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframeFrameProvider.h"
#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"
#include "reframe/ReframePlanBuilder.h"
#include "reframe/ReframePipeline.h"
#include "reframe/ReframeRenderer.h"
#include "target/EquirectProjection.h"
#include "target/EquirectViewPlan.h"
#include "target/ProcessTargetDetector.h"
#include "target/SphericalTargetTracker.h"
#include "target/AppearanceProvider.h"
#include "target/AppearanceTypes.h"
#include "target/IdentityReidentifier.h"
#include "target/ProcessAppearanceProvider.h"
#include "target/ProcessSpeakerProvider.h"
#include "target/SpeakerEvidenceAnalyzer.h"
#include "target/SpeakerEvidenceProvider.h"
#include "target/SpeakerReframePlanner.h"
#include "target/SpeakerTargetAssociator.h"
#include "target/SpeakerTimeline.h"
#include "target/SpeakerTypes.h"
#include "target/TargetCropExtractor.h"
#include "target/TargetDetector.h"
#include "target/TargetIdentity.h"
#include "target/TargetResolver.h"
#include "target/TargetSelector.h"
#include "target/TargetTrackPlanner.h"
#include "target/TargetTypes.h"
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#include "viewer/EquirectView.h"
#include "viewer/ViewerProjection.h"
#include "viewer/ViewerScene.h"
#include "viewer/ViewportState.h"

namespace {

// Deterministic synthetic equirectangular test pattern.
// Region colors are distinct so camera-view behavior is verifiable by color.
const QColor kPatternBackground(10, 10, 12);
const QColor kFrontColor(255, 213, 79);
const QColor kRightColor(240, 98, 146);
const QColor kLeftColor(129, 199, 132);
const QColor kUpColor(186, 104, 200);
const QColor kDownColor(255, 138, 101);
const QColor kUp20Color(0, 200, 200);
const QColor kUp30Color(255, 255, 255);

struct Region
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double halfYawDeg = 0.0;
    double halfPitchDeg = 0.0;
    QColor color;
};

QColor patternColorAt(double yawDeg, double pitchDeg)
{
    // Pole bands (whole top/bottom rows).
    if (pitchDeg >= 86.0) {
        return kUpColor;
    }
    if (pitchDeg <= -86.0) {
        return kDownColor;
    }

    const Region regions[] = {
        { 0.0, 0.0, 6.0, 6.0, kFrontColor },
        { 90.0, 0.0, 6.0, 6.0, kRightColor },
        { -90.0, 0.0, 6.0, 6.0, kLeftColor },
        { 0.0, 20.0, 6.0, 4.0, kUp20Color },
        { 0.0, 30.0, 6.0, 4.0, kUp30Color },
    };
    for (const Region &region : regions) {
        if (qAbs(yawDeg - region.yawDeg) <= region.halfYawDeg
            && qAbs(pitchDeg - region.pitchDeg) <= region.halfPitchDeg) {
            return region.color;
        }
    }
    return kPatternBackground;
}

QImage buildTestPattern(int width = 720, int height = 360)
{
    QImage pattern(width, height, QImage::Format_ARGB32);
    for (int sy = 0; sy < height; ++sy) {
        const double pitch = 90.0 - (sy + 0.5) * 180.0 / height;
        for (int sx = 0; sx < width; ++sx) {
            const double yaw = (sx + 0.5) * 360.0 / width - 180.0;
            pattern.setPixel(sx, sy, patternColorAt(yaw, pitch).rgb());
        }
    }
    return pattern;
}

bool imagesIdentical(const QImage &a, const QImage &b)
{
    if (a.size() != b.size() || a.format() != b.format()) {
        return false;
    }
    if (a.isNull() || b.isNull()) {
        return a.isNull() && b.isNull();
    }
    return std::memcmp(a.constBits(), b.constBits(), a.sizeInBytes()) == 0;
}

// Counts pixels of a color in the four screen quadrants around the center.
struct QuadrantCounts
{
    int above = 0;
    int below = 0;
    int left = 0;
    int right = 0;
};

QuadrantCounts countQuadrants(const QImage &image, const QColor &color)
{
    QuadrantCounts counts;
    const int centerX = image.width() / 2;
    const int centerY = image.height() / 2;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != color) {
                continue;
            }
            if (y < centerY) {
                ++counts.above;
            } else if (y > centerY) {
                ++counts.below;
            }
            if (x < centerX) {
                ++counts.left;
            } else if (x > centerX) {
                ++counts.right;
            }
        }
    }
    return counts;
}

// Per-second solid-color frames (red, green, blue, magenta) encoded to a
// keyframe-every-frame H.264 clip at 1 fps so seeks are accurate. Lossy codec
// conversion is tolerated via channel-dominance assertions.
bool createSteppedVideo(const QString &directory, const QString &ffmpegPath,
                        QString *outVideoPath)
{
    const QColor frameColors[] = {
        QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 0, 255)
    };
    for (int i = 0; i < 4; ++i) {
        QImage frame(64, 32, QImage::Format_RGB32);
        frame.fill(frameColors[i]);
        if (!frame.save(directory + QStringLiteral("/p%1.png").arg(i), "PNG")) {
            return false;
        }
    }

    const QString videoPath = directory + QStringLiteral("/stepped.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QStringLiteral("1"),
        QStringLiteral("-i"), directory + QStringLiteral("/p%d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QStringLiteral("1"),
        videoPath
    });
    const bool started = process.waitForStarted(10000);
    if (started) {
        process.waitForFinished(30000);
    }
    for (int i = 0; i < 4; ++i) {
        QFile::remove(directory + QStringLiteral("/p%1.png").arg(i));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

bool redDominant(const QColor &color)
{
    return color.red() > color.green() + 80 && color.red() > color.blue() + 80;
}

bool blueDominant(const QColor &color)
{
    return color.blue() > color.red() + 80 && color.blue() > color.green() + 80;
}

// Objective 15: deterministic equirectangular long-clip review fixture.
const QColor kReviewFrameColors[] = {
    QColor(230, 0, 0),   // red
    QColor(0, 230, 0),   // green
    QColor(0, 0, 230),   // blue
};
const QColor kReviewRightColor(230, 0, 230); // magenta: unique vs the cycle

bool reviewRedDominant(const QColor &color)
{
    return color.red() > 140 && color.green() < 110 && color.blue() < 110;
}
bool reviewGreenDominant(const QColor &color)
{
    return color.green() > 140 && color.red() < 110 && color.blue() < 110;
}
bool reviewBlueDominant(const QColor &color)
{
    return color.blue() > 140 && color.red() < 110 && color.green() < 110;
}
bool reviewMagentaDominant(const QColor &color)
{
    return color.red() > 140 && color.blue() > 140 && color.green() < 110;
}

QImage buildReviewFrame(int width, int height, int frameIndex)
{
    const QColor frontColor = kReviewFrameColors[frameIndex % 3];
    QImage frame(width, height, QImage::Format_ARGB32);
    frame.fill(QColor(10, 10, 12));
    for (int sy = 0; sy < height; ++sy) {
        const double pitch = 90.0 - (sy + 0.5) * 180.0 / height;
        for (int sx = 0; sx < width; ++sx) {
            const double yaw = (sx + 0.5) * 360.0 / width - 180.0;
            QColor color;
            if (qAbs(yaw) <= 10.0 && qAbs(pitch) <= 8.0) {
                color = frontColor;
            } else if (yaw >= 80.0 && yaw <= 100.0 && qAbs(pitch) <= 30.0) {
                color = kReviewRightColor;
            } else {
                continue;
            }
            frame.setPixel(sx, sy, color.rgb());
        }
    }
    return frame;
}

bool createEquirectReviewVideo(const QString &directory, const QString &ffmpegPath,
                               int frameCount, QString *outVideoPath)
{
    if (frameCount <= 0) {
        return false;
    }
    for (int i = 0; i < frameCount; ++i) {
        const QString name = QStringLiteral("/r_%1.png")
                                 .arg(i, 2, 10, QLatin1Char('0'));
        if (!buildReviewFrame(360, 180, i).save(directory + name, "PNG")) {
            return false;
        }
    }
    const QString videoPath = directory + QStringLiteral("/review_clip.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QStringLiteral("1"),
        QStringLiteral("-i"), directory + QStringLiteral("/r_%02d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QStringLiteral("1"),
        videoPath
    });
    const bool started = process.waitForStarted(15000);
    if (started) {
        process.waitForFinished(60000);
    }
    for (int i = 0; i < frameCount; ++i) {
        QFile::remove(directory + QStringLiteral("/r_%1.png")
                                  .arg(i, 2, 10, QLatin1Char('0')));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

// --- Phase 3 Objective 2: TEMPORARY feasibility probe helpers (tests only) ---
// These deliberately do NOT introduce production infrastructure. They prove that
// a persistent FFmpeg subprocess can stream frames, hit EOF, and be killed/
// restarted cleanly in this environment. Transport details (rawvideo, pacing,
// buffering) are recorded as measurements only, NOT decisions.

bool createStreamProbeVideo(const QString &directory, const QString &ffmpegPath,
                            int frameCount, int framesPerSecond, QString *outVideoPath)
{
    const int width = 160;
    const int height = 80;
    if (frameCount <= 0 || framesPerSecond <= 0) {
        return false;
    }
    for (int i = 0; i < frameCount; ++i) {
        const QString name = QStringLiteral("/s_%1.png")
                                 .arg(i, 3, 10, QLatin1Char('0'));
        if (!buildReviewFrame(width, height, i).save(directory + name, "PNG")) {
            return false;
        }
    }
    const QString videoPath = directory + QStringLiteral("/stream_probe.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QString::number(framesPerSecond),
        QStringLiteral("-i"), directory + QStringLiteral("/s_%03d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QString::number(framesPerSecond),
        videoPath
    });
    const bool started = process.waitForStarted(15000);
    if (started) {
        process.waitForFinished(60000);
    }
    for (int i = 0; i < frameCount; ++i) {
        QFile::remove(directory + QStringLiteral("/s_%1.png")
                                  .arg(i, 3, 10, QLatin1Char('0')));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

// Reads exactly one raw rgb24 frame (w*h*3 bytes) from the process stdout with a
// bounded total wait. Returns false on timeout/EOF without a full frame.
bool readRawFrame(QProcess &process, int width, int height, QByteArray &pending,
                  QImage *outImage, int timeoutMs)
{
    const qint64 need = static_cast<qint64>(width) * height * 3;
    QElapsedTimer timer;
    timer.start();
    while (pending.size() < need && timer.elapsed() < timeoutMs) {
        if (process.waitForReadyRead(250)) {
            pending += process.readAll();
        } else if (process.state() == QProcess::NotRunning) {
            break;
        }
    }
    if (pending.size() < need) {
        return false;
    }
    const QImage image(reinterpret_cast<const uchar *>(pending.constData()),
                       width, height, width * 3, QImage::Format_RGB888);
    *outImage = image.copy();
    pending.remove(0, need);
    return true;
}

bool classifyFrameColor(const QImage &frame, int expectedIndex)
{
    const QColor center = frame.pixelColor(frame.width() / 2, frame.height() / 2);
    switch (expectedIndex % 3) {
    case 0:
        return reviewRedDominant(center);
    case 1:
        return reviewGreenDominant(center);
    default:
        return reviewBlueDominant(center);
    }
}

// --- Phase 3 Objective 3: replaceable media-source test double (tests only) ---
// A deterministic in-memory FrameSource implementation. It proves the seam is
// genuinely replaceable: FramePump is driven exactly through the abstract
// contract with no ffmpeg subprocess, media file, or timing dependency, and
// every ReadResult branch can be produced on demand.
class FakeFrameSource : public FrameSource
{
public:
    enum class Mode { Frames, Error, Timeout };

    void appendFrame(const QImage &frame) { m_frames.append(frame); }
    void setMode(Mode mode) { m_mode = mode; }
    void setErrorText(const QString &text) { m_error = text; }
    int closeCount() const { return m_closeCount; }

    bool readNextFrame(int, ReadResult *result, QImage *outFrame) override
    {
        if (m_mode == Mode::Error) {
            if (result) {
                *result = ReadResult::Error;
            }
            return false;
        }
        if (m_mode == Mode::Timeout) {
            if (result) {
                *result = ReadResult::Timeout;
            }
            return false;
        }
        if (m_index >= m_frames.size()) {
            if (result) {
                *result = ReadResult::EndOfStream;
            }
            return false;
        }
        if (outFrame) {
            *outFrame = m_frames.at(m_index);
        }
        ++m_index;
        if (result) {
            *result = ReadResult::Ok;
        }
        return true;
    }
    void close() override
    {
        ++m_closeCount;
        m_open = false;
    }
    bool isOpen() const override { return m_open; }
    QString errorString() const override { return m_error; }

private:
    QList<QImage> m_frames;
    int m_index = 0;
    int m_closeCount = 0;
    bool m_open = true;
    Mode m_mode = Mode::Frames;
    QString m_error;
};

// --- Phase 3 Objective 4: deterministic player/timing test doubles ---
// ManualClock removes wall-clock dependence; RecordingPacingPolicy records the
// pacing inputs and returns a caller-controlled frame count so player behavior
// is verified without real-time delays.
class ManualClock : public Clock
{
public:
    qint64 nowMillis() const override { return m_now; }
    void setMillis(qint64 now) { m_now = now; }
    void advance(qint64 deltaMs) { m_now += deltaMs; }

private:
    qint64 m_now = 0;
};

class RecordingPacingPolicy : public PacingPolicy
{
public:
    explicit RecordingPacingPolicy(int framesPerCall = 0)
        : m_framesPerCall(framesPerCall)
    {
    }

    int framesToAdvance(qint64 elapsedMs, qint64 frameIntervalMs) const override
    {
        m_lastElapsedMs = elapsedMs;
        m_lastFrameIntervalMs = frameIntervalMs;
        ++m_calls;
        return m_framesPerCall;
    }

    void setFramesPerCall(int frames) { m_framesPerCall = frames; }
    qint64 lastElapsedMs() const { return m_lastElapsedMs; }
    qint64 lastFrameIntervalMs() const { return m_lastFrameIntervalMs; }
    int calls() const { return m_calls; }

private:
    int m_framesPerCall = 0;
    mutable qint64 m_lastElapsedMs = 0;
    mutable qint64 m_lastFrameIntervalMs = 0;
    mutable int m_calls = 0;
};

QImage playerTestFrame(int index)
{
    QImage image(4, 3, QImage::Format_RGB32);
    image.fill(QColor((index * 37) % 256, (index * 53) % 256, (index * 71) % 256));
    return image;
}

// --- 360 reframing engine test helpers -------------------------------------
// A deterministic in-memory ReframeFrameProvider. It proves the reframing
// engine is genuinely decoder-independent and lets the camera path, rendering,
// and failure paths be verified without any FFmpeg subprocess.
class SyntheticEquirectProvider : public ReframeFrameProvider
{
public:
    SyntheticEquirectProvider(int width = 720, int height = 360)
        : m_width(width), m_height(height) {}

    void setStaticPattern(bool staticPattern) { m_staticPattern = staticPattern; }
    void setFailAfterCalls(int calls) { m_failAfterCalls = calls; }
    int calls() const { return m_calls; }

    bool frameAt(qint64 timeMs, QImage *outFrame, QString *error) override
    {
        ++m_calls;
        if (m_failAfterCalls >= 0 && m_calls > m_failAfterCalls) {
            if (error) {
                *error = QStringLiteral("synthetic provider failure");
            }
            return false;
        }
        if (!outFrame) {
            return false;
        }
        if (m_staticPattern) {
            *outFrame = buildTestPattern(m_width, m_height);
        } else {
            const int bucket = static_cast<int>((timeMs / 1000) % 3);
            *outFrame = buildReviewFrame(m_width, m_height, bucket);
        }
        return true;
    }

private:
    int m_width = 720;
    int m_height = 360;
    bool m_staticPattern = true;
    int m_failAfterCalls = -1;
    int m_calls = 0;
};

CameraKeyframe makeKeyframe(qint64 timeMs, double yawDeg, double pitchDeg = 0.0,
                            double rollDeg = 0.0, double fovDeg = 90.0,
                            CameraKeyframe::Interpolation interpolation =
                                CameraKeyframe::Interpolation::Linear)
{
    CameraKeyframe frame;
    frame.timeMs = timeMs;
    frame.yawDeg = yawDeg;
    frame.pitchDeg = pitchDeg;
    frame.rollDeg = rollDeg;
    frame.fieldOfViewDeg = fovDeg;
    frame.interpolation = interpolation;
    return frame;
}

ReframePlan makeReframePlan(qint64 startMs, qint64 endMs, int width, int height,
                            double fps, const QList<CameraKeyframe> &keyframes)
{
    ReframePlan plan;
    ReframePlan::TimeRange range;
    range.startMs = startMs;
    range.endMs = endMs;
    plan.setSourceRange(range);
    ReframePlan::OutputSpec output;
    output.width = width;
    output.height = height;
    output.fps = fps;
    plan.setOutput(output);
    plan.setKeyframes(keyframes);
    return plan;
}

// --- 360 target resolution test helpers ------------------------------------
// Deterministic, model-free helpers: a synthetic equirect frame builder, a
// color-blob detector that implements the replaceable TargetDetector seam, and
// simple frame providers. They let the geometry, tracker, resolver, and
// planner be tested end-to-end without downloading any model.

struct EquirectDisk
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double radiusDeg = 8.0;
    QColor color;
};

QImage buildTargetEquirect(int width, int height, const QList<EquirectDisk> &disks)
{
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(QColor(0, 0, 0));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const SphericalDirection direction =
                EquirectProjection::directionFromEquirectPixel(x, y, width, height);
            for (const EquirectDisk &disk : disks) {
                const SphericalDirection center{ disk.yawDeg, disk.pitchDeg };
                if (EquirectProjection::angularDistanceDeg(direction, center)
                    <= disk.radiusDeg) {
                    image.setPixel(x, y, disk.color.rgb());
                    break;
                }
            }
        }
    }
    return image;
}

class SyntheticColorDetector : public TargetDetector
{
public:
    struct Spec
    {
        QColor color;
        QString label;
        int tolerance = 40;
        double confidence = 0.9;
    };

    void addSpec(const QColor &color, const QString &label, int tolerance = 40,
                 double confidence = 0.9)
    {
        m_specs.append(Spec{ color, label, tolerance, confidence });
    }

    QString name() const override { return QStringLiteral("synthetic-color"); }

    bool detect(const QImage &view, const TargetQuery &query,
                QList<TargetDetection> *out, QString *error) override
    {
        if (error) {
            error->clear();
        }
        if (out) {
            out->clear();
        }
        if (!out || view.isNull()) {
            return false;
        }
        for (const Spec &spec : m_specs) {
            if (!query.label.isEmpty() && spec.label != query.label) {
                continue;
            }
            int minX = view.width();
            int minY = view.height();
            int maxX = -1;
            int maxY = -1;
            int count = 0;
            for (int y = 0; y < view.height(); ++y) {
                for (int x = 0; x < view.width(); ++x) {
                    const QColor color = view.pixelColor(x, y);
                    const int diff = qMax(qMax(qAbs(color.red() - spec.color.red()),
                                               qAbs(color.green() - spec.color.green())),
                                          qAbs(color.blue() - spec.color.blue()));
                    if (diff <= spec.tolerance) {
                        minX = qMin(minX, x);
                        minY = qMin(minY, y);
                        maxX = qMax(maxX, x);
                        maxY = qMax(maxY, y);
                        ++count;
                    }
                }
            }
            if (count < 4) {
                continue;
            }
            TargetDetection detection;
            detection.boundingBox =
                QRectF(minX, minY, maxX - minX + 1, maxY - minY + 1);
            detection.label = spec.label;
            detection.confidence = spec.confidence;
            out->append(detection);
        }
        return true;
    }

private:
    QList<Spec> m_specs;
};

class StaticEquirectProvider : public ReframeFrameProvider
{
public:
    explicit StaticEquirectProvider(QImage image) : m_image(std::move(image)) {}

    bool frameAt(qint64, QImage *outFrame, QString *error) override
    {
        if (error) {
            error->clear();
        }
        if (!outFrame) {
            return false;
        }
        *outFrame = m_image;
        return true;
    }

private:
    QImage m_image;
};

// A frame provider whose target disk moves linearly in yaw over a duration.
class MovingDiskProvider : public ReframeFrameProvider
{
public:
    MovingDiskProvider(int width, int height, QColor color, double radiusDeg)
        : m_width(width), m_height(height), m_color(color), m_radius(radiusDeg)
    {
    }

    void setMotion(double startYawDeg, double endYawDeg, qint64 durationMs)
    {
        m_startYaw = startYawDeg;
        m_endYaw = endYawDeg;
        m_duration = durationMs > 0 ? durationMs : 1;
    }

    bool frameAt(qint64 timeMs, QImage *outFrame, QString *error) override
    {
        if (error) {
            error->clear();
        }
        if (!outFrame) {
            return false;
        }
        const double t =
            qBound(0.0, static_cast<double>(timeMs) / m_duration, 1.0);
        const double yaw = m_startYaw + (m_endYaw - m_startYaw) * t;
        QList<EquirectDisk> disks;
        disks.append(EquirectDisk{ yaw, 0.0, m_radius, m_color });
        *outFrame = buildTargetEquirect(m_width, m_height, disks);
        return true;
    }

private:
    int m_width;
    int m_height;
    QColor m_color;
    double m_radius;
    double m_startYaw = 0.0;
    double m_endYaw = 0.0;
    qint64 m_duration = 1000;
};

} // namespace

class TestMainWindow : public MainWindow
{
public:
    explicit TestMainWindow(QWidget *parent = nullptr) : MainWindow(parent) {}

protected:
    QString chooseSaveFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
    QString chooseOpenFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
    QString chooseMediaFilePath() override { return QStringLiteral("/tmp/reelcraft_media_test.bin"); }
};

class ProjectTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void newProjectHasValidDefaults();
    void saveAndLoadPreservesData();
    void loadMissingFileReturnsInvalid();
    void saveToInvalidPathFails();
    void applicationNewProjectEmitsSignal();
    void applicationOpenProjectEmitsSignal();
    void applicationBackgroundDemoEmitsSignal();
    void mainWindowButtonsEmitSignals();
    void mainWindowProjectLabelUpdates();
    void mainWindowStatusLabelUpdates();
    void applicationSaveInvalidPathEmitsError();
    void applicationOpenMissingFileEmitsError();
    void projectLifecycleDoesNotModifyOriginalMedia();
    void viewportStateDefaultsAreValid();
    void viewportStateYawNormalizes();
    void viewportStatePitchClamps();
    void viewportStateRollNormalizes();
    void viewportStateFieldOfViewClamps();
    void viewportStateSettersEmitSignalsOnlyOnChange();
    void applicationViewportStateIsValid();
    void applicationResetViewportRestoresDefaults();
    void mainWindowViewerLabelsUpdate();
    void mainWindowResetViewportButtonEmitsSignal();
    void applicationViewerStateUpdatesMainWindowLabels();
    void applicationResetViewportUpdatesMainWindowLabels();
    void mainWindowKeyboardEmitsViewportDeltas();
    void applicationKeyboardUpdatesViewportState();
    void newProjectResetsViewport();
    void openProjectResetsViewport();
    void viewportStateJsonRoundTrip();
    void viewportStateRejectsInvalidJson();
    void applicationSaveAndOpenPersistsViewportState();
    void restoredViewerStateUpdatesMainWindowLabels();
    void focusedButtonKeyPressEmitsViewportDelta();
    void invalidPersistedViewerStateFallsBackToDefaults();
    void applicationAdjustViewportSlots();
    void projectDefaultsHaveSchemaVersion();
    void legacyProjectWithoutVersionLoadsSchemaOne();
    void projectSchemaVersionRoundTrip();
    void futureProjectSchemaVersionIsRejected();
    void applicationOpenFutureSchemaProjectFailsSafely();
    void viewerSceneGroundTruthIsDeterministic();
    void viewerSceneMarkerRangesAreValid();
    void viewerWidgetPresentsDeterministicScene();
    void mainWindowContainsViewerSurface();
    void viewerWidgetRenderIsDeterministic();
    void viewerProjectionCentersFrontMarkerAtIdentity();
    void viewerProjectionYawAimsAtSideMarkers();
    void viewerProjectionPitchAimsAtPoleMarkers();
    void viewerProjectionPitchMirrorSymmetry();
    void viewerProjectionRollRotatesViewContent();
    void viewerProjectionLargerFieldOfViewBringsMarkersCloser();
    void viewerWidgetCameraFollowsApplicationViewportState();
    void mediaItemRecordsMetadataFromRealFile();
    void mediaItemRejectsInvalidPaths();
    void mediaItemIdIsDeterministic();
    void mediaItemJsonRoundTrip();
    void mediaItemRejectsInvalidJson();
    void applicationImportMediaRequiresActiveProject();
    void applicationImportMediaDeduplicatesAndOrders();
    void applicationImportMediaRejectsInvalidPath();
    void mediaReferencesPersistAndReopenDeterministically();
    void originalMediaUnchangedByImportAndLifecycle();
    void legacyProjectOpensWithEmptyMedia();
    void invalidPersistedMediaFallsBackSafely();
    void mainWindowImportButtonEmitsSignal();
    void reopenWithAvailableMediaIsSilentAndIdentical();
    void reopenAfterDeletingMediaFileFlagsUnavailable();
    void reopenAfterMovingMediaFileFlagsUnavailable();
    void duplicatePersistedMediaNormalizedOnOpenAndResave();
    void newProjectClearsUnavailableMediaState();
    void mediaImportEmitsMediaListChanged();
    void openProjectRestoresAndEmitsMediaListChanged();
    void newProjectEmitsEmptyMediaListChanged();
    void removeMediaRemovesMatchingRecordOnlyAndPersists();
    void removeMediaUnknownIdFailsDeterministically();
    void removeMediaRequiresActiveProject();
    void removeUnavailableMediaClearsUnavailableState();
    void mainWindowMediaListPopulatedAndRemoveWorks();
    void activeMediaRequiresProjectAndSetsState();
    void activeMediaSameIdAndUnknownIdBehavior();
    void activeMediaImportNeverAutoSelects();
    void activeMediaRemovalRules();
    void activeMediaClearedOnNewProject();
    void activeMediaRoundTripRestoresOnReopen();
    void activeMediaOpenClearsDanglingOrLegacy();
    void activeMediaUnavailableCannotBeActive();
    void mainWindowSetActiveAndLabelWork();
    void equirectViewIdentityCentersFront();
    void equirectViewYawCentersRightAndLeft();
    void equirectViewPitchReachesUpAndDown();
    void equirectViewPositiveRollRotatesContentCorrectly();
    void equirectViewFieldOfViewChangesCoverage();
    void equirectViewRejectsInvalidInput();
    void equirectViewDeterministicRepeatability();
    void viewerWidgetSourceImageRendersThroughCamera();
    void viewerWidgetClearingSourceRestoresSceneRendering();
    void equirectViewPerformanceSanity();
    void frameExtractorRejectsInvalidInput();
    void frameExtractorAvailabilityAndSingleFrameDecode();
    void frameExtractorDeterministicRepeatability();
    void applicationPreviewRequiresProjectAndActive();
    void applicationPreviewEmitsFramePreview();
    void mainWindowPreviewButtonEmitsSignal();
    void previewEndToEndShowsActiveFrameInViewer();
    void frameExtractorRejectsInvalidSeekTime();
    void frameExtractorExtractsFrameAtSeekTime();
    void frameExtractorSeekDeterministicRepeatability();
    void applicationTimeNavigationGuards();
    void applicationPreviewAtUpdatesPositionAndEmits();
    void applicationStepAdvancesAndClampsBelowZero();
    void applicationSeekBeyondEndFailsDeterministically();
    void applicationTimeResetsOnProjectActiveAndRemoval();
    void mainWindowStepButtonsAndTimeLabel();
    void viewerWidgetDragEmitsYawAndPitchDeltas();
    void viewerWidgetIgnoresNonLeftDragAndReleaseWithoutMove();
    void viewerWidgetWheelUpDecreasesFovWheelDownIncreases();
    void viewerWidgetDragUpdatesApplicationViewport();
    void mediaItemProjectionDefaultsUnknown();
    void mediaItemProjectionJsonRoundTrip();
    void applicationDeclareProjectionGuardsValidateAndEmit();
    void applicationDeclaredProjectionPersistsOnReopen();
    void viewerWidgetFlatModeLetterboxesAndCenters();
    void viewerWidgetFlatModeIgnoresCameraTransforms();
    void mainWindowProjectionButtonsEmitRequests();
    void previewRoutingHonorsDeclaredProjection();
    void viewerClearedOnNewProjectAfterPreview();
    void viewerClearedWhenActiveMediaChangesOrRemoved();
    void viewerSourcePersistsWhileContextStable();
    void equirectViewBilinearBlendsFourNeighbors();
    void equirectViewBilinearRobustAtSeamAndPoles();
    void equirectReviewFixtureFramesAreDistinctAndSeekable();
    void reviewPathEndToEndOnEquirectClip();
    void equirectReviewPerformanceInformational();
    void persistentStreamProbeDeliversFramesInOrder();
    void persistentStreamProbeErrorOnMissingFile();
    void persistentStreamProbeKillAndRestartLifecycle();
    void framePumpDeliversFramesAndEndFromReplaceableSource();
    void framePumpHandlesErrorTimeoutAndMissingSource();
    void fmpegFrameSourceStreamsFramesInOrder();
    void fmpegFrameSourceValidatesAndRestartsCleanly();
    void framePumpStreamsFfmpegSourceToEnd();
    void playheadTracksFrameCountIndexAndPosition();
    void playerInitialStateAndDefaults();
    void playerPlayPauseStopTransitions();
    void playerTickUsesInjectedClockAndPacingPolicy();
    void playerDefaultPacingAdvancesOnFrameIntervals();
    void playerStepOncePresentsExactlyOneFrame();
    void playerReachesEndOfStreamDeterministically();
    void playerReportsFramePumpErrors();
    void playerHandlesInvalidConfigurationAndBoundaries();
    void playerTickIsRepeatableWithoutWallClockDelays();
    void reframeCameraKeyframeJsonRoundTrip();
    void reframeCameraKeyframeRejectsInvalid();
    void reframePlanJsonRoundTrip();
    void reframePlanRejectsInvalid();
    void reframePlanFrameTimingIsDeterministic();
    void cameraPathHoldsOutsideKeyframes();
    void cameraPathInterpolatesLinearly();
    void cameraPathUsesShortestYawPath();
    void cameraPathHonorsHoldInterpolation();
    void cameraPathNormalizesAndClamps();
    void reframeRendererRendersDeterministicFrames();
    void reframeRendererFollowsCameraPath();
    void reframeRendererRejectsInvalidInputs();
    void reframeRendererReportsProviderFailure();
    void reframeRendererPngSequenceAndEncode();
    void reframeIntentParsesAspectAndPlatform();
    void reframeIntentParsesTimeRanges();
    void reframeIntentParsesNamedDirections();
    void reframeIntentParsesSubjectReferencesAsUnresolved();
    void reframeIntentHandlesGarbage();
    void reframeBuilderBuildsStaticPlan();
    void reframeBuilderBuildsTwoStopPath();
    void reframeBuilderResolvesTargetsCaseInsensitively();
    void reframeBuilderRejectsUnresolvedTargets();
    void reframeBuilderHonorsIntentTimeRangeAndOutput();
    void reframePipelineRendersRealVideoEndToEnd();
    void equirectDirectionFromCenterAndSides();
    void equirectPixelRoundTrip();
    void equirectAngularDistanceHandlesSeam();
    void equirectPitchClampAndValidity();
    void equirectViewCenterMatchesDirection();
    void equirectViewDirectionRoundTrip();
    void equirectViewRejectsBehindCamera();
    void equirectDetectionToDirectionMapsBox();
    void equirectDetectionRejectsInvalidBox();
    void equirectViewPlanCoversSphere();
    void equirectViewPlanIsDeterministic();
    void equirectViewPlanAddsPolarViews();
    void targetTrackerCreatesTrackFromDetection();
    void targetTrackerPersistsIdentityAcrossFrames();
    void targetTrackerSeparatesDistinctTargets();
    void targetTrackerMergesNearDuplicates();
    void targetTrackerGreedyPrefersNearest();
    void targetTrackerGateCreatesNewTrack();
    void targetTrackerDeactivatesAfterMisses();
    void targetTrackerFiltersLowConfidence();
    void targetTrackerSeamContinuity();
    void targetTrackerIsDeterministic();
    void targetTrackSampleAtInterpolates();
    void targetTrackRepresentativeTarget();
    void targetResolverFindsSyntheticTarget();
    void targetResolverHonorsLabelQuery();
    void targetResolverReportsUnresolved();
    void targetResolverRejectsInvalidInput();
    void targetResolverIsDeterministic();
    void targetResolverSequenceBuildsTrajectory();
    void targetResolverFeedsReframePlanBuilder();
    void targetProcessDetectorParsesResponse();
    void targetProcessDetectorRunsHelper();
    void targetProcessDetectorFailsOnMissingExecutable();
    void targetProcessDetectorFailsOnBadExit();
    void targetTrackPlannerBuildsFollowPlan();
    void targetTrackPlannerRejectsEmptyTrack();
    void targetTrackPlannerFiltersLowConfidence();
    void targetResolutionToRenderPipeline();
    void realDetectorIntegration();
    void targetTrackerPredictionMaintainsIdentityThroughCrossing();
    void targetTrackerReentryKeepsIdentityWithinWindow();
    void targetTrackerReentryBeyondWindowCreatesNewTrack();
    void targetTrackerPredictionIsDeterministic();
    void targetIdentityBindsFromSeedDirection();
    void targetIdentitySeedRejectsDistantOrInvalid();
    void targetIdentityTrackIdBindingAndClaimConflicts();
    void targetIdentityResolutionTracksActiveState();
    void targetIdentityContinuityRebindIsUnique();
    void targetIdentityContinuityRebindAmbiguousIsUnresolved();
    void targetIdentityJsonRoundTrip();
    void targetSelectorResolvesCreatorAliases();
    void targetSelectorUnresolvedCreatorWhenUnbound();
    void targetSelectorResolvesOtherPerson();
    void targetSelectorOtherPersonAmbiguous();
    void targetSelectorOrdinalIsDeterministicRegardlessOfInputOrder();
    void targetSelectorOrdinalOutOfRange();
    void targetSelectorLeftRightByYaw();
    void targetSelectorTrackIdAndUniqueLabel();
    void targetSelectorIsDeterministic();
    void targetSelectorResolvedTargetsFeedsReframePlanBuilder();
    void appearanceEmbeddingJsonRoundTrip();
    void appearanceEmbeddingRejectsInvalid();
    void appearanceNormalizeAndCosine();
    void appearanceAggregateAveragesAndNormalizes();
    void appearanceVerdictAndEvidenceJson();
    void appearanceProfileJsonRoundTrip();
    void appearanceProcessParseResponse();
    void appearanceProcessRunsHelper();
    void appearanceProcessFailsOnMissingExecutable();
    void appearanceProcessFailsOnBadExit();
    void appearanceTargetCropIsDeterministic();
    void appearanceRegistryProfileAndJson();
    void appearanceRegistryRejectPreventsGeometricRebind();
    void appearanceRegistryRebindWithAppearance();
    void appearanceReidentifierStrongReacquire();
    void appearanceReidentifierWeakMatchStaysUnresolved();
    void appearanceReidentifierAmbiguousCandidates();
    void appearanceReidentifierWrongPersonRejected();
    void appearanceReidentifierGeometryAppearanceConflict();
    void appearanceReidentifierExplicitSelectionPrecedence();
    void appearanceReidentifierMissingProvider();
    void appearanceReidentifierProviderFailure();
    void appearanceReidentifierCandidateOrderDeterministic();
    void appearanceReidentifierGeometryConfirmsAppearance();
    void speakerIntervalAndAnalysisJson();
    void speakerVerdictEvidenceAndSegmentJson();
    void speakerIntervalRejectsInvalid();
    void speakerProcessParseResponse();
    void speakerProcessRunsHelper();
    void speakerProcessFailsOnMissingExecutable();
    void speakerProcessFailsOnBadExit();
    void speakerAssociatorExplicitBinding();
    void speakerAssociatorExplicitBindingNotVisible();
    void speakerAssociatorSingleVisible();
    void speakerAssociatorMultipleVisibleIsAmbiguous();
    void speakerAssociatorSpatialAzimuth();
    void speakerTimelineSingleSpeaker();
    void speakerTimelineSpeakerChange();
    void speakerTimelineShortPauseIsHeld();
    void speakerTimelineRapidAlternationDoesNotThrash();
    void speakerTimelineOverlapIsPreserved();
    void speakerTimelineFiltersShortAndLowConfidence();
    void speakerAnalyzerAssociatesSingleSpeaker();
    void speakerAnalyzerExplicitBindingWithTwoVisible();
    void speakerAnalyzerWithoutProviderIsUnavailable();
    void speakerAnalyzerProviderFailureIsFailSafe();
    void speakerAnalyzerUnassociatedSpeechDoesNotInventTarget();
    void speakerAnalyzerTrackingLossKeepsNoTarget();
    void speakerAnalyzerOverlapIsPreserved();
    void speakerAnalyzerIsDeterministic();
    void speakerAnalyzerEvidenceDoesNotChangeIdentity();
    void speakerPlannerFollowsSingleSpeaker();
    void speakerPlannerCutsOnSpeakerChange();
    void speakerPlannerRejectsNoActiveSegment();
    void speakerRegistryAnnotateDoesNotChangeResolution();
};

void ProjectTest::initTestCase()
{
    qRegisterMetaType<Project>("Project");
    qRegisterMetaType<MediaItem>("MediaItem");
    qRegisterMetaType<QList<MediaItem>>("QList<MediaItem>");
    qRegisterMetaType<Player::State>("Player::State");
}

void ProjectTest::newProjectHasValidDefaults()
{
    Project project;
    QVERIFY(project.isValid());
    QVERIFY(!project.id().isEmpty());
    QCOMPARE(project.name(), QString("Untitled Project"));
    QVERIFY(project.created().isValid());
}

void ProjectTest::saveAndLoadPreservesData()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Demo Project"));

    const QString filePath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(project.save(filePath));

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);
    QVERIFY(ok);
    QVERIFY(loaded.isValid());
    QCOMPARE(loaded.name(), QStringLiteral("Demo Project"));
    QCOMPARE(loaded.id(), project.id());
    QCOMPARE(loaded.created(), project.created());
}

void ProjectTest::loadMissingFileReturnsInvalid()
{
    bool ok = true;
    Project loaded = Project::load(QStringLiteral("/nonexistent/nope.reel"), &ok);
    QVERIFY(!ok);
    QVERIFY(!loaded.isValid());
}

void ProjectTest::saveToInvalidPathFails()
{
    Project project;
    QString error;
    QVERIFY(!project.save(QStringLiteral("/nonexistent_dir/nope.reel"), &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationNewProjectEmitsSignal()
{
    Application app;
    QSignalSpy spy(&app, &Application::projectChanged);

    app.newProject();

    QCOMPARE(spy.count(), 1);
    QVERIFY(app.hasProject());
    QVERIFY(app.currentProject().isValid());
}

void ProjectTest::applicationOpenProjectEmitsSignal()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project original;
    original.setName(QStringLiteral("Reopen Me"));
    const QString filePath = tempDir.filePath(QStringLiteral("reopen.reel"));
    QVERIFY(original.save(filePath));

    Application app;
    QSignalSpy spy(&app, &Application::projectChanged);

    QVERIFY(app.openProject(filePath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(app.hasProject());
    QCOMPARE(app.currentProject().name(), QStringLiteral("Reopen Me"));
    QCOMPARE(app.currentProject().id(), original.id());
}

void ProjectTest::applicationBackgroundDemoEmitsSignal()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    app.runBackgroundDemo();

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 2000);
    QVERIFY(!app.hasProject());
}

void ProjectTest::mainWindowButtonsEmitSignals()
{
    TestMainWindow window;
    QSignalSpy newSpy(&window, &MainWindow::newProjectRequested);
    QSignalSpy saveSpy(&window, &MainWindow::saveProjectRequested);
    QSignalSpy openSpy(&window, &MainWindow::openProjectRequested);
    QSignalSpy bgSpy(&window, &MainWindow::backgroundDemoRequested);

    auto *newButton = window.findChild<QPushButton*>("newProjectButton");
    auto *saveButton = window.findChild<QPushButton*>("saveProjectButton");
    auto *openButton = window.findChild<QPushButton*>("openProjectButton");
    auto *bgButton = window.findChild<QPushButton*>("backgroundDemoButton");

    QVERIFY(newButton);
    QVERIFY(saveButton);
    QVERIFY(openButton);
    QVERIFY(bgButton);

    newButton->click();
    saveButton->click();
    openButton->click();
    bgButton->click();

    QCOMPARE(newSpy.count(), 1);
    QCOMPARE(saveSpy.count(), 1);
    QCOMPARE(openSpy.count(), 1);
    QCOMPARE(bgSpy.count(), 1);
}

void ProjectTest::mainWindowProjectLabelUpdates()
{
    TestMainWindow window;
    Project project;
    project.setName(QStringLiteral("Label Test"));

    window.showProject(project);

    auto *label = window.findChild<QLabel*>("projectLabel");
    QVERIFY(label);
    QVERIFY(label->text().contains(project.name()));
    QVERIFY(label->text().contains(project.id()));
}

void ProjectTest::mainWindowStatusLabelUpdates()
{
    TestMainWindow window;
    window.showStatus(QStringLiteral("Test Status"));

    auto *label = window.findChild<QLabel*>("statusLabel");
    QVERIFY(label);
    QCOMPARE(label->text(), QStringLiteral("Test Status"));
}

void ProjectTest::applicationSaveInvalidPathEmitsError()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    app.newProject();
    spy.clear();

    QVERIFY(!app.saveProject(QStringLiteral("/nonexistent_dir/nope.reel")));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Save failed")));
    QVERIFY(app.hasProject());
}

void ProjectTest::applicationOpenMissingFileEmitsError()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.openProject(QStringLiteral("/nonexistent/nope.reel")));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Open failed")));
    QVERIFY(!app.hasProject());
}


void ProjectTest::projectLifecycleDoesNotModifyOriginalMedia()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("source.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));

    const QByteArray originalBytes("FAKE_MEDIA_BYTES_0123456789_REELCRAFT_PHASE1");
    QCOMPARE(mediaFile.write(originalBytes), static_cast<qint64>(originalBytes.size()));
    mediaFile.close();

    const QByteArray originalHash =
        QCryptographicHash::hash(originalBytes, QCryptographicHash::Sha256);

    Application app;
    app.newProject();

    Project project = app.currentProject();
    project.setName(QStringLiteral("Media Safety"));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(project.save(projectPath));
    QVERIFY(app.openProject(projectPath));

    QFile reopened(mediaPath);
    QVERIFY(reopened.open(QIODevice::ReadOnly));
    const QByteArray afterBytes = reopened.readAll();
    reopened.close();

    QCOMPARE(afterBytes, originalBytes);
    QCOMPARE(QCryptographicHash::hash(afterBytes, QCryptographicHash::Sha256), originalHash);
}


void ProjectTest::viewportStateDefaultsAreValid()
{
    ViewportState state;

    QCOMPARE(state.yaw(), 0.0);
    QCOMPARE(state.pitch(), 0.0);
    QCOMPARE(state.roll(), 0.0);
    QCOMPARE(state.fieldOfView(), 90.0);
}

void ProjectTest::viewportStateYawNormalizes()
{
    ViewportState state;

    state.setYaw(190.0);
    QVERIFY(qAbs(state.yaw() - (-170.0)) < 0.000001);

    state.setYaw(-190.0);
    QVERIFY(qAbs(state.yaw() - 170.0) < 0.000001);
}

void ProjectTest::viewportStatePitchClamps()
{
    ViewportState state;

    state.setPitch(95.0);
    QCOMPARE(state.pitch(), 90.0);

    state.setPitch(-95.0);
    QCOMPARE(state.pitch(), -90.0);
}

void ProjectTest::viewportStateRollNormalizes()
{
    ViewportState state;

    state.setRoll(200.0);
    QVERIFY(qAbs(state.roll() - (-160.0)) < 0.000001);
}

void ProjectTest::viewportStateFieldOfViewClamps()
{
    ViewportState state;

    state.setFieldOfView(10.0);
    QCOMPARE(state.fieldOfView(), 20.0);

    state.setFieldOfView(170.0);
    QCOMPARE(state.fieldOfView(), 140.0);
}

void ProjectTest::viewportStateSettersEmitSignalsOnlyOnChange()
{
    ViewportState state;

    QSignalSpy yawSpy(&state, &ViewportState::yawChanged);
    QSignalSpy pitchSpy(&state, &ViewportState::pitchChanged);
    QSignalSpy rollSpy(&state, &ViewportState::rollChanged);
    QSignalSpy fovSpy(&state, &ViewportState::fieldOfViewChanged);

    state.setYaw(0.0);
    state.setPitch(0.0);
    state.setRoll(0.0);
    state.setFieldOfView(90.0);

    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
    QCOMPARE(rollSpy.count(), 0);
    QCOMPARE(fovSpy.count(), 0);

    state.setYaw(45.0);
    state.setPitch(-30.0);
    state.setRoll(10.0);
    state.setFieldOfView(75.0);

    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    QCOMPARE(rollSpy.count(), 1);
    QCOMPARE(fovSpy.count(), 1);
}

void ProjectTest::applicationViewportStateIsValid()
{
    Application app;

    QVERIFY(app.viewportState() != nullptr);
    QCOMPARE(app.viewportState()->yaw(), 0.0);
    QCOMPARE(app.viewportState()->pitch(), 0.0);
    QCOMPARE(app.viewportState()->roll(), 0.0);
    QCOMPARE(app.viewportState()->fieldOfView(), 90.0);
}

void ProjectTest::applicationResetViewportRestoresDefaults()
{
    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state != nullptr);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    app.resetViewport();

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::mainWindowViewerLabelsUpdate()
{
    TestMainWindow window;

    window.showYaw(30.5);
    window.showPitch(-15.25);
    window.showRoll(45.0);
    window.showFieldOfView(110.0);

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 30.50"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -15.25"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 45.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 110.00"));
}

void ProjectTest::mainWindowResetViewportButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::resetViewportRequested);

    auto *button = window.findChild<QPushButton*>("resetViewportButton");
    QVERIFY(button);

    button->click();
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::applicationViewerStateUpdatesMainWindowLabels()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(state, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(state, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(state, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(state, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);

    state->setYaw(30.5);
    state->setPitch(-15.25);
    state->setRoll(45.0);
    state->setFieldOfView(110.0);

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 30.50"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -15.25"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 45.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 110.00"));
}

void ProjectTest::applicationResetViewportUpdatesMainWindowLabels()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(state, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(state, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(state, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(state, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);
    QObject::connect(&window, &MainWindow::resetViewportRequested, &app, &Application::resetViewport);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    auto *button = window.findChild<QPushButton*>("resetViewportButton");
    QVERIFY(button);
    button->click();

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 0.00"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: 0.00"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 0.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 90.00"));
}

void ProjectTest::mainWindowKeyboardEmitsViewportDeltas()
{
    TestMainWindow window;
    QSignalSpy yawSpy(&window, &MainWindow::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&window, &MainWindow::viewportPitchDeltaRequested);
    QSignalSpy rollSpy(&window, &MainWindow::viewportRollDeltaRequested);
    QSignalSpy fovSpy(&window, &MainWindow::viewportFovDeltaRequested);

    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent q(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
    QKeyEvent minus(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier);

    QApplication::sendEvent(&window, &left);
    QApplication::sendEvent(&window, &up);
    QApplication::sendEvent(&window, &q);
    QApplication::sendEvent(&window, &minus);

    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    QCOMPARE(rollSpy.count(), 1);
    QCOMPARE(fovSpy.count(), 1);
}

void ProjectTest::applicationKeyboardUpdatesViewportState()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(&window, &MainWindow::viewportYawDeltaRequested,
                     [state](double delta) { state->setYaw(state->yaw() + delta); });
    QObject::connect(&window, &MainWindow::viewportPitchDeltaRequested,
                     [state](double delta) { state->setPitch(state->pitch() + delta); });
    QObject::connect(&window, &MainWindow::viewportRollDeltaRequested,
                     [state](double delta) { state->setRoll(state->roll() + delta); });
    QObject::connect(&window, &MainWindow::viewportFovDeltaRequested,
                     [state](double delta) { state->setFieldOfView(state->fieldOfView() + delta); });

    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent e(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier);
    QKeyEvent plus(QEvent::KeyPress, Qt::Key_Plus, Qt::NoModifier);

    QApplication::sendEvent(&window, &right);
    QApplication::sendEvent(&window, &down);
    QApplication::sendEvent(&window, &e);
    QApplication::sendEvent(&window, &plus);

    QCOMPARE(state->yaw(), 5.0);
    QCOMPARE(state->pitch(), -5.0);
    QCOMPARE(state->roll(), 5.0);
    QCOMPARE(state->fieldOfView(), 95.0);
}

void ProjectTest::newProjectResetsViewport()
{
    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    app.newProject();

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::openProjectResetsViewport()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Open Reset"));
    const QString filePath = tempDir.filePath(QStringLiteral("open_reset.reel"));
    QVERIFY(project.save(filePath));

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(25.0);
    state->setPitch(-40.0);
    state->setRoll(10.0);
    state->setFieldOfView(115.0);

    QVERIFY(app.openProject(filePath));

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::viewportStateJsonRoundTrip()
{
    ViewportState original;
    original.setYaw(30.0);
    original.setPitch(-15.0);
    original.setRoll(10.0);
    original.setFieldOfView(75.0);

    const QJsonObject object = original.toJsonObject();

    ViewportState restored;
    QString error;
    const bool ok = restored.readFromJsonObject(object, &error);

    QVERIFY(ok);
    QVERIFY(error.isEmpty());
    QCOMPARE(restored.yaw(), original.yaw());
    QCOMPARE(restored.pitch(), original.pitch());
    QCOMPARE(restored.roll(), original.roll());
    QCOMPARE(restored.fieldOfView(), original.fieldOfView());
}

void ProjectTest::viewportStateRejectsInvalidJson()
{
    ViewportState state;
    QString error;

    QJsonObject empty;
    QVERIFY(!state.readFromJsonObject(empty, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationSaveAndOpenPersistsViewportState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    app.newProject();
    state->setYaw(33.0);
    state->setPitch(-12.0);
    state->setRoll(7.0);
    state->setFieldOfView(100.0);

    const QString filePath = tempDir.filePath(QStringLiteral("viewer_persist.reel"));
    QVERIFY(app.saveProject(filePath));

    Application reopened;
    ViewportState *restored = reopened.viewportState();
    QVERIFY(restored);

    QVERIFY(reopened.openProject(filePath));

    QCOMPARE(restored->yaw(), 33.0);
    QCOMPARE(restored->pitch(), -12.0);
    QCOMPARE(restored->roll(), 7.0);
    QCOMPARE(restored->fieldOfView(), 100.0);
}

void ProjectTest::restoredViewerStateUpdatesMainWindowLabels()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    app.newProject();
    state->setYaw(42.0);
    state->setPitch(-18.0);
    state->setRoll(9.0);
    state->setFieldOfView(105.0);

    const QString filePath = tempDir.filePath(QStringLiteral("viewer_ui_restore.reel"));
    QVERIFY(app.saveProject(filePath));

    Application reopened;
    ViewportState *restored = reopened.viewportState();
    QVERIFY(restored);

    TestMainWindow window;

    QObject::connect(restored, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(restored, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(restored, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(restored, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);

    QVERIFY(reopened.openProject(filePath));

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 42.00"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -18.00"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 9.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 105.00"));
}

void ProjectTest::focusedButtonKeyPressEmitsViewportDelta()
{
    TestMainWindow window;
    QSignalSpy yawSpy(&window, &MainWindow::viewportYawDeltaRequested);

    auto *button = window.findChild<QPushButton*>("newProjectButton");
    QVERIFY(button);

    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(button, &right);

    QCOMPARE(yawSpy.count(), 1);
}

void ProjectTest::invalidPersistedViewerStateFallsBackToDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Invalid Viewer State"));

    QJsonObject invalidViewerState;
    invalidViewerState.insert(QStringLiteral("yaw"), QStringLiteral("not-a-number"));
    project.setViewerState(invalidViewerState);

    const QString filePath = tempDir.filePath(QStringLiteral("invalid_viewer.reel"));
    QVERIFY(project.save(filePath));

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(12.0);
    state->setPitch(-8.0);
    state->setRoll(3.0);
    state->setFieldOfView(100.0);

    QVERIFY(app.openProject(filePath));

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::applicationAdjustViewportSlots()
{
    Application app;

    app.adjustViewportYaw(10.0);
    app.adjustViewportPitch(-5.0);
    app.adjustViewportRoll(7.0);
    app.adjustViewportFieldOfView(-10.0);

    QCOMPARE(app.viewportState()->yaw(), 10.0);
    QCOMPARE(app.viewportState()->pitch(), -5.0);
    QCOMPARE(app.viewportState()->roll(), 7.0);
    QCOMPARE(app.viewportState()->fieldOfView(), 80.0);
}

void ProjectTest::projectDefaultsHaveSchemaVersion()
{
    Project project;
    QCOMPARE(project.schemaVersion(), Project::CurrentSchemaVersion);
}

void ProjectTest::legacyProjectWithoutVersionLoadsSchemaOne()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject legacy;
    legacy.insert(QStringLiteral("id"), QStringLiteral("legacy-id"));
    legacy.insert(QStringLiteral("name"), QStringLiteral("Legacy Project"));
    legacy.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    const QString filePath = tempDir.filePath(QStringLiteral("legacy.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(legacy).toJson(QJsonDocument::Compact));
    file.close();

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);

    QVERIFY(ok);
    QCOMPARE(loaded.schemaVersion(), 1);
}

void ProjectTest::projectSchemaVersionRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Schema Round Trip"));
    QCOMPARE(project.schemaVersion(), Project::CurrentSchemaVersion);

    const QString filePath = tempDir.filePath(QStringLiteral("schema_roundtrip.reel"));
    QVERIFY(project.save(filePath));

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);

    QVERIFY(ok);
    QCOMPARE(loaded.schemaVersion(), Project::CurrentSchemaVersion);
}

void ProjectTest::futureProjectSchemaVersionIsRejected()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject future;
    future.insert(QStringLiteral("id"), QStringLiteral("future-id"));
    future.insert(QStringLiteral("name"), QStringLiteral("Future Project"));
    future.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    future.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion + 1);

    const QString filePath = tempDir.filePath(QStringLiteral("future_schema.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(future).toJson(QJsonDocument::Compact));
    file.close();

    bool ok = true;
    QString error;
    Project loaded = Project::load(filePath, &ok, &error);

    QVERIFY(!ok);
    QVERIFY(!loaded.isValid());
    QVERIFY(error.contains(QStringLiteral("newer unsupported schema version")));
}

void ProjectTest::applicationOpenFutureSchemaProjectFailsSafely()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject future;
    future.insert(QStringLiteral("id"), QStringLiteral("future-app-id"));
    future.insert(QStringLiteral("name"), QStringLiteral("Future App Project"));
    future.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    future.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion + 1);

    const QString filePath = tempDir.filePath(QStringLiteral("future_app_schema.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(future).toJson(QJsonDocument::Compact));
    file.close();

    Application app;
    app.newProject();
    const QString originalName = app.currentProject().name();

    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.openProject(filePath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Open failed")));
    QVERIFY(app.hasProject());
    QCOMPARE(app.currentProject().name(), originalName);
}

void ProjectTest::viewerSceneGroundTruthIsDeterministic()
{
    const ViewerScene first = ViewerScene::createDeterministicTestScene();
    const ViewerScene second = ViewerScene::createDeterministicTestScene();

    QVERIFY(first.isValid());
    QCOMPARE(first.markerCount(), 10);
    QCOMPARE(first.markers().size(), second.markers().size());
    QCOMPARE(second.markers().size(), 10);

    for (int i = 0; i < first.markers().size(); ++i) {
        QCOMPARE(first.markers().at(i).label, second.markers().at(i).label);
        QVERIFY(qAbs(first.markers().at(i).yaw - second.markers().at(i).yaw) < 1e-9);
        QVERIFY(qAbs(first.markers().at(i).pitch - second.markers().at(i).pitch) < 1e-9);
    }

    const int front = first.markerIndex(QStringLiteral("FRONT"));
    QVERIFY(front >= 0);
    QVERIFY(qAbs(first.markers().at(front).yaw) < 1e-9);
    QVERIFY(qAbs(first.markers().at(front).pitch) < 1e-9);

    const int back = first.markerIndex(QStringLiteral("BACK"));
    QVERIFY(back >= 0);
    QVERIFY(qAbs(first.markers().at(back).yaw - (-180.0)) < 1e-9);
    QVERIFY(qAbs(first.markers().at(back).pitch) < 1e-9);

    const int up = first.markerIndex(QStringLiteral("UP"));
    QVERIFY(up >= 0);
    QVERIFY(qAbs(first.markers().at(up).pitch - 90.0) < 1e-9);

    const int rightUp = first.markerIndex(QStringLiteral("RIGHT_UP"));
    QVERIFY(rightUp >= 0);
    QVERIFY(qAbs(first.markers().at(rightUp).yaw - 90.0) < 1e-9);
    QVERIFY(qAbs(first.markers().at(rightUp).pitch - 45.0) < 1e-9);
}

void ProjectTest::viewerSceneMarkerRangesAreValid()
{
    const ViewerScene scene = ViewerScene::createDeterministicTestScene();
    QVERIFY(scene.isValid());

    QStringList seenLabels;
    for (const ViewerSceneMarker &marker : scene.markers()) {
        QVERIFY(!marker.label.isEmpty());
        QVERIFY(marker.yaw >= -180.0 && marker.yaw < 180.0);
        QVERIFY(marker.pitch >= -90.0 && marker.pitch <= 90.0);
        QVERIFY(!seenLabels.contains(marker.label));
        seenLabels.append(marker.label);
    }
}

void ProjectTest::viewerWidgetPresentsDeterministicScene()
{
    ViewerWidget widget;

    QVERIFY(widget.scene().isValid());
    QCOMPARE(widget.scene().markerCount(), 10);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("FRONT")) >= 0);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("BACK")) >= 0);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("LEFT_DOWN")) >= 0);
}

void ProjectTest::mainWindowContainsViewerSurface()
{
    TestMainWindow window;

    auto *viewer = window.findChild<ViewerWidget *>(QStringLiteral("viewerWidget"));
    QVERIFY(viewer);
    QVERIFY(viewer->scene().isValid());
    QCOMPARE(viewer->scene().markerCount(), 10);
}

void ProjectTest::viewerWidgetRenderIsDeterministic()
{
    ViewerWidget widget;
    widget.resize(400, 200);

    const QImage image = widget.grab().toImage();
    QVERIFY(!image.isNull());
    QVERIFY(image.width() > 0);
    QVERIFY(image.height() > 0);

    const double scaleX = static_cast<double>(image.width()) / 400.0;
    const double scaleY = static_cast<double>(image.height()) / 200.0;

    auto expectPixel = [scaleX, scaleY](const QImage &img, double logicalX,
                                        double logicalY, int red, int green, int blue) {
        const QColor color =
            img.pixelColor(qRound(logicalX * scaleX), qRound(logicalY * scaleY));
        QCOMPARE(color.red(), red);
        QCOMPARE(color.green(), green);
        QCOMPARE(color.blue(), blue);
    };

    // Without a viewport state the camera is at identity defaults
    // (yaw/pitch/roll 0, FOV 90), so FRONT is centered.

    // Background samples away from markers, labels, and the center reticle.
    expectPixel(image, 20.0, 180.0, 18, 20, 24);
    expectPixel(image, 380.0, 30.0, 18, 20, 24);

    // FRONT marker (yaw 0, pitch 0) maps to the canvas center.
    expectPixel(image, 200.0, 100.0, 255, 213, 79);

    // Side markers are not visible at the identity camera view, so their
    // former equirectangular positions are plain background now.
    expectPixel(image, 100.0, 100.0, 18, 20, 24);
    expectPixel(image, 300.0, 100.0, 18, 20, 24);
}

void ProjectTest::viewerProjectionCentersFrontMarkerAtIdentity()
{
    double x = 0.0;
    double y = 0.0;
    const bool visible = ViewerProjection::project(
        0.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y);

    QVERIFY(visible);
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionYawAimsAtSideMarkers()
{
    // At identity, side and back markers are not in front of the camera.
    double x = 0.0;
    double y = 0.0;
    QVERIFY(!ViewerProjection::project(90.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(-90.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(180.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));

    // Turning the camera +90 degrees of yaw centers the RIGHT marker.
    QVERIFY(ViewerProjection::project(90.0, 0.0, 90.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);

    // Turning the camera -90 degrees of yaw centers the LEFT marker.
    QVERIFY(ViewerProjection::project(-90.0, 0.0, -90.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionPitchAimsAtPoleMarkers()
{
    double x = 0.0;
    double y = 0.0;

    // At identity the poles are overhead/underfoot, not in front.
    QVERIFY(!ViewerProjection::project(0.0, 90.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(0.0, -90.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));

    // Tilting up +90 centers the UP marker; tilting down -90 centers DOWN.
    QVERIFY(ViewerProjection::project(0.0, 90.0, 0.0, 90.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);

    QVERIFY(ViewerProjection::project(0.0, -90.0, 0.0, -90.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionPitchMirrorSymmetry()
{
    double xAbove = 0.0;
    double yAbove = 0.0;
    double xBelow = 0.0;
    double yBelow = 0.0;

    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &xAbove, &yAbove));
    QVERIFY(ViewerProjection::project(0.0, -20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &xBelow, &yBelow));

    QVERIFY(qAbs(xAbove - 200.0) < 1e-6);
    QVERIFY(qAbs(xBelow - 200.0) < 1e-6);
    QVERIFY(yAbove < 100.0);
    QVERIFY(yBelow > 100.0);
    QVERIFY(qAbs((yAbove + yBelow) / 2.0 - 100.0) < 1e-6);
    QVERIFY(qAbs((100.0 - yAbove) - (yBelow - 100.0)) < 1e-6);
}

void ProjectTest::viewerProjectionRollRotatesViewContent()
{
    // FRONT stays centered for any roll.
    for (double roll : { -90.0, -45.0, 0.0, 45.0, 90.0 }) {
        double x = 0.0;
        double y = 0.0;
        QVERIFY(ViewerProjection::project(0.0, 0.0, 0.0, 0.0, roll, 90.0, 400.0, 200.0, &x, &y));
        QVERIFY(qAbs(x - 200.0) < 1e-6);
        QVERIFY(qAbs(y - 100.0) < 1e-6);
    }

    // A marker above center rotates counter-clockwise by the roll angle:
    // roll +90 moves it to the left of center at the same height.
    double x0 = 0.0;
    double y0 = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x0, &y0));
    const double radius = 100.0 - y0;
    QVERIFY(radius > 0.0);

    double xLeft = 0.0;
    double yLeft = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 90.0, 90.0, 400.0, 200.0, &xLeft, &yLeft));
    QVERIFY(qAbs(xLeft - (200.0 - radius)) < 1e-6);
    QVERIFY(qAbs(yLeft - 100.0) < 1e-6);

    double xRight = 0.0;
    double yRight = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, -90.0, 90.0, 400.0, 200.0, &xRight, &yRight));
    QVERIFY(qAbs(xRight - (200.0 + radius)) < 1e-6);
    QVERIFY(qAbs(yRight - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionLargerFieldOfViewBringsMarkersCloser()
{
    double xNarrow = 0.0;
    double yNarrow = 0.0;
    double xWide = 0.0;
    double yWide = 0.0;

    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 60.0, 400.0, 200.0, &xNarrow, &yNarrow));
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 120.0, 400.0, 200.0, &xWide, &yWide));

    const double distanceNarrow = qAbs(yNarrow - 100.0);
    const double distanceWide = qAbs(yWide - 100.0);
    QVERIFY(distanceNarrow > distanceWide);
    QVERIFY(distanceWide > 0.0);
}

void ProjectTest::viewerWidgetCameraFollowsApplicationViewportState()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);
    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);

    // Same wiring as main.cpp: the presentation follows the authoritative
    // application-owned viewport state.
    viewer->setViewportState(state);
    viewer->resize(400, 200);

    auto centerColor = [viewer]() {
        const QImage image = viewer->grab().toImage();
        return image.pixelColor(image.width() / 2, image.height() / 2);
    };
    auto expectCenter = [&centerColor](int red, int green, int blue) {
        const QColor color = centerColor();
        QCOMPARE(color.red(), red);
        QCOMPARE(color.green(), green);
        QCOMPARE(color.blue(), blue);
    };

    // Identity defaults: FRONT (amber) centered.
    expectCenter(255, 213, 79);

    // Application yaw adjustment +90 -> camera faces RIGHT (pink) marker.
    app.adjustViewportYaw(90.0);
    expectCenter(240, 98, 146);

    // Reset restores FRONT to center.
    app.resetViewport();
    expectCenter(255, 213, 79);
}

void ProjectTest::mediaItemRecordsMetadataFromRealFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clip_x5.360.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    const QByteArray bytes(4096, 'A');
    QCOMPARE(mediaFile.write(bytes), static_cast<qint64>(bytes.size()));
    mediaFile.close();

    QString error;
    const MediaItem item = MediaItem::createFromFilePath(mediaPath, &error);
    QVERIFY(item.isValid());
    QVERIFY(error.isEmpty());

    const QString canonical = QFileInfo(mediaPath).canonicalFilePath();
    QVERIFY(!canonical.isEmpty());
    QCOMPARE(item.path(), canonical);
    QCOMPARE(item.fileName(), QStringLiteral("clip_x5.360.mp4"));
    QCOMPARE(item.formatTag(), QStringLiteral("mp4"));
    QCOMPARE(item.sizeBytes(), static_cast<qint64>(bytes.size()));
    QVERIFY(item.lastModifiedUtc().isValid());
    QVERIFY(!item.id().isEmpty());
    QVERIFY(item.referenceExists());
}

void ProjectTest::mediaItemRejectsInvalidPaths()
{
    QString error;

    // Empty path.
    QVERIFY(!MediaItem::createFromFilePath(QString(), &error).isValid());
    QVERIFY(!error.isEmpty());

    // Missing file.
    const MediaItem missing =
        MediaItem::createFromFilePath(QStringLiteral("/nonexistent/nope.mp4"), &error);
    QVERIFY(!missing.isValid());
    QVERIFY(error.contains(QStringLiteral("does not exist")));

    // Directory.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const MediaItem directory = MediaItem::createFromFilePath(tempDir.path(), &error);
    QVERIFY(!directory.isValid());
    QVERIFY(error.contains(QStringLiteral("directory")));
}

void ProjectTest::mediaItemIdIsDeterministic()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("MEDIA");
        file.close();
    }

    const MediaItem first = MediaItem::createFromFilePath(firstPath);
    const MediaItem firstAgain = MediaItem::createFromFilePath(firstPath);
    const MediaItem second = MediaItem::createFromFilePath(secondPath);

    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QCOMPARE(first.id(), firstAgain.id());
    QVERIFY(first.id() != second.id());
}

void ProjectTest::mediaItemJsonRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("roundtrip.mov"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(128, 'B'));
    mediaFile.close();

    MediaItem original = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(original.isValid());
    QJsonObject attributes;
    attributes.insert(QStringLiteral("note"), QStringLiteral("future metadata home"));
    original.setAttributes(attributes);

    const QJsonObject object = original.toJsonObject();

    MediaItem restored;
    QString error;
    QVERIFY(restored.readFromJsonObject(object, &error));
    QVERIFY(error.isEmpty());
    QVERIFY(restored.isValid());

    QCOMPARE(restored.id(), original.id());
    QCOMPARE(restored.path(), original.path());
    QCOMPARE(restored.fileName(), original.fileName());
    QCOMPARE(restored.formatTag(), original.formatTag());
    QCOMPARE(restored.sizeBytes(), original.sizeBytes());
    QCOMPARE(restored.lastModifiedUtc().toMSecsSinceEpoch(),
             original.lastModifiedUtc().toMSecsSinceEpoch());
    QCOMPARE(restored.attributes().value(QStringLiteral("note")).toString(),
             QStringLiteral("future metadata home"));
}

void ProjectTest::mediaItemRejectsInvalidJson()
{
    MediaItem item;
    QString error;

    QJsonObject empty;
    QVERIFY(!item.readFromJsonObject(empty, &error));
    QVERIFY(!error.isEmpty());

    QJsonObject partial;
    partial.insert(QStringLiteral("id"), QStringLiteral("abc"));
    QVERIFY(!item.readFromJsonObject(partial, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationImportMediaRequiresActiveProject()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write("DATA");
    mediaFile.close();

    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.importMediaFile(mediaPath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("No project")));

    app.newProject();
    spy.clear();
    QVERIFY(app.importMediaFile(mediaPath));
    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Imported media")));
}

void ProjectTest::applicationImportMediaDeduplicatesAndOrders()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(16, 'x'));
        file.close();
    }

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    // Re-importing the same canonical path is idempotent.
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(app.mediaItems().size(), 2);
    QVERIFY(spy.last().first().toString().contains(QStringLiteral("already imported")));

    // Import order is preserved deterministically.
    QCOMPARE(app.mediaItems().at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(app.mediaItems().at(1).fileName(), QStringLiteral("second.mov"));
}

void ProjectTest::applicationImportMediaRejectsInvalidPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.importMediaFile(QStringLiteral("/nonexistent/nope.mp4")));
    QCOMPARE(app.mediaItems().size(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Import failed")));

    spy.clear();
    QVERIFY(!app.importMediaFile(tempDir.path()));
    QCOMPARE(app.mediaItems().size(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Import failed")));
}

void ProjectTest::mediaReferencesPersistAndReopenDeterministically()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(path.endsWith(QStringLiteral("mp4")) ? QByteArray(32, '1') : QByteArray(48, '2'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("media_project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));

    const QList<MediaItem> items = reopened.mediaItems();
    QCOMPARE(items.size(), 2);

    const QList<MediaItem> originalItems = app.mediaItems();
    QCOMPARE(items.at(0).id(), originalItems.at(0).id());
    QCOMPARE(items.at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(items.at(0).sizeBytes(), originalItems.at(0).sizeBytes());
    QCOMPARE(items.at(0).formatTag(), QStringLiteral("mp4"));
    QCOMPARE(items.at(1).id(), originalItems.at(1).id());
    QCOMPARE(items.at(1).fileName(), QStringLiteral("second.mov"));
    QCOMPARE(items.at(1).formatTag(), QStringLiteral("mov"));
}

void ProjectTest::originalMediaUnchangedByImportAndLifecycle()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("source.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    const QByteArray originalBytes("REAL_MEDIA_BYTES_0123456789_REELCRAFT_OBJ4");
    QCOMPARE(mediaFile.write(originalBytes), static_cast<qint64>(originalBytes.size()));
    mediaFile.close();

    const QByteArray originalHash =
        QCryptographicHash::hash(originalBytes, QCryptographicHash::Sha256);

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);

    QFile after(mediaPath);
    QVERIFY(after.open(QIODevice::ReadOnly));
    const QByteArray afterBytes = after.readAll();
    after.close();

    QCOMPARE(afterBytes, originalBytes);
    QCOMPARE(QCryptographicHash::hash(afterBytes, QCryptographicHash::Sha256), originalHash);
}

void ProjectTest::legacyProjectOpensWithEmptyMedia()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // A project file without any media section (legacy style).
    QJsonObject legacy;
    legacy.insert(QStringLiteral("id"), QStringLiteral("legacy-media-id"));
    legacy.insert(QStringLiteral("name"), QStringLiteral("Legacy No Media"));
    legacy.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    const QString projectPath = tempDir.filePath(QStringLiteral("legacy.reel"));
    QFile file(projectPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(legacy).toJson(QJsonDocument::Compact));
    file.close();

    Application app;
    QVERIFY(app.openProject(projectPath));
    QVERIFY(app.mediaItems().isEmpty());
    QVERIFY(app.currentProject().media().isEmpty());
}

void ProjectTest::invalidPersistedMediaFallsBackSafely()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto writeProject = [&tempDir](const QString &name, const QJsonValue &mediaValue) {
        QJsonObject project;
        project.insert(QStringLiteral("id"), QStringLiteral("id-") + name);
        project.insert(QStringLiteral("name"), QStringLiteral("Project ") + name);
        project.insert(QStringLiteral("created"),
                       QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
        project.insert(QStringLiteral("media"), mediaValue);

        const QString projectPath = tempDir.filePath(name + QStringLiteral(".reel"));
        QFile file(projectPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return QString();
        }
        file.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
        file.close();
        return projectPath;
    };

    // media present but not an array.
    const QString wrongTypePath = writeProject(QStringLiteral("wrongtype"), QJsonValue(QStringLiteral("nope")));
    QVERIFY(!wrongTypePath.isEmpty());
    Application appWrongType;
    QVERIFY(appWrongType.openProject(wrongTypePath));
    QVERIFY(appWrongType.mediaItems().isEmpty());

    // media array containing an invalid record.
    QJsonArray badArray;
    QJsonObject badRecord;
    badRecord.insert(QStringLiteral("id"), QStringLiteral("incomplete"));
    badArray.append(badRecord);
    const QString badRecordPath = writeProject(QStringLiteral("badrecord"), badArray);
    QVERIFY(!badRecordPath.isEmpty());
    Application appBadRecord;
    QVERIFY(appBadRecord.openProject(badRecordPath));
    QVERIFY(appBadRecord.mediaItems().isEmpty());
}

void ProjectTest::mainWindowImportButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::importMediaRequested);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("importMediaButton"));
    QVERIFY(button);

    button->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("/tmp/reelcraft_media_test.bin"));
}

void ProjectTest::reopenWithAvailableMediaIsSilentAndIdentical()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("available.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(64, 'v'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::backgroundCompleted);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).id(), app.mediaItems().at(0).id());
    QVERIFY(!reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 0);
    QCOMPARE(spy.count(), 0);
}

void ProjectTest::reopenAfterDeletingMediaFileFlagsUnavailable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("delete_me.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(32, 'd'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    // The referenced media disappears before reopen.
    QVERIFY(QFile::remove(mediaPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::backgroundCompleted);
    QVERIFY(reopened.openProject(projectPath));

    // The record is retained but flagged unavailable.
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).fileName(), QStringLiteral("delete_me.mp4"));
    QVERIFY(reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 1);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("unavailable")));
}

void ProjectTest::reopenAfterMovingMediaFileFlagsUnavailable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString originalPath = tempDir.filePath(QStringLiteral("moved.mp4"));
    QFile mediaFile(originalPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(24, 'm'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(originalPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    // The file moves to a new path before reopen; the recorded reference
    // (path-based) no longer resolves.
    const QString movedPath = tempDir.filePath(QStringLiteral("moved_away.mp4"));
    QVERIFY(QFile::rename(originalPath, movedPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QVERIFY(reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 1);
}

void ProjectTest::duplicatePersistedMediaNormalizedOnOpenAndResave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("dup.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'p'));
    mediaFile.close();

    // A project file that lists the same media record twice.
    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());
    QJsonArray duplicatedMedia;
    duplicatedMedia.append(item.toJsonObject());
    duplicatedMedia.append(item.toJsonObject());

    QJsonObject project;
    project.insert(QStringLiteral("id"), QStringLiteral("dup-project"));
    project.insert(QStringLiteral("name"), QStringLiteral("Duplicate Media"));
    project.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
    project.insert(QStringLiteral("media"), duplicatedMedia);

    const QString projectPath = tempDir.filePath(QStringLiteral("dups.reel"));
    QFile projectFile(projectPath);
    QVERIFY(projectFile.open(QIODevice::WriteOnly | QIODevice::Text));
    projectFile.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
    projectFile.close();

    Application app;
    QVERIFY(app.openProject(projectPath));
    QCOMPARE(app.mediaItems().size(), 1);

    // Re-saving normalizes the persisted section deterministically.
    const QString resavedPath = tempDir.filePath(QStringLiteral("dups_resaved.reel"));
    QVERIFY(app.saveProject(resavedPath));

    Application reopened;
    QVERIFY(reopened.openProject(resavedPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).id(), item.id());
    QVERIFY(!reopened.hasUnavailableMedia());
}

void ProjectTest::newProjectClearsUnavailableMediaState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("gone.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 'g'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    QVERIFY(QFile::remove(mediaPath));

    Application appOpen;
    QVERIFY(appOpen.openProject(projectPath));
    QVERIFY(appOpen.hasUnavailableMedia());

    appOpen.newProject();
    QVERIFY(!appOpen.hasUnavailableMedia());
    QCOMPARE(appOpen.unavailableMediaCount(), 0);
    QVERIFY(appOpen.mediaItems().isEmpty());
}

void ProjectTest::mediaImportEmitsMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto createFile = [&tempDir](const QString &name) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(QByteArray(16, 'i'));
        file.close();
        return path;
    };

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::mediaListChanged);

    const QString firstPath = createFile(QStringLiteral("first.bin"));
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<QList<MediaItem>>().size(), 1);
    QCOMPARE(spy.first().first().value<QList<MediaItem>>().at(0).id(),
             app.mediaItems().at(0).id());

    // Duplicate import does not change the list, so no emission.
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(spy.count(), 1);

    // A second distinct import emits again with both records.
    const QString secondPath = createFile(QStringLiteral("second.mov"));
    QVERIFY(app.importMediaFile(secondPath));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().first().value<QList<MediaItem>>().size(), 2);
}

void ProjectTest::openProjectRestoresAndEmitsMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(32, 'o'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::mediaListChanged);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(spy.count(), 1);
    const QList<MediaItem> restored = spy.first().first().value<QList<MediaItem>>();
    QCOMPARE(restored.size(), 2);
    QCOMPARE(restored.at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(restored.at(1).fileName(), QStringLiteral("second.mov"));
    QCOMPARE(restored.at(0).id(), app.mediaItems().at(0).id());
}

void ProjectTest::newProjectEmitsEmptyMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clear.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 'c'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    QCOMPARE(app.mediaItems().size(), 1);

    QSignalSpy spy(&app, &Application::mediaListChanged);
    app.newProject();

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().value<QList<MediaItem>>().isEmpty());
    QVERIFY(app.mediaItems().isEmpty());
}

void ProjectTest::removeMediaRemovesMatchingRecordOnlyAndPersists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("keep.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("remove_me.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(24, 'r'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));
    QCOMPARE(app.mediaItems().size(), 2);

    const QString removedId = app.mediaItems().at(1).id();
    const QString keptName = app.mediaItems().at(0).fileName();

    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.removeMedia(removedId));

    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(app.mediaItems().at(0).fileName(), keptName);
    QCOMPARE(listSpy.count(), 1);
    QCOMPARE(listSpy.first().first().value<QList<MediaItem>>().size(), 1);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Removed media")));

    // Persistence reflects the removal.
    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).fileName(), keptName);
}

void ProjectTest::removeMediaUnknownIdFailsDeterministically()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("stays.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 's'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.removeMedia(QStringLiteral("no-such-id")));

    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(listSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Remove failed")));
}

void ProjectTest::removeMediaRequiresActiveProject()
{
    Application app;
    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.removeMedia(QStringLiteral("any-id")));
    QCOMPARE(listSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No project")));
}

void ProjectTest::removeUnavailableMediaClearsUnavailableState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("gone_soon.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'g'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    QVERIFY(QFile::remove(mediaPath));

    Application appOpen;
    QVERIFY(appOpen.openProject(projectPath));
    QVERIFY(appOpen.hasUnavailableMedia());
    QCOMPARE(appOpen.unavailableMediaCount(), 1);

    const QString mediaId = appOpen.mediaItems().at(0).id();
    QVERIFY(appOpen.removeMedia(mediaId));

    QVERIFY(appOpen.mediaItems().isEmpty());
    QVERIFY(!appOpen.hasUnavailableMedia());
    QCOMPARE(appOpen.unavailableMediaCount(), 0);
}

void ProjectTest::mainWindowMediaListPopulatedAndRemoveWorks()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&window, &MainWindow::removeMediaRequested,
                     &app, &Application::removeMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    QCOMPARE(list->count(), 1);
    QVERIFY(list->item(0)->text().contains(QStringLiteral("media.bin")));

    auto *removeButton = window.findChild<QPushButton *>(QStringLiteral("removeMediaButton"));
    QVERIFY(removeButton);

    // No selection: clicking Remove is a no-op with status feedback.
    QSignalSpy spy(&window, &MainWindow::removeMediaRequested);
    removeButton->click();
    QCOMPARE(spy.count(), 0);
    auto *statusLabel = window.findChild<QLabel *>(QStringLiteral("statusLabel"));
    QVERIFY(statusLabel);
    QVERIFY(statusLabel->text().contains(QStringLiteral("No media selected")));

    // Selecting the entry and removing emits the request with its media id.
    const QString mediaId = app.mediaItems().at(0).id();
    list->setCurrentRow(0);
    removeButton->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), mediaId);

    QVERIFY(app.mediaItems().isEmpty());
    QCOMPARE(list->count(), 0);
}

void ProjectTest::activeMediaRequiresProjectAndSetsState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("act.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(12, 'a'));
    mediaFile.close();

    Application app;
    QSignalSpy spy(&app, &Application::activeMediaChanged);

    // No active project: selection fails safely.
    QVERIFY(!app.setActiveMedia(QStringLiteral("any")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();

    QVERIFY(app.setActiveMedia(id));
    QCOMPARE(app.activeMediaId(), id);
    QVERIFY(app.activeMediaItem() != nullptr);
    QCOMPARE(app.activeMediaItem()->id(), id);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), id);
}

void ProjectTest::activeMediaSameIdAndUnknownIdBehavior()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("same.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 's'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    // Same id again: no change, no activeMediaChanged emission.
    QSignalSpy spy(&app, &Application::activeMediaChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.setActiveMedia(id));
    QCOMPARE(spy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("already active")));
    QCOMPARE(app.activeMediaId(), id);

    // Unknown id (including empty): fails, state unchanged, no emission.
    QVERIFY(!app.setActiveMedia(QStringLiteral("no-such-id")));
    QVERIFY(!app.setActiveMedia(QString()));
    QCOMPARE(app.activeMediaId(), id);
    QCOMPARE(spy.count(), 0);
}

void ProjectTest::activeMediaImportNeverAutoSelects()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto createFile = [&tempDir](const QString &name) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(QByteArray(8, 'n'));
        file.close();
        return path;
    };

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("a.bin"))));
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("b.bin"))));

    // Imports never auto-select.
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);

    // Selecting then importing another file leaves the selection unchanged.
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("c.mov"))));
    QCOMPARE(app.activeMediaId(), id);
    QCOMPARE(app.mediaItems().size(), 3);
}

void ProjectTest::activeMediaRemovalRules()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(8, 'r'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString activeId = app.mediaItems().at(0).id();
    const QString otherId = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(activeId));

    QSignalSpy spy(&app, &Application::activeMediaChanged);

    // Removing a non-active record preserves the active id (no emission).
    QVERIFY(app.removeMedia(otherId));
    QCOMPARE(app.activeMediaId(), activeId);
    QCOMPARE(spy.count(), 0);

    // Removing the active record clears it with one emission.
    QVERIFY(app.removeMedia(activeId));
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().isEmpty());
}

void ProjectTest::activeMediaClearedOnNewProject()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clear.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(6, 'c'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    QSignalSpy spy(&app, &Application::activeMediaChanged);
    app.newProject();

    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().isEmpty());
}

void ProjectTest::activeMediaRoundTripRestoresOnReopen()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(16, 'p'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString activeId = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(activeId));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::activeMediaChanged);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(reopened.activeMediaId(), activeId);
    QVERIFY(reopened.activeMediaItem() != nullptr);
    QCOMPARE(reopened.activeMediaItem()->id(), activeId);
    QCOMPARE(reopened.activeMediaItem()->fileName(), QStringLiteral("second.mov"));
}

void ProjectTest::activeMediaOpenClearsDanglingOrLegacy()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();
    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());

    auto writeProject = [&tempDir](const QString &name, const QJsonArray &media,
                                   const QJsonValue &activeId) {
        QJsonObject project;
        project.insert(QStringLiteral("id"), QStringLiteral("id-") + name);
        project.insert(QStringLiteral("name"), QStringLiteral("Project ") + name);
        project.insert(QStringLiteral("created"),
                       QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
        project.insert(QStringLiteral("media"), media);
        if (activeId.isString()) {
            project.insert(QStringLiteral("activeMediaId"), activeId);
        }
        const QString projectPath = tempDir.filePath(name + QStringLiteral(".reel"));
        QFile file(projectPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return QString();
        }
        file.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
        file.close();
        return projectPath;
    };

    QJsonArray single;
    single.append(item.toJsonObject());

    // Dangling persisted active id: cleared deterministically.
    const QString danglingPath =
        writeProject(QStringLiteral("dangling"), single, QJsonValue(QStringLiteral("no-such-id")));
    QVERIFY(!danglingPath.isEmpty());
    Application appDangling;
    QVERIFY(appDangling.openProject(danglingPath));
    QCOMPARE(appDangling.mediaItems().size(), 1);
    QVERIFY(appDangling.activeMediaId().isEmpty());

    // Legacy project without the key: no active media.
    const QString legacyPath =
        writeProject(QStringLiteral("legacy"), single, QJsonValue());
    QVERIFY(!legacyPath.isEmpty());
    Application appLegacy;
    QVERIFY(appLegacy.openProject(legacyPath));
    QVERIFY(appLegacy.activeMediaId().isEmpty());

    // Duplicate media entries + valid active id: normalization yields one
    // record and the active id resolves to it.
    QJsonArray duplicated;
    duplicated.append(item.toJsonObject());
    duplicated.append(item.toJsonObject());
    const QString dupPath =
        writeProject(QStringLiteral("dups"), duplicated, QJsonValue(item.id()));
    QVERIFY(!dupPath.isEmpty());
    Application appDup;
    QSignalSpy spy(&appDup, &Application::activeMediaChanged);
    QVERIFY(appDup.openProject(dupPath));
    QCOMPARE(appDup.mediaItems().size(), 1);
    QCOMPARE(appDup.activeMediaId(), item.id());
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::activeMediaUnavailableCannotBeActive()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("vanishes.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'v'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    QVERIFY(QFile::remove(mediaPath));

    // Reopen: the record is retained but unavailable; the persisted active id
    // must be cleared (never a dangling/unavailable active selection).
    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QVERIFY(reopened.hasUnavailableMedia());
    QVERIFY(reopened.activeMediaId().isEmpty());
    QVERIFY(reopened.activeMediaItem() == nullptr);

    // Selecting an unavailable record is rejected.
    QSignalSpy messageSpy(&reopened, &Application::backgroundCompleted);
    QVERIFY(!reopened.setActiveMedia(id));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("unavailable")));
    QVERIFY(reopened.activeMediaId().isEmpty());
}

void ProjectTest::mainWindowSetActiveAndLabelWork()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&window, &MainWindow::setActiveRequested,
                     &app, &Application::setActiveMedia);
    QObject::connect(&window, &MainWindow::removeMediaRequested,
                     &app, &Application::removeMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    auto *setActiveButton = window.findChild<QPushButton *>(QStringLiteral("setActiveButton"));
    QVERIFY(setActiveButton);
    auto *activeLabel = window.findChild<QLabel *>(QStringLiteral("activeMediaLabel"));
    QVERIFY(activeLabel);
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: None"));

    // No selection: clicking Set Active is a no-op with status feedback.
    QSignalSpy spy(&window, &MainWindow::setActiveRequested);
    setActiveButton->click();
    QCOMPARE(spy.count(), 0);
    auto *statusLabel = window.findChild<QLabel *>(QStringLiteral("statusLabel"));
    QVERIFY(statusLabel);
    QVERIFY(statusLabel->text().contains(QStringLiteral("No media selected to set active")));

    // Selecting the row and setting active updates id, label, and row marking.
    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    const QString mediaId = app.mediaItems().at(0).id();
    list->setCurrentRow(0);
    setActiveButton->click();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), mediaId);
    QCOMPARE(app.activeMediaId(), mediaId);
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: media.bin"));
    QVERIFY(list->item(0)->text().startsWith(QStringLiteral("▶ ")));

    // Removing the active media clears the label back to None.
    auto *removeButton = window.findChild<QPushButton *>(QStringLiteral("removeMediaButton"));
    QVERIFY(removeButton);
    list->setCurrentRow(0);
    removeButton->click();
    QVERIFY(app.activeMediaId().isEmpty());
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: None"));
    QCOMPARE(list->count(), 0);
}

namespace {

void expectColor(const QImage &image, int x, int y, const QColor &expected)
{
    const QColor color = image.pixelColor(x, y);
    QCOMPARE(color.red(), expected.red());
    QCOMPARE(color.green(), expected.green());
    QCOMPARE(color.blue(), expected.blue());
}

double nearestPatchDistance(const QImage &image, const QColor &color,
                            int centerX, int centerY)
{
    double best = -1.0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != color) {
                continue;
            }
            const double dx = x - centerX;
            const double dy = y - centerY;
            const double distance = std::sqrt(dx * dx + dy * dy);
            if (best < 0.0 || distance < best) {
                best = distance;
            }
        }
    }
    return best;
}

} // namespace

void ProjectTest::equirectViewIdentityCentersFront()
{
    const QImage pattern = buildTestPattern();
    QImage view;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, &view));
    QCOMPARE(view.width(), 200);
    QCOMPARE(view.height(), 100);

    expectColor(view, 100, 50, kFrontColor);
    expectColor(view, 2, 2, kPatternBackground);
}

void ProjectTest::equirectViewYawCentersRightAndLeft()
{
    const QImage pattern = buildTestPattern();

    QImage viewRight;
    QVERIFY(EquirectView::render(pattern, 90.0, 0.0, 0.0, 90.0, 200, 100, &viewRight));
    expectColor(viewRight, 100, 50, kRightColor);

    QImage viewLeft;
    QVERIFY(EquirectView::render(pattern, -90.0, 0.0, 0.0, 90.0, 200, 100, &viewLeft));
    expectColor(viewLeft, 100, 50, kLeftColor);
}

void ProjectTest::equirectViewPitchReachesUpAndDown()
{
    const QImage pattern = buildTestPattern();

    QImage viewUp;
    QVERIFY(EquirectView::render(pattern, 0.0, 90.0, 0.0, 90.0, 200, 100, &viewUp));
    expectColor(viewUp, 100, 50, kUpColor);

    QImage viewDown;
    QVERIFY(EquirectView::render(pattern, 0.0, -90.0, 0.0, 90.0, 200, 100, &viewDown));
    expectColor(viewDown, 100, 50, kDownColor);
}

void ProjectTest::equirectViewPositiveRollRotatesContentCorrectly()
{
    const QImage pattern = buildTestPattern();

    // A stripe above FRONT (world yaw 0, pitch 20). At zero roll it must sit
    // above center; positive roll must move it to the left, negative roll to
    // the right (matching the ViewerProjection convention that positive roll
    // rotates projected content counter-clockwise on screen).
    QImage viewZero;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, &viewZero));
    const QuadrantCounts zeroCounts = countQuadrants(viewZero, kUp20Color);
    QVERIFY(zeroCounts.above > 0);
    QVERIFY(zeroCounts.above > zeroCounts.below);

    QImage viewRolled;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 90.0, 90.0, 200, 100, &viewRolled));
    const QuadrantCounts rolledCounts = countQuadrants(viewRolled, kUp20Color);
    QVERIFY(rolledCounts.left > 0);
    QVERIFY(rolledCounts.left > rolledCounts.right);
    QVERIFY(rolledCounts.left > rolledCounts.above);

    QImage viewRolledBack;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, -90.0, 90.0, 200, 100, &viewRolledBack));
    const QuadrantCounts backCounts = countQuadrants(viewRolledBack, kUp20Color);
    QVERIFY(backCounts.right > 0);
    QVERIFY(backCounts.right > backCounts.left);
    QVERIFY(backCounts.right > backCounts.above);
}

void ProjectTest::equirectViewFieldOfViewChangesCoverage()
{
    const QImage pattern = buildTestPattern();

    QImage viewNarrow;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 60.0, 200, 100, &viewNarrow));
    QImage viewWide;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 120.0, 200, 100, &viewWide));

    const double distanceNarrow = nearestPatchDistance(viewNarrow, kUp20Color, 100, 50);
    const double distanceWide = nearestPatchDistance(viewWide, kUp20Color, 100, 50);
    QVERIFY(distanceNarrow > 0.0);
    QVERIFY(distanceWide > 0.0);
    // A larger vertical FOV brings the same direction closer to center.
    QVERIFY(distanceNarrow > distanceWide);
}

void ProjectTest::equirectViewRejectsInvalidInput()
{
    const QImage pattern = buildTestPattern();
    QImage out(4, 4, QImage::Format_ARGB32);
    out.fill(QColor(255, 0, 0));

    const QColor sentinel(255, 0, 0);
    auto expectUnchanged = [&out, &sentinel]() {
        expectColor(out, 0, 0, sentinel);
        expectColor(out, 3, 3, sentinel);
    };

    // Empty source.
    QVERIFY(!EquirectView::render(QImage(), 0.0, 0.0, 0.0, 90.0, 200, 100, &out));
    expectUnchanged();

    // Invalid output dimensions.
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 0, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, -5, &out));
    expectUnchanged();

    // Invalid FOV (outside ViewportState clamp semantics [20, 140]).
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 10.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 150.0, 200, 100, &out));
    expectUnchanged();

    // Invalid camera values.
    QVERIFY(!EquirectView::render(pattern, 0.0, 91.0, 0.0, 90.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, -91.0, 0.0, 90.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, qQNaN(), 0.0, 0.0, 90.0, 200, 100, &out));
    expectUnchanged();

    // Null output pointer.
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, nullptr));
}

void ProjectTest::equirectViewDeterministicRepeatability()
{
    const QImage pattern = buildTestPattern();

    QImage first;
    QImage second;
    QVERIFY(EquirectView::render(pattern, 30.0, 15.0, 45.0, 95.0, 200, 100, &first));
    QVERIFY(EquirectView::render(pattern, 30.0, 15.0, 45.0, 95.0, 200, 100, &second));

    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::viewerWidgetSourceImageRendersThroughCamera()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(200, 100);

    widget.setSourceImage(buildTestPattern());
    QVERIFY(widget.hasSourceImage());

    QImage view = widget.grab().toImage();
    expectColor(view, 100, 50, kFrontColor);

    // The presentation follows the authoritative ViewportState: +90 yaw
    // centers the RIGHT region.
    state.setYaw(90.0);
    view = widget.grab().toImage();
    expectColor(view, 100, 50, kRightColor);
}

void ProjectTest::viewerWidgetClearingSourceRestoresSceneRendering()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(200, 100);

    widget.setSourceImage(buildTestPattern());
    QVERIFY(widget.hasSourceImage());
    state.setYaw(0.0);

    // Clearing the source returns to the deterministic marker scene.
    widget.setSourceImage(QImage());
    QVERIFY(!widget.hasSourceImage());

    QImage view = widget.grab().toImage();
    expectColor(view, 100, 50, QColor(255, 213, 79)); // FRONT marker dot
    expectColor(view, 10, 10, QColor(18, 20, 24));    // scene background
}

void ProjectTest::equirectViewPerformanceSanity()
{
    const QImage pattern = buildTestPattern();

    // Warm-up at the capped resolution.
    QImage warm;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0,
                                  EquirectView::MaxOutputWidth, 320, &warm));

    constexpr int kIterations = 5;
    QElapsedTimer timer;
    timer.start();
    QImage view;
    for (int i = 0; i < kIterations; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0,
                                     EquirectView::MaxOutputWidth, 320, &view));
    }
    const double cappedMs = static_cast<double>(timer.nsecsElapsed()) / kIterations / 1e6;
    qInfo("EquirectView bilinear CPU render: %.2f ms/frame at %dx%d (max width %d)",
          cappedMs, EquirectView::MaxOutputWidth, 320, EquirectView::MaxOutputWidth);

    // Informational only (no gate): quantifies the cost of a future cap raise.
    timer.restart();
    for (int i = 0; i < kIterations; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0, 1280, 640, &view));
    }
    const double wideMs = static_cast<double>(timer.nsecsElapsed()) / kIterations / 1e6;
    qInfo("EquirectView bilinear CPU render (informational): %.2f ms/frame at 1280x640",
          wideMs);

    // Generous sanity bound; this is a CPU-cost measurement, not a
    // performance regression gate.
    QVERIFY(cappedMs < 2000.0);
}

void ProjectTest::frameExtractorRejectsInvalidInput()
{
    // Deterministic failures that do not require an ffmpeg executable.
    QString error;
    QImage out;

    // Missing media file (checked before process start).
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QStringLiteral("/no/ffmpeg"), &out, &error));
    QVERIFY(error.contains(QStringLiteral("does not exist")));

    // Empty executable path is reported before file checks.
    error.clear();
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QString(), &out, &error));
    QVERIFY(error.contains(QStringLiteral("ffmpeg not found")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QStringLiteral("/no/ffmpeg"), nullptr, &error));
    QVERIFY(error.contains(QStringLiteral("null output")));
}

void ProjectTest::frameExtractorAvailabilityAndSingleFrameDecode()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QColor fill(0, 0, 255);
    QImage source(64, 32, QImage::Format_RGB32);
    source.fill(fill);
    const QString mediaPath = tempDir.filePath(QStringLiteral("frame.png"));
    QVERIFY(source.save(mediaPath, "PNG"));

    QImage decoded;
    QString error;
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &decoded, &error));
    QVERIFY(error.isEmpty());
    QVERIFY(!decoded.isNull());
    QCOMPARE(decoded.width(), 64);
    QCOMPARE(decoded.height(), 32);
    expectColor(decoded, 0, 0, fill);
    expectColor(decoded, 63, 31, fill);
}

void ProjectTest::frameExtractorDeterministicRepeatability()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QImage source(48, 24, QImage::Format_RGB32);
    source.fill(QColor(0, 200, 200));
    const QString mediaPath = tempDir.filePath(QStringLiteral("repeat.png"));
    QVERIFY(source.save(mediaPath, "PNG"));

    QImage first;
    QImage second;
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &first));
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &second));
    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::applicationPreviewRequiresProjectAndActive()
{
    Application app;
    QSignalSpy spy(&app, &Application::framePreviewReady);

    // No project.
    QVERIFY(!app.previewActiveMediaFrame());
    QCOMPARE(spy.count(), 0);

    // Project but no active media.
    app.newProject();
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.previewActiveMediaFrame());
    QCOMPARE(spy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No active media")));
}

void ProjectTest::applicationPreviewEmitsFramePreview()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("preview.png"));
    QImage source(80, 40, QImage::Format_RGB32);
    source.fill(QColor(255, 0, 0));
    QVERIFY(source.save(mediaPath, "PNG"));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy spy(&app, &Application::framePreviewReady);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.previewActiveMediaFrame());

    QCOMPARE(spy.count(), 1);
    const QImage frame = spy.first().first().value<QImage>();
    QVERIFY(!frame.isNull());
    QCOMPARE(frame.width(), 80);
    QCOMPARE(frame.height(), 40);
    expectColor(frame, 40, 20, QColor(255, 0, 0));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Frame extracted")));
}

void ProjectTest::mainWindowPreviewButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::previewFrameRequested);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("previewFrameButton"));
    QVERIFY(button);
    button->click();
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::previewEndToEndShowsActiveFrameInViewer()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // The deterministic equirectangular test pattern saved as a real PNG media
    // file: identity camera must center the FRONT region after decode.
    const QString mediaPath = tempDir.filePath(QStringLiteral("pattern.png"));
    QVERIFY(buildTestPattern().save(mediaPath, "PNG"));

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::framePreviewReady,
                     &window, &MainWindow::showFramePreview);
    QObject::connect(&window, &MainWindow::previewFrameRequested,
                     &app, &Application::previewActiveMediaFrame);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    viewer->resize(200, 100);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("previewFrameButton"));
    QVERIFY(button);

    QSignalSpy spy(&app, &Application::framePreviewReady);
    button->click();

    // Localize: inspect the decoded frame before the viewer mapping.
    QCOMPARE(spy.count(), 1);
    const QImage decoded = spy.first().first().value<QImage>();
    QVERIFY(!decoded.isNull());
    expectColor(decoded, decoded.width() / 2, decoded.height() / 2, kFrontColor);

    QVERIFY(viewer->hasSourceImage());
    const QImage view = viewer->grab().toImage();
    // Identity camera centers FRONT; sample the true image center (the widget
    // sits inside a layout, so its actual geometry may exceed the requested
    // resize).
    expectColor(view, view.width() / 2, view.height() / 2, kFrontColor);
}

void ProjectTest::frameExtractorRejectsInvalidSeekTime()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("frame.png"));
    QImage source(16, 16, QImage::Format_RGB32);
    source.fill(QColor(255, 255, 255));
    QVERIFY(source.save(mediaPath, "PNG"));

    QString error;
    QImage out;
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QStringLiteral("/no/ffmpeg"), -5.0, &out, &error));
    QVERIFY(error.contains(QStringLiteral("invalid seek time")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QStringLiteral("/no/ffmpeg"), qQNaN(), &out, &error));
    QVERIFY(error.contains(QStringLiteral("invalid seek time")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QString(), 0.0, &out, &error));
    QVERIFY(error.contains(QStringLiteral("ffmpeg not found")));
}

void ProjectTest::frameExtractorExtractsFrameAtSeekTime()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    QImage atZero;
    QImage atTwo;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 0.0, &atZero));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 2.0, &atTwo));

    QVERIFY(!atZero.isNull());
    QVERIFY(!atTwo.isNull());
    QVERIFY(redDominant(atZero.pixelColor(atZero.width() / 2, atZero.height() / 2)));
    QVERIFY(blueDominant(atTwo.pixelColor(atTwo.width() / 2, atTwo.height() / 2)));
    QVERIFY(!imagesIdentical(atZero, atTwo));
}

void ProjectTest::frameExtractorSeekDeterministicRepeatability()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    QImage first;
    QImage second;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.5, &first));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.5, &second));
    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::applicationTimeNavigationGuards()
{
    Application app;
    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);

    // No project.
    QVERIFY(!app.previewActiveMediaFrameAt(1.0));
    QVERIFY(!app.stepActiveMediaPreview(1.0));
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);

    // Project but no active media.
    app.newProject();
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.previewActiveMediaFrameAt(1.0));
    QVERIFY(!app.stepActiveMediaPreview(1.0));
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No active media")));
}

void ProjectTest::applicationPreviewAtUpdatesPositionAndEmits()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);

    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(app.previewTimeSeconds(), 2.0);
    QCOMPARE(timeSpy.count(), 1);
    QCOMPARE(timeSpy.first().first().toDouble(), 2.0);
    QCOMPARE(frameSpy.count(), 1);
    const QImage frame = frameSpy.first().first().value<QImage>();
    QVERIFY(!frame.isNull());
    QVERIFY(blueDominant(frame.pixelColor(frame.width() / 2, frame.height() / 2)));

    // Re-requesting the same position does not re-emit the time change.
    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(timeSpy.count(), 1);
    QCOMPARE(frameSpy.count(), 2);
}

void ProjectTest::applicationStepAdvancesAndClampsBelowZero()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);

    // +1 s from 0.
    QVERIFY(app.stepActiveMediaPreview(1.0));
    QCOMPARE(app.previewTimeSeconds(), 1.0);
    QCOMPARE(timeSpy.count(), 1);

    // Stepping below zero clamps to 0 and decodes there.
    QVERIFY(app.stepActiveMediaPreview(-3.0));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 2);
}

void ProjectTest::applicationSeekBeyondEndFailsDeterministically()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QVERIFY(app.previewActiveMediaFrameAt(0.5));
    QCOMPARE(app.previewTimeSeconds(), 0.5);

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    timeSpy.clear();
    frameSpy.clear();
    messageSpy.clear();

    QVERIFY(!app.previewActiveMediaFrameAt(500.0));
    QCOMPARE(app.previewTimeSeconds(), 0.5); // unchanged
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Preview failed")));
}

void ProjectTest::applicationTimeResetsOnProjectActiveAndRemoval()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString firstVideo;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &firstVideo));
    const QString secondVideo = tempDir.filePath(QStringLiteral("second.mp4"));
    QVERIFY(QFile::copy(firstVideo, secondVideo));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstVideo));
    const QString firstId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(firstId));
    QVERIFY(app.previewActiveMediaFrameAt(1.0));
    QCOMPARE(app.previewTimeSeconds(), 1.0);

    // New project resets the position.
    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    app.newProject();
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);

    // Changing the active media resets the position.
    QVERIFY(app.importMediaFile(firstVideo));
    QVERIFY(app.importMediaFile(secondVideo));
    const QString idA = app.mediaItems().at(0).id();
    const QString idB = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(idA));
    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(app.previewTimeSeconds(), 2.0);

    timeSpy.clear();
    QVERIFY(app.setActiveMedia(idB));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);

    // Removing the active media resets the position.
    QVERIFY(app.previewActiveMediaFrameAt(1.5));
    QCOMPARE(app.previewTimeSeconds(), 1.5);
    timeSpy.clear();
    QVERIFY(app.removeMedia(idB));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);
}

void ProjectTest::mainWindowStepButtonsAndTimeLabel()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::previewStepRequested);

    auto *backButton = window.findChild<QPushButton *>(QStringLiteral("stepBackButton"));
    auto *forwardButton = window.findChild<QPushButton *>(QStringLiteral("stepForwardButton"));
    auto *timeLabel = window.findChild<QLabel *>(QStringLiteral("previewTimeLabel"));
    QVERIFY(backButton);
    QVERIFY(forwardButton);
    QVERIFY(timeLabel);
    QCOMPARE(timeLabel->text(), QStringLiteral("Time: 0.0 s"));

    backButton->click();
    forwardButton->click();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).first().toDouble(), -1.0);
    QCOMPARE(spy.at(1).first().toDouble(), 1.0);

    window.showPreviewTime(3.5);
    QCOMPARE(timeLabel->text(), QStringLiteral("Time: 3.5 s"));
}

namespace {

// Objective 11 pointer test helpers.
constexpr double kLookDegreesPerPixelTest = 0.25;
constexpr double kFovDegreesPerWheelStepTest = 5.0;

void sendMouseEvent(ViewerWidget &widget, QEvent::Type type, const QPointF &position,
                    Qt::MouseButton button, Qt::MouseButtons buttons,
                    Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QMouseEvent event(type, position, button, buttons, modifiers);
    QApplication::sendEvent(&widget, &event);
}

void sendWheelEvent(ViewerWidget &widget, int angleDeltaY)
{
    QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0),
                      QPoint(0, angleDeltaY), Qt::NoButton, Qt::NoModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(&widget, &event);
}

bool nearDouble(double actual, double expected)
{
    return qAbs(actual - expected) < 1e-9;
}

} // namespace

void ProjectTest::viewerWidgetDragEmitsYawAndPitchDeltas()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy yawSpy(&widget, &ViewerWidget::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&widget, &ViewerWidget::viewportPitchDeltaRequested);

    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(100, 100),
                   Qt::LeftButton, Qt::LeftButton);
    // Drag right by 80 px: yaw increases by 0.25 * 80 = 20.
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(180, 100),
                   Qt::NoButton, Qt::LeftButton);
    QCOMPARE(yawSpy.count(), 1);
    QVERIFY(nearDouble(yawSpy.first().first().toDouble(), 20.0));
    QCOMPARE(pitchSpy.count(), 0);

    // Drag up by 40 px: pitch increases by 0.25 * 40 = 10.
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(180, 60),
                   Qt::NoButton, Qt::LeftButton);
    QCOMPARE(pitchSpy.count(), 1);
    QVERIFY(nearDouble(pitchSpy.first().first().toDouble(), 10.0));

    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(180, 60),
                   Qt::LeftButton, Qt::NoButton);
    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    Q_UNUSED(kLookDegreesPerPixelTest);
}

void ProjectTest::viewerWidgetIgnoresNonLeftDragAndReleaseWithoutMove()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy yawSpy(&widget, &ViewerWidget::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&widget, &ViewerWidget::viewportPitchDeltaRequested);

    // Right-button drag is ignored.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(10, 10),
                   Qt::RightButton, Qt::RightButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(60, 10),
                   Qt::NoButton, Qt::RightButton);
    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(60, 10),
                   Qt::RightButton, Qt::NoButton);

    // Press and release without any move emits nothing.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(20, 20),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(20, 20),
                   Qt::LeftButton, Qt::NoButton);
    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
}

void ProjectTest::viewerWidgetWheelUpDecreasesFovWheelDownIncreases()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy fovSpy(&widget, &ViewerWidget::viewportFovDeltaRequested);

    // Wheel up one step: FOV decreases by 5 (zoom in).
    sendWheelEvent(widget, 120);
    QCOMPARE(fovSpy.count(), 1);
    QVERIFY(nearDouble(fovSpy.first().first().toDouble(), -kFovDegreesPerWheelStepTest));

    // Wheel down one step: FOV increases by 5 (zoom out).
    sendWheelEvent(widget, -120);
    QCOMPARE(fovSpy.count(), 2);
    QVERIFY(nearDouble(fovSpy.last().first().toDouble(), kFovDegreesPerWheelStepTest));
}

void ProjectTest::viewerWidgetDragUpdatesApplicationViewport()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(&widget, &ViewerWidget::viewportYawDeltaRequested,
                     &app, &Application::adjustViewportYaw);
    QObject::connect(&widget, &ViewerWidget::viewportPitchDeltaRequested,
                     &app, &Application::adjustViewportPitch);
    QObject::connect(&widget, &ViewerWidget::viewportFovDeltaRequested,
                     &app, &Application::adjustViewportFieldOfView);

    // Drag right 200 px: yaw 0 -> 50.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(100, 100),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(300, 100),
                   Qt::NoButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(300, 100),
                   Qt::LeftButton, Qt::NoButton);
    QVERIFY(nearDouble(state->yaw(), 50.0));

    // Drag up by 400 px on a taller widget: pitch 0 -> clamped at 90 (max).
    widget.resize(400, 2000);
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(150, 1500),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(150, 300),
                   Qt::NoButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(150, 300),
                   Qt::LeftButton, Qt::NoButton);
    QVERIFY(nearDouble(state->pitch(), 90.0));

    // Wheel up one step: FOV 90 -> 85.
    sendWheelEvent(widget, 120);
    QVERIFY(nearDouble(state->fieldOfView(), 85.0));
}

void ProjectTest::mediaItemProjectionDefaultsUnknown()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("proj.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("X");
    file.close();

    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());
    QCOMPARE(item.projection(), MediaItem::Projection::Unknown);

    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Unknown), QString());
    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Equirectangular),
             QStringLiteral("equirectangular"));
    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Flat),
             QStringLiteral("flat"));
    QCOMPARE(MediaItem::projectionFromString(QString()), MediaItem::Projection::Unknown);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("garbage")),
             MediaItem::Projection::Unknown);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("flat")),
             MediaItem::Projection::Flat);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("equirectangular")),
             MediaItem::Projection::Equirectangular);
}

void ProjectTest::mediaItemProjectionJsonRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("proj.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("Y");
    file.close();

    // Flat round trip.
    MediaItem flatItem = MediaItem::createFromFilePath(mediaPath);
    flatItem.setProjection(MediaItem::Projection::Flat);
    const QJsonObject flatObject = flatItem.toJsonObject();
    QCOMPARE(flatObject.value(QStringLiteral("projection")).toString(),
             QStringLiteral("flat"));

    MediaItem flatRestored;
    QString error;
    QVERIFY(flatRestored.readFromJsonObject(flatObject, &error));
    QCOMPARE(flatRestored.projection(), MediaItem::Projection::Flat);

    // Equirectangular round trip.
    flatItem.setProjection(MediaItem::Projection::Equirectangular);
    const QJsonObject equirectObject = flatItem.toJsonObject();
    MediaItem equirectRestored;
    QVERIFY(equirectRestored.readFromJsonObject(equirectObject, &error));
    QCOMPARE(equirectRestored.projection(), MediaItem::Projection::Equirectangular);

    // Unknown is not serialized; absent/unrecognized values read as Unknown.
    flatItem.setProjection(MediaItem::Projection::Unknown);
    QVERIFY(!flatItem.toJsonObject().contains(QStringLiteral("projection")));

    QJsonObject withBadProjection = flatObject;
    withBadProjection.insert(QStringLiteral("projection"), QStringLiteral("fisheye"));
    MediaItem badRestored;
    QVERIFY(badRestored.readFromJsonObject(withBadProjection, &error));
    QCOMPARE(badRestored.projection(), MediaItem::Projection::Unknown);
}

void ProjectTest::applicationDeclareProjectionGuardsValidateAndEmit()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("decl.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("Z");
    file.close();

    Application app;
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);

    // No project.
    QVERIFY(!app.declareMediaProjection(QStringLiteral("x"), QStringLiteral("flat")));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No project")));

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();

    // Unknown media id.
    QVERIFY(!app.declareMediaProjection(QStringLiteral("no-such-id"),
                                        QStringLiteral("flat")));
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("not found")));

    // Invalid projection value.
    QVERIFY(!app.declareMediaProjection(mediaId, QStringLiteral("fisheye")));
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("unknown projection")));

    // Valid declarations update the record and emit the list change.
    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("flat")));
    QCOMPARE(app.mediaItems().at(0).projection(), MediaItem::Projection::Flat);
    QCOMPARE(listSpy.count(), 1);
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("projection")));

    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("equirectangular")));
    QCOMPARE(app.mediaItems().at(0).projection(), MediaItem::Projection::Equirectangular);
    QCOMPARE(listSpy.count(), 2);
}

void ProjectTest::applicationDeclaredProjectionPersistsOnReopen()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString flatPath = tempDir.filePath(QStringLiteral("flat.bin"));
    const QString unknownPath = tempDir.filePath(QStringLiteral("unknown.mov"));
    for (const QString &path : { flatPath, unknownPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(8, 'p'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(flatPath));
    QVERIFY(app.importMediaFile(unknownPath));
    const QString flatId = app.mediaItems().at(0).id();
    QVERIFY(app.declareMediaProjection(flatId, QStringLiteral("flat")));

    const QString projectPath = tempDir.filePath(QStringLiteral("proj.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 2);
    QCOMPARE(reopened.mediaItems().at(0).projection(), MediaItem::Projection::Flat);
    QCOMPARE(reopened.mediaItems().at(1).projection(), MediaItem::Projection::Unknown);
}

void ProjectTest::viewerWidgetFlatModeLetterboxesAndCenters()
{
    ViewerWidget widget;
    widget.resize(400, 200);

    // 4:3 solid source inside a 2:1 widget => vertical fit, side letterbox.
    QImage source(200, 150, QImage::Format_RGB32);
    source.fill(QColor(0, 255, 255));
    widget.setFlatSourceMode(true);
    widget.setSourceImage(source);
    QVERIFY(widget.isFlatSourceMode());

    const QImage view = widget.grab().toImage();
    expectColor(view, 200, 100, QColor(0, 255, 255)); // content center
    expectColor(view, 5, 100, QColor(18, 20, 24));    // left letterbox bar
    expectColor(view, 395, 100, QColor(18, 20, 24));  // right letterbox bar
}

void ProjectTest::viewerWidgetFlatModeIgnoresCameraTransforms()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(300, 300);

    QImage source(150, 150, QImage::Format_RGB32);
    source.fill(QColor(255, 0, 255));
    widget.setFlatSourceMode(true);
    widget.setSourceImage(source);

    const QImage before = widget.grab().toImage();
    state.setYaw(90.0);
    state.setPitch(45.0);
    state.setFieldOfView(40.0);
    const QImage after = widget.grab().toImage();
    QVERIFY(imagesIdentical(before, after));
}

void ProjectTest::mainWindowProjectionButtonsEmitRequests()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QList<MediaItem> items;
    const QStringList names = { QStringLiteral("a.bin"), QStringLiteral("b.bin") };
    for (const QString &name : names) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("M");
        file.close();
        items.append(MediaItem::createFromFilePath(path));
    }

    TestMainWindow window;
    window.showMediaList(items);

    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    QCOMPARE(list->count(), 2);

    auto *markFlat = window.findChild<QPushButton *>(QStringLiteral("markFlatButton"));
    auto *markEquirect = window.findChild<QPushButton *>(QStringLiteral("markEquirectButton"));
    QVERIFY(markFlat);
    QVERIFY(markEquirect);

    QSignalSpy spy(&window, &MainWindow::setMediaProjectionRequested);
    list->setCurrentRow(0);
    markFlat->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), items.at(0).id());
    QCOMPARE(spy.first().at(1).toString(), QStringLiteral("flat"));

    markEquirect->click();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().first().toString(), items.at(0).id());
    QCOMPARE(spy.last().at(1).toString(), QStringLiteral("equirectangular"));
}

void ProjectTest::previewRoutingHonorsDeclaredProjection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("P");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);

    // Flat solid frame presented in flat mode (no camera transform).
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("flat")));
    QImage flatFrame(200, 100, QImage::Format_RGB32);
    flatFrame.fill(QColor(0, 255, 255));
    window.showFramePreview(flatFrame);
    QVERIFY(viewer->isFlatSourceMode());
    QImage view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, QColor(0, 255, 255));

    // Equirectangular routing: equirectangular pixel path (FRONT centered).
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("equirectangular")));
    window.showFramePreview(buildTestPattern());
    QVERIFY(!viewer->isFlatSourceMode());
    view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, kFrontColor);
}

void ProjectTest::viewerClearedOnNewProjectAfterPreview()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("a.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("A");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(100, 50, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Creating a new project must clear the stale frame (back to scene) and
    // reset flat mode.
    viewer->setFlatSourceMode(true);
    app.newProject();
    QVERIFY(!viewer->hasSourceImage());
    QVERIFY(!viewer->isFlatSourceMode());

    // The deterministic marker scene is visible again at identity.
    const QImage view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, QColor(255, 213, 79));
}

void ProjectTest::viewerClearedWhenActiveMediaChangesOrRemoved()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("a.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("b.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("M");
        file.close();
    }

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);

    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));
    const QString idA = app.mediaItems().at(0).id();
    const QString idB = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(idA));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(50, 50, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Switching to a different active media clears the stale frame.
    QVERIFY(app.setActiveMedia(idB));
    QVERIFY(!viewer->hasSourceImage());

    window.showFramePreview(QImage(60, 40, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Removing the active media clears it as well.
    QVERIFY(app.removeMedia(idB));
    QVERIFY(!viewer->hasSourceImage());
}

void ProjectTest::viewerSourcePersistsWhileContextStable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("a.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("S");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(80, 40, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Re-announcing the same active id must not clear the presented frame.
    window.showActiveMedia(mediaId);
    QVERIFY(viewer->hasSourceImage());
}

void ProjectTest::equirectViewBilinearBlendsFourNeighbors()
{
    // 4x4 source with four distinct colors around texel coordinates
    // (1..2, 1..2); a 1x1 render whose center ray lands exactly at (1.5, 1.5)
    // must produce the quarter blend (bilinear), not any single neighbor
    // (nearest).
    QImage source(4, 4, QImage::Format_ARGB32);
    source.fill(QColor(0, 0, 0));
    source.setPixelColor(1, 1, QColor(200, 0, 0));
    source.setPixelColor(2, 1, QColor(0, 200, 0));
    source.setPixelColor(1, 2, QColor(0, 0, 200));
    source.setPixelColor(2, 2, QColor(100, 100, 100));

    QImage view;
    QVERIFY(EquirectView::render(source, -45.0, 22.5, 0.0, 90.0, 1, 1, &view));
    QCOMPARE(view.width(), 1);
    QCOMPARE(view.height(), 1);

    const QColor color = view.pixelColor(0, 0);
    QVERIFY(qAbs(color.red() - 75) <= 3);
    QVERIFY(qAbs(color.green() - 75) <= 3);
    QVERIFY(qAbs(color.blue() - 75) <= 3);
    // Bilinear, not nearest: must not equal any pure neighbor.
    QVERIFY(color != QColor(200, 0, 0));
    QVERIFY(color != QColor(0, 200, 0));
    QVERIFY(color != QColor(0, 0, 200));
}

void ProjectTest::equirectViewBilinearRobustAtSeamAndPoles()
{
    const QImage pattern = buildTestPattern();

    // Pole directions clamp vertically and stay exact on uniform rows.
    QImage viewUp;
    QVERIFY(EquirectView::render(pattern, 0.0, 90.0, 0.0, 90.0, 200, 100, &viewUp));
    expectColor(viewUp, 100, 50, kUpColor);

    QImage viewDown;
    QVERIFY(EquirectView::render(pattern, 0.0, -90.0, 0.0, 90.0, 200, 100, &viewDown));
    expectColor(viewDown, 100, 50, kDownColor);

    // The horizontal seam (+/-180 deg yaw) wraps deterministically without
    // artifacts; center is background (no marker region at yaw 180).
    QImage firstSeam;
    QImage secondSeam;
    QVERIFY(EquirectView::render(pattern, 180.0, 0.0, 0.0, 90.0, 200, 100, &firstSeam));
    QVERIFY(EquirectView::render(pattern, 180.0, 0.0, 0.0, 90.0, 200, 100, &secondSeam));
    QVERIFY(imagesIdentical(firstSeam, secondSeam));
    expectColor(firstSeam, 100, 50, kPatternBackground);
}

void ProjectTest::equirectReviewFixtureFramesAreDistinctAndSeekable()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    QImage atZero;
    QImage atOne;
    QImage atFive;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 0.0, &atZero));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.0, &atOne));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 5.0, &atFive));

    const QColor centerZero = atZero.pixelColor(atZero.width() / 2, atZero.height() / 2);
    const QColor centerOne = atOne.pixelColor(atOne.width() / 2, atOne.height() / 2);
    const QColor centerFive = atFive.pixelColor(atFive.width() / 2, atFive.height() / 2);

    QVERIFY(reviewRedDominant(centerZero));   // frame 0 = red
    QVERIFY(reviewGreenDominant(centerOne));  // frame 1 = green
    QVERIFY(reviewBlueDominant(centerFive));  // frame 5 % 3 == 2 = blue

    QVERIFY(!imagesIdentical(atZero, atOne));
}

void ProjectTest::reviewPathEndToEndOnEquirectClip()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&app, &Application::framePreviewReady,
                     &window, &MainWindow::showFramePreview);
    QObject::connect(&app, &Application::previewTimeChanged,
                     &window, &MainWindow::showPreviewTime);
    QObject::connect(&window, &MainWindow::previewStepRequested,
                     &app, &Application::stepActiveMediaPreview);

    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    viewer->setViewportState(app.viewportState());

    auto centerColor = [viewer]() {
        const QImage image = viewer->grab().toImage();
        return image.pixelColor(image.width() / 2, image.height() / 2);
    };

    // Preview at t=0: FRONT red.
    QVERIFY(app.previewActiveMediaFrameAt(0.0));
    QVERIFY(viewer->hasSourceImage());
    QVERIFY(reviewRedDominant(centerColor()));

    // Step +1 s: FRONT green.
    auto *stepForward = window.findChild<QPushButton *>(QStringLiteral("stepForwardButton"));
    QVERIFY(stepForward);
    stepForward->click();
    QVERIFY(reviewGreenDominant(centerColor()));
    QCOMPARE(app.previewTimeSeconds(), 1.0);

    // Look around while the frame is present: +90 yaw centers RIGHT (magenta).
    app.adjustViewportYaw(90.0);
    QVERIFY(reviewMagentaDominant(centerColor()));
    app.resetViewport();

    // Seek far forward: frame 5 % 3 == 2 => FRONT blue.
    QVERIFY(app.previewActiveMediaFrameAt(5.0));
    QVERIFY(reviewBlueDominant(centerColor()));
    QCOMPARE(app.previewTimeSeconds(), 5.0);
}

void ProjectTest::equirectReviewPerformanceInformational()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    // One-shot per-step decode cost on the real clip.
    QElapsedTimer timer;
    timer.start();
    QImage frame;
    for (int t = 0; t < 5; ++t) {
        QVERIFY(FrameExtractor::extractFrameAt(
            videoPath, FrameExtractor::defaultExecutablePath(),
            static_cast<double>(t), &frame));
    }
    const double decodeMs = static_cast<double>(timer.nsecsElapsed()) / 5 / 1e6;
    qInfo("Review path: single-frame decode ~%.1f ms/step on the equirect clip", decodeMs);

    // Per-paint equirect render cost on a decoded frame (640 cap).
    const QImage pattern = buildTestPattern();
    timer.restart();
    QImage view;
    for (int i = 0; i < 5; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0,
                                     EquirectView::MaxOutputWidth, 320, &view));
    }
    const double renderMs = static_cast<double>(timer.nsecsElapsed()) / 5 / 1e6;
    qInfo("Review path: per-paint render ~%.1f ms at %dx%d",
          renderMs, EquirectView::MaxOutputWidth, 320);

    // Informational only; no timing gate.
}

void ProjectTest::persistentStreamProbeDeliversFramesInOrder()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 25;
    constexpr int kFps = 5;
    constexpr int kWidth = 160;
    constexpr int kHeight = 80;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, kFps, &videoPath));

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(FrameExtractor::defaultExecutablePath(), {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-i"), videoPath,
        QStringLiteral("-f"), QStringLiteral("rawvideo"),
        QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
        QStringLiteral("-")
    });
    QVERIFY(process.waitForStarted(10000));

    QElapsedTimer clock;
    clock.start();
    QByteArray pending;
    QList<double> arrivalMs;
    QList<QImage> frames;
    constexpr int kReadTimeoutMs = 60000;
    while (frames.size() < kFrames && clock.elapsed() < kReadTimeoutMs) {
        QImage frame;
        if (!readRawFrame(process, kWidth, kHeight, pending, &frame, kReadTimeoutMs)) {
            break;
        }
        arrivalMs.append(clock.elapsed());
        frames.append(frame);
    }

    // Allow the process to finish and reach EOF within a bounded wait.
    if (process.state() != QProcess::NotRunning) {
        process.waitForFinished(15000);
    }
    const QByteArray errorOutput = process.readAllStandardError();

    const int delivered = frames.size();
    QVERIFY2(delivered >= kFrames, qPrintable(
        QStringLiteral("Expected %1 frames, delivered %2; stderr: %3")
            .arg(kFrames).arg(delivered)
            .arg(QString::fromUtf8(errorOutput).trimmed().left(200))));
    QVERIFY(process.exitStatus() == QProcess::NormalExit);
    QVERIFY(process.exitCode() == 0);

    // Content in order: frame i carries the i%3 color (red/green/blue cycle).
    for (int i = 0; i < delivered && i < 9; ++i) {
        QVERIFY2(classifyFrameColor(frames.at(i), i),
                 qPrintable(QStringLiteral("Frame %1 color mismatch").arg(i)));
    }

    const double elapsedSeconds = clock.elapsed() / 1000.0;
    const double deliveredFps = elapsedSeconds > 0.0 ? delivered / elapsedSeconds : 0.0;
    double firstArrivalMs = 0.0;
    if (arrivalMs.size() >= 2) {
        firstArrivalMs = arrivalMs.at(1) - arrivalMs.at(0);
    }
    qInfo("Persistent subprocess probe: %d/%d frames delivered, ~%.1f frames/s, "
          "median inter-frame latency ~%.1f ms (informational; rawvideo, unthrottled)",
          delivered, kFrames, deliveredFps, firstArrivalMs);

    // No timing gate; the above are evidence for the feasibility record.
}

void ProjectTest::persistentStreamProbeErrorOnMissingFile()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(FrameExtractor::defaultExecutablePath(), {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-i"), QStringLiteral("/nonexistent/probe.mp4"),
        QStringLiteral("-f"), QStringLiteral("rawvideo"),
        QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
        QStringLiteral("-")
    });
    QVERIFY(process.waitForStarted(10000));
    process.waitForFinished(15000);
    QVERIFY(process.exitStatus() == QProcess::NormalExit);
    QVERIFY(process.exitCode() != 0);
    QVERIFY(!process.readAllStandardError().isEmpty());
}

void ProjectTest::persistentStreamProbeKillAndRestartLifecycle()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   25, 5, &videoPath));

    auto startStream = [&videoPath](QProcess &process) {
        process.setProcessChannelMode(QProcess::SeparateChannels);
        process.start(FrameExtractor::defaultExecutablePath(), {
            QStringLiteral("-v"), QStringLiteral("error"),
            QStringLiteral("-nostdin"),
            QStringLiteral("-i"), videoPath,
            QStringLiteral("-f"), QStringLiteral("rawvideo"),
            QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
            QStringLiteral("-")
        });
        return process.waitForStarted(10000);
    };

    // First stream: read one frame, then kill cleanly.
    QProcess first;
    QVERIFY(startStream(first));
    QByteArray pending;
    QImage frame;
    QVERIFY(readRawFrame(first, 160, 80, pending, &frame, 30000));
    first.terminate();
    if (!first.waitForFinished(3000)) {
        first.kill();
        first.waitForFinished(3000);
    }
    QVERIFY(first.state() == QProcess::NotRunning);

    // Restart: a fresh process must stream from the beginning again.
    QProcess second;
    QVERIFY(startStream(second));
    pending.clear();
    QVERIFY(readRawFrame(second, 160, 80, pending, &frame, 30000));
    QVERIFY(classifyFrameColor(frame, 0)); // frame 0 = red
    second.terminate();
    if (!second.waitForFinished(3000)) {
        second.kill();
        second.waitForFinished(3000);
    }
    QVERIFY(second.state() == QProcess::NotRunning);
}

// --- Phase 3 Objective 3: replaceable media-source seam + deterministic frame pump ---

void ProjectTest::framePumpDeliversFramesAndEndFromReplaceableSource()
{
    // Replaceable-seam proof: an in-memory FrameSource drives FramePump with no
    // ffmpeg subprocess, media file, or timing dependency.
    FakeFrameSource source;
    QImage red(4, 3, QImage::Format_RGB32);
    red.fill(QColor(230, 0, 0));
    QImage green(4, 3, QImage::Format_RGB32);
    green.fill(QColor(0, 230, 0));
    source.appendFrame(red);
    source.appendFrame(green);

    FramePump pump;
    pump.setSource(&source);
    QVERIFY(pump.source() == static_cast<FrameSource *>(&source));

    QList<QImage> received;
    int ended = 0;
    int failed = 0;
    QObject::connect(&pump, &FramePump::frameReady,
                     [&received](const QImage &image) { received.append(image); });
    QObject::connect(&pump, &FramePump::streamEnded, [&ended]() { ++ended; });
    QObject::connect(&pump, &FramePump::streamFailed,
                     [&failed](const QString &) { ++failed; });

    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    QVERIFY(pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QCOMPARE(static_cast<int>(received.size()), 2);
    QVERIFY(imagesIdentical(received.at(0), red));
    QVERIFY(imagesIdentical(received.at(1), green));

    // End of a finite source is reported exactly once and delivers no frame.
    QVERIFY(!pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::EndOfStream);
    QCOMPARE(ended, 1);
    QCOMPARE(failed, 0);
    QCOMPARE(static_cast<int>(received.size()), 2);
}

void ProjectTest::framePumpHandlesErrorTimeoutAndMissingSource()
{
    // Error branch: the source error text is relayed through streamFailed.
    FakeFrameSource errorSource;
    errorSource.setMode(FakeFrameSource::Mode::Error);
    errorSource.setErrorText(QStringLiteral("synthetic decode failure"));
    FramePump errorPump;
    errorPump.setSource(&errorSource);
    int errorFailures = 0;
    QString failureMessage;
    QObject::connect(&errorPump, &FramePump::streamFailed,
                     [&errorFailures, &failureMessage](const QString &text) {
                         ++errorFailures;
                         failureMessage = text;
                     });
    FrameSource::ReadResult result = FrameSource::ReadResult::Ok;
    QVERIFY(!errorPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Error);
    QCOMPARE(errorFailures, 1);
    QCOMPARE(failureMessage, QStringLiteral("synthetic decode failure"));

    // Timeout branch: no signal is emitted; the caller may advance again.
    FakeFrameSource timeoutSource;
    timeoutSource.setMode(FakeFrameSource::Mode::Timeout);
    FramePump timeoutPump;
    timeoutPump.setSource(&timeoutSource);
    int timeoutSignals = 0;
    QObject::connect(&timeoutPump, &FramePump::frameReady,
                     [&timeoutSignals](const QImage &) { ++timeoutSignals; });
    QObject::connect(&timeoutPump, &FramePump::streamEnded,
                     [&timeoutSignals]() { ++timeoutSignals; });
    QObject::connect(&timeoutPump, &FramePump::streamFailed,
                     [&timeoutSignals](const QString &) { ++timeoutSignals; });
    QVERIFY(!timeoutPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Timeout);
    QCOMPARE(timeoutSignals, 0);

    // Missing source: deterministic failure, never a crash or a frame.
    FramePump emptyPump;
    int emptySignals = 0;
    QString emptyMessage;
    QObject::connect(&emptyPump, &FramePump::streamFailed,
                     [&emptySignals, &emptyMessage](const QString &text) {
                         ++emptySignals;
                         emptyMessage = text;
                     });
    QVERIFY(!emptyPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Error);
    QCOMPARE(emptySignals, 1);
    QVERIFY(!emptyMessage.isEmpty());
}

void ProjectTest::fmpegFrameSourceStreamsFramesInOrder()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 25;
    constexpr int kWidth = 160;
    constexpr int kHeight = 80;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, 5, &videoPath));

    FfmpegFrameSource source;
    QVERIFY2(source.open(videoPath, kWidth, kHeight),
             qPrintable(source.errorString()));
    QVERIFY(source.isOpen());
    QVERIFY(source.errorString().isEmpty());

    for (int i = 0; i < kFrames; ++i) {
        FrameSource::ReadResult result = FrameSource::ReadResult::Error;
        QImage frame;
        QVERIFY2(source.readNextFrame(30000, &result, &frame),
                 qPrintable(QStringLiteral("frame %1 failed: %2")
                                .arg(i).arg(source.errorString())));
        QVERIFY(result == FrameSource::ReadResult::Ok);
        QCOMPARE(frame.size(), QSize(kWidth, kHeight));
        QVERIFY2(classifyFrameColor(frame, i),
                 qPrintable(QStringLiteral("frame %1 color mismatch").arg(i)));
    }

    // The stream reaches end-of-stream deterministically after the last frame.
    FrameSource::ReadResult endResult = FrameSource::ReadResult::Error;
    QImage extra;
    QVERIFY(!source.readNextFrame(30000, &endResult, &extra));
    QVERIFY(endResult == FrameSource::ReadResult::EndOfStream);
    QVERIFY(source.errorString().isEmpty());

    source.close();
    QVERIFY(!source.isOpen());
}

void ProjectTest::fmpegFrameSourceValidatesAndRestartsCleanly()
{
    FfmpegFrameSource source;
    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    QImage frame;

    // Rawvideo carries no geometry metadata and ffprobe discovery is deferred,
    // so opening without explicit geometry must fail deterministically.
    QVERIFY(!source.open(QStringLiteral("/nonexistent/clip.mp4"), 0, 0));
    QVERIFY(!source.isOpen());
    QVERIFY(!source.errorString().isEmpty());
    QVERIFY(!source.readNextFrame(100, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Error);

    // Missing file with valid geometry also fails deterministically.
    QVERIFY(!source.open(QStringLiteral("/nonexistent/clip.mp4"), 160, 80));
    QVERIFY(!source.isOpen());
    QVERIFY(source.errorString().contains(QStringLiteral("does not exist")));

    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping stream restart portion.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   6, 5, &videoPath));

    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));
    QVERIFY(source.readNextFrame(30000, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(classifyFrameColor(frame, 0));

    // close() releases the process and reading afterwards fails cleanly.
    source.close();
    QVERIFY(!source.isOpen());
    QVERIFY(!source.readNextFrame(100, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Error);

    // The same instance can be reopened and streams from the start again.
    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));
    QVERIFY(source.readNextFrame(30000, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(classifyFrameColor(frame, 0));
    source.close();
    QVERIFY(!source.isOpen());
}

void ProjectTest::framePumpStreamsFfmpegSourceToEnd()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 12;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, 5, &videoPath));

    FfmpegFrameSource source;
    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));

    FramePump pump;
    pump.setSource(&source);

    QList<QImage> frames;
    int ended = 0;
    int failed = 0;
    QObject::connect(&pump, &FramePump::frameReady,
                     [&frames](const QImage &image) { frames.append(image); });
    QObject::connect(&pump, &FramePump::streamEnded, [&ended]() { ++ended; });
    QObject::connect(&pump, &FramePump::streamFailed,
                     [&failed](const QString &) { ++failed; });

    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    for (int i = 0; i < kFrames; ++i) {
        QVERIFY2(pump.advance(30000, &result),
                 qPrintable(QStringLiteral("pump advance %1 failed: %2")
                                .arg(i).arg(source.errorString())));
        QVERIFY(result == FrameSource::ReadResult::Ok);
    }
    QCOMPARE(static_cast<int>(frames.size()), kFrames);
    for (int i = 0; i < kFrames; ++i) {
        QVERIFY2(classifyFrameColor(frames.at(i), i),
                 qPrintable(QStringLiteral("pumped frame %1 color mismatch").arg(i)));
    }

    QVERIFY(!pump.advance(30000, &result));
    QVERIFY(result == FrameSource::ReadResult::EndOfStream);
    QCOMPARE(ended, 1);
    QCOMPARE(failed, 0);

    source.close();
    QVERIFY(!source.isOpen());
}

// --- Phase 3 Objective 4: player/timing subsystem foundation ---

void ProjectTest::playheadTracksFrameCountIndexAndPosition()
{
    Playhead playhead;
    QCOMPARE(playhead.frameCount(), qint64(0));
    QCOMPARE(playhead.currentFrameIndex(), qint64(-1));
    QCOMPARE(playhead.positionMs(40), qint64(0));

    playhead.advance();
    QCOMPARE(playhead.frameCount(), qint64(1));
    QCOMPARE(playhead.currentFrameIndex(), qint64(0));
    QCOMPARE(playhead.positionMs(40), qint64(0));

    playhead.advance();
    playhead.advance();
    QCOMPARE(playhead.frameCount(), qint64(3));
    QCOMPARE(playhead.currentFrameIndex(), qint64(2));
    QCOMPARE(playhead.positionMs(40), qint64(80));

    // Invalid intervals never produce a negative or undefined position.
    QCOMPARE(playhead.positionMs(0), qint64(0));
    QCOMPARE(playhead.positionMs(-5), qint64(0));

    playhead.reset();
    QCOMPARE(playhead.frameCount(), qint64(0));
    QCOMPARE(playhead.currentFrameIndex(), qint64(-1));
    QCOMPARE(playhead.positionMs(40), qint64(0));
}

void ProjectTest::playerInitialStateAndDefaults()
{
    FakeFrameSource source;
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing;

    Player player(&pump, &clock, &pacing);
    QVERIFY(player.isStopped());
    QVERIFY(!player.isPlaying());
    QVERIFY(!player.isPaused());
    QVERIFY(player.state() == Player::State::Stopped);
    QCOMPARE(player.frameCount(), qint64(0));
    QCOMPARE(player.currentFrameIndex(), qint64(-1));
    QCOMPARE(player.positionMs(), qint64(0));
    QCOMPARE(player.frameIntervalMs(), qint64(40));
    QVERIFY(player.framePump() == &pump);
    QVERIFY(player.clock() == static_cast<Clock *>(&clock));
    QVERIFY(player.pacingPolicy() == static_cast<PacingPolicy *>(&pacing));

    // Constructing a player emits nothing.
    int signalCount = 0;
    QObject::connect(&player, &Player::stateChanged,
                     [&signalCount](Player::State) { ++signalCount; });
    QObject::connect(&player, &Player::positionChanged,
                     [&signalCount](qint64, qint64) { ++signalCount; });
    QObject::connect(&player, &Player::framePresented,
                     [&signalCount](const QImage &, qint64, qint64) { ++signalCount; });
    QCOMPARE(signalCount, 0);

    // The convenience constructor owns usable clock/pacing defaults and can be
    // driven deterministically through stepOnce().
    FakeFrameSource defaultSource;
    defaultSource.appendFrame(playerTestFrame(0));
    FramePump defaultPump;
    defaultPump.setSource(&defaultSource);
    Player defaultPlayer(&defaultPump);
    QVERIFY(defaultPlayer.clock() != nullptr);
    QVERIFY(defaultPlayer.pacingPolicy() != nullptr);
    QCOMPARE(defaultPlayer.frameIntervalMs(), qint64(40));
    QVERIFY(defaultPlayer.stepOnce());
    QCOMPARE(defaultPlayer.frameCount(), qint64(1));
}

void ProjectTest::playerPlayPauseStopTransitions()
{
    FakeFrameSource source;
    source.appendFrame(playerTestFrame(0));
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing(0);

    Player player(&pump, &clock, &pacing);
    QList<Player::State> states;
    QObject::connect(&player, &Player::stateChanged,
                     [&states](Player::State state) { states.append(state); });

    player.play();
    QVERIFY(player.isPlaying());
    player.play(); // idempotent
    QCOMPARE(states.size(), 1);
    QVERIFY(states.at(0) == Player::State::Playing);

    player.pause();
    QVERIFY(player.isPaused());
    player.pause(); // idempotent
    QCOMPARE(states.size(), 2);
    QVERIFY(states.at(1) == Player::State::Paused);

    player.play();
    QVERIFY(player.isPlaying());
    QCOMPARE(states.size(), 3);
    QVERIFY(states.at(2) == Player::State::Playing);

    player.stop();
    QVERIFY(player.isStopped());
    QCOMPARE(states.size(), 4);
    QVERIFY(states.at(3) == Player::State::Stopped);

    // stop() while already stopped and pause() while stopped are no-ops.
    player.stop();
    player.pause();
    QVERIFY(player.isStopped());
    QCOMPARE(states.size(), 4);
}

void ProjectTest::playerTickUsesInjectedClockAndPacingPolicy()
{
    FakeFrameSource source;
    for (int i = 0; i < 6; ++i) {
        source.appendFrame(playerTestFrame(i));
    }
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing(0);

    Player player(&pump, &clock, &pacing);
    QVERIFY(player.setFrameIntervalMs(50));

    // tick() is a no-op unless Playing and never consults pacing then.
    QCOMPARE(player.tick(), 0);
    QCOMPARE(pacing.calls(), 0);
    QCOMPARE(player.frameCount(), qint64(0));

    player.play(); // records lastPacing at clock.nowMillis() == 0
    clock.setMillis(1000);

    pacing.setFramesPerCall(3);
    QCOMPARE(player.tick(), 3);
    QCOMPARE(pacing.calls(), 1);
    QCOMPARE(pacing.lastElapsedMs(), qint64(1000));
    QCOMPARE(pacing.lastFrameIntervalMs(), qint64(50));
    QCOMPARE(player.frameCount(), qint64(3));
    QCOMPARE(player.currentFrameIndex(), qint64(2));
    QCOMPARE(player.positionMs(), qint64(100));

    // A policy that reports no work advances nothing.
    pacing.setFramesPerCall(0);
    QCOMPARE(player.tick(), 0);
    QCOMPARE(player.frameCount(), qint64(3));

    // Replacing the policy takes effect immediately.
    pacing.setFramesPerCall(2);
    QCOMPARE(player.tick(), 2);
    QCOMPARE(player.frameCount(), qint64(5));
    QCOMPARE(player.positionMs(), qint64(200));

    // Pausing stops tick() advancement entirely.
    player.pause();
    clock.advance(10000);
    pacing.setFramesPerCall(50);
    QCOMPARE(player.tick(), 0);
    QCOMPARE(player.frameCount(), qint64(5));

    // Resuming continues from the retained playhead and drains the remainder.
    int ended = 0;
    QObject::connect(&player, &Player::playbackEnded, [&ended]() { ++ended; });
    player.play();
    clock.advance(100);
    QCOMPARE(player.tick(), 1);
    QCOMPARE(player.frameCount(), qint64(6));
    QVERIFY(player.isStopped());
    QCOMPARE(ended, 1);
}

void ProjectTest::playerDefaultPacingAdvancesOnFrameIntervals()
{
    FakeFrameSource source;
    for (int i = 0; i < 5; ++i) {
        source.appendFrame(playerTestFrame(i));
    }
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    DefaultPacingPolicy pacing;

    Player player(&pump, &clock, &pacing);
    QVERIFY(player.setFrameIntervalMs(40));
    player.play(); // lastPacing at 0

    QCOMPARE(player.tick(), 0); // elapsed 0

    clock.advance(39);
    QCOMPARE(player.tick(), 0); // not yet a full interval
    QCOMPARE(player.frameCount(), qint64(0));

    clock.advance(1); // now 40: exactly one interval
    QCOMPARE(player.tick(), 1);
    QCOMPARE(player.frameCount(), qint64(1));
    QCOMPARE(player.positionMs(), qint64(0));

    clock.advance(120); // now 160: 120 ms accrued == 3 intervals
    QCOMPARE(player.tick(), 3);
    QCOMPARE(player.frameCount(), qint64(4));
    QCOMPARE(player.positionMs(), qint64(120));

    // A tick with no accrued interval presents nothing.
    QCOMPARE(player.tick(), 0);
    QCOMPARE(player.frameCount(), qint64(4));

    // A non-monotonic clock is clamped, never a negative burst.
    clock.setMillis(0);
    QCOMPARE(player.tick(), 0);
    QCOMPARE(player.frameCount(), qint64(4));
}

void ProjectTest::playerStepOncePresentsExactlyOneFrame()
{
    FakeFrameSource source;
    for (int i = 0; i < 2; ++i) {
        source.appendFrame(playerTestFrame(i));
    }
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing(7); // pacing must be irrelevant to stepOnce

    Player player(&pump, &clock, &pacing);
    QList<QImage> presented;
    int positions = 0;
    int ended = 0;
    QObject::connect(&player, &Player::framePresented,
                     [&presented](const QImage &image, qint64, qint64) {
                         presented.append(image);
                     });
    QObject::connect(&player, &Player::positionChanged,
                     [&positions](qint64, qint64) { ++positions; });
    QObject::connect(&player, &Player::playbackEnded, [&ended]() { ++ended; });

    // Stepping is independent of state, clock, and pacing.
    QVERIFY(player.isStopped());
    QVERIFY(player.stepOnce());
    QCOMPARE(player.frameCount(), qint64(1));
    QCOMPARE(player.currentFrameIndex(), qint64(0));
    QCOMPARE(pacing.calls(), 0);

    QVERIFY(player.stepOnce());
    QCOMPARE(player.frameCount(), qint64(2));
    QCOMPARE(static_cast<int>(presented.size()), 2);
    QVERIFY(imagesIdentical(presented.at(0), playerTestFrame(0)));
    QVERIFY(imagesIdentical(presented.at(1), playerTestFrame(1)));
    QCOMPARE(positions, 2);

    // The third step hits end-of-stream: no frame, but it is reported.
    QVERIFY(!player.stepOnce());
    QCOMPARE(player.frameCount(), qint64(2));
    QCOMPARE(ended, 1);
    QCOMPARE(static_cast<int>(presented.size()), 2);
}

void ProjectTest::playerReachesEndOfStreamDeterministically()
{
    FakeFrameSource source;
    for (int i = 0; i < 3; ++i) {
        source.appendFrame(playerTestFrame(i));
    }
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing(10);

    Player player(&pump, &clock, &pacing);
    int ended = 0;
    QObject::connect(&player, &Player::playbackEnded, [&ended]() { ++ended; });

    player.play();
    clock.advance(1000);

    // The pacing policy asks for more frames than exist; the player presents
    // all three, then reports end-of-stream and stops within one tick.
    QCOMPARE(player.tick(), 3);
    QCOMPARE(player.frameCount(), qint64(3));
    QVERIFY(player.isStopped());
    QCOMPARE(ended, 1);

    // Further ticks are no-ops while stopped.
    QCOMPARE(player.tick(), 0);
    QCOMPARE(ended, 1);

    // Playing again immediately re-reports the already-exhausted stream.
    player.play();
    QVERIFY(player.isPlaying());
    QCOMPARE(player.tick(), 0);
    QVERIFY(player.isStopped());
    QCOMPARE(ended, 2);
}

void ProjectTest::playerReportsFramePumpErrors()
{
    FakeFrameSource source;
    source.setMode(FakeFrameSource::Mode::Error);
    source.setErrorText(QStringLiteral("synthetic player decode failure"));
    FramePump pump;
    pump.setSource(&source);
    ManualClock clock;
    RecordingPacingPolicy pacing(1);

    Player player(&pump, &clock, &pacing);
    int errors = 0;
    QString message;
    QObject::connect(&player, &Player::errorOccurred,
                     [&errors, &message](const QString &text) {
                         ++errors;
                         message = text;
                     });

    player.play();
    clock.advance(100);
    QCOMPARE(player.tick(), 0);
    QCOMPARE(errors, 1);
    QCOMPARE(message, QStringLiteral("synthetic player decode failure"));
    QVERIFY(player.isStopped());
}

void ProjectTest::playerHandlesInvalidConfigurationAndBoundaries()
{
    ManualClock clock;
    RecordingPacingPolicy pacing(1);

    // A player without a pump cannot play and reports why.
    Player noPump(nullptr, &clock, &pacing);
    int errors = 0;
    QObject::connect(&noPump, &Player::errorOccurred,
                     [&errors](const QString &) { ++errors; });
    noPump.play();
    QVERIFY(noPump.isStopped());
    QCOMPARE(errors, 1);
    QCOMPARE(noPump.tick(), 0);
    QVERIFY(!noPump.stepOnce());

    FakeFrameSource source;
    source.appendFrame(playerTestFrame(0));
    FramePump pump;
    pump.setSource(&source);
    Player player(&pump, &clock, &pacing);

    // Invalid frame intervals are rejected without changing the interval.
    QVERIFY(!player.setFrameIntervalMs(0));
    QVERIFY(!player.setFrameIntervalMs(-10));
    QCOMPARE(player.frameIntervalMs(), qint64(40));
    QVERIFY(player.setFrameIntervalMs(33));
    QCOMPARE(player.frameIntervalMs(), qint64(33));

    // A null pacing policy is ignored rather than replacing the current one.
    PacingPolicy *before = player.pacingPolicy();
    player.setPacingPolicy(nullptr);
    QVERIFY(player.pacingPolicy() == before);
}

void ProjectTest::playerTickIsRepeatableWithoutWallClockDelays()
{
    const auto runSequence = [](int *framesOut, qint64 *positionOut, int *signalsOut) {
        FakeFrameSource source;
        for (int i = 0; i < 6; ++i) {
            source.appendFrame(playerTestFrame(i));
        }
        FramePump pump;
        pump.setSource(&source);
        ManualClock clock;
        DefaultPacingPolicy pacing;
        Player player(&pump, &clock, &pacing);
        player.setFrameIntervalMs(30);

        int signalCount = 0;
        QObject::connect(&player, &Player::framePresented,
                         [&signalCount](const QImage &, qint64, qint64) { ++signalCount; });

        player.play();
        int total = 0;
        for (int step = 0; step < 5; ++step) {
            clock.advance(17);
            total += player.tick();
        }
        *framesOut = total;
        *positionOut = player.positionMs();
        *signalsOut = signalCount;
    };

    int framesA = 0;
    int framesB = 0;
    int signalsA = 0;
    int signalsB = 0;
    qint64 positionA = 0;
    qint64 positionB = 0;
    runSequence(&framesA, &positionA, &signalsA);
    runSequence(&framesB, &positionB, &signalsB);

    QCOMPARE(framesA, framesB);
    QCOMPARE(positionA, positionB);
    QCOMPARE(signalsA, signalsB);
    // 17 ms ticks at a 30 ms interval present exactly two frames over five ticks.
    QCOMPARE(framesA, 2);
    QCOMPARE(signalsA, 2);
}

// ======================= 360 reframing engine (Phase 4) =======================
// The reframing engine separates AI/creator decisions (ReframeIntent ->
// ReframePlan) from deterministic execution (CameraPath + ReframeRenderer), so
// the camera path, projection, and rendering can be verified without any
// network, model, or nondeterministic input.

void ProjectTest::reframeCameraKeyframeJsonRoundTrip()
{
    CameraKeyframe frame;
    frame.timeMs = 1500;
    frame.yawDeg = 45.5;
    frame.pitchDeg = -12.25;
    frame.rollDeg = 3.0;
    frame.fieldOfViewDeg = 72.0;
    frame.interpolation = CameraKeyframe::Interpolation::Hold;

    const QJsonObject object = frame.toJsonObject();
    CameraKeyframe restored;
    QString error;
    QVERIFY(CameraKeyframe::readFromJsonObject(object, &restored, &error));
    QCOMPARE(restored.timeMs, frame.timeMs);
    QVERIFY(qAbs(restored.yawDeg - frame.yawDeg) < 1e-9);
    QVERIFY(qAbs(restored.pitchDeg - frame.pitchDeg) < 1e-9);
    QVERIFY(qAbs(restored.rollDeg - frame.rollDeg) < 1e-9);
    QVERIFY(qAbs(restored.fieldOfViewDeg - frame.fieldOfViewDeg) < 1e-9);
    QVERIFY(restored.interpolation == CameraKeyframe::Interpolation::Hold);
}

void ProjectTest::reframeCameraKeyframeRejectsInvalid()
{
    CameraKeyframe out;
    QString error;
    QVERIFY(!CameraKeyframe::readFromJsonObject(QJsonObject(), &out, &error));
    QVERIFY(!error.isEmpty());

    QJsonObject missing = CameraKeyframe().toJsonObject();
    missing.remove(QStringLiteral("yawDeg"));
    QVERIFY(!CameraKeyframe::readFromJsonObject(missing, &out, &error));

    QJsonObject badPitch = CameraKeyframe().toJsonObject();
    badPitch.insert(QStringLiteral("pitchDeg"), 120.0);
    QVERIFY(!CameraKeyframe::readFromJsonObject(badPitch, &out, &error));

    QJsonObject badFov = CameraKeyframe().toJsonObject();
    badFov.insert(QStringLiteral("fieldOfViewDeg"), 5.0);
    QVERIFY(!CameraKeyframe::readFromJsonObject(badFov, &out, &error));

    QJsonObject badInterp = CameraKeyframe().toJsonObject();
    badInterp.insert(QStringLiteral("interpolation"), QStringLiteral("bounce"));
    QVERIFY(!CameraKeyframe::readFromJsonObject(badInterp, &out, &error));
}

void ProjectTest::reframePlanJsonRoundTrip()
{
    const ReframePlan plan = makeReframePlan(
        1000, 4000, 640, 360, 2.0,
        { makeKeyframe(1000, 0.0),
          makeKeyframe(4000, 90.0, 0.0, 0.0, 90.0,
                       CameraKeyframe::Interpolation::Hold) });
    ReframePlan withId = plan;
    withId.setSourceMediaId(QStringLiteral("media-1"));

    const QJsonObject object = withId.toJsonObject();
    ReframePlan restored;
    QString error;
    QVERIFY(ReframePlan::readFromJsonObject(object, &restored, &error));
    QCOMPARE(restored.sourceMediaId(), QStringLiteral("media-1"));
    QCOMPARE(restored.sourceRange().startMs, qint64(1000));
    QCOMPARE(restored.sourceRange().endMs, qint64(4000));
    QCOMPARE(restored.output().width, 640);
    QCOMPARE(restored.output().height, 360);
    QVERIFY(qAbs(restored.output().fps - 2.0) < 1e-9);
    QCOMPARE(restored.keyframes().size(), 2);
    QVERIFY(restored.keyframes().at(1).interpolation
            == CameraKeyframe::Interpolation::Hold);
    QVERIFY(restored.toJsonObject() == object);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("plan.json"));
    QVERIFY(withId.save(path, &error));
    bool ok = false;
    const ReframePlan loaded = ReframePlan::load(path, &ok, &error);
    QVERIFY(ok);
    QVERIFY(loaded.toJsonObject() == object);
}

void ProjectTest::reframePlanRejectsInvalid()
{
    QString error;

    ReframePlan empty;
    QVERIFY(!empty.isValid(&error));
    QVERIFY(!error.isEmpty());

    ReframePlan outOfRange = makeReframePlan(
        0, 1000, 320, 180, 1.0, { makeKeyframe(2000, 0.0) });
    QVERIFY(!outOfRange.isValid(&error));

    ReframePlan unsorted = makeReframePlan(
        0, 1000, 320, 180, 1.0,
        { makeKeyframe(800, 0.0), makeKeyframe(200, 0.0) });
    QVERIFY(!unsorted.isValid(&error));

    ReframePlan duplicate = makeReframePlan(
        0, 1000, 320, 180, 1.0,
        { makeKeyframe(500, 0.0), makeKeyframe(500, 10.0) });
    QVERIFY(!duplicate.isValid(&error));

    ReframePlan badOutput = makeReframePlan(
        0, 1000, 0, 180, 1.0, { makeKeyframe(0, 0.0) });
    QVERIFY(!badOutput.isValid(&error));

    ReframePlan tooShort = makeReframePlan(
        0, 10, 320, 180, 1.0, { makeKeyframe(0, 0.0) });
    QVERIFY(!tooShort.isValid(&error));
}

void ProjectTest::reframePlanFrameTimingIsDeterministic()
{
    const ReframePlan plan = makeReframePlan(
        1000, 3500, 320, 180, 2.0, { makeKeyframe(1000, 0.0) });
    QCOMPARE(plan.frameCount(), 5);
    QCOMPARE(plan.frameTimeMs(0), qint64(1000));
    QCOMPARE(plan.frameTimeMs(1), qint64(1500));
    QCOMPARE(plan.frameTimeMs(4), qint64(3000));

    const ReframePlan thirty = makeReframePlan(
        0, 1000, 320, 180, 30.0, { makeKeyframe(0, 0.0) });
    QCOMPARE(thirty.frameCount(), 30);
    QCOMPARE(thirty.frameTimeMs(1), qint64(33));
}

void ProjectTest::cameraPathHoldsOutsideKeyframes()
{
    const ReframePlan plan = makeReframePlan(
        0, 4000, 320, 180, 1.0,
        { makeKeyframe(1000, 30.0, 10.0), makeKeyframe(3000, 90.0, 20.0) });

    const CameraState before = CameraPath::stateAt(plan, 0);
    QVERIFY(qAbs(before.yawDeg - 30.0) < 1e-9);
    QVERIFY(qAbs(before.pitchDeg - 10.0) < 1e-9);

    const CameraState after = CameraPath::stateAt(plan, 5000);
    QVERIFY(qAbs(after.yawDeg - 90.0) < 1e-9);
    QVERIFY(qAbs(after.pitchDeg - 20.0) < 1e-9);
}

void ProjectTest::cameraPathInterpolatesLinearly()
{
    const ReframePlan plan = makeReframePlan(
        0, 2000, 320, 180, 1.0,
        { makeKeyframe(0, 0.0, 0.0, 0.0, 90.0),
          makeKeyframe(2000, 40.0, 20.0, 0.0, 50.0) });

    const CameraState mid = CameraPath::stateAt(plan, 1000);
    QVERIFY(qAbs(mid.yawDeg - 20.0) < 1e-9);
    QVERIFY(qAbs(mid.pitchDeg - 10.0) < 1e-9);
    QVERIFY(qAbs(mid.fieldOfViewDeg - 70.0) < 1e-9);
}

void ProjectTest::cameraPathUsesShortestYawPath()
{
    // 170 -> -170 is a +20 shortest path, not a -340 long way round.
    const ReframePlan plan = makeReframePlan(
        0, 2000, 320, 180, 1.0,
        { makeKeyframe(0, 170.0), makeKeyframe(2000, -170.0) });
    const CameraState mid = CameraPath::stateAt(plan, 1000);
    QVERIFY(qAbs(qAbs(mid.yawDeg) - 180.0) < 1e-6);

    QVERIFY(qAbs(CameraPath::shortestYawDelta(170.0, -170.0) - 20.0) < 1e-9);
    QVERIFY(qAbs(CameraPath::shortestYawDelta(-170.0, 170.0) + 20.0) < 1e-9);
}

void ProjectTest::cameraPathHonorsHoldInterpolation()
{
    const ReframePlan plan = makeReframePlan(
        0, 2000, 320, 180, 1.0,
        { makeKeyframe(0, 10.0, 0.0, 0.0, 90.0,
                       CameraKeyframe::Interpolation::Hold),
          makeKeyframe(2000, 90.0) });

    const CameraState mid = CameraPath::stateAt(plan, 1000);
    QVERIFY(qAbs(mid.yawDeg - 10.0) < 1e-9);
    const CameraState atEnd = CameraPath::stateAt(plan, 2000);
    QVERIFY(qAbs(atEnd.yawDeg - 90.0) < 1e-9);
}

void ProjectTest::cameraPathNormalizesAndClamps()
{
    const ReframePlan plan = makeReframePlan(
        0, 1000, 320, 180, 1.0,
        { makeKeyframe(0, 400.0, 200.0, 400.0, 500.0) });
    const CameraState state = CameraPath::stateAt(plan, 0);
    QVERIFY(qAbs(state.yawDeg - 40.0) < 1e-9);
    QVERIFY(qAbs(state.pitchDeg - 90.0) < 1e-9);
    QVERIFY(qAbs(state.rollDeg - 40.0) < 1e-9);
    QVERIFY(qAbs(state.fieldOfViewDeg - 140.0) < 1e-9);
}

void ProjectTest::reframeRendererRendersDeterministicFrames()
{
    const ReframePlan plan = makeReframePlan(
        0, 1000, 160, 90, 2.0, { makeKeyframe(0, 0.0) });
    SyntheticEquirectProvider provider;

    QList<QImage> frames;
    int count = 0;
    QString error;
    QVERIFY(ReframeRenderer::render(
        plan, &provider,
        [&frames](int, qint64, const QImage &frame) {
            frames.append(frame);
            return true;
        },
        &count, &error));
    QCOMPARE(count, 2);
    QCOMPARE(frames.size(), 2);
    QCOMPARE(frames.at(0).size(), QSize(160, 90));
    QVERIFY(frames.at(0).pixelColor(80, 45) == kFrontColor);

    QList<QImage> again;
    QVERIFY(ReframeRenderer::render(
        plan, &provider,
        [&again](int, qint64, const QImage &frame) {
            again.append(frame);
            return true;
        },
        nullptr, &error));
    QVERIFY(imagesIdentical(frames.at(0), again.at(0)));
    QVERIFY(imagesIdentical(frames.at(1), again.at(1)));
}

void ProjectTest::reframeRendererFollowsCameraPath()
{
    // Pan from front (yaw 0) to the right (yaw 90); the last output frame
    // lands exactly on the second keyframe.
    const ReframePlan plan = makeReframePlan(
        0, 1500, 160, 90, 2.0,
        { makeKeyframe(0, 0.0), makeKeyframe(1000, 90.0) });
    SyntheticEquirectProvider provider;

    QList<QImage> frames;
    QVERIFY(ReframeRenderer::render(
        plan, &provider,
        [&frames](int, qint64, const QImage &frame) {
            frames.append(frame);
            return true;
        },
        nullptr, nullptr));
    QCOMPARE(frames.size(), 3);
    QVERIFY(frames.at(0).pixelColor(80, 45) == kFrontColor);
    QVERIFY(frames.at(2).pixelColor(80, 45) == kRightColor);
}

void ProjectTest::reframeRendererRejectsInvalidInputs()
{
    ReframePlan invalid;
    SyntheticEquirectProvider provider;
    QString error;
    int count = -1;
    QVERIFY(!ReframeRenderer::render(
        invalid, &provider,
        [](int, qint64, const QImage &) { return true; }, &count, &error));
    QVERIFY(!error.isEmpty());

    const ReframePlan plan = makeReframePlan(
        0, 1000, 64, 64, 1.0, { makeKeyframe(0, 0.0) });
    QVERIFY(!ReframeRenderer::render(
        plan, nullptr,
        [](int, qint64, const QImage &) { return true; }, nullptr, &error));
    QVERIFY(!ReframeRenderer::render(
        plan, &provider, ReframeRenderer::FrameSink(), nullptr, &error));
}

void ProjectTest::reframeRendererReportsProviderFailure()
{
    const ReframePlan plan = makeReframePlan(
        0, 1500, 64, 64, 2.0, { makeKeyframe(0, 0.0) });
    SyntheticEquirectProvider provider;
    provider.setFailAfterCalls(1);

    int count = -1;
    QString error;
    QVERIFY(!ReframeRenderer::render(
        plan, &provider,
        [](int, qint64, const QImage &) { return true; }, &count, &error));
    QVERIFY(error.contains(QStringLiteral("synthetic provider failure")));
    QCOMPARE(provider.calls(), 2);
}

void ProjectTest::reframeRendererPngSequenceAndEncode()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg is unavailable in this environment");
    }
    const ReframePlan plan = makeReframePlan(
        0, 1500, 160, 90, 2.0,
        { makeKeyframe(0, 0.0), makeKeyframe(1000, 90.0) });
    SyntheticEquirectProvider provider;

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString framesDir = dir.filePath(QStringLiteral("frames"));
    QStringList paths;
    QString error;
    QVERIFY(ReframeRenderer::renderToPngSequence(plan, &provider, framesDir,
                                                 &paths, &error));
    QCOMPARE(paths.size(), 3);
    QVERIFY(QFileInfo::exists(paths.at(0)));

    const QString pattern =
        QDir(framesDir).filePath(ReframeRenderer::frameFileNamePattern());
    const QString output = dir.filePath(QStringLiteral("out.mp4"));
    QVERIFY(ReframeRenderer::encodeVideo(
        FrameExtractor::defaultExecutablePath(), pattern, 2.0, output, &error));
    QVERIFY(QFileInfo::exists(output));
    QVERIFY(QFileInfo(output).size() > 0);

    QImage decoded;
    QVERIFY(FrameExtractor::extractFirstFrame(
        output, FrameExtractor::defaultExecutablePath(), &decoded, &error));
    QCOMPARE(decoded.size(), QSize(160, 90));
}

void ProjectTest::reframeIntentParsesAspectAndPlatform()
{
    const ReframeIntent wide = ReframeIntentParser::parse(
        QStringLiteral("Make a normal 16:9 video from this 360 footage."));
    QVERIFY(wide.hasOutput);
    QCOMPARE(wide.outputWidth, 1920);
    QCOMPARE(wide.outputHeight, 1080);

    const ReframeIntent tiktok = ReframeIntentParser::parse(
        QStringLiteral("Make a TikTok version"));
    QVERIFY(tiktok.hasOutput);
    QCOMPARE(tiktok.outputWidth, 1080);
    QCOMPARE(tiktok.outputHeight, 1920);

    const ReframeIntent square = ReframeIntentParser::parse(
        QStringLiteral("give me a square cut"));
    QVERIFY(square.hasOutput);
    QCOMPARE(square.outputWidth, square.outputHeight);
}

void ProjectTest::reframeIntentParsesTimeRanges()
{
    const ReframeIntent a = ReframeIntentParser::parse(
        QStringLiteral("Use this section from 00:30 to 01:00."));
    QVERIFY(a.hasTimeRange);
    QCOMPARE(a.startMs, qint64(30000));
    QCOMPARE(a.endMs, qint64(60000));

    const ReframeIntent b = ReframeIntentParser::parse(
        QStringLiteral("From 00:35 to 01:10, follow the speaker."));
    QVERIFY(b.hasTimeRange);
    QCOMPARE(b.startMs, qint64(35000));
    QCOMPARE(b.endMs, qint64(70000));

    const ReframeIntent c = ReframeIntentParser::parse(
        QStringLiteral("start at 00:10 for 5 seconds"));
    QVERIFY(c.hasTimeRange);
    QCOMPARE(c.startMs, qint64(10000));
    QCOMPARE(c.endMs, qint64(15000));

    const ReframeIntent d = ReframeIntentParser::parse(
        QStringLiteral("use 1:02:03 to 1:02:05"));
    QVERIFY(d.hasTimeRange);
    QCOMPARE(d.startMs, qint64(3723000));
    QCOMPARE(d.endMs, qint64(3725000));
}

void ProjectTest::reframeIntentParsesNamedDirections()
{
    const ReframeIntent a = ReframeIntentParser::parse(
        QStringLiteral("Start facing forward, then pan toward the person on my right."));
    QCOMPARE(a.moves.size(), 2);
    QVERIFY(a.moves.at(0).hasDirection);
    QVERIFY(qAbs(a.moves.at(0).yawDeg - 0.0) < 1e-9);
    QVERIFY(a.moves.at(1).hasDirection);
    QVERIFY(qAbs(a.moves.at(1).yawDeg - 90.0) < 1e-9);
    QVERIFY(a.unresolvedTargets.isEmpty());

    const ReframeIntent b = ReframeIntentParser::parse(
        QStringLiteral("look left then look behind"));
    QCOMPARE(b.moves.size(), 2);
    QVERIFY(qAbs(b.moves.at(0).yawDeg + 90.0) < 1e-9);
    QVERIFY(qAbs(b.moves.at(1).yawDeg - 180.0) < 1e-9);

    const ReframeIntent c = ReframeIntentParser::parse(
        QStringLiteral("look up"));
    QCOMPARE(c.moves.size(), 1);
    QVERIFY(c.moves.at(0).hasDirection);
    QVERIFY(qAbs(c.moves.at(0).pitchDeg - 30.0) < 1e-9);
}

void ProjectTest::reframeIntentParsesSubjectReferencesAsUnresolved()
{
    const ReframeIntent a = ReframeIntentParser::parse(
        QStringLiteral("Start looking at the car, then move to me."));
    QCOMPARE(a.moves.size(), 2);
    QVERIFY(!a.moves.at(0).hasDirection);
    QCOMPARE(a.moves.at(0).targetRef, QStringLiteral("car"));
    QCOMPARE(a.moves.at(1).targetRef, QStringLiteral("me"));
    QCOMPARE(a.unresolvedTargets.size(), 2);
    QVERIFY(a.notes.join(QStringLiteral("\n"))
                .contains(QStringLiteral("Unresolved")));
}

void ProjectTest::reframeIntentHandlesGarbage()
{
    const ReframeIntent empty = ReframeIntentParser::parse(QString());
    QVERIFY(!empty.recognized);
    QVERIFY(!empty.notes.isEmpty());

    const ReframeIntent garbage = ReframeIntentParser::parse(
        QStringLiteral("qqqq zzzz"));
    QVERIFY(!garbage.recognized);

    const ReframeIntent centered = ReframeIntentParser::parse(
        QStringLiteral("keep me centered"));
    QVERIFY(centered.recognized);
    QCOMPARE(centered.moves.size(), 1);
    QCOMPARE(centered.moves.at(0).targetRef, QStringLiteral("me"));
}

void ProjectTest::reframeBuilderBuildsStaticPlan()
{
    const ReframeIntent intent = ReframeIntentParser::parse(
        QStringLiteral("Make a normal 16:9 video"));
    const ReframeBuildResult result = ReframePlanBuilder::build(
        intent, {}, ReframePlan::TimeRange{ 0, 3000 },
        ReframePlan::OutputSpec{ 1920, 1080, 30.0 });
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.plan.keyframes().size(), 1);
    QCOMPARE(result.plan.output().width, 1920);
    QVERIFY(qAbs(result.plan.keyframes().at(0).yawDeg) < 1e-9);
}

void ProjectTest::reframeBuilderBuildsTwoStopPath()
{
    const ReframeIntent intent = ReframeIntentParser::parse(
        QStringLiteral("Start facing forward, then pan toward the person on my right."));
    const ReframeBuildResult result = ReframePlanBuilder::build(
        intent, {}, ReframePlan::TimeRange{ 0, 2000 },
        ReframePlan::OutputSpec{ 640, 360, 2.0 });
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.plan.keyframes().size(), 2);
    QCOMPARE(result.plan.keyframes().at(0).timeMs, qint64(0));
    QCOMPARE(result.plan.keyframes().at(1).timeMs, qint64(2000));
    QVERIFY(qAbs(result.plan.keyframes().at(1).yawDeg - 90.0) < 1e-9);
}

void ProjectTest::reframeBuilderResolvesTargetsCaseInsensitively()
{
    const ReframeIntent intent = ReframeIntentParser::parse(
        QStringLiteral("Start looking at the car, then move to me."));
    QList<ReframeTarget> targets;
    targets.append(ReframeTarget{ QStringLiteral("CAR"), -45.0, 5.0 });
    targets.append(ReframeTarget{ QStringLiteral("Me"), 30.0, -2.0 });

    const ReframeBuildResult result = ReframePlanBuilder::build(
        intent, targets, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 320, 180, 1.0 });
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.plan.keyframes().size(), 2);
    QVERIFY(qAbs(result.plan.keyframes().at(0).yawDeg + 45.0) < 1e-9);
    QVERIFY(qAbs(result.plan.keyframes().at(1).yawDeg - 30.0) < 1e-9);
    QVERIFY(qAbs(result.plan.keyframes().at(1).pitchDeg + 2.0) < 1e-9);
}

void ProjectTest::reframeBuilderRejectsUnresolvedTargets()
{
    const ReframeIntent intent = ReframeIntentParser::parse(
        QStringLiteral("Follow the speaker."));
    const ReframeBuildResult result = ReframePlanBuilder::build(
        intent, {}, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 320, 180, 1.0 });
    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(QStringLiteral("Unresolved target")));
}

void ProjectTest::reframeBuilderHonorsIntentTimeRangeAndOutput()
{
    const ReframeIntent intent = ReframeIntentParser::parse(
        QStringLiteral("From 00:35 to 01:10, make a TikTok version."));
    QVERIFY(intent.hasTimeRange);
    QVERIFY(intent.hasOutput);

    const ReframeBuildResult result = ReframePlanBuilder::build(
        intent, {}, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 320, 180, 1.0 });
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.plan.sourceRange().startMs, qint64(35000));
    QCOMPARE(result.plan.sourceRange().endMs, qint64(70000));
    QCOMPARE(result.plan.output().width, 1080);
    QCOMPARE(result.plan.output().height, 1920);
}

void ProjectTest::reframePipelineRendersRealVideoEndToEnd()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg is unavailable in this environment");
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString sourcePath;
    QVERIFY(createEquirectReviewVideo(
        dir.path(), FrameExtractor::defaultExecutablePath(), 4, &sourcePath));

    ReframePipeline::Request request;
    request.sourcePath = sourcePath;
    request.sourceMediaId = QStringLiteral("review-media");
    request.instruction = QStringLiteral(
        "Start facing forward, then pan toward the person on my right.");
    request.outputPath = dir.filePath(QStringLiteral("reframed.mp4"));
    request.defaultRange = ReframePlan::TimeRange{ 0, 3000 };
    request.defaultOutput = ReframePlan::OutputSpec{ 160, 90, 2.0 };

    const ReframePipeline::Result result = ReframePipeline::run(request);
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.frameCount, 6);
    QCOMPARE(result.plan.sourceMediaId(), QStringLiteral("review-media"));
    QVERIFY(QFileInfo::exists(result.outputPath));
    QVERIFY(QFileInfo(result.outputPath).size() > 0);

    QImage decoded;
    QString error;
    QVERIFY(FrameExtractor::extractFirstFrame(
        result.outputPath, FrameExtractor::defaultExecutablePath(), &decoded,
        &error));
    QCOMPARE(decoded.size(), QSize(160, 90));
}

// ==================== 360 target resolution (Phase 4, Obj 2) ====================
// Deterministic infrastructure tests plus a model-free end-to-end path. No
// model is downloaded and no CV dependency is linked; the replaceable
// TargetDetector seam is exercised with a synthetic color detector and a
// subprocess helper.

namespace {

TargetObservation makeTargetObservation(
    qint64 timeMs, double yawDeg, double pitchDeg,
    const QString &label = QStringLiteral("person"), double confidence = 0.9,
    const QString &targetId = QString())
{
    TargetObservation observation;
    observation.timeMs = timeMs;
    observation.yawDeg = yawDeg;
    observation.pitchDeg = pitchDeg;
    observation.label = label;
    observation.confidence = confidence;
    observation.targetId = targetId;
    return observation;
}

TargetResolveConfig smallResolverConfig()
{
    TargetResolveConfig config;
    config.viewPlan.fieldOfViewDeg = 75.0;
    config.viewPlan.yawCount = 6;
    config.viewPlan.pitchCount = 3;
    config.viewPlan.viewWidth = 160;
    config.viewPlan.viewHeight = 120;
    config.minConfidence = 0.3;
    return config;
}

} // namespace

void ProjectTest::equirectDirectionFromCenterAndSides()
{
    const int width = 360;
    const int height = 180;
    const SphericalDirection center = EquirectProjection::directionFromEquirectPixel(
        179.5, 89.5, width, height);
    QVERIFY(qAbs(center.yawDeg) < 1e-6);
    QVERIFY(qAbs(center.pitchDeg) < 1e-6);

    const SphericalDirection right = EquirectProjection::directionFromEquirectPixel(
        269.5, 89.5, width, height);
    QVERIFY(qAbs(right.yawDeg - 90.0) < 1e-6);

    const SphericalDirection left = EquirectProjection::directionFromEquirectPixel(
        89.5, 89.5, width, height);
    QVERIFY(qAbs(left.yawDeg + 90.0) < 1e-6);

    const SphericalDirection top = EquirectProjection::directionFromEquirectPixel(
        179.5, -0.5, width, height);
    QVERIFY(qAbs(top.pitchDeg - 90.0) < 1e-6);
}

void ProjectTest::equirectPixelRoundTrip()
{
    const int width = 720;
    const int height = 360;
    const double yaws[] = { -170.0, -90.0, -45.0, 0.0, 45.0, 90.0, 170.0 };
    const double pitches[] = { -80.0, -30.0, 0.0, 30.0, 80.0 };
    for (double yaw : yaws) {
        for (double pitch : pitches) {
            QPointF pixel;
            QVERIFY(EquirectProjection::equirectPixelFromDirection(yaw, pitch, width,
                                                                    height, &pixel));
            const SphericalDirection back = EquirectProjection::directionFromEquirectPixel(
                pixel.x(), pixel.y(), width, height);
            const SphericalDirection expected{ yaw, pitch };
            QVERIFY(EquirectProjection::angularDistanceDeg(expected, back) < 0.5);
        }
    }
}

void ProjectTest::equirectAngularDistanceHandlesSeam()
{
    QVERIFY(qAbs(EquirectProjection::angularDistanceDeg({ 179.0, 0.0 }, { -179.0, 0.0 })
                 - 2.0) < 1e-6);
    QVERIFY(qAbs(EquirectProjection::angularDistanceDeg({ 179.5, 0.0 }, { -179.5, 0.0 })
                 - 1.0) < 1e-6);
    QVERIFY(qAbs(EquirectProjection::angularDistanceDeg({ 10.0, 5.0 }, { 10.0, 5.0 }))
            < 1e-9);
    QVERIFY(qAbs(EquirectProjection::angularDistanceDeg({ 0.0, 0.0 }, { 0.0, 10.0 })
                 - 10.0) < 1e-6);
    QVERIFY(qAbs(EquirectProjection::shortestYawDeltaDeg(179.0, -179.0) - 2.0) < 1e-6);
    QVERIFY(qAbs(EquirectProjection::shortestYawDeltaDeg(-179.0, 179.0) + 2.0) < 1e-6);
}

void ProjectTest::equirectPitchClampAndValidity()
{
    QVERIFY(qAbs(EquirectProjection::clampPitchDeg(120.0) - 90.0) < 1e-9);
    QVERIFY(qAbs(EquirectProjection::clampPitchDeg(-120.0) + 90.0) < 1e-9);
    QVERIFY(EquirectProjection::isValidDirection(0.0, 90.0));
    QVERIFY(!EquirectProjection::isValidDirection(0.0, 90.5));
    QVERIFY(!EquirectProjection::isValidDirection(std::nan(""), 0.0));
    QVERIFY(qAbs(EquirectProjection::normalizeYawDeg(190.0) + 170.0) < 1e-9);
    QVERIFY(qAbs(EquirectProjection::normalizeYawDeg(-190.0) - 170.0) < 1e-9);
}

void ProjectTest::equirectViewCenterMatchesDirection()
{
    const PerspectiveView view{ 40.0, 10.0, 75.0, 320, 240 };
    const SphericalDirection center =
        EquirectProjection::directionFromViewPixel(view, 159.5, 119.5);
    QVERIFY(EquirectProjection::angularDistanceDeg(center, { 40.0, 10.0 }) < 1e-6);
}

void ProjectTest::equirectViewDirectionRoundTrip()
{
    const PerspectiveView view{ 30.0, -20.0, 75.0, 320, 240 };
    const double yaws[] = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    const double pitches[] = { -30.0, -20.0, -10.0 };
    for (double yaw : yaws) {
        for (double pitch : pitches) {
            QPointF pixel;
            QVERIFY(EquirectProjection::viewPixelFromDirection(view, yaw, pitch, &pixel));
            const SphericalDirection back =
                EquirectProjection::directionFromViewPixel(view, pixel.x(), pixel.y());
            QVERIFY(EquirectProjection::angularDistanceDeg({ yaw, pitch }, back) < 1e-4);
        }
    }
}

void ProjectTest::equirectViewRejectsBehindCamera()
{
    const PerspectiveView view{ 0.0, 0.0, 75.0, 320, 240 };
    QVERIFY(EquirectProjection::isDirectionInView(view, 0.0, 0.0));
    QVERIFY(!EquirectProjection::isDirectionInView(view, 180.0, 0.0));
    QPointF pixel;
    QVERIFY(!EquirectProjection::viewPixelFromDirection(view, 180.0, 0.0, &pixel));
}

void ProjectTest::equirectDetectionToDirectionMapsBox()
{
    const PerspectiveView view{ 0.0, 0.0, 90.0, 320, 180 };
    SphericalDirection center;
    double yawRadius = 0.0;
    double pitchRadius = 0.0;
    QVERIFY(EquirectProjection::detectionToDirection(
        view, QRectF(150.0, 80.0, 20.0, 20.0), &center, &yawRadius, &pitchRadius));
    QVERIFY(EquirectProjection::angularDistanceDeg(center, { 0.0, 0.0 }) < 2.0);
    QVERIFY(yawRadius > 0.0);
    QVERIFY(pitchRadius > 0.0);
    QVERIFY(yawRadius < 90.0);
    QVERIFY(pitchRadius < 90.0);
}

void ProjectTest::equirectDetectionRejectsInvalidBox()
{
    const PerspectiveView view{ 0.0, 0.0, 90.0, 320, 180 };
    SphericalDirection center;
    double yawRadius = 0.0;
    double pitchRadius = 0.0;
    QVERIFY(!EquirectProjection::detectionToDirection(view, QRectF(), &center,
                                                      &yawRadius, &pitchRadius));
    QVERIFY(!EquirectProjection::detectionToDirection(
        view, QRectF(10.0, 10.0, -5.0, 10.0), &center, &yawRadius, &pitchRadius));
}

void ProjectTest::equirectViewPlanCoversSphere()
{
    EquirectViewPlan::Config config;
    config.fieldOfViewDeg = 75.0;
    config.yawCount = 6;
    config.pitchCount = 3;
    config.viewWidth = 160;
    config.viewHeight = 120;
    const QList<PerspectiveView> views = EquirectViewPlan::coveringViews(config);
    QVERIFY(views.size() >= 18);

    const double pitches[] = { -85.0, -75.0, -45.0, 0.0, 45.0, 75.0, 85.0 };
    for (double pitch : pitches) {
        for (double yaw = -180.0; yaw < 180.0; yaw += 15.0) {
            QVERIFY2(EquirectViewPlan::covers(views, yaw, pitch),
                     qPrintable(QStringLiteral("uncovered yaw=%1 pitch=%2")
                                    .arg(yaw).arg(pitch)));
        }
    }
}

void ProjectTest::equirectViewPlanIsDeterministic()
{
    EquirectViewPlan::Config config;
    config.fieldOfViewDeg = 75.0;
    config.yawCount = 5;
    config.pitchCount = 4;
    config.viewWidth = 160;
    config.viewHeight = 120;
    const QList<PerspectiveView> a = EquirectViewPlan::coveringViews(config);
    const QList<PerspectiveView> b = EquirectViewPlan::coveringViews(config);
    QCOMPARE(a.size(), b.size());
    for (int i = 0; i < a.size(); ++i) {
        QVERIFY(qAbs(a.at(i).yawDeg - b.at(i).yawDeg) < 1e-12);
        QVERIFY(qAbs(a.at(i).pitchDeg - b.at(i).pitchDeg) < 1e-12);
        QCOMPARE(a.at(i).width, b.at(i).width);
        QCOMPARE(a.at(i).height, b.at(i).height);
    }
}

void ProjectTest::equirectViewPlanAddsPolarViews()
{
    EquirectViewPlan::Config config;
    config.fieldOfViewDeg = 75.0;
    config.yawCount = 4;
    config.pitchCount = 1;
    config.viewWidth = 160;
    config.viewHeight = 120;
    const QList<PerspectiveView> views = EquirectViewPlan::coveringViews(config);
    QVERIFY(EquirectViewPlan::covers(views, 0.0, 89.0));
    QVERIFY(EquirectViewPlan::covers(views, 0.0, -89.0));
    QVERIFY(EquirectViewPlan::covers(views, 123.0, 85.0));
}

void ProjectTest::targetTrackerCreatesTrackFromDetection()
{
    SphericalTargetTracker tracker;
    const QList<TargetObservation> assigned =
        tracker.update({ makeTargetObservation(0, 10.0, 0.0) }, 0);
    QCOMPARE(assigned.size(), 1);
    QCOMPARE(assigned.at(0).targetId, QStringLiteral("t1"));
    QCOMPARE(tracker.tracks().size(), 1);
    QCOMPARE(tracker.tracks().at(0).size(), 1);
    QCOMPARE(tracker.tracks().at(0).label(), QStringLiteral("person"));
}

void ProjectTest::targetTrackerPersistsIdentityAcrossFrames()
{
    SphericalTargetTracker tracker;
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    const QList<TargetObservation> second =
        tracker.update({ makeTargetObservation(500, 5.0, 0.0) }, 500);
    QCOMPARE(second.size(), 1);
    QCOMPARE(second.at(0).targetId, QStringLiteral("t1"));
    QCOMPARE(tracker.tracks().size(), 1);
    QCOMPARE(tracker.tracks().at(0).size(), 2);
}

void ProjectTest::targetTrackerSeparatesDistinctTargets()
{
    SphericalTargetTracker tracker;
    tracker.update({ makeTargetObservation(0, -60.0, 0.0),
                     makeTargetObservation(0, 60.0, 0.0) }, 0);
    QCOMPARE(tracker.tracks().size(), 2);
    QVERIFY(tracker.trackById(QStringLiteral("t1")) != nullptr);
    QVERIFY(tracker.trackById(QStringLiteral("t2")) != nullptr);
}

void ProjectTest::targetTrackerMergesNearDuplicates()
{
    QList<TargetObservation> observations = {
        makeTargetObservation(0, 10.0, 0.0, QStringLiteral("person"), 0.8),
        makeTargetObservation(0, 12.0, 0.0, QStringLiteral("person"), 0.9)
    };
    QList<TargetObservation> merged =
        SphericalTargetTracker::mergeNearDuplicates(observations, 8.0);
    QCOMPARE(merged.size(), 1);
    QVERIFY(qAbs(merged.at(0).confidence - 0.9) < 1e-9);

    std::reverse(observations.begin(), observations.end());
    merged = SphericalTargetTracker::mergeNearDuplicates(observations, 8.0);
    QCOMPARE(merged.size(), 1);
    QVERIFY(qAbs(merged.at(0).confidence - 0.9) < 1e-9);
}

void ProjectTest::targetTrackerGreedyPrefersNearest()
{
    SphericalTargetTracker tracker;
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    tracker.update({ makeTargetObservation(500, 12.0, 0.0),
                     makeTargetObservation(500, 2.0, 0.0) }, 500);
    QCOMPARE(tracker.tracks().size(), 2);
    const TargetTrack *track = tracker.trackById(QStringLiteral("t1"));
    QVERIFY(track != nullptr);
    QVERIFY(qAbs(track->observations().last().yawDeg - 2.0) < 1e-9);
}

void ProjectTest::targetTrackerGateCreatesNewTrack()
{
    SphericalTargetTracker tracker;
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    tracker.update({ makeTargetObservation(500, 80.0, 0.0) }, 500);
    QCOMPARE(tracker.tracks().size(), 2);
    const TargetTrack *first = tracker.trackById(QStringLiteral("t1"));
    QVERIFY(first != nullptr);
    QCOMPARE(first->missCount(), 1);
}

void ProjectTest::targetTrackerDeactivatesAfterMisses()
{
    SphericalTargetTracker::Config config;
    config.maxMisses = 2;
    SphericalTargetTracker tracker(config);
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    tracker.update({}, 500);
    tracker.update({}, 1000);
    QVERIFY(tracker.trackById(QStringLiteral("t1"))->active());
    tracker.update({}, 1500);
    const TargetTrack *track = tracker.trackById(QStringLiteral("t1"));
    QVERIFY(!track->active());
    QCOMPARE(track->size(), 1);
    QCOMPARE(tracker.activeTracks().size(), 0);
}

void ProjectTest::targetTrackerFiltersLowConfidence()
{
    SphericalTargetTracker::Config config;
    config.minConfidence = 0.5;
    SphericalTargetTracker tracker(config);
    tracker.update({ makeTargetObservation(0, 0.0, 0.0, QStringLiteral("person"), 0.2) }, 0);
    QCOMPARE(tracker.tracks().size(), 0);
}

void ProjectTest::targetTrackerSeamContinuity()
{
    SphericalTargetTracker tracker;
    tracker.update({ makeTargetObservation(0, 179.0, 0.0) }, 0);
    tracker.update({ makeTargetObservation(500, -179.0, 0.0) }, 500);
    QCOMPARE(tracker.tracks().size(), 1);
    QCOMPARE(tracker.tracks().at(0).size(), 2);
}

void ProjectTest::targetTrackerIsDeterministic()
{
    const auto run = []() {
        SphericalTargetTracker tracker;
        tracker.update({ makeTargetObservation(0, -40.0, 0.0),
                         makeTargetObservation(0, 40.0, 0.0) }, 0);
        tracker.update({ makeTargetObservation(500, -38.0, 0.0),
                         makeTargetObservation(500, 42.0, 0.0) }, 500);
        return tracker;
    };
    const SphericalTargetTracker a = run();
    const SphericalTargetTracker b = run();
    QCOMPARE(a.tracks().size(), b.tracks().size());
    for (int i = 0; i < a.tracks().size(); ++i) {
        QCOMPARE(a.tracks().at(i).id(), b.tracks().at(i).id());
        QCOMPARE(a.tracks().at(i).size(), b.tracks().at(i).size());
        QVERIFY(qAbs(a.tracks().at(i).observations().last().yawDeg
                     - b.tracks().at(i).observations().last().yawDeg) < 1e-12);
    }
}

void ProjectTest::targetTrackSampleAtInterpolates()
{
    TargetTrack track(QStringLiteral("t1"), QStringLiteral("person"));
    track.append(makeTargetObservation(0, 170.0, 10.0, QStringLiteral("person"), 0.8));
    track.append(makeTargetObservation(1000, -170.0, 20.0, QStringLiteral("person"), 0.6));

    TargetObservation sampled;
    QVERIFY(track.sampleAt(500, &sampled));
    QVERIFY(qAbs(qAbs(sampled.yawDeg) - 180.0) < 1e-6);
    QVERIFY(qAbs(sampled.pitchDeg - 15.0) < 1e-6);
    QVERIFY(qAbs(sampled.confidence - 0.7) < 1e-6);

    QVERIFY(track.sampleAt(-100, &sampled));
    QVERIFY(qAbs(sampled.yawDeg - 170.0) < 1e-9);
    QVERIFY(track.sampleAt(5000, &sampled));
    QVERIFY(qAbs(sampled.yawDeg + 170.0) < 1e-9);
}

void ProjectTest::targetTrackRepresentativeTarget()
{
    TargetTrack track(QStringLiteral("t7"), QStringLiteral("person"));
    track.append(makeTargetObservation(0, 10.0, 5.0, QStringLiteral("person"), 0.5,
                                       QStringLiteral("t7")));
    track.append(makeTargetObservation(500, 20.0, 6.0, QStringLiteral("person"), 0.95,
                                       QStringLiteral("t7")));
    TargetObservation representative;
    QVERIFY(track.representative(&representative));
    QVERIFY(qAbs(representative.yawDeg - 20.0) < 1e-9);

    const ReframeTarget target = track.representativeTarget();
    QCOMPARE(target.id, QStringLiteral("t7"));
    QVERIFY(qAbs(target.yawDeg - 20.0) < 1e-9);
    QVERIFY(qAbs(target.pitchDeg - 6.0) < 1e-9);
}

void ProjectTest::targetResolverFindsSyntheticTarget()
{
    const QImage frame = buildTargetEquirect(
        360, 180, { EquirectDisk{ 40.0, 10.0, 8.0, QColor(255, 0, 0) } });
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("person");
    query.minConfidence = 0.3;

    QList<TargetObservation> observations;
    QString error;
    QVERIFY2(resolver.resolveFrame(frame, 0, query, &detector, &observations, &error),
             qPrintable(error));
    QCOMPARE(observations.size(), 1);
    QVERIFY(qAbs(observations.at(0).yawDeg - 40.0) < 4.0);
    QVERIFY(qAbs(observations.at(0).pitchDeg - 10.0) < 4.0);
    QVERIFY(observations.at(0).confidence > 0.3);
    QCOMPARE(observations.at(0).targetId, QStringLiteral("t1"));
    QCOMPARE(observations.at(0).label, QStringLiteral("person"));
}

void ProjectTest::targetResolverHonorsLabelQuery()
{
    const QImage frame = buildTargetEquirect(
        360, 180,
        { EquirectDisk{ -50.0, 0.0, 8.0, QColor(255, 0, 0) },
          EquirectDisk{ 50.0, 0.0, 8.0, QColor(0, 0, 255) } });
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    detector.addSpec(QColor(0, 0, 255), QStringLiteral("car"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("car");
    QList<TargetObservation> observations;
    QString error;
    QVERIFY2(resolver.resolveFrame(frame, 0, query, &detector, &observations, &error),
             qPrintable(error));
    QCOMPARE(observations.size(), 1);
    QCOMPARE(observations.at(0).label, QStringLiteral("car"));
    QVERIFY(qAbs(observations.at(0).yawDeg - 50.0) < 4.0);
}

void ProjectTest::targetResolverReportsUnresolved()
{
    QImage frame(360, 180, QImage::Format_ARGB32);
    frame.fill(QColor(0, 0, 0));
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("person");
    QList<TargetObservation> observations;
    QString error;
    QVERIFY(resolver.resolveFrame(frame, 0, query, &detector, &observations, &error));
    QCOMPARE(observations.size(), 0);
    QVERIFY(!resolver.notes().isEmpty());
    QVERIFY(resolver.notes().join(QStringLiteral("\n"))
                .contains(QStringLiteral("Unresolved")));
    QCOMPARE(resolver.tracks().size(), 0);
}

void ProjectTest::targetResolverRejectsInvalidInput()
{
    TargetResolver resolver(smallResolverConfig());
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    QList<TargetObservation> observations;
    QString error;
    QVERIFY(!resolver.resolveFrame(QImage(), 0, TargetQuery(), &detector, &observations,
                                   &error));
    QVERIFY(!error.isEmpty());

    QImage frame(360, 180, QImage::Format_ARGB32);
    frame.fill(QColor(0, 0, 0));
    QVERIFY(!resolver.resolveFrame(frame, 0, TargetQuery(), nullptr, &observations,
                                   &error));
}

void ProjectTest::targetResolverIsDeterministic()
{
    const QImage frame = buildTargetEquirect(
        360, 180, { EquirectDisk{ 25.0, -15.0, 8.0, QColor(255, 0, 0) } });
    const auto run = [&frame]() {
        SyntheticColorDetector detector;
        detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
        TargetResolver resolver(smallResolverConfig());
        QList<TargetObservation> observations;
        resolver.resolveFrame(frame, 0, TargetQuery{}, &detector, &observations, nullptr);
        return observations;
    };
    const QList<TargetObservation> a = run();
    const QList<TargetObservation> b = run();
    QCOMPARE(a.size(), b.size());
    for (int i = 0; i < a.size(); ++i) {
        QCOMPARE(a.at(i).targetId, b.at(i).targetId);
        QVERIFY(qAbs(a.at(i).yawDeg - b.at(i).yawDeg) < 1e-12);
        QVERIFY(qAbs(a.at(i).pitchDeg - b.at(i).pitchDeg) < 1e-12);
    }
}

void ProjectTest::targetResolverSequenceBuildsTrajectory()
{
    MovingDiskProvider provider(360, 180, QColor(255, 0, 0), 8.0);
    provider.setMotion(-20.0, 20.0, 1000);
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("person");
    QList<TargetTrack> tracks;
    QString error;
    QVERIFY2(resolver.resolveSequence(&provider, { 0, 500, 1000 }, query, &detector,
                                      &tracks, &error),
             qPrintable(error));
    QCOMPARE(tracks.size(), 1);
    QCOMPARE(tracks.at(0).size(), 3);
    const QList<TargetObservation> &observations = tracks.at(0).observations();
    QVERIFY(observations.at(0).yawDeg < observations.at(1).yawDeg);
    QVERIFY(observations.at(1).yawDeg < observations.at(2).yawDeg);
    QVERIFY(qAbs(observations.at(1).yawDeg) < 6.0);
}

void ProjectTest::targetResolverFeedsReframePlanBuilder()
{
    const QImage frame = buildTargetEquirect(
        360, 180, { EquirectDisk{ 45.0, 5.0, 8.0, QColor(255, 0, 0) } });
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("person");
    QList<TargetObservation> observations;
    QString error;
    QVERIFY2(resolver.resolveFrame(frame, 0, query, &detector, &observations, &error),
             qPrintable(error));
    QVERIFY(!observations.isEmpty());

    const QList<ReframeTarget> targets = resolver.resolvedTargets(QStringLiteral("person"));
    QCOMPARE(targets.size(), 1);
    QCOMPARE(targets.at(0).id, QStringLiteral("person"));
    QVERIFY(qAbs(targets.at(0).yawDeg - 45.0) < 4.0);

    const ReframeIntent intent =
        ReframeIntentParser::parse(QStringLiteral("look at the person"));
    const ReframeBuildResult built = ReframePlanBuilder::build(
        intent, targets, ReframePlan::TimeRange{ 0, 2000 },
        ReframePlan::OutputSpec{ 160, 90, 2.0 });
    QVERIFY2(built.ok, qPrintable(built.error));

    const CameraState state = CameraPath::stateAt(built.plan, 0);
    QVERIFY(qAbs(state.yawDeg - targets.at(0).yawDeg) < 1e-6);
}

void ProjectTest::targetProcessDetectorParsesResponse()
{
    QList<TargetDetection> detections;
    QString error;
    QVERIFY(ProcessTargetDetector::parseResponse(
        "{\"detections\":[{\"x\":1,\"y\":2,\"width\":3,\"height\":4,"
        "\"label\":\"person\",\"confidence\":0.8,\"id\":\"a\"}]}",
        &detections, &error));
    QCOMPARE(detections.size(), 1);
    QCOMPARE(detections.at(0).label, QStringLiteral("person"));
    QVERIFY(qAbs(detections.at(0).confidence - 0.8) < 1e-9);
    QCOMPARE(detections.at(0).targetId, QStringLiteral("a"));

    QVERIFY(!ProcessTargetDetector::parseResponse("not json", &detections, &error));
    QVERIFY(!ProcessTargetDetector::parseResponse("{}", &detections, &error));
    QVERIFY(ProcessTargetDetector::parseResponse(
        "{\"detections\":[{\"x\":0,\"y\":0,\"width\":0,\"height\":0}]}",
        &detections, &error));
    QCOMPARE(detections.size(), 0);
}

void ProjectTest::targetProcessDetectorRunsHelper()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString script = directory.filePath(QStringLiteral("helper.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\n"
               "cat > \"$2\" <<'EOF'\n"
               "{\"detections\":[{\"x\":10,\"y\":20,\"width\":30,\"height\":40,"
               "\"label\":\"person\",\"confidence\":0.85,\"id\":\"p1\"}]}\n"
               "EOF\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessTargetDetector detector(QStringLiteral("/bin/sh"), { script });
    QImage view(64, 48, QImage::Format_ARGB32);
    view.fill(QColor(0, 0, 0));
    QList<TargetDetection> detections;
    QString error;
    QVERIFY2(detector.detect(view, TargetQuery(), &detections, &error),
             qPrintable(error));
    QCOMPARE(detections.size(), 1);
    QCOMPARE(detections.at(0).label, QStringLiteral("person"));
    QVERIFY(qAbs(detections.at(0).confidence - 0.85) < 1e-9);
}

void ProjectTest::targetProcessDetectorFailsOnMissingExecutable()
{
    ProcessTargetDetector detector(QStringLiteral("/nonexistent/rc-helper-xyz"));
    QImage view(32, 32, QImage::Format_ARGB32);
    view.fill(QColor(0, 0, 0));
    QList<TargetDetection> detections;
    QString error;
    QVERIFY(!detector.detect(view, TargetQuery(), &detections, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::targetProcessDetectorFailsOnBadExit()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString script = directory.filePath(QStringLiteral("bad.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\nexit 3\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessTargetDetector detector(QStringLiteral("/bin/sh"), { script });
    QImage view(32, 32, QImage::Format_ARGB32);
    view.fill(QColor(0, 0, 0));
    QList<TargetDetection> detections;
    QString error;
    QVERIFY(!detector.detect(view, TargetQuery(), &detections, &error));
    QVERIFY(error.contains(QStringLiteral("failed")));
}

void ProjectTest::targetTrackPlannerBuildsFollowPlan()
{
    TargetTrack track(QStringLiteral("t1"), QStringLiteral("person"));
    track.append(makeTargetObservation(0, 0.0, 0.0));
    track.append(makeTargetObservation(500, 30.0, 5.0));
    track.append(makeTargetObservation(1000, 60.0, 10.0));

    ReframePlan plan;
    QString error;
    TargetTrackPlanner::Config config;
    config.fieldOfViewDeg = 90.0;
    config.maxKeyframes = 10;
    config.minConfidence = 0.3;
    QVERIFY2(TargetTrackPlanner::planTrack(
                 track, ReframePlan::TimeRange{ 0, 1500 },
                 ReframePlan::OutputSpec{ 160, 90, 2.0 }, config, &plan, &error),
             qPrintable(error));
    QCOMPARE(plan.keyframes().size(), 3);
    QCOMPARE(plan.keyframes().at(1).timeMs, qint64(500));

    QVERIFY(qAbs(CameraPath::stateAt(plan, 0).yawDeg - 0.0) < 1e-9);
    QVERIFY(qAbs(CameraPath::stateAt(plan, 1000).yawDeg - 60.0) < 1e-9);
    QVERIFY(qAbs(CameraPath::stateAt(plan, 500).yawDeg - 30.0) < 1e-9);
}

void ProjectTest::targetTrackPlannerRejectsEmptyTrack()
{
    TargetTrack track(QStringLiteral("t1"), QStringLiteral("person"));
    ReframePlan plan;
    QString error;
    QVERIFY(!TargetTrackPlanner::planTrack(
        track, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 160, 90, 1.0 }, TargetTrackPlanner::Config{},
        &plan, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::targetTrackPlannerFiltersLowConfidence()
{
    TargetTrack track(QStringLiteral("t1"), QStringLiteral("person"));
    track.append(makeTargetObservation(0, 0.0, 0.0, QStringLiteral("person"), 0.1));
    ReframePlan plan;
    QString error;
    TargetTrackPlanner::Config config;
    config.minConfidence = 0.3;
    QVERIFY(!TargetTrackPlanner::planTrack(
        track, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 160, 90, 1.0 }, config, &plan, &error));
}

void ProjectTest::targetResolutionToRenderPipeline()
{
    const QImage frame = buildTargetEquirect(
        360, 180, { EquirectDisk{ 50.0, 0.0, 10.0, QColor(255, 0, 0) } });
    SyntheticColorDetector detector;
    detector.addSpec(QColor(255, 0, 0), QStringLiteral("person"));
    TargetResolver resolver(smallResolverConfig());

    TargetQuery query;
    query.label = QStringLiteral("person");
    QList<TargetObservation> observations;
    QString error;
    QVERIFY2(resolver.resolveFrame(frame, 0, query, &detector, &observations, &error),
             qPrintable(error));
    QVERIFY(!observations.isEmpty());

    TargetTrack track(QStringLiteral("t1"), QStringLiteral("person"));
    track.append(observations.at(0));
    ReframePlan plan;
    QString planError;
    QVERIFY2(TargetTrackPlanner::planTrack(
                 track, ReframePlan::TimeRange{ 0, 1000 },
                 ReframePlan::OutputSpec{ 160, 90, 1.0 },
                 TargetTrackPlanner::Config{}, &plan, &planError),
             qPrintable(planError));

    StaticEquirectProvider provider(frame);
    int centeredFrames = 0;
    QString renderError;
    QVERIFY2(ReframeRenderer::render(
                 plan, &provider,
                 [&centeredFrames](int, qint64, const QImage &image) {
                     if (redDominant(image.pixelColor(image.width() / 2,
                                                      image.height() / 2))) {
                         ++centeredFrames;
                     }
                     return true;
                 },
                 nullptr, &renderError),
             qPrintable(renderError));
    QCOMPARE(centeredFrames, 1);
}


// Real-detector integration (skipped unless configured). This is the only test
// that runs an actual computer-vision model; the normal suite stays model-free.
// Configure with:
//   REELCRAFT_TARGET_DETECTOR_PY=<python>           (e.g. /usr/bin/python3)
//   REELCRAFT_TARGET_DETECTOR_SCRIPT=<helper.py>    (tools/detector_helper/yolox_detector.py)
//   REELCRAFT_TARGET_YOLOX_MODEL=<model.onnx>       (OpenCV Zoo YOLOX, Apache-2.0)
//   REELCRAFT_TARGET_CLIP=<clip.mp4>                (equirect 360 source/proxy)
//   REELCRAFT_TARGET_OUTPUT=<out.mp4>               (optional; default temp dir)
void ProjectTest::realDetectorIntegration()
{
    const QString python = qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_PY");
    const QString script = qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_SCRIPT");
    const QString model = qEnvironmentVariable("REELCRAFT_TARGET_YOLOX_MODEL");
    const QString clip = qEnvironmentVariable("REELCRAFT_TARGET_CLIP");
    if (python.isEmpty() || script.isEmpty() || model.isEmpty() || clip.isEmpty()) {
        QSKIP("real detector integration not configured "
              "(set REELCRAFT_TARGET_DETECTOR_PY/_SCRIPT, REELCRAFT_TARGET_YOLOX_MODEL, "
              "REELCRAFT_TARGET_CLIP)");
    }
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg is unavailable");
    }

    ProcessTargetDetector detector(python, { script, QStringLiteral("--model"), model });
    FfmpegSeekFrameProvider provider(clip, FrameExtractor::defaultExecutablePath());

    TargetResolveConfig config;
    config.viewPlan.fieldOfViewDeg = 110.0;
    config.viewPlan.yawCount = 4;
    config.viewPlan.pitchCount = 1;
    config.viewPlan.viewWidth = 512;
    config.viewPlan.viewHeight = 512;
    config.minConfidence = 0.35;
    config.tracker.maxAssociationDistanceDeg = 40.0;
    config.tracker.maxMisses = 3;
    TargetResolver resolver(config);

    TargetQuery query;
    query.label = QStringLiteral("person");
    query.minConfidence = 0.35;

    const QList<qint64> timestamps = { 5500, 6000, 6500, 7000, 7500 };
    QList<TargetTrack> tracks;
    QString error;
    QVERIFY2(resolver.resolveSequence(&provider, timestamps, query, &detector,
                                      &tracks, &error),
             qPrintable(error));
    for (const QString &note : resolver.notes()) {
        qInfo("resolver note: %s", qPrintable(note));
    }

    qInfo("real detector: %d track(s)", static_cast<int>(tracks.size()));
    QVERIFY2(!tracks.isEmpty(), "no real person was detected in the 360 clip");

    // Canonical, deterministic person ordering (never detector output order).
    const QList<TargetTrack> ordered =
        TargetSelector::canonicalOrder(tracks, QStringLiteral("person"));
    QVERIFY2(ordered.size() >= 2, "expected at least two people in the clip");
    for (int i = 0; i < ordered.size(); ++i) {
        qInfo("canonical person %d: id=%s obs=%d firstMs=%lld meanConf=%.3f",
              i + 1, qPrintable(ordered.at(i).id()),
              static_cast<int>(ordered.at(i).size()),
              static_cast<long long>(ordered.at(i).firstTimeMs()),
              ordered.at(i).meanConfidence());
    }

    // Pick the strongest presenter as the one the creator selects.
    int best = 0;
    for (int i = 1; i < ordered.size(); ++i) {
        if (ordered.at(i).size() > ordered.at(best).size()
            || (ordered.at(i).size() == ordered.at(best).size()
                && ordered.at(i).meanConfidence()
                       > ordered.at(best).meanConfidence())) {
            best = i;
        }
    }
    const TargetTrack &selectedTrack = ordered.at(best);

    // Simulate the creator selecting that person: a structured seed observation
    // at the selected person's direction/time.
    TargetObservation seedObservation;
    QVERIFY(selectedTrack.representative(&seedObservation));
    CreatorTargetSelection selection;
    selection.identity = QStringLiteral("me");
    selection.timeMs = seedObservation.timeMs;
    selection.yawDeg = seedObservation.yawDeg;
    selection.pitchDeg = seedObservation.pitchDeg;
    selection.label = QStringLiteral("person");
    selection.evidence =
        QStringLiteral("creator selected presenter %1").arg(selectedTrack.id());

    TargetIdentityRegistry registry;
    QVERIFY2(registry.bindFromSelection(selection, tracks, &error), qPrintable(error));
    registry.update(tracks, timestamps.last());
    QVERIFY2(registry.isResolved(TargetIdentityRegistry::creatorIdentity()),
             qPrintable(registry.notes().join(QStringLiteral("; "))));
    const QString meId =
        registry.targetId(TargetIdentityRegistry::creatorIdentity());
    qInfo("identity 'me' bound to track %s (method=%s)", qPrintable(meId),
          qPrintable(registry.binding(TargetIdentityRegistry::creatorIdentity())
                         ->method));

    const TargetSelectionResult me =
        TargetSelector::select(QStringLiteral("me"), tracks, registry);
    QVERIFY2(me.resolved, qPrintable(me.error));
    QCOMPARE(me.targetId, meId);

    const TargetSelectionResult firstPerson =
        TargetSelector::select(QStringLiteral("person 1"), tracks, registry);
    qInfo("selection 'person 1' -> %s",
          firstPerson.resolved ? qPrintable(firstPerson.targetId)
                               : qPrintable(firstPerson.error));
    const TargetSelectionResult other =
        TargetSelector::select(QStringLiteral("the other person"), tracks, registry);
    if (other.resolved) {
        qInfo("selection 'the other person' -> %s", qPrintable(other.targetId));
        QVERIFY(other.targetId != meId);
    } else {
        qInfo("selection 'the other person' -> %s (%s); candidates=%d",
              other.ambiguous ? "ambiguous" : "unresolved",
              qPrintable(other.error), static_cast<int>(other.candidates.size()));
    }

    const TargetTrack *resolvedTrack = nullptr;
    for (const TargetTrack &candidate : tracks) {
        if (candidate.id() == meId) {
            resolvedTrack = &candidate;
        }
    }
    QVERIFY(resolvedTrack != nullptr);
    const TargetTrack &track = *resolvedTrack;
    qInfo("selected track: id=%s label=%s observations=%d meanConf=%.3f",
          qPrintable(track.id()), qPrintable(track.label()),
          static_cast<int>(track.size()), track.meanConfidence());
    for (const TargetObservation &observation : track.observations()) {
        qInfo("  t=%lldms yaw=%.2f pitch=%.2f conf=%.3f radius=(%.2f,%.2f) source=%s",
              static_cast<long long>(observation.timeMs), observation.yawDeg,
              observation.pitchDeg, observation.confidence,
              observation.yawRadiusDeg, observation.pitchRadiusDeg,
              qPrintable(observation.source));
    }
    QVERIFY(track.size() >= 2);

    // --- Real appearance-based re-identification (Objective 5) --------------
    TargetTrack returnedTrack;
    const TargetTrack *finalTrack = &track;
    const QString reidPython = qEnvironmentVariable("REELCRAFT_REID_PY");
    const QString reidScript = qEnvironmentVariable("REELCRAFT_REID_SCRIPT");
    const QString reidModel = qEnvironmentVariable("REELCRAFT_REID_MODEL");
    if (!reidPython.isEmpty() && !reidScript.isEmpty() && !reidModel.isEmpty()) {
        ProcessAppearanceProvider appearance(
            reidPython, { reidScript, QStringLiteral("--model"), reidModel });
        appearance.setExpectedDimension(256);
        IdentityReidentifier reidentifier;
        const QString creatorKey = TargetIdentityRegistry::creatorIdentity();

        const IdentityReidentifier::Result profileResult =
            reidentifier.reidentify(&registry, tracks, &provider, &appearance,
                                    timestamps.last());
        for (const QString &note : profileResult.notes) {
            qInfo("  appearance note: %s", qPrintable(note));
        }
        QVERIFY2(registry.hasAppearanceProfile(creatorKey),
                 qPrintable(profileResult.notes.join(QStringLiteral("; "))));

        QImage appearanceFrame;
        QVERIFY2(provider.frameAt(timestamps.last(), &appearanceFrame, &error),
                 qPrintable(error));
        TargetObservation meObservation;
        QVERIFY(track.representative(&meObservation));
        QImage meCrop;
        QVERIFY2(TargetCropExtractor::crop(
                     appearanceFrame, meObservation.yawDeg, meObservation.pitchDeg,
                     meObservation.yawRadiusDeg, meObservation.pitchRadiusDeg,
                     TargetCropExtractor::Config{}, &meCrop, &error),
                 qPrintable(error));
        AppearanceEmbedding meEmbedding;
        QVERIFY2(appearance.encode(meCrop, meId, timestamps.last(), &meEmbedding,
                                   &error),
                 qPrintable(error));
        double sameSimilarity = 0.0;
        QVERIFY2(AppearanceMath::cosineSimilarity(
                     registry.appearanceProfile(creatorKey)->reference, meEmbedding,
                     &sameSimilarity, &error),
                 qPrintable(error));

        const TargetSelectionResult secondSelection =
            TargetSelector::select(QStringLiteral("person 2"), tracks, registry);
        double otherSimilarity = -1.0;
        const TargetTrack *secondTrack = nullptr;
        if (secondSelection.resolved) {
            for (const TargetTrack &candidate : tracks) {
                if (candidate.id() == secondSelection.targetId) {
                    secondTrack = &candidate;
                }
            }
        }
        if (secondTrack) {
            TargetObservation secondObservation;
            secondTrack->representative(&secondObservation);
            QImage secondCrop;
            if (TargetCropExtractor::crop(
                    appearanceFrame, secondObservation.yawDeg,
                    secondObservation.pitchDeg, secondObservation.yawRadiusDeg,
                    secondObservation.pitchRadiusDeg, TargetCropExtractor::Config{},
                    &secondCrop, &error)) {
                AppearanceEmbedding secondEmbedding;
                if (appearance.encode(secondCrop, secondSelection.targetId,
                                      timestamps.last(), &secondEmbedding, &error)) {
                    AppearanceMath::cosineSimilarity(
                        registry.appearanceProfile(creatorKey)->reference,
                        secondEmbedding, &otherSimilarity, &error);
                }
            }
        }
        qInfo("appearance: same-person=%.3f other-person=%.3f", sameSimilarity,
              otherSimilarity);
        QVERIFY2(sameSimilarity >= 0.75,
                 "appearance did not agree with the same presenter");
        if (otherSimilarity >= 0.0) {
            QVERIFY2(otherSimilarity < 0.75,
                     "appearance did not discriminate the other presenter");
        }

        // Controlled re-acquisition: a tracker gap beyond the geometric window
        // gives the returning presenter a new id while the other presenter is
        // still present. Appearance must re-acquire the correct person.
        TargetIdentityRegistry::Config gapConfig = registry.config();
        gapConfig.rebindWindowMs = 0;
        registry.setConfig(gapConfig);
        const qint64 returningTimeMs = timestamps.last() + 1;
        returnedTrack = TargetTrack(QStringLiteral("t100"), QStringLiteral("person"));
        TargetObservation returningObservation = makeTargetObservation(
            returningTimeMs, meObservation.yawDeg, meObservation.pitchDeg,
            QStringLiteral("person"), 0.92, QStringLiteral("t100"));
        // Match the original detection's angular extent so the appearance crop
        // uses the same framing as the profile.
        returningObservation.yawRadiusDeg = meObservation.yawRadiusDeg;
        returningObservation.pitchRadiusDeg = meObservation.pitchRadiusDeg;
        returnedTrack.append(returningObservation);
        returnedTrack.setActive(true);
        TargetTrack inactiveMe = track;
        inactiveMe.setActive(false);
        QList<TargetTrack> gapTracks;
        gapTracks.append(inactiveMe);
        gapTracks.append(returnedTrack);
        if (secondTrack) {
            gapTracks.append(*secondTrack);
        }
        const IdentityReidentifier::Result reacquired =
            reidentifier.reidentify(&registry, gapTracks, &provider, &appearance,
                                    returningTimeMs);
        qInfo("appearance re-acquisition: resolved=%d target=%s",
              registry.isResolved(creatorKey) ? 1 : 0,
              qPrintable(registry.targetId(creatorKey)));
        for (const QString &note : reacquired.notes) {
            qInfo("  reappearance note: %s", qPrintable(note));
        }
        QVERIFY2(registry.isResolved(creatorKey),
                 qPrintable(reacquired.notes.join(QStringLiteral("; "))));
        QCOMPARE(registry.targetId(creatorKey), QStringLiteral("t100"));
        finalTrack = &returnedTrack;
        // Make the re-acquired track visible to later stages (speaker
        // association and planning).
        tracks.append(returnedTrack);
    }

    ReframePlan plan;
    QString planError;
    TargetTrackPlanner::Config plannerConfig;
    plannerConfig.fieldOfViewDeg = 70.0;
    plannerConfig.maxKeyframes = 12;
    plannerConfig.minConfidence = 0.35;
    QVERIFY2(TargetTrackPlanner::planTrack(
                 *finalTrack, ReframePlan::TimeRange{ timestamps.first(), 8500 },
                 ReframePlan::OutputSpec{ 640, 360, 2.0 }, plannerConfig, &plan,
                 &planError),
             qPrintable(planError));
    QVERIFY(plan.isValid());
    for (const CameraKeyframe &keyframe : plan.keyframes()) {
        qInfo("  keyframe t=%lldms yaw=%.2f pitch=%.2f",
              static_cast<long long>(keyframe.timeMs), keyframe.yawDeg,
              keyframe.pitchDeg);
    }

    QString outputPath = qEnvironmentVariable("REELCRAFT_TARGET_OUTPUT");
    QTemporaryDir tempDirectory;
    if (outputPath.isEmpty()) {
        QVERIFY(tempDirectory.isValid());
        outputPath = tempDirectory.filePath(QStringLiteral("real_follow.mp4"));
    }
    const QString framesDir =
        QDir(QFileInfo(outputPath).absolutePath())
            .filePath(QFileInfo(outputPath).completeBaseName() + QStringLiteral("_frames"));
    QStringList paths;
    QString renderError;
    QVERIFY2(ReframeRenderer::renderToPngSequence(plan, &provider, framesDir, &paths,
                                                  &renderError),
             qPrintable(renderError));
    QVERIFY(!paths.isEmpty());
    const QString pattern =
        QDir(framesDir).filePath(ReframeRenderer::frameFileNamePattern());
    QVERIFY2(ReframeRenderer::encodeVideo(FrameExtractor::defaultExecutablePath(),
                                          pattern, plan.output().fps, outputPath,
                                          &renderError),
             qPrintable(renderError));
    QVERIFY(QFileInfo::exists(outputPath));
    QVERIFY(QFileInfo(outputPath).size() > 0);
    qInfo("rendered %d frame(s) -> %s", static_cast<int>(paths.size()),
          qPrintable(outputPath));

    QImage decoded;
    QVERIFY2(FrameExtractor::extractFirstFrame(
                 outputPath, FrameExtractor::defaultExecutablePath(), &decoded,
                 &renderError),
             qPrintable(renderError));
    QCOMPARE(decoded.size(), QSize(640, 360));

    // --- Real speech / speaker evidence (Objective 6) -----------------------
    const QString speakerPython = qEnvironmentVariable("REELCRAFT_SPEAKER_PY");
    const QString speakerScript = qEnvironmentVariable("REELCRAFT_SPEAKER_SCRIPT");
    const QString sileroModel = qEnvironmentVariable("REELCRAFT_SILERO_MODEL");
    if (!speakerPython.isEmpty() && !speakerScript.isEmpty() && !sileroModel.isEmpty()) {
        ProcessSpeakerProvider speakerProvider(
            speakerPython, { speakerScript, QStringLiteral("--model"), sileroModel });
        SpeakerTargetAssociator associator;
        // The creator identifies the speaking presenter once; the audio provider
        // then tracks speech activity over time.
        associator.bind(QStringLiteral("spk1"), finalTrack->id());
        SpeakerEvidenceAnalyzer speakerAnalyzer;
        const qint64 speakerWindowEndMs = 12000;
        const SpeakerEvidenceAnalyzer::Result speakerResult =
            speakerAnalyzer.analyze(clip, 0, speakerWindowEndMs, tracks,
                                    &speakerProvider, associator);
        for (const QString &note : speakerResult.notes) {
            qInfo("  speaker note: %s", qPrintable(note));
        }
        qInfo("speaker: available=%d intervals=%d segments=%d evidence=%d",
              speakerResult.analysis.available ? 1 : 0,
              static_cast<int>(speakerResult.analysis.intervals.size()),
              static_cast<int>(speakerResult.segments.size()),
              static_cast<int>(speakerResult.evidence.size()));
        QVERIFY2(speakerResult.analysis.available,
                 qPrintable(speakerResult.notes.join(QStringLiteral("; "))));
        QVERIFY(!speakerResult.analysis.intervals.isEmpty());
        bool boundTargetActive = false;
        for (const SpeakerEvidence &evidence : speakerResult.evidence) {
            if (evidence.targetId == finalTrack->id()
                && evidence.verdict == SpeakerVerdict::Active) {
                boundTargetActive = true;
            }
        }
        QVERIFY2(boundTargetActive,
                 "real speaker evidence did not associate with the selected target");

        ReframePlan speakerPlan;
        QString speakerPlanError;
        QVERIFY2(SpeakerReframePlanner::plan(
                     speakerResult.segments, tracks,
                     ReframePlan::TimeRange{ 0, speakerWindowEndMs },
                     ReframePlan::OutputSpec{ 640, 360, 2.0 }, {}, &speakerPlan,
                     &speakerPlanError),
                 qPrintable(speakerPlanError));
        QTemporaryDir speakerDirectory;
        QString speakerOutputPath = qEnvironmentVariable("REELCRAFT_SPEAKER_OUTPUT");
        if (speakerOutputPath.isEmpty()) {
            QVERIFY(speakerDirectory.isValid());
            speakerOutputPath =
                speakerDirectory.filePath(QStringLiteral("speaker_follow.mp4"));
        }
        const QString speakerFramesDir =
            QDir(QFileInfo(speakerOutputPath).absolutePath())
                .filePath(QFileInfo(speakerOutputPath).completeBaseName()
                          + QStringLiteral("_frames"));
        QStringList speakerPaths;
        QString speakerRenderError;
        QVERIFY2(ReframeRenderer::renderToPngSequence(
                     speakerPlan, &provider, speakerFramesDir, &speakerPaths,
                     &speakerRenderError),
                 qPrintable(speakerRenderError));
        const QString speakerPattern =
            QDir(speakerFramesDir)
                .filePath(ReframeRenderer::frameFileNamePattern());
        QVERIFY2(ReframeRenderer::encodeVideo(
                     FrameExtractor::defaultExecutablePath(), speakerPattern,
                     speakerPlan.output().fps, speakerOutputPath,
                     &speakerRenderError),
                 qPrintable(speakerRenderError));
        QVERIFY(QFileInfo::exists(speakerOutputPath));
        qInfo("speaker plan: %d keyframe(s); rendered %d frame(s) -> %s",
              static_cast<int>(speakerPlan.keyframes().size()),
              static_cast<int>(speakerPaths.size()),
              qPrintable(speakerOutputPath));
    }
}

// ================= 360 target identity & selection (Phase 4, Obj 4) =================
// Deterministic, model-free tests for tracker motion hardening, the structured
// identity registry, and the deterministic target selector. These never load a
// model or download weights.

namespace {

TargetTrack makeIdTrack(const QString &id, const QString &label,
                        const QList<TargetObservation> &observations)
{
    TargetTrack track(id, label);
    for (const TargetObservation &observation : observations) {
        TargetObservation copy = observation;
        copy.targetId = id;
        track.append(copy);
    }
    return track;
}

QList<TargetTrack> twoPresenterTracks()
{
    QList<TargetTrack> tracks;
    tracks.append(makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                              { makeTargetObservation(0, -28.0, 0.0),
                                makeTargetObservation(1000, -28.0, 0.0) }));
    tracks.append(makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                              { makeTargetObservation(0, 27.0, 0.0),
                                makeTargetObservation(1000, 27.0, 0.0) }));
    return tracks;
}

} // namespace

void ProjectTest::targetTrackerPredictionMaintainsIdentityThroughCrossing()
{
    SphericalTargetTracker::Config config;
    config.useVelocityPrediction = true;
    config.maxAssociationDistanceDeg = 25.0;
    SphericalTargetTracker tracker(config);
    tracker.update({ makeTargetObservation(0, -15.0, 0.0),
                     makeTargetObservation(0, 15.0, 0.0) }, 0);
    tracker.update({ makeTargetObservation(500, -5.0, 0.0),
                     makeTargetObservation(500, 5.0, 0.0) }, 500);
    tracker.update({ makeTargetObservation(1000, 5.0, 0.0),
                     makeTargetObservation(1000, -5.0, 0.0) }, 1000);
    tracker.update({ makeTargetObservation(1500, 15.0, 0.0),
                     makeTargetObservation(1500, -15.0, 0.0) }, 1500);

    QCOMPARE(tracker.tracks().size(), 2);
    const TargetTrack *first = tracker.trackById(QStringLiteral("t1"));
    const TargetTrack *second = tracker.trackById(QStringLiteral("t2"));
    QVERIFY(first != nullptr);
    QVERIFY(second != nullptr);
    QCOMPARE(first->size(), 4);
    QCOMPARE(second->size(), 4);
    // t1 tracked the target moving left->right; t2 the one moving right->left.
    QVERIFY(first->observations().at(0).yawDeg < first->observations().at(3).yawDeg);
    QVERIFY(second->observations().at(0).yawDeg > second->observations().at(3).yawDeg);
    QVERIFY(qAbs(first->observations().at(3).yawDeg - 15.0) < 1e-9);
    QVERIFY(qAbs(second->observations().at(3).yawDeg + 15.0) < 1e-9);
}

void ProjectTest::targetTrackerReentryKeepsIdentityWithinWindow()
{
    SphericalTargetTracker::Config config;
    config.useVelocityPrediction = false;
    config.maxAssociationDistanceDeg = 15.0;
    config.reentryGateDeg = 60.0;
    config.reentryWindowMs = 2000;
    config.maxMisses = 1;
    SphericalTargetTracker tracker(config);
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    tracker.update({}, 500);
    tracker.update({}, 1000);
    QVERIFY(!tracker.trackById(QStringLiteral("t1"))->active());
    tracker.update({ makeTargetObservation(1500, 40.0, 0.0) }, 1500);
    QCOMPARE(tracker.tracks().size(), 1);
    const TargetTrack *track = tracker.trackById(QStringLiteral("t1"));
    QVERIFY(track->active());
    QCOMPARE(track->size(), 2);
}

void ProjectTest::targetTrackerReentryBeyondWindowCreatesNewTrack()
{
    SphericalTargetTracker::Config config;
    config.useVelocityPrediction = false;
    config.maxAssociationDistanceDeg = 15.0;
    config.reentryGateDeg = 60.0;
    config.reentryWindowMs = 1000;
    config.maxMisses = 1;
    SphericalTargetTracker tracker(config);
    tracker.update({ makeTargetObservation(0, 0.0, 0.0) }, 0);
    tracker.update({}, 500);
    tracker.update({}, 1000);
    tracker.update({ makeTargetObservation(1500, 40.0, 0.0) }, 1500);
    QCOMPARE(tracker.tracks().size(), 2);
}

void ProjectTest::targetTrackerPredictionIsDeterministic()
{
    const auto run = []() {
        SphericalTargetTracker::Config config;
        config.useVelocityPrediction = true;
        SphericalTargetTracker tracker(config);
        tracker.update({ makeTargetObservation(0, -15.0, 0.0),
                         makeTargetObservation(0, 15.0, 0.0) }, 0);
        tracker.update({ makeTargetObservation(500, -5.0, 0.0),
                         makeTargetObservation(500, 5.0, 0.0) }, 500);
        tracker.update({ makeTargetObservation(1000, 5.0, 0.0),
                         makeTargetObservation(1000, -5.0, 0.0) }, 1000);
        return tracker;
    };
    const SphericalTargetTracker a = run();
    const SphericalTargetTracker b = run();
    QCOMPARE(a.tracks().size(), b.tracks().size());
    for (int i = 0; i < a.tracks().size(); ++i) {
        QCOMPARE(a.tracks().at(i).id(), b.tracks().at(i).id());
        QCOMPARE(a.tracks().at(i).size(), b.tracks().at(i).size());
        for (int j = 0; j < a.tracks().at(i).size(); ++j) {
            QVERIFY(qAbs(a.tracks().at(i).observations().at(j).yawDeg
                         - b.tracks().at(i).observations().at(j).yawDeg) < 1e-12);
        }
    }
}

void ProjectTest::targetIdentityBindsFromSeedDirection()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    CreatorTargetSelection seed;
    seed.identity = QStringLiteral("me");
    seed.timeMs = 0;
    seed.yawDeg = -28.0;
    seed.pitchDeg = 0.0;
    seed.label = QStringLiteral("person");
    seed.evidence = QStringLiteral("creator selected the left presenter");
    QString error;
    QVERIFY2(registry.bindFromSelection(seed, tracks, &error), qPrintable(error));
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t1"));
    const IdentityBinding *binding = registry.binding(QStringLiteral("me"));
    QVERIFY(binding != nullptr);
    QCOMPARE(binding->method, QStringLiteral("seed-direction"));
    QVERIFY(binding->distanceDeg < 1.0);
}

void ProjectTest::targetIdentitySeedRejectsDistantOrInvalid()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    CreatorTargetSelection far;
    far.identity = QStringLiteral("me");
    far.timeMs = 0;
    far.yawDeg = 150.0;
    far.pitchDeg = 0.0;
    QString error;
    QVERIFY(!registry.bindFromSelection(far, tracks, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!registry.isBound(QStringLiteral("me")));

    CreatorTargetSelection emptyIdentity;
    emptyIdentity.yawDeg = 0.0;
    emptyIdentity.pitchDeg = 0.0;
    QVERIFY(!emptyIdentity.isValid());

    CreatorTargetSelection badPitch;
    badPitch.identity = QStringLiteral("me");
    badPitch.pitchDeg = 200.0;
    QVERIFY(!badPitch.isValid());
}

void ProjectTest::targetIdentityTrackIdBindingAndClaimConflicts()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY2(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                  tracks, &error), qPrintable(error));
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QVERIFY(!registry.bindToTrack(QStringLiteral("other"), QStringLiteral("t1"), 0,
                                  tracks, &error));
    QVERIFY(!registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t9"), 0,
                                  tracks, &error));
}

void ProjectTest::targetIdentityResolutionTracksActiveState()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> tracks = twoPresenterTracks();
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    registry.update(tracks, 500);
    QVERIFY(registry.isResolved(QStringLiteral("me")));

    TargetTrack inactive = makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                                       { makeTargetObservation(0, -28.0, 0.0) });
    inactive.setActive(false);
    registry.update({ inactive }, 100000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    QVERIFY(!registry.notes().isEmpty());
}

void ProjectTest::targetIdentityContinuityRebindIsUnique()
{
    TargetIdentityRegistry::Config config;
    config.rebindGateDeg = 60.0;
    config.rebindWindowMs = 3000;
    TargetIdentityRegistry registry(config);
    QString error;
    const QList<TargetTrack> original = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0),
                      makeTargetObservation(500, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 original, &error));

    TargetTrack inactive = makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                                       { makeTargetObservation(0, -20.0, 0.0),
                                         makeTargetObservation(500, 0.0, 0.0) });
    inactive.setActive(false);
    const TargetTrack continuation = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 20.0, 0.0) });
    registry.update({ inactive, continuation }, 1000);
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t2"));
    QCOMPARE(registry.binding(QStringLiteral("me"))->method,
             QStringLiteral("continuity-rebind"));
}

void ProjectTest::targetIdentityContinuityRebindAmbiguousIsUnresolved()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> original = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 original, &error));

    TargetTrack inactive = makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                                       { makeTargetObservation(0, 0.0, 0.0) });
    inactive.setActive(false);
    const TargetTrack left = makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                                         { makeTargetObservation(1000, 10.0, 0.0) });
    const TargetTrack right = makeIdTrack(QStringLiteral("t3"), QStringLiteral("person"),
                                          { makeTargetObservation(1000, -10.0, 0.0) });
    registry.update({ inactive, left, right }, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    QVERIFY(registry.notes().join(QStringLiteral("\n"))
                .contains(QStringLiteral("ambiguous")));
}

void ProjectTest::targetIdentityJsonRoundTrip()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 123,
                                 tracks, &error));

    const QJsonObject object = registry.toJsonObject();
    TargetIdentityRegistry restored;
    QVERIFY2(restored.readFromJsonObject(object, &error), qPrintable(error));
    QCOMPARE(restored.targetId(QStringLiteral("me")), QStringLiteral("t1"));
    // Resolution is refreshed against live tracks, not trusted from disk.
    QVERIFY(!restored.isResolved(QStringLiteral("me")));
    restored.update(tracks, 0);
    QVERIFY(restored.isResolved(QStringLiteral("me")));

    CreatorTargetSelection seed;
    seed.identity = QStringLiteral("me");
    seed.timeMs = 10;
    seed.yawDeg = 1.0;
    seed.pitchDeg = 2.0;
    seed.label = QStringLiteral("person");
    CreatorTargetSelection seedBack;
    QVERIFY(CreatorTargetSelection::readFromJsonObject(seed.toJsonObject(),
                                                       &seedBack, &error));
    QCOMPARE(seedBack.identity, seed.identity);
    QCOMPARE(seedBack.timeMs, seed.timeMs);
    QVERIFY(qAbs(seedBack.yawDeg - seed.yawDeg) < 1e-9);
    QVERIFY(qAbs(seedBack.pitchDeg - seed.pitchDeg) < 1e-9);
}

void ProjectTest::targetSelectorResolvesCreatorAliases()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    const QStringList references = {
        QStringLiteral("me"),
        QStringLiteral("myself"),
        QStringLiteral("the person I selected"),
        QStringLiteral("my selection"),
        QStringLiteral("the selected person"),
    };
    for (const QString &reference : references) {
        const TargetSelectionResult result =
            TargetSelector::select(reference, tracks, registry);
        QVERIFY2(result.resolved, qPrintable(result.error));
        QCOMPARE(result.targetId, QStringLiteral("t1"));
        QCOMPARE(result.method, QStringLiteral("identity"));
        QCOMPARE(result.target.id, TargetSelector::normalizeReference(reference));
    }
}

void ProjectTest::targetSelectorUnresolvedCreatorWhenUnbound()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    const TargetIdentityRegistry registry;
    const TargetSelectionResult result =
        TargetSelector::select(QStringLiteral("me"), tracks, registry);
    QVERIFY(!result.resolved);
    QVERIFY(!result.ambiguous);
    QVERIFY(result.error.contains(QStringLiteral("not resolved")));
    QVERIFY(TargetSelector::resolvedTargets(QStringLiteral("me"), tracks, registry)
                .isEmpty());
}

void ProjectTest::targetSelectorResolvesOtherPerson()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    const TargetSelectionResult result =
        TargetSelector::select(QStringLiteral("the other person"), tracks, registry);
    QVERIFY2(result.resolved, qPrintable(result.error));
    QCOMPARE(result.targetId, QStringLiteral("t2"));
    QCOMPARE(result.method, QStringLiteral("other-person"));
}

void ProjectTest::targetSelectorOtherPersonAmbiguous()
{
    QList<TargetTrack> tracks = twoPresenterTracks();
    tracks.append(makeIdTrack(QStringLiteral("t3"), QStringLiteral("person"),
                              { makeTargetObservation(0, 80.0, 0.0) }));
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    const TargetSelectionResult result =
        TargetSelector::select(QStringLiteral("the other person"), tracks, registry);
    QVERIFY(!result.resolved);
    QVERIFY(result.ambiguous);
    QCOMPARE(result.candidates.size(), 2);
}

void ProjectTest::targetSelectorOrdinalIsDeterministicRegardlessOfInputOrder()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    const QList<TargetTrack> reversed = { tracks.at(1), tracks.at(0) };
    const TargetIdentityRegistry registry;
    const TargetSelectionResult firstA =
        TargetSelector::select(QStringLiteral("person 1"), tracks, registry);
    const TargetSelectionResult firstB =
        TargetSelector::select(QStringLiteral("person 1"), reversed, registry);
    const TargetSelectionResult secondA =
        TargetSelector::select(QStringLiteral("person 2"), tracks, registry);
    const TargetSelectionResult secondB =
        TargetSelector::select(QStringLiteral("person 2"), reversed, registry);
    QVERIFY(firstA.resolved && firstB.resolved);
    QCOMPARE(firstA.targetId, firstB.targetId);
    QVERIFY(secondA.resolved && secondB.resolved);
    QCOMPARE(secondA.targetId, secondB.targetId);
    QVERIFY(firstA.targetId != secondA.targetId);
}

void ProjectTest::targetSelectorOrdinalOutOfRange()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    const TargetIdentityRegistry registry;
    const TargetSelectionResult result =
        TargetSelector::select(QStringLiteral("person 9"), tracks, registry);
    QVERIFY(!result.resolved);
    QVERIFY(result.error.contains(QStringLiteral("not visible")));
}

void ProjectTest::targetSelectorLeftRightByYaw()
{
    QList<TargetTrack> tracks;
    tracks.append(makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                              { makeTargetObservation(0, -30.0, 0.0) }));
    tracks.append(makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                              { makeTargetObservation(0, 40.0, 0.0) }));
    const TargetIdentityRegistry registry;
    const TargetSelectionResult left =
        TargetSelector::select(QStringLiteral("the person on the left"), tracks, registry);
    const TargetSelectionResult right =
        TargetSelector::select(QStringLiteral("the person on my right"), tracks, registry);
    QVERIFY(left.resolved);
    QCOMPARE(left.targetId, QStringLiteral("t1"));
    QCOMPARE(left.method, QStringLiteral("left"));
    QVERIFY(right.resolved);
    QCOMPARE(right.targetId, QStringLiteral("t2"));
    QCOMPARE(right.method, QStringLiteral("right"));
}

void ProjectTest::targetSelectorTrackIdAndUniqueLabel()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    const TargetIdentityRegistry registry;
    const TargetSelectionResult byId =
        TargetSelector::select(QStringLiteral("t2"), tracks, registry);
    QVERIFY(byId.resolved);
    QCOMPARE(byId.targetId, QStringLiteral("t2"));

    const TargetSelectionResult labelAmbiguous =
        TargetSelector::select(QStringLiteral("person"), tracks, registry);
    QVERIFY(!labelAmbiguous.resolved);
    QVERIFY(labelAmbiguous.ambiguous);

    const QList<TargetTrack> single = { tracks.at(0) };
    const TargetSelectionResult labelUnique =
        TargetSelector::select(QStringLiteral("person"), single, registry);
    QVERIFY(labelUnique.resolved);
    QCOMPARE(labelUnique.targetId, QStringLiteral("t1"));
}

void ProjectTest::targetSelectorIsDeterministic()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    for (const QString &reference : { QStringLiteral("me"),
                                      QStringLiteral("the other person"),
                                      QStringLiteral("person 2") }) {
        const TargetSelectionResult a =
            TargetSelector::select(reference, tracks, registry);
        const TargetSelectionResult b =
            TargetSelector::select(reference, tracks, registry);
        QCOMPARE(a.resolved, b.resolved);
        QCOMPARE(a.targetId, b.targetId);
        QCOMPARE(a.method, b.method);
    }
}

void ProjectTest::targetSelectorResolvedTargetsFeedsReframePlanBuilder()
{
    const QList<TargetTrack> tracks = twoPresenterTracks();
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));

    const QList<ReframeTarget> targets =
        TargetSelector::resolvedTargets(QStringLiteral("me"), tracks, registry);
    QCOMPARE(targets.size(), 1);
    QCOMPARE(targets.at(0).id, QStringLiteral("me"));

    const ReframeIntent intent =
        ReframeIntentParser::parse(QStringLiteral("follow me"));
    QVERIFY(intent.recognized);
    const ReframeBuildResult built = ReframePlanBuilder::build(
        intent, targets, ReframePlan::TimeRange{ 0, 2000 },
        ReframePlan::OutputSpec{ 160, 90, 2.0 });
    QVERIFY2(built.ok, qPrintable(built.error));
    const CameraState state = CameraPath::stateAt(built.plan, 0);
    QVERIFY(qAbs(state.yawDeg + 28.0) < 1e-9);
}

// ================= 360 appearance re-identification (Phase 4, Obj 5) =================
// Deterministic, model-free tests for the appearance evidence layer, the
// subprocess appearance provider, and the identity re-identification policy.
// The real ReID model is exercised only by the separate integration test.

namespace {

struct AppearanceFrameSpec
{
    qint64 timeMs = 0;
    QList<EquirectDisk> disks;
};

class ScriptedFrameProvider : public ReframeFrameProvider
{
public:
    void addFrame(qint64 timeMs, const QList<EquirectDisk> &disks)
    {
        AppearanceFrameSpec spec;
        spec.timeMs = timeMs;
        spec.disks = disks;
        m_specs.append(spec);
    }

    bool frameAt(qint64 timeMs, QImage *outFrame, QString *error) override
    {
        if (error) {
            error->clear();
        }
        const AppearanceFrameSpec *best = nullptr;
        for (const AppearanceFrameSpec &spec : m_specs) {
            if (spec.timeMs <= timeMs && (!best || spec.timeMs > best->timeMs)) {
                best = &spec;
            }
        }
        if (!best) {
            if (error) {
                *error = QStringLiteral("no scripted frame");
            }
            return false;
        }
        if (outFrame) {
            *outFrame = buildTargetEquirect(360, 180, best->disks);
        }
        return true;
    }

private:
    QList<AppearanceFrameSpec> m_specs;
};

// Deterministic fake appearance provider: the embedding is the normalized
// RGB of the crop's centre pixel. Similarities are therefore exact and
// controllable (red vs red = 1.0, red vs (r,g,0) = r/sqrt(r^2+g^2), ...).
class ColorAppearanceProvider : public AppearanceProvider
{
public:
    void setFail(bool fail) { m_fail = fail; }
    int calls() const { return m_calls; }
    QString name() const override { return QStringLiteral("color"); }

    bool encode(const QImage &crop, const QString &, qint64,
                AppearanceEmbedding *out, QString *error) override
    {
        ++m_calls;
        if (error) {
            error->clear();
        }
        if (m_fail) {
            if (error) {
                *error = QStringLiteral("provider failure");
            }
            return false;
        }
        if (!out || crop.isNull()) {
            if (error) {
                *error = QStringLiteral("empty crop");
            }
            return false;
        }
        const QColor color = crop.pixelColor(crop.width() / 2, crop.height() / 2);
        AppearanceEmbedding embedding;
        embedding.values = { double(color.red()), double(color.green()),
                             double(color.blue()) };
        embedding.provider = QStringLiteral("color");
        if (!AppearanceMath::normalize(&embedding, error)) {
            return false;
        }
        *out = embedding;
        return true;
    }

private:
    bool m_fail = false;
    int m_calls = 0;
};

TargetTrack inactiveCopy(const TargetTrack &track)
{
    TargetTrack copy = track;
    copy.setActive(false);
    return copy;
}

} // namespace

void ProjectTest::appearanceEmbeddingJsonRoundTrip()
{
    AppearanceEmbedding embedding;
    embedding.values = { 1.0, 2.0, 3.0 };
    embedding.quality = 0.8;
    embedding.provider = QStringLiteral("unit");
    const QJsonObject object = embedding.toJsonObject();
    AppearanceEmbedding restored;
    QString error;
    QVERIFY2(AppearanceEmbedding::readFromJsonObject(object, &restored, &error),
             qPrintable(error));
    QCOMPARE(restored.dimension(), 3);
    QVERIFY(qAbs(restored.values.at(1) - 2.0) < 1e-12);
    QVERIFY(qAbs(restored.quality - 0.8) < 1e-12);
    QCOMPARE(restored.provider, QStringLiteral("unit"));
}

void ProjectTest::appearanceEmbeddingRejectsInvalid()
{
    QString error;
    AppearanceEmbedding empty;
    QVERIFY(!empty.isValid(&error));
    AppearanceEmbedding nonFinite;
    nonFinite.values = { 1.0, std::nan("") };
    QVERIFY(!nonFinite.isValid(&error));
    AppearanceEmbedding badQuality;
    badQuality.values = { 1.0 };
    badQuality.quality = 2.0;
    QVERIFY(!badQuality.isValid(&error));

    AppearanceEmbedding restored;
    QVERIFY(!AppearanceEmbedding::readFromJsonObject(QJsonObject(), &restored, &error));
    QJsonObject object;
    QJsonArray array;
    array.append(1.0);
    array.append(QStringLiteral("x"));
    object.insert(QStringLiteral("embedding"), array);
    QVERIFY(!AppearanceEmbedding::readFromJsonObject(object, &restored, &error));
}

void ProjectTest::appearanceNormalizeAndCosine()
{
    AppearanceEmbedding a;
    a.values = { 3.0, 4.0 };
    QVERIFY(AppearanceMath::normalize(&a));
    QVERIFY(qAbs(std::sqrt(a.values.at(0) * a.values.at(0)
                           + a.values.at(1) * a.values.at(1)) - 1.0) < 1e-9);
    AppearanceEmbedding zero;
    zero.values = { 0.0, 0.0 };
    QVERIFY(!AppearanceMath::normalize(&zero));

    AppearanceEmbedding x;
    x.values = { 1.0, 0.0 };
    double similarity = 0.0;
    QVERIFY(AppearanceMath::cosineSimilarity(a, x, &similarity));
    QVERIFY(qAbs(similarity - 0.6) < 1e-9);
    AppearanceEmbedding negative;
    negative.values = { -1.0, 0.0 };
    QVERIFY(AppearanceMath::cosineSimilarity(x, negative, &similarity));
    QVERIFY(qAbs(similarity + 1.0) < 1e-9);
    AppearanceEmbedding y;
    y.values = { 0.0, 1.0 };
    QVERIFY(AppearanceMath::cosineSimilarity(x, y, &similarity));
    QVERIFY(qAbs(similarity) < 1e-9);
    AppearanceEmbedding three;
    three.values = { 1.0, 0.0, 0.0 };
    QVERIFY(!AppearanceMath::cosineSimilarity(x, three, &similarity));
}

void ProjectTest::appearanceAggregateAveragesAndNormalizes()
{
    AppearanceEmbedding first;
    first.values = { 1.0, 0.0 };
    first.provider = QStringLiteral("color");
    AppearanceEmbedding second;
    second.values = { 0.0, 1.0 };
    AppearanceEmbedding aggregated;
    QVERIFY(AppearanceMath::aggregate({ first, second }, &aggregated));
    QVERIFY(qAbs(aggregated.values.at(0) - aggregated.values.at(1)) < 1e-9);
    QVERIFY(qAbs(std::sqrt(aggregated.values.at(0) * aggregated.values.at(0)
                           + aggregated.values.at(1) * aggregated.values.at(1))
                 - 1.0) < 1e-9);
    QVERIFY(!AppearanceMath::aggregate({}, &aggregated));
    AppearanceEmbedding three;
    three.values = { 1.0, 0.0, 0.0 };
    QVERIFY(!AppearanceMath::aggregate({ first, three }, &aggregated));
}

void ProjectTest::appearanceVerdictAndEvidenceJson()
{
    QCOMPARE(appearanceVerdictToString(AppearanceVerdict::Agree),
             QStringLiteral("agree"));
    QCOMPARE(appearanceVerdictFromString(QStringLiteral("disagree")),
             AppearanceVerdict::Disagree);
    QCOMPARE(appearanceVerdictFromString(QStringLiteral("bogus")),
             AppearanceVerdict::Unavailable);

    AppearanceEvidence evidence;
    evidence.identity = QStringLiteral("me");
    evidence.candidateTrackId = QStringLiteral("t2");
    evidence.verdict = AppearanceVerdict::Weak;
    evidence.similarity = 0.65;
    evidence.acceptThreshold = 0.75;
    evidence.rejectThreshold = 0.55;
    evidence.provider = QStringLiteral("color");
    evidence.detail = QStringLiteral("candidate");
    evidence.timeMs = 5;
    AppearanceEvidence restored;
    QString error;
    QVERIFY(AppearanceEvidence::readFromJsonObject(evidence.toJsonObject(),
                                                   &restored, &error));
    QCOMPARE(restored.identity, QStringLiteral("me"));
    QCOMPARE(restored.candidateTrackId, QStringLiteral("t2"));
    QVERIFY(restored.verdict == AppearanceVerdict::Weak);
    QVERIFY(qAbs(restored.similarity - 0.65) < 1e-12);
    QCOMPARE(restored.provider, QStringLiteral("color"));
}

void ProjectTest::appearanceProfileJsonRoundTrip()
{
    AppearanceProfile profile;
    profile.identity = QStringLiteral("me");
    profile.reference.values = { 1.0, 0.0 };
    profile.reference.provider = QStringLiteral("color");
    profile.sampleCount = 3;
    profile.updatedAtMs = 42;
    profile.provider = QStringLiteral("color");
    AppearanceProfile restored;
    QString error;
    QVERIFY2(AppearanceProfile::readFromJsonObject(profile.toJsonObject(),
                                                   &restored, &error),
             qPrintable(error));
    QCOMPARE(restored.identity, QStringLiteral("me"));
    QCOMPARE(restored.sampleCount, 3);
    QVERIFY(qAbs(restored.updatedAtMs - 42) < 1e-9);
    QCOMPARE(restored.reference.dimension(), 2);
}

void ProjectTest::appearanceProcessParseResponse()
{
    AppearanceEmbedding out;
    QString error;
    QVERIFY(ProcessAppearanceProvider::parseResponse(
        "{\"embedding\":[1,2,3],\"quality\":0.9}", 0, &out, &error));
    QCOMPARE(out.dimension(), 3);
    QVERIFY(!ProcessAppearanceProvider::parseResponse("not json", 0, &out, &error));
    QVERIFY(!ProcessAppearanceProvider::parseResponse("{}", 0, &out, &error));
    QVERIFY(!ProcessAppearanceProvider::parseResponse(
        "{\"embedding\":[1,2,3]}", 4, &out, &error));
    QVERIFY(!ProcessAppearanceProvider::parseResponse(
        "{\"embedding\":[\"x\"]}", 0, &out, &error));
}

void ProjectTest::appearanceProcessRunsHelper()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString script = directory.filePath(QStringLiteral("helper.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\n"
               "cat > \"$2\" <<'EOF'\n"
               "{\"embedding\":[0.6,0.8],\"quality\":1.0,\"provider\":\"shell\"}\n"
               "EOF\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessAppearanceProvider provider(QStringLiteral("/bin/sh"), { script });
    provider.setExpectedDimension(2);
    QImage crop(128, 256, QImage::Format_ARGB32);
    crop.fill(QColor(0, 0, 0));
    AppearanceEmbedding out;
    QString error;
    QVERIFY2(provider.encode(crop, QStringLiteral("t1"), 0, &out, &error),
             qPrintable(error));
    QCOMPARE(out.dimension(), 2);
    QVERIFY(qAbs(out.values.at(1) - 0.8) < 1e-9);
}

void ProjectTest::appearanceProcessFailsOnMissingExecutable()
{
    ProcessAppearanceProvider provider(QStringLiteral("/nonexistent/rc-reid-xyz"));
    QImage crop(32, 32, QImage::Format_ARGB32);
    crop.fill(QColor(0, 0, 0));
    AppearanceEmbedding out;
    QString error;
    QVERIFY(!provider.encode(crop, QStringLiteral("t1"), 0, &out, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::appearanceProcessFailsOnBadExit()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString script = directory.filePath(QStringLiteral("bad.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\nexit 3\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessAppearanceProvider provider(QStringLiteral("/bin/sh"), { script });
    QImage crop(32, 32, QImage::Format_ARGB32);
    crop.fill(QColor(0, 0, 0));
    AppearanceEmbedding out;
    QString error;
    QVERIFY(!provider.encode(crop, QStringLiteral("t1"), 0, &out, &error));
    QVERIFY(error.contains(QStringLiteral("failed")));
}

void ProjectTest::appearanceTargetCropIsDeterministic()
{
    const QImage equirect = buildTargetEquirect(
        360, 180, { EquirectDisk{ 10.0, 0.0, 30.0, QColor(255, 0, 0) } });
    TargetCropExtractor::Config config;
    config.width = 128;
    config.height = 256;
    QImage first;
    QImage second;
    QString error;
    QVERIFY(TargetCropExtractor::crop(equirect, 10.0, 0.0, 30.0, 40.0, config,
                                      &first, &error));
    QVERIFY(TargetCropExtractor::crop(equirect, 10.0, 0.0, 30.0, 40.0, config,
                                      &second, &error));
    QCOMPARE(first.size(), QSize(128, 256));
    QVERIFY(imagesIdentical(first, second));
    QVERIFY(first.pixelColor(64, 128).red() > 200);
}

void ProjectTest::appearanceRegistryProfileAndJson()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -28.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    AppearanceProfile profile;
    profile.identity = QStringLiteral("me");
    profile.reference.values = { 1.0, 0.0 };
    profile.reference.provider = QStringLiteral("color");
    profile.sampleCount = 2;
    profile.updatedAtMs = 10;
    profile.provider = QStringLiteral("color");
    registry.setAppearanceProfile(QStringLiteral("me"), profile);
    QVERIFY(registry.hasAppearanceProfile(QStringLiteral("me")));
    QCOMPARE(registry.appearanceProfile(QStringLiteral("me"))->sampleCount, 2);

    TargetIdentityRegistry restored;
    QVERIFY2(restored.readFromJsonObject(registry.toJsonObject(), &error),
             qPrintable(error));
    QVERIFY(restored.hasAppearanceProfile(QStringLiteral("me")));
    QCOMPARE(restored.appearanceProfile(QStringLiteral("me"))->sampleCount, 2);
    QCOMPARE(restored.appearanceProfile(QStringLiteral("me"))->reference.dimension(), 2);
}

void ProjectTest::appearanceRegistryRejectPreventsGeometricRebind()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0),
                      makeTargetObservation(500, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack continuation = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 0.0, 0.0) });
    registry.update({ inactive, continuation }, 1000);
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t2"));

    registry.rejectTarget(QStringLiteral("me"), QStringLiteral("t2"),
                          QStringLiteral("appearance conflict"));
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    registry.update({ inactive, continuation }, 2000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
}

void ProjectTest::appearanceRegistryRebindWithAppearance()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack returning = makeIdTrack(
        QStringLiteral("t10"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    registry.update({ inactive, returning }, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));

    QVERIFY(registry.rebindWithAppearance(QStringLiteral("me"),
                                          QStringLiteral("t10"), 0.95,
                                          QStringLiteral("test"), &error));
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t10"));
    QCOMPARE(registry.binding(QStringLiteral("me"))->method,
             QStringLiteral("appearance-rebind"));
    QVERIFY(qAbs(registry.binding(QStringLiteral("me"))->appearanceSimilarity - 0.95)
            < 1e-9);
}

void ProjectTest::appearanceReidentifierStrongReacquire()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 0);
    QVERIFY(registry.hasAppearanceProfile(QStringLiteral("me")));

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack returning = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, returning }, &frames,
                                &provider, 1000);
    QVERIFY2(registry.isResolved(QStringLiteral("me")),
             qPrintable(result.notes.join(QStringLiteral("; "))));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t2"));
    QCOMPARE(registry.binding(QStringLiteral("me"))->method,
             QStringLiteral("appearance-rebind"));
}

void ProjectTest::appearanceReidentifierWeakMatchStaysUnresolved()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 255, 150) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 0);

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack returning = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, returning }, &frames,
                                &provider, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    bool sawWeak = false;
    for (const AppearanceEvidence &evidence : result.evidence) {
        if (evidence.verdict == AppearanceVerdict::Weak) {
            sawWeak = true;
        }
    }
    QVERIFY(sawWeak);
}

void ProjectTest::appearanceReidentifierAmbiguousCandidates()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) },
                            EquirectDisk{ -80.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 0);

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack first = makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                                          { makeTargetObservation(1000, 80.0, 0.0) });
    const TargetTrack second = makeIdTrack(QStringLiteral("t3"), QStringLiteral("person"),
                                           { makeTargetObservation(1000, -80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, first, second }, &frames,
                                &provider, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    int strong = 0;
    for (const AppearanceEvidence &evidence : result.evidence) {
        if (evidence.verdict == AppearanceVerdict::Agree) {
            ++strong;
        }
    }
    QVERIFY(strong >= 2);
    QVERIFY(result.notes.join(QStringLiteral("\n"))
                .contains(QStringLiteral("ambiguous")));
}

void ProjectTest::appearanceReidentifierWrongPersonRejected()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(0, 0, 255) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 0);

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack stranger = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, stranger }, &frames,
                                &provider, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    bool sawDisagree = false;
    for (const AppearanceEvidence &evidence : result.evidence) {
        if (evidence.verdict == AppearanceVerdict::Disagree) {
            sawDisagree = true;
        }
    }
    QVERIFY(sawDisagree);
}

void ProjectTest::appearanceReidentifierGeometryAppearanceConflict()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(500, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(0, 0, 255) },
                            EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0),
                      makeTargetObservation(500, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 500);

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack geometric = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 0.0, 0.0) });
    const TargetTrack appearance = makeIdTrack(
        QStringLiteral("t3"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, geometric, appearance },
                                &frames, &provider, 1000);

    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t2"));
    QVERIFY(registry.binding(QStringLiteral("me"))->rejectedTargetIds
                .contains(QStringLiteral("t2")));
    QVERIFY(result.notes.join(QStringLiteral("\n"))
                .contains(QStringLiteral("Conflict")));
}

void ProjectTest::appearanceReidentifierExplicitSelectionPrecedence()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) },
                            EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 80.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    reidentifier.reidentify(&registry, tracks, &frames, &provider, 1000);
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t1"));
}

void ProjectTest::appearanceReidentifierMissingProvider()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    IdentityReidentifier reidentifier;
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, tracks, &frames, nullptr, 0);
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QVERIFY(result.evidence.isEmpty());
    QVERIFY(!result.notes.isEmpty());
}

void ProjectTest::appearanceReidentifierProviderFailure()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 0);
    QVERIFY(registry.hasAppearanceProfile(QStringLiteral("me")));

    provider.setFail(true);
    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack returning = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 80.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, returning }, &frames,
                                &provider, 1000);
    QVERIFY(!registry.isResolved(QStringLiteral("me")));
    bool sawUnavailable = false;
    for (const AppearanceEvidence &evidence : result.evidence) {
        if (evidence.verdict == AppearanceVerdict::Unavailable) {
            sawUnavailable = true;
        }
    }
    QVERIFY(sawUnavailable);
}

void ProjectTest::appearanceReidentifierCandidateOrderDeterministic()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 80.0, 0.0, 30.0, QColor(255, 0, 0) },
                            EquirectDisk{ -80.0, 0.0, 30.0, QColor(0, 0, 255) } });
    ColorAppearanceProvider provider;

    const auto run = [&frames, &provider](bool reversed) {
        IdentityReidentifier reidentifier;
        TargetIdentityRegistry registry;
        QString error;
        const QList<TargetTrack> initial = {
            makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                        { makeTargetObservation(0, 0.0, 0.0) })
        };
        registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                             initial, &error);
        reidentifier.reidentify(&registry, initial, &frames, &provider, 0);
        const TargetTrack inactive = inactiveCopy(initial.at(0));
        const TargetTrack match = makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                                              { makeTargetObservation(1000, 80.0, 0.0) });
        const TargetTrack decoy = makeIdTrack(QStringLiteral("t3"), QStringLiteral("person"),
                                              { makeTargetObservation(1000, -80.0, 0.0) });
        const QList<TargetTrack> tracks = reversed ? QList<TargetTrack>{ inactive, decoy, match }
                                                   : QList<TargetTrack>{ inactive, match, decoy };
        reidentifier.reidentify(&registry, tracks, &frames, &provider, 1000);
        return registry.targetId(QStringLiteral("me"));
    };
    const QString first = run(false);
    const QString second = run(true);
    QCOMPARE(first, QStringLiteral("t2"));
    QCOMPARE(second, QStringLiteral("t2"));
}

void ProjectTest::appearanceReidentifierGeometryConfirmsAppearance()
{
    ScriptedFrameProvider frames;
    frames.addFrame(0, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(500, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    frames.addFrame(1000, { EquirectDisk{ 0.0, 0.0, 30.0, QColor(255, 0, 0) } });
    ColorAppearanceProvider provider;
    IdentityReidentifier reidentifier;

    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> initial = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0),
                      makeTargetObservation(500, 0.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 initial, &error));
    reidentifier.reidentify(&registry, initial, &frames, &provider, 500);

    const TargetTrack inactive = inactiveCopy(initial.at(0));
    const TargetTrack continuation = makeIdTrack(
        QStringLiteral("t2"), QStringLiteral("person"),
        { makeTargetObservation(1000, 0.0, 0.0) });
    const IdentityReidentifier::Result result =
        reidentifier.reidentify(&registry, { inactive, continuation }, &frames,
                                &provider, 1000);
    QVERIFY2(registry.isResolved(QStringLiteral("me")),
             qPrintable(result.notes.join(QStringLiteral("; "))));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t2"));
    QCOMPARE(registry.binding(QStringLiteral("me"))->method,
             QStringLiteral("continuity-rebind"));
    QCOMPARE(registry.binding(QStringLiteral("me"))->appearanceVerdict,
             QStringLiteral("agree"));
}

// ================= 360 speaker / audio-visual association (Phase 4, Obj 6) =================
// Deterministic, model-free tests for the speaker evidence model, the
// subprocess provider, association, temporal hysteresis, orchestration, and the
// speaker-follow planner. Real Silero VAD runs only in the integration test.

namespace {

SpeakerInterval makeSpeakerInterval(qint64 startMs, qint64 endMs,
                                    const QString &id = QStringLiteral("spk1"),
                                    double confidence = 0.9,
                                    bool overlap = false)
{
    SpeakerInterval interval;
    interval.startMs = startMs;
    interval.endMs = endMs;
    interval.speakerId = id;
    interval.confidence = confidence;
    interval.overlap = overlap;
    return interval;
}

class ScriptedSpeakerProvider : public SpeakerEvidenceProvider
{
public:
    void setIntervals(const QList<SpeakerInterval> &intervals)
    {
        m_analysis = SpeakerAnalysis();
        m_analysis.available = true;
        m_analysis.provider = QStringLiteral("scripted");
        m_analysis.startMs = 0;
        m_analysis.endMs = 10000;
        m_analysis.intervals = intervals;
    }
    void setUnavailable(const QString &error)
    {
        m_analysis = SpeakerAnalysis();
        m_analysis.available = false;
        m_analysis.provider = QStringLiteral("scripted");
        m_analysis.error = error;
    }
    void setFail(bool fail) { m_fail = fail; }
    QString name() const override { return QStringLiteral("scripted"); }

    bool analyze(const QString &, qint64, qint64, SpeakerAnalysis *out,
                 QString *error) override
    {
        if (error) {
            error->clear();
        }
        if (m_fail) {
            if (error) {
                *error = QStringLiteral("speaker provider failure");
            }
            return false;
        }
        if (out) {
            *out = m_analysis;
        }
        return true;
    }

private:
    SpeakerAnalysis m_analysis;
    bool m_fail = false;
};

int activeSegmentCount(const QList<SpeakerSegment> &segments)
{
    int count = 0;
    for (const SpeakerSegment &segment : segments) {
        if (segment.verdict == SpeakerVerdict::Active) {
            ++count;
        }
    }
    return count;
}

QList<SpeakerSegment> activeSegments(const QList<SpeakerSegment> &segments)
{
    QList<SpeakerSegment> active;
    for (const SpeakerSegment &segment : segments) {
        if (segment.verdict == SpeakerVerdict::Active) {
            active.append(segment);
        }
    }
    return active;
}

} // namespace

void ProjectTest::speakerIntervalAndAnalysisJson()
{
    const SpeakerInterval interval = makeSpeakerInterval(100, 900);
    SpeakerInterval restored;
    QString error;
    QVERIFY(SpeakerInterval::readFromJsonObject(interval.toJsonObject(),
                                                &restored, &error));
    QCOMPARE(restored.startMs, qint64(100));
    QCOMPARE(restored.endMs, qint64(900));
    QCOMPARE(restored.speakerId, QStringLiteral("spk1"));

    SpeakerAnalysis analysis;
    analysis.available = true;
    analysis.provider = QStringLiteral("p");
    analysis.startMs = 0;
    analysis.endMs = 1000;
    analysis.intervals.append(interval);
    SpeakerAnalysis analysisBack;
    QVERIFY(SpeakerAnalysis::readFromJsonObject(analysis.toJsonObject(),
                                                &analysisBack, &error));
    QCOMPARE(analysisBack.intervals.size(), 1);
    QCOMPARE(analysisBack.provider, QStringLiteral("p"));

    SpeakerAnalysis unavailable;
    unavailable.available = false;
    unavailable.startMs = 0;
    unavailable.endMs = 1000;
    QVERIFY(unavailable.isValid(&error));
}

void ProjectTest::speakerVerdictEvidenceAndSegmentJson()
{
    QCOMPARE(speakerVerdictToString(SpeakerVerdict::Active),
             QStringLiteral("active"));
    QCOMPARE(speakerVerdictFromString(QStringLiteral("overlap")),
             SpeakerVerdict::Overlap);
    QCOMPARE(speakerVerdictFromString(QStringLiteral("bogus")),
             SpeakerVerdict::Unavailable);

    SpeakerEvidence evidence;
    evidence.targetId = QStringLiteral("t1");
    evidence.speakerId = QStringLiteral("spk1");
    evidence.verdict = SpeakerVerdict::Active;
    evidence.confidence = 0.9;
    evidence.startMs = 0;
    evidence.endMs = 500;
    evidence.provider = QStringLiteral("p");
    evidence.detail = QStringLiteral("d");
    SpeakerEvidence evidenceBack;
    QString error;
    QVERIFY(SpeakerEvidence::readFromJsonObject(evidence.toJsonObject(),
                                                &evidenceBack, &error));
    QCOMPARE(evidenceBack.targetId, QStringLiteral("t1"));
    QVERIFY(evidenceBack.verdict == SpeakerVerdict::Active);

    SpeakerSegment segment;
    segment.startMs = 0;
    segment.endMs = 500;
    segment.speakerId = QStringLiteral("spk1");
    segment.targetId = QStringLiteral("t1");
    segment.verdict = SpeakerVerdict::Active;
    segment.confidence = 0.8;
    SpeakerSegment segmentBack;
    QVERIFY(SpeakerSegment::readFromJsonObject(segment.toJsonObject(),
                                               &segmentBack, &error));
    QVERIFY(segmentBack.verdict == SpeakerVerdict::Active);
    QCOMPARE(segmentBack.targetId, QStringLiteral("t1"));
}

void ProjectTest::speakerIntervalRejectsInvalid()
{
    QString error;
    SpeakerInterval empty;
    QVERIFY(!empty.isValid(&error));
    SpeakerInterval reversed = makeSpeakerInterval(900, 100);
    QVERIFY(!reversed.isValid(&error));
    SpeakerInterval badConfidence = makeSpeakerInterval(100, 900);
    badConfidence.confidence = 2.0;
    QVERIFY(!badConfidence.isValid(&error));

    SpeakerAnalysis analysis;
    analysis.available = true;
    analysis.startMs = 0;
    analysis.endMs = 1000;
    analysis.intervals.append(reversed);
    QVERIFY(!analysis.isValid(&error));

    SpeakerAnalysis out;
    QVERIFY(!SpeakerAnalysis::readFromJsonObject(QJsonObject(), &out, &error));
}

void ProjectTest::speakerProcessParseResponse()
{
    SpeakerAnalysis out;
    QString error;
    QVERIFY(ProcessSpeakerProvider::parseResponse(
        "{\"available\":true,\"provider\":\"p\",\"startMs\":0,\"endMs\":1000,"
        "\"intervals\":[{\"startMs\":0,\"endMs\":500,\"speakerId\":\"a\","
        "\"confidence\":0.9}]}",
        &out, &error));
    QCOMPARE(out.intervals.size(), 1);
    QVERIFY(!ProcessSpeakerProvider::parseResponse("not json", &out, &error));
    QVERIFY(!ProcessSpeakerProvider::parseResponse(
        "{\"available\":true,\"startMs\":0,\"endMs\":0}", &out, &error));
    QVERIFY(!ProcessSpeakerProvider::parseResponse(
        "{\"available\":true,\"provider\":\"p\",\"startMs\":0,\"endMs\":1000,"
        "\"intervals\":[{\"startMs\":500,\"endMs\":100,\"speakerId\":\"a\","
        "\"confidence\":0.9}]}",
        &out, &error));
}

void ProjectTest::speakerProcessRunsHelper()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString media = directory.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(media);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write("x");
    mediaFile.close();

    const QString script = directory.filePath(QStringLiteral("helper.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\n"
               "cat > \"$2\" <<'EOF'\n"
               "{\"available\":true,\"provider\":\"shell\",\"startMs\":0,"
               "\"endMs\":1000,\"intervals\":[{\"startMs\":0,\"endMs\":500,"
               "\"speakerId\":\"spk1\",\"confidence\":0.9}]}\n"
               "EOF\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessSpeakerProvider provider(QStringLiteral("/bin/sh"), { script });
    SpeakerAnalysis out;
    QString error;
    QVERIFY2(provider.analyze(media, 0, 1000, &out, &error), qPrintable(error));
    QCOMPARE(out.intervals.size(), 1);
    QCOMPARE(out.provider, QStringLiteral("shell"));
}

void ProjectTest::speakerProcessFailsOnMissingExecutable()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString media = directory.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(media);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write("x");
    mediaFile.close();

    ProcessSpeakerProvider provider(QStringLiteral("/nonexistent/speaker-xyz"));
    SpeakerAnalysis out;
    QString error;
    QVERIFY(!provider.analyze(media, 0, 1000, &out, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::speakerProcessFailsOnBadExit()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString media = directory.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(media);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write("x");
    mediaFile.close();
    const QString script = directory.filePath(QStringLiteral("bad.sh"));
    QFile file(script);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("#!/bin/sh\nexit 3\n");
    file.close();
    QVERIFY(QFile::setPermissions(
        script, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ExeOwner));

    ProcessSpeakerProvider provider(QStringLiteral("/bin/sh"), { script });
    SpeakerAnalysis out;
    QString error;
    QVERIFY(!provider.analyze(media, 0, 1000, &out, &error));
    QVERIFY(error.contains(QStringLiteral("failed")));
}

void ProjectTest::speakerAssociatorExplicitBinding()
{
    SpeakerTargetAssociator associator;
    associator.bind(QStringLiteral("spk1"), QStringLiteral("t2"));
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 50.0, 0.0) })
    };
    QString target;
    QString method;
    bool ambiguous = false;
    QString error;
    QVERIFY(associator.associate(makeSpeakerInterval(0, 500), tracks,
                                 SpeakerTargetAssociator::Config{}, &target,
                                 &ambiguous, &method, &error));
    QCOMPARE(target, QStringLiteral("t2"));
    QCOMPARE(method, QStringLiteral("explicit"));
    QVERIFY(!ambiguous);
}

void ProjectTest::speakerAssociatorExplicitBindingNotVisible()
{
    SpeakerTargetAssociator associator;
    associator.bind(QStringLiteral("spk1"), QStringLiteral("t9"));
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QString target;
    QString method;
    bool ambiguous = false;
    QString error;
    QVERIFY(associator.associate(makeSpeakerInterval(0, 500), tracks,
                                 SpeakerTargetAssociator::Config{}, &target,
                                 &ambiguous, &method, &error));
    QVERIFY(target.isEmpty());
    QVERIFY(!ambiguous);
}

void ProjectTest::speakerAssociatorSingleVisible()
{
    SpeakerTargetAssociator associator;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    QString target;
    QString method;
    bool ambiguous = false;
    QString error;
    QVERIFY(associator.associate(makeSpeakerInterval(0, 500), tracks,
                                 SpeakerTargetAssociator::Config{}, &target,
                                 &ambiguous, &method, &error));
    QCOMPARE(target, QStringLiteral("t1"));
    QCOMPARE(method, QStringLiteral("single-visible"));
}

void ProjectTest::speakerAssociatorMultipleVisibleIsAmbiguous()
{
    SpeakerTargetAssociator associator;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 50.0, 0.0) })
    };
    QString target;
    QString method;
    bool ambiguous = false;
    QString error;
    QVERIFY(associator.associate(makeSpeakerInterval(0, 500), tracks,
                                 SpeakerTargetAssociator::Config{}, &target,
                                 &ambiguous, &method, &error));
    QVERIFY(target.isEmpty());
    QVERIFY(ambiguous);
}

void ProjectTest::speakerAssociatorSpatialAzimuth()
{
    SpeakerTargetAssociator associator;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -10.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0) })
    };
    SpeakerInterval interval = makeSpeakerInterval(0, 500);
    interval.hasAzimuth = true;
    interval.azimuthDeg = 35.0;
    QString target;
    QString method;
    bool ambiguous = false;
    QString error;
    QVERIFY(associator.associate(interval, tracks,
                                 SpeakerTargetAssociator::Config{}, &target,
                                 &ambiguous, &method, &error));
    QCOMPARE(target, QStringLiteral("t2"));
    QCOMPARE(method, QStringLiteral("spatial"));
}

void ProjectTest::speakerTimelineSingleSpeaker()
{
    SpeakerTimeline::Config config;
    const QList<SpeakerSegment> segments = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 2000) }, config);
    QCOMPARE(segments.size(), 1);
    QVERIFY(segments.at(0).verdict == SpeakerVerdict::Active);
    QCOMPARE(segments.at(0).speakerId, QStringLiteral("spk1"));
    QCOMPARE(segments.at(0).endMs, qint64(2000));
}

void ProjectTest::speakerTimelineSpeakerChange()
{
    SpeakerTimeline::Config config;
    const QList<SpeakerSegment> segments = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 2000, QStringLiteral("A")),
          makeSpeakerInterval(3000, 5000, QStringLiteral("B")) },
        config);
    const QList<SpeakerSegment> active = activeSegments(segments);
    QCOMPARE(active.size(), 2);
    QCOMPARE(active.at(0).speakerId, QStringLiteral("A"));
    QCOMPARE(active.at(1).speakerId, QStringLiteral("B"));
}

void ProjectTest::speakerTimelineShortPauseIsHeld()
{
    SpeakerTimeline::Config config;
    const QList<SpeakerSegment> segments = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 2000, QStringLiteral("A")),
          makeSpeakerInterval(2400, 4000, QStringLiteral("A")) },
        config);
    const QList<SpeakerSegment> active = activeSegments(segments);
    QCOMPARE(active.size(), 1);
    QCOMPARE(active.at(0).endMs, qint64(4000));
}

void ProjectTest::speakerTimelineRapidAlternationDoesNotThrash()
{
    SpeakerTimeline::Config config;
    const QList<SpeakerSegment> segments = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 2000, QStringLiteral("A")),
          makeSpeakerInterval(2100, 2500, QStringLiteral("B")),
          makeSpeakerInterval(2600, 4000, QStringLiteral("A")) },
        config);
    QCOMPARE(activeSegmentCount(segments), 1);
    QCOMPARE(segments.at(0).speakerId, QStringLiteral("A"));
}

void ProjectTest::speakerTimelineOverlapIsPreserved()
{
    SpeakerTimeline::Config config;
    const QList<SpeakerSegment> segments = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 3000, QStringLiteral("A")),
          makeSpeakerInterval(500, 3500, QStringLiteral("B"), 0.9, true) },
        config);
    bool sawOverlap = false;
    for (const SpeakerSegment &segment : segments) {
        if (segment.verdict == SpeakerVerdict::Overlap) {
            sawOverlap = true;
        }
    }
    QVERIFY(sawOverlap);
}

void ProjectTest::speakerTimelineFiltersShortAndLowConfidence()
{
    SpeakerTimeline::Config config;
    QVERIFY(SpeakerTimeline::build({ makeSpeakerInterval(0, 200) }, config).isEmpty());
    QVERIFY(SpeakerTimeline::build(
                { makeSpeakerInterval(0, 2000, QStringLiteral("spk1"), 0.3) },
                config)
                .isEmpty());
    QVERIFY(SpeakerTimeline::build({}, config).isEmpty());
}

void ProjectTest::speakerAnalyzerAssociatesSingleSpeaker()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 2000) });
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0),
                      makeTargetObservation(2000, -20.0, 0.0) })
    };
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 2000, tracks, &provider,
        SpeakerTargetAssociator());
    QVERIFY(result.analysis.available);
    QCOMPARE(result.evidence.size(), 1);
    QCOMPARE(result.evidence.at(0).targetId, QStringLiteral("t1"));
    QVERIFY(result.evidence.at(0).verdict == SpeakerVerdict::Active);
    // The association is also recorded on the timeline segment so the planner
    // can consume it.
    QCOMPARE(result.segments.size(), 1);
    QCOMPARE(result.segments.at(0).targetId, QStringLiteral("t1"));
}

void ProjectTest::speakerAnalyzerExplicitBindingWithTwoVisible()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 2000) });
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0) })
    };
    SpeakerTargetAssociator associator;
    associator.bind(QStringLiteral("spk1"), QStringLiteral("t2"));
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 2000, tracks, &provider, associator);
    QCOMPARE(result.evidence.size(), 1);
    QCOMPARE(result.evidence.at(0).targetId, QStringLiteral("t2"));
}

void ProjectTest::speakerAnalyzerWithoutProviderIsUnavailable()
{
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 1000, tracks, nullptr,
        SpeakerTargetAssociator());
    QVERIFY(!result.analysis.available);
    QVERIFY(result.evidence.isEmpty());
    QVERIFY(!result.notes.isEmpty());
}

void ProjectTest::speakerAnalyzerProviderFailureIsFailSafe()
{
    ScriptedSpeakerProvider provider;
    provider.setFail(true);
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 1000, tracks, &provider,
        SpeakerTargetAssociator());
    QVERIFY(!result.analysis.available);
    QVERIFY(result.evidence.isEmpty());
    QVERIFY(result.notes.join(QStringLiteral("\n"))
                .contains(QStringLiteral("failed")));
}

void ProjectTest::speakerAnalyzerUnassociatedSpeechDoesNotInventTarget()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 2000) });
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0) })
    };
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 2000, tracks, &provider,
        SpeakerTargetAssociator());
    QCOMPARE(result.evidence.size(), 1);
    QVERIFY(result.evidence.at(0).targetId.isEmpty());
    QVERIFY(result.evidence.at(0).verdict == SpeakerVerdict::Ambiguous);
}

void ProjectTest::speakerAnalyzerTrackingLossKeepsNoTarget()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 2000) });
    TargetTrack bound = makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                                    { makeTargetObservation(0, -20.0, 0.0) });
    bound.setActive(false);
    const QList<TargetTrack> tracks = {
        bound,
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0) })
    };
    SpeakerTargetAssociator associator;
    associator.bind(QStringLiteral("spk1"), QStringLiteral("t1"));
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 2000, tracks, &provider, associator);
    QCOMPARE(result.evidence.size(), 1);
    QVERIFY(result.evidence.at(0).targetId.isEmpty());
    QVERIFY(result.evidence.at(0).verdict == SpeakerVerdict::Unassociated);
}

void ProjectTest::speakerAnalyzerOverlapIsPreserved()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 3000, QStringLiteral("A")),
                            makeSpeakerInterval(500, 3500, QStringLiteral("B"),
                                                0.9, true) });
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, 0.0, 0.0) })
    };
    SpeakerEvidenceAnalyzer analyzer;
    const auto result = analyzer.analyze(
        QStringLiteral("dummy"), 0, 3500, tracks, &provider,
        SpeakerTargetAssociator());
    bool sawOverlap = false;
    for (const SpeakerEvidence &evidence : result.evidence) {
        if (evidence.verdict == SpeakerVerdict::Overlap) {
            sawOverlap = true;
        }
    }
    QVERIFY(sawOverlap);
}

void ProjectTest::speakerAnalyzerIsDeterministic()
{
    ScriptedSpeakerProvider provider;
    provider.setIntervals({ makeSpeakerInterval(0, 2000, QStringLiteral("A")),
                            makeSpeakerInterval(2200, 4000, QStringLiteral("B"),
                                                0.9) });
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0),
                      makeTargetObservation(4000, -20.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0),
                      makeTargetObservation(4000, 40.0, 0.0) })
    };
    SpeakerTargetAssociator associator;
    associator.bind(QStringLiteral("A"), QStringLiteral("t1"));
    associator.bind(QStringLiteral("B"), QStringLiteral("t2"));
    SpeakerEvidenceAnalyzer analyzer;
    const auto first = analyzer.analyze(QStringLiteral("dummy"), 0, 4000, tracks,
                                        &provider, associator);
    const auto second = analyzer.analyze(QStringLiteral("dummy"), 0, 4000, tracks,
                                         &provider, associator);
    QCOMPARE(first.segments.size(), second.segments.size());
    QCOMPARE(first.evidence.size(), second.evidence.size());
    for (int i = 0; i < first.evidence.size(); ++i) {
        QCOMPARE(first.evidence.at(i).targetId, second.evidence.at(i).targetId);
        QVERIFY(first.evidence.at(i).verdict == second.evidence.at(i).verdict);
    }

    // Reversed interval input produces the same timeline (deterministic).
    const auto forward = SpeakerTimeline::build(
        { makeSpeakerInterval(0, 2000, QStringLiteral("A")),
          makeSpeakerInterval(3000, 5000, QStringLiteral("B")) },
        SpeakerTimeline::Config{});
    const auto reversed = SpeakerTimeline::build(
        { makeSpeakerInterval(3000, 5000, QStringLiteral("B")),
          makeSpeakerInterval(0, 2000, QStringLiteral("A")) },
        SpeakerTimeline::Config{});
    QCOMPARE(forward.size(), reversed.size());
    for (int i = 0; i < forward.size(); ++i) {
        QCOMPARE(forward.at(i).speakerId, reversed.at(i).speakerId);
    }
}

void ProjectTest::speakerAnalyzerEvidenceDoesNotChangeIdentity()
{
    TargetIdentityRegistry registry;
    QString error;
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(0, 40.0, 0.0) })
    };
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    registry.annotateSpeaker(QStringLiteral("me"), QStringLiteral("spk1"), 0.9,
                             SpeakerVerdict::Active,
                             QStringLiteral("speaker suggests t2"));
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t1"));
    const IdentityBinding *binding = registry.binding(QStringLiteral("me"));
    QVERIFY(binding != nullptr);
    QCOMPARE(binding->speakerId, QStringLiteral("spk1"));
    QCOMPARE(binding->speakerVerdict, QStringLiteral("active"));
}

void ProjectTest::speakerPlannerFollowsSingleSpeaker()
{
    const QList<SpeakerSegment> segments = {
        SpeakerSegment{ 0, 2000, QStringLiteral("spk1"), QStringLiteral("t1"),
                        SpeakerVerdict::Active, 0.9 }
    };
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0),
                      makeTargetObservation(2000, -20.0, 0.0) })
    };
    ReframePlan plan;
    QString error;
    QVERIFY2(SpeakerReframePlanner::plan(
                 segments, tracks, ReframePlan::TimeRange{ 0, 2000 },
                 ReframePlan::OutputSpec{ 160, 90, 2.0 }, {}, &plan, &error),
             qPrintable(error));
    QCOMPARE(plan.keyframes().size(), 1);
    QVERIFY(qAbs(plan.keyframes().at(0).yawDeg + 20.0) < 1e-9);
}

void ProjectTest::speakerPlannerCutsOnSpeakerChange()
{
    const QList<SpeakerSegment> segments = {
        SpeakerSegment{ 0, 1000, QStringLiteral("A"), QStringLiteral("t1"),
                        SpeakerVerdict::Active, 0.9 },
        SpeakerSegment{ 1000, 2000, QStringLiteral("B"), QStringLiteral("t2"),
                        SpeakerVerdict::Active, 0.9 }
    };
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0),
                      makeTargetObservation(1000, -20.0, 0.0) }),
        makeIdTrack(QStringLiteral("t2"), QStringLiteral("person"),
                    { makeTargetObservation(1000, 40.0, 0.0),
                      makeTargetObservation(2000, 40.0, 0.0) })
    };
    ReframePlan plan;
    QString error;
    QVERIFY2(SpeakerReframePlanner::plan(
                 segments, tracks, ReframePlan::TimeRange{ 0, 2000 },
                 ReframePlan::OutputSpec{ 160, 90, 2.0 }, {}, &plan, &error),
             qPrintable(error));
    QCOMPARE(plan.keyframes().size(), 3);
    const CameraState before = CameraPath::stateAt(plan, 999);
    const CameraState atSwitch = CameraPath::stateAt(plan, 1000);
    QVERIFY(qAbs(before.yawDeg + 20.0) < 1e-6);
    QVERIFY(qAbs(atSwitch.yawDeg - 40.0) < 1e-6);
}

void ProjectTest::speakerPlannerRejectsNoActiveSegment()
{
    const QList<SpeakerSegment> segments = {
        SpeakerSegment{ 0, 1000, QString(), QString(),
                        SpeakerVerdict::Silence, 0.0 }
    };
    ReframePlan plan;
    QString error;
    QVERIFY(!SpeakerReframePlanner::plan(
        segments, {}, ReframePlan::TimeRange{ 0, 1000 },
        ReframePlan::OutputSpec{ 160, 90, 1.0 }, {}, &plan, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::speakerRegistryAnnotateDoesNotChangeResolution()
{
    const QList<TargetTrack> tracks = {
        makeIdTrack(QStringLiteral("t1"), QStringLiteral("person"),
                    { makeTargetObservation(0, -20.0, 0.0) })
    };
    TargetIdentityRegistry registry;
    QString error;
    QVERIFY(registry.bindToTrack(QStringLiteral("me"), QStringLiteral("t1"), 0,
                                 tracks, &error));
    registry.annotateSpeaker(QStringLiteral("me"), QStringLiteral("spk1"), 0.75,
                             SpeakerVerdict::Active,
                             QStringLiteral("speaker evidence"));
    QVERIFY(registry.isResolved(QStringLiteral("me")));
    QCOMPARE(registry.targetId(QStringLiteral("me")), QStringLiteral("t1"));

    TargetIdentityRegistry restored;
    QVERIFY2(restored.readFromJsonObject(registry.toJsonObject(), &error),
             qPrintable(error));
    QCOMPARE(restored.targetId(QStringLiteral("me")), QStringLiteral("t1"));
    QCOMPARE(restored.binding(QStringLiteral("me"))->speakerId,
             QStringLiteral("spk1"));
}

QTEST_MAIN(ProjectTest)
#include "test_project.moc"
