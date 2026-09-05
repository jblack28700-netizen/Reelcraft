QT += testlib core concurrent widgets
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = reelcraft_tests
INCLUDEPATH += ../app

SOURCES += \
    test_project.cpp \
    ../app/core/MediaItem.cpp \
    ../app/core/Project.cpp \
    ../app/application/Application.cpp \
    ../app/viewer/EquirectView.cpp \
    ../app/viewer/ViewerProjection.cpp \
    ../app/viewer/ViewerScene.cpp \
    ../app/viewer/ViewportState.cpp \
    ../app/ui/MainWindow.cpp \
    ../app/ui/ViewerWidget.cpp

HEADERS += \
    ../app/core/MediaItem.h \
    ../app/core/Project.h \
    ../app/application/Application.h \
    ../app/viewer/EquirectView.h \
    ../app/viewer/ViewerProjection.h \
    ../app/viewer/ViewerScene.h \
    ../app/viewer/ViewportState.h \
    ../app/ui/MainWindow.h \
    ../app/ui/ViewerWidget.h
