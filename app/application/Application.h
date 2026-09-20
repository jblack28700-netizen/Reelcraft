#pragma once

#include <QObject>
#include <QImage>
#include <QList>

#include <QPair>

#include <functional>
#include <memory>

#include "analysis/MediaAnalysis.h"
#include "application/DecisionProvenance.h"
#include "application/ReframeCommandOutcome.h"
#include "application/ReframePlanReview.h"
#include "core/MediaItem.h"
#include "core/Project.h"
#include "media/MediaDurationProbe.h"
#include "playback/Player.h"
#include "reframe/ReframeCommandRunner.h"
#include "reframe/ReframePipeline.h"

class QTimer;
class ViewportState;
class SpeakerEvidenceProvider;

// The application-level command execution function. It defaults to
// ReframeCommandRunner::run; tests inject a fake or a prepare-only executor so
// application orchestration stays model-free and deterministic.
using ReframeCommandExecutor = std::function<ReframeCommandResult(
    const ReframeCommandRequest &, TargetDetector *, ReframeFrameProvider *)>;

// The application-level render-preview decoder. It defaults to decoding the
// first frame of a generated output with the external FFmpeg seam; tests inject
// a fake so preview orchestration stays model-free.
using ReframePreviewDecoder =
    std::function<bool(const QString &path, QImage *out, QString *error)>;

// Builds a playback source for one persisted rendered result. The default opens
// an FfmpegFrameSource using the record's geometry; tests inject an in-memory
// source so playback orchestration stays model-free.
using PlaybackSourceFactory = std::function<std::unique_ptr<FrameSource>(
    const ReframeCommandOutcome &record, QString *error)>;

// Builds the continuous decode source for SOURCE-media playback (Objective 19).
// The default opens an FfmpegFrameSource at the requested proxy geometry and
// start offset, so playback uses one persistent decoding process and never a
// process per displayed frame. Tests inject an in-memory source so source
// playback orchestration stays model-free.
using SourcePlaybackSourceFactory = std::function<std::unique_ptr<FrameSource>(
    const QString &path, int proxyWidth, int proxyHeight, qint64 startMs,
    QString *error)>;

// The application-level replay renderer. It defaults to
// ReframePipeline::renderPlan, which needs only a validated plan and a source
// path: replay uses no detector, no frame provider and no parser. Tests inject a
// fake so replay orchestration stays model-free.
using ReframeReplayRenderer = std::function<ReframePipeline::Result(
    const ReframePlan &plan, const QString &sourcePath,
    const QString &outputPath)>;

// The outcome of one replay attempt. On success newRecordIndex identifies the
// newly appended record; the replayed source record is never modified.
struct ReplayResult
{
    bool ok = false;
    int newRecordIndex = -1;
    QString error;
};

// The outcome of one creator revision attempt (Objective 17). A revision always
// produces a NEW record; the parent record is never modified.
struct RevisionResult
{
    bool ok = false;
    int newRecordIndex = -1;
    QString error;
};

// The outcome of one creator-review accept attempt (Objective 34). Accept
// either renders the reviewed plan (ok, newRecordIndex points at the appended
// record) or fails honestly with a reason; a rejection is not an attempt.
struct ReframeReviewResult
{
    bool ok = false;
    int newRecordIndex = -1;
    int frameCount = 0;
    QString error;
};

// DecisionProvenance (the read-only "why" view of a persisted decision) lives in
// application/DecisionProvenance.h.

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    void initialize();

    Project currentProject() const;
    bool hasProject() const;
    ViewportState *viewportState() const;

    // The authoritative, deterministic media list (import order preserved).
    QList<MediaItem> mediaItems() const;

    // Point-in-time filesystem availability of the current media list. Media
    // records are never removed when their file becomes unavailable; they are
    // surfaced through these helpers instead.
    bool hasUnavailableMedia() const;
    int unavailableMediaCount() const;

    // The active/selected media ("viewer source" contract). The invariant is:
    // the active id is empty, or it resolves to exactly one current MediaItem
    // in the normalized list whose file was available at selection/restore
    // time. It is never a dangling reference.
    QString activeMediaId() const;
    const MediaItem *activeMediaItem() const;

public slots:
    void newProject();
    bool saveProject(const QString &filePath);
    bool openProject(const QString &filePath);
    void runBackgroundDemo();
    void resetViewport();
    void adjustViewportYaw(double delta);
    void adjustViewportPitch(double delta);
    void adjustViewportRoll(double delta);
    void adjustViewportFieldOfView(double delta);

    // Validates a real media file and records it in the current project.
    // Requires an active project; duplicates of the same canonical path are
    // idempotent. The original media file is never modified.
    bool importMediaFile(const QString &filePath);

    // Removes the media record with the given id from the current project.
    // Requires an active project and an existing media id. Only the record is
    // removed; the referenced file is never modified or deleted. If the active
    // media is removed, the active id is cleared.
    bool removeMedia(const QString &mediaId);

    // Sets the active/selected media to the record with the given id. Requires
    // an active project, an existing media id, and an available referenced
    // file. Never auto-selects on import; idempotent when already active.
    bool setActiveMedia(const QString &mediaId);

    // Decodes a single preview frame from the active media (FFmpeg-CLI decode
    // adapter, Objective 9) and emits framePreviewReady() on success. Requires
    // an active project, an active media record, and an available file. The
    // media file is never modified. The frame is presented through the
    // existing viewer pixel path; no viewer code changes. Decodes at the
    // current preview time position.
    bool previewActiveMediaFrame();

    // Requests a preview frame at the given time position (seconds) of the
    // active media. Negative positions clamp to 0. Beyond-end/undecodable
    // requests fail deterministically and leave the current preview position
    // unchanged. The requested position is a seek/preview request, not a
    // guarantee of exact frame or presentation-timestamp accuracy.
    bool previewActiveMediaFrameAt(double seconds);

    // Steps the current preview position by deltaSeconds and requests a frame
    // there (floor of 0 when stepping below the start). Beyond-end steps fail
    // deterministically and leave the position unchanged.
    bool stepActiveMediaPreview(double deltaSeconds);

    // The current preview time position (seconds; session state only, not
    // persisted). Reset to 0 on new project, project open, active-media
    // change, and removal of the active media.
    double previewTimeSeconds() const;

    // Declares the projection of the media record with the given id
    // (projectionValue: "equirectangular" or "flat"; other values rejected).
    // Additive record data only; requires an active project and an existing
    // media id. Re-emits mediaListChanged so presentation routing updates.
    bool declareMediaProjection(const QString &mediaId, const QString &projectionValue);

    // --- 360 reframe command orchestration (Objective 9) --------------------
    // Runs a natural-language reframe command against the ACTIVE media through
    // the existing ReframeCommandRunner. The application owns input/lifecycle
    // validation, output-path handling, and structured user feedback; command
    // interpretation/resolution/planning/execution are delegated (no duplicate
    // logic). startMs/endMs are the fallback source range used when the command
    // does not specify one and must be a valid range. The original media is
    // never modified. Emits reframeCommandFinished() for success and failure.
    bool runReframeCommand(const QString &instruction, qint64 startMs, qint64 endMs);

    // As above, but with an explicit output path. An empty path derives a
    // deterministic default next to the source (<base>_reframe.mp4).
    bool runReframeCommandTo(const QString &instruction, qint64 startMs,
                             qint64 endMs, const QString &outputPath);

    // The most recent command outcome (session state; not persisted).
    const ReframeCommandOutcome &lastReframeCommandOutcome() const;

    // Optional, replaceable command inputs. The application does NOT own them.
    void setTargetDetector(TargetDetector *detector);
    TargetDetector *targetDetector() const;
    void setCommandFrameProvider(ReframeFrameProvider *provider);
    ReframeFrameProvider *commandFrameProvider() const;

    // Test/DI seam: the command executor defaults to ReframeCommandRunner::run.
    void setReframeCommandExecutor(const ReframeCommandExecutor &executor);
    void resetReframeCommandExecutor();

    // Default output specification used when the command does not specify one.
    void setReframeDefaultOutput(int width, int height, double fps);

    // --- creator review of the prepared plan (Objective 34) ------------------
    // Creator Review lets a creator inspect the structured edit/reframe plan
    // BEFORE committing to a render. It is a VIEW of the canonical plan, not a
    // second editor and not a second plan representation:
    //
    //   prepare -> inspect -> accept (render EXACTLY the reviewed plan)
    //                      -> reject (render nothing)
    //
    // There is no revision path here, no timeline editing, and no undo: the
    // creator either accepts what the decision stage planned or goes back and
    // runs a different command. The authoritative artifact stays the validated
    // ReframePlan and, once rendered, the EditDecision that carries it.
    //
    // prepareReframeCommand() runs the DECISION stage only (parse, resolve,
    // plan) through the same validation and request construction the direct
    // command path uses, so a reviewed plan cannot drift from an executed one.
    // It never renders, never appends a record, and never modifies the source
    // media. On success a pending review is available; on failure the reason is
    // reported through reframeCommandFinished() with no output path, so nothing
    // is recorded as a render.
    bool prepareReframeCommand(const QString &instruction, qint64 startMs,
                               qint64 endMs);

    // True while a prepared plan is awaiting the creator's decision.
    bool hasPendingReview() const;
    // The pending review; an invalid review (isValid() false) when there is
    // none. The plan it carries is the exact plan Accept executes.
    ReframePlanReview pendingReview() const;

    // Accept: renders the EXACT reviewed plan through the existing
    // deterministic render seam (the same ReframePipeline entry point replay
    // uses) and records the result through the single append gate, with an
    // EditDecision built from that same plan. The plan is never re-parsed,
    // re-resolved or re-planned, and it is never mutated by review.
    //
    // An empty outputPath keeps the destination the review was prepared with; a
    // non-empty one replaces it after the same validation the command path
    // applies (the directory must exist and the path must differ from the
    // source media). The pending review is cleared on every outcome, so a
    // failed render cannot be re-accepted against a stale plan.
    ReframeReviewResult acceptReframeReview(const QString &outputPath = QString());

    // Reject: discards the pending review. Nothing is rendered, no record is
    // appended, and the source media is untouched.
    void rejectReframeReview();

    // Test/DI seam: the preparer defaults to ReframeCommandRunner::prepare. It
    // has the same shape as the command executor, and only the decision stage
    // is used.
    void setReframeCommandPreparer(const ReframeCommandExecutor &preparer);
    void resetReframeCommandPreparer();

    // --- generated render records (Objective 10) ----------------------------
    // The authoritative, ordered list of command attempts that reached an
    // output target (success and failure), recorded during the session and
    // persisted additively in the project.
    QList<ReframeCommandOutcome> reframeOutputs() const;

    // --- media analysis references (Objective 21) ----------------------------
    // Analysis is DERIVED data, never a correctness dependency. Every accessor
    // here is safe when there is no analysis at all, when the artifact file is
    // missing or unreadable, when the source media has moved or changed, and when
    // the artifact was produced by a different specification. Each of those is
    // reported as a STATUS through resolveAnalysis(); none of them can fail a
    // project load, a render or a replay.
    //
    // Analysis is deliberately NOT reachable from replayEditDecision() or from
    // any deterministic execution path (Decision 040).
    QList<MediaAnalysisReference> analysisReferences() const;
    // The reference recorded for one media record. An invalid reference when
    // none is recorded.
    MediaAnalysisReference analysisReferenceFor(const QString &mediaId) const;
    // Records (or replaces) the reference for one media record. An invalid
    // reference is refused rather than stored.
    bool setAnalysisReference(const MediaAnalysisReference &reference);
    // Resolves the recorded reference to a usable artifact, reading it from disk.
    // expectedSpecHash is optional; when supplied, an artifact produced by a
    // different specification resolves as Stale rather than Resolved.
    MediaAnalysisResolution resolveAnalysis(
        const QString &mediaId,
        const QString &expectedSpecHash = QString()) const;

    // --- edit-decision replay (Objective 16) --------------------------------
    // Re-renders the indexed persisted record from its EditDecision, without
    // re-parsing the instruction and without any perception provider: the
    // decision's stored plan is executed through the existing
    // ReframePipeline::renderPlan(). No detector, no provider, no parser.
    //
    // Output-path policy. Replay writes ONLY to the caller-supplied path:
    //   - an empty path is rejected (replay never invents an output location);
    //   - a path equal to the decision's source media is rejected, so replay can
    //     never overwrite the original media;
    //   - a path that already exists is refused outright, with the error
    //     "output path already exists: <path>; delete it or choose a fresh
    //     path". The existing file is not touched, truncated or partially
    //     written, and no record is appended. There is currently NO override for
    //     this: a caller that means to replace a file must delete it or choose a
    //     fresh path. A future explicit allowOverwrite option is recorded in
    //     Decision 033 as a forward extension and is NOT implemented.
    //
    // Validation is completed in full before any render is attempted: the index,
    // the presence of a decision, and the source fingerprint (a missing file and
    // a changed file are distinct, separately reported failures). On success a
    // NEW record is appended through the same append gate and signal as the
    // command path; the original record is left byte-identical, and the new
    // record carries the SAME decision (same decisionHash and createdUtc) with
    // the new output path.
    ReplayResult replayEditDecision(int index, const QString &outputPath);

    // Test/DI seam: the replay renderer defaults to ReframePipeline::renderPlan.
    void setReframeReplayRenderer(const ReframeReplayRenderer &renderer);
    void resetReframeReplayRenderer();

    // --- creator revision and provenance (Objective 17) ---------------------
    // Revises the indexed record by producing a NEW immutable EditDecision from
    // the creator's free-text instruction, through the SAME pipeline the command
    // path uses (parser -> plan builder -> plan -> execution). The revised
    // decision carries the revised record's single parent as parentDecisionHash.
    // The parent record is read only and is left byte-identical; there is no
    // mutator for a persisted decision.
    //
    // The output path is required and follows the same policy as replay: it must
    // be non-empty, must differ from the source media, and must not already
    // exist. Validation happens before any render is attempted.
    RevisionResult reviseEditDecision(int index, const QString &revisedInstruction,
                                      const QString &outputPath);

    // Read-only provenance of the decision behind a record. Never executes,
    // never modifies, and reports honestly when there is no usable decision.
    DecisionProvenance decisionProvenance(int index) const;

    // --- creator revision surface (Objective 35) ----------------------------
    // Record-level creator revision: the creator looks at a RENDERED edit and
    // asks for a change in words. The revision travels the existing Objective 17
    // mechanism unchanged -- a new free-text instruction through the same command
    // pipeline, producing a NEW immutable EditDecision whose single parent is the
    // record it revises -- so nothing about decision persistence, lineage,
    // hashing or replay is redefined here (Decision 055).
    //
    // The derived fresh destination (Decision 055): "<media base>_reframe_rev<N>.mp4"
    // in the revised record's directory, N being the smallest positive integer
    // whose path does not already exist. A revision through this surface therefore
    // never overwrites its parent render, and never overwrites anything else.
    QString revisionOutputPath(int index) const;

    // Revises the indexed record from the creator's revised instruction, writing
    // to the derived fresh destination. Inherits every refusal of
    // reviseEditDecision(): unknown index, a record without a usable decision, an
    // empty instruction, and a source that no longer matches the recorded
    // fingerprint. A refusal renders nothing, appends nothing and modifies
    // nothing.
    RevisionResult reviseReframeOutput(int index, const QString &revisedInstruction);

    // Supersession, DERIVED at read time (Decision 055): the records that name
    // this record's decision as their parent. Nothing is stored; a record is
    // superseded exactly when another held record points at it. Empty when the
    // index or the record's decision is unusable, and honest about what the
    // application currently holds -- a record whose parent was never loaded is
    // simply not listed anywhere.
    QList<int> revisionsOf(int index) const;

    // Replaceable, optional media duration probe used to default a whole-clip
    // range (a zero start/end). The application owns a built-in ffprobe probe;
    // this override is non-owned. Passing null restores the built-in probe.
    void setMediaDurationProbe(MediaDurationProbe *probe);
    MediaDurationProbe *mediaDurationProbe() const;

    // Optional, non-owned speaker evidence provider used by speaker references
    // such as "follow the speaker" (Objective 11). The application does NOT own
    // it. When absent, a speaker command is reported honestly as unresolved.
    void setSpeakerEvidenceProvider(SpeakerEvidenceProvider *provider);
    SpeakerEvidenceProvider *speakerEvidenceProvider() const;

    // Optional explicit speakerId -> targetId creator bindings honoured before
    // any speaker inference, so audio never overrides an explicit choice.
    void setSpeakerBindings(const QList<QPair<QString, QString>> &bindings);
    QList<QPair<QString, QString>> speakerBindings() const;

    // --- creator target selection (Objective 12) ----------------------------
    // Seeds the canonical creator identity "me" from the current viewport
    // direction (yaw/pitch) at the current preview time, so "follow me" and
    // "keep me centered" commands can resolve without a known track id. Session
    // state; not persisted. Requires a project and an available active media.
    bool selectCreatorTargetFromViewport();
    void clearCreatorSelection();
    bool hasCreatorSelection() const;
    CreatorTargetSelection creatorSelection() const;

    // --- generated render preview (Objective 12) ----------------------------
    // Decodes the first frame of the indexed persisted render output and emits
    // reframeOutputPreviewReady() for flat presentation. Fails honestly when the
    // index is invalid, the output file is missing, or no frame decodes. The
    // output file is only read.
    bool previewReframeOutput(int index);

    // Test/DI seam: the preview decoder defaults to the FFmpeg frame seam.
    void setReframePreviewDecoder(const ReframePreviewDecoder &decoder);
    void resetReframePreviewDecoder();

    // --- rendered-result playback (Objective 13) ----------------------------
    // Continuous deterministic playback of a PERSISTED 360 -> flat rendered
    // result, using the existing media/player seams. The Application owns the
    // source/pump/player for the selected record and drives Player::tick() from
    // its own timer; the Player owns no event loop. General media playback,
    // audio, timeline editing, and duration metadata are out of scope.
    //
    // startReframeOutputPlayback() opens (or replaces) the indexed render
    // record and plays it; if that same record is currently paused it resumes.
    // It returns false with a reason on invalid index, missing output, unknown
    // dimensions, or source-open failure.
    bool startReframeOutputPlayback(int index);
    bool pauseReframeOutputPlayback();
    bool resumeReframeOutputPlayback();
    void stopReframeOutputPlayback();

    // Event-loop driver entry point. Normally invoked by the owned timer;
    // exposed so tests can drive playback deterministically. Returns the number
    // of frames presented.
    int tickReframeOutputPlayback();

    bool isReframeOutputPlaybackActive() const;
    bool isReframeOutputPlaying() const;
    qint64 reframeOutputPlaybackFrameCount() const;
    qint64 reframeOutputPlaybackPositionMs() const;
    int reframeOutputPlaybackRecordIndex() const;

    // Injectable test seams (non-owning). The source factory defaults to an
    // FfmpegFrameSource opened with the record's geometry; the clock/pacing
    // default to the Player's owned SystemClock/DefaultPacingPolicy.
    void setPlaybackSourceFactory(const PlaybackSourceFactory &factory);
    void resetPlaybackSourceFactory();
    void setPlaybackClock(Clock *clock);
    void setPlaybackPacing(PacingPolicy *pacing);

    // --- source-media 360 playback (Objective 19) ---------------------------
    // Continuous playback of the ACTIVE media through the persistent streaming
    // decode path, so a creator can watch 360 footage and keep looking around
    // while it plays. Frames are emitted through sourcePlaybackFrameReady() and
    // presented through the EXISTING equirectangular viewer path, so the current
    // ViewportState (yaw/pitch/roll/FOV) keeps working during playback. The
    // original media is only ever read.
    //
    // play/pause/seek/stop are deterministic; seeking reopens the continuous
    // stream at the requested absolute source position and preserves whether
    // playback was running. End of media stops cleanly and emits
    // sourcePlaybackEnded().
    bool startSourcePlayback();
    bool pauseSourcePlayback();
    bool resumeSourcePlayback();
    void stopSourcePlayback();
    bool seekSourcePlayback(qint64 positionMs);

    // Event-loop driver entry point. Normally invoked by the owned timer;
    // exposed so tests can drive source playback deterministically. Returns the
    // number of frames presented.
    int tickSourcePlayback();

    bool isSourcePlaybackActive() const;
    bool isSourcePlaybackPlaying() const;
    // Absolute source position: the seek offset plus the frames presented since
    // the last open.
    qint64 sourcePlaybackPositionMs() const;
    // Probed source duration, or 0 when unknown.
    qint64 sourcePlaybackDurationMs() const;
    // Presentation frame interval in use for source playback
    // (1000/fps when the source rate is known, otherwise the default).
    qint64 sourcePlaybackFrameIntervalMs() const;

    // Test/DI seam (non-owning).
    void setSourcePlaybackSourceFactory(const SourcePlaybackSourceFactory &factory);
    void resetSourcePlaybackSourceFactory();

signals:
    void projectChanged(const Project &project);
    void backgroundCompleted(const QString &message);

    // Emitted whenever the authoritative media list changes: after a
    // successful import that appends a record, after a successful removal,
    // after a project is opened, and after a new project is created (empty).
    void mediaListChanged(const QList<MediaItem> &items);

    // Emitted whenever the active media changes (empty id = none active).
    void activeMediaChanged(const QString &mediaId);

    // Emitted after previewActiveMediaFrame() decodes a frame successfully.
    void framePreviewReady(const QImage &image);

    // Emitted whenever the current preview time position changes.
    void previewTimeChanged(double seconds);

    // Emitted after every 360 reframe command attempt (success or failure) with
    // structured, application-visible information. A preparation that produced
    // no plan (Objective 34) reports its failure the same way, with an empty
    // output path, so a failed preparation is never recorded as a render.
    void reframeCommandFinished(const ReframeCommandOutcome &outcome);

    // Emitted whenever the pending creator review changes (Objective 34): after
    // a successful prepare it carries the review, and after accept, reject, or
    // any change that invalidates it (new/opened project, active-media change
    // or removal) it carries an invalid review, meaning "there is none".
    void reframeReviewChanged(const ReframePlanReview &review);

    // Emitted whenever the persisted render-record list changes (a new record,
    // a new/opened project).
    void reframeOutputsChanged(const QList<ReframeCommandOutcome> &outputs);

    // Emitted whenever the media-analysis reference list changes (Objective 21).
    void analysisReferencesChanged();

    // Emitted when the creator "me" selection is set or cleared (Objective 12).
    void creatorSelectionChanged(bool hasSelection);

    // Emitted after previewReframeOutput() decodes a frame successfully.
    void reframeOutputPreviewReady(const QImage &image);

    // Emitted for each presented playback frame (a flat rendered result).
    void reframePlaybackFrameReady(const QImage &image);

    // Emitted when rendered-result playback starts, pauses, or stops
    // (playing=true only while frames are actually advancing).
    void reframePlaybackStateChanged(bool playing);

    // Emitted when the playback position advances (frame count, position ms).
    void reframePlaybackPositionChanged(qint64 frameCount, qint64 positionMs);

    // Emitted when a rendered result reaches its end.
    void reframePlaybackEnded();

    // Objective 19: emitted for each presented SOURCE frame (equirectangular,
    // presented through the viewer existing 360 camera path).
    void sourcePlaybackFrameReady(const QImage &image);
    void sourcePlaybackStateChanged(bool playing);
    void sourcePlaybackPositionChanged(qint64 positionMs);
    void sourcePlaybackEnded();

private:
    bool decodePreviewFrameAt(double targetSeconds);
    QString defaultReframeOutputPath(const MediaItem &media) const;
    QJsonArray reframeOutputsJson() const;
    void restoreReframeOutputsFromJson(const QJsonArray &outputs);
    QJsonArray analysisRefsJson() const;
    // Restores references leniently: a malformed entry is skipped and REPORTED,
    // never silently dropped and never fatal.
    void restoreAnalysisRefsFromJson(const QJsonArray &refs);
    // The single append/emit path for render records, shared by the command path
    // and by replayEditDecision so records are only ever persisted through one
    // gate (Objective 16).
    void appendReframeOutput(const ReframeCommandOutcome &outcome);
    // The single command implementation, shared by runReframeCommandTo() and
    // reviseEditDecision(). parentDecision is null for an ordinary command and
    // non-null for a revision, in which case the attached decision records it as
    // its single parent.
    bool runReframeCommandInternal(const QString &instruction, qint64 startMs,
                                   qint64 endMs, const QString &outputPath,
                                   const EditDecision *parentDecision);

    // Objective 34. The shared command-side validation and request
    // construction: the direct command path, the revision path and the review
    // prepare stage all build their request here, so the plan a creator reviews
    // is built from exactly the request that would otherwise have been executed.
    // On failure ok is false and outcome.error carries the reason, with everything
    // before the failing step already filled in.
    struct CommandContext
    {
        bool ok = false;
        ReframeCommandOutcome outcome;
        MediaItem media;                 // by-value snapshot of the active media
        ReframeCommandRequest request;   // meaningful only when ok
    };
    CommandContext buildReframeCommandContext(const QString &instruction,
                                              qint64 startMs, qint64 endMs,
                                              const QString &outputPath);

    // Discards any pending review, emitting reframeReviewChanged() only when
    // there was one. Never renders and never touches media.
    void clearPendingReview();

    // Objective 36: the index of the record that already writes to the given
    // output path, or -1 when no held record claims it. This is the single
    // definition of "a path a render record owns", used both to derive a fresh
    // revision destination and to refuse a revision that would overwrite another
    // record's render (Decision 056).
    int recordHoldingOutputPath(const QString &path) const;

    QJsonArray mediaJson() const;
    void restoreMediaFromJson(const QJsonArray &media);
    // Restores the active id from a persisted value after the media list has
    // been normalized: the id is kept only when it resolves to a current,
    // available media record; otherwise it is cleared deterministically.
    void restoreActiveMediaFromProject(const QString &persistedId);

    Project m_currentProject;
    bool m_hasProject = false;
    ViewportState *m_viewportState = nullptr;
    QList<MediaItem> m_mediaItems;
    QString m_activeMediaId;
    double m_previewTimeSeconds = 0.0;

    // 360 reframe command orchestration (Objective 9).
    ReframeCommandExecutor m_commandExecutor;
    // Objective 34: the decision-stage-only counterpart of m_commandExecutor,
    // used by the creator review prepare stage.
    ReframeCommandExecutor m_commandPreparer;
    TargetDetector *m_targetDetector = nullptr;
    ReframeFrameProvider *m_commandFrameProvider = nullptr;
    ReframeCommandOutcome m_lastReframeOutcome;
    int m_reframeOutputWidth = 1920;
    int m_reframeOutputHeight = 1080;
    double m_reframeOutputFps = 30.0;

    std::unique_ptr<MediaDurationProbe> m_ownedDurationProbe;
    MediaDurationProbe *m_durationProbe = nullptr;
    SpeakerEvidenceProvider *m_speakerProvider = nullptr;
    QList<QPair<QString, QString>> m_speakerBindings;
    QList<ReframeCommandOutcome> m_reframeOutputs;
    QList<MediaAnalysisReference> m_analysisRefs;
    ReframeReplayRenderer m_replayRenderer;

    // Objective 34: the pending creator review (session state, never persisted)
    // and the destination Accept will use unless it is given another one.
    ReframePlanReview m_pendingReview;
    QString m_pendingReviewOutputPath;
    QList<ReframeTarget> m_pendingReviewTargets;

    // Objective 12: creator "me" selection and render preview.
    CreatorTargetSelection m_creatorSelection;
    bool m_hasCreatorSelection = false;
    ReframePreviewDecoder m_previewDecoder;

    // Objective 13: rendered-result playback. The Application owns the playback
    // objects and the event-loop driver; the Player owns no timer/thread.
    PlaybackSourceFactory m_playbackSourceFactory;
    Clock *m_playbackClock = nullptr;
    PacingPolicy *m_playbackPacing = nullptr;
    QTimer *m_playbackTimer = nullptr;
    std::unique_ptr<FrameSource> m_playbackSource;
    std::unique_ptr<FramePump> m_playbackPump;
    std::unique_ptr<Player> m_playbackPlayer;
    int m_playbackRecordIndex = -1;

    // Objective 19: source-media playback. Deliberately SEPARATE from the
    // rendered-result playback objects so the tested Objective 13 path is not
    // perturbed; the two share the single event-loop timer and are mutually
    // exclusive.
    SourcePlaybackSourceFactory m_sourcePlaybackFactory;
    std::unique_ptr<FrameSource> m_sourcePlaybackSource;
    std::unique_ptr<FramePump> m_sourcePlaybackPump;
    std::unique_ptr<Player> m_sourcePlaybackPlayer;
    bool m_sourcePlaybackActive = false;
    qint64 m_sourcePlaybackOffsetMs = 0;
    qint64 m_sourcePlaybackDurationMs = 0;
    qint64 m_sourcePlaybackFrameIntervalMs = 40;

    int playbackIntervalMsForFps(double fps) const;

    // Objective 19. Opens the continuous source stream at an absolute source
    // position, optionally starting playback, tearing down any previous source
    // stream first. Never touches the rendered-result playback objects.
    bool openSourcePlaybackAt(qint64 positionMs, bool play);
    void teardownSourcePlayback();
};
