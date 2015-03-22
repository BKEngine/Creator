#ifndef BKEPROJECT_H
#define BKEPROJECT_H

#include <QSettings>
#include "weh.h"
#include "paper/creator_parser.h"
#include "bkeSci/bkemarks.h"
#include "bkeprojectconfig.h"

class ItemInfo;

void BKE_PROJECT_READITEM( QTreeWidgetItem *dest,ItemInfo &info) ;

class ItemInfo
{
public:
    QTreeWidgetItem *Root ;  //所属目录树，项目下的导入、脚本、资源
	QTreeWidgetItem *_this;
    qint64  IconKey ;   //图标hash码，用于标识文件类型
    QString ProName ;  //项目名称
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
		if (Layer <= 1)
			return QString();
		return FullName;
	}
};


typedef QHash<QString,QStringList*> BkeFilesHash ;

class BkeProject :public QObject
{
public:
    BkeProject(QObject *parent = 0);
    ~BkeProject() ;
	QString ProjectFile() const;
    QString FileDir() const;
    QString ProjectName() const ;
    QString IconKey(qint64 key) ;
    QString absName(const QString &name){ return FileDir()+"/"+name ; }
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
    void MakeItems(QTreeWidgetItem *dest,BkeFilesHash &hash) ;
    QStringList ListFiles(int type) ;
	void ListFiles(QStringList &ls, QTreeWidgetItem *root, const QString &parentdir);
    QStringList ItemDirs(QTreeWidgetItem *dest) ;
    void AddFileToHash(BkeFilesHash *hash,const QString &filename) ;
    bool removeFromHash(BkeFilesHash *hash, const ItemInfo &f ) ;
    bool SearchDir(BkeFilesHash &hash,const QString &dir,const QString &suffix) ;
    void ItemFromHash(QTreeWidgetItem *dest,BkeFilesHash &hash) ;
    void Addfiles(const QStringList &ls , const ItemInfo &f, bool autochange) ;
    void AddDir(const QString &dir ,const QString &relativeName, const ItemInfo &f) ;
	bool checkFileExist(const QString &f){ return files.contains(f); }
	bool checkIsDir(const ItemInfo &f);

    QStringList AllScriptFiles() ;
    QTreeWidgetItem *FindItem(QTreeWidgetItem *dest,const QString &dir,bool mkempty = true) ;
    QTreeWidgetItem *FindItemAll(const QString &name) ;
    QTreeWidgetItem *MakeItem(QTreeWidgetItem *dest,const QString &dir) ;
    BkeFilesHash *typeHash(int type) ;
    BkeFilesHash *typeHash(const QString &n) ;

	QTreeWidgetItem *ConfigFile;
	QTreeWidgetItem *Import;
    QTreeWidgetItem *Script ;
    QTreeWidgetItem *Source ;
    QTreeWidgetItem *BKAfile ;
    QTreeWidgetItem *ImageFile ;
    QTreeWidgetItem *VoiceFile ;
    QTreeWidgetItem *Root ;
    BkeParser *lex ;
    BkeProjectConfig *config;

	QMap<QString, long> files;

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
    QString ErrorInfo ;
    QString Time ;
    QStringList OutFilelist ;
    QStringList emptylist ;
    BkeFilesHash ImportHash ;
    BkeFilesHash ScriptHash ;
    BkeFilesHash SourceHash ;
    QJsonObject *bkpAdmin ;
    bool isnull ;
    int  currentptr ;

	QTreeWidgetItem* findItemIn(QTreeWidgetItem *p, const QString &name, bool createnew = false, QIcon *icon = NULL);
	void ForceAddScript(const QString &f, QTreeWidgetItem* p);
    void MakeImport() ;
    void ListToIni(QSettings *bkp,QStringList list) ;
    void BuildItem(const QString &name) ;
    void SetIconFromSuffix(QTreeWidgetItem *dest,const QString &suffix) ;
    void SortItem(QTreeWidgetItem *dest) ;
    void SortTree(QTreeWidgetItem *tree) ;
    void SearchTree(BkeFilesHash &hash, QTreeWidgetItem *dest,const QString &dir) ;
    QJsonObject HashToJson(BkeFilesHash &hash) ;
	QJsonObject TreeToJson(QTreeWidgetItem *tree);
    void JsonToHash(BkeFilesHash &hash,QJsonObject llm, bool lowVersion) ;
	void JsonToTree(QTreeWidgetItem *tree, QJsonObject llm, int version);
    QHash<QString,QStringList*> *ItemToHashptr(const QTreeWidgetItem *root) ;

    void FindSection(const QString &section , int &from , int &end) ;

    void copyStencil(const QString &file) ;
    QString MarksToString(BkeMarkList *mk) ;
    void CheckDir(BkeFilesHash *hash, const QString dirnow) ;
};

#endif // BKEPROJECT_H
