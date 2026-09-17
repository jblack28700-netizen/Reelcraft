#pragma once

#include <QImage>
#include <QString>

#include "target/AppearanceTypes.h"

// AppearanceProvider is the replaceable appearance-inference boundary.
//
// It receives a deterministic target crop (produced by TargetCropExtractor from
// the 360 source) and returns a unit-normalized appearance embedding. The C++
// core never links Python, OpenCV, ONNX Runtime, or any embedding model: an
// in-process implementation, a subprocess model helper, or a future GPU service
// can all implement this interface.
//
// Implementations must never modify the source media and must fail
// deterministically rather than returning a fabricated embedding.
class AppearanceProvider
{
public:
    virtual ~AppearanceProvider() = default;

    virtual QString name() const = 0;

    // Encodes one crop. Returns false with a descriptive error when the crop is
    // invalid, inference fails, or the output is malformed.
    virtual bool encode(const QImage &crop, const QString &targetId, qint64 timeMs,
                        AppearanceEmbedding *out, QString *error = nullptr) = 0;

protected:
    AppearanceProvider() = default;
};
