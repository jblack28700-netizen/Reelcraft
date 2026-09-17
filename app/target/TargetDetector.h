#pragma once

#include <QImage>
#include <QList>
#include <QString>

#include "target/TargetTypes.h"

// TargetDetector is the replaceable detection boundary for target resolution.
//
// It receives an already-extracted perspective/tangent view of the 360 source
// (never the distorted full equirectangular frame) plus a query, and returns 2D
// detections in view-pixel coordinates. The reframing engine never depends on a
// specific model or runtime: an in-process detector, a subprocess model helper,
// or a future GPU service can all implement this interface.
//
// Implementations must never modify the source media.
class TargetDetector
{
public:
    virtual ~TargetDetector() = default;

    // Human-readable backend identity (used in evidence and diagnostics).
    virtual QString name() const = 0;

    // Detects targets in one perspective view. Returns false with a
    // deterministic error on failure; an empty result list is success.
    virtual bool detect(const QImage &perspectiveView, const TargetQuery &query,
                        QList<TargetDetection> *outDetections,
                        QString *error = nullptr) = 0;

protected:
    TargetDetector() = default;
};
