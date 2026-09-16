#pragma once

#include <QImage>
#include <QObject>
#include <QString>

#include "FrameSource.h"

// FramePump is a small, synchronous consumer of a FrameSource (Phase 3
// Objective 3). One explicit advance() operation requests exactly one frame and
// emits the corresponding signal. It has no timer, no playback clock, no
// pacing/rate policy, and no worker thread; callers drive it.
class FramePump : public QObject
{
    Q_OBJECT

public:
    explicit FramePump(QObject *parent = nullptr);

    // The source must be opened by the caller before use.
    void setSource(FrameSource *source);
    FrameSource *source() const { return m_source; }

    // Requests one frame with a bounded wait. Returns true and emits
    // frameReady() on Ok; emits streamEnded() on EndOfStream; emits
    // streamFailed() on Error; returns false without a signal on Timeout.
    bool advance(int timeoutMs, FrameSource::ReadResult *result = nullptr);

signals:
    void frameReady(const QImage &image);
    void streamEnded();
    void streamFailed(const QString &message);

private:
    FrameSource *m_source = nullptr;
};
