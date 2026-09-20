#include "ReframeIntent.h"

#include <QHash>
#include <QPair>
#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <cmath>

#include "reframe/ReframeMath.h"

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

// Which subject patterns ask for CONTINUOUS framing rather than a one-shot aim.
// "follow X", "keep me centered" and "keep X centered" all describe a behaviour
// that has to hold over the whole instruction range, so they are executed as a
// camera path through the subject's track. "look at X", "move to X",
// "centered on X" and "center X" describe a single direction.
const bool kFollowPatterns[] = {
    true,  // follow(ing) <subject>
    false, // look at <subject>
    false, // move/pan/turn/point/aim to <subject>
    false, // towards/to <subject>
    true,  // keep me centered
    false, // centered on <subject>
    true,  // keep <subject> centered
    false, // center <subject>
    false, // Objective 29: zoom/push/focus in on <subject> (an aim)
};

QString subjectFromClause(const QString &clause, bool *outFollow = nullptr)
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
        // Objective 29: a framing clause that also names what it frames
        // ("zoom in on the presenter", "push in on me"). An aim, not a follow.
        QRegularExpression(QStringLiteral(
            "(?:zoom|zooming|push|pushing|focus|focusing|lock|locking)\\s+"
            "(?:in\\s+)?(?:closer\\s+)?(?:on|onto)\\s+(?:the\\s+)?(.+)$")),
    };
    static const int patternCount =
        static_cast<int>(sizeof(patterns) / sizeof(patterns[0]));

    for (int i = 0; i < patternCount; ++i) {
        const QRegularExpressionMatch match = patterns[i].match(clause);
        if (!match.hasMatch()) {
            continue;
        }
        if (outFollow) {
            *outFollow = kFollowPatterns[i];
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

// Objective 29: framing. A framing clause asks for a LENS, not for a
// direction: "zoom in", "go wide", "close-up", "field of view 60". The named
// levels are absolute vertical fields of view in degrees and are listed most
// specific first, so "much wider" is never read as "wider" and "extreme
// close-up" never as "close-up". Every value lies inside the range the plan
// already validates ([20, 140]); a request outside it is reported rather than
// clamped, because silently changing what was asked for is not honest.
struct FramingLevel
{
    const char *phrase;
    double fieldOfViewDeg;
};

const FramingLevel kFramingLevels[] = {
    // Tightening.
    { "extreme close-up", 40.0 },
    { "extreme close up", 40.0 },
    { "zoom right in", 40.0 },
    { "zoom all the way in", 40.0 },
    { "zoom way in", 40.0 },
    { "zoom in a lot", 40.0 },
    { "much closer", 40.0 },
    { "a lot closer", 40.0 },
    { "much tighter", 40.0 },
    { "very tight", 40.0 },
    { "close-up", 60.0 },
    { "close up", 60.0 },
    { "zoomed in", 60.0 },
    { "zoom into", 60.0 },
    { "zoom in", 60.0 },
    { "push in", 60.0 },
    { "closer", 60.0 },
    { "tighter", 60.0 },
    { "tighten", 60.0 },
    // Widening.
    { "zoom all the way out", 140.0 },
    { "zoom way out", 140.0 },
    { "as wide as possible", 140.0 },
    { "much wider", 140.0 },
    { "a lot wider", 140.0 },
    { "very wide", 140.0 },
    { "ultra wide", 140.0 },
    { "widest", 140.0 },
    { "zoom out", 120.0 },
    { "pull back", 120.0 },
    { "back up", 120.0 },
    { "pull out", 120.0 },
    { "wider", 120.0 },
    { "wide", 120.0 },
    { "more of the scene", 120.0 },
    { "show more", 120.0 },
    { "see more", 120.0 },
};

// Index of a whole-word/whole-phrase occurrence, or -1. Whole-word matching is
// what keeps "wide" out of "widescreen" and "closer" out of an unrelated word.
int wholePhraseIndex(const QString &text, const QString &phrase)
{
    int from = 0;
    while (true) {
        const int index = text.indexOf(phrase, from);
        if (index < 0) {
            return -1;
        }
        const bool leftOk = index == 0 || !text.at(index - 1).isLetterOrNumber();
        const int end = index + phrase.size();
        const bool rightOk =
            end >= text.size() || !text.at(end).isLetterOrNumber();
        if (leftOk && rightOk) {
            return index;
        }
        from = index + 1;
    }
}

bool hasSofteningModifier(const QString &clause)
{
    return wholePhraseIndex(clause, QStringLiteral("slightly")) >= 0
        || wholePhraseIndex(clause, QStringLiteral("a bit")) >= 0
        || wholePhraseIndex(clause, QStringLiteral("a little")) >= 0;
}

// Detects a requested field of view in a clause and reports the clause span it
// occupies, so the caller can remove it before direction detection: "pull back"
// and "back up" contain direction words, and a phrase that has been consumed as
// framing must not also be read as a camera move. Explicit numbers win over
// names ("field of view 60", "60 degree field of view", "fov=60").
bool fieldOfViewFromClause(const QString &clause, double *outFieldOfViewDeg,
                           int *outStart = nullptr, int *outEnd = nullptr)
{
    static const QRegularExpression explicitAfter(QStringLiteral(
        "(?:field[\\s-]*of[\\s-]*view|fov)\\s*(?:of|=|:|to|at|is)?\\s*"
        "(\\d{1,3}(?:\\.\\d+)?)"));
    static const QRegularExpression explicitBefore(QStringLiteral(
        "(\\d{1,3}(?:\\.\\d+)?)\\s*(?:degrees?|deg|[\\x{00B0}])\\s*"
        "(?:field[\\s-]*of[\\s-]*view|fov)"));

    const QRegularExpression *explicitPatterns[] = { &explicitAfter,
                                                     &explicitBefore };
    for (const QRegularExpression *pattern : explicitPatterns) {
        const QRegularExpressionMatch match = pattern->match(clause);
        if (!match.hasMatch()) {
            continue;
        }
        bool ok = false;
        const double value = match.captured(1).toDouble(&ok);
        if (!ok || !std::isfinite(value)) {
            continue;
        }
        if (outStart) {
            *outStart = match.capturedStart();
        }
        if (outEnd) {
            *outEnd = match.capturedEnd();
        }
        if (outFieldOfViewDeg) {
            *outFieldOfViewDeg = value;
        }
        return true;
    }

    const int levelCount =
        static_cast<int>(sizeof(kFramingLevels) / sizeof(kFramingLevels[0]));
    for (int i = 0; i < levelCount; ++i) {
        const QString phrase = QString::fromLatin1(kFramingLevels[i].phrase);
        const int index = wholePhraseIndex(clause, phrase);
        if (index < 0) {
            continue;
        }
        double value = kFramingLevels[i].fieldOfViewDeg;
        // "slightly closer" is a smaller step than "closer". The modifier applies
        // to a named level only: an explicit number is a request in itself.
        if (hasSofteningModifier(clause)) {
            value = 90.0 + (value - 90.0) / 2.0;
        }
        if (outStart) {
            *outStart = index;
        }
        if (outEnd) {
            *outEnd = index + phrase.size();
        }
        if (outFieldOfViewDeg) {
            *outFieldOfViewDeg = value;
        }
        return true;
    }
    return false;
}

// The clause with one character span removed (the consumed framing phrase).
QString withoutSpan(const QString &text, int start, int end)
{
    if (start < 0 || end <= start || end > text.size()) {
        return text;
    }
    QString result = text;
    result.remove(start, end - start);
    return result;
}

// Objective 30/31: group framing. The phrase names a GROUP and, when it says so,
// its SIZE — never which tracks: those are decided at command time against the
// current tracks and identity state, exactly as a single reference is. A framing
// verb (or an explicit "in frame") must be present, so a passing mention of
// several people is not turned into a framing instruction.
//
//   "both of us" / "the three of us"   -> creator + (n-1) other visible people
//   "all of us"                        -> creator + every other visible person
//   "both people" / "the three people" -> exactly n visible people
//   "everyone" / "all of them"         -> every visible person
//
// A size is read as a word (two..ten) or digits (2..10); a named size below two
// is not a group request at all. Most specific phrases are listed first, so
// "everyone of us" is never read as "everyone" (a different family).
struct GroupPhrase
{
    const char *phrase;
    ReframeSubjectGroup group;
    int count; // 0 = count-free: the whole resolvable set
};

const GroupPhrase kGroupPhrases[] = {
    // Creator family, named size.
    { "both of us", ReframeSubjectGroup::CreatorAndOthers, 2 },
    { "us both", ReframeSubjectGroup::CreatorAndOthers, 2 },
    { "both of we", ReframeSubjectGroup::CreatorAndOthers, 2 },
    // Creator family, count-free.
    { "all of us", ReframeSubjectGroup::CreatorAndOthers, 0 },
    { "us all", ReframeSubjectGroup::CreatorAndOthers, 0 },
    { "everyone of us", ReframeSubjectGroup::CreatorAndOthers, 0 },
    // People family, named size (the numbered form below also catches these).
    { "both people", ReframeSubjectGroup::VisiblePeople, 2 },
    { "both persons", ReframeSubjectGroup::VisiblePeople, 2 },
    { "both of them", ReframeSubjectGroup::VisiblePeople, 2 },
    { "both of the people", ReframeSubjectGroup::VisiblePeople, 2 },
    // People family, count-free.
    { "everyone", ReframeSubjectGroup::VisiblePeople, 0 },
    { "everybody", ReframeSubjectGroup::VisiblePeople, 0 },
    { "all people", ReframeSubjectGroup::VisiblePeople, 0 },
    { "all persons", ReframeSubjectGroup::VisiblePeople, 0 },
    { "all of them", ReframeSubjectGroup::VisiblePeople, 0 },
    { "all the people", ReframeSubjectGroup::VisiblePeople, 0 },
    { "all of the people", ReframeSubjectGroup::VisiblePeople, 0 },
};

int numberWordToInt(const QString &word)
{
    static const QHash<QString, int> words = {
        { QStringLiteral("two"), 2 },   { QStringLiteral("three"), 3 },
        { QStringLiteral("four"), 4 },  { QStringLiteral("five"), 5 },
        { QStringLiteral("six"), 6 },   { QStringLiteral("seven"), 7 },
        { QStringLiteral("eight"), 8 }, { QStringLiteral("nine"), 9 },
        { QStringLiteral("ten"), 10 },
    };
    bool ok = false;
    const int digits = word.toInt(&ok);
    if (ok) {
        return digits;
    }
    return words.value(word, 0);
}

bool pluralGroupFromClause(const QString &clause, ReframeSubjectGroup *outGroup,
                           int *outCount)
{
    static const QRegularExpression framingVerb(QStringLiteral(
        "\\b(keep|keeping|follow|following|frame|frames|framing|hold|holding|"
        "stay|staying|show|showing|include|including|cent(?:er|re)(?:ed|d)?|"
        "in frame|in shot|in view)\\b"));
    if (!framingVerb.match(clause).hasMatch()) {
        return false;
    }

    // A named size first ("the three of us", "3 people", "four of them"), so a
    // phrase like "all three of us" is a group of exactly three.
    static const QRegularExpression numbered(QStringLiteral(
        "\\b(\\d{1,2}|two|three|four|five|six|seven|eight|nine|ten)\\s+"
        "(?:of\\s+)?(us|we|people|persons|them)\\b"));
    const QRegularExpressionMatch numberedMatch = numbered.match(clause);
    if (numberedMatch.hasMatch()) {
        const int count = numberWordToInt(numberedMatch.captured(1));
        if (count >= 2) {
            const QString noun = numberedMatch.captured(2);
            *outGroup = (noun == QLatin1String("us") || noun == QLatin1String("we"))
                ? ReframeSubjectGroup::CreatorAndOthers
                : ReframeSubjectGroup::VisiblePeople;
            if (outCount) {
                *outCount = count;
            }
            return true;
        }
    }

    const int phraseCount =
        static_cast<int>(sizeof(kGroupPhrases) / sizeof(kGroupPhrases[0]));
    for (int i = 0; i < phraseCount; ++i) {
        const QString phrase = QString::fromLatin1(kGroupPhrases[i].phrase);
        if (wholePhraseIndex(clause, phrase) < 0) {
            continue;
        }
        *outGroup = kGroupPhrases[i].group;
        if (outCount) {
            *outCount = kGroupPhrases[i].count;
        }
        return true;
    }
    return false;
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

// Objective 29: a framing clause joined by "and" ("follow me and zoom in")
// must not become part of the subject reference, for exactly the reason a
// trailing temporal word must not ("follow me and keep 0:00 to 0:30"). Note that
// plain "and" is NOT a clause separator, so the subject capture can swallow a
// framing instruction that the same clause also carries.
QString stripTrailingFramingFiller(const QString &subject)
{
    // A bare comma does not separate clauses either ("follow me, zoom in"), so
    // every conjunction that can introduce a trailing framing clause is
    // considered, latest first, and only one whose tail really is a framing
    // clause is removed.
    static const QRegularExpression separator(
        QStringLiteral("\\s+and\\s+|\\s*[,;]\\s*"));
    QList<QPair<int, int>> separators;
    QRegularExpressionMatchIterator it = separator.globalMatch(subject);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        separators.append(qMakePair(match.capturedStart(), match.capturedEnd()));
    }
    for (int i = separators.size() - 1; i >= 0; --i) {
        const QString tail = subject.mid(separators.at(i).second).trimmed();
        double ignored = 0.0;
        if (!tail.isEmpty() && fieldOfViewFromClause(tail, &ignored)) {
            return subject.left(separators.at(i).first).trimmed();
        }
    }
    return subject;
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

QList<double> ReframeIntent::requestedFieldOfViews() const
{
    QList<double> values;
    for (const ReframeCameraMove &move : moves) {
        if (!move.hasFieldOfView) {
            continue;
        }
        bool present = false;
        for (double existing : values) {
            if (qAbs(existing - move.fieldOfViewDeg) < 1e-9) {
                present = true;
                break;
            }
        }
        if (!present) {
            values.append(move.fieldOfViewDeg);
        }
    }
    return values;
}

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
        // Objective 29: framing is a camera instruction even when the clause
        // names no camera verb, so it is detected BEFORE the clause is
        // classified, and the phrase it consumes is removed from the text used
        // for direction detection — "pull back" and "back up" contain direction
        // words that the framing phrase has already accounted for.
        double requestedFieldOfViewDeg = 0.0;
        int fieldOfViewStart = -1;
        int fieldOfViewEnd = -1;
        const bool hasFieldOfView =
            fieldOfViewFromClause(cameraText, &requestedFieldOfViewDeg,
                                  &fieldOfViewStart, &fieldOfViewEnd);
        // Objective 30: a plural framing clause is a camera instruction too, and
        // it is detected before the single-subject patterns so that "keep both
        // of us centered" is never read as the single subject "both of us".
        ReframeSubjectGroup subjectGroup = ReframeSubjectGroup::None;
        int subjectCount = 0;
        const bool plural =
            pluralGroupFromClause(cameraText, &subjectGroup, &subjectCount);
        if (!clauseHasCameraKeyword(cameraText) && !hasFieldOfView && !plural) {
            continue;
        }
        if (hasFieldOfView
            && !reframe::isValidFieldOfViewDeg(requestedFieldOfViewDeg)) {
            // Reported, never clamped: the plan validator refuses the
            // unsatisfiable value, and this note says what was asked for.
            intent.notes.append(
                QStringLiteral("Requested field of view %1 degrees is outside "
                               "the supported range [%2, %3].")
                    .arg(QString::number(requestedFieldOfViewDeg, 'g', 10))
                    .arg(reframe::kMinFieldOfViewDeg)
                    .arg(reframe::kMaxFieldOfViewDeg));
        }
        const QString directionText = hasFieldOfView
            ? withoutSpan(cameraText, fieldOfViewStart, fieldOfViewEnd)
            : cameraText;

        ReframeCameraMove move;
        move.label = cameraText.trimmed();
        if (hasFieldOfView) {
            move.hasFieldOfView = true;
            move.fieldOfViewDeg = requestedFieldOfViewDeg;
        }

        if (plural) {
            // Continuous framing of several subjects: the same class of request
            // as "keep me centered", executed as one camera path. A direction in
            // the same clause is still parsed, so the command can refuse the
            // combination honestly instead of silently dropping half of it.
            move.subjectGroup = subjectGroup;
            move.subjectCount = subjectCount;
            move.followSubject = true;
        }

        double yaw = 0.0;
        double pitch = 0.0;
        if (directionFromClause(directionText, &yaw, &pitch)) {
            move.hasDirection = true;
            move.yawDeg = yaw;
            move.pitchDeg = pitch;
        } else if (!plural) {
            bool follow = false;
            QString subject = subjectFromClause(cameraText, &follow);
            if (temporal) {
                subject = stripTrailingTemporalFiller(subject);
            }
            subject = stripTrailingFramingFiller(subject);
            if (!subject.isEmpty()) {
                move.targetRef = subject;
                move.followSubject = follow;
                if (!intent.unresolvedTargets.contains(subject)) {
                    intent.unresolvedTargets.append(subject);
                }
            }
        }

        // A framing-only clause is KEPT: it changes the lens without moving the
        // camera, which is what turns "start wide, then push in on me" into a
        // real two-keyframe move instead of a silently dropped instruction.
        if (move.hasDirection || !move.targetRef.isEmpty()
            || move.hasFieldOfView
            || move.subjectGroup != ReframeSubjectGroup::None) {
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

    // Objective 30/31: one deterministic note naming the group asked for.
    for (const ReframeCameraMove &move : intent.moves) {
        if (move.subjectGroup == ReframeSubjectGroup::None) {
            continue;
        }
        const bool creator =
            move.subjectGroup == ReframeSubjectGroup::CreatorAndOthers;
        if (move.subjectCount >= 2) {
            intent.notes.append(
                QStringLiteral("Multi-subject framing: %1 (%2 subjects).")
                    .arg(creator
                             ? QStringLiteral("the creator and %1 other visible "
                                              "people")
                                   .arg(move.subjectCount - 1)
                             : QStringLiteral("%1 visible people")
                                   .arg(move.subjectCount))
                    .arg(move.subjectCount));
        } else {
            intent.notes.append(
                creator
                    ? QStringLiteral("Multi-subject framing: the creator and "
                                     "every other visible person.")
                    : QStringLiteral("Multi-subject framing: every visible "
                                     "person."));
        }
        break;
    }

    // Objective 29: one deterministic note naming the framing the instruction
    // asked for, in instruction order.
    const QList<double> requestedFieldOfViews = intent.requestedFieldOfViews();
    if (!requestedFieldOfViews.isEmpty()) {
        QStringList values;
        for (double value : requestedFieldOfViews) {
            values.append(QString::number(value, 'g', 10));
        }
        intent.notes.append(QStringLiteral("Framing: field of view %1 degrees.")
                                .arg(values.join(QStringLiteral(", "))));
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
    // (A framing-only instruction is recognized because it produces a move.)
    if (!intent.recognized) {
        intent.notes.append(QStringLiteral(
            "Instruction was not recognized by the current grammar."));
    }
    return intent;
}
