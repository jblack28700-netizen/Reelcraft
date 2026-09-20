#include "Worker.h"

#include "JobRegistry.h"
#include "target/ProcessTargetDetector.h"

Worker::Worker(JobRegistry *registry, QObject *parent)
    : QObject(parent), m_registry(registry)
{
}

Worker::~Worker()
{
    stop();
}

void Worker::setMediaPath(const QString &mediaPath)
{
    m_mediaPath = mediaPath;
}

void Worker::setDetector(const QString &python, const QString &script,
                         const QString &model)
{
    m_detectorPython = python;
    m_detectorScript = script;
    m_detectorModel = model;
}

void Worker::start()
{
    if (m_thread) {
        return;
    }
    // No parent: the QThread object stays owned by the thread that manages it.
    m_thread = new QThread();
    m_thread->setObjectName(QStringLiteral("reelcraft-application"));
    // From here on this object lives on the worker thread, so Application is
    // constructed, used and destroyed there and nowhere else.
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &Worker::initialize);
    m_thread->start();
}

void Worker::stop()
{
    if (!m_thread) {
        return;
    }
    if (m_thread->isRunning()) {
        QMetaObject::invokeMethod(this, "shutdown", Qt::BlockingQueuedConnection);
        m_thread->quit();
        m_thread->wait(10000);
    }
    delete m_thread;
    m_thread = nullptr;
}

void Worker::initialize()
{
    m_application = new Application();
    m_application->newProject();

    if (!m_mediaPath.isEmpty()) {
        if (!m_application->importMediaFile(m_mediaPath)) {
            emit initialized(false,
                             QStringLiteral("Could not import media: %1")
                                 .arg(m_mediaPath),
                             {});
            return;
        }
    }

    const QList<MediaItem> items = m_application->mediaItems();
    if (!items.isEmpty()) {
        m_application->setActiveMedia(items.first().id());
    }

    // Startup-only capability wiring, mirroring app/main.cpp:207-235 exactly.
    // Never reconfigured in response to a request.
    if (!m_detectorPython.isEmpty() && !m_detectorScript.isEmpty()
        && !m_detectorModel.isEmpty()) {
        m_detector = std::make_unique<ProcessTargetDetector>(
            m_detectorPython,
            QStringList{ m_detectorScript, QStringLiteral("--model"),
                         m_detectorModel });
        m_application->setTargetDetector(m_detector.get());
    }

    // Observation only. Job state transitions are made from the return values of
    // the calls below, so a signal can never be mistaken for a terminal state.
    connect(m_application, &Application::mediaListChanged, this,
            [this](const QList<MediaItem> &list) { emit mediaListChanged(list); });
    connect(m_application, &Application::reframeOutputsChanged, this,
            [this](const QList<ReframeCommandOutcome> &records) {
                emit statusMessage(QStringLiteral("render records held: %1")
                                       .arg(records.size()));
            });
    connect(m_application, &Application::reframeCommandFinished, this,
            [this](const ReframeCommandOutcome &outcome) {
                emit statusMessage(
                    outcome.ok
                        ? QStringLiteral("command finished: %1")
                              .arg(outcome.outputPath)
                        : QStringLiteral("command failed: %1").arg(outcome.error));
            });

    m_ready.store(true);
    emit initialized(true, QString(), items);
}

void Worker::shutdown()
{
    m_ready.store(false);
    delete m_application;
    m_application = nullptr;
    m_detector.reset();
}

void Worker::prepare(const QString &jobId, const QString &mediaId,
                     const QString &instruction)
{
    Q_UNUSED(mediaId);
    if (!m_application) {
        m_registry->setError(jobId, QStringLiteral("The backend is not ready."));
        m_registry->setState(jobId, JobState::Failed);
        emit jobFailed(jobId, QStringLiteral("The backend is not ready."));
        return;
    }

    m_registry->setState(jobId, JobState::Preparing);
    emit statusMessage(QStringLiteral("preparing: %1").arg(instruction));

    // Decision stage only, through the existing shared request builder. This
    // call blocks the worker thread for as long as perception takes; that is
    // exactly why it does not run on the HTTP thread.
    const bool ok = m_application->prepareReframeCommand(instruction, 0, 0);

    if (ok && m_application->hasPendingReview()) {
        const ReframePlanReview review = m_application->pendingReview();
        m_registry->setReview(jobId, review);
        m_registry->setState(jobId, JobState::AwaitingReview);
        emit reviewReady(jobId, review);
        return;
    }

    QString error = m_application->lastReframeCommandOutcome().error;
    if (error.isEmpty()) {
        error = QStringLiteral("The plan could not be prepared.");
    }
    m_registry->setError(jobId, error);
    m_registry->setState(jobId, JobState::Failed);
    emit jobFailed(jobId, error);
}

void Worker::accept(const QString &jobId)
{
    if (!m_application) {
        m_registry->setError(jobId, QStringLiteral("The backend is not ready."));
        m_registry->setState(jobId, JobState::Failed);
        emit jobFailed(jobId, QStringLiteral("The backend is not ready."));
        return;
    }

    m_registry->setState(jobId, JobState::Rendering);
    emit statusMessage(QStringLiteral("rendering job %1").arg(jobId));

    // Renders EXACTLY the reviewed plan through the existing deterministic render
    // seam, appends through the single record gate, and derives a fresh
    // destination at commit time. The server adds nothing to this.
    const ReframeReviewResult result = m_application->acceptReframeReview();
    const ReframeCommandOutcome outcome = m_application->lastReframeCommandOutcome();

    if (result.ok) {
        m_registry->setResult(jobId, outcome);
        m_registry->setState(jobId, JobState::Done);
        emit renderFinished(jobId, outcome);
        return;
    }

    QString error = result.error;
    if (error.isEmpty()) {
        error = outcome.error.isEmpty()
            ? QStringLiteral("The render failed.")
            : outcome.error;
    }
    m_registry->setError(jobId, error);
    m_registry->setState(jobId, JobState::Failed);
    emit jobFailed(jobId, error);
}

void Worker::reject(const QString &jobId)
{
    if (!m_application) {
        m_registry->setError(jobId, QStringLiteral("The backend is not ready."));
        m_registry->setState(jobId, JobState::Failed);
        emit jobFailed(jobId, QStringLiteral("The backend is not ready."));
        return;
    }
    m_application->rejectReframeReview();
    m_registry->setState(jobId, JobState::Rejected);
    emit jobRejected(jobId);
}
