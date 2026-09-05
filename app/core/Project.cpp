#include "Project.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUuid>

Project::Project()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_name(QStringLiteral("Untitled Project")),
      m_created(QDateTime::currentDateTimeUtc())
{
}

bool Project::isValid() const
{
    return !m_id.isEmpty() && !m_name.isEmpty() && m_created.isValid();
}

QString Project::id() const
{
    return m_id;
}

QString Project::name() const
{
    return m_name;
}

QDateTime Project::created() const
{
    return m_created;
}

void Project::setName(const QString &name)
{
    m_name = name;
}

QJsonObject Project::viewerState() const
{
    return m_viewerState;
}

void Project::setViewerState(const QJsonObject &viewerState)
{
    m_viewerState = viewerState;
}

QJsonArray Project::media() const
{
    return m_media;
}

void Project::setMedia(const QJsonArray &media)
{
    m_media = media;
}

QString Project::activeMediaId() const
{
    return m_activeMediaId;
}

void Project::setActiveMediaId(const QString &activeMediaId)
{
    m_activeMediaId = activeMediaId;
}

int Project::schemaVersion() const
{
    return m_schemaVersion;
}

bool Project::save(const QString &filePath, QString *error) const
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), m_id);
    object.insert(QStringLiteral("name"), m_name);
    object.insert(QStringLiteral("created"), m_created.toString(Qt::ISODateWithMs));
    object.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    if (!m_viewerState.isEmpty()) {
        object.insert(QStringLiteral("viewerState"), m_viewerState);
    }
    if (!m_media.isEmpty()) {
        object.insert(QStringLiteral("media"), m_media);
    }
    if (!m_activeMediaId.isEmpty()) {
        object.insert(QStringLiteral("activeMediaId"), m_activeMediaId);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    const QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Indented);
    if (file.write(data) < 0) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    return true;
}

Project Project::load(const QString &filePath, bool *ok, QString *error)
{
    if (ok) {
        *ok = false;
    }

    Project invalid;
    invalid.m_id.clear();
    invalid.m_name.clear();
    invalid.m_created = QDateTime();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return invalid;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.errorString();
        }
        return invalid;
    }

    const QJsonObject object = document.object();
    const QString id = object.value(QStringLiteral("id")).toString();
    const QString name = object.value(QStringLiteral("name")).toString();
    const QDateTime created = QDateTime::fromString(
        object.value(QStringLiteral("created")).toString(), Qt::ISODateWithMs);

    if (id.isEmpty() || name.isEmpty() || !created.isValid()) {
        if (error) {
            *error = QStringLiteral("Project file is missing required fields.");
        }
        return invalid;
    }

    Project project;
    project.m_id = id;
    project.m_name = name;
    project.m_created = created;
    const int schemaVersion = object.value(QStringLiteral("schemaVersion")).toInt(1);

    if (schemaVersion > Project::CurrentSchemaVersion) {
        if (error) {
            *error = QStringLiteral("Project file was created with a newer unsupported schema version.");
        }
        return invalid;
    }

    project.m_schemaVersion = schemaVersion > 0 ? schemaVersion : 1;

    const QJsonValue viewerStateValue = object.value(QStringLiteral("viewerState"));
    if (viewerStateValue.isObject()) {
        project.m_viewerState = viewerStateValue.toObject();
    }

    const QJsonValue mediaValue = object.value(QStringLiteral("media"));
    if (mediaValue.isArray()) {
        project.m_media = mediaValue.toArray();
    }

    const QJsonValue activeMediaIdValue = object.value(QStringLiteral("activeMediaId"));
    if (activeMediaIdValue.isString()) {
        project.m_activeMediaId = activeMediaIdValue.toString();
    }

    if (ok) {
        *ok = true;
    }

    return project;
}
