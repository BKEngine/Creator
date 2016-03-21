#pragma once
#include <weh.h>
#include "qthread.h"
#include "ParseData.h"
#include <qmutex.h>
#include <atomic>
#include <list>
#include <algorithm>
#include "cmdlist_wrapper.h"
#include "ParserHelper/parser/parser.h"

using std::atomic_bool;

class BkeScintilla;

struct BKEMacros
{
	QString name;
	QString definefile;
	QString comment;
	bkplong pos;
	vector<pair<QString, QString>> paramqueue;
};

/// <summary>
/// Analysis Project Background
/// </summary>
class BG_Analysis :
	public QThread
{
	Q_OBJECT
public:
	BG_Analysis(const QString &p);
	~BG_Analysis();

	QString pdir;

	virtual void run() override;

private:
	volatile enum
	{
		STATE_IDLE,
		STATE_PARSEFILE,
		STATE_PARSEMACRO,
	}cur_state;

	BKE_VarClosure *backup_topclo;

	BKE_VarClosure *topclo;

	/// <summary>
	/// Global macro data.
	/// </summary>
	QHash<QString, BKEMacros> macrodata;

	/// <summary>
	/// The backup_macrodata, updated by sub thread
	/// </summary>
	QHash<QString, BKEMacros> backup_macrodata;

	/// <summary>
	/// The macrofiles.
	/// </summary>
	list<QString> macrofiles;

	/// <summary>
	/// The backup_macrofiles.
	/// </summary>
	list<QString> backup_macrofiles;

	/// <summary>
	/// Whether macro has changed.
	/// </summary>
	atomic_bool newmacrofile;

	/// <summary>
	/// Internal analysis data.
	/// </summary>
	QHash<QString, ParseData*> data;

	/// <summary>
	/// File Buffer
	/// </summary>
	QHash<QString, QByteArray> filebuf;

	/// <summary>
	/// File which is being analysised.
	/// </summary>
	QString curfile;

	/// <summary>
	/// Files to be analysised.
	/// </summary>
	QStringList listfile;

	/// <summary>
	/// The mutex to lock message.
	/// </summary>
	QMutex msgmutex;

	/// <summary>
	/// The mutex to lock analysis.
	/// </summary>
	QMutex analysis_mutex;
	
	/// <summary>
	/// Whether stop the thread.
	/// </summary>
	atomic_bool stop;

	/// <summary>
	/// Whether cancel the current analysising.
	/// </summary>
	atomic_bool cancel;

	/// <summary>
	/// Searches the full relativ name by its filename.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <returns></returns>
	QString searchFile(QString file)
	{
		if (file.isEmpty())
			return QString();
		if (file[0] == '\"' && file.length() > 1 && file[file.length() - 1] == '\"')
			file = file.mid(1, file.length() - 2);
		if (!file.endsWith(".bkscr"))
			file += ".bkscr";
		for (auto it = data.begin(); it != data.end(); it++)
		{
			if (it.key().endsWith(file))
				return it.key();
		}
		if (curfile.endsWith(file))
			return curfile;
		for (auto &it : listfile)
		{
			if (it.endsWith(file))
				return it;
		}
		return QString();
	}

	void parseMacro(const QString &file);
	void notifyExit();

public:
	/// <summary>
	/// Adds the file to analysis immediately.
	/// If the file is analysising, abort it and analysis immediately.
	/// </summary>
	/// <param name="file">The file.</param>
	void pushFile(const QString &file, const char *buffer = NULL)
	{
		msgmutex.lock();
		if (curfile == file)
		{
			cancel = true;
		}
		//deleted in child-thread
		//auto p = data[file];
		//delete p;
		//data[file] = NULL;
		listfile.push_front(file);
		if (buffer)
		{
			//update buffer
			filebuf[file] = buffer;
		}
		if (file == "macro.bkscr" || std::find(macrofiles.begin(), macrofiles.end(), file) != macrofiles.end())
		{
			newmacrofile = true;
			if (cur_state == STATE_PARSEMACRO)
				cancel = true;
		}
		msgmutex.unlock();
	}

	/// <summary>
	/// Adds files to analysis.
	/// If one file is analysising, abort it and analysis later.
	/// </summary>
	/// <param name="files">Files.</param>
	void addFiles(const QStringList &files)
	{
		msgmutex.lock();
		listfile.append(files);
		newmacrofile = true;
		msgmutex.unlock();
	}

	/// <summary>
	/// Inform a file inserts the chars.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <param name="p">The p.</param>
	/// <param name="offset">The offset.</param>
	void insertChars(const QString &file, Pos p, Pos offset);

	/// <summary>
	/// Inform a file deletes the chars.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <param name="p">The p.</param>
	/// <param name="offset">The offset.</param>
	void deleteChars(const QString &file, Pos p, Pos offset);

	/// <summary>
	/// Finds the label.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <param name="label">The label.</param>
	/// <returns>pos</returns>
	int findLabel(const QString &file, const QString &label)
	{
		int n = -1;
		msgmutex.lock();
		auto d = data[file];
		if (d)
		{
			n = d->findLabel(label);
		}
		msgmutex.unlock();
		return n;
	}

	/// <summary>
	/// Finds the node.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <param name="p">position.</param>
	/// <returns>Nodes finded.</returns>
	BaseNode *findNode(const QString &file, int p)
	{
		BaseNode *n = NULL;
		msgmutex.lock();
		auto d = data[file];
		if (d)
		{
			n = d->findNode(p);
		}
		msgmutex.unlock();
		return n;
	}

	/// <summary>
	/// Gets the labels.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <param name="l">Lables.</param>
	/// <returns>whether success</returns>
	bool getLabels(const QString &file, set<QString> &l)
	{
		bool res = false;
		msgmutex.lock();
		auto d = data[file];
		if (d)
		{
			res = true;
			d->getLabels(l);
		}
		msgmutex.unlock();
		return res;
	}

	/// <summary>
	/// whether the file is OK, if OK, lockFile.
	/// </summary>
	/// <param name="file">The file.</param>
	/// <returns></returns>
	ParseData* lockFile(const QString &file)
	{
		msgmutex.lock();
		ParseData *res = data[file];
		return res;
	}

	/// <summary>
	/// Unlocks the file.
	/// </summary>
	/// <param name="file">The file.</param>
	void unlockFile()
	{
		msgmutex.unlock();
	}

	/// <summary>
	/// Gets all the commands and macros.
	/// </summary>
	/// <returns></returns>
	QString getCmdList()
	{
		std::set<QString> auto_list;
		for (auto &it : CmdList)
		{
			auto_list.insert(it.name + "?0");
		}
		for (auto it = SpecialCmdList.begin(); it != SpecialCmdList.end(); it++)
		{
			auto_list.insert(it.key() + "?0");
		}
		msgmutex.lock();
		for (auto it = macrodata.begin(); it != macrodata.end(); it++)
		{
			auto_list.insert(it.key() + "?3");
		}
		msgmutex.unlock();
		QString cmd;
		auto it = auto_list.begin();
		cmd = *it;
		for (++it; it != auto_list.end(); it++)
		{
			cmd += " " + *it;
		}
		return cmd;
	}

	bool findMacro(const QString &name, BKEMacros *m)
	{
		msgmutex.lock();
		auto it = macrodata.find(name);
		if (it == macrodata.end())
		{
			msgmutex.unlock();
			return false;
		}
		*m = it.value();
		msgmutex.unlock();
		return true;
	}
};

