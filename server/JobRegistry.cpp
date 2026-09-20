#include "JobRegistry.h"

#include <QCryptographicHash>
#include <QUuid>

QString jobStateToString(JobState state)
{
    switch (state) {
    case JobState::Queued:
        return QStringLiteral("queued");
    case JobState::Preparing:
        return QStringLiteral("preparing");
    case JobState::AwaitingReview:
        return QStringLiteral("awaiting_review");
    case JobState::Rendering:
        return QStringLiteral("rendering");
    case JobState::Done:
        return QStringLiteral("done");
    case JobState::Failed:
        return QStringLiteral("failed");
    case JobState::Rejected:
        return QStringLiteral("rejected");
    }
    return QStringLiteral("failed");
}

QString JobRegistry::outputIdForPath(const QString &path)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString JobRegistry::createJob(const QString &mediaId, const QString &instruction)
{
    QMutexLocker locker(&m_mutex);
    JobRecord record;
    // Opaque, server-generated, never derived from anything the browser sent.
    record.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    record.state = JobState::Queued;
    record.mediaId = mediaId;
    record.instruction = instruction;
    m_jobs.insert(record.id, record);
    return record.id;
}

bool JobRegistry::get(const QString &jobId, JobRecord *out) const
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_jobs.constFind(jobId);
    if (it == m_jobs.constEnd()) {
        return false;
    }
    if (out) {
        *out = it.value();
    }
    return true;
}

void JobRegistry::setState(const QString &jobId, JobState state)
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_jobs.find(jobId);
    if (it != m_jobs.end()) {
        it->state = state;
    }
}

void JobRegistry::setReview(const QString &jobId, const ReframePlanReview &review)
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_jobs.find(jobId);
    if (it != m_jobs.end()) {
        it->review = review;
        it->hasReview = review.isValid();
    }
}

void JobRegistry::setResult(const QString &jobId, const ReframeCommandOutcome &outcome)
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_jobs.find(jobId);
    if (it == m_jobs.end()) {
        return;
    }
    it->result = outcome;
    it->hasResult = true;
    it->outputPath = outcome.outputPath;
    if (outcome.outputPath.isEmpty()) {
        it->outputId.clear();
        return;
    }
    it->outputId = outputIdForPath(outcome.outputPath);
    m_outputToJob.insert(it->outputId, jobId);
}

void JobRegistry::setError(const QString &jobId, const QString &error)
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_jobs.find(jobId);
    if (it != m_jobs.end()) {
        it->error = error;
    }
}

bool JobRegistry::resolveOutput(const QString &outputId, QString *path, QString *jobId) const
{
    QMutexLocker locker(&m_mutex);
    const auto it = m_outputToJob.constFind(outputId);
    if (it == m_outputToJob.constEnd()) {
        return false;
    }
    const auto job = m_jobs.constFind(it.value());
    if (job == m_jobs.constEnd() || job->outputPath.isEmpty()) {
        return false;
    }
    if (path) {
        *path = job->outputPath;
    }
    if (jobId) {
        *jobId = job->id;
    }
    return true;
}
