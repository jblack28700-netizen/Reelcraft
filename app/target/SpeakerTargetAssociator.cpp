#include "target/SpeakerTargetAssociator.h"

#include "target/EquirectProjection.h"

#include <cmath>

void SpeakerTargetAssociator::bind(const QString &speakerId,
                                   const QString &targetId)
{
    if (speakerId.isEmpty() || targetId.isEmpty()) {
        return;
    }
    for (QPair<QString, QString> &binding : m_bindings) {
        if (binding.first == speakerId) {
            binding.second = targetId;
            return;
        }
    }
    m_bindings.append({ speakerId, targetId });
}

void SpeakerTargetAssociator::clearBindings()
{
    m_bindings.clear();
}

bool SpeakerTargetAssociator::isBound(const QString &speakerId) const
{
    return !boundTarget(speakerId).isEmpty();
}

QString SpeakerTargetAssociator::boundTarget(const QString &speakerId) const
{
    for (const QPair<QString, QString> &binding : m_bindings) {
        if (binding.first == speakerId) {
            return binding.second;
        }
    }
    return QString();
}

QList<QPair<QString, QString>> SpeakerTargetAssociator::bindings() const
{
    return m_bindings;
}

bool SpeakerTargetAssociator::associate(const SpeakerInterval &interval,
                                        const QList<TargetTrack> &visibleTracks,
                                        const Config &config,
                                        QString *outTargetId, bool *outAmbiguous,
                                        QString *outMethod, QString *error) const
{
    if (error) {
        error->clear();
    }
    if (outTargetId) {
        outTargetId->clear();
    }
    if (outAmbiguous) {
        *outAmbiguous = false;
    }
    if (outMethod) {
        outMethod->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!outTargetId || !outAmbiguous || !outMethod) {
        return fail(QStringLiteral("Speaker association output is null."));
    }

    QList<TargetTrack> candidates;
    for (const TargetTrack &track : visibleTracks) {
        if (track.isEmpty() || !track.active()) {
            continue;
        }
        if (!config.label.isEmpty()
            && track.label().compare(config.label, Qt::CaseInsensitive) != 0) {
            continue;
        }
        candidates.append(track);
    }

    // 1. Explicit binding.
    const QString bound = boundTarget(interval.speakerId);
    if (!bound.isEmpty()) {
        for (const TargetTrack &track : candidates) {
            if (track.id() == bound) {
                *outTargetId = bound;
                *outMethod = QStringLiteral("explicit");
                return true;
            }
        }
        // The explicitly bound target is not visible: do not invent one.
        return true;
    }

    if (candidates.isEmpty()) {
        return true; // unassociated
    }

    // 2. Spatial (direction of arrival), unique only.
    if (interval.hasAzimuth && std::isfinite(interval.azimuthDeg)) {
        double best = 1.0e9;
        int bestIndex = -1;
        int ties = 0;
        for (int i = 0; i < candidates.size(); ++i) {
            const TargetTrack &track = candidates.at(i);
            TargetObservation observation;
            if (!track.sampleAt(interval.startMs, &observation)) {
                continue;
            }
            const double distance = qAbs(EquirectProjection::shortestYawDeltaDeg(
                interval.azimuthDeg, observation.yawDeg));
            if (distance < best - 1e-9) {
                best = distance;
                bestIndex = i;
                ties = 1;
            } else if (distance <= best + 1e-9) {
                ++ties;
            }
        }
        if (bestIndex >= 0 && best <= config.spatialGateDeg) {
            if (ties == 1) {
                *outTargetId = candidates.at(bestIndex).id();
                *outMethod = QStringLiteral("spatial");
                return true;
            }
            *outAmbiguous = true;
            return true;
        }
    }

    // 3. Single visible person.
    if (candidates.size() == 1) {
        *outTargetId = candidates.first().id();
        *outMethod = QStringLiteral("single-visible");
        return true;
    }

    // 4. Several plausible people and no explicit/spatial resolution.
    *outAmbiguous = true;
    return true;
}
