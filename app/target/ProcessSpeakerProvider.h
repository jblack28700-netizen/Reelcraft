#pragma once

#include <QString>
#include <QStringList>

#include "target/SpeakerEvidenceProvider.h"

// ProcessSpeakerProvider runs an external audio/speaker helper as a subprocess
// over a small file/JSON protocol. It links no ML/audio runtime into Reelcraft.
//
// Protocol:
//   <executable> [arguments...] <request.json> <response.json>
//
// request.json:
//   { "media": "<path>", "startMs": 114000, "endMs": 126000 }
//
// response.json:
//   { "available": true, "provider": "silero-vad", "startMs": 0, "endMs": 12000,
//     "intervals": [ { "startMs": 0, "endMs": 2100, "speakerId": "spk1",
//                      "confidence": 0.9, "overlap": false } ] }
//
// Fail-safe: a missing executable or media, non-zero exit, timeout, malformed
// JSON, a missing/invalid interval, or an out-of-range confidence is a
// deterministic error. Failures must never cause Reelcraft to guess a speaker.
// Reelcraft remains fully usable without any speaker provider.
class ProcessSpeakerProvider : public SpeakerEvidenceProvider
{
public:
    ProcessSpeakerProvider();
    ProcessSpeakerProvider(QString executable, QStringList arguments = {});

    QString name() const override;

    void setExecutable(const QString &executable);
    void setArguments(const QStringList &arguments);
    void setTimeoutMs(int timeoutMs);

    bool analyze(const QString &mediaPath, qint64 startMs, qint64 endMs,
                 SpeakerAnalysis *out, QString *error = nullptr) override;

    // Parses a response document. Exposed for deterministic tests.
    static bool parseResponse(const QByteArray &json, SpeakerAnalysis *out,
                              QString *error = nullptr);

private:
    QString m_executable;
    QStringList m_arguments;
    int m_timeoutMs = 60000;
};
