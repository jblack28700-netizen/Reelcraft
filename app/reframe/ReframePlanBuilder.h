#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"

// ReframeBuildResult carries the structured plan produced from an intent, or a
// deterministic error explaining why it could not be built.
struct ReframeBuildResult
{
    bool ok = false;
    QString error;
    ReframePlan plan;
    ReframeIntent intent;
    QStringList notes;
};

// ReframePlanBuilder converts a structured intent plus RESOLVED target
// directions into a validated ReframePlan. It never fabricates a subject
// position: an unresolved target reference produces an error so the caller can
// resolve it first.
class ReframePlanBuilder
{
public:
    // defaultRange/defaultOutput supply the source range and output spec when
    // the intent does not specify them.
    static ReframeBuildResult build(
        const ReframeIntent &intent,
        const QList<ReframeTarget> &resolvedTargets,
        const ReframePlan::TimeRange &defaultRange,
        const ReframePlan::OutputSpec &defaultOutput);
};
