#include <weh.h>
#include "bkeproject.h"
//#include "dia/newversiondatawizard.h"
//#include "dia/versioninfo.h"

QString BkeProject::NormalizeDirPath(QString path)
{
	path.replace('\\', '/');
	if (!path.endsWith('/'))
	{
		path.push_back('/');
	}
	return path;
}

//读取一个item信息s
void BKE_PROJECT_READITEM(QTreeWidgetItem *dest, ItemInfo &info)
{
	info.Name = dest->text(0);
	info.IconKey = dest->icon(0).cacheKey();

	QTreeWidgetItem *root = dest;
	QStringList list;
	while (root->parent() != 0){
		root = root->parent();
		list.prepend(root->text(0)); //父节点总是在前面
	}
	info.Dirs.clear();
	for (int i = 2; i < list.size(); i++){
		info.Dirs.append(list.at(i));
		info.Dirs.append('/');
	}
	info.Layer = list.size();
	info.Root = dest;
	if (list.size() == 0){  //工程文件本身
		info.RootName = "";
		info.FullName = info.Dirs + info.Name;
	}
	else if (list.size() == 1){  //导入，脚本，资源
		info.RootName = info.Name;
		info.FullName = info.Dirs;
		if (info.FullName.isEmpty())
		{
			//config.bkpsr这个东西现在直接属于顶级目录下
			info.FullName = info.RootName;
		}
	}
	else{
		info.RootName = list.at(1);
		info.FullName = info.Dirs + info.Name;
	}
}

//新建一个工程
BkeProject::BkeProject(QObject *parent)
	:QObject(parent)
{
	isnull = true;
	currentptr = 0;
	analysis = 0;

	fileico = new QIcon(":/project/file.png");
	dirsico = new QIcon(":/project/doc.png");
	baseico = new QIcon(":/project/database.png");
	importico = new QIcon(":/project/import.png");
	bksdocico = new QIcon(":/project/bksdoc.png");
	sourcedocico = new QIcon(":/project/sourcedoc.png");
	bksfileico = new QIcon(":/project/bksfile.png");
	imgfileico = new QIcon(":/project/image.png");
	volfileico = new QIcon(":/project/music.png");
	movfileico = new QIcon(":/project/movie.png");
	config = nullptr;
}

BkeProject::~BkeProject()
{
	//delete lex;
	delete analysis;
	delete config;

	delete fileico;
	delete dirsico;
	delete baseico;
	delete importico;
	delete bksdocico;
	delete sourcedocico;
	delete bksfileico;
	delete imgfileico;
	delete volfileico;
	delete movfileico;
	//lex->deleteLater();
}


//初始化工程
void BkeProject::BuildItem(const QString &name)
{
	Root = new QTreeWidgetItem(QStringList() << name);
	ConfigFile = new QTreeWidgetItem(QStringList() << "config.bkpsr");
	Import = new QTreeWidgetItem(QStringList() << "宏");
	Script = new QTreeWidgetItem(QStringList() << "脚本");
	Source = new QTreeWidgetItem(QStringList() << "资源");

	Root->setIcon(0, *baseico);
	ConfigFile->setIcon(0, *bksfileico);
	Import->setIcon(0, *importico);
	Script->setIcon(0, *bksdocico);
	Source->setIcon(0, *sourcedocico);

	Root->addChild(ConfigFile);
	Root->addChild(Import);
	Root->addChild(Script);
	Root->addChild(Source);
	Root->setExpanded(true);
}

bool BkeProject::NewProject(const QString &dir, const QString &name)
{
	pdir = NormalizeDirPath(dir);
	pname = name;

	//初始化工程
	BuildItem(pname);
	MakeImport();

	/*QStringList ls = SearchDir(FileDir(), "", ".bkscr");
	for (auto &&file : ls)
	{
		ForceAddScript(file, Script);
	}*/

	delete analysis;
	analysis = new BG_Analysis(ProjectDir());
	return WriteBkpFile();
}

/*
QTreeWidgetItem* BkeProject::FindItemIn(QTreeWidgetItem *p, const QString &name, bool createnew, QIcon *icon)
{
	for (int i = 0; i < p->childCount(); i++)
	{
		if (p->child(i)->text(0) == name)
			return p->child(i);
	}
	if (createnew)
	{
		QTreeWidgetItem *res = new QTreeWidgetItem(QStringList() << name);
		p->addChild(res);
		if (icon)
			res->setIcon(0, *icon);
		return res;
	}
	return NULL;
}*/

void BkeProject::ForceAddScript(const QString &f, QTreeWidgetItem* p)
{
	FindItem(p, f, true);
	/*int pos = f.indexOf(QRegExp("[/\\]"));
	if (pos >= 0)
	{
		QString f1 = f.left(pos);
		QString f2 = f.mid(pos + 1);
		QTreeWidgetItem *sub = FindItemIn(p, f1, true, bksdocico);
		ForceAddScript(f2, sub);
	}
	else
	{
		In(p, f, true, bksfileico);
	}*/
}

//读取文件
bool BkeProject::OpenProject(const QString &name)
{
	QJsonObject bkpAdmin = LoadProject(name);
	if (bkpAdmin.isEmpty())
		return false;

	int version = int(bkpAdmin.value("version").toString().toDouble() * 10 + 0.5);

	QFileInfo fi(name);
	pdir = NormalizeDirPath(fi.path());
	pfile = fi.fileName();
	pname = bkpAdmin.value("name").toString();
	//if( bkpAdmin.value("version").toString().isEmpty() ) pdir = pdir + "/" + pname ;

	delete config;
	config = new BkeProjectConfig(pdir, pdir + "config.bkpsr");
	config->readFile();
	SetupConfig();

	BuildItem(pname);
	{
		JsonToTree(Import, bkpAdmin.value("import").toObject(), version);
		JsonToTree(Script, bkpAdmin.value("script").toObject(), version);
		JsonToTree(Source, bkpAdmin.value("source").toObject(), version);
	}
	//remove config.bkpsr from Import
	auto it = FindItem(Import, "config.bkpsr", false);
	if (it)
	{
		Import->removeChild(it);
	}

	delete analysis;
	auto ls = AllScriptFiles();

	analysis = new BG_Analysis(ProjectDir());
	analysis->addFiles(ls);
	//检查路径是否已经改变
	/*
	QFileInfo llm(name) ;
	if( LOLI_OS_QSTRING( FileDir()) != LOLI_OS_QSTRING(llm.path()) ){
	//被改变
	CheckDir(&ImportHash,llm.path());
	CheckDir(&ScriptHash,llm.path());
	CheckDir(&SourceHash,llm.path());
	pdir = llm.path() ;
	WriteBkpFile() ;
	}*/

	SortTree(Root);

	if (version < SAVE_VERSION)
		WriteBkpFile();

	return true;
}

QJsonObject BkeProject::LoadProject(const QString &name)
{
	QFile f(name);
	if (!f.isOpen() && !f.open(QFile::ReadOnly)) return QJsonObject();

	QJsonDocument kk = QJsonDocument::fromJson(f.readAll());
	if (kk.isNull() || !kk.isObject()) return QJsonObject();

	return kk.object();
}

void BkeProject::SetupConfig()
{
	

	//connect(config, &BkeProjectConfig::onScriptAutoSearchPathChanged, nullptr, Qt::QueuedConnection);

}

//寻找输入输出文件
void BkeProject::MakeImport()
{
	QString dirs = ProjectDir();

	QStringList OutFilelist;
	OutFilelist << "config.bkpsr" << "main.bkscr" << "macro.bkscr";

	//从模版中复制文件，如果没有则创建
	for (int i = 0; i < OutFilelist.size(); i++){
		LOLI_MAKE_NULL_FILE(dirs + "/" + OutFilelist.at(i));
	}

	//新建工程设置
	delete config;
	config = new BkeProjectConfig(dirs, dirs + "/config.bkpsr");
	config->readFile();
	config->projectName = ProjectName();
	config->writeFile();

	SetupConfig();

	//语法分析
	//lex->ParserFile("macro.bkscr", dirs);
	//OutFilelist.append(lex->GetImportFiles()); //导入的脚本，将在脚本中属于例外

	//config.bkpsr单独放
	//main.bkscr放到脚本工程底下，其余放宏

	{
		FindItem(Script, OutFilelist[1], true);
	}

	for (int i = 2; i < OutFilelist.size(); i++){
		if (OutFilelist.at(i).trimmed().isEmpty()) continue;
		FindItem(Import, OutFilelist.at(i), true);
	}

	SortItem(Import);
	return;
}

//从qstring列表中创建工程
void BkeProject::MakeItems(QTreeWidgetItem *dest, const QStringList &list)
{
	QString temp;
	for (int i = 0; i < list.size(); i++){
		temp = list.at(i);
		if (temp.isEmpty()) continue;
		FindItem(dest, temp, true);
	}
}

//带排序的创建
QTreeWidgetItem *BkeProject::MakeItem(QTreeWidgetItem *dest, const QString &dir)
{
	QString abc = AllNameToName(dir);
	if (abc.isEmpty()) return dest;

	QTreeWidgetItem *le = FindItem(dest, abc);
	ItemInfo f;
	BKE_PROJECT_READITEM(le, f);

	if (le->parent() != 0) SortItem(le->parent());
	return le;
}

//寻找文件，失败返回0，创建空路径时将创建不存在的节点
QTreeWidgetItem *BkeProject::FindItem(QTreeWidgetItem *dest, const QString &dir, bool mkempty)
{
	QString llm = dir;

	if (llm.isEmpty()) return dest;

	QTreeWidgetItem *root = dest;
	QStringList tk = llm.split("/");
	QString a, b;
	for (int i = 0; i < tk.size(); i++){
		int s = root->childCount() - 1;

		while (s >= 0){
			a = LOLI_OS_QSTRING(root->child(s)->text(0));  //为应对不同平台
			b = LOLI_OS_QSTRING(tk.at(i));
			if (a != b) s--;
			else break;
		}

		if (s >= 0) root = root->child(s);
		else if (mkempty)
		{
			QTreeWidgetItem *le = new QTreeWidgetItem;
			le->setText(0, tk.at(i));
			SetIconFromSuffix(le, tk.at(i));
			root->addChild(le);
			root = le;
			SortTree(root);
		}
		else return 0;
	}

	return root;
}

QString BkeProject::ProjectDir() const
{
	return pdir;
}

QString BkeProject::ProjectName() const
{
	return pname;
}

QString BkeProject::ProjectFile() const
{
	return pdir + pfile;
}

QString BkeProject::ProjectLangFile() const
{
	return pdir + pfile + ".user";
}

//寻找指定的文件
QStringList BkeProject::SearchDir(const	QString &root, const QString &dir, const QStringList &suffix)
{
	QStringList list;
	QDir d(root + "/" + dir);
	if (!d.exists()) return list;

	d.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
	d.setSorting(QDir::DirsFirst);

	QFileInfoList infols = d.entryInfoList();
	QFileInfo fff;
	for (int i = 0; i < infols.size(); i++){
		fff = infols.at(i);

		if (fff.fileName() == "." || fff.fileName() == "..")
			continue;
		else if (fff.isDir())
			list.append(SearchDir(root, dir + "/" + fff.fileName(), suffix));
		else if (suffix.indexOf(chopFileExt(fff.fileName()).toLower()) < 0)
			continue; //不是已指定后缀结尾
		else
		{
			list << QDir(root).relativeFilePath(fff.absoluteFilePath());
		}
	}
	return list;
}

void BkeProject::SetIconFromType(QTreeWidgetItem *dest, ItemType type)
{
	switch (type)
	{
	case ITEM_FILE:
		dest->setIcon(0, *fileico);
		break;
	case ITEM_SCRIPT:
		dest->setIcon(0, *bksfileico);
		break;
	case ITEM_AUDIO:
		dest->setIcon(0, *volfileico);
		break;
	case ITEM_IMAGE:
		dest->setIcon(0, *imgfileico);
		break;
	case ITEM_VIDEO:
		dest->setIcon(0, *movfileico);
		break;
	case ITEM_DIR:
		dest->setIcon(0, *dirsico);
		break;
	default:
		break;
	}
}

//设置图标
void BkeProject::SetIconFromSuffix(QTreeWidgetItem *dest, const QString &suffix)
{
	SetIconFromType(dest, GetTypeFromSuffix(suffix));
}

bool BkeProject::WriteBkpFile()
{
	QJsonObject bkpAdmin;
	bkpAdmin.insert("name", pname);

	//bkpAdmin.insert("import", HashToJson(ImportHash));
	//bkpAdmin.insert("script", HashToJson(ScriptHash));
	//bkpAdmin.insert("source", HashToJson(SourceHash));
	bkpAdmin.insert("import", TreeToJson(Import));
	bkpAdmin.insert("script", TreeToJson(Script));
	bkpAdmin.insert("source", TreeToJson(Source));
	bkpAdmin.insert("version", QString("1.2"));

	QJsonDocument llm;
	llm.setObject(bkpAdmin);
	return LOLI::AutoWrite(ProjectDir() + BKE_PROJECT_NAME, llm.toJson());
}

QJsonObject BkeProject::TreeToJson(QTreeWidgetItem *tree)
{
	QJsonObject llm;
	for (auto i = 0; i < tree->childCount(); i++)
	{
		auto ch = tree->child(i);
		if (ch->icon(0).cacheKey() == dirsico->cacheKey())
		{
			llm.insert(ch->text(0), TreeToJson(ch));
		}
		else
			llm.insert(ch->text(0), 0);
	}
	return llm;
}

void BkeProject::JsonToTree(QTreeWidgetItem *tree, QJsonObject llm, int version)
{
	switch (version)
	{
	case 12:
	{
		for (auto it = llm.begin(); it != llm.end(); it++)
		{
			if (it.value().isObject())
			{
				//folder
				auto tr = new QTreeWidgetItem();
				tr->setText(0, it.key());
				tr->setIcon(0, *dirsico);
				tree->addChild(tr);
				JsonToTree(tr, it.value().toObject(), version);
			}
			else
			{
				//file
				auto tr = new QTreeWidgetItem();
				tr->setText(0, it.key());
				ItemType type = GetTypeFromSuffix(it.key());
				SetIconFromSuffix(tr, it.key());
				tree->addChild(tr);
				JsonToTree(tr, it.value().toObject(), version);
			}
		}
	}
	}
}

static QStringList scriptSuffixes = {
	".bkscr"
};

static QStringList imageSuffixes = {
	".png",
	".jpg",
	".bmp",
	".webp",
	".apng",
	".gif",
};

static QStringList audioSuffixes = {
	".mp3",
	".ogg",
	".wav",
	".opus",
};

static QStringList videoSuffixes = {
	".mp4"
};

ItemType BkeProject::GetTypeFromSuffix(const QString &filename)
{
	QString suffix = chopFileExt(filename).toLower();
	if (suffix.isEmpty()) {
		return ITEM_DIR;
	}
	else if (imageSuffixes.contains(suffix)) {
		return ITEM_IMAGE;
	}
	else if (audioSuffixes.contains(suffix)) {
		return ITEM_AUDIO;
	}
	else if (videoSuffixes.contains(suffix)) {
		return ITEM_VIDEO;
	}
	else if (scriptSuffixes.contains(suffix)) {
		return ITEM_SCRIPT;
	}
	else {
		return ITEM_FILE;
	}
}

//排序,文件夹优先，大小写不敏感
void BkeProject::SortItem(QTreeWidgetItem *dest)
{
	bool isExpanded = dest->isExpanded();
	QList<QTreeWidgetItem *> root = dest->takeChildren();
	QStringList dirlist, filelist;
	QHash<QString, QTreeWidgetItem *> temp;
	QTreeWidgetItem *le;

	for (int i = 0; i < root.size(); i++){
		le = root.at(i);
		if (le->icon(0).cacheKey() == dirsico->cacheKey()) dirlist.append(le->text(0));
		else filelist.append(le->text(0));
		temp[le->text(0)] = le;
	}

	dirlist.sort(Qt::CaseInsensitive);
	filelist.sort(Qt::CaseInsensitive);
	for (int i = 0; i < dirlist.size(); i++){
		dest->addChild(temp.value(dirlist.at(i)));
	}

	for (int i = 0; i < filelist.size(); i++){
		dest->addChild(temp.value(filelist.at(i)));
	}
	if (isExpanded)
		dest->setExpanded(true);
}

//排序整个目录
void BkeProject::SortTree(QTreeWidgetItem *tree)
{
	QTreeWidgetItem *le;

	for (int i = 0; i < tree->childCount(); i++){
		le = tree->child(i);
		if (le->childCount() > 0) SortTree(le);
	}
	SortItem(tree);
}

bool BkeProject::RemoveItem(QTreeWidgetItem *Item)
{
	QTreeWidgetItem *le;
	for (int i = 0; i < Item->childCount(); i++){
		if (Item->child(i)->childCount() > 0){
			RemoveItem(Item->child(i));
		}
		delete Item->child(i);
	}

	le = Item->parent();
	if (le == 0) return true;
	le->removeChild(Item);
	return true;
}

bool BkeProject::RemoveItem(const ItemInfo &f)
{
	return RemoveItem(f.Root);
}

bool BkeProject::RemoveItem(const QString &file)
{
	QTreeWidgetItem *le = FindItemAll(file);
	if (le == 0) return false;
	ItemInfo f;
	BKE_PROJECT_READITEM(le, f);
	RemoveItem(f);
	return true;
}

QStringList BkeProject::ListFiles(int type)
{
	QString path;
	QStringList filelist;

	auto root = Root->child(type);

	ListFiles(filelist, root, "");

	return filelist;
}

void BkeProject::ListFiles(QStringList &ls, QTreeWidgetItem *root, const QString &parentdir)
{
	for (int i = 0; i < root->childCount(); i++)
	{
		auto r = root->child(i);
		QString relatePath = parentdir.isEmpty() ? r->text(0) : parentdir + '/' + r->text(0);
		if (r->childCount() == 0)
			ls.push_back(relatePath);
		else
			ListFiles(ls, r, relatePath);
	}
}

QStringList BkeProject::AllScriptFiles()
{
	QStringList temp;
	temp.append(ListFiles(1));
	temp.append(ListFiles(2));
	return temp;
}

QStringList BkeProject::AllSourceFiles()
{
	QStringList temp;
	temp.append(ListFiles(3));
	return temp;
}

QTreeWidgetItem *BkeProject::FindItemAll(const QString &name)
{
	QTreeWidgetItem *le;
	QString temp = AllNameToName(name);
	le = FindItem(Import, temp, false);
	if (le == 0) le = FindItem(Script, temp, false);
	else if (le == 0) le = FindItem(Source, temp, false);
	return le;
}


//bool BKEproject::WriteMarks()
//{
//    QStringList list ;
//    list.append( ListFiles(0) );
//    list.append( ListFiles(1) );

//    QString bookmarks ;
//    BkeMarkList *akb ;
//    for( int i = 0 ; i < list.size() ; i++){
//        akb = marksAdmin.GetBookMark( LOLI_OS_QSTRING(list.at(i)),false) ;
//        if( !akb->isEmpty() ){

//        }

//    }
//}

void BkeProject::SetTopLeveBold(bool t)
{
	QFont a = Root->font(0);
	a.setBold(t);
	Root->setFont(0, a);
}

bool BkeProject::WriteMarkFile(BkeMarkSupport *m)
{
	QString akb;
	QStringList list/* = AllScriptFiles()*/;
	BkeMarkList *mks;
	for (int i = 0; i < list.size(); i++){
		mks = m->GetBookMark(list.at(i), false);
		if (mks->isEmpty()) continue;

		akb.append(':' + list.at(i) + "\r\n");
		akb.append(MarksToString(mks));
	}

	return LOLI::AutoWrite(ProjectDir() + "BkeProject.bkpmk", akb);
}

QString BkeProject::MarksToString(BkeMarkList *mk)
{
	QString result;
	QString temp;
	BkeMarkerBase *base;
	for (int i = 0; i < mk->size(); i++){
		base = mk->at(i);
		temp.setNum(base->Atpos);
		temp.append("/" + base->Information + "\r\n");
		result.append(temp);
	}
	return result;
}

//返回dest所有的子目录，不包括自身
QStringList BkeProject::ItemDirs(QTreeWidgetItem *dest)
{
	QStringList ls;
	for (int i = 0; i < dest->childCount(); i++){
		if (dest->child(i)->childCount() > 0) ls.append(ItemDirs(dest->child(i)));
	}
	return ls;
}

void BkeProject::AddFiles(const QStringList &ls, const ItemInfo &f)
{
	for (auto &&s : ls)
	{
		FindItem(f.Root, s);
		analysis->pushFile(f.FullName + "/" + s);
	}
	SortTree(f.Root);
	WriteBkpFile();
}

void BkeProject::AddDir(const QString &dir, const ItemInfo &f)
{
	auto ff = f.getLayer1ItemInfo();
	FindItem(f.Root, dir);
	
	QStringList ls = SearchDir(ProjectDir()+f.getDir(), dir, (ff.Name == "脚本" || ff.Name == "宏") ? scriptSuffixes : (audioSuffixes + imageSuffixes));
	AddFiles(ls, f);
}

bool BkeProject::CheckIsDir(const ItemInfo &f)
{
	return f.IconKey == dirsico->cacheKey();
}

int BkeProject::addVersionData(QWidget *parent)
{
	/*
	NewVersionDataWizard wizard(this, parent);
	wizard.exec();
	if(wizard.isAccepted())
	{
	int index = _versionData.size();
	_versionData.append(VersionData{wizard.name,wizard.time,wizard.info,wizard.data});
	return index;
	}*/
	return -1;
}

QString BkeProject::AllNameToName(const QString &allname)
{

	if (allname.startsWith("/") || (allname.length() > 1 && allname[1] == QChar(':'))){
		if (allname.length() == ProjectDir().length()) return "";
		else return  allname.right(allname.length() - ProjectDir().length());
	}
	else return allname;
}

void BkeProject::ReleaseGame()
{
	
}
