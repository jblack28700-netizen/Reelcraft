#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "target/AppearanceTypes.h"
#include "target/TargetTypes.h"

// 360 Reframing Objective 4 — structured creator-target identity.
//
// Identity here is a deterministic binding between a creator-meaningful name
// (for example "me") and a tracker track id. It is explicitly NOT biometric
// identity and NOT long-term appearance re-identification: it survives normal
// movement, crossing trajectories, temporary detection loss, and short
// re-entry through geometric/trajectory continuity only. When identity cannot
// be established confidently the registry reports it as unresolved rather than
// guessing.
//
// This layer sits above detection/tracking and is independent of the detector,
// the model/runtime, and the renderer. An appearance/embedding re-identifier can
// later extend it behind the same structured binding.

// A creator's structured selection of a target. This is the data a UI click
// (or an explicit selection) produces: a direction in the 360 sphere at a time,
// and/or a specific track id.
struct CreatorTargetSelection
{
    QString identity; // canonical key, e.g. "me"
    qint64 timeMs = 0;
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    QString targetId; // optional direct track id
    QString label;    // optional class filter, e.g. "person"
    QString evidence; // human-readable provenance of the selection

    bool isValid(QString *error = nullptr) const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   CreatorTargetSelection *out,
                                   QString *error = nullptr);
};

// Structured identity binding state.
struct IdentityBinding
{
    QString identity;
    QString targetId;
    qint64 boundAtMs = 0;
    QString method; // "seed-direction" | "track-id" | "continuity-rebind"
                    // | "appearance-rebind"
    double distanceDeg = -1.0;
    bool resolved = false;
    // Appearance evidence annotation (optional; -1 = no evidence).
    double appearanceSimilarity = -1.0;
    QString appearanceVerdict; // "agree" | "disagree" | "weak" | "ambiguous"
    // Targets that appearance evidence has vetoed for this identity; geometric
    // re-binding must not silently re-accept them.
    QStringList rejectedTargetIds;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object, IdentityBinding *out,
                                   QString *error = nullptr);
};

class TargetIdentityRegistry
{
public:
    struct Config
    {
        // Maximum angular distance between a creator selection and a track for
        // the seed binding to succeed.
        double bindingGateDeg = 25.0;
        // Search window around the selection time for candidate observations.
        qint64 bindingSearchWindowMs = 2000;
        // Continuity re-binding when the bound track stops being observed.
        double rebindGateDeg = 60.0;
        qint64 rebindWindowMs = 3000;
        // Prediction horizon used for continuity re-binding.
        qint64 maxPredictionMs = 1500;
    };

    TargetIdentityRegistry() = default;
    explicit TargetIdentityRegistry(Config config);

    void reset();
    Config config() const;
    void setConfig(const Config &config);

    static QString normalizeIdentity(const QString &identity);
    static QString creatorIdentity();

    // Binds a creator identity to a track using a structured selection.
    // Deterministic: the nearest in-window observation wins; ties are broken by
    // (confidence desc, track firstTimeMs, track id). Returns false with a
    // descriptive error when nothing is close enough or the selection is
    // invalid; it never guesses.
    bool bindFromSelection(const CreatorTargetSelection &selection,
                           const QList<TargetTrack> &tracks,
                           QString *error = nullptr);

    // Explicitly binds an identity to an existing, unclaimed track id.
    bool bindToTrack(const QString &identity, const QString &targetId,
                     qint64 boundAtMs, const QList<TargetTrack> &tracks,
                     QString *error = nullptr);

    // Refreshes bindings against the latest tracks. A lost bound track is
    // re-bound only when exactly one active, unclaimed, label-compatible track
    // continues its predicted trajectory; otherwise the identity is marked
    // unresolved. Returns the identities whose resolution state changed.
    QStringList update(const QList<TargetTrack> &tracks, qint64 timeMs);

    bool isBound(const QString &identity) const;
    bool isResolved(const QString &identity) const;
    QString targetId(const QString &identity) const;
    const IdentityBinding *binding(const QString &identity) const;
    QStringList identities() const;
    const QStringList &notes() const;

    // --- appearance evidence (Objective 5) ---------------------------------
    // Stores the appearance reference used for re-acquisition. The registry
    // remains free of media/model concerns: appearance inference happens in an
    // external AppearanceProvider orchestrated by IdentityReidentifier.
    void setAppearanceProfile(const QString &identity,
                              const AppearanceProfile &profile);
    const AppearanceProfile *appearanceProfile(const QString &identity) const;
    bool hasAppearanceProfile(const QString &identity) const;

    // Re-binds an identity to a new track based on appearance evidence.
    bool rebindWithAppearance(const QString &identity, const QString &targetId,
                              double similarity, const QString &detail,
                              QString *error = nullptr);
    // Records appearance evidence without changing resolution.
    void annotateAppearance(const QString &identity, double similarity,
                            AppearanceVerdict verdict, const QString &detail);
    // Marks an identity unresolved with a reason, preserving the last target.
    void markUnresolved(const QString &identity, const QString &reason);
    // Vetoes a specific track for an identity (appearance conflict) and marks
    // the identity unresolved. Geometric re-binding will not re-accept it.
    void rejectTarget(const QString &identity, const QString &targetId,
                      const QString &reason);

    QJsonObject toJsonObject() const;
    bool readFromJsonObject(const QJsonObject &object, QString *error = nullptr);

private:
    int indexOf(const QString &identity) const;
    IdentityBinding *bindingFor(const QString &identity);
    bool trackClaimedByOther(const QString &targetId,
                             const QString &identity) const;

    Config m_config;
    QList<IdentityBinding> m_bindings;
    QList<AppearanceProfile> m_appearanceProfiles;
    QStringList m_notes;
};
