#-------------------------------------------------
#
# Project created by QtCreator 2014-06-21T12:53:20
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = update
TEMPLATE = app

CONFIG+=c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    weh.cpp \
    loli/loli_island.cpp

HEADERS  += mainwindow.h \
    weh.h \
    loli/loli_island.h

FORMS    += mainwindow.ui
