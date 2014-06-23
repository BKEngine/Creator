# The project file for the QScintilla library.
#
# Copyright (c) 2012 Riverbank Computing Limited <info@riverbankcomputing.com>
# 
# This file is part of QScintilla.
# 
# This file may be used under the terms of the GNU General Public
# License versions 2.0 or 3.0 as published by the Free Software
# Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
# included in the packaging of this file.  Alternatively you may (at
# your option) use any later version of the GNU General Public
# License if such license has been publicly approved by Riverbank
# Computing Limited (or its successors, if any) and the KDE Free Qt
# Foundation. In addition, as a special exception, Riverbank gives you
# certain additional rights. These rights are described in the Riverbank
# GPL Exception version 1.1, which can be found in the file
# GPL_EXCEPTION.txt in this package.
# 
# If you are unsure which license is appropriate for your use, please
# contact the sales department at sales@riverbankcomputing.com.
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


# This must be kept in sync with configure.py, Qt4Qt5/application.pro and
# Qt4Qt5/designer.pro.
!win32:VERSION = 9.0.2

TEMPLATE = lib
TARGET = qscintilla2
CONFIG += qt warn_off release staticlib thread
#CONFIG += warn_off
INCLUDEPATH = . ../include ../lexlib ../src
DEFINES = QT SCI_LEXER

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	QT += printsupport
}

# Comment this in if you want the internal Scintilla classes to be placed in a
# Scintilla namespace rather than pollute the global namespace.
#DEFINES += SCI_NAMESPACE

# Handle both Qt v4 and v3.
target.path = $$[QT_INSTALL_LIBS]
isEmpty(target.path) {
	target.path = $(QTDIR)/lib
}

header.path = $$[QT_INSTALL_HEADERS]
header.files = Qsci
isEmpty(header.path) {
	header.path = $(QTDIR)/include/Qsci
	header.files = Qsci/qsci*.h
}

trans.path = $$[QT_INSTALL_TRANSLATIONS]
trans.files = qscintilla_*.qm
isEmpty(trans.path) {
	trans.path = $(QTDIR)/translations
}

qsci.path = $$[QT_INSTALL_DATA]
qsci.files = ../qsci
isEmpty(qsci.path) {
	qsci.path = $(QTDIR)
}

INSTALLS += header trans qsci target

HEADERS = \
	./Qsci/qsciglobal.h \
	./Qsci/qsciscintilla.h \
	./Qsci/qsciscintillabase.h \
	./Qsci/qsciabstractapis.h \
	./Qsci/qsciapis.h \
	./Qsci/qscicommand.h \
	./Qsci/qscicommandset.h \
	./Qsci/qscidocument.h \
	./Qsci/qscilexer.h \
	./Qsci/qscimacro.h \
	./Qsci/qsciprinter.h \
	./Qsci/qscistyle.h \
	./Qsci/qscistyledtext.h \
	ListBoxQt.h \
	SciClasses.h \
	SciNamespace.h \
	ScintillaQt.h \
	../include/ILexer.h \
	../include/Platform.h \
	../include/SciLexer.h \
	../include/Scintilla.h \
	../include/ScintillaWidget.h \
	../lexlib/Accessor.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexAccessor.h \
	../lexlib/LexerBase.h \
	../lexlib/LexerModule.h \
	../lexlib/LexerNoExceptions.h \
	../lexlib/LexerSimple.h \
	../lexlib/OptionSet.h \
	../lexlib/PropSetSimple.h \
	../lexlib/StyleContext.h \
	../lexlib/WordList.h \
	../src/AutoComplete.h \
	../src/CallTip.h \
	../src/Catalogue.h \
	../src/CellBuffer.h \
	../src/CharClassify.h \
	../src/ContractionState.h \
	../src/Decoration.h \
	../src/Document.h \
	../src/Editor.h \
	../src/ExternalLexer.h \
	../src/FontQuality.h \
	../src/Indicator.h \
	../src/KeyMap.h \
	../src/LineMarker.h \
	../src/Partitioning.h \
	../src/PerLine.h \
	../src/PositionCache.h \
	../src/RESearch.h \
	../src/RunStyles.h \
	../src/ScintillaBase.h \
	../src/Selection.h \
	../src/SplitVector.h \
	../src/Style.h \
	../src/SVector.h \
	../src/UniConversion.h \
	../src/ViewStyle.h \
	../src/XPM.h \

SOURCES = \
	qsciscintilla.cpp \
	qsciscintillabase.cpp \
	qsciabstractapis.cpp \
	qsciapis.cpp \
	qscicommand.cpp \
	qscicommandset.cpp \
	qscidocument.cpp \
	qscilexer.cpp \
	qscimacro.cpp \
	qsciprinter.cpp \
	qscistyle.cpp \
	qscistyledtext.cpp \
	SciClasses.cpp \
	ListBoxQt.cpp \
	PlatQt.cpp \
	ScintillaQt.cpp \
	../lexlib/Accessor.cpp \
	../lexlib/CharacterSet.cpp \
	../lexlib/LexerBase.cpp \
	../lexlib/LexerModule.cpp \
	../lexlib/LexerNoExceptions.cpp \
	../lexlib/LexerSimple.cpp \
	../lexlib/PropSetSimple.cpp \
	../lexlib/StyleContext.cpp \
	../lexlib/WordList.cpp \
	../src/AutoComplete.cpp \
	../src/CallTip.cpp \
	../src/Catalogue.cpp \
	../src/CellBuffer.cpp \
	../src/CharClassify.cpp \
	../src/ContractionState.cpp \
	../src/Decoration.cpp \
	../src/Document.cpp \
	../src/Editor.cpp \
	../src/ExternalLexer.cpp \
	../src/Indicator.cpp \
    ../src/KeyMap.cpp \
	../src/LineMarker.cpp \
	../src/PerLine.cpp \
	../src/PositionCache.cpp \
    ../src/RESearch.cpp \
	../src/RunStyles.cpp \
    ../src/ScintillaBase.cpp \
    ../src/Selection.cpp \
	../src/Style.cpp \
	../src/UniConversion.cpp \
	../src/ViewStyle.cpp \
	../src/XPM.cpp \
    ../lexers/LexBke.cpp

TRANSLATIONS = \
	qscintilla_cs.ts \
	qscintilla_de.ts \
	qscintilla_es.ts \
	qscintilla_fr.ts \
	qscintilla_pt_br.ts \
	qscintilla_ru.ts
