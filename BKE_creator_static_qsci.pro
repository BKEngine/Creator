#-------------------------------------------------
#
# Project created by QtCreator 2014-01-23T09:48:00
#
#-------------------------------------------------

QT       += core gui
QT += network websockets

greaterThan(QT_MAJOR_VERSION, 4): {
QT += widgets
}

TARGET = BKE_Creator
TEMPLATE = app

CONFIG += warn_off qt
CONFIG+= c++14

DEFINES += QT SCI_LEXER SCINTILLA_QT BKE_CREATOR
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
    dia/searchbox.cpp \
    bkeSci/bkemarks.cpp \
    dia/bkeconfigdialog.cpp \
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
    dia/cconfigdia.cpp \
    dia/ctextedit.cpp \
    bkeprojectconfig.cpp \
    BKS_info.cpp \
    BKEscintilla/lexers/BKE_Lexer.cpp \
    ParserHelper/ParserHelper.cpp \
    ParserHelper/parser/BKE_number.cpp \
    ParserHelper/parser/BKE_string.cpp \
    ParserHelper/parser/BKE_variable.cpp \
    ParserHelper/parser/parser.cpp \
    ParserHelper/parser/utils.cpp \
    dia/cdiroption.cpp \
    dia/cskinoption.cpp \
    dia/Setoptiondia.cpp \
    dia/WaitWindow.cpp \
    dia/versioninfo.cpp \
    ParserHelper/parser/bkutf8.cpp \
    ParserHelper/parser/extend.cpp \
    ParserHelper/parser/parserextend.cpp \
    BG_Analysis.cpp \
    ParseData.cpp \
    dia/GameProperty.cpp \
    dia/LangOpt.cpp \
    dia/doubleinput.cpp \
    BKEscintilla/Qt4Qt5/InputMethod.cpp \
    BKEscintilla/lexlib/CharacterCategory.cpp \
    BKEscintilla/src/CaseConvert.cpp \
    BKEscintilla/src/CaseFolder.cpp \
    BKEscintilla/src/EditModel.cpp \
    BKEscintilla/src/EditView.cpp \
    BKEscintilla/src/MarginView.cpp \
    BKEscintilla/Qt4Qt5/MacPasteboardMime.cpp \
    dia/ParserEditor.cpp \
    dia/ParserEditorTreeItem.cpp \
    dia/ParserEditorTreeModel.cpp \
    CmdListLoader.cpp \
    dia/openlabeldialog.cpp \
    QFuzzyMatcher/QFuzzyMatcher.cpp \
    QFuzzyMatcher/score_match.cpp \
    QPinyin/ChineseToPinyinResource.cpp \
    QPinyin/HanyuPinyinOutputFormat.cpp \
    QPinyin/PinyinFormatter.cpp \
    QPinyin/PinyinHelper.cpp \
    QPinyin/QPinyin.cpp \
    QPinyin/ResourceHelper.cpp \
    dia/qnofocusitemdelegate.cpp \
    dia/gotofiledialog.cpp \
    qmacopenfileapplication.cpp \
    dia/autocompletelist.cpp \
    Debugger/DebugServer.cpp \
    Debugger/BreakpointManager.cpp \
    dia/ParserEditorUndoCommand.cpp \
    TinyProcess\process.cpp \
    TinyProcess\process_unix.cpp \
    TinyProcess\process_win.cpp \
    dia/bkespriteviewer.cpp

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
    dia/searchbox.h \
    bkeSci/BkeIndicatorBase.h \
    bkeSci/bkemarks.h \
    dia/bkeconfigdialog.h \
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
    dia/WaitWindow.h \
    dia/versioninfo.h \
    dia/doubleinput.h \
    dia/GameProperty.h \
    dia/LangOpt.h \
    BG_Analysis.h \
    BKS_info.h \
    ParseData.h \
    BKEscintilla/lexlib/StringCopy.h \
    BKEscintilla/src/CaseConvert.h \
    BKEscintilla/src/CaseFolder.h \
    BKEscintilla/src/EditModel.h \
    BKEscintilla/src/EditView.h \
    BKEscintilla/src/MarginView.h \
    BKEscintilla/src/UnicodeFromUTF8.h \
    dia/ParserEditor.h \
    dia/ParserEditorTreeItem.h \
    dia/ParserEditorTreeModel.h \
    BKECmdList.h \
    CmdListLoader.h \
    dia/openlabeldialog.h \
    QFuzzyMatcher/QFuzzyMatcher.h \
    QFuzzyMatcher/score_match.h \
    QPinyin/ChineseToPinyinResource.h \
    QPinyin/HanyuPinyinCaseType.h \
    QPinyin/HanyuPinyinOutputFormat.h \
    QPinyin/HanyuPinyinToneType.h \
    QPinyin/HanyuPinyinVCharType.h \
    QPinyin/pinyin4cpp_global.h \
    QPinyin/PinyinFormatter.h \
    QPinyin/PinyinHelper.h \
    QPinyin/QPinyin.h \
    QPinyin/ResourceHelper.h \
    dia/qnofocusitemdelegate.h \
    dia/gotofiledialog.h \
    qmacopenfileapplication.h \
    dia/autocompletelist.h \
    DebugServer.h \
    dia/ParserEditorUndoCommand.h \
    TinyProcess\process.hpp \
    dia/bkespriteviewer.h

RESOURCES += \
    source/source.qrc \
    QPinyin/res.qrc

INCLUDEPATH += ./BKEscintilla ./BKEscintilla/lexlib ./BKEscintilla/include ./BKEscintilla/src ./BKEscintilla/Qt4Qt5 ./

FORMS += \
    dia/ctextedit.ui \
    dia/cdiroption.ui \
    dia/cskinoption.ui \
    dia/Setoptiondia.ui \
    dia/WaitWindow.ui \
    dia/versioninfo.ui \
    dia/wizard/qbkewizard.ui \
    dia/bkeconfigdialog.ui \
    dia/LangOpt.ui \
    dia/GameProperty.ui \
    dia/doubleinput.ui \
    dia/ParserEditor.ui \
    dia/openlabeldialog.ui \
    dia/gotofiledialog.ui \
    dia/autocompletelist.ui \
    dia/bkespriteviewer.ui

mac{
    QMAKE_INFO_PLIST = info-mac.plist
    QT += macextras
    LIBS += -L$$PWD/quazip -lquazip
    CONFIG(release, debug|release){
        QMAKE_LFLAGS += -dead_strip
    }
}

unix:!mac:{
    QMAKE_LFLAGS_RPATH=
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/../lib\',--gc-sections,--icf=safe"
    QMAKE_CFLAGS += -fdata-sections -ffunction-sections
}
linux{

}
unix:CONFIG(release, debug|release):{
    QMAKE_CFLAGS += -fvisibility=hidden
    QMAKE_LFLAGS += -s
}

DISTFILES += \
    ico.rc \
    bkeico.ico
