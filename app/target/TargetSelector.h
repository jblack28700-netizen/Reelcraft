#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "target/TargetIdentity.h"
#include "target/TargetTypes.h"

// TargetSelector resolves a natural-language target reference to exactly one
// track, deterministically, above detection/tracking and independently of the
// detector, model, or renderer.
//
// Supported references (case-insensitive; a leading "the" is optional):
//   - creator identity: "me", "myself", "my", "the person I selected",
//     "my selection", "the selected person", "the person I picked"
//   - relative: "the other person" / "the other one" / "the other"
//   - ordinal: "person 1", "person 2", ..., "the first person",
//     "the second person", ...
//   - spatial: "the person on the left", "the person on the right" (and
//     "my left"/"my right")
//   - exact track id (e.g. "t2") and exact label when it is unique
//
// Ordering never depends on detector output order: the canonical order is
// (first observation time, numeric track id, id string). When a reference is
// genuinely ambiguous (for example "the other person" with three candidates)
// the result is reported as ambiguous with its candidates; the selector never
// silently picks one.
struct TargetSelectionResult
{
    bool resolved = false;
    bool ambiguous = false;
    ReframeTarget target;
    QString reference;
    QString identity;
    QString targetId;
    QString method; // "identity" | "other-person" | "ordinal" | "left" | "right"
                    // | "track-id" | "label"
    QString error;
    QStringList candidates;
};

class TargetSelector
{
public:
    // Lowercase, trim, collapse whitespace, drop a leading "the" and trailing
    // punctuation.
    static QString normalizeReference(const QString &reference);

    // Deterministic canonical order: first observation time, then numeric track
    // id, then id string. Optionally restricted to a label.
    static QList<TargetTrack> canonicalOrder(const QList<TargetTrack> &tracks,
                                             const QString &label = QString());

    // Resolves one reference against the current tracks and identity state.
    static TargetSelectionResult select(const QString &reference,
                                        const QList<TargetTrack> &tracks,
                                        const TargetIdentityRegistry &registry,
                                        const QString &creatorIdentity = QStringLiteral("me"));

    // Convenience for ReframePlanBuilder: on success returns one ReframeTarget
    // whose id is the original reference (so the builder matches it); empty on
    // any unresolved/ambiguous reference.
    static QList<ReframeTarget> resolvedTargets(const QString &reference,
                                                const QList<TargetTrack> &tracks,
                                                const TargetIdentityRegistry &registry,
                                                const QString &creatorIdentity = QStringLiteral("me"));
};
