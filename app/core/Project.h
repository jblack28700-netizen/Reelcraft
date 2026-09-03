#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>

class Project
{
public:
    Project();

    bool isValid() const;
    QString id() const;
    QString name() const;
    QDateTime created() const;

    void setName(const QString &name);

    bool save(const QString &filePath, QString *error = nullptr) const;
    static Project load(const QString &filePath, bool *ok = nullptr, QString *error = nullptr);

private:
    QString m_id;
    QString m_name;
    QDateTime m_created;
};

Q_DECLARE_METATYPE(Project)
