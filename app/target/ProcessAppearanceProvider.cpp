#include "target/ProcessAppearanceProvider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QTemporaryDir>

ProcessAppearanceProvider::ProcessAppearanceProvider() = default;

ProcessAppearanceProvider::ProcessAppearanceProvider(QString executable,
                                                     QStringList arguments)
    : m_executable(std::move(executable)), m_arguments(std::move(arguments))
{
}

QString ProcessAppearanceProvider::name() const
{
    if (m_executable.isEmpty()) {
        return QStringLiteral("process");
    }
    return QStringLiteral("process:") + QFileInfo(m_executable).fileName();
}

void ProcessAppearanceProvider::setExecutable(const QString &executable)
{
    m_executable = executable;
}

void ProcessAppearanceProvider::setArguments(const QStringList &arguments)
{
    m_arguments = arguments;
}

void ProcessAppearanceProvider::setTimeoutMs(int timeoutMs)
{
    m_timeoutMs = timeoutMs > 0 ? timeoutMs : 30000;
}

void ProcessAppearanceProvider::setExpectedDimension(int dimension)
{
    m_expectedDimension = dimension;
}

int ProcessAppearanceProvider::expectedDimension() const
{
    return m_expectedDimension;
}

bool ProcessAppearanceProvider::parseResponse(const QByteArray &json,
                                              int expectedDimension,
                                              AppearanceEmbedding *out,
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
        return fail(QStringLiteral("Appearance output is null."));
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Appearance response is not valid JSON."));
    }
    AppearanceEmbedding embedding;
    if (!AppearanceEmbedding::readFromJsonObject(document.object(), &embedding,
                                                 error)) {
        return false;
    }
    if (expectedDimension > 0 && embedding.dimension() != expectedDimension) {
        return fail(QStringLiteral(
            "Appearance embedding dimension %1 does not match expected %2.")
                        .arg(embedding.dimension())
                        .arg(expectedDimension));
    }
    *out = embedding;
    return true;
}

bool ProcessAppearanceProvider::encode(const QImage &crop,
                                       const QString &targetId, qint64 timeMs,
                                       AppearanceEmbedding *out, QString *error)
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
        return fail(QStringLiteral("Appearance output is null."));
    }
    if (crop.isNull()) {
        return fail(QStringLiteral("Appearance crop is empty."));
    }
    if (m_executable.isEmpty()) {
        return fail(QStringLiteral("Appearance provider executable is not configured."));
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return fail(QStringLiteral("Could not create an appearance workspace."));
    }
    const QString imagePath = directory.filePath(QStringLiteral("crop.png"));
    if (!crop.save(imagePath, "PNG")) {
        return fail(QStringLiteral("Could not write the appearance crop."));
    }

    QJsonObject request;
    request.insert(QStringLiteral("image"), imagePath);
    request.insert(QStringLiteral("width"), crop.width());
    request.insert(QStringLiteral("height"), crop.height());
    request.insert(QStringLiteral("targetId"), targetId);
    request.insert(QStringLiteral("timeMs"), static_cast<double>(timeMs));

    const QString requestPath = directory.filePath(QStringLiteral("request.json"));
    QFile requestFile(requestPath);
    if (!requestFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Could not write the appearance request."));
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
        return fail(QStringLiteral("Appearance provider could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(m_timeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Appearance provider timed out."));
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString detail =
            QString::fromUtf8(process.readAllStandardError()).trimmed().left(300);
        return fail(QStringLiteral("Appearance provider failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("unknown error")
                                              : detail));
    }

    QFile responseFile(responsePath);
    if (!responseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(QStringLiteral("Appearance provider produced no response file."));
    }
    const QByteArray response = responseFile.readAll();
    responseFile.close();
    return parseResponse(response, m_expectedDimension, out, error);
}
