#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "core/MediaSourceReference.h"

// MediaAnalysis (Objective 21) is the persisted, versioned record of what
// Reelcraft learned about ONE piece of media. It is the artifact the documented
// workflow calls "Media Analysis":
//
//   Source Media -> Media Analysis -> Project Context -> Creator Intent -> ...
//
// It is deliberately shaped like EditDecision (the other persisted artifact):
// its own schema version, a source reference with a cheap fingerprint, a
// deterministic identity digest, a strict envelope loader, version-retaining
// serialization, and preservation of anything it cannot interpret so that an
// older build can never silently destroy data written by a newer one.
//
// BOUNDARIES (the reason this stays small):
//   - It is EVIDENCE, never editorial decision. It may assert what is
//     observable; it may never assert what to do. There is no plan, no cut, no
//     camera keyframe and no A-roll/B-roll label anywhere in this model.
//   - It is DERIVED data. Deleting every analysis artifact must leave a project
//     fully loadable, renderable and replayable; the worst case is the previous
//     behaviour of resolving perception on demand.
//   - It is NEVER on the deterministic replay path. Replay consumes
//     EditDecision::plan() and nothing else, so replay stays perception-free and
//     byte-reproducible (Decision 033).
//   - It never stores decoded pixels and never modifies source media.
//
// SHAPE: a small, closed ENVELOPE (addressing, provenance, lifecycle, coverage,
// confidence) containing named CAPABILITY LAYERS. Capability-specific
// observations live inside their layer; there is deliberately no catch-all
// struct. A new capability is a new layer kind, not a new envelope field.
//
// Envelope rule: if a proposed field is neither addressing, provenance,
// lifecycle, coverage nor confidence, it belongs in a layer.
class MediaAnalysis
{
public:
    // The artifact's own compatibility gate, independent of Project's schema.
    static constexpr int CurrentSchemaVersion = 1;
    // Layers are versioned independently of the envelope AND of each other, so
    // one capability can evolve without invalidating the others.
    static constexpr int CurrentLayerSchemaVersion = 1;

    // --- capability layer kinds (persisted strings, deliberately open-ended) --
    // A kind that this build does not recognize is PRESERVED, never dropped: a
    // newer build may have written a capability this one knows nothing about.
    static QString technicalLayerKind();
    static QString targetsLayerKind();
    static bool isKnownLayerKind(const QString &kind);

    // The 360 frame convention every stored spherical observation is expressed
    // in. Observations are only interpretable relative to a convention, so the
    // technical layer records it; a convention change would invalidate stored
    // angles rather than silently reinterpret them.
    static QString equirectFrameConvention();

    // --- layer lifecycle ------------------------------------------------------
    // "Unavailable" (this environment cannot produce the capability) and
    // "Failed" (it was attempted and errored) are DIFFERENT, and both are
    // different from an empty successful result. An empty transcript must never
    // be indistinguishable from "no transcript provider is configured".
    enum class LayerState
    {
        NotStarted,   // nothing attempted
        InProgress,   // running; coverage records how far it got
        Partial,      // some coverage, not the declared scope
        Complete,     // the declared scope is covered
        Failed,       // attempted, deterministically failed
        Unavailable,  // cannot run here (no provider, no audio track, ...)
        Stale,        // produced against an older spec/provider
    };
    static QString layerStateToString(LayerState state);
    static bool layerStateFromString(const QString &value, LayerState *outState);

    // --- addressing primitives ------------------------------------------------
    struct TimeRange
    {
        qint64 startMs = 0;
        qint64 endMs = 0;

        bool isValid() const { return endMs > startMs && startMs >= 0; }
        qint64 durationMs() const { return endMs - startMs; }
        bool contains(qint64 timeMs) const
        {
            return isValid() && timeMs >= startMs && timeMs <= endMs;
        }
    };

    // Who produced a layer. Recorded so that staleness and caching are decidable
    // and so that swapping a provider produces a different layer revision rather
    // than silently overwriting history.
    struct ProviderInfo
    {
        QString name;
        QString version;
        QString modelId;

        bool isEmpty() const
        {
            return name.isEmpty() && version.isEmpty() && modelId.isEmpty();
        }
    };

    // What the layer was computed at. Persisted because an observation is only
    // comparable with another made at the same resolution and sampling.
    struct LayerSpec
    {
        // Perception resolution: the equirectangular frame size fed to the
        // capability. Deliberately NOT the viewer's display proxy and NOT the
        // source resolution.
        int perceptionWidth = 0;
        int perceptionHeight = 0;
        // Temporal sampling step. 0 means "not sampled" (audio, technical).
        qint64 sampleIntervalMs = 0;

        bool operator==(const LayerSpec &other) const
        {
            return perceptionWidth == other.perceptionWidth
                && perceptionHeight == other.perceptionHeight
                && sampleIntervalMs == other.sampleIntervalMs;
        }
    };

    struct Layer
    {
        QString kind;
        int layerVersion = CurrentLayerSchemaVersion;
        LayerState state = LayerState::NotStarted;
        // The time ranges this layer ACTUALLY covers. Mandatory for any state
        // that carries evidence: without it, "no observation at 07:30" would be
        // indistinguishable from "we never looked at 07:30".
        QList<TimeRange> coverage;
        ProviderInfo provider;
        LayerSpec spec;
        double minConfidence = 0.0;
        // Deterministic reason for Failed / Unavailable. Never empty for those
        // two states, so a missing capability is always explainable.
        QString error;
        // Capability-specific, normalized observations (Tier 1). Opaque to the
        // envelope, typed and versioned per layer kind.
        QJsonArray observations;
        // Provider-native output (Tier 2): retained for debugging/reprocessing,
        // never interpreted by core.
        QJsonObject providerOutput;

        // False when this build could not interpret the layer (unknown kind,
        // newer layer version, or malformed content). Such a layer is re-emitted
        // VERBATIM from raw so that opening and re-saving an artifact can never
        // destroy data this build does not understand.
        bool recognized = true;
        QString preservationReason;
        QJsonObject raw;

        qint64 coveredMs() const;
        bool covers(qint64 timeMs) const;
        // True only when the layer carries usable evidence.
        bool hasEvidence() const
        {
            return state == LayerState::Complete || state == LayerState::Partial;
        }

        QJsonObject toJsonObject() const;
        static bool readFromJsonObject(const QJsonObject &object, Layer *outLayer,
                                       QString *error = nullptr);
    };

    MediaAnalysis();

    int schemaVersion() const { return m_schemaVersion; }
    QDateTime createdUtc() const { return m_createdUtc; }
    void setCreatedUtc(const QDateTime &createdUtc);

    MediaSourceReference source() const { return m_source; }
    void setSource(const MediaSourceReference &source) { m_source = source; }

    // Identity of the analysis SPECIFICATION (perception resolution, sampling,
    // provider identities, participating capabilities). Two analyses are
    // interchangeable only when this agrees.
    QString analysisSpecHash() const { return m_analysisSpecHash; }
    void setAnalysisSpecHash(const QString &specHash) { m_analysisSpecHash = specHash; }

    // --- layers ---------------------------------------------------------------
    const QList<Layer> &layers() const { return m_layers; }
    void clearLayers();
    // Adds or REPLACES the layer of the same kind: one artifact describes one
    // pass, so a kind appears at most once.
    void setLayer(const Layer &layer);
    const Layer *layer(const QString &kind) const;
    bool hasLayer(const QString &kind) const;

    // --- validity / status ----------------------------------------------------
    // Strict envelope validation: version, creation time, source reference,
    // specification identity. Layer contents are NOT validated here (they are
    // preserved instead of rejected).
    bool isValid(QString *error = nullptr) const;

    // Source validity NOW (FileMissing and FingerprintMismatch kept distinct).
    MediaSourceStatus checkSource(QString *detail = nullptr) const;

    // Analysis freshness: was this produced by the specification currently in
    // use? Deliberately a separate axis from source validity -- a layer can be
    // fresh about a file that has since moved, or stale about an unchanged file.
    bool matchesSpec(const QString &specHash) const;

    // Human-readable per-layer summaries for status reporting. Never used for
    // logic.
    QStringList layerSummaries() const;

    // --- identity and serialization -------------------------------------------
    static bool isValidId(const QString &id);
    // SHA-256 (lowercase hex) over the compact JSON of payloadWithoutId().
    QByteArray analysisId() const;

    // Every persisted field EXCEPT "analysisId". createdUtc IS part of it.
    QJsonObject payloadWithoutId() const;
    // payloadWithoutId() plus "analysisId".
    QJsonObject toJsonObject() const;

    // Strict envelope loader. Fails when the artifact cannot be interpreted:
    // missing/unsupported schemaVersion, missing or malformed creation time,
    // missing/incomplete source reference, missing specification identity, a
    // layers entry that is not an object, or a present analysisId that disagrees
    // with the recomputed digest. A layer whose CONTENT cannot be interpreted is
    // preserved rather than rejected.
    static bool readFromJsonObject(const QJsonObject &object, MediaAnalysis *out,
                                   QString *error = nullptr);

    bool save(const QString &filePath, QString *error = nullptr) const;
    static MediaAnalysis load(const QString &filePath, bool *ok = nullptr,
                              QString *error = nullptr);

    // Content-addressed file name, so a new analysis never overwrites an older
    // artifact that a newer build wrote and this build cannot interpret.
    QString suggestedFileName() const;

    // Deterministic identity of a specification description. The caller builds
    // the canonical object; this hashes its compact JSON.
    static QString specHashFor(const QJsonObject &canonicalSpec);

private:
    int m_schemaVersion = CurrentSchemaVersion;
    QDateTime m_createdUtc;
    MediaSourceReference m_source;
    QString m_analysisSpecHash;
    QList<Layer> m_layers;
};

// ---------------------------------------------------------------------------
// Project integration: a SMALL reference to a stored analysis artifact.
//
// The reference is what the project persists; the artifact itself lives beside
// the project. The reference is deliberately sufficient only to (a) locate the
// artifact and (b) verify that it still corresponds to the expected media and
// source content -- never to carry analysis data itself.
// ---------------------------------------------------------------------------
struct MediaAnalysisReference
{
    QString mediaId;
    QString artifactId;    // MediaAnalysis::analysisId() of the referenced artifact
    QString artifactPath;  // where it lives; empty means "recorded but not materialized"
    qint64 sourceSizeBytes = -1;
    QDateTime sourceLastModifiedUtc;

    bool isValid() const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   MediaAnalysisReference *out,
                                   QString *error = nullptr);
};

// Why a reference could not produce usable analysis. Every value except
// Resolved is a NON-FATAL condition: the project stays loadable and every
// deterministic operation keeps working.
enum class MediaAnalysisRefStatus
{
    None,                 // this media has no analysis reference
    Resolved,             // artifact read, source matches, specification current
    ArtifactMissing,      // the referenced artifact file is not there
    ArtifactUnreadable,   // present but refused by the strict envelope loader
    ArtifactMismatch,     // present but is not the artifact the reference names
    SourceMissing,        // the analyzed media file no longer exists
    SourceChanged,        // the media file exists but is not the analyzed file
    Stale,                // analyzed with a different specification
};

QString mediaAnalysisRefStatusToString(MediaAnalysisRefStatus status);

struct MediaAnalysisResolution
{
    MediaAnalysisRefStatus status = MediaAnalysisRefStatus::None;
    MediaAnalysis analysis;
    // Always populated for a non-Resolved outcome, so a degraded state is
    // reportable rather than silent.
    QString detail;

    bool isResolved() const { return status == MediaAnalysisRefStatus::Resolved; }
};

// Resolves a reference to a usable artifact. expectedSpecHash is optional: when
// non-empty, a fresh-but-differently-specified artifact is reported Stale
// instead of Resolved.
MediaAnalysisResolution resolveMediaAnalysisReference(
    const MediaAnalysisReference &reference,
    const QString &expectedSpecHash = QString());

