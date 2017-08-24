#include <weh.h>
#include "bkecompile.h"
#include "../BKS_info.h"

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

BkeCompile::~BkeCompile()
{
	if (cmd)
	{
		delete cmd;
	}
}

void BkeCompile::CompileLang(const QString &dir, bool release/* = false*/)
{
	cmd = new QProcess();
	connect(cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(StandardOutput()));
	connect(cmd, SIGNAL(finished(int)), this, SLOT(finished(int)));
	connect(cmd, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
	cmd->setWorkingDirectory(dir);
	QString exeName = release ? "BKCompiler" : "BKCompiler_Dev";
	QString lang = QString::fromStdWString(global_bke_info.projsetting[L"lang"].getString(L"chs"));
	QString langopt = dir + "/" + BKE_PROJECT_NAME + ".user|langopt";
#ifdef Q_OS_WIN
	cmd->start(BKE_CURRENT_DIR + "/tool/" + exeName + ".exe", QStringList() << dir << "-nopause" << "-nocompile" << "-lang" << lang << "-langopt" << langopt);
#elif defined(Q_OS_MAC)
	/*QString str = "./BKCompiler_Dev '" + dir + "'-nopause";
	QFile file(BKE_CURRENT_DIR + "/run_bkc.sh");
	if(!file.open(QFile::WriteOnly))
	{
	QMessageBox::information(0, "Error", "Cannot write run_bkc.sh.");
	return;
	}
	file.write(str.toUtf8());
	file.close();
	cmd->start("open", QStringList() << "-a" << "Terminal.app sh" << BKE_CURRENT_DIR + "/run_bkc.sh");*/
	cmd->start(BKE_CURRENT_DIR + "/" + exeName, QStringList() << dir << "-nopause" << "-lang" << lang << "-langopt" << langopt);
#else
	cmd->start(BKE_CURRENT_DIR + "/tool/" + exeName, QStringList() << dir << "-nopause" << "-lang" << lang << "-langopt" << langopt);
#endif
}

void BkeCompile::Compile(const QString &dir, bool release/* = false*/)
{
	cmd = new QProcess();
	connect(cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(StandardOutput()));
	connect(cmd, SIGNAL(finished(int)), this, SLOT(finished(int)));
	connect(cmd, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
	cmd->setWorkingDirectory(dir);
	QString exeName = release ? "BKCompiler" : "BKCompiler_Dev";
#ifdef Q_OS_WIN
	cmd->start(BKE_CURRENT_DIR + "/tool/" + exeName + ".exe", QStringList() << dir << "-nopause" << "-nocompile");
#elif defined(Q_OS_MAC)
	/*QString str = "./BKCompiler_Dev '" + dir + "'-nopause";
	QFile file(BKE_CURRENT_DIR + "/run_bkc.sh");
	if(!file.open(QFile::WriteOnly))
	{
		QMessageBox::information(0, "Error", "Cannot write run_bkc.sh.");
		return;
	}
	file.write(str.toUtf8());
	file.close();
	cmd->start("open", QStringList() << "-a" << "Terminal.app sh" << BKE_CURRENT_DIR + "/run_bkc.sh");*/
	cmd->start(BKE_CURRENT_DIR + "/"+exeName, QStringList() << dir << "-nopause");
#else
	cmd->start(BKE_CURRENT_DIR + "/tool/"+exeName, QStringList() << dir << "-nopause");
#endif
}

void BkeCompile::StandardOutput()
{
	QByteArray temp = cmd->readAll();
#ifdef Q_OS_WIN
	QString name = QString::fromStdWString(std::wstring((wchar_t*)temp.data(), temp.size() / 2));
#else
    QString name = QString::fromUtf8(temp);
#endif
    result.append(temp);
#ifdef Q_OS_WIN
	if (name.endsWith(".bkscr") && list.indexOf(name) < 0) {
		list.append(name);
		emit NewFileReady(list.size());
	}
#else
    QStringList qs = name.split('\n');
    for(QString q : qs)
    {
        if (q.endsWith(".bkscr") && list.indexOf(q) < 0) {
            list.append(name);
            emit NewFileReady(list.size());
        }
    }
#endif
}

void BkeCompile::finished(int exitCode)
{
	emit CompliteFinish();
	cmd->deleteLater();
	cmd = NULL;
	list.clear();
	result.clear();
}

QString BkeCompile::Result()
{
#ifdef Q_OS_WIN
    return QString::fromStdWString(std::wstring((wchar_t*)result.data(), result.size() / 2)).remove('\r');
#else
    return QString::fromUtf8(result);
#endif
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
}
