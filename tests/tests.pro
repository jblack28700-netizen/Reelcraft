QT += testlib core concurrent widgets
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = reelcraft_tests
INCLUDEPATH += ../app

SOURCES += \
    test_project.cpp \
    ../app/core/MediaItem.cpp \
    ../app/core/MediaSourceReference.cpp \
    ../app/core/Project.cpp \
    ../app/application/Application.cpp \
    ../app/analysis/MediaAnalysis.cpp \
    ../app/analysis/MediaAnalysisRunner.cpp \
    ../app/media/FfmpegFrameSource.cpp \
    ../app/media/FfprobeDurationProbe.cpp \
    ../app/media/FrameExtractor.cpp \
    ../app/media/FramePump.cpp \
    ../app/media/FrameSource.cpp \
    ../app/playback/DefaultPacingPolicy.cpp \
    ../app/playback/Playhead.cpp \
    ../app/playback/Player.cpp \
    ../app/playback/SystemClock.cpp \
    ../app/viewer/EquirectView.cpp \
    ../app/viewer/ViewerProjection.cpp \
    ../app/viewer/ViewerScene.cpp \
    ../app/viewer/ViewportState.cpp \
    ../app/ui/MainWindow.cpp \
    ../app/ui/ViewerWidget.cpp \
    ../app/reframe/CameraKeyframe.cpp \
    ../app/reframe/CameraPath.cpp \
    ../app/reframe/EditDecision.cpp \
    ../app/reframe/FfmpegSeekFrameProvider.cpp \
    ../app/reframe/ReframeCommandRunner.cpp \
    ../app/reframe/ReframeContract.cpp \
    ../app/reframe/ReframeIntent.cpp \
    ../app/reframe/ReframePlan.cpp \
    ../app/reframe/ReframePlanBuilder.cpp \
    ../app/reframe/TemporalEditPlan.cpp \
    ../app/reframe/ReframePipeline.cpp \
    ../app/reframe/ReframeRenderer.cpp \
    ../app/reframe/ReframeStreamFrameProvider.cpp \
    ../app/target/EquirectProjection.cpp \
    ../app/target/EquirectViewPlan.cpp \
    ../app/target/ProcessTargetDetector.cpp \
    ../app/target/SphericalTargetTracker.cpp \
    ../app/target/AppearanceTypes.cpp \
    ../app/target/IdentityReidentifier.cpp \
    ../app/target/ProcessAppearanceProvider.cpp \
    ../app/target/ProcessSpeakerProvider.cpp \
    ../app/target/SpeakerEvidenceAnalyzer.cpp \
    ../app/target/SpeakerReframePlanner.cpp \
    ../app/target/SpeakerTargetAssociator.cpp \
    ../app/target/SpeakerTimeline.cpp \
    ../app/target/SpeakerTypes.cpp \
    ../app/target/TargetCropExtractor.cpp \
    ../app/target/TargetIdentity.cpp \
    ../app/target/TargetResolver.cpp \
    ../app/target/TargetSelector.cpp \
    ../app/target/TargetTrackPlanner.cpp \
    ../app/target/TargetTypes.cpp

HEADERS += \
    ../app/core/MediaItem.h \
    ../app/core/MediaSourceReference.h \
    ../app/core/Project.h \
    ../app/application/Application.h \
    ../app/analysis/MediaAnalysis.h \
    ../app/analysis/MediaAnalysisRunner.h \
    ../app/application/ReframeCommandOutcome.h \
    ../app/media/FfmpegFrameSource.h \
    ../app/media/FfprobeDurationProbe.h \
    ../app/media/FrameExtractor.h \
    ../app/media/MediaDurationProbe.h \
    ../app/media/FramePump.h \
    ../app/media/FrameSource.h \
    ../app/playback/Clock.h \
    ../app/playback/DefaultPacingPolicy.h \
    ../app/playback/PacingPolicy.h \
    ../app/playback/Playhead.h \
    ../app/playback/Player.h \
    ../app/playback/SystemClock.h \
    ../app/viewer/EquirectView.h \
    ../app/viewer/ViewerProjection.h \
    ../app/viewer/ViewerScene.h \
    ../app/viewer/ViewportState.h \
    ../app/ui/MainWindow.h \
    ../app/ui/ViewerWidget.h \
    ../app/reframe/CameraKeyframe.h \
    ../app/reframe/CameraPath.h \
    ../app/reframe/EditDecision.h \
    ../app/reframe/FfmpegSeekFrameProvider.h \
    ../app/reframe/ReframeCommandRunner.h \
    ../app/reframe/ReframeContract.h \
    ../app/reframe/ReframeFrameProvider.h \
    ../app/reframe/ReframeIntent.h \
    ../app/reframe/ReframeMath.h \
    ../app/reframe/ReframePlan.h \
    ../app/reframe/ReframePlanBuilder.h \
    ../app/reframe/TemporalEditPlan.h \
    ../app/reframe/ReframePipeline.h \
    ../app/reframe/ReframeRenderer.h \
    ../app/reframe/ReframeStreamFrameProvider.h \
    ../app/target/EquirectProjection.h \
    ../app/target/EquirectViewPlan.h \
    ../app/target/ProcessTargetDetector.h \
    ../app/target/SphericalTargetTracker.h \
    ../app/target/TargetDetector.h \
    ../app/target/AppearanceProvider.h \
    ../app/target/AppearanceTypes.h \
    ../app/target/IdentityReidentifier.h \
    ../app/target/ProcessAppearanceProvider.h \
    ../app/target/SpeakerEvidenceAnalyzer.h \
    ../app/target/SpeakerEvidenceProvider.h \
    ../app/target/SpeakerReframePlanner.h \
    ../app/target/SpeakerTargetAssociator.h \
    ../app/target/SpeakerTimeline.h \
    ../app/target/SpeakerTypes.h \
    ../app/target/ProcessSpeakerProvider.h \
    ../app/target/TargetCropExtractor.h \
    ../app/target/TargetIdentity.h \
    ../app/target/TargetResolver.h \
    ../app/target/TargetSelector.h \
    ../app/target/TargetTrackPlanner.h \
    ../app/target/TargetTypes.h
