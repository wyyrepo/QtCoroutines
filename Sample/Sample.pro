QT += concurrent
QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
		main.cpp

include(../qtcoroutines.pri)
