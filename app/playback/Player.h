#pragma once

#include <QImage>
#include <QObject>
#include <QString>

#include <memory>

#include "media/FramePump.h"
#include "playback/Clock.h"
#include "playback/PacingPolicy.h"
#include "playback/Playhead.h"

// Player is the deterministic player/timing foundation (Phase 3 Objective 4).
//
// It owns playback state (Stopped/Playing/Paused), the playhead, the pacing
// policy, and the interaction with an injected FramePump. It has no timer,
// thread, UI, audio, duration metadata, or timeline; callers drive advancement
// through tick() (clock/pacing) or stepOnce() (an explicit single frame).
//
// Clock and PacingPolicy are injected so tests can verify behavior
// deterministically without wall-clock delays. The FramePump stays a passive,
// caller-driven decoder; the Player never turns it into a timer or clock.
class Player : public QObject
{
    Q_OBJECT

public:
    enum class State {
        Stopped,
        Playing,
        Paused
    };
    Q_ENUM(State)

    // Owns a SystemClock and a DefaultPacingPolicy.
    explicit Player(FramePump *pump, QObject *parent = nullptr);

    // Injects a Clock and/or PacingPolicy (non-owning; they must outlive the
    // player). A null argument falls back to an owned default.
    Player(FramePump *pump, Clock *clock, PacingPolicy *pacing,
           QObject *parent = nullptr);
    ~Player() override;

    State state() const;
    bool isStopped() const;
    bool isPlaying() const;
    bool isPaused() const;

    // Playback frame interval used for pacing and playhead position. This is a
    // playback pacing parameter, NOT media duration/frame-rate metadata.
    qint64 frameIntervalMs() const;
    // Sets the frame interval. Returns false (no change) for a non-positive
    // interval.
    bool setFrameIntervalMs(qint64 intervalMs);

    qint64 frameCount() const;
    qint64 currentFrameIndex() const;
    qint64 positionMs() const;

    FramePump *framePump() const;
    Clock *clock() const;
    PacingPolicy *pacingPolicy() const;
    // Replaces the pacing policy (non-owning; null is ignored).
    void setPacingPolicy(PacingPolicy *policy);

public slots:
    void play();
    void pause();
    void stop();

    // Presents frames according to the injected clock and pacing policy.
    // Returns the number of frames presented. No-op unless Playing.
    int tick();

    // Presents exactly one frame, independent of clock and pacing. Returns true
    // when a frame was presented. Does not change play/pause state (end-of-
    // stream and error still move the player to Stopped).
    bool stepOnce();

signals:
    void stateChanged(Player::State state);
    void positionChanged(qint64 frameCount, qint64 positionMs);
    void framePresented(const QImage &image, qint64 frameCount, qint64 positionMs);
    void playbackEnded();
    void errorOccurred(const QString &message);

private:
    void initialize();
    void setState(State state);
    bool advanceOneFrame();
    void onPumpFrameReady(const QImage &image);
    void onPumpStreamEnded();
    void onPumpStreamFailed(const QString &message);

    FramePump *m_pump = nullptr;
    Clock *m_clock = nullptr;
    PacingPolicy *m_pacing = nullptr;
    std::unique_ptr<Clock> m_ownedClock;
    std::unique_ptr<PacingPolicy> m_ownedPacing;
    Playhead m_playhead;
    State m_state = State::Stopped;
    qint64 m_frameIntervalMs = 40; // 25 fps playback pacing default
    qint64 m_lastPacingMs = 0;
};
