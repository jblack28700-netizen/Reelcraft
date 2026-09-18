#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

// Vocabulary shared by every persisted artifact that references a source media
// file (EditDecision, MediaAnalysis).
//
// It lives in core because it belongs to neither: it answers one question --
// "which file was this made against, and is it still that file?". Keeping a
// single definition is what stops independently evolving artifacts from
// drifting apart in field names, fingerprint semantics or JSON vocabulary.
//
// Nothing here ever writes to the referenced file. Every reader opens it
// read-only.
struct MediaSourceReference
{
    // Anchors the artifact to the project's MediaItem record (the deterministic
    // id MediaItem derives from the canonical path).
    QString mediaId;
    // What a reader actually opens. Always read-only.
    QString path;
    // The cheap, always-available fingerprint: size + modification time.
    qint64 sizeBytes = -1;
    QDateTime lastModifiedUtc;
    // Optional stronger fingerprint. Deliberately NOT filled in by default:
    // hashing multi-gigabyte 360 footage is not free, so a caller that wants the
    // stronger guarantee sets it explicitly.
    QString contentSha256;

    bool isValid() const;
};

// Result of comparing a recorded fingerprint against the file on disk RIGHT NOW.
// Deliberately an enum, not a boolean: "the file is gone" and "the file changed"
// are different failures with different operator meaning and are never collapsed
// into one another.
enum class MediaSourceStatus
{
    Matches,              // the file exists and the fingerprint agrees
    FileMissing,          // the referenced path does not exist
    FingerprintMismatch,  // the file exists but is not the recorded file
};

QString mediaSourceStatusToString(MediaSourceStatus status);

// Compares reference against the file on disk now. On a mismatch, *detail (when
// given) explains exactly which field disagreed.
MediaSourceStatus checkMediaSourceStatus(const MediaSourceReference &reference,
                                         QString *detail = nullptr);

// Streaming SHA-256 of a file's content. Chunked, so a multi-gigabyte source is
// never loaded into memory. Only used when a reference carries contentSha256.
bool computeMediaContentSha256(const QString &path, QByteArray *out,
                               QString *error = nullptr);

// Fingerprint timestamps are quantized to whole milliseconds.
//
// Artifacts serialize timestamps as ISO-8601 with milliseconds
// (Qt::ISODateWithMs), the same precision MediaItem uses, while QFileInfo
// reports the filesystem's full (often sub-millisecond) resolution. Comparing
// raw values would report a spurious "the file changed" after every save/load
// cycle, so both sides are quantized to the precision the artifact stores.
QDateTime mediaSourceTimestampToUtcMs(const QDateTime &value);

