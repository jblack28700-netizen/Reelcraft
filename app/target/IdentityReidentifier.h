#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "reframe/ReframeFrameProvider.h"
#include "target/AppearanceProvider.h"
#include "target/TargetCropExtractor.h"
#include "target/TargetIdentity.h"
#include "target/TargetTypes.h"

// IdentityReidentifier is the optional, replaceable appearance/re-identification
// orchestrator. It sits above detection/tracking/identity and uses an external
// AppearanceProvider to:
//   - maintain an appearance profile for a bound identity;
//   - confirm or veto a geometric continuation;
//   - re-acquire an identity when geometry alone is insufficient.
//
// Deterministic precedence (documented, conservative):
//   1. An explicit creator selection (or any active bound track) is authoritative;
//      appearance never overrides it.
//   2. A valid existing tracker continuity needs no re-identification.
//   3. A unique geometric continuation is accepted; if appearance evidence is
//      available and strongly DISAGREES, the continuation is vetoed and the
//      identity becomes unresolved (no silent transfer).
//   4. Appearance evidence may re-acquire exactly one strong candidate when
//      geometry provides none; multiple strong candidates are ambiguous.
//   5. Otherwise the identity remains unresolved.
//
// The class is media/model agnostic: it only calls the existing
// ReframeFrameProvider and the injected AppearanceProvider. Reelcraft remains
// fully usable with a null provider.
class IdentityReidentifier
{
public:
    struct Config
    {
        // Cosine similarity at/above which appearance "agrees".
        double acceptThreshold = 0.75;
        // Cosine similarity below which appearance "disagrees".
        double rejectThreshold = 0.55;
        // Maximum reference crops sampled when building a profile.
        int maxReferenceSamples = 5;
        // Class filter for candidate tracks (empty = any label).
        QString label = QStringLiteral("person");
        TargetCropExtractor::Config crop;
    };

    struct Result
    {
        QList<AppearanceEvidence> evidence;
        QStringList changedIdentities;
        QStringList notes;
    };

    IdentityReidentifier() = default;
    explicit IdentityReidentifier(Config config);

    Config config() const;
    void setConfig(const Config &config);

    // Refreshes geometry, maintains appearance profiles, and applies the
    // precedence policy above. Never throws; failures are reported in notes.
    Result reidentify(TargetIdentityRegistry *registry,
                      const QList<TargetTrack> &tracks,
                      ReframeFrameProvider *frames,
                      AppearanceProvider *provider, qint64 timeMs);

    // Builds a normalized appearance profile from a track. Returns false with a
    // deterministic error when no crop could be encoded.
    bool buildProfile(const TargetTrack &track, ReframeFrameProvider *frames,
                      AppearanceProvider *provider, qint64 timeMs,
                      AppearanceProfile *out, QString *error = nullptr) const;

    AppearanceVerdict classify(double similarity) const;

private:
    bool encodeObservation(const TargetObservation &observation,
                           ReframeFrameProvider *frames,
                           AppearanceProvider *provider,
                           AppearanceEmbedding *out, QString *error) const;

    Config m_config;
};
