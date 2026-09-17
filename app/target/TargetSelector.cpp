#include "target/TargetSelector.h"

#include "target/EquirectProjection.h"

#include <QRegularExpression>

#include <algorithm>
#include <climits>

namespace {

int trackNumber(const QString &id)
{
    if (id.size() >= 2 && id.at(0) == QLatin1Char('t')) {
        bool ok = false;
        const int number = id.mid(1).toInt(&ok);
        if (ok) {
            return number;
        }
    }
    return INT_MAX;
}

QStringList idsOf(const QList<TargetTrack> &tracks)
{
    QStringList ids;
    for (const TargetTrack &track : tracks) {
        ids.append(track.id());
    }
    return ids;
}

const TargetTrack *findById(const QList<TargetTrack> &tracks, const QString &id)
{
    for (const TargetTrack &track : tracks) {
        if (track.id() == id) {
            return &track;
        }
    }
    return nullptr;
}

TargetSelectionResult selectTrack(const TargetTrack &track,
                                  const QString &reference,
                                  const QString &method,
                                  const QString &identity = QString())
{
    TargetSelectionResult result;
    result.resolved = true;
    result.reference = reference;
    result.method = method;
    result.identity = identity;
    result.targetId = track.id();
    TargetObservation representative;
    if (track.representative(&representative)) {
        result.target.yawDeg = representative.yawDeg;
        result.target.pitchDeg = representative.pitchDeg;
    }
    // The id is the normalized reference so the existing ReframePlanBuilder
    // (which matches the parsed subject reference case-insensitively) resolves
    // it without any special-casing.
    result.target.id = TargetSelector::normalizeReference(reference);
    return result;
}

bool isCreatorAlias(const QString &reference)
{
    static const QStringList aliases = {
        QStringLiteral("me"),
        QStringLiteral("myself"),
        QStringLiteral("my self"),
        QStringLiteral("i"),
        QStringLiteral("my"),
        QStringLiteral("my selection"),
        QStringLiteral("my selected person"),
        QStringLiteral("selected person"),
        QStringLiteral("person i selected"),
        QStringLiteral("person i picked"),
        QStringLiteral("person i chose"),
        QStringLiteral("my pick"),
        QStringLiteral("chosen person"),
        QStringLiteral("my chosen person"),
    };
    return aliases.contains(reference);
}

bool isOtherAlias(const QString &reference)
{
    static const QStringList aliases = {
        QStringLiteral("other person"),
        QStringLiteral("other one"),
        QStringLiteral("other"),
        QStringLiteral("other guy"),
        QStringLiteral("other people"),
        QStringLiteral("the rest"),
    };
    return aliases.contains(reference);
}

int ordinalWord(const QString &word)
{
    static const QStringList words = {
        QStringLiteral("first"),  QStringLiteral("second"),
        QStringLiteral("third"),  QStringLiteral("fourth"),
        QStringLiteral("fifth"),  QStringLiteral("sixth"),
        QStringLiteral("seventh"), QStringLiteral("eighth"),
        QStringLiteral("ninth"),  QStringLiteral("tenth"),
    };
    const int index = words.indexOf(word);
    return index >= 0 ? index + 1 : 0;
}

} // namespace

QString TargetSelector::normalizeReference(const QString &reference)
{
    QString normalized = reference.toLower().simplified();
    while (normalized.startsWith(QStringLiteral("the "))) {
        normalized = normalized.mid(4).trimmed();
    }
    while (!normalized.isEmpty()
           && QStringLiteral(".,;:!?").contains(normalized.back())) {
        normalized.chop(1);
    }
    return normalized.trimmed();
}

QList<TargetTrack> TargetSelector::canonicalOrder(
    const QList<TargetTrack> &tracks, const QString &label)
{
    QList<TargetTrack> pool;
    for (const TargetTrack &track : tracks) {
        if (track.isEmpty()) {
            continue;
        }
        if (!label.isEmpty()
            && track.label().compare(label, Qt::CaseInsensitive) != 0) {
            continue;
        }
        pool.append(track);
    }
    std::stable_sort(pool.begin(), pool.end(),
                     [](const TargetTrack &a, const TargetTrack &b) {
                         if (a.firstTimeMs() != b.firstTimeMs()) {
                             return a.firstTimeMs() < b.firstTimeMs();
                         }
                         const int numberA = trackNumber(a.id());
                         const int numberB = trackNumber(b.id());
                         if (numberA != numberB) {
                             return numberA < numberB;
                         }
                         return a.id() < b.id();
                     });
    return pool;
}

TargetSelectionResult TargetSelector::select(
    const QString &reference, const QList<TargetTrack> &tracks,
    const TargetIdentityRegistry &registry, const QString &creatorIdentity)
{
    TargetSelectionResult result;
    result.reference = reference;
    const QString normalized = normalizeReference(reference);
    if (normalized.isEmpty()) {
        result.error = QStringLiteral("Empty target reference.");
        return result;
    }

    QList<TargetTrack> active;
    for (const TargetTrack &track : tracks) {
        if (track.active() && !track.isEmpty()) {
            active.append(track);
        }
    }
    QList<TargetTrack> people;
    for (const TargetTrack &track : active) {
        if (track.label().compare(QStringLiteral("person"),
                                  Qt::CaseInsensitive) == 0) {
            people.append(track);
        }
    }
    if (people.isEmpty()) {
        people = active; // graceful fallback when the detector uses other labels
    }

    // 1. Creator identity (e.g. "me").
    if (isCreatorAlias(normalized)) {
        result.identity =
            TargetIdentityRegistry::normalizeIdentity(creatorIdentity);
        if (!registry.isResolved(creatorIdentity)) {
            result.error = QStringLiteral(
                "Identity '%1' is not resolved; select the target first.")
                               .arg(result.identity);
            return result;
        }
        const TargetTrack *track =
            findById(active, registry.targetId(creatorIdentity));
        if (!track) {
            result.error = QStringLiteral(
                "Identity '%1' points to an inactive track.")
                               .arg(result.identity);
            return result;
        }
        return selectTrack(*track, reference, QStringLiteral("identity"),
                           result.identity);
    }

    // 2. "the other person" — requires a resolved creator target.
    if (isOtherAlias(normalized)) {
        const QString meId = registry.isResolved(creatorIdentity)
            ? registry.targetId(creatorIdentity)
            : QString();
        if (meId.isEmpty()) {
            result.error = QStringLiteral(
                "Cannot resolve 'the other person' without a selected creator "
                "target ('me').");
            return result;
        }
        QList<TargetTrack> others;
        for (const TargetTrack &track : people) {
            if (track.id() != meId) {
                others.append(track);
            }
        }
        if (others.isEmpty()) {
            result.error = QStringLiteral("No other person is currently visible.");
            return result;
        }
        if (others.size() > 1) {
            result.ambiguous = true;
            result.candidates = idsOf(others);
            result.error = QStringLiteral(
                "Multiple other people are visible; reference is ambiguous.");
            return result;
        }
        return selectTrack(others.first(), reference,
                           QStringLiteral("other-person"));
    }

    // 3. Ordinal references ("person 2", "the first person").
    {
        static const QRegularExpression personNumber(
            QStringLiteral("^(?:person|people|guy|man|woman|speaker)?\\s*"
                           "(?:number\\s*)?(\\d+)$"));
        static const QRegularExpression ordinalNumber(
            QStringLiteral("^(first|second|third|fourth|fifth|sixth|seventh|"
                           "eighth|ninth|tenth)(?:\\s+person)?$"));
        int index = 0;
        const QRegularExpressionMatch numberMatch = personNumber.match(normalized);
        if (numberMatch.hasMatch()) {
            index = numberMatch.captured(1).toInt();
        } else {
            const QRegularExpressionMatch wordMatch = ordinalNumber.match(normalized);
            if (wordMatch.hasMatch()) {
                index = ordinalWord(wordMatch.captured(1));
            }
        }
        if (index > 0) {
            const QList<TargetTrack> ordered = canonicalOrder(people, QString());
            if (index > ordered.size()) {
                result.error = QStringLiteral(
                    "person %1 is not visible (%2 person(s) detected).")
                                   .arg(index)
                                   .arg(ordered.size());
                return result;
            }
            return selectTrack(ordered.at(index - 1), reference,
                               QStringLiteral("ordinal"));
        }
    }

    // 4. Spatial references.
    if (normalized.contains(QStringLiteral("left"))
        || normalized.contains(QStringLiteral("right"))) {
        const bool wantLeft = normalized.contains(QStringLiteral("left"));
        const TargetTrack *best = nullptr;
        double bestYaw = 0.0;
        for (const TargetTrack &track : people) {
            TargetObservation representative;
            if (!track.representative(&representative)) {
                continue;
            }
            if (!best
                || (wantLeft ? representative.yawDeg < bestYaw
                             : representative.yawDeg > bestYaw)) {
                best = &track;
                bestYaw = representative.yawDeg;
            }
        }
        if (best) {
            return selectTrack(*best, reference,
                               wantLeft ? QStringLiteral("left")
                                        : QStringLiteral("right"));
        }
    }

    // 5. Exact track id.
    for (const TargetTrack &track : active) {
        if (track.id().compare(normalized, Qt::CaseInsensitive) == 0) {
            return selectTrack(track, reference, QStringLiteral("track-id"));
        }
    }

    // 6. Exact label when unique.
    QList<TargetTrack> labelMatches;
    for (const TargetTrack &track : active) {
        if (!track.label().isEmpty()
            && track.label().compare(normalized, Qt::CaseInsensitive) == 0) {
            labelMatches.append(track);
        }
    }
    if (labelMatches.size() == 1) {
        return selectTrack(labelMatches.first(), reference,
                           QStringLiteral("label"));
    }
    if (labelMatches.size() > 1) {
        result.ambiguous = true;
        result.candidates = idsOf(labelMatches);
        result.error = QStringLiteral(
            "Reference '%1' matches %2 tracks; use 'person 1', 'person 2', ...")
                           .arg(normalized)
                           .arg(labelMatches.size());
        return result;
    }

    result.error = QStringLiteral("Unrecognized target reference '%1'.")
                       .arg(reference);
    return result;
}

QList<ReframeTarget> TargetSelector::resolvedTargets(
    const QString &reference, const QList<TargetTrack> &tracks,
    const TargetIdentityRegistry &registry, const QString &creatorIdentity)
{
    const TargetSelectionResult result =
        select(reference, tracks, registry, creatorIdentity);
    if (!result.resolved) {
        return {};
    }
    return { result.target };
}
