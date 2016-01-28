#-------------------------------------------------
#
# Project created by QtCreator 2015-03-05T19:06:29
#
#-------------------------------------------------

QT       -= gui

TARGET = ParserHelper
TEMPLATE = lib
CONFIG += staticlib

SOURCES += parserhelper.cpp \
    parser/BKE_hash.cpp \
    parser/BKE_number.cpp \
    parser/BKE_string.cpp \
    parser/BKE_variable.cpp \
    parser/extend.cpp \
    parser/parser.cpp \
    parser/utils.cpp

HEADERS += parserhelper.h \
    parser/BKE_array.h \
    parser/BKE_hash.hpp \
    parser/BKE_number.h \
    parser/BKE_string.h \
    parser/BKE_variable.h \
    parser/defines.h \
    parser/extend.h \
    parser/memorypool.h \
    parser/parser.h \
    parser/utils.h \
    parser/vcode.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
