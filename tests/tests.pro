QT += testlib core concurrent
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = reelcraft_tests
INCLUDEPATH += ../app

SOURCES += \
    test_project.cpp \
    ../app/core/Project.cpp \
    ../app/application/Application.cpp

HEADERS += \
    ../app/core/Project.h \
    ../app/application/Application.h
