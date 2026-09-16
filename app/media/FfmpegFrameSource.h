#pragma once

#include <QProcess>

#include "FrameSource.h"

// FfmpegFrameSource is a FrameSource implementation that streams decoded
// frames from a persistent FFmpeg subprocess (Phase 3 Objective 3). It
// implements the persistent-subprocess feasibility path proven in Objective 2
// with bounded waits and guaranteed process cleanup.
//
// Rawvideo carries no frame-dimension metadata and ffprobe geometry discovery
// is deferred, so the caller MUST supply frame geometry at open time. There is
// deliberately no production default geometry and no Objective-2 fixture
// constant in this file. The transport specifics (rawvideo/rgb24 and geometry)
// are configuration for this implementation only and are NOT product
// decisions.
class FfmpegFrameSource : public FrameSource
{
public:
    FfmpegFrameSource();
    ~FfmpegFrameSource() override;

    // Opens filePath for frame streaming. frameWidth/frameHeight are required
    // (rawvideo). Returns false with a deterministic error when geometry is
    // missing, the file is invalid, or the FFmpeg process cannot start.
    bool open(const QString &filePath, int frameWidth, int frameHeight);

    bool readNextFrame(int timeoutMs, ReadResult *result,
                       QImage *outFrame) override;
    void close() override;
    bool isOpen() const override;
    QString errorString() const override;

private:
    void setError(const QString &message);
    void stopProcess();

    QProcess m_process;
    bool m_open = false;
    int m_frameWidth = 0;
    int m_frameHeight = 0;
    QByteArray m_pendingBytes;
    bool m_atEnd = false;
    QString m_error;
};
