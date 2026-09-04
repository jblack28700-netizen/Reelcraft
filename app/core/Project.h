#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QJsonObject>

class Project
{
public:
    Project();

    bool isValid() const;
    QString id() const;
    QString name() const;
    QDateTime created() const;

    void setName(const QString &name);

    QJsonObject viewerState() const;
    void setViewerState(const QJsonObject &viewerState);

    int schemaVersion() const;
    static constexpr int CurrentSchemaVersion = 2;

    bool save(const QString &filePath, QString *error = nullptr) const;
    static Project load(const QString &filePath, bool *ok = nullptr, QString *error = nullptr);

private:
    QString m_id;
    QString m_name;
    QDateTime m_created;
    QJsonObject m_viewerState;
    int m_schemaVersion = CurrentSchemaVersion;
};

Q_DECLARE_METATYPE(Project)
