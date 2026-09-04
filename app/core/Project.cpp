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

bool Project::save(const QString &filePath, QString *error) const
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), m_id);
    object.insert(QStringLiteral("name"), m_name);
    object.insert(QStringLiteral("created"), m_created.toString(Qt::ISODateWithMs));
    if (!m_viewerState.isEmpty()) {
        object.insert(QStringLiteral("viewerState"), m_viewerState);
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

    const QJsonValue viewerStateValue = object.value(QStringLiteral("viewerState"));
    if (viewerStateValue.isObject()) {
        project.m_viewerState = viewerStateValue.toObject();
    }

    if (ok) {
        *ok = true;
    }

    return project;
}
