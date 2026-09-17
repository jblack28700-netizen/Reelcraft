#include "target/AppearanceTypes.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

namespace {

constexpr double kEpsilon = 1e-9;

} // namespace

bool AppearanceEmbedding::isValid(QString *error) const
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (values.isEmpty()) {
        return fail(QStringLiteral("Appearance embedding is empty."));
    }
    for (double value : values) {
        if (!std::isfinite(value)) {
            return fail(QStringLiteral("Appearance embedding has a non-finite value."));
        }
    }
    if (!std::isfinite(quality) || quality < 0.0 || quality > 1.0) {
        return fail(QStringLiteral("Appearance embedding quality is out of range."));
    }
    return true;
}

QJsonObject AppearanceEmbedding::toJsonObject() const
{
    QJsonArray array;
    for (double value : values) {
        array.append(value);
    }
    QJsonObject object;
    object.insert(QStringLiteral("embedding"), array);
    object.insert(QStringLiteral("dimension"), dimension());
    object.insert(QStringLiteral("quality"), quality);
    if (!provider.isEmpty()) {
        object.insert(QStringLiteral("provider"), provider);
    }
    return object;
}

bool AppearanceEmbedding::readFromJsonObject(const QJsonObject &object,
                                             AppearanceEmbedding *out,
                                             QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Appearance embedding output is null."));
    }
    const QJsonValue embeddingValue = object.value(QStringLiteral("embedding"));
    if (!embeddingValue.isArray()) {
        return fail(QStringLiteral("Appearance embedding is missing its array."));
    }
    AppearanceEmbedding embedding;
    for (const QJsonValue &value : embeddingValue.toArray()) {
        if (!value.isDouble()) {
            return fail(QStringLiteral("Appearance embedding contains a non-number."));
        }
        embedding.values.append(value.toDouble());
    }
    embedding.quality = object.value(QStringLiteral("quality")).toDouble(1.0);
    embedding.provider = object.value(QStringLiteral("provider")).toString();
    if (!embedding.isValid(error)) {
        return false;
    }
    *out = embedding;
    return true;
}

bool AppearanceMath::normalize(AppearanceEmbedding *embedding, QString *error)
{
    if (error) {
        error->clear();
    }
    if (!embedding || !embedding->isValid(error)) {
        if (error && error->isEmpty()) {
            *error = QStringLiteral("Appearance embedding is invalid.");
        }
        return false;
    }
    double sumSquares = 0.0;
    for (double value : embedding->values) {
        sumSquares += value * value;
    }
    const double norm = std::sqrt(sumSquares);
    if (norm <= kEpsilon) {
        if (error) {
            *error = QStringLiteral("Appearance embedding has zero magnitude.");
        }
        return false;
    }
    for (double &value : embedding->values) {
        value /= norm;
    }
    return true;
}

bool AppearanceMath::cosineSimilarity(const AppearanceEmbedding &a,
                                      const AppearanceEmbedding &b,
                                      double *outSimilarity, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!outSimilarity) {
        return fail(QStringLiteral("Similarity output is null."));
    }
    if (a.dimension() <= 0 || a.dimension() != b.dimension()) {
        return fail(QStringLiteral(
            "Appearance embeddings have incompatible dimensions."));
    }
    double dotProduct = 0.0;
    double normA = 0.0;
    double normB = 0.0;
    for (int i = 0; i < a.dimension(); ++i) {
        dotProduct += a.values.at(i) * b.values.at(i);
        normA += a.values.at(i) * a.values.at(i);
        normB += b.values.at(i) * b.values.at(i);
    }
    if (normA <= kEpsilon || normB <= kEpsilon) {
        return fail(QStringLiteral("Appearance embedding has zero magnitude."));
    }
    const double similarity = dotProduct / (std::sqrt(normA) * std::sqrt(normB));
    *outSimilarity = qBound(-1.0, similarity, 1.0);
    return true;
}

bool AppearanceMath::aggregate(const QList<AppearanceEmbedding> &samples,
                               AppearanceEmbedding *out, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Appearance aggregate output is null."));
    }
    if (samples.isEmpty()) {
        return fail(QStringLiteral("No appearance samples to aggregate."));
    }
    const int dimension = samples.first().dimension();
    AppearanceEmbedding aggregated;
    aggregated.values = QList<double>(dimension, 0.0);
    aggregated.provider = samples.first().provider;
    for (const AppearanceEmbedding &sample : samples) {
        if (sample.dimension() != dimension) {
            return fail(QStringLiteral(
                "Appearance samples have inconsistent dimensions."));
        }
        for (int i = 0; i < dimension; ++i) {
            aggregated.values[i] += sample.values.at(i);
        }
    }
    for (double &value : aggregated.values) {
        value /= static_cast<double>(samples.size());
    }
    if (!normalize(&aggregated, error)) {
        return false;
    }
    *out = aggregated;
    return true;
}

QString appearanceVerdictToString(AppearanceVerdict verdict)
{
    switch (verdict) {
    case AppearanceVerdict::Agree:
        return QStringLiteral("agree");
    case AppearanceVerdict::Disagree:
        return QStringLiteral("disagree");
    case AppearanceVerdict::Weak:
        return QStringLiteral("weak");
    case AppearanceVerdict::Ambiguous:
        return QStringLiteral("ambiguous");
    case AppearanceVerdict::Unavailable:
    default:
        return QStringLiteral("unavailable");
    }
}

AppearanceVerdict appearanceVerdictFromString(const QString &value)
{
    if (value == QStringLiteral("agree")) {
        return AppearanceVerdict::Agree;
    }
    if (value == QStringLiteral("disagree")) {
        return AppearanceVerdict::Disagree;
    }
    if (value == QStringLiteral("weak")) {
        return AppearanceVerdict::Weak;
    }
    if (value == QStringLiteral("ambiguous")) {
        return AppearanceVerdict::Ambiguous;
    }
    return AppearanceVerdict::Unavailable;
}

bool AppearanceProfile::isValid(QString *error) const
{
    if (error) {
        error->clear();
    }
    if (identity.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Appearance profile identity is empty.");
        }
        return false;
    }
    return reference.isValid(error);
}

QJsonObject AppearanceProfile::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("identity"), identity);
    object.insert(QStringLiteral("reference"), reference.toJsonObject());
    object.insert(QStringLiteral("sampleCount"), sampleCount);
    object.insert(QStringLiteral("updatedAtMs"), static_cast<double>(updatedAtMs));
    if (!provider.isEmpty()) {
        object.insert(QStringLiteral("provider"), provider);
    }
    return object;
}

bool AppearanceProfile::readFromJsonObject(const QJsonObject &object,
                                           AppearanceProfile *out, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Appearance profile output is null."));
    }
    AppearanceProfile profile;
    profile.identity = object.value(QStringLiteral("identity")).toString();
    const QJsonValue referenceValue = object.value(QStringLiteral("reference"));
    if (!referenceValue.isObject()
        || !AppearanceEmbedding::readFromJsonObject(referenceValue.toObject(),
                                                    &profile.reference, error)) {
        return false;
    }
    profile.sampleCount = object.value(QStringLiteral("sampleCount")).toInt();
    profile.updatedAtMs = static_cast<qint64>(
        object.value(QStringLiteral("updatedAtMs")).toDouble());
    profile.provider = object.value(QStringLiteral("provider")).toString();
    if (!profile.isValid(error)) {
        return false;
    }
    *out = profile;
    return true;
}

QJsonObject AppearanceEvidence::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("identity"), identity);
    object.insert(QStringLiteral("candidateTrackId"), candidateTrackId);
    object.insert(QStringLiteral("verdict"), appearanceVerdictToString(verdict));
    object.insert(QStringLiteral("similarity"), similarity);
    object.insert(QStringLiteral("acceptThreshold"), acceptThreshold);
    object.insert(QStringLiteral("rejectThreshold"), rejectThreshold);
    object.insert(QStringLiteral("provider"), provider);
    object.insert(QStringLiteral("detail"), detail);
    object.insert(QStringLiteral("timeMs"), static_cast<double>(timeMs));
    return object;
}

bool AppearanceEvidence::readFromJsonObject(const QJsonObject &object,
                                            AppearanceEvidence *out, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Appearance evidence output is null."));
    }
    AppearanceEvidence evidence;
    evidence.identity = object.value(QStringLiteral("identity")).toString();
    evidence.candidateTrackId =
        object.value(QStringLiteral("candidateTrackId")).toString();
    evidence.verdict = appearanceVerdictFromString(
        object.value(QStringLiteral("verdict")).toString());
    evidence.similarity = object.value(QStringLiteral("similarity")).toDouble(-1.0);
    evidence.acceptThreshold =
        object.value(QStringLiteral("acceptThreshold")).toDouble(0.75);
    evidence.rejectThreshold =
        object.value(QStringLiteral("rejectThreshold")).toDouble(0.55);
    evidence.provider = object.value(QStringLiteral("provider")).toString();
    evidence.detail = object.value(QStringLiteral("detail")).toString();
    evidence.timeMs = static_cast<qint64>(
        object.value(QStringLiteral("timeMs")).toDouble());
    if (evidence.identity.isEmpty()) {
        return fail(QStringLiteral("Appearance evidence is missing its identity."));
    }
    *out = evidence;
    return true;
}
