#include "ReframeIntent.h"

#include <QRegularExpression>

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

    // Time range: "<start> to <end>" or the same with a dash.
    const QList<TimeToken> times = findTimeTokens(lower);
    if (times.size() >= 2) {
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
    if (!intent.hasTimeRange && times.size() == 1) {
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

    // Camera clauses.
    const QStringList clauses = splitClauses(lower);
    for (const QString &rawClause : clauses) {
        const QString clause = rawClause.trimmed();
        if (clause.isEmpty() || !clauseHasCameraKeyword(clause)) {
            continue;
        }
        ReframeCameraMove move;
        move.label = clause;

        double yaw = 0.0;
        double pitch = 0.0;
        if (directionFromClause(clause, &yaw, &pitch)) {
            move.hasDirection = true;
            move.yawDeg = yaw;
            move.pitchDeg = pitch;
        } else {
            const QString subject = subjectFromClause(clause);
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
        || !intent.moves.isEmpty();
    if (!intent.recognized) {
        intent.notes.append(QStringLiteral(
            "Instruction was not recognized by the current grammar."));
    }
    return intent;
}
