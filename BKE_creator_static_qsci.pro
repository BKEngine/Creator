#-------------------------------------------------
#
# Project created by QtCreator 2014-01-23T09:48:00
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): {
QT += widgets
}

TARGET = BKE_creator
TEMPLATE = app

CONFIG += warn_off qt
CONFIG+= c++11

DEFINES += QT SCI_LEXER
#if use in linux,you must use a full name
#LIBS += /usr/lib/i386-linux-gnu/libqscintilla2.a

#if use in windows
#LIBS += libqscintilla2

win{
RC_FILE += ico.rc
}

mac{
ICON = icon.icns
}

SOURCES += ./main.cpp \
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
    paper/creator_parser.cpp \
    paper/completebase.cpp \
    dia/searchbox.cpp \
    bkeSci/bkemarks.cpp \
    dia/bkeconfiguimodel.cpp \
    loli/loli_island.cpp \
    otherbasicwin.cpp \
    dia/bkeleftfilewidget.cpp \
    dia/qsearchlineedit.cpp \
    BKEscintilla/src/AutoComplete.cpp \
    BKEscintilla/src/CallTip.cpp \
    BKEscintilla/src/Catalogue.cpp \
    BKEscintilla/src/CellBuffer.cpp \
    BKEscintilla/src/CharClassify.cpp \
    BKEscintilla/src/ContractionState.cpp \
    BKEscintilla/src/Decoration.cpp \
    BKEscintilla/src/Document.cpp \
    BKEscintilla/src/Editor.cpp \
    BKEscintilla/src/ExternalLexer.cpp \
    BKEscintilla/src/Indicator.cpp \
    BKEscintilla/src/KeyMap.cpp \
    BKEscintilla/src/LineMarker.cpp \
    BKEscintilla/src/PerLine.cpp \
    BKEscintilla/src/PositionCache.cpp \
    BKEscintilla/src/RESearch.cpp \
    BKEscintilla/src/RunStyles.cpp \
    BKEscintilla/src/ScintillaBase.cpp \
    BKEscintilla/src/Selection.cpp \
    BKEscintilla/src/Style.cpp \
    BKEscintilla/src/UniConversion.cpp \
    BKEscintilla/src/ViewStyle.cpp \
    BKEscintilla/src/XPM.cpp \
    BKEscintilla/lexlib/Accessor.cpp \
    BKEscintilla/lexlib/LexerBase.cpp \
    BKEscintilla/lexlib/LexerModule.cpp \
    BKEscintilla/lexlib/LexerNoExceptions.cpp \
    BKEscintilla/lexlib/LexerSimple.cpp \
    BKEscintilla/lexlib/PropSetSimple.cpp \
    BKEscintilla/lexlib/StyleContext.cpp \
    BKEscintilla/lexlib/WordList.cpp \
    BKEscintilla/Qt4Qt5/ListBoxQt.cpp \
    BKEscintilla/Qt4Qt5/PlatQt.cpp \
    BKEscintilla/Qt4Qt5/qsciabstractapis.cpp \
    BKEscintilla/Qt4Qt5/qsciapis.cpp \
    BKEscintilla/Qt4Qt5/qscicommand.cpp \
    BKEscintilla/Qt4Qt5/qscicommandset.cpp \
    BKEscintilla/Qt4Qt5/qscidocument.cpp \
    BKEscintilla/Qt4Qt5/qscilexer.cpp \
    BKEscintilla/Qt4Qt5/qscimacro.cpp \
    BKEscintilla/Qt4Qt5/qsciscintilla.cpp \
    BKEscintilla/Qt4Qt5/qsciscintillabase.cpp \
    BKEscintilla/Qt4Qt5/qscistyle.cpp \
    BKEscintilla/Qt4Qt5/qscistyledtext.cpp \
    BKEscintilla/Qt4Qt5/SciClasses.cpp \
    BKEscintilla/Qt4Qt5/ScintillaQt.cpp \
    BKEscintilla/lexlib/CharacterSet.cpp \
    singleapplication.cpp \
    dia/cconfigdia.cpp \
    dia/ctextedit.cpp \
    bkeprojectconfig.cpp \
    BKS_info.cpp \
    BKEscintilla/lexers/BKE_Lexer.cpp \
    ParserHelper/ParserHelper.cpp \
    ParserHelper/parser/BKE_hash.cpp \
    ParserHelper/parser/BKE_number.cpp \
    ParserHelper/parser/BKE_string.cpp \
    ParserHelper/parser/BKE_variable.cpp \
    ParserHelper/parser/extend_wrapper.cpp \
    ParserHelper/parser/parser.cpp \
    ParserHelper/parser/parserextend_wrapper.cpp \
    ParserHelper/parser/utils.cpp \
    dia/cdiroption.cpp \
    dia/cskinoption.cpp \
    dia/Setoptiondia.cpp \
    dia/WaitWindow.cpp

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
    paper/creator_parser.h \
    paper/completebase.h \
    dia/searchbox.h \
    bkeSci/BkeIndicatorBase.h \
    bkeSci/bkemarks.h \
    dia/bkeconfiguimodel.h \
    loli/loli_island.h \
    otherbasicwin.h \
    dia/bkeleftfilewidget.h \
    dia/qsearchlineedit.h \
    BKEscintilla/src/AutoComplete.h \
    BKEscintilla/src/CallTip.h \
    BKEscintilla/src/Catalogue.h \
    BKEscintilla/src/CellBuffer.h \
    BKEscintilla/src/CharClassify.h \
    BKEscintilla/src/ContractionState.h \
    BKEscintilla/src/Decoration.h \
    BKEscintilla/src/Document.h \
    BKEscintilla/src/Editor.h \
    BKEscintilla/src/ExternalLexer.h \
    BKEscintilla/src/FontQuality.h \
    BKEscintilla/src/Indicator.h \
    BKEscintilla/src/KeyMap.h \
    BKEscintilla/src/LineMarker.h \
    BKEscintilla/src/Partitioning.h \
    BKEscintilla/src/PerLine.h \
    BKEscintilla/src/PositionCache.h \
    BKEscintilla/src/RESearch.h \
    BKEscintilla/src/RunStyles.h \
    BKEscintilla/src/ScintillaBase.h \
    BKEscintilla/src/Selection.h \
    BKEscintilla/src/SplitVector.h \
    BKEscintilla/src/Style.h \
    BKEscintilla/src/SVector.h \
    BKEscintilla/src/UniConversion.h \
    BKEscintilla/src/ViewStyle.h \
    BKEscintilla/src/XPM.h \
    BKEscintilla/lexlib/Accessor.h \
    BKEscintilla/lexlib/CharacterCategory.h \
    BKEscintilla/lexlib/CharacterSet.h \
    BKEscintilla/lexlib/LexAccessor.h \
    BKEscintilla/lexlib/LexerBase.h \
    BKEscintilla/lexlib/LexerModule.h \
    BKEscintilla/lexlib/LexerNoExceptions.h \
    BKEscintilla/lexlib/LexerSimple.h \
    BKEscintilla/lexlib/OptionSet.h \
    BKEscintilla/lexlib/PropSetSimple.h \
    BKEscintilla/lexlib/StyleContext.h \
    BKEscintilla/lexlib/SubStyles.h \
    BKEscintilla/lexlib/WordList.h \
    BKEscintilla/Qt4Qt5/bkestyle.h \
    BKEscintilla/Qt4Qt5/ListBoxQt.h \
    BKEscintilla/Qt4Qt5/SciClasses.h \
    BKEscintilla/Qt4Qt5/SciNamespace.h \
    BKEscintilla/Qt4Qt5/ScintillaQt.h \
    BKEscintilla/lexlib/SparseState.h \
    BKEscintilla/Qt4Qt5/Qsci/qsciabstractapis.h \
    BKEscintilla/Qt4Qt5/Qsci/qsciapis.h \
    BKEscintilla/Qt4Qt5/Qsci/qscicommand.h \
    BKEscintilla/Qt4Qt5/Qsci/qscicommandset.h \
    BKEscintilla/Qt4Qt5/Qsci/qscidocument.h \
    BKEscintilla/Qt4Qt5/Qsci/qsciglobal.h \
    BKEscintilla/Qt4Qt5/Qsci/qscilexer.h \
    BKEscintilla/Qt4Qt5/Qsci/qscimacro.h \
    BKEscintilla/Qt4Qt5/Qsci/qsciscintilla.h \
    BKEscintilla/Qt4Qt5/Qsci/qsciscintillabase.h \
    BKEscintilla/Qt4Qt5/Qsci/qscistyle.h \
    BKEscintilla/Qt4Qt5/Qsci/qscistyledtext.h \
    singleapplication.h \
    dia/cconfigdia.h \
    dia/ctextedit.h \
    bkeprojectconfig.h \
    ParserHelper/ParserHelper.h \
    ParserHelper/parser/BKE_array.h \
    ParserHelper/parser/BKE_hash.hpp \
    ParserHelper/parser/BKE_number.h \
    ParserHelper/parser/BKE_string.h \
    ParserHelper/parser/BKE_variable.h \
    ParserHelper/parser/defines.h \
    ParserHelper/parser/extend.h \
    ParserHelper/parser/memorypool.h \
    ParserHelper/parser/parser.h \
    ParserHelper/parser/utils.h \
    ParserHelper/parser/vcode.h \
    dia/cdiroption.h \
    dia/cskinoption.h \
    dia/Setoptiondia.h \
    dia/WaitWindow.h

RESOURCES += \
    source.qrc

INCLUDEPATH += ./BKEscintilla ./BKEscintilla/lexlib ./BKEscintilla/include ./BKEscintilla/src ./BKEscintilla/Qt4Qt5 ./

FORMS += \
    dia/ctextedit.ui \
    dia/cdiroption.ui \
    dia/cskinoption.ui \
    dia/Setoptiondia.ui \
    dia/WaitWindow.ui

mac{
    LIBS += -L$$PWD/quazip -lquazip
}
