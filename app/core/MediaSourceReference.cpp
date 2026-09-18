#include "MediaSourceReference.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

bool MediaSourceReference::isValid() const
{
    return !mediaId.isEmpty() && !path.isEmpty() && sizeBytes >= 0
        && lastModifiedUtc.isValid();
}

QString mediaSourceStatusToString(MediaSourceStatus status)
{
    switch (status) {
    case MediaSourceStatus::Matches:
        return QStringLiteral("matches");
    case MediaSourceStatus::FileMissing:
        return QStringLiteral("file-missing");
    case MediaSourceStatus::FingerprintMismatch:
        return QStringLiteral("fingerprint-mismatch");
    }
    return QStringLiteral("unknown");
}

QDateTime mediaSourceTimestampToUtcMs(const QDateTime &value)
{
    if (!value.isValid()) {
        return QDateTime();
    }
    return QDateTime::fromMSecsSinceEpoch(value.toMSecsSinceEpoch(), Qt::UTC);
}

bool computeMediaContentSha256(const QString &path, QByteArray *out, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    constexpr qint64 kChunk = 1 << 20;
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(kChunk);
        if (chunk.isEmpty() && file.error() != QFile::NoError) {
            if (error) {
                *error = file.errorString();
            }
            return false;
        }
        hash.addData(chunk);
    }
    if (out) {
        *out = hash.result().toHex();
    }
    return true;
}

MediaSourceStatus checkMediaSourceStatus(const MediaSourceReference &reference,
                                         QString *detail)
{
    if (detail) {
        detail->clear();
    }

    // Class 1: the referenced file is not there at all.
    const QFileInfo info(reference.path);
    if (reference.path.isEmpty() || !info.exists() || !info.isFile()) {
        if (detail) {
            *detail = QStringLiteral("The referenced source file does not exist: %1")
                          .arg(reference.path.isEmpty()
                                   ? QStringLiteral("(empty path)")
                                   : reference.path);
        }
        return MediaSourceStatus::FileMissing;
    }

    // Class 2: the file is there but it is not the file the artifact was made
    // against. Reported separately from FileMissing on purpose.
    if (reference.sizeBytes >= 0 && info.size() != reference.sizeBytes) {
        if (detail) {
            *detail = QStringLiteral(
                          "The source file has changed: recorded %1 bytes, found %2 "
                          "bytes.")
                          .arg(reference.sizeBytes)
                          .arg(info.size());
        }
        return MediaSourceStatus::FingerprintMismatch;
    }

    const QDateTime currentLastModified =
        mediaSourceTimestampToUtcMs(info.lastModified());
    if (reference.lastModifiedUtc.isValid()
        && currentLastModified != reference.lastModifiedUtc) {
        if (detail) {
            *detail = QStringLiteral(
                          "The source file has changed: recorded modification time "
                          "%1, found %2.")
                          .arg(reference.lastModifiedUtc.toString(Qt::ISODateWithMs),
                               currentLastModified.toString(Qt::ISODateWithMs));
        }
        return MediaSourceStatus::FingerprintMismatch;
    }

    if (!reference.contentSha256.isEmpty()) {
        QByteArray actual;
        QString hashError;
        if (!computeMediaContentSha256(reference.path, &actual, &hashError)) {
            if (detail) {
                *detail = QStringLiteral(
                              "The source file could not be read for verification: %1")
                              .arg(hashError);
            }
            return MediaSourceStatus::FingerprintMismatch;
        }
        if (actual.compare(reference.contentSha256.toLatin1(), Qt::CaseInsensitive) != 0) {
            if (detail) {
                *detail = QStringLiteral(
                              "The source file has changed: recorded content hash %1, "
                              "found %2.")
                              .arg(reference.contentSha256, QString::fromLatin1(actual));
            }
            return MediaSourceStatus::FingerprintMismatch;
        }
    }

    return MediaSourceStatus::Matches;
}

