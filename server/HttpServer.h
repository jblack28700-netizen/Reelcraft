#pragma once

#include <QHttpServer>
#include <QObject>
#include <QString>

#include "core/MediaItem.h"

class JobRegistry;

// The Slice 1 HTTP surface (Decision 059).
//
// This is a transport/control boundary and nothing else. It never plans,
// resolves, renders, reviews or decides: it validates a request, records
// bookkeeping in the job registry, and asks the worker thread to run an
// existing Application operation. It runs on the main thread and never touches
// Application directly.
class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(JobRegistry *registry, QObject *parent = nullptr);

    bool listen(const QString &host, quint16 port);
    quint16 port() const { return m_port; }
    bool isReady() const { return m_ready; }
    QString startupError() const { return m_startupError; }

signals:
    // Consumed by Worker slots through queued connections (main thread ->
    // worker thread).
    void prepareRequested(const QString &jobId, const QString &mediaId,
                          const QString &instruction);
    void acceptRequested(const QString &jobId);
    void rejectRequested(const QString &jobId);

public slots:
    void onWorkerInitialized(bool ok, const QString &error,
                             const QList<MediaItem> &items);
    void onMediaListChanged(const QList<MediaItem> &items);

private:
    void registerRoutes();

    JobRegistry *m_registry = nullptr;
    QHttpServer m_server;
    QList<MediaItem> m_media;
    bool m_ready = false;
    QString m_startupError;
    quint16 m_port = 0;
};
