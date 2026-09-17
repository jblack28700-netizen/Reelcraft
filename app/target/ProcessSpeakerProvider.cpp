#include "target/ProcessSpeakerProvider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>

ProcessSpeakerProvider::ProcessSpeakerProvider() = default;

ProcessSpeakerProvider::ProcessSpeakerProvider(QString executable,
                                               QStringList arguments)
    : m_executable(std::move(executable)), m_arguments(std::move(arguments))
{
}

QString ProcessSpeakerProvider::name() const
{
    if (m_executable.isEmpty()) {
        return QStringLiteral("process");
    }
    return QStringLiteral("process:") + QFileInfo(m_executable).fileName();
}

void ProcessSpeakerProvider::setExecutable(const QString &executable)
{
    m_executable = executable;
}

void ProcessSpeakerProvider::setArguments(const QStringList &arguments)
{
    m_arguments = arguments;
}

void ProcessSpeakerProvider::setTimeoutMs(int timeoutMs)
{
    m_timeoutMs = timeoutMs > 0 ? timeoutMs : 60000;
}

bool ProcessSpeakerProvider::parseResponse(const QByteArray &json,
                                           SpeakerAnalysis *out, QString *error)
{
    if (error) {
        error->clear();
    }
    if (!out) {
        if (error) {
            *error = QStringLiteral("Speaker analysis output is null.");
        }
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = QStringLiteral("Speaker response is not valid JSON.");
        }
        return false;
    }
    return SpeakerAnalysis::readFromJsonObject(document.object(), out, error);
}

bool ProcessSpeakerProvider::analyze(const QString &mediaPath, qint64 startMs,
                                     qint64 endMs, SpeakerAnalysis *out,
                                     QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Speaker analysis output is null."));
    }
    if (endMs <= startMs || startMs < 0) {
        return fail(QStringLiteral("Speaker analysis time range is invalid."));
    }
    if (mediaPath.isEmpty() || !QFileInfo::exists(mediaPath)) {
        return fail(QStringLiteral("Speaker analysis media does not exist."));
    }
    if (m_executable.isEmpty()) {
        return fail(QStringLiteral("Speaker provider executable is not configured."));
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return fail(QStringLiteral("Could not create a speaker workspace."));
    }
    QJsonObject request;
    request.insert(QStringLiteral("media"), mediaPath);
    request.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
    request.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
    const QString requestPath = directory.filePath(QStringLiteral("request.json"));
    QFile requestFile(requestPath);
    if (!requestFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Could not write the speaker request."));
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
        return fail(QStringLiteral("Speaker provider could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(m_timeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Speaker provider timed out."));
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString detail =
            QString::fromUtf8(process.readAllStandardError()).trimmed().left(300);
        return fail(QStringLiteral("Speaker provider failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("unknown error")
                                              : detail));
    }

    QFile responseFile(responsePath);
    if (!responseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Speaker provider produced no response file."));
    }
    const QByteArray response = responseFile.readAll();
    responseFile.close();
    return parseResponse(response, out, error);
}
