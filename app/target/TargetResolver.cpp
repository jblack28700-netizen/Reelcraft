#include "target/TargetResolver.h"

#include "viewer/EquirectView.h"

#include <QtGlobal>

TargetResolver::TargetResolver(TargetResolveConfig config)
    : m_config(std::move(config)), m_tracker(m_config.tracker)
{
}

void TargetResolver::reset()
{
    m_tracker.reset();
    m_notes.clear();
}

TargetResolveConfig TargetResolver::config() const
{
    return m_config;
}

void TargetResolver::setConfig(const TargetResolveConfig &config)
{
    m_config = config;
    m_tracker.setConfig(config.tracker);
}

const SphericalTargetTracker &TargetResolver::tracker() const
{
    return m_tracker;
}

const QList<TargetTrack> &TargetResolver::tracks() const
{
    return m_tracker.tracks();
}

const QStringList &TargetResolver::notes() const
{
    return m_notes;
}

QList<ReframeTarget> TargetResolver::resolvedTargets(const QString &reference) const
{
    QList<ReframeTarget> targets;
    for (const TargetTrack &track : m_tracker.activeTracks()) {
        const bool match = reference.isEmpty()
            || track.id().compare(reference, Qt::CaseInsensitive) == 0
            || track.label().compare(reference, Qt::CaseInsensitive) == 0;
        if (!match) {
            continue;
        }
        TargetObservation observation;
        if (!track.representative(&observation)) {
            continue;
        }
        ReframeTarget target = observation.toReframeTarget();
        target.id = reference.isEmpty() ? track.id() : reference;
        targets.append(target);
    }
    return targets;
}

bool TargetResolver::resolveFrame(const QImage &equirect, qint64 timeMs,
                                  const TargetQuery &query, TargetDetector *detector,
                                  QList<TargetObservation> *outObservations,
                                  QString *error)
{
    if (error) {
        error->clear();
    }
    if (outObservations) {
        outObservations->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (equirect.isNull() || equirect.width() <= 0 || equirect.height() <= 0) {
        return fail(QStringLiteral("Target resolution requires a valid frame."));
    }
    if (!detector) {
        return fail(QStringLiteral("Target resolution requires a detector."));
    }

    const double minConfidence =
        qMax(query.minConfidence, m_config.minConfidence);

    const QList<PerspectiveView> views =
        EquirectViewPlan::coveringViews(m_config.viewPlan);

    QList<TargetObservation> observations;
    for (const PerspectiveView &view : views) {
        QImage viewImage;
        if (!EquirectView::render(equirect, view.yawDeg, view.pitchDeg, 0.0,
                                  view.fieldOfViewDeg, view.width, view.height,
                                  &viewImage)) {
            return fail(QStringLiteral("Could not render a detection view."));
        }

        QList<TargetDetection> detections;
        QString detectorError;
        if (!detector->detect(viewImage, query, &detections, &detectorError)) {
            return fail(QStringLiteral("Detector failed: %1")
                            .arg(detectorError.isEmpty()
                                     ? QStringLiteral("unknown error")
                                     : detectorError));
        }

        for (const TargetDetection &detection : detections) {
            if (!detection.isValid() || detection.confidence < minConfidence) {
                continue;
            }
            if (!query.label.isEmpty() && !detection.label.isEmpty()
                && detection.label != query.label) {
                continue;
            }
            SphericalDirection center;
            double yawRadius = 0.0;
            double pitchRadius = 0.0;
            if (!EquirectProjection::detectionToDirection(
                    view, detection.boundingBox, &center, &yawRadius, &pitchRadius)) {
                continue;
            }
            TargetObservation observation;
            observation.timeMs = timeMs;
            observation.targetId = detection.targetId;
            observation.label = detection.label;
            observation.confidence = detection.confidence;
            observation.yawDeg = center.yawDeg;
            observation.pitchDeg = center.pitchDeg;
            observation.yawRadiusDeg = yawRadius;
            observation.pitchRadiusDeg = pitchRadius;
            observation.source =
                QStringLiteral("%1@yaw%2,pitch%3")
                    .arg(detector->name())
                    .arg(view.yawDeg, 0, 'f', 1)
                    .arg(view.pitchDeg, 0, 'f', 1);
            observations.append(observation);
        }
    }

    const QList<TargetObservation> assigned = m_tracker.update(observations, timeMs);
    if (outObservations) {
        *outObservations = assigned;
    }

    if (assigned.isEmpty()) {
        m_notes.append(QStringLiteral(
            "Unresolved: no target matching '%1' at %2 ms.")
                           .arg(query.label.isEmpty() ? QStringLiteral("any")
                                                      : query.label)
                           .arg(timeMs));
    }
    return true;
}

bool TargetResolver::resolveSequence(ReframeFrameProvider *provider,
                                     const QList<qint64> &timestamps,
                                     const TargetQuery &query,
                                     TargetDetector *detector,
                                     QList<TargetTrack> *outTracks, QString *error)
{
    if (error) {
        error->clear();
    }
    if (!provider) {
        if (error) {
            *error = QStringLiteral("Target resolution requires a frame provider.");
        }
        return false;
    }

    for (qint64 timeMs : timestamps) {
        QImage frame;
        QString providerError;
        if (!provider->frameAt(timeMs, &frame, &providerError)) {
            // A single undecodable sample (for example the exact end of a clip,
            // or one corrupt frame) must not abort the whole sequence: record it
            // and continue. If no sample resolves, the result is honestly empty.
            m_notes.append(QStringLiteral("Frame at %1 ms could not be decoded: %2")
                               .arg(timeMs)
                               .arg(providerError.isEmpty()
                                        ? QStringLiteral("unknown error")
                                        : providerError));
            continue;
        }
        if (!resolveFrame(frame, timeMs, query, detector, nullptr, error)) {
            return false;
        }
    }

    if (outTracks) {
        *outTracks = m_tracker.tracks();
    }
    return true;
}
