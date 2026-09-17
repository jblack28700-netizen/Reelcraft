#pragma once

#include <QString>
#include <QStringList>

#include "target/TargetDetector.h"

// ProcessTargetDetector runs an external detector helper as a subprocess over a
// small file/JSON protocol. It deliberately links no computer-vision library
// into Reelcraft, so the core build stays dependency-free and the model,
// runtime, and license can be chosen and replaced independently.
//
// Protocol (all paths are passed on the command line):
//   <executable> [arguments...] <request.json> <response.json>
//
// request.json:
//   { "image": "<abs path to PNG>", "width": N, "height": N,
//     "query": { "label": .., "targetId": .., "minConfidence": .. } }
//
// response.json:
//   { "detections": [ { "x": .., "y": .., "width": .., "height": ..,
//                       "label": "person", "confidence": 0.9, "id": ".." } ] }
//
// Coordinates are in view pixels. Malformed detections are skipped; a missing
// or non-array "detections" field, a non-zero exit, or a timeout is an error.
class ProcessTargetDetector : public TargetDetector
{
public:
    ProcessTargetDetector();
    ProcessTargetDetector(QString executable, QStringList arguments = {});

    QString name() const override;

    void setExecutable(const QString &executable);
    void setArguments(const QStringList &arguments);
    void setTimeoutMs(int timeoutMs);

    bool detect(const QImage &perspectiveView, const TargetQuery &query,
                QList<TargetDetection> *outDetections,
                QString *error = nullptr) override;

    // Parses a response JSON document. Exposed for deterministic tests.
    static bool parseResponse(const QByteArray &json, QList<TargetDetection> *out,
                              QString *error = nullptr);

private:
    QString m_executable;
    QStringList m_arguments;
    int m_timeoutMs = 30000;
};
