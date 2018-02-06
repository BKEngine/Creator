#ifndef WEH_H
#define WEH_H

//#include <vld.h>

#include <QWidget>
#include <QLabel>
#include <Qsci/qsciscintilla.h>
#include <QStackedWidget>
#include <QVector>
#include <QList>
#include <QFile>
#include <QMessageBox>
#include <QComboBox>
#include <QListWidget>
#include <QWidgetAction>
#include <QToolBar>
#include <QSplitter>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QDialog>
#include <QLineEdit>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QCheckBox>
#include <QProcess>
#include <QtCore>
#include <QRegExp>
#include <QHelpEvent>
#include "function.h"

#include "ParserHelper/ParserHelper.h"
#include "ParserHelper/Bagel/Bagel_Include.h"

#if _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#define SAVE_VERSION 12

class CodeWindow;
class OtherWindow;
class ProjectWindow;
class BkeLeftFileWidget;

extern int list_colors[13];
extern QSplitter *ras[3];
extern CodeWindow *codeedit;
extern OtherWindow *otheredit;
extern ProjectWindow *projectedit;
extern BkeLeftFileWidget *fileListWidget;

typedef unsigned char u8;

#endif // WEH_H
