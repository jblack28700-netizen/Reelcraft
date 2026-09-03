QT += widgets concurrent
INCLUDEPATH += app
CONFIG += c++17
TEMPLATE = app
TARGET = reelcraft

SOURCES += \
    app/main.cpp \
    app/application/Application.cpp \
    app/core/Project.cpp \
    app/ui/MainWindow.cpp

HEADERS += \
    app/application/Application.h \
    app/core/Project.h \
    app/ui/MainWindow.h
