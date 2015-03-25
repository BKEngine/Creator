﻿#include <weh.h>
#include "bkecompile.h"

BkeCompile::BkeCompile(QObject *parent) :
QObject(parent)
{
#ifdef Q_OS_WIN
	codec = QTextCodec::codecForName("GBK");
#else
    codec = QTextCodec::codecForName("UTF-8");
#endif
    cmd = NULL;
}

void BkeCompile::Compile(const QString dir)
{
	result.clear();
	if (cmd)
		delete cmd;
	cmd = new QProcess(this);
	connect(cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(StandardOutput()));
	connect(cmd, SIGNAL(finished(int)), this, SLOT(finished(int)));
	connect(cmd, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
	list.clear();
#ifdef Q_OS_WIN
    cmd->start(BKE_CURRENT_DIR + "/tool/BKCompiler_Dev.exe", QStringList() << dir << "-nopause");
#elif defined(Q_OS_MAC)
    QString str = "./BKCompiler_Dev '" + dir + "'-nopause";
    QFile file(BKE_CURRENT_DIR + "/run_bkc.sh");
    if(!file.open(QFile::WriteOnly))
    {
        QMessageBox::information(0, "Error", "Cannot write run_bkc.sh.");
        return;
    }
    file.write(str.toUtf8());
    file.close();
    //cmd->start("open", QStringList() << "-a" << "Terminal.app sh" << BKE_CURRENT_DIR + "/run_bkc.sh");
    cmd->start(BKE_CURRENT_DIR + "/BKCompiler_Dev", QStringList() << dir << "-nopause");
#else
    cmd->start(BKE_CURRENT_DIR + "/tool/BKCompiler_Dev", QStringList() << dir << "-nopause");
#endif
}

void BkeCompile::StandardOutput()
{
	QByteArray temp = cmd->readAll();
	result.append(temp);
	QString name = codec->toUnicode(temp);
	if (name.endsWith(".bkscr") && list.indexOf(name) < 0) {
		list.append(name);
		emit NewFileReady(list.size());
	}
}

void BkeCompile::finished(int exitCode)
{
	emit CompliteFinish();
	//delete cmd ;
	//cmd = 0 ;
}

QString BkeCompile::Result()
{
	text.clear();
	text = codec->toUnicode(result);
	return text;
}

void BkeCompile::error(QProcess::ProcessError e)
{
	QString s;
	switch (e) {
	case QProcess::FailedToStart:
		s = "无法启动";
		break;
	case QProcess::Crashed:
		s = "崩溃";
		break;
	case QProcess::Timedout:
		s = "超时";
		break;
	case QProcess::ReadError:
		s = "读取错误";
		break;
	case QProcess::WriteError:
		s = "写入错误";
		break;
	case QProcess::UnknownError:
		s = "未知错误";
	default:
		break;
	}
	emit CompliteError(s);
	delete cmd;
	cmd = 0;
}
