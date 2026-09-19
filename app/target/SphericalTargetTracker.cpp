#include "target/SphericalTargetTracker.h"

#include "target/EquirectProjection.h"

#include <algorithm>

namespace {

bool labelsCompatible(const QString &a, const QString &b)
{
    return a.isEmpty() || b.isEmpty() || a == b;
}

SphericalDirection directionOf(const TargetObservation &observation)
{
    return { observation.yawDeg, observation.pitchDeg };
}

// Objective 27: are these two same-frame observations the SAME target, or two?
//
// One target that straddles the boundary between two covering views is reported
// twice: once complete by the view it sits in, and once as a clipped sliver by
// the neighbouring view. The sliver's bounding box is truncated by the view
// edge, so its centre -- and therefore the spherical direction derived from it
// -- is biased toward that view's axis. Measured on the reproducing fixture the
// two land 9.61 degrees apart with yaw radii of 11.68 and 1.35 degrees, i.e. the
// duplicate sits just outside a person-tuned 8 degree distance.
//
// A distance alone therefore cannot separate "one target seen twice" from "two
// targets": the clipped duplicate is not merely close, it is contained in the
// other observation's footprint, and the two footprints overlap. That is the
// test used here in addition to the distance:
//
//   * the caller's mergeDistanceDeg still applies unchanged, so person-sized
//     targets behave exactly as before; and
//   * beyond it, two observations are one target when their angular footprints
//     overlap, computed on the yaw radii because the covering views are
//     distributed in yaw and a yaw separation is what the duplicate produces.
//
// This deliberately is NOT a raised global threshold: the extension can only
// merge two observations when at least one of them reports an extent big enough
// to reach the other's centre, so an observation that reports no extent (zero
// radius) cannot trigger it at all, and two separate people whose reported
// footprints do not overlap are unaffected by it however the distance is set.
bool sameTargetAcrossCoveringViews(const TargetObservation &a,
                                   const TargetObservation &b,
                                   double mergeDistanceDeg)
{
    const double separation =
        EquirectProjection::angularDistanceDeg(directionOf(a), directionOf(b));
    if (separation <= mergeDistanceDeg) {
        return true;
    }
    return separation <= a.yawRadiusDeg + b.yawRadiusDeg;
}

} // namespace

SphericalTargetTracker::SphericalTargetTracker(Config config)
    : m_config(config)
{
}

void SphericalTargetTracker::reset()
{
    m_tracks.clear();
    m_nextNumber = 1;
}

SphericalTargetTracker::Config SphericalTargetTracker::config() const
{
    return m_config;
}

void SphericalTargetTracker::setConfig(const Config &config)
{
    m_config = config;
}

QList<TargetObservation> SphericalTargetTracker::mergeNearDuplicates(
    const QList<TargetObservation> &observations, double mergeDistanceDeg)
{
    QList<TargetObservation> sorted;
    for (const TargetObservation &observation : observations) {
        if (observation.isValid()) {
            sorted.append(observation);
        }
    }
    // Prefer the more confident detection, then the one with the larger angular
    // footprint. A target partially clipped at a view edge produces a smaller
    // footprint than the same target fully visible in an overlapping view, so
    // this keeps the more complete detection.
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const TargetObservation &a, const TargetObservation &b) {
                         if (a.confidence != b.confidence) {
                             return a.confidence > b.confidence;
                         }
                         const double areaA = a.yawRadiusDeg * a.pitchRadiusDeg;
                         const double areaB = b.yawRadiusDeg * b.pitchRadiusDeg;
                         if (areaA != areaB) {
                             return areaA > areaB;
                         }
                         if (a.label != b.label) {
                             return a.label < b.label;
                         }
                         if (a.yawDeg != b.yawDeg) {
                             return a.yawDeg < b.yawDeg;
                         }
                         return a.pitchDeg < b.pitchDeg;
                     });

    QList<TargetObservation> kept;
    for (const TargetObservation &candidate : sorted) {
        bool duplicate = false;
        for (const TargetObservation &existing : kept) {
            if (!labelsCompatible(candidate.label, existing.label)) {
                continue;
            }
            if (sameTargetAcrossCoveringViews(candidate, existing,
                                              mergeDistanceDeg)) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            kept.append(candidate);
        }
    }
    return kept;
}

QList<TargetObservation> SphericalTargetTracker::update(
    const QList<TargetObservation> &observations, qint64 timeMs)
{
    QList<TargetObservation> detections;
    for (const TargetObservation &observation : observations) {
        if (observation.isValid()
            && observation.confidence >= m_config.minConfidence) {
            detections.append(observation);
        }
    }
    detections = mergeNearDuplicates(detections, m_config.mergeDistanceDeg);

    struct Candidate
    {
        double distance = 0.0;
        double confidence = 0.0;
        int trackIndex = -1;
        int detectionIndex = -1;
    };
    QList<Candidate> candidates;
    for (int ti = 0; ti < m_tracks.size(); ++ti) {
        const TargetTrack &track = m_tracks.at(ti);
        TargetObservation last;
        if (!track.lastObservation(&last)) {
            continue;
        }
        // Deterministic constant-velocity prediction (yaw/pitch only; no
        // appearance). Keeps identity through crossing trajectories.
        SphericalDirection reference = directionOf(last);
        if (m_config.useVelocityPrediction && track.size() >= 2
            && timeMs > last.timeMs) {
            const TargetObservation &previous =
                track.observations().at(track.size() - 2);
            const qint64 dt = last.timeMs - previous.timeMs;
            if (dt > 0) {
                qint64 horizon = timeMs - last.timeMs;
                if (horizon > m_config.maxPredictionMs) {
                    horizon = m_config.maxPredictionMs;
                }
                const double velocityYaw =
                    EquirectProjection::shortestYawDeltaDeg(previous.yawDeg,
                                                            last.yawDeg)
                    / static_cast<double>(dt);
                const double velocityPitch =
                    (last.pitchDeg - previous.pitchDeg) / static_cast<double>(dt);
                reference.yawDeg = EquirectProjection::normalizeYawDeg(
                    last.yawDeg + velocityYaw * static_cast<double>(horizon));
                reference.pitchDeg = EquirectProjection::clampPitchDeg(
                    last.pitchDeg + velocityPitch * static_cast<double>(horizon));
            }
        }

        // A previously-missed track gets a wider (but bounded) re-entry gate.
        double gate = m_config.maxAssociationDistanceDeg;
        if (track.missCount() > 0
            && timeMs - last.timeMs <= m_config.reentryWindowMs) {
            gate = qMax(gate, m_config.reentryGateDeg);
        }

        for (int di = 0; di < detections.size(); ++di) {
            const TargetObservation &detection = detections.at(di);
            if (!labelsCompatible(track.label(), detection.label)) {
                continue;
            }
            const double distance = EquirectProjection::angularDistanceDeg(
                reference, directionOf(detection));
            if (distance > gate) {
                continue;
            }
            Candidate candidate;
            candidate.distance = distance;
            candidate.confidence = detection.confidence;
            candidate.trackIndex = ti;
            candidate.detectionIndex = di;
            candidates.append(candidate);
        }
    }

    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Candidate &a, const Candidate &b) {
                         if (a.distance != b.distance) {
                             return a.distance < b.distance;
                         }
                         if (a.confidence != b.confidence) {
                             return a.confidence > b.confidence;
                         }
                         if (a.detectionIndex != b.detectionIndex) {
                             return a.detectionIndex < b.detectionIndex;
                         }
                         return a.trackIndex < b.trackIndex;
                     });

    QList<bool> trackAssigned(m_tracks.size(), false);
    QList<bool> detectionAssigned(detections.size(), false);
    QList<TargetObservation> accepted;

    for (const Candidate &candidate : candidates) {
        if (trackAssigned.at(candidate.trackIndex)
            || detectionAssigned.at(candidate.detectionIndex)) {
            continue;
        }
        trackAssigned[candidate.trackIndex] = true;
        detectionAssigned[candidate.detectionIndex] = true;

        TargetTrack &track = m_tracks[candidate.trackIndex];
        TargetObservation observation = detections.at(candidate.detectionIndex);
        observation.timeMs = timeMs;
        observation.targetId = track.id();
        if (track.label().isEmpty()) {
            track.setLabel(observation.label);
        }
        track.append(observation);
        track.setMissCount(0);
        track.setActive(true);
        accepted.append(observation);
    }

    for (int di = 0; di < detections.size(); ++di) {
        if (detectionAssigned.at(di)) {
            continue;
        }
        TargetObservation observation = detections.at(di);
        const QString id = QStringLiteral("t%1").arg(m_nextNumber++);
        observation.timeMs = timeMs;
        observation.targetId = id;
        TargetTrack track(id, observation.label);
        track.append(observation);
        track.setMissCount(0);
        track.setActive(true);
        m_tracks.append(track);
        trackAssigned.append(true);
        accepted.append(observation);
    }

    for (int ti = 0; ti < m_tracks.size(); ++ti) {
        if (trackAssigned.at(ti)) {
            continue;
        }
        TargetTrack &track = m_tracks[ti];
        if (track.isEmpty()) {
            continue;
        }
        track.setMissCount(track.missCount() + 1);
        if (track.missCount() > m_config.maxMisses) {
            track.setActive(false);
        }
    }

    std::stable_sort(accepted.begin(), accepted.end(),
                     [](const TargetObservation &a, const TargetObservation &b) {
                         if (a.targetId != b.targetId) {
                             return a.targetId < b.targetId;
                         }
                         return a.timeMs < b.timeMs;
                     });
    return accepted;
}

const QList<TargetTrack> &SphericalTargetTracker::tracks() const
{
    return m_tracks;
}

QList<TargetTrack> SphericalTargetTracker::activeTracks() const
{
    QList<TargetTrack> active;
    for (const TargetTrack &track : m_tracks) {
        if (track.active() && !track.isEmpty()) {
            active.append(track);
        }
    }
    return active;
}

const TargetTrack *SphericalTargetTracker::trackById(const QString &id) const
{
    for (const TargetTrack &track : m_tracks) {
        if (track.id() == id) {
            return &track;
        }
    }
    return nullptr;
}
