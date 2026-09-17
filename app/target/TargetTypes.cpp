#include "TargetTypes.h"

#include "target/EquirectProjection.h"

#include <cmath>

bool TargetDetection::isValid() const
{
    return boundingBox.isValid() && std::isfinite(boundingBox.x())
        && std::isfinite(boundingBox.y()) && boundingBox.width() > 0.0
        && boundingBox.height() > 0.0 && std::isfinite(confidence)
        && confidence >= 0.0 && confidence <= 1.0;
}

bool TargetObservation::isValid() const
{
    return std::isfinite(yawDeg) && std::isfinite(pitchDeg)
        && std::isfinite(confidence) && confidence >= 0.0 && confidence <= 1.0
        && std::isfinite(yawRadiusDeg) && yawRadiusDeg >= 0.0
        && std::isfinite(pitchRadiusDeg) && pitchRadiusDeg >= 0.0
        && pitchDeg >= -90.0 && pitchDeg <= 90.0;
}

ReframeTarget TargetObservation::toReframeTarget() const
{
    ReframeTarget target;
    target.id = targetId.isEmpty() ? label : targetId;
    target.yawDeg = yawDeg;
    target.pitchDeg = pitchDeg;
    return target;
}

TargetTrack::TargetTrack(QString id, QString label)
    : m_id(std::move(id)), m_label(std::move(label))
{
}

QString TargetTrack::id() const
{
    return m_id;
}

QString TargetTrack::label() const
{
    return m_label;
}

void TargetTrack::setLabel(const QString &label)
{
    m_label = label;
}

bool TargetTrack::active() const
{
    return m_active;
}

void TargetTrack::setActive(bool active)
{
    m_active = active;
}

int TargetTrack::missCount() const
{
    return m_missCount;
}

void TargetTrack::setMissCount(int missCount)
{
    m_missCount = missCount;
}

const QList<TargetObservation> &TargetTrack::observations() const
{
    return m_observations;
}

void TargetTrack::append(const TargetObservation &observation)
{
    int index = m_observations.size();
    while (index > 0 && m_observations.at(index - 1).timeMs > observation.timeMs) {
        --index;
    }
    m_observations.insert(index, observation);
}

int TargetTrack::size() const
{
    return m_observations.size();
}

bool TargetTrack::isEmpty() const
{
    return m_observations.isEmpty();
}

qint64 TargetTrack::firstTimeMs() const
{
    return m_observations.isEmpty() ? 0 : m_observations.first().timeMs;
}

qint64 TargetTrack::lastTimeMs() const
{
    return m_observations.isEmpty() ? 0 : m_observations.last().timeMs;
}

bool TargetTrack::lastObservation(TargetObservation *out) const
{
    if (m_observations.isEmpty()) {
        return false;
    }
    if (out) {
        *out = m_observations.last();
    }
    return true;
}

double TargetTrack::meanConfidence() const
{
    if (m_observations.isEmpty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (const TargetObservation &observation : m_observations) {
        sum += observation.confidence;
    }
    return sum / static_cast<double>(m_observations.size());
}

bool TargetTrack::sampleAt(qint64 timeMs, TargetObservation *out) const
{
    if (m_observations.isEmpty()) {
        return false;
    }
    if (timeMs <= m_observations.first().timeMs) {
        if (out) {
            *out = m_observations.first();
        }
        return true;
    }
    if (timeMs >= m_observations.last().timeMs) {
        if (out) {
            *out = m_observations.last();
        }
        return true;
    }

    for (int i = 0; i + 1 < m_observations.size(); ++i) {
        const TargetObservation &a = m_observations.at(i);
        const TargetObservation &b = m_observations.at(i + 1);
        if (timeMs < a.timeMs || timeMs > b.timeMs) {
            continue;
        }
        if (!(b.timeMs > a.timeMs)) {
            if (out) {
                *out = b;
            }
            return true;
        }
        const double t = static_cast<double>(timeMs - a.timeMs)
            / static_cast<double>(b.timeMs - a.timeMs);
        TargetObservation sampled = a;
        const double delta = EquirectProjection::shortestYawDeltaDeg(a.yawDeg, b.yawDeg);
        sampled.yawDeg = EquirectProjection::normalizeYawDeg(a.yawDeg + delta * t);
        sampled.pitchDeg = EquirectProjection::clampPitchDeg(
            a.pitchDeg + (b.pitchDeg - a.pitchDeg) * t);
        sampled.confidence = a.confidence + (b.confidence - a.confidence) * t;
        sampled.timeMs = timeMs;
        if (out) {
            *out = sampled;
        }
        return true;
    }
    if (out) {
        *out = m_observations.last();
    }
    return true;
}

bool TargetTrack::representative(TargetObservation *out) const
{
    if (m_observations.isEmpty()) {
        return false;
    }
    int best = 0;
    for (int i = 1; i < m_observations.size(); ++i) {
        const TargetObservation &candidate = m_observations.at(i);
        const TargetObservation &current = m_observations.at(best);
        if (candidate.confidence > current.confidence
            || (candidate.confidence == current.confidence
                && candidate.timeMs >= current.timeMs)) {
            best = i;
        }
    }
    if (out) {
        *out = m_observations.at(best);
    }
    return true;
}

ReframeTarget TargetTrack::representativeTarget() const
{
    TargetObservation observation;
    if (!representative(&observation)) {
        return ReframeTarget();
    }
    return observation.toReframeTarget();
}
