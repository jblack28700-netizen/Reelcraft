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
            if (EquirectProjection::angularDistanceDeg(directionOf(candidate),
                                                       directionOf(existing))
                <= mergeDistanceDeg) {
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
        for (int di = 0; di < detections.size(); ++di) {
            const TargetObservation &detection = detections.at(di);
            if (!labelsCompatible(track.label(), detection.label)) {
                continue;
            }
            const double distance = EquirectProjection::angularDistanceDeg(
                directionOf(last), directionOf(detection));
            if (distance > m_config.maxAssociationDistanceDeg) {
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
