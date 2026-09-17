#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

// 360 Reframing Objective 5 — appearance evidence (structured, inspectable).
//
// Appearance is OPTIONAL evidence about whether two crops depict the same
// person. It is deliberately not reduced to an unexplained magic score:
//   - an embedding is a unit-normalized float vector from a replaceable
//     provider (the C++ core never links a model runtime);
//   - similarity is cosine similarity;
//   - a verdict says how the evidence relates to the existing identity:
//     Unavailable / Agree / Disagree / Weak / Ambiguous.
//
// Appearance can support bounded re-acquisition and can veto a geometric
// continuation, but it never overrides an explicit creator selection and never
// claims biometric certainty.

struct AppearanceEmbedding
{
    QList<double> values;
    double quality = 1.0;
    QString provider;

    int dimension() const { return static_cast<int>(values.size()); }
    bool isValid(QString *error = nullptr) const;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   AppearanceEmbedding *out,
                                   QString *error = nullptr);
};

class AppearanceMath
{
public:
    // L2-normalizes in place. Fails on empty or non-finite vectors.
    static bool normalize(AppearanceEmbedding *embedding, QString *error = nullptr);

    // Cosine similarity of two embeddings. Returns false (with an error) on
    // dimension mismatch or invalid vectors; *outSimilarity in [-1, 1].
    static bool cosineSimilarity(const AppearanceEmbedding &a,
                                 const AppearanceEmbedding &b,
                                 double *outSimilarity,
                                 QString *error = nullptr);

    // Deterministic temporal aggregation: element-wise mean of same-dimension
    // samples, then L2 normalization.
    static bool aggregate(const QList<AppearanceEmbedding> &samples,
                          AppearanceEmbedding *out, QString *error = nullptr);
};

enum class AppearanceVerdict
{
    Unavailable, // no provider / no evidence / provider failed
    Agree,       // strong match to the identity's appearance profile
    Disagree,    // strong mismatch
    Weak,        // similarity between the reject and accept thresholds
    Ambiguous    // more than one strong candidate
};

QString appearanceVerdictToString(AppearanceVerdict verdict);
AppearanceVerdict appearanceVerdictFromString(const QString &value);

// A stored appearance reference for one identity.
struct AppearanceProfile
{
    QString identity;
    AppearanceEmbedding reference;
    int sampleCount = 0;
    qint64 updatedAtMs = 0;
    QString provider;

    bool isValid(QString *error = nullptr) const;
    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   AppearanceProfile *out,
                                   QString *error = nullptr);
};

// One piece of appearance evidence about a candidate track.
struct AppearanceEvidence
{
    QString identity;
    QString candidateTrackId;
    AppearanceVerdict verdict = AppearanceVerdict::Unavailable;
    double similarity = -1.0;
    double acceptThreshold = 0.75;
    double rejectThreshold = 0.55;
    QString provider;
    QString detail;
    qint64 timeMs = 0;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   AppearanceEvidence *out,
                                   QString *error = nullptr);
};
