#include "reframe/EditDecision.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

namespace {

// Streaming content digest. Chunked so a multi-gigabyte 360 source never has to
// be loaded into memory; used only when a decision carries contentSha256.
bool computeContentSha256(const QString &path, QByteArray *out, QString *error)
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

// Fingerprint timestamps are quantized to whole milliseconds.
//
// The artifact is serialized as ISO-8601 with milliseconds (Qt::ISODateWithMs),
// the same precision MediaItem uses when it persists a media record, while
// QFileInfo reports the filesystem's full (often sub-millisecond) resolution.
// Comparing raw values would therefore report a spurious "the file changed"
// after every save/load cycle, so both sides are quantized to the precision the
// artifact actually stores.
QDateTime toUtcMilliseconds(const QDateTime &value)
{
    if (!value.isValid()) {
        return QDateTime();
    }
    return QDateTime::fromMSecsSinceEpoch(value.toMSecsSinceEpoch(), Qt::UTC);
}

} // namespace

bool EditDecision::SourceReference::isValid() const
{
    return !mediaId.isEmpty() && !path.isEmpty() && sizeBytes >= 0
        && lastModifiedUtc.isValid();
}

EditDecision EditDecision::fromPlan(const ReframePlan &plan, const MediaItem &media,
                                    const QString &instruction,
                                    const QDateTime &createdUtc)
{
    EditDecision decision;
    decision.m_schemaVersion = CurrentSchemaVersion;
    decision.m_createdUtc = toUtcMilliseconds(createdUtc);
    decision.m_instruction = instruction;
    decision.m_source.mediaId = media.id();
    decision.m_source.path = media.path();
    decision.m_source.sizeBytes = media.sizeBytes();
    decision.m_source.lastModifiedUtc =
        toUtcMilliseconds(media.lastModifiedUtc());
    // contentSha256 is intentionally left empty: the cheap, always-available
    // fingerprint is (sizeBytes, lastModifiedUtc). A caller that wants the
    // stronger content digest sets it explicitly.
    decision.m_source.contentSha256.clear();
    decision.m_plan = plan;
    return decision;
}

QString EditDecision::sourceStatusToString(SourceStatus status)
{
    switch (status) {
    case SourceStatus::Matches:
        return QStringLiteral("matches");
    case SourceStatus::FileMissing:
        return QStringLiteral("file-missing");
    case SourceStatus::FingerprintMismatch:
        return QStringLiteral("fingerprint-mismatch");
    }
    return QStringLiteral("unknown");
}

bool EditDecision::isValid(QString *error) const
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

    if (m_schemaVersion <= 0 || m_schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral("Edit decision schema version is unsupported."));
    }
    if (!m_createdUtc.isValid()) {
        return fail(QStringLiteral("Edit decision has no valid creation time."));
    }
    if (!m_source.isValid()) {
        return fail(QStringLiteral("Edit decision source reference is incomplete."));
    }
    QString planError;
    if (!m_plan.isValid(&planError)) {
        return fail(QStringLiteral("Edit decision plan is invalid: %1").arg(planError));
    }
    return true;
}

EditDecision::SourceStatus EditDecision::checkSource(QString *detail) const
{
    if (detail) {
        detail->clear();
    }

    // Class 1: the referenced file is not there at all.
    const QFileInfo info(m_source.path);
    if (m_source.path.isEmpty() || !info.exists() || !info.isFile()) {
        if (detail) {
            *detail = QStringLiteral("The referenced source file does not exist: %1")
                          .arg(m_source.path.isEmpty()
                                   ? QStringLiteral("(empty path)")
                                   : m_source.path);
        }
        return SourceStatus::FileMissing;
    }

    // Class 2: the file is there but it is not the file the decision was made
    // against. Reported separately from FileMissing on purpose.
    if (m_source.sizeBytes >= 0 && info.size() != m_source.sizeBytes) {
        if (detail) {
            *detail = QStringLiteral(
                          "The source file has changed: recorded %1 bytes, found %2 "
                          "bytes.")
                          .arg(m_source.sizeBytes)
                          .arg(info.size());
        }
        return SourceStatus::FingerprintMismatch;
    }

    const QDateTime currentLastModified =
        toUtcMilliseconds(info.lastModified());
    if (m_source.lastModifiedUtc.isValid()
        && currentLastModified != m_source.lastModifiedUtc) {
        if (detail) {
            *detail = QStringLiteral(
                          "The source file has changed: recorded modification time "
                          "%1, found %2.")
                          .arg(m_source.lastModifiedUtc.toString(Qt::ISODateWithMs),
                               currentLastModified.toString(Qt::ISODateWithMs));
        }
        return SourceStatus::FingerprintMismatch;
    }

    if (!m_source.contentSha256.isEmpty()) {
        QByteArray actual;
        QString hashError;
        if (!computeContentSha256(m_source.path, &actual, &hashError)) {
            if (detail) {
                *detail = QStringLiteral(
                              "The source file could not be read for verification: %1")
                              .arg(hashError);
            }
            return SourceStatus::FingerprintMismatch;
        }
        if (actual.compare(m_source.contentSha256.toLatin1(), Qt::CaseInsensitive) != 0) {
            if (detail) {
                *detail = QStringLiteral(
                              "The source file has changed: recorded content hash %1, "
                              "found %2.")
                              .arg(m_source.contentSha256, QString::fromLatin1(actual));
            }
            return SourceStatus::FingerprintMismatch;
        }
    }

    return SourceStatus::Matches;
}

QJsonObject EditDecision::payloadWithoutHash() const
{
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    object.insert(QStringLiteral("createdUtc"),
                  toUtcMilliseconds(m_createdUtc).toString(Qt::ISODateWithMs));
    object.insert(QStringLiteral("instruction"), m_instruction);

    QJsonObject source;
    source.insert(QStringLiteral("mediaId"), m_source.mediaId);
    source.insert(QStringLiteral("path"), m_source.path);
    source.insert(QStringLiteral("sizeBytes"),
                  static_cast<double>(m_source.sizeBytes));
    source.insert(QStringLiteral("lastModifiedUtc"),
                  toUtcMilliseconds(m_source.lastModifiedUtc)
                      .toString(Qt::ISODateWithMs));
    if (!m_source.contentSha256.isEmpty()) {
        source.insert(QStringLiteral("contentSha256"), m_source.contentSha256);
    }
    object.insert(QStringLiteral("source"), source);

    object.insert(QStringLiteral("plan"), m_plan.toJsonObject());
    return object;
}

// decisionHash rule (Objective 16, amendment 1):
//
//   decisionHash = lowercase hex SHA-256 over
//   QJsonDocument(payloadWithoutHash()).toJson(QJsonDocument::Compact)
//
// Concretely: the "decisionHash" key itself is REMOVED before hashing, and
// createdUtc IS included. Qt serializes QJsonObject keys in a deterministic
// sorted order and renders doubles deterministically, so the same decision
// produces the same digest in any process. This makes the digest usable for
// diffs, integrity checks on load, and replay comparisons.
QByteArray EditDecision::decisionHash() const
{
    const QByteArray canonical =
        QJsonDocument(payloadWithoutHash()).toJson(QJsonDocument::Compact);
    return QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex();
}

QJsonObject EditDecision::toJsonObject() const
{
    QJsonObject object = payloadWithoutHash();
    object.insert(QStringLiteral("decisionHash"),
                  QString::fromLatin1(decisionHash()));
    return object;
}

bool EditDecision::readFromJsonObject(const QJsonObject &object, EditDecision *out,
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
        return fail(QStringLiteral("Edit decision output is null."));
    }

    EditDecision decision;

    // Version gate. A missing or unknown version is refused outright rather than
    // parsed on the assumption that the layout is compatible.
    const QJsonValue schemaValue = object.value(QStringLiteral("schemaVersion"));
    if (!schemaValue.isDouble()) {
        return fail(QStringLiteral("Edit decision is missing schemaVersion."));
    }
    decision.m_schemaVersion = schemaValue.toInt();
    if (decision.m_schemaVersion <= 0
        || decision.m_schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral(
                        "Edit decision schema version %1 is unsupported (supported: "
                        "1..%2).")
                        .arg(decision.m_schemaVersion)
                        .arg(CurrentSchemaVersion));
    }

    decision.m_createdUtc = toUtcMilliseconds(QDateTime::fromString(
        object.value(QStringLiteral("createdUtc")).toString(), Qt::ISODateWithMs));
    if (!decision.m_createdUtc.isValid()) {
        return fail(QStringLiteral("Edit decision has no valid creation time."));
    }

    decision.m_instruction = object.value(QStringLiteral("instruction")).toString();

    const QJsonValue sourceValue = object.value(QStringLiteral("source"));
    if (!sourceValue.isObject()) {
        return fail(QStringLiteral("Edit decision is missing its source reference."));
    }
    const QJsonObject sourceObject = sourceValue.toObject();
    decision.m_source.mediaId = sourceObject.value(QStringLiteral("mediaId")).toString();
    decision.m_source.path = sourceObject.value(QStringLiteral("path")).toString();

    const QJsonValue sizeValue = sourceObject.value(QStringLiteral("sizeBytes"));
    if (!sizeValue.isDouble()) {
        return fail(QStringLiteral(
            "Edit decision source reference is missing a numeric size."));
    }
    decision.m_source.sizeBytes = static_cast<qint64>(sizeValue.toDouble());
    decision.m_source.lastModifiedUtc = toUtcMilliseconds(QDateTime::fromString(
        sourceObject.value(QStringLiteral("lastModifiedUtc")).toString(),
        Qt::ISODateWithMs));
    decision.m_source.contentSha256 =
        sourceObject.value(QStringLiteral("contentSha256")).toString();

    if (!decision.m_source.isValid()) {
        return fail(QStringLiteral(
            "Edit decision source reference is incomplete (mediaId, path, size and "
            "modification time are required)."));
    }

    const QJsonValue planValue = object.value(QStringLiteral("plan"));
    if (!planValue.isObject()) {
        return fail(QStringLiteral("Edit decision is missing its plan."));
    }
    QString planError;
    if (!ReframePlan::readFromJsonObject(planValue.toObject(), &decision.m_plan,
                                         &planError)) {
        return fail(QStringLiteral("Edit decision plan could not be read: %1")
                        .arg(planError));
    }

    // Integrity: a present digest must agree. A missing digest is tolerated
    // because it is derived, not authoritative.
    const QJsonValue hashValue = object.value(QStringLiteral("decisionHash"));
    if (!hashValue.isUndefined() && !hashValue.isNull()) {
        if (!hashValue.isString()) {
            return fail(QStringLiteral("Edit decision hash is not a string."));
        }
        const QString recorded = hashValue.toString();
        if (!recorded.isEmpty()) {
            const QString actual = QString::fromLatin1(decision.decisionHash());
            if (recorded.compare(actual, Qt::CaseInsensitive) != 0) {
                return fail(QStringLiteral(
                                "Edit decision hash mismatch: the stored decision "
                                "does not match its recorded digest (recorded %1, "
                                "computed %2).")
                                .arg(recorded, actual));
            }
        }
    }

    QString validationError;
    if (!decision.isValid(&validationError)) {
        return fail(validationError);
    }

    *out = decision;
    return true;
}

bool EditDecision::save(const QString &filePath, QString *error) const
{
    if (error) {
        error->clear();
    }
    QString validationError;
    if (!isValid(&validationError)) {
        if (error) {
            *error = validationError;
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    const QByteArray data =
        QJsonDocument(toJsonObject()).toJson(QJsonDocument::Indented);
    if (file.write(data) < 0) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

EditDecision EditDecision::load(const QString &filePath, bool *ok, QString *error)
{
    if (ok) {
        *ok = false;
    }
    if (error) {
        error->clear();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return EditDecision();
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.errorString();
        }
        return EditDecision();
    }

    EditDecision decision;
    if (!readFromJsonObject(document.object(), &decision, error)) {
        return EditDecision();
    }
    if (ok) {
        *ok = true;
    }
    return decision;
}
