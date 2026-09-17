#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

// 360 Reframing Objective 6 — audio/speaker evidence (structured, inspectable).
//
// Speaker evidence is OPTIONAL evidence about which visible tracked person is
// speaking. It never mutates identity resolution and never overrides an explicit
// creator selection:
//   - an analysis is a set of speech intervals with an opaque provider-local
//     speakerId (VAD-only providers may report a single id);
//   - an association maps a speakerId to an existing target track id;
//   - a verdict says how the evidence relates to a track:
//     Unavailable / Active / Ambiguous / Overlap / Silence / Unassociated.
//
// The representation is provider-agnostic and JSON-serializable: a local CPU
// VAD, a GPU diarization service, or a future audio-visual model can all feed
// it.

struct SpeakerInterval
{
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString speakerId;      // provider-local id (e.g. "spk1")
    double confidence = 0.0;
    bool overlap = false;   // simultaneous/overlapping speech
    // Optional direction of arrival (degrees) for spatial audio-visual
    // association; NaN/absent when the provider does not supply it.
    double azimuthDeg = 0.0;
    bool hasAzimuth = false;
    // Optional provider attribution hint: a specific target track id supplied
    // by an audio-visual active-speaker provider. It is only honoured when that
    // target is visible; it never invents a target and never overrides an
    // explicit creator binding.
    QString targetIdHint;

    bool isValid(QString *error = nullptr) const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   SpeakerInterval *out,
                                   QString *error = nullptr);
};

struct SpeakerAnalysis
{
    bool available = false;
    QString provider;
    qint64 startMs = 0;
    qint64 endMs = 0;
    QList<SpeakerInterval> intervals;
    QString error;

    bool isValid(QString *error = nullptr) const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   SpeakerAnalysis *out,
                                   QString *error = nullptr);
};

enum class SpeakerVerdict
{
    Unavailable,  // no provider / analysis failed
    Active,       // one likely active speaker associated with this target
    Ambiguous,    // multiple plausible speaker candidates
    Overlap,      // simultaneous speech (cannot force a single identity)
    Silence,      // no speech
    Unassociated  // speech exists but maps to no visible target
};

QString speakerVerdictToString(SpeakerVerdict verdict);
SpeakerVerdict speakerVerdictFromString(const QString &value);

// One piece of speaker evidence about a target track.
struct SpeakerEvidence
{
    QString targetId;
    QString speakerId;
    SpeakerVerdict verdict = SpeakerVerdict::Unavailable;
    double confidence = 0.0;
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString provider;
    QString detail;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   SpeakerEvidence *out,
                                   QString *error = nullptr);
};

// A deterministic timeline segment after hysteresis.
struct SpeakerSegment
{
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString speakerId;   // empty for silence
    QString targetId;    // associated target, empty when unassociated
    SpeakerVerdict verdict = SpeakerVerdict::Silence;
    double confidence = 0.0;
    // Optional provider attribution hint carried from the originating interval
    // so association can honour a direct audio-visual attribution.
    QString targetIdHint;

    bool isValid(QString *error = nullptr) const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   SpeakerSegment *out,
                                   QString *error = nullptr);
};
