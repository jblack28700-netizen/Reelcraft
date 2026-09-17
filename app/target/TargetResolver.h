#pragma once

#include <QImage>
#include <QList>
#include <QString>
#include <QStringList>

#include "reframe/ReframeFrameProvider.h"
#include "target/EquirectProjection.h"
#include "target/EquirectViewPlan.h"
#include "target/SphericalTargetTracker.h"
#include "target/TargetDetector.h"

// Configuration for the target resolver.
struct TargetResolveConfig
{
    // How the sphere is covered with detection views. Detecting in tangent
    // views avoids equirectangular pole/seam distortion.
    EquirectViewPlan::Config viewPlan;
    // Observations below this confidence are discarded regardless of the query.
    double minConfidence = 0.3;
    SphericalTargetTracker::Config tracker;
};

// TargetResolver is the orchestration boundary for 360 target resolution:
//
//   360 frame -> covering perspective views -> TargetDetector
//     -> detections mapped to spherical directions -> tracker -> TargetTrack(s)
//
// It never fabricates a target: when nothing matches the query it records an
// honest unresolved note and produces no observations. Detector and tracker are
// replaceable; the resolver itself only owns the deterministic geometry and
// orchestration.
class TargetResolver
{
public:
    TargetResolver() = default;
    explicit TargetResolver(TargetResolveConfig config);

    void reset();

    TargetResolveConfig config() const;
    void setConfig(const TargetResolveConfig &config);

    const SphericalTargetTracker &tracker() const;
    const QList<TargetTrack> &tracks() const;
    const QStringList &notes() const;

    // Resolved targets for the existing ReframePlanBuilder. Each returned
    // ReframeTarget carries the supplied reference as its id (so label-based
    // requests such as "look at the person" match), or the persistent track id
    // when reference is empty. Only tracks matching reference by id or label
    // (case-insensitive) are returned; nothing is fabricated.
    QList<ReframeTarget> resolvedTargets(const QString &reference) const;

    // Resolves one equirectangular frame. Returns false only on a real failure
    // (invalid frame, detector error); an empty result is success and is
    // reported in notes().
    bool resolveFrame(const QImage &equirect, qint64 timeMs,
                      const TargetQuery &query, TargetDetector *detector,
                      QList<TargetObservation> *outObservations = nullptr,
                      QString *error = nullptr);

    // Convenience over a replaceable frame provider. Timestamps are processed
    // in the given order; the tracker accumulates identity across frames.
    bool resolveSequence(ReframeFrameProvider *provider,
                         const QList<qint64> &timestamps,
                         const TargetQuery &query, TargetDetector *detector,
                         QList<TargetTrack> *outTracks = nullptr,
                         QString *error = nullptr);

private:
    TargetResolveConfig m_config;
    SphericalTargetTracker m_tracker;
    QStringList m_notes;
};
