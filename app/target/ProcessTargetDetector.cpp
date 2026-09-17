#include "target/ProcessTargetDetector.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>

ProcessTargetDetector::ProcessTargetDetector() = default;

ProcessTargetDetector::ProcessTargetDetector(QString executable,
                                             QStringList arguments)
    : m_executable(std::move(executable)), m_arguments(std::move(arguments))
{
}

QString ProcessTargetDetector::name() const
{
    if (m_executable.isEmpty()) {
        return QStringLiteral("process");
    }
    return QStringLiteral("process:") + QFileInfo(m_executable).fileName();
}

void ProcessTargetDetector::setExecutable(const QString &executable)
{
    m_executable = executable;
}

void ProcessTargetDetector::setArguments(const QStringList &arguments)
{
    m_arguments = arguments;
}

void ProcessTargetDetector::setTimeoutMs(int timeoutMs)
{
    m_timeoutMs = timeoutMs > 0 ? timeoutMs : 30000;
}

bool ProcessTargetDetector::parseResponse(const QByteArray &json,
                                          QList<TargetDetection> *out,
                                          QString *error)
{
    if (error) {
        error->clear();
    }
    if (out) {
        out->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Detector output is null."));
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Detector response is not valid JSON."));
    }

    const QJsonValue detectionsValue =
        document.object().value(QStringLiteral("detections"));
    if (!detectionsValue.isArray()) {
        return fail(QStringLiteral(
            "Detector response is missing a detections array."));
    }

    const QJsonArray detections = detectionsValue.toArray();
    for (const QJsonValue &value : detections) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject object = value.toObject();
        TargetDetection detection;
        detection.boundingBox = QRectF(
            object.value(QStringLiteral("x")).toDouble(),
            object.value(QStringLiteral("y")).toDouble(),
            object.value(QStringLiteral("width")).toDouble(),
            object.value(QStringLiteral("height")).toDouble());
        detection.confidence = object.value(QStringLiteral("confidence")).toDouble();
        detection.label = object.value(QStringLiteral("label")).toString();
        detection.targetId = object.value(QStringLiteral("id")).toString();
        if (detection.isValid()) {
            out->append(detection);
        }
    }
    return true;
}

bool ProcessTargetDetector::detect(const QImage &perspectiveView,
                                   const TargetQuery &query,
                                   QList<TargetDetection> *outDetections,
                                   QString *error)
{
    if (error) {
        error->clear();
    }
    if (outDetections) {
        outDetections->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (!outDetections) {
        return fail(QStringLiteral("Detector output is null."));
    }
    if (perspectiveView.isNull()) {
        return fail(QStringLiteral("Detector view is empty."));
    }
    if (m_executable.isEmpty()) {
        return fail(QStringLiteral("Detector executable is not configured."));
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return fail(QStringLiteral("Could not create a detector workspace."));
    }
    const QString imagePath = directory.filePath(QStringLiteral("view.png"));
    if (!perspectiveView.save(imagePath, "PNG")) {
        return fail(QStringLiteral("Could not write the detector view image."));
    }

    QJsonObject queryObject;
    queryObject.insert(QStringLiteral("label"), query.label);
    queryObject.insert(QStringLiteral("targetId"), query.targetId);
    queryObject.insert(QStringLiteral("minConfidence"), query.minConfidence);

    QJsonObject request;
    request.insert(QStringLiteral("image"), imagePath);
    request.insert(QStringLiteral("width"), perspectiveView.width());
    request.insert(QStringLiteral("height"), perspectiveView.height());
    request.insert(QStringLiteral("query"), queryObject);

    const QString requestPath = directory.filePath(QStringLiteral("request.json"));
    QFile requestFile(requestPath);
    if (!requestFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Could not write the detector request."));
    }
    requestFile.write(QJsonDocument(request).toJson(QJsonDocument::Compact));
    requestFile.close();

    const QString responsePath = directory.filePath(QStringLiteral("response.json"));

    QStringList arguments = m_arguments;
    arguments << requestPath << responsePath;

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(m_executable, arguments);
    if (!process.waitForStarted(10000)) {
        return fail(QStringLiteral("Detector process could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(m_timeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Detector process timed out."));
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString detail =
            QString::fromUtf8(process.readAllStandardError()).trimmed().left(300);
        return fail(QStringLiteral("Detector process failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("unknown error")
                                              : detail));
    }

    QFile responseFile(responsePath);
    if (!responseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(QStringLiteral(
            "Detector produced no response file."));
    }
    const QByteArray response = responseFile.readAll();
    responseFile.close();
    return parseResponse(response, outDetections, error);
}
