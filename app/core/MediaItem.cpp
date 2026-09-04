#include "MediaItem.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QJsonValue>
#include <QJsonArray>

namespace {

QString canonicalPathFor(const QFileInfo &info)
{
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}

} // namespace

MediaItem MediaItem::createFromFilePath(const QString &filePath, QString *error)
{
    MediaItem item;

    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return MediaItem();
    };

    if (filePath.isEmpty()) {
        return fail(QStringLiteral("No media file selected."));
    }

    const QFileInfo info(filePath);
    if (!info.exists()) {
        return fail(QStringLiteral("Media file does not exist: %1").arg(filePath));
    }
    if (info.isDir()) {
        return fail(QStringLiteral("Media path is a directory, not a file: %1").arg(filePath));
    }
    if (!info.isFile()) {
        return fail(QStringLiteral("Media path is not a regular file: %1").arg(filePath));
    }
    if (!info.isReadable()) {
        return fail(QStringLiteral("Media file is not readable: %1").arg(filePath));
    }

    item.m_path = canonicalPathFor(info);
    item.m_fileName = info.fileName();
    item.m_sizeBytes = info.size();
    item.m_lastModifiedUtc = info.lastModified().toUTC();

    const QString suffix = info.suffix().toLower();
    item.m_formatTag = suffix.isEmpty() ? QStringLiteral("unknown") : suffix;

    const QByteArray pathBytes = item.m_path.toUtf8();
    item.m_id = QString::fromLatin1(
        QCryptographicHash::hash(pathBytes, QCryptographicHash::Sha256).toHex());

    return item;
}

bool MediaItem::isValid() const
{
    return !m_id.isEmpty() && !m_path.isEmpty() && !m_fileName.isEmpty()
        && m_sizeBytes >= 0 && m_lastModifiedUtc.isValid();
}

bool MediaItem::referenceExists() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).exists();
}

QJsonObject MediaItem::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), m_id);
    object.insert(QStringLiteral("path"), m_path);
    object.insert(QStringLiteral("fileName"), m_fileName);
    object.insert(QStringLiteral("formatTag"), m_formatTag);
    object.insert(QStringLiteral("sizeBytes"), QString::number(m_sizeBytes));
    object.insert(QStringLiteral("lastModifiedUtc"), m_lastModifiedUtc.toString(Qt::ISODateWithMs));
    if (!m_attributes.isEmpty()) {
        object.insert(QStringLiteral("attributes"), m_attributes);
    }
    return object;
}

bool MediaItem::readFromJsonObject(const QJsonObject &object, QString *error)
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

    const QString id = object.value(QStringLiteral("id")).toString();
    const QString path = object.value(QStringLiteral("path")).toString();
    const QString fileName = object.value(QStringLiteral("fileName")).toString();
    const QString formatTag = object.value(QStringLiteral("formatTag")).toString();

    bool sizeOk = false;
    const qint64 sizeBytes = object.value(QStringLiteral("sizeBytes")).toString().toLongLong(&sizeOk);

    const QDateTime lastModifiedUtc = QDateTime::fromString(
        object.value(QStringLiteral("lastModifiedUtc")).toString(), Qt::ISODateWithMs);

    if (id.isEmpty() || path.isEmpty() || fileName.isEmpty() || formatTag.isEmpty()
        || !sizeOk || sizeBytes < 0 || !lastModifiedUtc.isValid()) {
        return fail(QStringLiteral("Media record is missing required fields."));
    }

    m_id = id;
    m_path = path;
    m_fileName = fileName;
    m_formatTag = formatTag;
    m_sizeBytes = sizeBytes;
    m_lastModifiedUtc = lastModifiedUtc;

    const QJsonValue attributesValue = object.value(QStringLiteral("attributes"));
    if (attributesValue.isObject()) {
        m_attributes = attributesValue.toObject();
    } else {
        m_attributes = QJsonObject();
    }

    return true;
}
