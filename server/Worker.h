#pragma once

#include <QObject>
#include <QString>
#include <QThread>

#include <atomic>
#include <memory>

#include "application/Application.h"

class JobRegistry;
class TargetDetector;

// The Application worker (Decision 059 addendum).
//
// A dedicated QThread owns the single Application instance. Every call into
// Application happens on that thread; the HTTP layer never touches Application
// directly. The two threads communicate only through queued signal/slot
// connections and the mutex-guarded JobRegistry.
//
// Configuration (media path, detector) is startup configuration only. Nothing
// here is ever reconfigured in response to a browser request.
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(JobRegistry *registry, QObject *parent = nullptr);
    ~Worker() override;

    // Main thread, before start().
    void setMediaPath(const QString &mediaPath);
    void setDetector(const QString &python, const QString &script, const QString &model);

    void start();
    void stop();
    bool isReady() const { return m_ready.load(); }

public slots:
    // All of these run on the worker thread, reached only by queued invocation.
    void initialize();
    void shutdown();
    void prepare(const QString &jobId, const QString &mediaId, const QString &instruction);
    void accept(const QString &jobId);
    void reject(const QString &jobId);

signals:
    // Emitted from the worker thread; delivered to the main thread by queued
    // connections.
    void initialized(bool ok, const QString &error, const QList<MediaItem> &items);
    void mediaListChanged(const QList<MediaItem> &items);
    void reviewReady(const QString &jobId, const ReframePlanReview &review);
    void renderFinished(const QString &jobId, const ReframeCommandOutcome &outcome);
    void jobFailed(const QString &jobId, const QString &error);
    void jobRejected(const QString &jobId);
    void statusMessage(const QString &message);

private:
    JobRegistry *m_registry = nullptr;
    QThread *m_thread = nullptr;
    QString m_mediaPath;
    QString m_detectorPython;
    QString m_detectorScript;
    QString m_detectorModel;
    std::atomic<bool> m_ready{ false };

    // Worker-thread only.
    Application *m_application = nullptr;
    std::unique_ptr<TargetDetector> m_detector;
};
