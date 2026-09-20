#pragma once

#include <QHash>
#include <QMutex>
#include <QString>

#include "application/ReframeCommandOutcome.h"
#include "application/ReframePlanReview.h"

// Slice 1 job bookkeeping (Decision 059 and its addendum).
//
// This is the ONLY state shared between the HTTP thread and the Application
// worker thread, and every access is mutex-guarded. It is bookkeeping and
// nothing else: it never plans, resolves, renders, reviews or decides. Every
// fact it holds was produced by Application and handed to it, so it introduces
// no second domain model.
enum class JobState {
    Queued,
    Preparing,
    AwaitingReview,
    Rendering,
    Done,
    Failed,
    Rejected
};

QString jobStateToString(JobState state);

struct JobRecord {
    QString id;
    JobState state = JobState::Queued;
    QString instruction;
    QString mediaId;
    bool hasReview = false;
    ReframePlanReview review;
    bool hasResult = false;
    ReframeCommandOutcome result;
    QString error;
    QString outputPath;
    QString outputId;
};

class JobRegistry
{
public:
    QString createJob(const QString &mediaId, const QString &instruction);
    bool get(const QString &jobId, JobRecord *out) const;

    void setState(const QString &jobId, JobState state);
    void setReview(const QString &jobId, const ReframePlanReview &review);
    void setResult(const QString &jobId, const ReframeCommandOutcome &outcome);
    void setError(const QString &jobId, const QString &error);

    // Output identity is the SHA-256 of the record's outputPath. Decision 057
    // makes that unique: no render ever writes to a path a held record owns, and
    // derived destinations are fresh. Resolving an output id is the only way the
    // server maps a URL to a file, and it never concatenates the URL into a path.
    bool resolveOutput(const QString &outputId, QString *path, QString *jobId) const;
    static QString outputIdForPath(const QString &path);

private:
    mutable QMutex m_mutex;
    QHash<QString, JobRecord> m_jobs;
    QHash<QString, QString> m_outputToJob;
    int m_sequence = 0;
};
