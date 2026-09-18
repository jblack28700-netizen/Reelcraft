#include "analysis/MediaAnalysis.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

namespace {

QString compactHash(const QJsonObject &object)
{
    const QByteArray canonical =
        QJsonDocument(object).toJson(QJsonDocument::Compact);
    return QString::fromLatin1(
        QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex());
}

QJsonObject timeRangeToJson(const MediaAnalysis::TimeRange &range)
{
    QJsonObject object;
    object.insert(QStringLiteral("startMs"), static_cast<double>(range.startMs));
    object.insert(QStringLiteral("endMs"), static_cast<double>(range.endMs));
    return object;
}

bool readTimeRange(const QJsonValue &value, MediaAnalysis::TimeRange *outRange)
{
    if (!outRange || !value.isObject()) {
        return false;
    }
    const QJsonObject object = value.toObject();
    const QJsonValue startValue = object.value(QStringLiteral("startMs"));
    const QJsonValue endValue = object.value(QStringLiteral("endMs"));
    if (!startValue.isDouble() || !endValue.isDouble()) {
        return false;
    }
    MediaAnalysis::TimeRange range;
    range.startMs = static_cast<qint64>(startValue.toDouble());
    range.endMs = static_cast<qint64>(endValue.toDouble());
    // An unparsable or nonsensical range is treated as unreadable rather than
    // silently clamped: coverage that claims something untrue is worse than
    // coverage that is reported as unreadable.
    if (!range.isValid()) {
        return false;
    }
    *outRange = range;
    return true;
}

QJsonObject providerToJson(const MediaAnalysis::ProviderInfo &provider)
{
    QJsonObject object;
    if (!provider.name.isEmpty()) {
        object.insert(QStringLiteral("name"), provider.name);
    }
    if (!provider.version.isEmpty()) {
        object.insert(QStringLiteral("version"), provider.version);
    }
    if (!provider.modelId.isEmpty()) {
        object.insert(QStringLiteral("modelId"), provider.modelId);
    }
    return object;
}

MediaAnalysis::ProviderInfo providerFromJson(const QJsonObject &object)
{
    MediaAnalysis::ProviderInfo provider;
    provider.name = object.value(QStringLiteral("name")).toString();
    provider.version = object.value(QStringLiteral("version")).toString();
    provider.modelId = object.value(QStringLiteral("modelId")).toString();
    return provider;
}

QJsonObject specToJson(const MediaAnalysis::LayerSpec &spec)
{
    QJsonObject object;
    if (spec.perceptionWidth > 0) {
        object.insert(QStringLiteral("perceptionWidth"), spec.perceptionWidth);
    }
    if (spec.perceptionHeight > 0) {
        object.insert(QStringLiteral("perceptionHeight"), spec.perceptionHeight);
    }
    if (spec.sampleIntervalMs > 0) {
        object.insert(QStringLiteral("sampleIntervalMs"),
                      static_cast<double>(spec.sampleIntervalMs));
    }
    return object;
}

MediaAnalysis::LayerSpec specFromJson(const QJsonObject &object)
{
    MediaAnalysis::LayerSpec spec;
    spec.perceptionWidth =
        object.value(QStringLiteral("perceptionWidth")).toInt();
    spec.perceptionHeight =
        object.value(QStringLiteral("perceptionHeight")).toInt();
    spec.sampleIntervalMs = static_cast<qint64>(
        object.value(QStringLiteral("sampleIntervalMs")).toDouble());
    if (spec.perceptionWidth < 0) {
        spec.perceptionWidth = 0;
    }
    if (spec.perceptionHeight < 0) {
        spec.perceptionHeight = 0;
    }
    if (spec.sampleIntervalMs < 0) {
        spec.sampleIntervalMs = 0;
    }
    return spec;
}

} // namespace

// ---------------------------------------------------------------------------
// Vocabulary
// ---------------------------------------------------------------------------

QString MediaAnalysis::technicalLayerKind()
{
    return QStringLiteral("technical");
}

QString MediaAnalysis::targetsLayerKind()
{
    return QStringLiteral("targets");
}

bool MediaAnalysis::isKnownLayerKind(const QString &kind)
{
    return kind == technicalLayerKind() || kind == targetsLayerKind();
}

QString MediaAnalysis::equirectFrameConvention()
{
    // The convention implemented by ViewportState / ViewerProjection /
    // EquirectView: yaw in [-180, 180) increasing to the right, pitch in
    // [-90, 90] increasing upward, identity looking at the FRONT of the sphere.
    // Stored so that a future convention change invalidates stored angles
    // instead of silently reinterpreting them.
    return QStringLiteral("reelcraft-equirect-v1");
}

QString MediaAnalysis::layerStateToString(LayerState state)
{
    switch (state) {
    case LayerState::NotStarted:
        return QStringLiteral("not-started");
    case LayerState::InProgress:
        return QStringLiteral("in-progress");
    case LayerState::Partial:
        return QStringLiteral("partial");
    case LayerState::Complete:
        return QStringLiteral("complete");
    case LayerState::Failed:
        return QStringLiteral("failed");
    case LayerState::Unavailable:
        return QStringLiteral("unavailable");
    case LayerState::Stale:
        return QStringLiteral("stale");
    }
    return QStringLiteral("not-started");
}

bool MediaAnalysis::layerStateFromString(const QString &value, LayerState *outState)
{
    if (!outState) {
        return false;
    }
    static const struct
    {
        const char *tag;
        LayerState state;
    } kStates[] = {
        { "not-started", LayerState::NotStarted },
        { "in-progress", LayerState::InProgress },
        { "partial", LayerState::Partial },
        { "complete", LayerState::Complete },
        { "failed", LayerState::Failed },
        { "unavailable", LayerState::Unavailable },
        { "stale", LayerState::Stale },
    };
    for (const auto &entry : kStates) {
        if (value == QLatin1String(entry.tag)) {
            *outState = entry.state;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Layer
// ---------------------------------------------------------------------------

qint64 MediaAnalysis::Layer::coveredMs() const
{
    qint64 total = 0;
    for (const TimeRange &range : coverage) {
        if (range.isValid()) {
            total += range.durationMs();
        }
    }
    return total;
}

bool MediaAnalysis::Layer::covers(qint64 timeMs) const
{
    for (const TimeRange &range : coverage) {
        if (range.contains(timeMs)) {
            return true;
        }
    }
    return false;
}

QJsonObject MediaAnalysis::Layer::toJsonObject() const
{
    // A layer this build could not interpret is re-emitted byte-for-byte as it
    // was read, so that an older build opening and re-saving an artifact can
    // never destroy a newer capability's data.
    if (!recognized) {
        return raw;
    }

    QJsonObject object;
    object.insert(QStringLiteral("kind"), kind);
    object.insert(QStringLiteral("layerVersion"), layerVersion);
    object.insert(QStringLiteral("state"), layerStateToString(state));

    QJsonArray coverageArray;
    for (const TimeRange &range : coverage) {
        coverageArray.append(timeRangeToJson(range));
    }
    object.insert(QStringLiteral("coverage"), coverageArray);

    if (!provider.isEmpty()) {
        object.insert(QStringLiteral("provider"), providerToJson(provider));
    }
    const QJsonObject specObject = specToJson(spec);
    if (!specObject.isEmpty()) {
        object.insert(QStringLiteral("spec"), specObject);
    }
    if (minConfidence > 0.0) {
        object.insert(QStringLiteral("minConfidence"), minConfidence);
    }
    if (!error.isEmpty()) {
        object.insert(QStringLiteral("error"), error);
    }
    if (!observations.isEmpty()) {
        object.insert(QStringLiteral("observations"), observations);
    }
    if (!providerOutput.isEmpty()) {
        object.insert(QStringLiteral("providerOutput"), providerOutput);
    }
    return object;
}

bool MediaAnalysis::Layer::readFromJsonObject(const QJsonObject &object,
                                              Layer *outLayer, QString *error)
{
    if (error) {
        error->clear();
    }
    if (!outLayer) {
        if (error) {
            *error = QStringLiteral("Media analysis layer output is null.");
        }
        return false;
    }

    Layer layer;
    layer.raw = object;

    const auto preserve = [&layer, outLayer](const QString &reason) {
        layer.recognized = false;
        layer.preservationReason = reason;
        *outLayer = layer;
        return true;
    };

    const QJsonValue kindValue = object.value(QStringLiteral("kind"));
    if (!kindValue.isString() || kindValue.toString().isEmpty()) {
        // A layer with no kind cannot be addressed at all; it is preserved
        // rather than rejected so that no written data is ever lost.
        return preserve(QStringLiteral("the layer does not name a capability kind"));
    }
    layer.kind = kindValue.toString();

    const QJsonValue versionValue = object.value(QStringLiteral("layerVersion"));
    if (!versionValue.isDouble()) {
        return preserve(QStringLiteral("the layer has no numeric layer version"));
    }
    layer.layerVersion = versionValue.toInt();
    if (layer.layerVersion <= 0) {
        return preserve(QStringLiteral("the layer version is not positive"));
    }

    if (!isKnownLayerKind(layer.kind)) {
        return preserve(QStringLiteral("the capability '%1' is not known to this "
                                       "build").arg(layer.kind));
    }
    if (layer.layerVersion > CurrentLayerSchemaVersion) {
        return preserve(QStringLiteral("the capability '%1' was written by a newer "
                                       "layer schema (%2)")
                            .arg(layer.kind)
                            .arg(layer.layerVersion));
    }

    const QJsonValue stateValue = object.value(QStringLiteral("state"));
    LayerState state = LayerState::NotStarted;
    if (!stateValue.isString()
        || !layerStateFromString(stateValue.toString(), &state)) {
        return preserve(QStringLiteral("the layer state '%1' is not recognized")
                            .arg(stateValue.toString()));
    }
    layer.state = state;

    const QJsonValue coverageValue = object.value(QStringLiteral("coverage"));
    if (!coverageValue.isUndefined() && !coverageValue.isNull()) {
        if (!coverageValue.isArray()) {
            return preserve(QStringLiteral("the layer coverage is not a list"));
        }
        for (const QJsonValue &entry : coverageValue.toArray()) {
            TimeRange range;
            if (!readTimeRange(entry, &range)) {
                return preserve(QStringLiteral(
                    "the layer coverage contains an unusable time range"));
            }
            layer.coverage.append(range);
        }
    }

    const QJsonValue providerValue = object.value(QStringLiteral("provider"));
    if (providerValue.isObject()) {
        layer.provider = providerFromJson(providerValue.toObject());
    }
    const QJsonValue specValue = object.value(QStringLiteral("spec"));
    if (specValue.isObject()) {
        layer.spec = specFromJson(specValue.toObject());
    }
    layer.minConfidence =
        object.value(QStringLiteral("minConfidence")).toDouble(0.0);
    layer.error = object.value(QStringLiteral("error")).toString();
    const QJsonValue observationsValue =
        object.value(QStringLiteral("observations"));
    if (observationsValue.isArray()) {
        layer.observations = observationsValue.toArray();
    }
    const QJsonValue providerOutputValue =
        object.value(QStringLiteral("providerOutput"));
    if (providerOutputValue.isObject()) {
        layer.providerOutput = providerOutputValue.toObject();
    }

    layer.recognized = true;
    *outLayer = layer;
    return true;
}

// ---------------------------------------------------------------------------
// MediaAnalysis
// ---------------------------------------------------------------------------

MediaAnalysis::MediaAnalysis() = default;

void MediaAnalysis::setCreatedUtc(const QDateTime &createdUtc)
{
    m_createdUtc = mediaSourceTimestampToUtcMs(createdUtc);
}

void MediaAnalysis::clearLayers()
{
    m_layers.clear();
}

void MediaAnalysis::setLayer(const Layer &layer)
{
    for (int i = 0; i < m_layers.size(); ++i) {
        if (m_layers.at(i).kind == layer.kind) {
            m_layers[i] = layer;
            return;
        }
    }
    m_layers.append(layer);
}

const MediaAnalysis::Layer *MediaAnalysis::layer(const QString &kind) const
{
    for (const Layer &entry : m_layers) {
        if (entry.kind == kind) {
            return &entry;
        }
    }
    return nullptr;
}

bool MediaAnalysis::hasLayer(const QString &kind) const
{
    return layer(kind) != nullptr;
}

bool MediaAnalysis::isValid(QString *error) const
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

    if (m_schemaVersion <= 0 || m_schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral("Media analysis schema version is unsupported."));
    }
    if (!m_createdUtc.isValid()) {
        return fail(QStringLiteral("Media analysis has no valid creation time."));
    }
    if (!m_source.isValid()) {
        return fail(QStringLiteral("Media analysis source reference is incomplete."));
    }
    if (m_analysisSpecHash.isEmpty()) {
        return fail(QStringLiteral(
            "Media analysis has no specification identity."));
    }
    return true;
}

MediaSourceStatus MediaAnalysis::checkSource(QString *detail) const
{
    return checkMediaSourceStatus(m_source, detail);
}

bool MediaAnalysis::matchesSpec(const QString &specHash) const
{
    return !specHash.isEmpty() && m_analysisSpecHash == specHash;
}

QStringList MediaAnalysis::layerSummaries() const
{
    QStringList summaries;
    for (const Layer &entry : m_layers) {
        QString summary = QStringLiteral("%1: %2")
                              .arg(entry.kind, layerStateToString(entry.state));
        if (!entry.coverage.isEmpty()) {
            summary += QStringLiteral(" (coverage %1 ms)")
                           .arg(entry.coveredMs());
        }
        if (!entry.provider.name.isEmpty()) {
            summary += QStringLiteral(" via %1").arg(entry.provider.name);
        }
        if (!entry.recognized) {
            summary += QStringLiteral(" [preserved: %1]")
                           .arg(entry.preservationReason);
        } else if (!entry.error.isEmpty()) {
            summary += QStringLiteral(" [%1]").arg(entry.error);
        }
        summaries.append(summary);
    }
    return summaries;
}

bool MediaAnalysis::isValidId(const QString &id)
{
    if (id.size() != 64) {
        return false;
    }
    for (const QChar c : id) {
        const ushort u = c.unicode();
        const bool lowerHex = (u >= '0' && u <= '9') || (u >= 'a' && u <= 'f');
        if (!lowerHex) {
            return false;
        }
    }
    return true;
}

QJsonObject MediaAnalysis::payloadWithoutId() const
{
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    object.insert(QStringLiteral("createdUtc"),
                  mediaSourceTimestampToUtcMs(m_createdUtc)
                      .toString(Qt::ISODateWithMs));

    // Mirrors EditDecision's source object exactly: one vocabulary, one JSON
    // shape, so an external tool reading either artifact sees the same fields.
    QJsonObject source;
    source.insert(QStringLiteral("mediaId"), m_source.mediaId);
    source.insert(QStringLiteral("path"), m_source.path);
    source.insert(QStringLiteral("sizeBytes"),
                  static_cast<double>(m_source.sizeBytes));
    source.insert(QStringLiteral("lastModifiedUtc"),
                  mediaSourceTimestampToUtcMs(m_source.lastModifiedUtc)
                      .toString(Qt::ISODateWithMs));
    if (!m_source.contentSha256.isEmpty()) {
        source.insert(QStringLiteral("contentSha256"), m_source.contentSha256);
    }
    object.insert(QStringLiteral("source"), source);

    object.insert(QStringLiteral("analysisSpecHash"), m_analysisSpecHash);

    QJsonArray layersArray;
    for (const Layer &entry : m_layers) {
        layersArray.append(entry.toJsonObject());
    }
    object.insert(QStringLiteral("layers"), layersArray);
    return object;
}

QByteArray MediaAnalysis::analysisId() const
{
    const QByteArray canonical =
        QJsonDocument(payloadWithoutId()).toJson(QJsonDocument::Compact);
    return QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex();
}

QJsonObject MediaAnalysis::toJsonObject() const
{
    QJsonObject object = payloadWithoutId();
    object.insert(QStringLiteral("analysisId"),
                  QString::fromLatin1(analysisId()));
    return object;
}

bool MediaAnalysis::readFromJsonObject(const QJsonObject &object,
                                       MediaAnalysis *out, QString *error)
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
        return fail(QStringLiteral("Media analysis output is null."));
    }

    const QJsonValue versionValue = object.value(QStringLiteral("schemaVersion"));
    if (!versionValue.isDouble()) {
        return fail(QStringLiteral("Media analysis has no schema version."));
    }
    const int schemaVersion = versionValue.toInt();
    if (schemaVersion <= 0 || schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral(
            "Media analysis schema version is unsupported."));
    }

    const QDateTime createdUtc = QDateTime::fromString(
        object.value(QStringLiteral("createdUtc")).toString(),
        Qt::ISODateWithMs);
    if (!createdUtc.isValid()) {
        return fail(QStringLiteral("Media analysis has no valid creation time."));
    }

    const QJsonValue sourceValue = object.value(QStringLiteral("source"));
    if (!sourceValue.isObject()) {
        return fail(QStringLiteral("Media analysis has no source reference."));
    }
    const QJsonObject sourceObject = sourceValue.toObject();
    MediaSourceReference source;
    source.mediaId = sourceObject.value(QStringLiteral("mediaId")).toString();
    source.path = sourceObject.value(QStringLiteral("path")).toString();
    bool sizeOk = false;
    const double sizeNumber =
        sourceObject.value(QStringLiteral("sizeBytes")).toDouble(-1.0);
    source.sizeBytes = static_cast<qint64>(sizeNumber);
    sizeOk = sizeNumber >= 0.0;
    source.lastModifiedUtc = QDateTime::fromString(
        sourceObject.value(QStringLiteral("lastModifiedUtc")).toString(),
        Qt::ISODateWithMs);
    source.contentSha256 =
        sourceObject.value(QStringLiteral("contentSha256")).toString();
    if (!sizeOk || !source.isValid()) {
        return fail(QStringLiteral(
            "Media analysis source reference is incomplete."));
    }

    const QString specHash =
        object.value(QStringLiteral("analysisSpecHash")).toString();
    if (specHash.isEmpty()) {
        return fail(QStringLiteral(
            "Media analysis has no specification identity."));
    }

    MediaAnalysis analysis;
    analysis.m_schemaVersion = schemaVersion;
    analysis.m_createdUtc = createdUtc;
    analysis.m_source = source;
    analysis.m_analysisSpecHash = specHash;

    const QJsonValue layersValue = object.value(QStringLiteral("layers"));
    if (!layersValue.isUndefined() && !layersValue.isNull()) {
        if (!layersValue.isArray()) {
            return fail(QStringLiteral("Media analysis layers are not a list."));
        }
        for (const QJsonValue &entry : layersValue.toArray()) {
            // A non-object entry cannot be interpreted as a layer at all and is
            // therefore envelope corruption rather than preservable content.
            if (!entry.isObject()) {
                return fail(QStringLiteral(
                    "Media analysis contains a layer entry that is not an "
                    "object."));
            }
            Layer layer;
            QString layerError;
            if (!Layer::readFromJsonObject(entry.toObject(), &layer, &layerError)) {
                return fail(layerError.isEmpty()
                                ? QStringLiteral("Media analysis layer is invalid.")
                                : layerError);
            }
            analysis.m_layers.append(layer);
        }
    }

    // The stored digest is DERIVED, not authoritative: a missing one is
    // tolerated and recomputed on demand, but a PRESENT one that disagrees means
    // the artifact was altered or written by an incompatible build, and is
    // refused rather than silently trusted.
    const QJsonValue idValue = object.value(QStringLiteral("analysisId"));
    if (!idValue.isUndefined() && !idValue.isNull()) {
        const QString storedId = idValue.toString();
        if (!isValidId(storedId)) {
            return fail(QStringLiteral(
                "Media analysis id is not 64 lowercase hex characters."));
        }
        if (storedId != QString::fromLatin1(analysis.analysisId())) {
            return fail(QStringLiteral(
                "Media analysis id does not match its contents."));
        }
    }

    *out = analysis;
    return true;
}

bool MediaAnalysis::save(const QString &filePath, QString *error) const
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

    QString validationError;
    if (!isValid(&validationError)) {
        return fail(validationError);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return fail(file.errorString());
    }
    const QJsonDocument document(toJsonObject());
    if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
        return fail(file.errorString());
    }
    file.close();
    return true;
}

MediaAnalysis MediaAnalysis::load(const QString &filePath, bool *ok, QString *error)
{
    if (ok) {
        *ok = false;
    }
    if (error) {
        error->clear();
    }
    const auto fail = [ok, error](const QString &message) {
        if (error) {
            *error = message;
        }
        return MediaAnalysis();
    };

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(file.errorString());
    }
    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Media analysis file is not valid JSON."));
    }

    MediaAnalysis analysis;
    QString readError;
    if (!readFromJsonObject(document.object(), &analysis, &readError)) {
        return fail(readError);
    }
    if (ok) {
        *ok = true;
    }
    return analysis;
}

QString MediaAnalysis::suggestedFileName() const
{
    return QStringLiteral("analysis-%1.json")
        .arg(QString::fromLatin1(analysisId()));
}

QString MediaAnalysis::specHashFor(const QJsonObject &canonicalSpec)
{
    return compactHash(canonicalSpec);
}

// ---------------------------------------------------------------------------
// Project integration: references
// ---------------------------------------------------------------------------

bool MediaAnalysisReference::isValid() const
{
    return !mediaId.isEmpty() && !artifactId.isEmpty();
}

QJsonObject MediaAnalysisReference::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("mediaId"), mediaId);
    object.insert(QStringLiteral("artifactId"), artifactId);
    if (!artifactPath.isEmpty()) {
        object.insert(QStringLiteral("artifactPath"), artifactPath);
    }
    if (sourceSizeBytes >= 0) {
        object.insert(QStringLiteral("sourceSizeBytes"),
                      static_cast<double>(sourceSizeBytes));
    }
    if (sourceLastModifiedUtc.isValid()) {
        object.insert(QStringLiteral("sourceLastModifiedUtc"),
                      mediaSourceTimestampToUtcMs(sourceLastModifiedUtc)
                          .toString(Qt::ISODateWithMs));
    }
    return object;
}

bool MediaAnalysisReference::readFromJsonObject(const QJsonObject &object,
                                                MediaAnalysisReference *out,
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
        return fail(QStringLiteral("Media analysis reference output is null."));
    }

    MediaAnalysisReference reference;
    reference.mediaId = object.value(QStringLiteral("mediaId")).toString();
    reference.artifactId = object.value(QStringLiteral("artifactId")).toString();
    reference.artifactPath = object.value(QStringLiteral("artifactPath")).toString();
    const QJsonValue sizeValue = object.value(QStringLiteral("sourceSizeBytes"));
    reference.sourceSizeBytes =
        sizeValue.isDouble() ? static_cast<qint64>(sizeValue.toDouble()) : -1;
    const QDateTime modified = QDateTime::fromString(
        object.value(QStringLiteral("sourceLastModifiedUtc")).toString(),
        Qt::ISODateWithMs);
    if (modified.isValid()) {
        reference.sourceLastModifiedUtc = modified;
    }

    if (!reference.isValid()) {
        return fail(QStringLiteral(
            "Media analysis reference is missing its media or artifact id."));
    }
    *out = reference;
    return true;
}

QString mediaAnalysisRefStatusToString(MediaAnalysisRefStatus status)
{
    switch (status) {
    case MediaAnalysisRefStatus::None:
        return QStringLiteral("none");
    case MediaAnalysisRefStatus::Resolved:
        return QStringLiteral("resolved");
    case MediaAnalysisRefStatus::ArtifactMissing:
        return QStringLiteral("artifact-missing");
    case MediaAnalysisRefStatus::ArtifactUnreadable:
        return QStringLiteral("artifact-unreadable");
    case MediaAnalysisRefStatus::ArtifactMismatch:
        return QStringLiteral("artifact-mismatch");
    case MediaAnalysisRefStatus::SourceMissing:
        return QStringLiteral("source-missing");
    case MediaAnalysisRefStatus::SourceChanged:
        return QStringLiteral("source-changed");
    case MediaAnalysisRefStatus::Stale:
        return QStringLiteral("stale");
    }
    return QStringLiteral("none");
}

MediaAnalysisResolution resolveMediaAnalysisReference(
    const MediaAnalysisReference &reference, const QString &expectedSpecHash)
{
    MediaAnalysisResolution resolution;
    if (!reference.isValid()) {
        resolution.status = MediaAnalysisRefStatus::None;
        resolution.detail = QStringLiteral("No analysis reference was recorded.");
        return resolution;
    }

    if (reference.artifactPath.isEmpty()) {
        resolution.status = MediaAnalysisRefStatus::ArtifactMissing;
        resolution.detail = QStringLiteral(
            "The analysis artifact for this media was never written to disk.");
        return resolution;
    }

    // "The file is not there" and "the file is there but cannot be read" are
    // different conditions and are never collapsed into one another: the first is
    // a routine cache miss, the second means something wrote an artifact this
    // build cannot interpret.
    const QFileInfo artifactInfo(reference.artifactPath);
    if (!artifactInfo.exists() || !artifactInfo.isFile()) {
        resolution.status = MediaAnalysisRefStatus::ArtifactMissing;
        resolution.detail = QStringLiteral(
            "The referenced analysis artifact does not exist: %1")
                                .arg(reference.artifactPath);
        return resolution;
    }

    bool loaded = false;
    QString loadError;
    MediaAnalysis analysis =
        MediaAnalysis::load(reference.artifactPath, &loaded, &loadError);
    if (!loaded) {
        resolution.status = MediaAnalysisRefStatus::ArtifactUnreadable;
        resolution.detail = loadError;
        return resolution;
    }

    // The artifact must be the artifact the reference names. A different id means
    // the file was replaced or the reference points at the wrong artifact.
    if (!reference.artifactId.isEmpty()
        && QString::fromLatin1(analysis.analysisId()) != reference.artifactId) {
        resolution.status = MediaAnalysisRefStatus::ArtifactMismatch;
        resolution.detail = QStringLiteral(
            "The stored analysis artifact is not the one this project "
            "references.");
        return resolution;
    }

    // Source validity is checked against the MEDIA, not the artifact, so a moved
    // or edited source is reported honestly rather than silently trusted.
    QString sourceDetail;
    const MediaSourceStatus sourceStatus = analysis.checkSource(&sourceDetail);
    if (sourceStatus == MediaSourceStatus::FileMissing) {
        resolution.status = MediaAnalysisRefStatus::SourceMissing;
        resolution.detail = sourceDetail;
        return resolution;
    }
    if (sourceStatus == MediaSourceStatus::FingerprintMismatch) {
        resolution.status = MediaAnalysisRefStatus::SourceChanged;
        resolution.detail = sourceDetail;
        return resolution;
    }

    if (!expectedSpecHash.isEmpty() && !analysis.matchesSpec(expectedSpecHash)) {
        resolution.status = MediaAnalysisRefStatus::Stale;
        resolution.detail = QStringLiteral(
            "The stored analysis was produced by a different analysis "
            "specification and must be recomputed before it can be trusted.");
        resolution.analysis = analysis;
        return resolution;
    }

    resolution.status = MediaAnalysisRefStatus::Resolved;
    resolution.analysis = analysis;
    return resolution;
}

