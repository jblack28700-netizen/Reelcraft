QT += testlib core concurrent widgets
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = reelcraft_tests
INCLUDEPATH += ../app

SOURCES += \
    test_project.cpp \
    ../app/core/Project.cpp \
    ../app/application/Application.cpp \
    ../app/ui/MainWindow.cpp

HEADERS += \
    ../app/core/Project.h \
    ../app/application/Application.h \
    ../app/ui/MainWindow.h
