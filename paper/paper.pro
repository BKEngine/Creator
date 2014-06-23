#-------------------------------------------------
#
# Project created by QtCreator 2014-03-11T19:29:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = paper
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11
TEMPLATE = app


SOURCES += main.cpp \
    wordsupport.cpp \
    parser.cpp \
    completebase.cpp \
    debugout.cpp

HEADERS += \
    wordsupport.h \
    parser.h \
    completebase.h \
    debugout.h
