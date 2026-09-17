#pragma once

#include <QList>
#include <QRectF>
#include <QString>

#include "reframe/ReframeIntent.h" // ReframeTarget

// 360 Reframing Objective 2 — target/subject resolution types.
//
// These types form the boundary between a replaceable detector/tracker and the
// existing deterministic reframing engine. A detector reports 2D boxes in a
// perspective (tangent) view of the 360 source; the resolver maps them into
// spherical directions (yaw/pitch) using the exact EquirectView convention; and
// a tracker maintains identity over time. Nothing here fabricates a direction:
// an unresolved target simply produces no observation.

// One 2D detection inside a perspective view (view-pixel coordinates).
struct TargetDetection
{
    QRectF boundingBox;
    double confidence = 0.0;
    QString label;
    QString targetId; // detector-supplied identity, optional

    bool isValid() const;
};

// What the caller is looking for. Empty label/targetId means "any".
struct TargetQuery
{
    QString label;
    QString targetId;
    double minConfidence = 0.0;
};

// One resolved target observation in spherical (yaw/pitch) coordinates.
struct TargetObservation
{
    qint64 timeMs = 0;
    int frameIndex = -1;
    QString targetId;
    QString label;
    double confidence = 0.0;
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    // Angular half-extent of the detection mapped to the sphere (>= 0). Useful
    // for framing decisions; 0 when unknown.
    double yawRadiusDeg = 0.0;
    double pitchRadiusDeg = 0.0;
    QString source; // evidence: view/detector description

    bool isValid() const;
    ReframeTarget toReframeTarget() const;
};

// A temporally ordered track of observations for a single identity.
class TargetTrack
{
public:
    TargetTrack() = default;
    TargetTrack(QString id, QString label);

    QString id() const;
    QString label() const;
    void setLabel(const QString &label);
    bool active() const;
    void setActive(bool active);
    int missCount() const;
    void setMissCount(int missCount);

    const QList<TargetObservation> &observations() const;
    void append(const TargetObservation &observation);
    int size() const;
    bool isEmpty() const;
    qint64 firstTimeMs() const;
    qint64 lastTimeMs() const;
    bool lastObservation(TargetObservation *out) const;
    double meanConfidence() const;

    // Interpolated observation at timeMs. Yaw follows the shortest angular path;
    // pitch and confidence interpolate linearly. Before the first observation
    // returns the first; after the last returns the last. Returns false only
    // when the track is empty.
    bool sampleAt(qint64 timeMs, TargetObservation *out) const;

    // Highest-confidence observation (ties resolved toward the latest).
    bool representative(TargetObservation *out) const;
    // A ReframeTarget usable with the existing ReframePlanBuilder (empty id
    // when the track is empty).
    ReframeTarget representativeTarget() const;

private:
    QString m_id;
    QString m_label;
    bool m_active = true;
    int m_missCount = 0;
    QList<TargetObservation> m_observations;
};
