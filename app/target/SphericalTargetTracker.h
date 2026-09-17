#pragma once

#include <QList>
#include <QString>

#include "target/TargetTypes.h"

// SphericalTargetTracker maintains target identity across frames with a small,
// deterministic, replaceable algorithm: spherical non-maximum suppression
// within a frame, then greedy nearest-neighbour association on great-circle
// angular distance with a gate. It has no motion model and no appearance model;
// a stronger tracker (motion prediction, re-identification, face embeddings)
// can replace it behind the same class later.
//
// The algorithm is fully deterministic: association candidates are ordered by
// (distance, confidence, detection index, track index), identities are assigned
// from a deterministic counter, and identical input always yields identical
// tracks and ids.
class SphericalTargetTracker
{
public:
    struct Config
    {
        // Maximum great-circle distance for a detection to associate with an
        // existing track.
        double maxAssociationDistanceDeg = 25.0;
        // Detections closer than this (same label) are merged within a frame.
        double mergeDistanceDeg = 8.0;
        // A track is deactivated after this many consecutive frames without an
        // association. Its history is retained.
        int maxMisses = 8;
        // Detections below this confidence are ignored.
        double minConfidence = 0.3;
        // Motion hardening (deterministic; still no appearance model). When
        // enabled, association uses a constant-velocity prediction from the
        // track's last two observations, which keeps identity through crossing
        // trajectories. Prediction is never extrapolated beyond
        // maxPredictionMs.
        bool useVelocityPrediction = true;
        qint64 maxPredictionMs = 1500;
        // A track that missed at least one frame may still associate within
        // this wider gate, for up to reentryWindowMs after its last
        // observation. This supports temporary loss/occlusion/re-entry without
        // an appearance model.
        double reentryGateDeg = 60.0;
        qint64 reentryWindowMs = 3000;
    };

    SphericalTargetTracker() = default;
    explicit SphericalTargetTracker(Config config);

    void reset();

    Config config() const;
    void setConfig(const Config &config);

    // Deterministic within-frame duplicate suppression. When observations
    // overlap, the more confident one wins, then the one with the larger angular
    // footprint (a target clipped at a view edge produces a smaller footprint
    // than the same target fully visible in an overlapping view). Input order
    // does not affect the result.
    static QList<TargetObservation> mergeNearDuplicates(
        const QList<TargetObservation> &observations, double mergeDistanceDeg);

    // Associates one frame's observations with existing tracks and returns the
    // accepted observations carrying their persistent track ids, in a
    // deterministic order. Unmatched detections start new tracks; unmatched
    // tracks accrue misses.
    QList<TargetObservation> update(const QList<TargetObservation> &observations,
                                    qint64 timeMs);

    const QList<TargetTrack> &tracks() const;
    QList<TargetTrack> activeTracks() const;
    const TargetTrack *trackById(const QString &id) const;

private:
    Config m_config;
    QList<TargetTrack> m_tracks;
    int m_nextNumber = 1;
};
