#pragma once

#include <QString>
#include <QStringList>

// Read-only provenance view of the decision behind a render record (Objective
// 17). Inspection never executes anything and never modifies the artifact.
//
// It lives in its own header beside the other application-visible value types
// (ReframeCommandOutcome, ReframePlanReview) so the UI can present provenance
// without depending on the whole Application.
struct DecisionProvenance
{
    bool available = false;      // a decision is attached and loaded
    QString error;               // why it is unavailable, when it is not
    QString origin;
    QString instruction;
    // Objective 41: the record's own notes, which is where a creator modification
    // explains its parameter (for example a lens widening). Presentation of stored
    // facts; the FACT of a modification is the hashed origin/parentDecisionHash.
    QStringList notes;
    QString parentDecisionHash;
    // Referential lineage check: does the referenced parent actually exist among
    // the records this application holds? A syntactically valid hash is not
    // proof of a valid lineage relationship.
    bool hasParent = false;
    bool parentResolved = false;
    QString sourceStatus;        // EditDecision::sourceStatusToString()
    QString sourceDetail;
    // Plan summary (no keyframes are copied).
    int keyframeCount = 0;
    int segmentCount = 0;
    qint64 planStartMs = 0;
    qint64 planEndMs = 0;
    int outputWidth = 0;
    int outputHeight = 0;
    double outputFps = 0.0;
    int planFrameCount = 0;
};
