#ifndef BKEPROJECT_H
#define BKEPROJECT_H

#include <QSettings>
#include <QMap>
#include "weh.h"
#include "bkeSci/bkemarks.h"
#include "bkeprojectconfig.h"
#include "BG_Analysis.h"

class ItemInfo;

void BKE_PROJECT_READITEM( QTreeWidgetItem *dest,ItemInfo &info);

class ItemInfo
{
public:
	QTreeWidgetItem *Root ;  //所属目录树，工程下的导入、脚本、资源
	qint64  IconKey ;   //图标hash码，用于标识文件类型
	QString Dirs ;  //相对路径
	QString Name ;  //名称
	QString RootName ;
	QString FullName ;
	int Layer ;   //层级

	ItemInfo getLastItemInfo() const
	{
		if (Layer == 0)
			return *this;
		ItemInfo p;
		BKE_PROJECT_READITEM(Root->parent(), p);
		return p;
	}

	ItemInfo getLayer1ItemInfo() const
	{
		if (Layer <= 1)
			return *this;
		QTreeWidgetItem *r = Root;
		int L = Layer;
		while (L > 1)
		{
			L--;
			r = r->parent();
		}
		ItemInfo p;
		BKE_PROJECT_READITEM(r, p);
		return p;
	}

	QString getDir() const
	{
		return Dirs;
	}

	QString getFullName() const
	{
		return FullName;
	}

	ItemInfo getParentInfo() const
	{
		ItemInfo p;
		BKE_PROJECT_READITEM(Root->parent(), p);
		return p;
	}
};

enum ItemType
{
	ITEM_FILE = 0,
	ITEM_SCRIPT = 1,
	ITEM_IMAGE = 2,
	ITEM_AUDIO = 3,
	ITEM_VIDEO = 4,
	ITEM_DIR = 5,
};

class CodeWindow;

class BkeProject :public QObject
{
public:
	BkeProject(QObject *parent = 0);
	~BkeProject() ;
	QString ProjectFile() const;
	QString ProjectLangFile() const;
	QString ProjectDir() const;
	QString ProjectName() const ;
	QString absName(const QString &name){ return ProjectDir()+name ; }
	QString AllNameToName(const QString &allname) ;
	bool NewProject(const QString &dir,const QString &name) ;
	bool OpenProject(const QString &name) ;
	bool IsNull() ;
	bool WriteBkpFile() ;
	bool WriteMarkFile(BkeMarkSupport *m) ;
	bool RemoveItem(QTreeWidgetItem *Item) ;
	bool RemoveItem(const QString &file) ;
	bool RemoveItem(const ItemInfo &f) ;
	void SetTopLeveBold(bool t) ;
	void MakeItems(QTreeWidgetItem *dest,const QStringList &list) ;
	QStringList ListFiles(int type) ;
	void ListFiles(QStringList &ls, QTreeWidgetItem *root, const QString &parentdir);
	QStringList ItemDirs(QTreeWidgetItem *dest) ;
	void AddFiles(const QStringList &ls , const ItemInfo &f);
	void AddDir(const QString &dir, const ItemInfo &f) ;
	bool CheckIsDir(const ItemInfo &f);
	/**
	 *	在dir中搜索指定后缀名的文件
	 *	@param dir 相对于FileDir()的路径
	 *	@param suffixes 一个指定后缀名的字符串列表
	 *	@return 返回相对于FileDir()的文件路径
	 */
	QStringList SearchDir(const	QString &root, const QString &dir, const QStringList &suffixes);
	QStringList SearchDir(const	QString &root, const QString &dir, const QString suffix){
		return SearchDir(root, dir, QStringList()<<suffix);
	}

	//发布游戏
	void ReleaseGame();

	QStringList AllScriptFiles() ;
	QStringList AllSourceFiles() ;
	QTreeWidgetItem *FindItem(QTreeWidgetItem *dest,const QString &dir,bool mkempty = true) ;
	QTreeWidgetItem *FindItemAll(const QString &name) ;
	QTreeWidgetItem *MakeItem(QTreeWidgetItem *dest,const QString &dir) ;

	QTreeWidgetItem *ConfigFile;
	QTreeWidgetItem *Import;
	QTreeWidgetItem *Script;
	QTreeWidgetItem *Source;
	QTreeWidgetItem *BKAfile;
	QTreeWidgetItem *ImageFile;
	QTreeWidgetItem *VoiceFile;
	QTreeWidgetItem *Root;

private:
	void SetupConfig();

public:
	BkeProjectConfig *config;
	BG_Analysis *analysis;

	QIcon *fileico ;
	QIcon *dirsico ;
	QIcon *baseico ;
	QIcon *importico ;
	QIcon *bksdocico ;
	QIcon *sourcedocico ;
	QIcon *bksfileico ;
	QIcon *imgfileico ;
	QIcon *volfileico ;
	QIcon *movfileico ;

private:
	QString pdir ;
	QString pname ;
	QString pfile;
	QString ErrorInfo ;
	QString Time ;
	QStringList emptylist ;
	QJsonObject *bkpAdmin ;
	bool isnull ;
	int  currentptr ;

	/**
	 *	在QTreeWidgetItem节点中寻找一个节点，名称是{name}。若不存在，由{createnew}参数指定是否创建一个新的节点，图标为{icon}
	 */
	//QTreeWidgetItem* FindItemIn(QTreeWidgetItem *p, const QString &name, bool createnew = false, QIcon *icon = NULL);
	void ForceAddScript(const QString &f, QTreeWidgetItem* p);
	void MakeImport() ;
	void ListToIni(QSettings *bkp,QStringList list) ;
	void BuildItem(const QString &name) ;
	void SortItem(QTreeWidgetItem *dest) ;
	void SortTree(QTreeWidgetItem *tree) ;
	QJsonObject TreeToJson(QTreeWidgetItem *tree);
	void JsonToTree(QTreeWidgetItem *tree, QJsonObject llm, int version);

	/**
	 *	从文件名的Suffix判断ItemType
	 */
	ItemType GetTypeFromSuffix(const QString &filename);
	void SetIconFromType(QTreeWidgetItem *dest, ItemType type);
	void SetIconFromSuffix(QTreeWidgetItem *dest, const QString &suffix);

	void FindSection(const QString &section , int &from , int &end) ;

	void copyStencil(const QString &file) ;
	QString MarksToString(BkeMarkList *mk) ;

public:
	struct VersionData
	{
		QString name;
		QDateTime date; //这里储存的时间是UTC时间
		QString info;
		QMap<QString, QDateTime> data;  //这里储存的时间是UTC时间
	};
private:
	QList<VersionData> _versionData;
public:
	QList<VersionData> &getVersionDataList(){return _versionData;}
	int addVersionData(QWidget *parent); //返回编号，如果是-1表明没有添加成功或者用户取消了添加操作
};

#endif // BKEPROJECT_H
