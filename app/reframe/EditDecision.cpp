#include "reframe/EditDecision.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

Q_LOGGING_CATEGORY(reelcraftDecision, "reelcraft.decision")

// The source-reference vocabulary, its fingerprint comparison and the streaming
// content digest now live in core/MediaSourceReference.h so that EditDecision and
// MediaAnalysis share one definition. The local helper below is the only piece
// that stayed here; it is used for timestamp serialization.

namespace {

QDateTime toUtcMilliseconds(const QDateTime &value)
{
    return mediaSourceTimestampToUtcMs(value);
}

} // namespace

QString EditDecision::originCommand()
{
    return QStringLiteral("command");
}

QString EditDecision::originCreatorRevision()
{
    return QStringLiteral("creator-revision");
}

bool EditDecision::isValidOrigin(const QString &origin)
{
    return origin == originCommand() || origin == originCreatorRevision();
}

bool EditDecision::isValidDecisionHash(const QString &hash)
{
    if (hash.size() != 64) {
        return false;
    }
    for (const QChar c : hash) {
        const ushort u = c.unicode();
        const bool lowerHex = (u >= '0' && u <= '9') || (u >= 'a' && u <= 'f');
        if (!lowerHex) {
            return false;
        }
    }
    return true;
}

EditDecision EditDecision::fromPlan(const ReframePlan &plan, const MediaItem &media,
                                    const QString &instruction,
                                    const QDateTime &createdUtc)
{
    EditDecision decision;
    decision.m_schemaVersion = CurrentSchemaVersion;
    decision.m_createdUtc = toUtcMilliseconds(createdUtc);
    decision.m_instruction = instruction;
    // Formed by the command pipeline unless revisedFrom() says otherwise.
    decision.m_origin = originCommand();
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
    qCInfo(reelcraftDecision) << "created:" << decision.decisionHash()
                              << "origin" << decision.m_origin;
    return decision;
}

EditDecision EditDecision::revisedFrom(const EditDecision &parent,
                                       const ReframePlan &plan,
                                       const MediaItem &media,
                                       const QString &instruction,
                                       const QDateTime &createdUtc)
{
    // A revision is a NEW artifact. It reads the parent and never writes to it.
    EditDecision decision = fromPlan(plan, media, instruction, createdUtc);
    decision.m_origin = originCreatorRevision();
    decision.m_parentDecisionHash = parent.decisionHash();
    qCInfo(reelcraftDecision) << "revised:" << decision.decisionHash() << "parent"
                              << decision.m_parentDecisionHash;
    return decision;
}

QString EditDecision::sourceStatusToString(SourceStatus status)
{
    // One definition for every artifact that references source media.
    return mediaSourceStatusToString(status);
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
    if (!m_origin.isEmpty() && !isValidOrigin(m_origin)) {
        return fail(QStringLiteral("Edit decision origin is not recognized."));
    }
    if (!m_parentDecisionHash.isEmpty()
        && !isValidDecisionHash(m_parentDecisionHash)) {
        return fail(QStringLiteral(
            "Edit decision parent hash is not 64 lowercase hex characters."));
    }
    QString planError;
    if (!m_plan.isValid(&planError)) {
        return fail(QStringLiteral("Edit decision plan is invalid: %1").arg(planError));
    }
    return true;
}

EditDecision::SourceStatus EditDecision::checkSource(QString *detail) const
{
    // One implementation for every artifact that references source media, so
    // the three outcomes mean exactly the same thing everywhere.
    return checkMediaSourceStatus(m_source, detail);
}

QJsonObject EditDecision::payloadWithoutHash() const
{
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    object.insert(QStringLiteral("createdUtc"),
                  toUtcMilliseconds(m_createdUtc).toString(Qt::ISODateWithMs));
    object.insert(QStringLiteral("instruction"), m_instruction);
    // Objective 17. Both are OMITTED when unset. Writing them (even empty) would
    // add payload keys to every legacy v1 decision, changing its recomputed
    // digest so that it no longer matched the stored decisionHash -- the strict
    // loader would then refuse a previously-valid decision.
    if (!m_origin.isEmpty()) {
        object.insert(QStringLiteral("origin"), m_origin);
    }
    if (!m_parentDecisionHash.isEmpty()) {
        object.insert(QStringLiteral("parentDecisionHash"), m_parentDecisionHash);
    }

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
        // A refusal is never silent (Objective 17).
        qCWarning(reelcraftDecision) << "refused:" << message;
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

    // Objective 17 provenance. ABSENT means a legacy v1 decision, which is not an
    // error; PRESENT but unrecognized is malformed and is refused.
    decision.m_origin = object.value(QStringLiteral("origin")).toString();
    if (!decision.m_origin.isEmpty() && !isValidOrigin(decision.m_origin)) {
        return fail(QStringLiteral("Edit decision origin '%1' is not recognized.")
                        .arg(decision.m_origin));
    }
    decision.m_parentDecisionHash =
        object.value(QStringLiteral("parentDecisionHash")).toString();
    if (!decision.m_parentDecisionHash.isEmpty()
        && !isValidDecisionHash(decision.m_parentDecisionHash)) {
        return fail(QStringLiteral(
            "Edit decision parent hash is not 64 lowercase hex characters."));
    }

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

    qCInfo(reelcraftDecision)
        << "loaded:" << decision.decisionHash()
        << "schema" << decision.m_schemaVersion
        << "origin" << (decision.m_origin.isEmpty() ? QStringLiteral("(none)")
                                                    : decision.m_origin);

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
