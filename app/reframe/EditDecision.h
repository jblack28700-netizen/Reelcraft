#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QString>

#include "core/MediaItem.h"
#include "core/MediaSourceReference.h"
#include "reframe/ReframePlan.h"

// EditDecision (Objective 16) is the persisted, versioned, reproducible record of
// ONE editing decision.
//
// It exists to answer a single question: "can this exact render be reproduced
// later, without re-parsing the natural-language command and without any
// perception provider?" The answer is yes because the decision carries the
// already-resolved ReframePlan (concrete camera keyframes, retained segments,
// output specification) plus the source reference the decision was made against.
//
// Boundaries (deliberate, and the reason this stays small):
//   - It never stores decoded pixels and never modifies source media. The source
//     is referenced by id + path + fingerprint only.
//   - It is NOT a second rendering pipeline. Replay goes through the existing
//     ReframePipeline::renderPlan().
//   - The instruction is provenance. It is NEVER re-parsed on replay.
//   - It is NOT a timeline editor. One decision = one render input.
//
// Compatibility policy: the artifact owns its own schemaVersion. The loader
// refuses to silently mis-parse an unknown/older/incompatible version rather than
// ignoring fields it does not understand.
//
// Known limitation: a decision is only persisted where an Application outcome
// record exists, which (see Application::runReframeCommandTo) means a command
// that reached a non-empty output path. Early validation failures produce no
// decision.
//
// Forward constraint (Objective 17, now implemented): persisted decisions are
// IMMUTABLE. A revision is expressed as a NEW decision that references the prior
// one through a single optional parentDecisionHash (a chain, never a graph), and
// no mutator exists that changes a stored artifact.
class EditDecision
{
public:
    // Objective 16 introduced v1. Objective 17 introduced v2, which adds the
    // OPTIONAL "origin" and "parentDecisionHash" fields. v1 decisions remain fully
    // readable: the gate accepts every version in 1..CurrentSchemaVersion, and a
    // loaded decision RETAINS the version it was loaded with, so a v1 decision
    // re-serializes byte-identically (keeping its digest) instead of being silently
    // upgraded in place.
    static constexpr int CurrentSchemaVersion = 2;

    // The media a decision was made against. Stores facts only; the file itself
    // is always opened read-only and is never written by anything here.
    //
    // mediaId anchors the decision to the project's MediaItem record (the
    // deterministic id MediaItem derives from the canonical path). path is what
    // replay actually opens. sizeBytes + lastModifiedUtc are the cheap
    // fingerprint; contentSha256 is an OPTIONAL stronger fingerprint, normally
    // empty because hashing multi-gigabyte 360 footage is not free.
    // Objective 21: the vocabulary is SHARED with the other persisted artifacts
    // that reference source media (core/MediaSourceReference.h), so field names,
    // fingerprint semantics and JSON vocabulary cannot drift apart between them.
    // These are aliases, not new types, so every existing
    // EditDecision::SourceReference / EditDecision::SourceStatus use keeps
    // compiling and the serialized payload is unchanged.
    using SourceReference = MediaSourceReference;

    // Result of comparing a decision's fingerprint against the file on disk RIGHT
    // NOW. Deliberately an enum, not a boolean: "the file is gone" and "the file
    // changed" are different failures with different operator meaning and are
    // never collapsed into one another.
    using SourceStatus = MediaSourceStatus;

    EditDecision() = default;

    // Builds a decision from a validated plan plus the media record the command
    // actually ran against. The MediaItem's recorded fingerprint is copied (it is
    // provenance of what the command used); nothing is re-stat'ed here.
    static EditDecision fromPlan(const ReframePlan &plan, const MediaItem &media,
                                 const QString &instruction,
                                 const QDateTime &createdUtc = QDateTime::currentDateTimeUtc());

    // Builds a REVISION: a brand new immutable decision formed from a creator's
    // free-text instruction, whose single parent is the decision being revised.
    // The parent is only read; nothing about it is modified.
    static EditDecision revisedFrom(const EditDecision &parent, const ReframePlan &plan,
                                    const MediaItem &media, const QString &instruction,
                                    const QDateTime &createdUtc = QDateTime::currentDateTimeUtc());

    static QString sourceStatusToString(SourceStatus status);

    int schemaVersion() const { return m_schemaVersion; }
    QDateTime createdUtc() const { return m_createdUtc; }
    QString instruction() const { return m_instruction; }
    SourceReference source() const { return m_source; }
    ReframePlan plan() const { return m_plan; }

    // --- Objective 17: provenance and single-parent lineage -----------------
    // How the decision was FORMED. Replay deliberately does NOT stamp a new
    // origin: a replayed record carries the same decision, so origin describes
    // how the decision was made, not how the record was produced.
    QString origin() const { return m_origin; }
    QString parentDecisionHash() const { return m_parentDecisionHash; }
    bool hasParentDecision() const { return !m_parentDecisionHash.isEmpty(); }

    // The only two origin values in Objective 17.
    static QString originCommand();
    static QString originCreatorRevision();

    // True for the exact origin vocabulary above; an unrecognized non-empty
    // origin is a malformed artifact, not an unknown-but-tolerable value.
    static bool isValidOrigin(const QString &origin);
    // 64 lowercase hex characters.
    static bool isValidDecisionHash(const QString &hash);

    // Version in range, a valid source reference, and a valid plan.
    bool isValid(QString *error = nullptr) const;

    // Compares the recorded fingerprint against the file on disk now. On a
    // mismatch, *detail (when given) explains exactly which field disagreed.
    SourceStatus checkSource(QString *detail = nullptr) const;

    // The canonical, hashable payload: every persisted field EXCEPT
    // "decisionHash". createdUtc IS part of it.
    QJsonObject payloadWithoutHash() const;

    // SHA-256 (lowercase hex) over the COMPACT JSON encoding of
    // payloadWithoutHash(): the "decisionHash" key itself is excluded, createdUtc
    // is included, and key order is Qt's deterministic sorted order, so the digest
    // is stable across processes. See the rule stated on the definition.
    QByteArray decisionHash() const;

    // payloadWithoutHash() plus the "decisionHash" key.
    QJsonObject toJsonObject() const;

    // Strict loader. Fails (with a descriptive error) when schemaVersion is
    // missing, non-numeric, <= 0 or greater than CurrentSchemaVersion; when the
    // source reference is missing/malformed; when the plan is missing or fails
    // ReframePlan::isValid(); or when a PRESENT decisionHash disagrees with the
    // recomputed digest; when origin is present but not in the known vocabulary; or
    // when parentDecisionHash is present but not 64 lowercase hex. A missing
    // decisionHash is tolerated (it is derived, not authoritative) and recomputed
    // on demand. Referential lineage (does the parent actually exist?) cannot be
    // decided by a single artifact and is validated by whoever consumes lineage.
    static bool readFromJsonObject(const QJsonObject &object, EditDecision *out,
                                   QString *error = nullptr);

    bool save(const QString &filePath, QString *error = nullptr) const;
    static EditDecision load(const QString &filePath, bool *ok = nullptr,
                             QString *error = nullptr);

private:
    int m_schemaVersion = CurrentSchemaVersion;
    QDateTime m_createdUtc;
    QString m_instruction;
    // Objective 17. Both are omitted from the payload when empty, which is what
    // keeps v1 decisions byte-identical (and therefore hash-valid) on re-save.
    QString m_origin;
    QString m_parentDecisionHash;
    SourceReference m_source;
    ReframePlan m_plan;
};

// Structured logging for the decision lifecycle (Objective 17).
Q_DECLARE_LOGGING_CATEGORY(reelcraftDecision)
