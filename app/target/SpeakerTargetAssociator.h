#pragma once

#include <QList>
#include <QPair>
#include <QString>

#include "target/SpeakerTypes.h"
#include "target/TargetTypes.h"

// SpeakerTargetAssociator maps a provider-local speakerId to an existing target
// track id. It never creates a new identity and never selects a target that is
// not visible.
//
// Deterministic association order for one interval:
//   1. explicit creator/structured binding (speakerId -> targetId), if that
//      target is visible in the interval;
//   2. spatial: if the interval carries an azimuth (direction of arrival),
//      the visible track whose yaw is nearest within the gate (unique only);
//   3. single visible person: exactly one visible person track;
//   4. otherwise unassociated (or ambiguous when several are plausible).
class SpeakerTargetAssociator
{
public:
    struct Config
    {
        double spatialGateDeg = 35.0;
        QString label = QStringLiteral("person");
    };

    void bind(const QString &speakerId, const QString &targetId);
    void clearBindings();
    bool isBound(const QString &speakerId) const;
    QString boundTarget(const QString &speakerId) const;
    QList<QPair<QString, QString>> bindings() const;

    // Associates one interval against the given visible tracks. On success
    // *outTargetId is non-empty and *outMethod describes the rule used.
    // *outAmbiguous is set when more than one visible target is plausible and
    // no explicit binding resolves it.
    bool associate(const SpeakerInterval &interval,
                   const QList<TargetTrack> &visibleTracks,
                   const Config &config, QString *outTargetId, bool *outAmbiguous,
                   QString *outMethod, QString *error = nullptr) const;

private:
    QList<QPair<QString, QString>> m_bindings;
};
