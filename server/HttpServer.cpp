#include "HttpServer.h"

#include <QFile>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QHttpServerResponder>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "JobRegistry.h"

namespace {

constexpr QHttpServerResponder::StatusCode kOk =
    QHttpServerResponder::StatusCode::Ok;
constexpr QHttpServerResponder::StatusCode kBadRequest =
    QHttpServerResponder::StatusCode::BadRequest;
constexpr QHttpServerResponder::StatusCode kNotFound =
    QHttpServerResponder::StatusCode::NotFound;
constexpr QHttpServerResponder::StatusCode kConflict =
    QHttpServerResponder::StatusCode::Conflict;
constexpr QHttpServerResponder::StatusCode kUnavailable =
    QHttpServerResponder::StatusCode::ServiceUnavailable;

QHttpServerResponse jsonResponse(const QJsonObject &object,
                                 QHttpServerResponder::StatusCode status = kOk)
{
    return QHttpServerResponse(QByteArrayLiteral("application/json"),
                               QJsonDocument(object).toJson(QJsonDocument::Compact),
                               status);
}

QHttpServerResponse errorResponse(const QString &message,
                                  QHttpServerResponder::StatusCode status)
{
    QJsonObject object;
    object.insert(QStringLiteral("error"), message);
    return jsonResponse(object, status);
}

// The review as the creator sees it. Every field is a value ReframePlanReview
// already carries; nothing is derived, renamed or invented here.
QJsonObject reviewToJson(const ReframePlanReview &review)
{
    QJsonObject object;
    QJsonArray lines;
    for (const QString &line : review.summaryLines()) {
        lines.append(line);
    }
    object.insert(QStringLiteral("summaryLines"), lines);
    object.insert(QStringLiteral("framing"), review.framing);
    object.insert(QStringLiteral("cameraMovement"), review.cameraMovement);
    object.insert(QStringLiteral("lens"), review.lens);
    object.insert(QStringLiteral("timeRange"), review.timeRange);
    object.insert(QStringLiteral("output"), review.output);
    object.insert(QStringLiteral("audio"), review.audio);
    object.insert(QStringLiteral("planDigest"), review.planDigest);
    object.insert(QStringLiteral("keyframeCount"), review.keyframeCount);
    object.insert(QStringLiteral("instruction"), review.instruction);
    object.insert(QStringLiteral("understanding"), review.understanding);
    return object;
}

// The rendered picture's length, read from the record the engine already wrote:
// the retained source spans when the plan has them (a temporal edit), otherwise
// the record's single range. Nothing is probed or invented here.
qint64 recordDurationMs(const ReframeCommandOutcome &outcome)
{
    if (!outcome.temporalSegments.isEmpty()) {
        qint64 total = 0;
        for (const QPair<qint64, qint64> &segment : outcome.temporalSegments) {
            total += segment.second - segment.first;
        }
        return total;
    }
    return outcome.endMs > outcome.startMs ? outcome.endMs - outcome.startMs : 0;
}

// The render result, projected from ReframeCommandOutcome. downloadUrl is a
// registry-resolved id, never a filesystem path.
QJsonObject resultToJson(const ReframeCommandOutcome &outcome,
                         const QString &outputId)
{
    QJsonObject object;
    object.insert(QStringLiteral("ok"), outcome.ok);
    object.insert(QStringLiteral("outputPath"), outcome.outputPath);
    object.insert(QStringLiteral("outputWidth"), outcome.outputWidth);
    object.insert(QStringLiteral("outputHeight"), outcome.outputHeight);
    object.insert(QStringLiteral("outputFps"), outcome.outputFps);
    object.insert(QStringLiteral("frameCount"), outcome.frameCount);
    object.insert(QStringLiteral("durationMs"),
                  static_cast<double>(recordDurationMs(outcome)));
    object.insert(QStringLiteral("error"), outcome.error);
    object.insert(QStringLiteral("downloadUrl"),
                  outputId.isEmpty()
                      ? QString()
                      : QStringLiteral("/api/outputs/%1").arg(outputId));
    return object;
}

QJsonObject jobToJson(const JobRecord &job)
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), job.id);
    object.insert(QStringLiteral("state"), jobStateToString(job.state));
    object.insert(QStringLiteral("instruction"), job.instruction);
    object.insert(QStringLiteral("review"),
                  job.hasReview ? QJsonValue(reviewToJson(job.review))
                                : QJsonValue(QJsonValue::Null));
    object.insert(QStringLiteral("result"),
                  job.hasResult ? QJsonValue(resultToJson(job.result, job.outputId))
                                : QJsonValue(QJsonValue::Null));
    object.insert(QStringLiteral("error"),
                  job.error.isEmpty() ? QJsonValue(QJsonValue::Null)
                                      : QJsonValue(job.error));
    return object;
}

} // namespace

HttpServer::HttpServer(JobRegistry *registry, QObject *parent)
    : QObject(parent), m_registry(registry)
{
}

void HttpServer::onWorkerInitialized(bool ok, const QString &error,
                                     const QList<MediaItem> &items)
{
    m_ready = ok;
    m_startupError = error;
    m_media = items;
}

void HttpServer::onMediaListChanged(const QList<MediaItem> &items)
{
    // A projection refreshed from the authoritative signal, not a second list.
    m_media = items;
}

bool HttpServer::listen(const QString &host, quint16 port)
{
    registerRoutes();
    const quint16 bound = m_server.listen(QHostAddress(host), port);
    m_port = bound;
    return bound != 0;
}

void HttpServer::registerRoutes()
{
    // The static page. Self-contained in the binary through the Qt resource.
    m_server.route(QStringLiteral("/"), [](const QHttpServerRequest &) {
        QFile page(QStringLiteral(":/web/index.html"));
        if (!page.open(QIODevice::ReadOnly)) {
            return errorResponse(QStringLiteral("the page resource is missing"),
                                 kNotFound);
        }
        return QHttpServerResponse(
            QByteArrayLiteral("text/html; charset=utf-8"), page.readAll());
    });

    m_server.route(QStringLiteral("/api/health"), []() {
        QJsonObject object;
        object.insert(QStringLiteral("ok"), true);
        return jsonResponse(object);
    });

    m_server.route(QStringLiteral("/api/media"), [this]() {
        QJsonArray items;
        for (const MediaItem &item : m_media) {
            QJsonObject entry;
            entry.insert(QStringLiteral("id"), item.id());
            entry.insert(QStringLiteral("name"), item.fileName());
            items.append(entry);
        }
        QJsonObject object;
        object.insert(QStringLiteral("items"), items);
        return jsonResponse(object);
    });

    m_server.route(
        QStringLiteral("/api/reframe"), QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest &request) {
            if (!m_ready) {
                return errorResponse(
                    m_startupError.isEmpty()
                        ? QStringLiteral("the backend is still starting")
                        : m_startupError,
                    kUnavailable);
            }

            QJsonParseError parseError;
            const QJsonDocument document =
                QJsonDocument::fromJson(request.body(), &parseError);
            if (parseError.error != QJsonParseError::NoError
                || !document.isObject()) {
                return errorResponse(QStringLiteral("the request body is not a JSON object"),
                                     kBadRequest);
            }
            const QJsonObject body = document.object();
            const QString mediaId = body.value(QStringLiteral("mediaId")).toString();
            const QString instruction =
                body.value(QStringLiteral("instruction")).toString();
            if (mediaId.isEmpty() || instruction.trimmed().isEmpty()) {
                return errorResponse(
                    QStringLiteral("mediaId and instruction are required"),
                    kBadRequest);
            }
            bool known = false;
            for (const MediaItem &item : m_media) {
                if (item.id() == mediaId) {
                    known = true;
                    break;
                }
            }
            if (!known) {
                return errorResponse(QStringLiteral("no such media: %1").arg(mediaId),
                                     kNotFound);
            }

            const QString jobId = m_registry->createJob(mediaId, instruction.trimmed());
            emit prepareRequested(jobId, mediaId, instruction.trimmed());

            QJsonObject object;
            object.insert(QStringLiteral("jobId"), jobId);
            return jsonResponse(object);
        });

    m_server.route(QStringLiteral("/api/jobs/<arg>"),
                   [this](const QString &jobId) {
                       JobRecord job;
                       if (!m_registry->get(jobId, &job)) {
                           return errorResponse(
                               QStringLiteral("no such job: %1").arg(jobId),
                               kNotFound);
                       }
                       return jsonResponse(jobToJson(job));
                   });

    m_server.route(
        QStringLiteral("/api/jobs/<arg>/accept"), QHttpServerRequest::Method::Post,
        [this](const QString &jobId) {
            JobRecord job;
            if (!m_registry->get(jobId, &job)) {
                return errorResponse(QStringLiteral("no such job: %1").arg(jobId),
                                     kNotFound);
            }
            if (job.state != JobState::AwaitingReview) {
                return errorResponse(
                    QStringLiteral("job %1 is %2, not awaiting_review")
                        .arg(jobId, jobStateToString(job.state)),
                    kConflict);
            }
            emit acceptRequested(jobId);
            QJsonObject object;
            object.insert(QStringLiteral("ok"), true);
            return jsonResponse(object);
        });

    m_server.route(
        QStringLiteral("/api/jobs/<arg>/reject"), QHttpServerRequest::Method::Post,
        [this](const QString &jobId) {
            JobRecord job;
            if (!m_registry->get(jobId, &job)) {
                return errorResponse(QStringLiteral("no such job: %1").arg(jobId),
                                     kNotFound);
            }
            if (job.state != JobState::AwaitingReview) {
                return errorResponse(
                    QStringLiteral("job %1 is %2, not awaiting_review")
                        .arg(jobId, jobStateToString(job.state)),
                    kConflict);
            }
            emit rejectRequested(jobId);
            QJsonObject object;
            object.insert(QStringLiteral("ok"), true);
            return jsonResponse(object);
        });

    // Rendered media. The id resolves through the registry to the output path the
    // engine recorded; the URL is never used as, or concatenated into, a path.
    m_server.route(QStringLiteral("/api/outputs/<arg>"),
                   [this](const QString &outputId,
                          const QHttpServerRequest &request) -> QHttpServerResponse {
                       QString path;
                       QString jobId;
                       if (!m_registry->resolveOutput(outputId, &path, &jobId)) {
                           return errorResponse(
                               QStringLiteral("no such output: %1").arg(outputId),
                               kNotFound);
                       }
                       QFile file(path);
                       if (!file.open(QIODevice::ReadOnly)) {
                           return errorResponse(
                               QStringLiteral("the rendered file is not readable"),
                               kNotFound);
                       }
                       const qint64 total = file.size();
                       if (total <= 0) {
                           return errorResponse(
                               QStringLiteral("the rendered file is empty"), kNotFound);
                       }

                       qint64 start = 0;
                       qint64 end = total - 1;
                       bool partial = false;
                       const QByteArray rangeHeader = request.value("Range");
                       if (!rangeHeader.isEmpty()) {
                           bool satisfiable = false;
                           QString ignoreError;
                           partial = true;

                           const int equals = rangeHeader.indexOf('=');
                           const QByteArray unit =
                               equals > 0 ? rangeHeader.left(equals).trimmed().toLower()
                                          : QByteArray();
                           const QByteArray spec =
                               equals > 0 ? rangeHeader.mid(equals + 1).trimmed()
                                          : QByteArray();
                           // A single range only; multi-range is not supported.
                           if (unit == QByteArrayLiteral("bytes") && !spec.isEmpty()
                               && !spec.contains(',')) {
                               const int dash = spec.indexOf('-');
                               if (dash >= 0) {
                                   const QByteArray first = spec.left(dash).trimmed();
                                   const QByteArray last = spec.mid(dash + 1).trimmed();
                                   if (first.isEmpty()) {
                                       bool ok = false;
                                       const qint64 suffix = last.toLongLong(&ok);
                                       if (ok && suffix > 0) {
                                           start = qMax<qint64>(0, total - suffix);
                                           end = total - 1;
                                           satisfiable = true;
                                       }
                                   } else {
                                       bool ok = false;
                                       const qint64 from = first.toLongLong(&ok);
                                       if (ok && from >= 0 && from < total) {
                                           qint64 to = total - 1;
                                           if (!last.isEmpty()) {
                                               bool okTo = false;
                                               const qint64 parsed =
                                                   last.toLongLong(&okTo);
                                               if (!okTo) {
                                                   satisfiable = false;
                                               } else {
                                                   to = qMin(parsed, total - 1);
                                                   satisfiable = to >= from;
                                               }
                                           }
                                           if (satisfiable) {
                                               start = from;
                                               end = to;
                                           }
                                       }
                                   }
                               }
                           }

                           if (!satisfiable) {
                               QHttpServerResponse response(
                                   QHttpServerResponder::StatusCode::
                                       RequestRangeNotSatisfiable);
                               response.setHeader(
                                   QByteArrayLiteral("Content-Range"),
                                   QByteArrayLiteral("bytes */")
                                       + QByteArray::number(total));
                               return response;
                           }
                       }

                       const qint64 length = end - start + 1;
                       if (!file.seek(start)) {
                           return errorResponse(
                               QStringLiteral("the rendered file could not be read"),
                               kNotFound);
                       }
                       const QByteArray chunk = file.read(length);

                       QHttpServerResponse response(
                           QByteArrayLiteral("video/mp4"), chunk,
                           partial ? QHttpServerResponder::StatusCode::PartialContent
                                   : kOk);
                       response.setHeader(QByteArrayLiteral("Accept-Ranges"),
                                          QByteArrayLiteral("bytes"));
                       response.setHeader(QByteArrayLiteral("Content-Length"),
                                          QByteArray::number(chunk.size()));
                       if (partial) {
                           response.setHeader(
                               QByteArrayLiteral("Content-Range"),
                               QByteArrayLiteral("bytes ")
                                   + QByteArray::number(start) + QByteArrayLiteral("-")
                                   + QByteArray::number(end) + QByteArrayLiteral("/")
                                   + QByteArray::number(total));
                       }
                       return response;
                   });
}
