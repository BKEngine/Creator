﻿#include <weh.h>
#include "bkeprojectconfig.h"
#include "loli/loli_island.h"

BkeProjectConfig::BkeProjectConfig()
{
	resolutionSize[0] = 800;
	resolutionSize[1] = 600;
	maxSaveDataNum = 10000;
	saveDir = "savedata";
	defaultFontSize = 24;
	defaultFontColor = QBkeVariable(0xFFFFFF);
	debugLevel = 3;
	metaData = QBkeVariable::dic();
}

BkeProjectConfig::BkeProjectConfig(const QString &projDir, const QString &path)
	:BkeProjectConfig()
{
	filePath = path;
	this->projDir = projDir;
	watcher.addPath(path);
	connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged()));
}

void BkeProjectConfig::setupWatcher()
{
	watcher.addPath(filePath);
}

void BkeProjectConfig::fileChanged()
{
	readFile();
	emit onFileChanged();
	emit onConfigChanged();
}

BkeProjectConfig &BkeProjectConfig::operator =(const BkeProjectConfig &r)
{
	projectName = r.projectName;
	resolutionSize[0] = r.resolutionSize[0];
	resolutionSize[1] = r.resolutionSize[1];
	gameTitle = r.gameTitle;
	saveDir = r.saveDir;
	imageAutoSearchPath = r.imageAutoSearchPath;
	audioAutoSearchPath = r.audioAutoSearchPath;
	scriptAutoSearchPath = r.scriptAutoSearchPath;
	maxSaveDataNum = r.maxSaveDataNum;
	defaultFontColor = r.defaultFontColor;
	defaultFontName = r.defaultFontName;
	defaultFontSize = r.defaultFontSize;
	debugLevel = r.debugLevel;
	live2DKey = r.live2DKey;
	metaData = r.metaData;
	return *this;
}

void BkeProjectConfig::readFile()
{
	QString text;
	LOLI::AutoRead(text, filePath);
	if (text.isEmpty())
		emit readFileError("打开config.bkpsr文件失败！");
	try
	{
		QBkeVariable v = QBkeVariable::closureDicFromString(text);
		projectName = v["GameName"].value().toString();
		resolutionSize[0] = v["ResolutionSize"][0].value().toInt();
		resolutionSize[1] = v["ResolutionSize"][1].value().toInt();
		gameTitle = v["GameTitle"].value().toString();
		saveDir = v["SaveDir"].value().toString();
		{
			imageAutoSearchPath.clear();
			if (!v["ImageAutoSearchPath"].isNull())
			{
				for (int i = 0; i < v["ImageAutoSearchPath"].getCount(); i++)
				{
					imageAutoSearchPath.append(v["ImageAutoSearchPath"][i].value().toString());
				}
			}
		}
		{
			audioAutoSearchPath.clear();
			if (!v["AudioAutoSearchPath"].isNull())
			{
				for (int i = 0; i < v["AudioAutoSearchPath"].getCount(); i++)
				{
					audioAutoSearchPath.append(v["AudioAutoSearchPath"][i].value().toString());
				}
			}
		}
		{
			scriptAutoSearchPath.clear();
			if (!v["ScriptAutoSearchPath"].isNull())
			{
				for (int i = 0; i < v["ScriptAutoSearchPath"].getCount(); i++)
				{
					scriptAutoSearchPath.append(v["ScriptAutoSearchPath"][i].value().toString());
				}
			}
		}
		maxSaveDataNum = v["MaxSaveDataNum"].isNull() ? 10000 : v["MaxSaveDataNum"].value().toInt();
		defaultFontSize = v["DefaultFontSize"].isNull() ? 24 : v["DefaultFontSize"].value().toInt();
		defaultFontColor = v["DefaultFontColor"];
		defaultFontName = v["DefaultFontName"].value().toString();
		debugLevel = v["DebugLevel"].isNull() ? 3 : v["DebugLevel"].value().toInt();
		live2DKey = v["Live2DKey"].value().toString();
		metaData = v["MetaData"];
	}
	catch (QBkeVarExcept &e)
	{
		emit readFileError("读取config.bkpsr文件失败：" + e.getMsg());
	}
}

void BkeProjectConfig::writeFile()
{
	watcher.removePath(this->filePath);
	QTimer::singleShot(0, this, SLOT(setupWatcher()));
	emit onConfigChanged();
	QStringList results;
	results.append("//Auto Generated by BKE_Creator.\n//Don't modify this file if not necessery.\n");
	results.append("GameName = \"" + projectName + "\";");
	results.append(QString("ResolutionSize = [%1, %2];").arg(resolutionSize[0]).arg(resolutionSize[1]));
	if (!gameTitle.isEmpty())
		results.append("GameTitle = \"" + gameTitle + "\";");
	if (!saveDir.isEmpty())
		results.append("SaveDir = \"" + saveDir + "\";");
	if (!imageAutoSearchPath.isEmpty())
	{
		QBkeVariable v = QBkeVariable::array(imageAutoSearchPath.size());
		for (int i = 0; i < imageAutoSearchPath.size(); i++)
		{
			v[i] = imageAutoSearchPath[i];
		}
		results.append("ImageAutoSearchPath = " + v.saveToString() + ";");
	}
	if (!audioAutoSearchPath.isEmpty())
	{
		QBkeVariable v = QBkeVariable::array(audioAutoSearchPath.size());
		for (int i = 0; i < audioAutoSearchPath.size(); i++)
		{
			v[i] = audioAutoSearchPath[i];
		}
		results.append("AudioAutoSearchPath = " + v.saveToString() + ";");
	}
	if (!scriptAutoSearchPath.isEmpty())
	{
		QBkeVariable v = QBkeVariable::array(scriptAutoSearchPath.size());
		for (int i = 0; i < scriptAutoSearchPath.size(); i++)
		{
			v[i] = scriptAutoSearchPath[i];
		}
		results.append("ScriptAutoSearchPath = " + v.saveToString() + ";");
	}
	results.append(QString("MaxSaveDataNum = %1;").arg(maxSaveDataNum));
	if (!defaultFontName.isEmpty())
		results.append("DefaultFontName = \"" + defaultFontName + "\";");
	results.append(QString("DefaultFontSize = %1;").arg(defaultFontSize));
	if (!defaultFontColor.isNull())
		results.append("DefaultFontColor = " + defaultFontColor.saveToString() + ";");
	results.append(QString("DebugLevel = %1;").arg(debugLevel));
	if (!live2DKey.isEmpty())
		results.append("Live2DKey = \"" + live2DKey + "\";");
	if (!metaData.isNull() && metaData.getCount())
		results.append("MetaData = " + metaData.saveToString() + ";");

	QString result = results.join('\n');
	LOLI::AutoWrite(filePath, result);
}

void BkeProjectConfig::addScriptDir(QString dir)
{
	if (dir[0] == '/')
		dir = dir.right(dir.length() - 1);
	if (scriptAutoSearchPath.contains(dir))
		return;
	scriptAutoSearchPath.push_back(dir);
}

void BkeProjectConfig::removeScriptDir(QString dir)
{
	if (dir[0] == '/')
		dir = dir.right(dir.length() - 1);
	scriptAutoSearchPath.removeAll(dir);
}
