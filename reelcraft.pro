QT += widgets concurrent
INCLUDEPATH += app
CONFIG += c++17
TEMPLATE = app
TARGET = reelcraft

SOURCES += \
    app/main.cpp \
    app/application/Application.cpp \
    app/core/MediaItem.cpp \
    app/core/Project.cpp \
    app/media/FfmpegFrameSource.cpp \
    app/media/FrameExtractor.cpp \
    app/media/FramePump.cpp \
    app/media/FrameSource.cpp \
    app/playback/DefaultPacingPolicy.cpp \
    app/playback/Playhead.cpp \
    app/playback/Player.cpp \
    app/playback/SystemClock.cpp \
    app/viewer/EquirectView.cpp \
    app/viewer/ViewerProjection.cpp \
    app/viewer/ViewerScene.cpp \
    app/viewer/ViewportState.cpp \
    app/ui/MainWindow.cpp \
    app/ui/ViewerWidget.cpp \
    app/reframe/CameraKeyframe.cpp \
    app/reframe/CameraPath.cpp \
    app/reframe/FfmpegSeekFrameProvider.cpp \
    app/reframe/ReframeIntent.cpp \
    app/reframe/ReframePlan.cpp \
    app/reframe/ReframePlanBuilder.cpp \
    app/reframe/ReframePipeline.cpp \
    app/reframe/ReframeRenderer.cpp \
    app/target/EquirectProjection.cpp \
    app/target/EquirectViewPlan.cpp \
    app/target/ProcessTargetDetector.cpp \
    app/target/SphericalTargetTracker.cpp \
    app/target/TargetIdentity.cpp \
    app/target/TargetResolver.cpp \
    app/target/TargetSelector.cpp \
    app/target/TargetTrackPlanner.cpp \
    app/target/TargetTypes.cpp

HEADERS += \
    app/application/Application.h \
    app/core/MediaItem.h \
    app/core/Project.h \
    app/media/FfmpegFrameSource.h \
    app/media/FrameExtractor.h \
    app/media/FramePump.h \
    app/media/FrameSource.h \
    app/playback/Clock.h \
    app/playback/DefaultPacingPolicy.h \
    app/playback/PacingPolicy.h \
    app/playback/Playhead.h \
    app/playback/Player.h \
    app/playback/SystemClock.h \
    app/viewer/EquirectView.h \
    app/viewer/ViewerProjection.h \
    app/viewer/ViewerScene.h \
    app/viewer/ViewportState.h \
    app/ui/MainWindow.h \
    app/ui/ViewerWidget.h \
    app/reframe/CameraKeyframe.h \
    app/reframe/CameraPath.h \
    app/reframe/FfmpegSeekFrameProvider.h \
    app/reframe/ReframeFrameProvider.h \
    app/reframe/ReframeIntent.h \
    app/reframe/ReframeMath.h \
    app/reframe/ReframePlan.h \
    app/reframe/ReframePlanBuilder.h \
    app/reframe/ReframePipeline.h \
    app/reframe/ReframeRenderer.h \
    app/target/EquirectProjection.h \
    app/target/EquirectViewPlan.h \
    app/target/ProcessTargetDetector.h \
    app/target/SphericalTargetTracker.h \
    app/target/TargetDetector.h \
    app/target/TargetIdentity.h \
    app/target/TargetResolver.h \
    app/target/TargetSelector.h \
    app/target/TargetTrackPlanner.h \
    app/target/TargetTypes.h
