#pragma once

#include <QString>

#include "media/MediaDurationProbe.h"

// FfprobeDurationProbe reports media duration using the external ffprobe
// executable (never linked) and the plain-text `format=duration` value. This
// mirrors the FrameExtractor external-tool seam: the executable is resolved at
// runtime (REELCRAFT_FFPROBE, then a sibling of the ffmpeg executable, then
// PATH), the source is only read, and every failure is deterministic.
class FfprobeDurationProbe : public MediaDurationProbe
{
public:
    FfprobeDurationProbe();
    explicit FfprobeDurationProbe(QString executablePath);

    QString name() const override { return QStringLiteral("ffprobe"); }

    bool frameRate(const QString &filePath, double *outFps,
                   QString *error = nullptr) override;

    // Objective 21: elementary-stream summary for the media-analysis technical
    // layer. One ffprobe invocation, JSON output, no decoding.
    bool streamSummary(const QString &filePath, StreamSummary *out,
                       QString *error = nullptr) override;

    bool durationMs(const QString &filePath, qint64 *outDurationMs,
                    QString *error = nullptr) override;

    // Resolution order: REELCRAFT_FFPROBE, then <dir>/ffprobe next to the
    // resolved ffmpeg executable, then QStandardPaths. Empty when not found.
    static QString defaultExecutablePath();

    // Parses ffprobe's plain-text seconds value (for example "12.012000").
    // Exposed so the parsing contract is unit-testable without a process.
    static bool parseDurationOutput(const QString &output, qint64 *outDurationMs,
                                    QString *error = nullptr);

    // Parses ffprobe's JSON stream listing (exposed so the parsing contract is
    // unit-testable without a process). Reports only what the file actually
    // declares: an absent video stream is hasVideo == false, never a default
    // geometry.
    static bool parseStreamSummary(const QByteArray &json,
                                   StreamSummary *outSummary,
                                   QString *error = nullptr);

    QString executablePath() const { return m_executablePath; }

private:
    QString m_executablePath;
};
