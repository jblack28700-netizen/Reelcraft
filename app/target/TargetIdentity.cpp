#include "target/TargetIdentity.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QtMath>

#include <algorithm>
#include <cmath>

#include "target/EquirectProjection.h"

namespace {

const QString kCreatorIdentity = QStringLiteral("me");

SphericalDirection predictedDirection(const TargetTrack &track, qint64 atMs,
                                      qint64 maxHorizonMs)
{
    TargetObservation last;
    if (!track.lastObservation(&last)) {
        return SphericalDirection();
    }
    SphericalDirection direction{ last.yawDeg, last.pitchDeg };
    if (track.size() >= 2 && atMs > last.timeMs) {
        const TargetObservation &previous =
            track.observations().at(track.size() - 2);
        const qint64 dt = last.timeMs - previous.timeMs;
        if (dt > 0) {
            qint64 horizon = atMs - last.timeMs;
            if (horizon > maxHorizonMs) {
                horizon = maxHorizonMs;
            }
            const double velocityYaw =
                EquirectProjection::shortestYawDeltaDeg(previous.yawDeg,
                                                        last.yawDeg)
                / static_cast<double>(dt);
            const double velocityPitch =
                (last.pitchDeg - previous.pitchDeg) / static_cast<double>(dt);
            direction.yawDeg = EquirectProjection::normalizeYawDeg(
                last.yawDeg + velocityYaw * static_cast<double>(horizon));
            direction.pitchDeg = EquirectProjection::clampPitchDeg(
                last.pitchDeg + velocityPitch * static_cast<double>(horizon));
        }
    }
    return direction;
}

bool labelsCompatible(const QString &a, const QString &b)
{
    return a.isEmpty() || b.isEmpty() || a == b;
}

} // namespace

bool CreatorTargetSelection::isValid(QString *error) const
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
    if (TargetIdentityRegistry::normalizeIdentity(identity).isEmpty()) {
        return fail(QStringLiteral("Target selection identity is empty."));
    }
    if (timeMs < 0) {
        return fail(QStringLiteral("Target selection time must not be negative."));
    }
    if (targetId.isEmpty()) {
        if (!EquirectProjection::isValidDirection(yawDeg, pitchDeg)) {
            return fail(QStringLiteral(
                "Target selection needs a valid direction or a track id."));
        }
    }
    return true;
}

QJsonObject CreatorTargetSelection::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("identity"), identity);
    object.insert(QStringLiteral("timeMs"), static_cast<double>(timeMs));
    object.insert(QStringLiteral("yawDeg"), yawDeg);
    object.insert(QStringLiteral("pitchDeg"), pitchDeg);
    if (!targetId.isEmpty()) {
        object.insert(QStringLiteral("targetId"), targetId);
    }
    if (!label.isEmpty()) {
        object.insert(QStringLiteral("label"), label);
    }
    if (!evidence.isEmpty()) {
        object.insert(QStringLiteral("evidence"), evidence);
    }
    return object;
}

bool CreatorTargetSelection::readFromJsonObject(const QJsonObject &object,
                                                CreatorTargetSelection *out,
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
        return fail(QStringLiteral("Target selection output is null."));
    }
    CreatorTargetSelection selection;
    selection.identity = object.value(QStringLiteral("identity")).toString();
    const QJsonValue timeValue = object.value(QStringLiteral("timeMs"));
    const QJsonValue yawValue = object.value(QStringLiteral("yawDeg"));
    const QJsonValue pitchValue = object.value(QStringLiteral("pitchDeg"));
    if (!timeValue.isDouble() || !yawValue.isDouble() || !pitchValue.isDouble()) {
        return fail(QStringLiteral(
            "Target selection is missing numeric time/yaw/pitch."));
    }
    selection.timeMs = static_cast<qint64>(timeValue.toDouble());
    selection.yawDeg = yawValue.toDouble();
    selection.pitchDeg = pitchValue.toDouble();
    selection.targetId = object.value(QStringLiteral("targetId")).toString();
    selection.label = object.value(QStringLiteral("label")).toString();
    selection.evidence = object.value(QStringLiteral("evidence")).toString();
    QString validationError;
    if (!selection.isValid(&validationError)) {
        return fail(validationError);
    }
    *out = selection;
    return true;
}

QJsonObject IdentityBinding::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("identity"), identity);
    object.insert(QStringLiteral("targetId"), targetId);
    object.insert(QStringLiteral("boundAtMs"), static_cast<double>(boundAtMs));
    object.insert(QStringLiteral("method"), method);
    object.insert(QStringLiteral("distanceDeg"), distanceDeg);
    object.insert(QStringLiteral("resolved"), resolved);
    object.insert(QStringLiteral("appearanceSimilarity"), appearanceSimilarity);
    if (!appearanceVerdict.isEmpty()) {
        object.insert(QStringLiteral("appearanceVerdict"), appearanceVerdict);
    }
    if (!rejectedTargetIds.isEmpty()) {
        QJsonArray rejected;
        for (const QString &id : rejectedTargetIds) {
            rejected.append(id);
        }
        object.insert(QStringLiteral("rejectedTargetIds"), rejected);
    }
    object.insert(QStringLiteral("speakerConfidence"), speakerConfidence);
    if (!speakerId.isEmpty()) {
        object.insert(QStringLiteral("speakerId"), speakerId);
    }
    if (!speakerVerdict.isEmpty()) {
        object.insert(QStringLiteral("speakerVerdict"), speakerVerdict);
    }
    return object;
}

bool IdentityBinding::readFromJsonObject(const QJsonObject &object,
                                         IdentityBinding *out, QString *error)
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
        return fail(QStringLiteral("Identity binding output is null."));
    }
    IdentityBinding binding;
    binding.identity = object.value(QStringLiteral("identity")).toString();
    binding.targetId = object.value(QStringLiteral("targetId")).toString();
    binding.method = object.value(QStringLiteral("method")).toString();
    const QJsonValue boundValue = object.value(QStringLiteral("boundAtMs"));
    const QJsonValue distanceValue = object.value(QStringLiteral("distanceDeg"));
    if (!boundValue.isDouble() || !distanceValue.isDouble()
        || binding.identity.isEmpty()) {
        return fail(QStringLiteral("Identity binding is missing required fields."));
    }
    binding.boundAtMs = static_cast<qint64>(boundValue.toDouble());
    binding.distanceDeg = distanceValue.toDouble();
    binding.resolved = object.value(QStringLiteral("resolved")).toBool(false);
    binding.appearanceSimilarity =
        object.value(QStringLiteral("appearanceSimilarity")).toDouble(-1.0);
    binding.appearanceVerdict =
        object.value(QStringLiteral("appearanceVerdict")).toString();
    const QJsonValue rejectedValue =
        object.value(QStringLiteral("rejectedTargetIds"));
    if (rejectedValue.isArray()) {
        for (const QJsonValue &value : rejectedValue.toArray()) {
            if (value.isString()) {
                binding.rejectedTargetIds.append(value.toString());
            }
        }
    }
    binding.speakerId = object.value(QStringLiteral("speakerId")).toString();
    binding.speakerConfidence =
        object.value(QStringLiteral("speakerConfidence")).toDouble(-1.0);
    binding.speakerVerdict =
        object.value(QStringLiteral("speakerVerdict")).toString();
    *out = binding;
    return true;
}

TargetIdentityRegistry::TargetIdentityRegistry(Config config)
    : m_config(config)
{
}

void TargetIdentityRegistry::reset()
{
    m_bindings.clear();
    m_appearanceProfiles.clear();
    m_notes.clear();
}

TargetIdentityRegistry::Config TargetIdentityRegistry::config() const
{
    return m_config;
}

void TargetIdentityRegistry::setConfig(const Config &config)
{
    m_config = config;
}

QString TargetIdentityRegistry::normalizeIdentity(const QString &identity)
{
    QString normalized = identity.toLower().simplified();
    while (normalized.startsWith(QStringLiteral("the "))) {
        normalized = normalized.mid(4).trimmed();
    }
    return normalized;
}

QString TargetIdentityRegistry::creatorIdentity()
{
    return kCreatorIdentity;
}

int TargetIdentityRegistry::indexOf(const QString &identity) const
{
    const QString normalized = normalizeIdentity(identity);
    for (int i = 0; i < m_bindings.size(); ++i) {
        if (m_bindings.at(i).identity == normalized) {
            return i;
        }
    }
    return -1;
}

IdentityBinding *TargetIdentityRegistry::bindingFor(const QString &identity)
{
    const int index = indexOf(identity);
    return index >= 0 ? &m_bindings[index] : nullptr;
}

bool TargetIdentityRegistry::trackClaimedByOther(const QString &targetId,
                                                 const QString &identity) const
{
    const QString normalized = normalizeIdentity(identity);
    for (const IdentityBinding &binding : m_bindings) {
        if (binding.identity != normalized && binding.targetId == targetId) {
            return true;
        }
    }
    return false;
}

bool TargetIdentityRegistry::bindFromSelection(
    const CreatorTargetSelection &selection, const QList<TargetTrack> &tracks,
    QString *error)
{
    if (error) {
        error->clear();
    }
    m_notes.clear();
    QString validationError;
    if (!selection.isValid(&validationError)) {
        if (error) {
            *error = validationError;
        }
        return false;
    }
    const QString identity = normalizeIdentity(selection.identity);

    if (!selection.targetId.isEmpty()) {
        return bindToTrack(identity, selection.targetId, selection.timeMs, tracks,
                           error);
    }

    struct Best
    {
        double distance = 1.0e9;
        double confidence = -1.0;
        qint64 firstTimeMs = 0;
        QString trackId;
        bool found = false;
    };
    Best best;
    for (const TargetTrack &track : tracks) {
        if (!labelsCompatible(selection.label, track.label())) {
            continue;
        }
        for (const TargetObservation &observation : track.observations()) {
            if (!observation.isValid()) {
                continue;
            }
            if (qAbs(observation.timeMs - selection.timeMs)
                > m_config.bindingSearchWindowMs) {
                continue;
            }
            const double distance = EquirectProjection::angularDistanceDeg(
                SphericalDirection{ selection.yawDeg, selection.pitchDeg },
                SphericalDirection{ observation.yawDeg, observation.pitchDeg });
            const bool better = !best.found || distance < best.distance
                || (distance == best.distance
                    && observation.confidence > best.confidence)
                || (distance == best.distance
                    && observation.confidence == best.confidence
                    && track.firstTimeMs() < best.firstTimeMs)
                || (distance == best.distance
                    && observation.confidence == best.confidence
                    && track.firstTimeMs() == best.firstTimeMs
                    && track.id() < best.trackId);
            if (better) {
                best.distance = distance;
                best.confidence = observation.confidence;
                best.firstTimeMs = track.firstTimeMs();
                best.trackId = track.id();
                best.found = true;
            }
        }
    }

    if (!best.found) {
        if (error) {
            *error = QStringLiteral(
                "No target near the creator selection at t=%1 ms.")
                         .arg(selection.timeMs);
        }
        return false;
    }
    if (best.distance > m_config.bindingGateDeg) {
        if (error) {
            *error = QStringLiteral(
                "Nearest target is %1 deg from the selection (gate %2 deg).")
                         .arg(best.distance, 0, 'f', 2)
                         .arg(m_config.bindingGateDeg, 0, 'f', 2);
        }
        return false;
    }
    if (trackClaimedByOther(best.trackId, identity)) {
        if (error) {
            *error = QStringLiteral(
                "Track %1 is already bound to another identity.")
                         .arg(best.trackId);
        }
        return false;
    }

    IdentityBinding binding;
    binding.identity = identity;
    binding.targetId = best.trackId;
    binding.boundAtMs = selection.timeMs;
    binding.method = QStringLiteral("seed-direction");
    binding.distanceDeg = best.distance;
    binding.resolved = true;

    const int existing = indexOf(identity);
    if (existing >= 0) {
        m_bindings[existing] = binding;
    } else {
        m_bindings.append(binding);
    }
    m_notes.append(QStringLiteral(
        "Bound identity '%1' to track %2 (seed-direction, %3 deg).")
                       .arg(identity, best.trackId)
                       .arg(best.distance, 0, 'f', 2));
    return true;
}

bool TargetIdentityRegistry::bindToTrack(const QString &identity,
                                         const QString &targetId,
                                         qint64 boundAtMs,
                                         const QList<TargetTrack> &tracks,
                                         QString *error)
{
    if (error) {
        error->clear();
    }
    const QString normalized = normalizeIdentity(identity);
    if (normalized.isEmpty() || targetId.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Identity and target id are required.");
        }
        return false;
    }
    const TargetTrack *track = nullptr;
    for (const TargetTrack &candidate : tracks) {
        if (candidate.id() == targetId) {
            track = &candidate;
            break;
        }
    }
    if (!track) {
        if (error) {
            *error = QStringLiteral("Track %1 does not exist.").arg(targetId);
        }
        return false;
    }
    if (trackClaimedByOther(targetId, normalized)) {
        if (error) {
            *error = QStringLiteral("Track %1 is already bound to another identity.")
                         .arg(targetId);
        }
        return false;
    }

    IdentityBinding binding;
    binding.identity = normalized;
    binding.targetId = targetId;
    binding.boundAtMs = boundAtMs;
    binding.method = QStringLiteral("track-id");
    binding.resolved = true;
    const int existing = indexOf(normalized);
    if (existing >= 0) {
        m_bindings[existing] = binding;
    } else {
        m_bindings.append(binding);
    }
    return true;
}

QStringList TargetIdentityRegistry::update(const QList<TargetTrack> &tracks,
                                           qint64 timeMs)
{
    m_notes.clear();
    QStringList changed;
    for (IdentityBinding &binding : m_bindings) {
        bool wasResolved = binding.resolved;
        const QString previousTarget = binding.targetId;

        const TargetTrack *track = nullptr;
        for (const TargetTrack &candidate : tracks) {
            if (candidate.id() == binding.targetId) {
                track = &candidate;
                break;
            }
        }

        const bool currentlyResolved = track && !track->isEmpty()
            && track->active()
            && !binding.rejectedTargetIds.contains(binding.targetId);
        if (currentlyResolved) {
            binding.resolved = true;
            if (!wasResolved) {
                changed.append(binding.identity);
            }
            continue;
        }

        // Attempt one deterministic continuity re-binding.
        struct Rebind
        {
            double distance = 1.0e9;
            qint64 firstTimeMs = 0;
            QString targetId;
            bool found = false;
        };
        QList<Rebind> candidates;
        if (track && !track->isEmpty()) {
            TargetObservation last;
            track->lastObservation(&last);
            for (const TargetTrack &candidate : tracks) {
                if (candidate.id() == binding.targetId || candidate.isEmpty()
                    || !candidate.active()) {
                    continue;
                }
                if (!labelsCompatible(track->label(), candidate.label())) {
                    continue;
                }
                const TargetObservation &first = candidate.observations().first();
                if (first.timeMs + 1 < last.timeMs) {
                    continue;
                }
                if (first.timeMs - last.timeMs > m_config.rebindWindowMs) {
                    continue;
                }
                const SphericalDirection predicted = predictedDirection(
                    *track, first.timeMs, m_config.maxPredictionMs);
                const double distance = EquirectProjection::angularDistanceDeg(
                    predicted,
                    SphericalDirection{ first.yawDeg, first.pitchDeg });
                if (distance > m_config.rebindGateDeg) {
                    continue;
                }
                if (trackClaimedByOther(candidate.id(), binding.identity)) {
                    continue;
                }
                if (binding.rejectedTargetIds.contains(candidate.id())) {
                    continue;
                }
                Rebind rebind;
                rebind.distance = distance;
                rebind.firstTimeMs = candidate.firstTimeMs();
                rebind.targetId = candidate.id();
                rebind.found = true;
                candidates.append(rebind);
            }
            std::stable_sort(candidates.begin(), candidates.end(),
                             [](const Rebind &a, const Rebind &b) {
                                 if (a.distance != b.distance) {
                                     return a.distance < b.distance;
                                 }
                                 if (a.firstTimeMs != b.firstTimeMs) {
                                     return a.firstTimeMs < b.firstTimeMs;
                                 }
                                 return a.targetId < b.targetId;
                             });
        }

        if (candidates.size() == 1) {
            binding.targetId = candidates.first().targetId;
            binding.boundAtMs = timeMs;
            binding.method = QStringLiteral("continuity-rebind");
            binding.distanceDeg = candidates.first().distance;
            binding.resolved = true;
            m_notes.append(QStringLiteral(
                "Identity '%1' re-bound to track %2 by continuity (%3 deg).")
                               .arg(binding.identity, binding.targetId)
                               .arg(binding.distanceDeg, 0, 'f', 2));
        } else {
            binding.resolved = false;
            if (candidates.size() > 1) {
                m_notes.append(QStringLiteral(
                    "Identity '%1' ambiguous: %2 candidate tracks continue its "
                    "trajectory.").arg(binding.identity)
                                   .arg(candidates.size()));
            } else {
                m_notes.append(QStringLiteral(
                    "Identity '%1' is unresolved (track %2 lost at %3 ms).")
                                   .arg(binding.identity,
                                        previousTarget.isEmpty()
                                            ? QStringLiteral("?")
                                            : previousTarget)
                                   .arg(timeMs));
            }
        }
        if (wasResolved != binding.resolved
            || previousTarget != binding.targetId) {
            changed.append(binding.identity);
        }
    }
    return changed;
}

bool TargetIdentityRegistry::isBound(const QString &identity) const
{
    return indexOf(identity) >= 0;
}

bool TargetIdentityRegistry::isResolved(const QString &identity) const
{
    const int index = indexOf(identity);
    return index >= 0 && m_bindings.at(index).resolved
        && !m_bindings.at(index).targetId.isEmpty();
}

QString TargetIdentityRegistry::targetId(const QString &identity) const
{
    const int index = indexOf(identity);
    return index >= 0 ? m_bindings.at(index).targetId : QString();
}

const IdentityBinding *TargetIdentityRegistry::binding(
    const QString &identity) const
{
    const int index = indexOf(identity);
    return index >= 0 ? &m_bindings.at(index) : nullptr;
}

QStringList TargetIdentityRegistry::identities() const
{
    QStringList result;
    for (const IdentityBinding &binding : m_bindings) {
        result.append(binding.identity);
    }
    return result;
}

const QStringList &TargetIdentityRegistry::notes() const
{
    return m_notes;
}

void TargetIdentityRegistry::setAppearanceProfile(
    const QString &identity, const AppearanceProfile &profile)
{
    const QString normalized = normalizeIdentity(identity);
    AppearanceProfile copy = profile;
    copy.identity = normalized;
    for (int i = 0; i < m_appearanceProfiles.size(); ++i) {
        if (m_appearanceProfiles.at(i).identity == normalized) {
            m_appearanceProfiles[i] = copy;
            return;
        }
    }
    m_appearanceProfiles.append(copy);
}

const AppearanceProfile *TargetIdentityRegistry::appearanceProfile(
    const QString &identity) const
{
    const QString normalized = normalizeIdentity(identity);
    for (const AppearanceProfile &profile : m_appearanceProfiles) {
        if (profile.identity == normalized) {
            return &profile;
        }
    }
    return nullptr;
}

bool TargetIdentityRegistry::hasAppearanceProfile(const QString &identity) const
{
    return appearanceProfile(identity) != nullptr;
}

bool TargetIdentityRegistry::rebindWithAppearance(
    const QString &identity, const QString &targetId, double similarity,
    const QString &detail, QString *error)
{
    if (error) {
        error->clear();
    }
    const QString normalized = normalizeIdentity(identity);
    IdentityBinding *binding = bindingFor(normalized);
    if (!binding) {
        if (error) {
            *error = QStringLiteral("Identity '%1' has no binding.").arg(normalized);
        }
        return false;
    }
    if (targetId.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Appearance re-bind requires a target id.");
        }
        return false;
    }
    if (trackClaimedByOther(targetId, normalized)) {
        if (error) {
            *error = QStringLiteral(
                "Track %1 is already bound to another identity.").arg(targetId);
        }
        return false;
    }
    binding->targetId = targetId;
    binding->method = QStringLiteral("appearance-rebind");
    binding->resolved = true;
    binding->appearanceSimilarity = similarity;
    binding->appearanceVerdict =
        appearanceVerdictToString(AppearanceVerdict::Agree);
    binding->rejectedTargetIds.removeAll(targetId);
    m_notes.append(QStringLiteral(
        "Identity '%1' re-acquired track %2 by appearance (%3). %4")
                       .arg(normalized, targetId)
                       .arg(similarity, 0, 'f', 3)
                       .arg(detail));
    return true;
}

void TargetIdentityRegistry::annotateAppearance(const QString &identity,
                                                double similarity,
                                                AppearanceVerdict verdict,
                                                const QString &detail)
{
    IdentityBinding *binding = bindingFor(identity);
    if (!binding) {
        return;
    }
    binding->appearanceSimilarity = similarity;
    binding->appearanceVerdict = appearanceVerdictToString(verdict);
    if (!detail.isEmpty()) {
        m_notes.append(QStringLiteral("Identity '%1' appearance: %2")
                           .arg(binding->identity, detail));
    }
}

void TargetIdentityRegistry::markUnresolved(const QString &identity,
                                            const QString &reason)
{
    IdentityBinding *binding = bindingFor(identity);
    if (!binding) {
        return;
    }
    binding->resolved = false;
    if (!reason.isEmpty()) {
        m_notes.append(QStringLiteral("Identity '%1' unresolved: %2")
                           .arg(binding->identity, reason));
    }
}

void TargetIdentityRegistry::annotateSpeaker(const QString &identity,
                                             const QString &speakerId,
                                             double confidence,
                                             SpeakerVerdict verdict,
                                             const QString &detail)
{
    IdentityBinding *binding = bindingFor(identity);
    if (!binding) {
        return;
    }
    binding->speakerId = speakerId;
    binding->speakerConfidence = confidence;
    binding->speakerVerdict = speakerVerdictToString(verdict);
    if (!detail.isEmpty()) {
        m_notes.append(QStringLiteral("Identity '%1' speaker: %2")
                           .arg(binding->identity, detail));
    }
}

void TargetIdentityRegistry::rejectTarget(const QString &identity,
                                          const QString &targetId,
                                          const QString &reason)
{
    IdentityBinding *binding = bindingFor(identity);
    if (!binding) {
        return;
    }
    if (!targetId.isEmpty() && !binding->rejectedTargetIds.contains(targetId)) {
        binding->rejectedTargetIds.append(targetId);
    }
    if (binding->targetId == targetId) {
        binding->resolved = false;
    }
    if (!reason.isEmpty()) {
        m_notes.append(QStringLiteral("Identity '%1' rejected track %2: %3")
                           .arg(binding->identity, targetId, reason));
    }
}

QJsonObject TargetIdentityRegistry::toJsonObject() const
{
    QJsonArray array;
    for (const IdentityBinding &binding : m_bindings) {
        array.append(binding.toJsonObject());
    }
    QJsonArray profiles;
    for (const AppearanceProfile &profile : m_appearanceProfiles) {
        profiles.append(profile.toJsonObject());
    }
    QJsonObject object;
    object.insert(QStringLiteral("bindings"), array);
    object.insert(QStringLiteral("appearanceProfiles"), profiles);
    return object;
}

bool TargetIdentityRegistry::readFromJsonObject(const QJsonObject &object,
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
    const QJsonValue bindingsValue = object.value(QStringLiteral("bindings"));
    if (!bindingsValue.isArray()) {
        return fail(QStringLiteral("Identity state is missing a bindings array."));
    }
    QList<IdentityBinding> bindings;
    for (const QJsonValue &value : bindingsValue.toArray()) {
        if (!value.isObject()) {
            return fail(QStringLiteral("Identity binding entry is not an object."));
        }
        IdentityBinding binding;
        QString bindingError;
        if (!IdentityBinding::readFromJsonObject(value.toObject(), &binding,
                                                 &bindingError)) {
            return fail(bindingError);
        }
        binding.identity = normalizeIdentity(binding.identity);
        binding.resolved = false; // refreshed against live tracks by update()
        bindings.append(binding);
    }
    m_bindings = bindings;
    m_appearanceProfiles.clear();
    const QJsonValue profilesValue =
        object.value(QStringLiteral("appearanceProfiles"));
    if (profilesValue.isArray()) {
        for (const QJsonValue &value : profilesValue.toArray()) {
            if (!value.isObject()) {
                return fail(QStringLiteral(
                    "Appearance profile entry is not an object."));
            }
            AppearanceProfile profile;
            QString profileError;
            if (!AppearanceProfile::readFromJsonObject(value.toObject(), &profile,
                                                       &profileError)) {
                return fail(profileError);
            }
            profile.identity = normalizeIdentity(profile.identity);
            m_appearanceProfiles.append(profile);
        }
    }
    m_notes.clear();
    return true;
}
