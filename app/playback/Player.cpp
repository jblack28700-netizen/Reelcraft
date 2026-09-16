#include "Player.h"

#include "playback/DefaultPacingPolicy.h"
#include "playback/SystemClock.h"

namespace {

// Bounded per-tick frame allowance: a replaceable pacing policy may report a
// large backlog after a stall, so the player catches up in bounded chunks and
// one tick can never present an unbounded number of frames.
constexpr int kMaxFramesPerTick = 240;

// Bounded read wait for a single frame: long enough for a persistent FFmpeg
// rawvideo stream to deliver the next frame, short enough not to hang a caller.
constexpr int kFrameReadTimeoutMs = 200;

} // namespace

Player::Player(FramePump *pump, QObject *parent)
    : QObject(parent)
    , m_pump(pump)
{
    initialize();
}

Player::Player(FramePump *pump, Clock *clock, PacingPolicy *pacing,
               QObject *parent)
    : QObject(parent)
    , m_pump(pump)
    , m_clock(clock)
    , m_pacing(pacing)
{
    initialize();
}

Player::~Player() = default;

void Player::initialize()
{
    if (!m_clock) {
        m_ownedClock = std::make_unique<SystemClock>();
        m_clock = m_ownedClock.get();
    }
    if (!m_pacing) {
        m_ownedPacing = std::make_unique<DefaultPacingPolicy>();
        m_pacing = m_ownedPacing.get();
    }
    if (m_pump) {
        connect(m_pump, &FramePump::frameReady, this, &Player::onPumpFrameReady);
        connect(m_pump, &FramePump::streamEnded, this, &Player::onPumpStreamEnded);
        connect(m_pump, &FramePump::streamFailed, this, &Player::onPumpStreamFailed);
    }
}

Player::State Player::state() const
{
    return m_state;
}

bool Player::isStopped() const
{
    return m_state == State::Stopped;
}

bool Player::isPlaying() const
{
    return m_state == State::Playing;
}

bool Player::isPaused() const
{
    return m_state == State::Paused;
}

qint64 Player::frameIntervalMs() const
{
    return m_frameIntervalMs;
}

bool Player::setFrameIntervalMs(qint64 intervalMs)
{
    if (intervalMs <= 0) {
        return false;
    }
    m_frameIntervalMs = intervalMs;
    return true;
}

qint64 Player::frameCount() const
{
    return m_playhead.frameCount();
}

qint64 Player::currentFrameIndex() const
{
    return m_playhead.currentFrameIndex();
}

qint64 Player::positionMs() const
{
    return m_playhead.positionMs(m_frameIntervalMs);
}

FramePump *Player::framePump() const
{
    return m_pump;
}

Clock *Player::clock() const
{
    return m_clock;
}

PacingPolicy *Player::pacingPolicy() const
{
    return m_pacing;
}

void Player::setPacingPolicy(PacingPolicy *policy)
{
    if (policy) {
        m_pacing = policy;
    }
}

void Player::play()
{
    if (m_state == State::Playing) {
        return;
    }
    if (!m_pump) {
        emit errorOccurred(QStringLiteral("Player has no frame pump."));
        return;
    }
    m_lastPacingMs = m_clock ? m_clock->nowMillis() : 0;
    setState(State::Playing);
}

void Player::pause()
{
    if (m_state != State::Playing) {
        return;
    }
    setState(State::Paused);
}

void Player::stop()
{
    const bool hadFrames = m_playhead.frameCount() > 0;
    m_playhead.reset();
    m_lastPacingMs = m_clock ? m_clock->nowMillis() : 0;
    setState(State::Stopped);
    if (hadFrames) {
        emit positionChanged(0, 0);
    }
}

int Player::tick()
{
    if (m_state != State::Playing || !m_pump || !m_clock || !m_pacing) {
        return 0;
    }

    const qint64 now = m_clock->nowMillis();
    qint64 elapsed = now - m_lastPacingMs;
    if (elapsed < 0) {
        elapsed = 0; // defensive: an injected clock moved backwards
    }

    int framesToAdvance =
        m_pacing->framesToAdvance(elapsed, m_frameIntervalMs);
    if (framesToAdvance <= 0) {
        return 0;
    }
    if (framesToAdvance > kMaxFramesPerTick) {
        framesToAdvance = kMaxFramesPerTick;
    }

    int presented = 0;
    for (int i = 0; i < framesToAdvance; ++i) {
        if (m_state != State::Playing) {
            break;
        }
        if (!advanceOneFrame()) {
            break;
        }
        ++presented;
    }

    if (presented > 0) {
        // Advance by exactly the frames actually presented so unfulfilled time
        // (timeout, end of stream) is retried rather than silently dropped.
        m_lastPacingMs += static_cast<qint64>(presented) * m_frameIntervalMs;
    }
    return presented;
}

bool Player::stepOnce()
{
    return advanceOneFrame();
}

bool Player::advanceOneFrame()
{
    if (!m_pump) {
        return false;
    }
    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    return m_pump->advance(kFrameReadTimeoutMs, &result);
}

void Player::setState(State state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    emit stateChanged(m_state);
}

void Player::onPumpFrameReady(const QImage &image)
{
    m_playhead.advance();
    const qint64 count = m_playhead.frameCount();
    const qint64 position = m_playhead.positionMs(m_frameIntervalMs);
    emit positionChanged(count, position);
    emit framePresented(image, count, position);
}

void Player::onPumpStreamEnded()
{
    setState(State::Stopped);
    emit playbackEnded();
}

void Player::onPumpStreamFailed(const QString &message)
{
    setState(State::Stopped);
    emit errorOccurred(message);
}
