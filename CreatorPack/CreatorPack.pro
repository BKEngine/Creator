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

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    weh.cpp \
    loli/loli_island.cpp \
    ../qt-qtftp/src/qftp/qftp.cpp \
    ../qt-qtftp/src/qftp/qurlinfo.cpp

HEADERS  += mainwindow.h \
    weh.h \
    loli/loli_island.h \
    ../qt-qtftp/src/qftp/qftp.h \
    ../qt-qtftp/src/qftp/qurlinfo.h \
    bkefunctions.h

INCLUDEPATH += ./QtFtp

LIBS += -lquazip -lz #-L$${PWD}/QtFtp -lQt5Ftp

FORMS    += mainwindow.ui
