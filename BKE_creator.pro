#-------------------------------------------------
#
# Project created by QtCreator 2014-01-23T09:48:00
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BKE_creator
TEMPLATE = app

CONFIG += warn_off
CONFIG+= c++11
#if use in linux,you must use a full name
linux{
    LIBS += /home/icemic/Dev/Qt5.3.2_x64/5.3/gcc_64/lib/libqscintilla2.a
}

#if use in windows
win32 {
    LIBS += libqscintilla2
}

RC_FILE += ico.rc

SOURCES += main.cpp \
    topbarwindow.cpp \
    projectwindow.cpp \
    codewindow.cpp \
    mainwindow.cpp \
    otherwindow.cpp \
    bkeproject.cpp \
    dia/newprodia.cpp \
    bkeSci/bkescintilla.cpp \
    bkeSci/qscilexerbkescript.cpp \
    dia/lablesuredialog.cpp \
    bkeSci/bkecompile.cpp \
    function.cpp \
    paper/wordsupport.cpp \
    paper/parser.cpp \
    paper/completebase.cpp \
    dia/searchbox.cpp \
    bkeSci/bkemarks.cpp \
    dia/bkeconfiguimodel.cpp \
    loli/loli_island.cpp \
    otherbasicwin.cpp \
    dia/bkeleftfilewidget.cpp \
    dia/qsearchlineedit.cpp \
    singleapplication.cpp \
    dia/cconfigdia.cpp \
    dia/ctextedit.cpp \
    bkeprojectconfig.cpp

HEADERS  += \
    topbarwindow.h \
    projectwindow.h \
    codewindow.h \
    mainwindow.h \
    otherwindow.h \
    weh.h \
    bkeproject.h \
    dia/newprodia.h \
    bkeSci/bkescintilla.h \
    bkeSci/qscilexerbkescript.h \
    dia/lablesuredialog.h \
    bkeSci/bkecompile.h \
    function.h \
    function.h \
    paper/wordsupport.h \
    paper/parser.h \
    paper/completebase.h \
    dia/searchbox.h \
    bkeSci/BkeIndicatorBase.h \
    bkeSci/bkemarks.h \
    dia/bkeconfiguimodel.h \
    loli/loli_island.h \
    otherbasicwin.h \
    dia/bkeleftfilewidget.h \
    dia/qsearchlineedit.h \
    singleapplication.h \
    dia/cconfigdia.h \
    dia/ctextedit.h \
    bkeprojectconfig.h

RESOURCES += \
    source.qrc

FORMS += \
    dia/ctextedit.ui

LIBS += -L$$PWD/ParserHelper -lParserHelper
