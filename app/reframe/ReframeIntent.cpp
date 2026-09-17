#include "ReframeIntent.h"

#include <QPair>
#include <QRegularExpression>

#include <algorithm>

namespace {

QString normalized(const QString &text)
{
    QString result = text.toLower();
    result.replace(QChar(0x2013), QLatin1Char('-')); // en dash
    result.replace(QChar(0x2014), QLatin1Char('-')); // em dash
    return result;
}

struct TimeToken
{
    int start = 0;
    int end = 0;
    qint64 ms = 0;
};

QList<TimeToken> findTimeTokens(const QString &text)
{
    static const QRegularExpression expression(
        QStringLiteral("(?:(\\d{1,2}):)?(\\d{1,2}):(\\d{2})"));
    QList<TimeToken> tokens;
    QRegularExpressionMatchIterator it = expression.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString hoursText = match.captured(1);
        const int hours = hoursText.isEmpty() ? 0 : hoursText.toInt();
        const int minutes = match.captured(2).toInt();
        const int seconds = match.captured(3).toInt();
        if (minutes > 59 || seconds > 59 || hours > 99) {
            continue;
        }
        TimeToken token;
        token.start = match.capturedStart();
        token.end = match.capturedEnd();
        token.ms = ((static_cast<qint64>(hours) * 60 + minutes) * 60 + seconds)
            * 1000;
        tokens.append(token);
    }
    return tokens;
}

bool clauseHasCameraKeyword(const QString &clause)
{
    static const QRegularExpression expression(QStringLiteral(
        "\\b(look|looking|face|facing|follow|following|pan|move|moving|turn|"
        "toward|towards|center|centered|centre|centred|point|aim|view|camera|"
        "start|keep|hold|stay)\\b"));
    return expression.match(clause).hasMatch();
}

// Detects an explicit or named direction within a clause. Returns true and
// writes yaw/pitch when a direction is present.
bool directionFromClause(const QString &clause, double *yawDeg, double *pitchDeg)
{
    static const QRegularExpression yawExpression(
        QStringLiteral("yaw\\s*[=:]?\\s*(-?\\d+(?:\\.\\d+)?)"));
    static const QRegularExpression pitchExpression(
        QStringLiteral("pitch\\s*[=:]?\\s*(-?\\d+(?:\\.\\d+)?)"));

    bool found = false;
    double yaw = 0.0;
    double pitch = 0.0;

    const QRegularExpressionMatch yawMatch = yawExpression.match(clause);
    if (yawMatch.hasMatch()) {
        yaw = yawMatch.captured(1).toDouble();
        found = true;
    }
    const QRegularExpressionMatch pitchMatch = pitchExpression.match(clause);
    if (pitchMatch.hasMatch()) {
        pitch = pitchMatch.captured(1).toDouble();
        found = true;
    }
    if (found) {
        *yawDeg = yaw;
        *pitchDeg = pitch;
        return true;
    }

    const bool forward = clause.contains(QStringLiteral("forward"))
        || clause.contains(QStringLiteral("front"))
        || clause.contains(QStringLiteral("ahead"));
    const bool backward = clause.contains(QStringLiteral("behind"))
        || clause.contains(QStringLiteral("backward"))
        || clause.contains(QStringLiteral("backwards"))
        || QRegularExpression(QStringLiteral("\\bback\\b")).match(clause).hasMatch();
    const bool left = QRegularExpression(QStringLiteral("\\bleft\\b")).match(clause).hasMatch();
    const bool right = QRegularExpression(QStringLiteral("\\bright\\b")).match(clause).hasMatch();
    const bool up = QRegularExpression(QStringLiteral("\\bup\\b")).match(clause).hasMatch();
    const bool down = QRegularExpression(QStringLiteral("\\bdown\\b")).match(clause).hasMatch();

    if (forward) {
        yaw = 0.0;
        found = true;
    } else if (backward) {
        yaw = 180.0;
        found = true;
    }
    if (left) {
        yaw = -90.0;
        found = true;
    } else if (right) {
        yaw = 90.0;
        found = true;
    }
    if (up) {
        pitch = 30.0;
        found = true;
    } else if (down) {
        pitch = -30.0;
        found = true;
    }

    if (found) {
        *yawDeg = yaw;
        *pitchDeg = pitch;
    }
    return found;
}

QString cleanSubject(QString subject)
{
    subject = subject.trimmed();
    static const QRegularExpression trailing(
        QStringLiteral("[\\s.,;:!\\?]+$"));
    subject.remove(trailing);
    static const QRegularExpression leading(QStringLiteral("^[\\s.,;:!\\?]+"));
    subject.remove(leading);
    return subject.trimmed();
}

QString subjectFromClause(const QString &clause)
{
    static const QRegularExpression patterns[] = {
        QRegularExpression(QStringLiteral(
            "follow(?:ing)?\\s+(?:the\\s+)?(.+)$")),
        QRegularExpression(QStringLiteral(
            "(?:looking|look)\\s+(?:at|towards?|to)\\s+(?:the\\s+)?(.+)$")),
        QRegularExpression(QStringLiteral(
            "(?:move|moving|pan|turn|point|aim)\\s+(?:to|towards?|at)\\s+(?:the\\s+)?(.+)$")),
        QRegularExpression(QStringLiteral(
            "(?:towards?|to)\\s+(?:the\\s+)?(.+)$")),
        QRegularExpression(QStringLiteral(
            "keep\\s+me\\s+centered")),
        QRegularExpression(QStringLiteral(
            "centered?\\s+on\\s+(?:the\\s+)?(.+)$")),
        QRegularExpression(QStringLiteral(
            "keep\\s+(?:the\\s+)?(.+?)\\s+center(?:ed|red)$")),
        QRegularExpression(QStringLiteral(
            "center(?:ed|red)?\\s+(?:the\\s+)?(.+)$")),
    };
    static const int patternCount =
        static_cast<int>(sizeof(patterns) / sizeof(patterns[0]));

    for (int i = 0; i < patternCount; ++i) {
        const QRegularExpressionMatch match = patterns[i].match(clause);
        if (!match.hasMatch()) {
            continue;
        }
        if (i == 4) {
            return QStringLiteral("me");
        }
        const QString subject = cleanSubject(match.captured(1));
        if (!subject.isEmpty()) {
            return subject;
        }
    }
    return QString();
}

QStringList splitClauses(const QString &text)
{
    static const QRegularExpression splitter(QStringLiteral(
        "\\s*(?:,|;|\\.)\\s*then\\s*|\\s+then\\s+|\\s+after\\s+that\\s+|"
        "\\s+and\\s+then\\s+"));
    return text.split(splitter, Qt::SkipEmptyParts);
}

void applyAspect(ReframeIntent &intent, const QString &text)
{
    if (text.contains(QStringLiteral("9:16"))
        || text.contains(QStringLiteral("tiktok"))
        || text.contains(QStringLiteral("vertical"))
        || text.contains(QStringLiteral("reel"))
        || text.contains(QStringLiteral("shorts"))
        || text.contains(QStringLiteral("story"))) {
        intent.hasOutput = true;
        intent.outputWidth = 1080;
        intent.outputHeight = 1920;
        return;
    }
    if (text.contains(QStringLiteral("1:1"))
        || text.contains(QStringLiteral("square"))) {
        intent.hasOutput = true;
        intent.outputWidth = 1080;
        intent.outputHeight = 1080;
        return;
    }
    if (text.contains(QStringLiteral("4:5"))) {
        intent.hasOutput = true;
        intent.outputWidth = 1080;
        intent.outputHeight = 1350;
        return;
    }
    if (text.contains(QStringLiteral("16:9"))
        || text.contains(QStringLiteral("widescreen"))
        || text.contains(QStringLiteral("landscape"))
        || text.contains(QStringLiteral("normal video"))
        || text.contains(QStringLiteral("flat video"))) {
        intent.hasOutput = true;
        intent.outputWidth = 1920;
        intent.outputHeight = 1080;
    }
}

QList<TimeToken> findWordTimeTokens(const QString &text)
{
    QList<TimeToken> tokens;
    static const QRegularExpression minutes(
        QStringLiteral("(\\d+(?:\\.\\d+)?)[\\s-]*(?:minutes?|mins?|min)\\b"
                       "(?:[\\s-]*(?:and[\\s-]*)?(\\d+(?:\\.\\d+)?)"
                       "[\\s-]*(?:seconds?|secs?|sec|s)?)?"));
    static const QRegularExpression seconds(
        QStringLiteral("(\\d+(?:\\.\\d+)?)[\\s-]*(?:seconds?|secs?|sec|s)\\b"));

    QRegularExpressionMatchIterator it = minutes.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        TimeToken token;
        token.start = match.capturedStart();
        token.end = match.capturedEnd();
        token.ms = qRound64(match.captured(1).toDouble() * 60000.0);
        if (!match.captured(2).isEmpty()) {
            token.ms += qRound64(match.captured(2).toDouble() * 1000.0);
        }
        tokens.append(token);
    }
    it = seconds.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        bool overlaps = false;
        for (const TimeToken &existing : tokens) {
            if (match.capturedStart() < existing.end
                && match.capturedEnd() > existing.start) {
                overlaps = true;
                break;
            }
        }
        if (overlaps) {
            continue;
        }
        TimeToken token;
        token.start = match.capturedStart();
        token.end = match.capturedEnd();
        token.ms = qRound64(match.captured(1).toDouble() * 1000.0);
        tokens.append(token);
    }
    return tokens;
}

QList<TimeToken> allTimeTokens(const QString &text)
{
    QList<TimeToken> tokens = findTimeTokens(text);
    tokens.append(findWordTimeTokens(text));
    std::stable_sort(tokens.begin(), tokens.end(),
                     [](const TimeToken &a, const TimeToken &b) {
                         return a.start < b.start;
                     });
    return tokens;
}

// Objective 15: a temporal range plus the character span it occupies, so the
// camera parser can strip the temporal half of a compound clause.
struct TemporalSpans
{
    QList<TemporalRange> ranges;
    QList<QPair<int, int>> spans; // [from, to) character spans of each range
};

TemporalSpans temporalSpansFromText(const QString &text,
                                    const QList<TimeToken> &tokens)
{
    TemporalSpans result;
    for (int i = 0; i + 1 < tokens.size(); ++i) {
        const QString between =
            text.mid(tokens[i].end, tokens[i + 1].start - tokens[i].end);
        static const QRegularExpression separator(
            QStringLiteral("(?:to|until|through|-|>)"));
        if (separator.match(between).hasMatch()) {
            TemporalRange range;
            range.startMs = tokens[i].ms;
            range.endMs = tokens[i + 1].ms;
            result.ranges.append(range);
            result.spans.append(qMakePair(tokens[i].start, tokens[i + 1].end));
            ++i;
        }
    }
    return result;
}

QString removeSpans(QString text, const QList<QPair<int, int>> &spans)
{
    for (int i = spans.size() - 1; i >= 0; --i) {
        text.remove(spans.at(i).first, spans.at(i).second - spans.at(i).first);
    }
    return text;
}

bool hasWholeWord(const QString &text, const QString &word)
{
    const QRegularExpression expression(
        QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(word)));
    return expression.match(text).hasMatch();
}

bool isTemporalClause(const QString &clause)
{
    static const QRegularExpression targetDuration(
        QStringLiteral("(?:make|create|produce|render|generate|build|give me)?"
                       "\\s*(?:a\\s+)?(\\d+(?:\\.\\d+)?)[\\s-]*(?:second|sec)s?"
                       "\\s+(?:version|cut|clip|edit|video)"));
    if (targetDuration.match(clause).hasMatch()) {
        return true;
    }
    const bool hasTokens = !allTimeTokens(clause).isEmpty();
    const bool removeWord = clause.contains(QStringLiteral("cut out"))
        || clause.contains(QStringLiteral("cut-out"))
        || hasWholeWord(clause, QStringLiteral("remove"))
        || hasWholeWord(clause, QStringLiteral("delete"))
        || hasWholeWord(clause, QStringLiteral("drop"))
        || hasWholeWord(clause, QStringLiteral("skip"));
    const bool keepWord = clause.contains(QStringLiteral("cut from"))
        || hasWholeWord(clause, QStringLiteral("keep"))
        || hasWholeWord(clause, QStringLiteral("retain"))
        || hasWholeWord(clause, QStringLiteral("only"));
    const bool bareCut = hasWholeWord(clause, QStringLiteral("cut"))
        && !removeWord && !keepWord;
    if (bareCut) {
        return true;
    }
    if (!removeWord && !keepWord) {
        return false;
    }
    if (hasTokens) {
        return true;
    }
    return hasWholeWord(clause, QStringLiteral("section"))
        || hasWholeWord(clause, QStringLiteral("part"))
        || hasWholeWord(clause, QStringLiteral("portion"))
        || hasWholeWord(clause, QStringLiteral("segment"))
        || hasWholeWord(clause, QStringLiteral("chunk"))
        || hasWholeWord(clause, QStringLiteral("bit"))
        || hasWholeWord(clause, QStringLiteral("clip"));
}

// Objective 15: the camera-relevant remainder of a temporal clause. Time
// ranges (and a target-duration phrase) are removed so a camera/target
// instruction joined by "and" survives, while the temporal operation keyword
// is kept so "keep <subject> centered" still matches.
QString cameraClauseText(const QString &clause)
{
    static const QRegularExpression targetDuration(
        QStringLiteral("(?:make|create|produce|render|generate|build|give me)?"
                       "\\s*(?:a\\s+)?(\\d+(?:\\.\\d+)?)[\\s-]*(?:second|sec)s?"
                       "\\s+(?:version|cut|clip|edit|video)"));
    const QRegularExpressionMatch targetMatch = targetDuration.match(clause);
    if (targetMatch.hasMatch()) {
        QString residue = clause;
        residue.remove(targetMatch.capturedStart(),
                       targetMatch.capturedLength());
        return residue;
    }
    const QList<TimeToken> tokens = allTimeTokens(clause);
    if (tokens.isEmpty()) {
        return clause;
    }
    return removeSpans(clause, temporalSpansFromText(clause, tokens).spans);
}

// Removes a trailing temporal operation word left behind when the camera
// instruction precedes the temporal edit ("Follow me and keep 0:00 to 0:30").
QString stripTrailingTemporalFiller(QString subject)
{
    static const QRegularExpression trailing(QStringLiteral(
        "(?:\\s+and)?\\s+(?:keep|retain|remove|delete|drop|skip|only|"
        "cut(?:\\s+out)?)\\s*$"));
    subject.remove(trailing);
    return subject.trimmed();
}

struct TemporalParseResult
{
    bool hasRequest = false;
    bool usesDefaultRange = false;
    TemporalEditPlan plan;
    QString error;
};

TemporalParseResult parseTemporal(const QStringList &clauses)
{
    TemporalParseResult result;
    QList<TemporalRange> keepRanges;
    QList<TemporalRange> removeRanges;
    bool hasKeep = false;
    bool hasRemove = false;
    bool hasTarget = false;
    qint64 targetMs = 0;

    static const QRegularExpression targetDuration(
        QStringLiteral("(?:make|create|produce|render|generate|build|give me)?"
                       "\\s*(?:a\\s+)?(\\d+(?:\\.\\d+)?)[\\s-]*(?:second|sec)s?"
                       "\\s+(?:version|cut|clip|edit|video)"));
    static const QRegularExpression targetStartMarker(
        QStringLiteral("\\b(from|starting\\s+at|start\\s+at)\\b"));

    for (const QString &rawClause : clauses) {
        const QString clause = rawClause.trimmed();
        if (clause.isEmpty()) {
            continue;
        }
        const QList<TimeToken> tokens = allTimeTokens(clause);
        const bool hasTokens = !tokens.isEmpty();

        const QRegularExpressionMatch targetMatch = targetDuration.match(clause);
        if (targetMatch.hasMatch()) {
            if (hasKeep || hasRemove || hasTarget) {
                result.hasRequest = true;
                result.error = QStringLiteral(
                    "The instruction mixes a target-duration request with "
                    "another temporal operation; use one at a time.");
                return result;
            }
            result.hasRequest = true;
            hasTarget = true;
            targetMs = qRound64(targetMatch.captured(1).toDouble() * 1000.0);
            const int durationStart = targetMatch.capturedStart();
            const int durationEnd = targetMatch.capturedEnd();
            const bool startRequested = targetStartMarker.match(clause).hasMatch();
            qint64 startMs = -1;
            QList<QPair<int, int>> consumed;
            consumed.append(qMakePair(durationStart, durationEnd));
            for (const TimeToken &token : tokens) {
                if (token.start >= durationStart && token.end <= durationEnd) {
                    continue;
                }
                if (startRequested) {
                    startMs = token.ms;
                    consumed.append(qMakePair(token.start, token.end));
                }
                break;
            }
            // Objective 15: a leftover time token means the camera instruction
            // carries its own interval; that applicability is not supported.
            if (!allTimeTokens(removeSpans(clause, consumed)).isEmpty()) {
                result.error = QStringLiteral(
                    "The command specifies a separate time interval for the "
                    "camera instruction; applying a camera to only part of the "
                    "retained range is not supported.");
                return result;
            }
            result.plan = TemporalEditPlan::targetDuration(targetMs, startMs);
            continue;
        }

        const bool removeWord = clause.contains(QStringLiteral("cut out"))
            || clause.contains(QStringLiteral("cut-out"))
            || hasWholeWord(clause, QStringLiteral("remove"))
            || hasWholeWord(clause, QStringLiteral("delete"))
            || hasWholeWord(clause, QStringLiteral("drop"))
            || hasWholeWord(clause, QStringLiteral("skip"));
        const bool keepWord = clause.contains(QStringLiteral("cut from"))
            || hasWholeWord(clause, QStringLiteral("keep"))
            || hasWholeWord(clause, QStringLiteral("retain"))
            || hasWholeWord(clause, QStringLiteral("only"));
        const bool bareCut = hasWholeWord(clause, QStringLiteral("cut"))
            && !removeWord && !keepWord;
        const bool sectionReference =
            hasWholeWord(clause, QStringLiteral("section"))
            || hasWholeWord(clause, QStringLiteral("part"))
            || hasWholeWord(clause, QStringLiteral("portion"))
            || hasWholeWord(clause, QStringLiteral("segment"))
            || hasWholeWord(clause, QStringLiteral("chunk"))
            || hasWholeWord(clause, QStringLiteral("bit"))
            || hasWholeWord(clause, QStringLiteral("clip"));

        int kind = 0; // 0 none, 1 keep, 2 remove, 3 ambiguous cut
        if (removeWord && keepWord && hasTokens) {
            result.hasRequest = true;
            result.error = QStringLiteral(
                "The instruction asks to keep and remove footage at the same "
                "time; choose one.");
            return result;
        }
        if (removeWord) {
            kind = 2;
        } else if (keepWord) {
            kind = 1;
        } else if (bareCut) {
            kind = 3;
        } else {
            continue;
        }

        if (kind == 3) {
            result.hasRequest = true;
            result.error = QStringLiteral(
                "The word 'cut' is ambiguous here; use 'cut from X to Y' to "
                "keep a section or 'cut out X to Y' to remove one.");
            return result;
        }
        if (!hasTokens) {
            if (sectionReference) {
                // "this section"/"the boring part": the operation applies to the
                // command's effective range, which the runner substitutes.
                result.hasRequest = true;
                result.usesDefaultRange = true;
                if (kind == 1) {
                    if (hasRemove || hasTarget) {
                        result.error = QStringLiteral(
                            "The instruction asks to keep and remove footage at "
                            "the same time; choose one.");
                        return result;
                    }
                    hasKeep = true;
                } else {
                    if (hasKeep || hasTarget) {
                        result.error = QStringLiteral(
                            "The instruction asks to keep and remove footage at "
                            "the same time; choose one.");
                        return result;
                    }
                    hasRemove = true;
                }
                continue;
            }
            if (kind == 1) {
                // A non-temporal "keep" such as "keep the reframing behavior"
                // or "keep me centered" — leave it to the camera parser.
                continue;
            }
            result.hasRequest = true;
            result.error = QStringLiteral(
                "A remove request needs a time range, for example "
                "'remove 2:10 to 2:45'.");
            return result;
        }

        const TemporalSpans temporal = temporalSpansFromText(clause, tokens);
        const QList<TemporalRange> &ranges = temporal.ranges;
        if (ranges.isEmpty()) {
            result.hasRequest = true;
            result.error = QStringLiteral(
                "Could not read a time range from '%1'; write it as "
                "'from 2:10 to 2:45' or '0:35-1:10'.").arg(clause);
            return result;
        }
        // Objective 15: a leftover time token means the camera instruction
        // carries its own interval; that applicability is not supported.
        if (!allTimeTokens(removeSpans(clause, temporal.spans)).isEmpty()) {
            result.hasRequest = true;
            result.error = QStringLiteral(
                "The command specifies a separate time interval for the camera "
                "instruction; applying a camera to only part of the retained "
                "range is not supported.");
            return result;
        }
        if (kind == 1) {
            if (hasRemove || hasTarget) {
                result.hasRequest = true;
                result.error = QStringLiteral(
                    "The instruction asks to keep and remove footage at the "
                    "same time; choose one.");
                return result;
            }
            hasKeep = true;
            keepRanges.append(ranges);
        } else {
            if (hasKeep || hasTarget) {
                result.hasRequest = true;
                result.error = QStringLiteral(
                    "The instruction asks to keep and remove footage at the "
                    "same time; choose one.");
                return result;
            }
            hasRemove = true;
            removeRanges.append(ranges);
        }
        result.hasRequest = true;
    }

    if (!result.hasRequest || hasTarget) {
        return result;
    }
    if (result.usesDefaultRange) {
        result.plan = hasKeep ? TemporalEditPlan::keep(QList<TemporalRange>{})
                              : TemporalEditPlan::remove(QList<TemporalRange>{});
        return result;
    }
    if (hasKeep && !hasRemove) {
        result.plan = TemporalEditPlan::keep(keepRanges);
    } else if (hasRemove && !hasKeep) {
        result.plan = TemporalEditPlan::remove(removeRanges);
    }
    QString normalizeError;
    if (!result.plan.normalize(&normalizeError)) {
        result.error = normalizeError.isEmpty()
            ? QStringLiteral("The requested time ranges are not usable.")
            : normalizeError;
    }
    return result;
}

} // namespace

ReframeIntent ReframeIntentParser::parse(const QString &text)
{
    ReframeIntent intent;
    const QString lower = normalized(text);
    if (lower.trimmed().isEmpty()) {
        intent.notes.append(QStringLiteral("Empty instruction."));
        return intent;
    }

    applyAspect(intent, lower);

    // Objective 14: parse temporal editing into a structured value first, so
    // the legacy single-range logic does not misread it as a camera window.
    const QStringList clauses = splitClauses(lower);
    const TemporalParseResult temporal = parseTemporal(clauses);
    intent.hasTemporalRequest = temporal.hasRequest;
    intent.temporalEdit = temporal.plan;
    intent.temporalError = temporal.error;
    intent.temporalUsesDefaultRange = temporal.usesDefaultRange;
    if (intent.hasTemporalRequest) {
        if (!intent.temporalError.isEmpty()) {
            intent.notes.append(intent.temporalError);
        } else {
            intent.notes.append(QStringLiteral("Temporal edit: %1.")
                .arg(TemporalEditPlan::operationToString(
                    intent.temporalEdit.operation())));
        }
    }

    // Time range: "<start> to <end>" or the same with a dash.
    const QList<TimeToken> times = findTimeTokens(lower);
    if (!intent.hasTemporalRequest && times.size() >= 2) {
        const QString between =
            lower.mid(times[0].end, times[1].start - times[0].end);
        static const QRegularExpression separator(
            QStringLiteral("(?:to|until|through|-|>)"));
        if (separator.match(between).hasMatch()) {
            intent.hasTimeRange = true;
            intent.startMs = times[0].ms;
            intent.endMs = times[1].ms;
        }
    }
    if (!intent.hasTemporalRequest && !intent.hasTimeRange && times.size() == 1) {
        static const QRegularExpression startMarker(
            QStringLiteral("\\b(from|start(?:ing)?\\s+(?:at|from)|use|take)\\b"));
        if (startMarker.match(lower).hasMatch()) {
            intent.hasTimeRange = true;
            intent.startMs = times[0].ms;
            intent.endMs = times[0].ms;
            static const QRegularExpression forDuration(
                QStringLiteral("for\\s+(\\d+(?:\\.\\d+)?)\\s*"
                               "(?:seconds|second|secs|sec|s)\\b"));
            const QRegularExpressionMatch durationMatch = forDuration.match(lower);
            if (durationMatch.hasMatch()) {
                intent.endMs = intent.startMs
                    + static_cast<qint64>(
                        durationMatch.captured(1).toDouble() * 1000.0);
            }
        }
    }

    // Camera clauses. Objective 15: a temporal clause may also carry a
    // camera/target instruction joined by "and" ("Keep 0:00 to 0:30 and follow
    // me."); strip the temporal ranges and parse the remainder, so both halves
    // survive. The temporal edit applies to the whole retained range.
    for (const QString &rawClause : clauses) {
        const QString clause = rawClause.trimmed();
        if (clause.isEmpty()) {
            continue;
        }
        const bool temporal = isTemporalClause(clause);
        QString cameraText = temporal ? cameraClauseText(clause) : clause;
        if (temporal) {
            // The residual sentence punctuation would defeat the end-anchored
            // subject patterns ("keep the person I selected centered.").
            static const QRegularExpression trailingPunctuation(
                QStringLiteral("[\\s.,;:!\\?]+$"));
            cameraText.remove(trailingPunctuation);
        }
        if (!clauseHasCameraKeyword(cameraText)) {
            continue;
        }
        ReframeCameraMove move;
        move.label = cameraText.trimmed();

        double yaw = 0.0;
        double pitch = 0.0;
        if (directionFromClause(cameraText, &yaw, &pitch)) {
            move.hasDirection = true;
            move.yawDeg = yaw;
            move.pitchDeg = pitch;
        } else {
            QString subject = subjectFromClause(cameraText);
            if (temporal) {
                subject = stripTrailingTemporalFiller(subject);
            }
            if (!subject.isEmpty()) {
                move.targetRef = subject;
                if (!intent.unresolvedTargets.contains(subject)) {
                    intent.unresolvedTargets.append(subject);
                }
            }
        }

        if (move.hasDirection || !move.targetRef.isEmpty()) {
            intent.moves.append(move);
        }
    }

    // Objective 15: make the compound composition explicit. When a temporal
    // edit and a camera/target instruction coexist, the camera/target applies
    // to the entire retained temporal range (multiple moves interpolate as
    // before).
    if (intent.hasTemporalRequest && !intent.moves.isEmpty()
        && intent.temporalError.isEmpty()) {
        intent.notes.append(QStringLiteral(
            "Compound command: the camera/target instruction applies to the "
            "entire retained temporal range."));
    }

    if (!intent.moves.isEmpty()) {
        if (intent.moves.size() == 1) {
            intent.notes.append(QStringLiteral(
                "Single camera instruction: the direction is held for the clip."));
        } else {
            intent.notes.append(QStringLiteral(
                "Camera path interpolates across %1 instructions.")
                                   .arg(intent.moves.size()));
        }
    } else if (intent.hasOutput || intent.hasTimeRange) {
        intent.notes.append(QStringLiteral(
            "No camera instruction: a centered forward view is used."));
    }

    if (!intent.unresolvedTargets.isEmpty()) {
        intent.notes.append(QStringLiteral(
            "Unresolved subject reference(s): %1. Target resolution is a "
            "separate capability; provide resolved directions to build a plan.")
                               .arg(intent.unresolvedTargets.join(
                                   QStringLiteral(", "))));
    }

    intent.recognized = intent.hasOutput || intent.hasTimeRange
        || intent.hasTemporalRequest || !intent.moves.isEmpty();
    if (!intent.recognized) {
        intent.notes.append(QStringLiteral(
            "Instruction was not recognized by the current grammar."));
    }
    return intent;
}
