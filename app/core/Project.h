#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QJsonArray>
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

    // Optional additive media section (array of media record objects). Kept as
    // opaque JSON here, mirroring the viewerState pattern: Application owns the
    // authoritative media list and serializes it into this section on save.
    QJsonArray media() const;
    void setMedia(const QJsonArray &media);

    // Optional additive section for generated 360 render records (Objective 10).
    // Kept as opaque JSON, mirroring the media section: Application owns the
    // authoritative records and serializes them here on save.
    QJsonArray reframeOutputs() const;
    void setReframeOutputs(const QJsonArray &reframeOutputs);

    // Optional additive reference to the active/selected media record id.
    // Written only when non-empty; read leniently. Mirrors the viewerState
    // pattern: Application owns the authoritative value and validates it
    // against the media list after open.
    QString activeMediaId() const;
    void setActiveMediaId(const QString &activeMediaId);

    int schemaVersion() const;
    static constexpr int CurrentSchemaVersion = 3;

    bool save(const QString &filePath, QString *error = nullptr) const;
    static Project load(const QString &filePath, bool *ok = nullptr, QString *error = nullptr);

private:
    QString m_id;
    QString m_name;
    QDateTime m_created;
    QJsonObject m_viewerState;
    QJsonArray m_media;
    QJsonArray m_reframeOutputs;
    QString m_activeMediaId;
    int m_schemaVersion = CurrentSchemaVersion;
};

Q_DECLARE_METATYPE(Project)
