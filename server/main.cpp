#include <QGuiApplication>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "HttpServer.h"
#include "JobRegistry.h"
#include "Worker.h"

namespace {

void printUsage(QTextStream &out)
{
    out << "reelcraft_server — Reelcraft browser control layer (Decision 059, Slice 1)" << Qt::endl
        << Qt::endl
        << "Usage: reelcraft_server --media <path> [--host <addr>] [--port <n>]" << Qt::endl
        << Qt::endl
        << "  --media <path>   an existing 360 media file (required)" << Qt::endl
        << "  --host <addr>    address to bind (default 127.0.0.1)" << Qt::endl
        << "  --port <n>       port to bind (default 8732)" << Qt::endl
        << Qt::endl
        << "The server is unauthenticated and has no TLS. It binds to loopback by" << Qt::endl
        << "default; --host is for explicit, deliberate exposure (for example a" << Qt::endl
        << "RunPod port-forward during testing). A real network posture is a" << Qt::endl
        << "separate future decision." << Qt::endl;
}

} // namespace

int main(int argc, char *argv[])
{
    // The backend has no window. This must be set before the application object
    // exists.
    qputenv("QT_QPA_PLATFORM", "offscreen");

    // argv is parsed by hand: the backend takes three flags and needs nothing
    // from QtCore it does not already have.
    QStringList args;
    args.reserve(argc > 0 ? argc - 1 : 0);
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]));
    }

    QString mediaPath;
    QString host = QStringLiteral("127.0.0.1");
    quint16 port = 8732;

    QTextStream err(stderr);
    QTextStream out(stdout);

    for (int i = 0; i < args.size(); ++i) {
        const QString flag = args.at(i);
        const bool needsValue = flag == QStringLiteral("--media")
            || flag == QStringLiteral("--host") || flag == QStringLiteral("--port");
        QString value;
        if (needsValue) {
            if (i + 1 >= args.size()) {
                err << "missing value for " << flag << Qt::endl;
                return 2;
            }
            value = args.at(++i);
        }

        if (flag == QStringLiteral("--media")) {
            mediaPath = value;
        } else if (flag == QStringLiteral("--host")) {
            host = value;
        } else if (flag == QStringLiteral("--port")) {
            bool ok = false;
            const int parsed = value.toInt(&ok);
            if (!ok || parsed <= 0 || parsed > 65535) {
                err << "invalid --port: " << value << Qt::endl;
                return 2;
            }
            port = static_cast<quint16>(parsed);
        } else if (flag == QStringLiteral("--help") || flag == QStringLiteral("-h")) {
            printUsage(out);
            return 0;
        } else {
            err << "unknown argument: " << flag << Qt::endl;
            printUsage(err);
            return 2;
        }
    }

    if (mediaPath.isEmpty()) {
        err << "--media is required" << Qt::endl;
        printUsage(err);
        return 2;
    }

    QGuiApplication app(argc, argv);

    // Queued connections carry value types across the worker boundary; register
    // them explicitly so delivery cannot fail at runtime.
    qRegisterMetaType<QList<MediaItem>>("QList<MediaItem>");
    qRegisterMetaType<ReframePlanReview>("ReframePlanReview");
    qRegisterMetaType<ReframeCommandOutcome>("ReframeCommandOutcome");

    JobRegistry registry;
    Worker worker(&registry);
    worker.setMediaPath(mediaPath);
    // Startup-only capability wiring, mirroring app/main.cpp:207-235. Absent
    // variables simply mean the capability is absent, and a command that needs it
    // fails honestly.
    worker.setDetector(qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_PY"),
                       qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_SCRIPT"),
                       qEnvironmentVariable("REELCRAFT_TARGET_YOLOX_MODEL"));

    HttpServer server(&registry);

    // HTTP thread -> worker thread. Cross-thread signal/slot connections are
    // queued by Qt, so no Application call ever runs on the HTTP thread.
    QObject::connect(&server, &HttpServer::prepareRequested, &worker,
                     &Worker::prepare);
    QObject::connect(&server, &HttpServer::acceptRequested, &worker,
                     &Worker::accept);
    QObject::connect(&server, &HttpServer::rejectRequested, &worker,
                     &Worker::reject);

    // Worker thread -> HTTP thread.
    QObject::connect(&worker, &Worker::initialized, &server,
                     &HttpServer::onWorkerInitialized);
    QObject::connect(&worker, &Worker::mediaListChanged, &server,
                     &HttpServer::onMediaListChanged);
    QObject::connect(&worker, &Worker::statusMessage, [](const QString &message) {
        QTextStream(stderr) << "[reelcraft_server] " << message << Qt::endl;
    });

    worker.start();

    if (!server.listen(host, port)) {
        err << "could not bind " << host << ":" << port << Qt::endl;
        worker.stop();
        return 1;
    }

    out << "reelcraft_server listening on http://" << host << ":" << server.port()
        << Qt::endl
        << "media: " << mediaPath << Qt::endl
        << "detector: "
        << (qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_PY").isEmpty()
                ? QStringLiteral("absent (subject commands will refuse honestly)")
                : QStringLiteral("configured"))
        << Qt::endl;
    out.flush();

    const int rc = app.exec();
    worker.stop();
    return rc;
}
