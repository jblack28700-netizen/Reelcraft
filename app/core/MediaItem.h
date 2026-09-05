#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QMetaType>
#include <QString>

// MediaItem is a deterministic record of one imported real media file.
//
// It records only factual, file-system-derived metadata (id, stored path,
// file name, size, last-modified time, extension-derived format tag) plus an
// optional creator-declared projection. It never decodes, probes, or reads
// media content, and it never modifies the original file. The optional
// attributes object is the future home for content/360° classification
// metadata; no classification logic exists here and no camera-specific
// behavior is introduced.
//
// MediaItem is QtCore-only so media/project logic stays headless-testable and
// independent of any viewer or playback code.
class MediaItem
{
public:
    // Source projection (Objective 12). Unknown = not declared / legacy /
    // unrecognized persisted value; equirectangular and flat are the only
    // recognized values.
    enum class Projection { Unknown, Equirectangular, Flat };

    MediaItem() = default;

    // Creates a record for an existing, readable, regular file. Fails with a
    // descriptive error for empty paths, missing files, directories, and
    // unreadable paths. The media file itself is never modified.
    static MediaItem createFromFilePath(const QString &filePath, QString *error = nullptr);

    bool isValid() const;

    // True when the stored path currently refers to an existing file. A valid
    // record may reference a file that is temporarily unavailable; validation
    // happens again on import/open without invalidating the record.
    bool referenceExists() const;

    QString id() const { return m_id; }
    QString path() const { return m_path; }
    QString fileName() const { return m_fileName; }
    QString formatTag() const { return m_formatTag; }
    qint64 sizeBytes() const { return m_sizeBytes; }
    QDateTime lastModifiedUtc() const { return m_lastModifiedUtc; }

    // Optional creator-declared projection. Serialized additively only when
    // declared (not Unknown); recognized JSON values are "equirectangular" and
    // "flat"; absent or unrecognized values read back as Unknown.
    Projection projection() const { return m_projection; }
    void setProjection(Projection projection) { m_projection = projection; }

    static QString projectionToString(Projection projection);
    static Projection projectionFromString(const QString &value);

    QJsonObject attributes() const { return m_attributes; }
    void setAttributes(const QJsonObject &attributes) { m_attributes = attributes; }

    QJsonObject toJsonObject() const;
    // Restores a record from JSON. Returns false (with a descriptive error)
    // when required fields are missing or malformed.
    bool readFromJsonObject(const QJsonObject &object, QString *error = nullptr);

private:
    QString m_id;
    QString m_path;
    QString m_fileName;
    QString m_formatTag;
    qint64 m_sizeBytes = -1;
    QDateTime m_lastModifiedUtc;
    Projection m_projection = Projection::Unknown;
    QJsonObject m_attributes;
};

Q_DECLARE_METATYPE(MediaItem)
