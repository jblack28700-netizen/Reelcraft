#pragma once

#include <QString>
#include <QStringList>

#include "target/AppearanceProvider.h"

// ProcessAppearanceProvider runs an external appearance/ReID helper as a
// subprocess over a small file/JSON protocol. It links no machine-learning
// runtime into Reelcraft, so the model, runtime, and license remain
// independently replaceable.
//
// Protocol:
//   <executable> [arguments...] <request.json> <response.json>
//
// request.json:
//   { "image": "<abs path to a PNG target crop>", "width": N, "height": N,
//     "targetId": "t1", "timeMs": 5500 }
//
// response.json:
//   { "embedding": [ ... ], "quality": 1.0, "provider": "reid-retail-0277" }
//
// Fail-safe behavior: a missing executable or image, non-zero exit, timeout,
// malformed JSON, a missing embedding array, non-finite values, or an embedding
// whose dimension does not match expectedDimension() is a deterministic error.
// Reelcraft remains usable without any appearance provider.
class ProcessAppearanceProvider : public AppearanceProvider
{
public:
    ProcessAppearanceProvider();
    ProcessAppearanceProvider(QString executable, QStringList arguments = {});

    QString name() const override;

    void setExecutable(const QString &executable);
    void setArguments(const QStringList &arguments);
    void setTimeoutMs(int timeoutMs);
    // Expected embedding dimension; <= 0 accepts any dimension.
    void setExpectedDimension(int dimension);
    int expectedDimension() const;

    bool encode(const QImage &crop, const QString &targetId, qint64 timeMs,
                AppearanceEmbedding *out, QString *error = nullptr) override;

    // Parses a response document. Exposed for deterministic tests.
    static bool parseResponse(const QByteArray &json, int expectedDimension,
                              AppearanceEmbedding *out, QString *error = nullptr);

private:
    QString m_executable;
    QStringList m_arguments;
    int m_timeoutMs = 30000;
    int m_expectedDimension = 0;
};
