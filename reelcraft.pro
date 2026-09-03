QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = reelcraft

SOURCES += \
    app/main.cpp \
    app/application/Application.cpp \
    app/ui/MainWindow.cpp

HEADERS += \
    app/application/Application.h \
    app/ui/MainWindow.h
