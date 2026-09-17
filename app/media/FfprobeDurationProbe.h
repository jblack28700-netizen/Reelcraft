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

    bool durationMs(const QString &filePath, qint64 *outDurationMs,
                    QString *error = nullptr) override;

    // Resolution order: REELCRAFT_FFPROBE, then <dir>/ffprobe next to the
    // resolved ffmpeg executable, then QStandardPaths. Empty when not found.
    static QString defaultExecutablePath();

    // Parses ffprobe's plain-text seconds value (for example "12.012000").
    // Exposed so the parsing contract is unit-testable without a process.
    static bool parseDurationOutput(const QString &output, qint64 *outDurationMs,
                                    QString *error = nullptr);

    QString executablePath() const { return m_executablePath; }

private:
    QString m_executablePath;
};
