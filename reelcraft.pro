QT += widgets concurrent
INCLUDEPATH += app
CONFIG += c++17
TEMPLATE = app
TARGET = reelcraft

SOURCES += \
    app/main.cpp \
    app/application/Application.cpp \
    app/core/Project.cpp \
    app/viewer/ViewerScene.cpp \
    app/viewer/ViewportState.cpp \
    app/ui/MainWindow.cpp \
    app/ui/ViewerWidget.cpp

HEADERS += \
    app/application/Application.h \
    app/core/Project.h \
    app/viewer/ViewerScene.h \
    app/viewer/ViewportState.h \
    app/ui/MainWindow.h \
    app/ui/ViewerWidget.h
