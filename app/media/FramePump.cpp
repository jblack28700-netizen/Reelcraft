#include "FramePump.h"

FramePump::FramePump(QObject *parent)
    : QObject(parent)
{
}

void FramePump::setSource(FrameSource *source)
{
    m_source = source;
}

bool FramePump::advance(int timeoutMs, FrameSource::ReadResult *result)
{
    if (result) {
        *result = FrameSource::ReadResult::Error;
    }
    if (!m_source) {
        if (result) {
            *result = FrameSource::ReadResult::Error;
        }
        emit streamFailed(QStringLiteral("Frame pump has no source."));
        return false;
    }

    QImage frame;
    FrameSource::ReadResult readResult = FrameSource::ReadResult::Error;
    const bool ok = m_source->readNextFrame(timeoutMs, &readResult, &frame);
    if (result) {
        *result = readResult;
    }

    switch (readResult) {
    case FrameSource::ReadResult::Ok:
        emit frameReady(frame);
        return ok;
    case FrameSource::ReadResult::EndOfStream:
        emit streamEnded();
        return false;
    case FrameSource::ReadResult::Error:
        emit streamFailed(m_source->errorString());
        return false;
    case FrameSource::ReadResult::Timeout:
    default:
        return false;
    }
}
