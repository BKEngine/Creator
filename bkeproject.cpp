#include "bkeproject.h"

//读取一个item信息s
void BKE_PROJECT_READITEM( QTreeWidgetItem *dest,ItemInfo &info)
{
    info.Name = dest->text(0) ;
    info.IconKey = dest->icon(0).cacheKey() ;

    QTreeWidgetItem *root = dest ;
    QStringList list ;
    while(root->parent() != 0){
        root = root->parent() ;
        list.prepend(root->text(0)); //父节点总是在前面
    }
    info.Dirs.clear(); ;
    for(int i = 2 ; i < list.size() ; i++){
        info.Dirs.append( list.at(i)) ;
        if( i != list.size()-1) info.Dirs.append("/") ;
    }

    info.Layer = list.size() ;
    if( list.size() == 0){  //项目文件本身
        info.ProName = info.Name ;
        info.Root = dest ;
    }
    else if( list.size() == 1){  //导入，脚本，资源
        info.ProName = list.at(0) ;
        info.RootName = info.Name ;
        info.Root = dest ;
    }
    else{
        info.ProName = list.at(0) ;
        for( int i = 0 ; i < root->childCount() ; i++){
            if( root->child(i)->text(0) == list.at(1)){
                info.Root = root->child(i) ;
                info.RootName = root->child(i)->text(0) ;
                break ;
            }
        }
    }

    if( info.Dirs.isEmpty()) info.FullName = info.Name ;
    else info.FullName = info.Dirs + "/" + info.Name ;
}

//新建一个项目
BKEproject::BKEproject(QObject *parent)
    :QObject(parent)
{
    isnull = true ;
    currentptr = 0 ;
    lex = new BkeParser(this) ;
    fileico = new QIcon(":/project/source/file.png") ;
    dirsico = new QIcon(":/project/source/doc.png") ;
    baseico = new QIcon(":/project/source/database.png") ;
    importico = new QIcon(":/project/source/import.png") ;
    bksdocico = new QIcon(":/project/source/bksdoc.png") ;
    sourcedocico = new QIcon(":/project/source/sourcedoc.png") ;
    bksfileico = new QIcon(":/project/source/bksfile.png") ;
    imgfileico = new QIcon(":/project/source/image.png") ;
    volfileico = new QIcon(":/project/source/music.png") ;
    movfileico = new QIcon(":/project/source/movie.png") ;

}


//初始化项目
void BKEproject::BuildItem(const QString &name)
{
    Root = new QTreeWidgetItem(QStringList()<<name) ;
    Import = new QTreeWidgetItem(QStringList()<<"初始化") ;
    Script = new QTreeWidgetItem(QStringList()<<"脚本") ;
    Source = new QTreeWidgetItem(QStringList()<<"资源") ;

    Root->setIcon(0,*baseico);
    Import->setIcon(0,*importico);
    Script->setIcon(0,*bksdocico);
    Source->setIcon(0,*sourcedocico);

    Root->addChild(Import);
    Root->addChild(Script);
    Root->addChild(Source);
}

bool BKEproject::NewProject(const QString &dir,const QString &name)
{
    pdir = dir ;
    pname = name ;

    //初始化项目
    BuildItem(pname);
    MakeImport();

    SearchTree(ImportHash,Import,FileDir());
    SearchDir(ScriptHash,FileDir(),".bkscr") ;
    //SearchDir(SourceHash,FileDir(),".jpg.jpeg.png.bmp.ogg.wav.mp3.aac.mp4.avi.mpg") ;
    MakeItems(Script,ScriptHash);
    MakeItems(Source,SourceHash);

    return WriteBkpFile() ;
}

//读取文件
bool BKEproject::OpenProject(const QString &name)
{
    QFile f(name) ;
    if( !f.isOpen() && !f.open(QFile::ReadOnly) ) return false ;

    QJsonDocument kk = QJsonDocument::fromJson(f.readAll()) ;
    if( kk.isNull() || !kk.isObject() ) return false;

    bkpAdmin = new QJsonObject( kk.object() ) ;
    if( bkpAdmin->isEmpty() ) return false ;

    pdir = bkpAdmin->value("dir").toString() ;
    pname = bkpAdmin->value("name").toString() ;
    if( bkpAdmin->value("version").toString().isEmpty() ) pdir = pdir+pname ;


    BuildItem(pname);
    JsonToHash(ImportHash,bkpAdmin->value("import").toObject());
    JsonToHash(ScriptHash,bkpAdmin->value("script").toObject());
    JsonToHash(SourceHash,bkpAdmin->value("source").toObject());

    //检查路径是否已经改变
    QFileInfo llm(name) ;
    if( LOLI_OS_QSTRING( FileDir()) != LOLI_OS_QSTRING(llm.path()) ){
        //被改变
        CheckDir(&ImportHash,llm.path());
        CheckDir(&ScriptHash,llm.path());
        CheckDir(&SourceHash,llm.path());
        pdir = llm.path() ;
        WriteBkpFile() ;
    }

    MakeItems(Import,ImportHash);
    MakeItems(Script,ScriptHash);
    MakeItems(Source,SourceHash);





    return true ;
}

//寻找输入输出文件
void BKEproject::MakeImport()
{
    OutFilelist.clear();
    QString dirs = FileDir() ;

    OutFilelist <<"main.bkscr"<<"macro.bkscr"<<"config.bkpsr" ;
    QStringList ls = BkeCreator::CopyStencil(dirs,OutFilelist) ;
    LOLI::makeNullFile(ls,dirs) ;

    lex->ParserFile("macro.bkscr",dirs);
    OutFilelist.append( lex->GetImportFiles() ); //导入的脚本，将在脚本中属于例外


    for( int i = 0 ; i < OutFilelist.size() ; i++){
        if( OutFilelist.at(i).trimmed().isEmpty() ) continue ;
        FindItem(Import,OutFilelist.at(i),true) ;
        OutFilelist[i] = LOLI_OS_QSTRING( FileDir()+"/"+OutFilelist.at(i) ) ;
    }

    SortItem(Import);
    return ;
}

//从qstring列表中创建项目
void BKEproject::MakeItems(QTreeWidgetItem *dest,const QStringList &list)
{
    QString temp ;
    for( int i = 0 ; i < list.size() ; i++){
        temp = list.at(i) ;
        if( temp.indexOf(":") > 0) temp = BkeFullnameToName(temp,FileDir()) ;
        if( temp.isEmpty() ) continue ;
        FindItem(dest,temp,true) ;
    }
}

//从hash列表中创建项目
void BKEproject::MakeItems(QTreeWidgetItem *dest,BkeFilesHash &hash)
{
    QTreeWidgetItem *le ;
    for( auto ptr = hash.begin() ; ptr != hash.end() ; ptr++){
        le = FindItem(dest,ptr.key(),true) ;
        MakeItems(le, *(ptr.value()) );
        SortItem(le);
    }
    SortItem( dest );
}

//带排序的创建
QTreeWidgetItem *BKEproject::MakeItem(QTreeWidgetItem *dest,const QString &dir)
{
    QString abc = dir ;
    if( abc.indexOf(":") >= 0) abc = BkeFullnameToName(dir,FileDir()) ;
    if( abc.isEmpty() ) return dest ;

    QTreeWidgetItem *root = dest ;
    QStringList tk = abc.split("/") ;

    for( int i = 0 ; i < tk.size() ; i++){
        root = simpleMakeItem(root,tk.at(i)) ;
        SetIconFromSuffix(root,tk.at(i));
        SortItem(root->parent()); //
    }
    return root ;
}

QTreeWidgetItem *BKEproject::simpleMakeItem(QTreeWidgetItem *dest,const QString &t)
{
    QTreeWidgetItem *le = FindItem(dest,t,false) ;
    if( le != 0) return le ;

    int i ;
    for( i = 0 ; i < dest->childCount() ; i++){
        if( t.compare(dest->child(i)->text(0),Qt::CaseInsensitive) < 0) break ;
    }

    le = new QTreeWidgetItem ;
    le->setText(0,t);
    dest->insertChild(i,le);
    return le ;
}

//寻找文件，失败返回0，创建空路径时将创建不存在的节点
QTreeWidgetItem *BKEproject::FindItem(QTreeWidgetItem *dest,const QString &dir,bool mkempty )
{
    QString llm = dir ;

    if( dir.indexOf(':') >= 0 ) llm = BkeFullnameToName(dir,FileDir()) ;
    if( llm.isEmpty() ) return dest ;

    QTreeWidgetItem *root = dest ;
    QStringList tk = llm.split("/") ;
    QString a,b ;
    for( int i = 0 ; i < tk.size() ; i++){
        int s = root->childCount() - 1 ;

        while( s>= 0 ){
            a = LOLI_OS_QSTRING( root->child(s)->text(0) ) ;  //为应对不同平台
            b = LOLI_OS_QSTRING( tk.at(i) ) ;
            if( a != b) s-- ;
            else break ;
        }

        if( s >= 0 ) root = root->child(s) ;
        else if( mkempty)
        {
            QTreeWidgetItem *le = new QTreeWidgetItem;
            le->setText(0,tk.at(i));
            SetIconFromSuffix(le,tk.at(i));
            root->addChild(le);
            root = le ;
        }
        else return 0 ;
    }

    return root ;
}

QString BKEproject::FileDir() const
{
    return pdir ;
}

QString BKEproject::ProjectName() const
{
    return pname ;
}

//寻找指定的文件，并把文件加入到hash中
bool BKEproject::SearchDir(BkeFilesHash &hash,const QString &dir,const QString &suffix)
{
    QDir d( dir ) ;
    if( !d.exists() ) return false ;

    d.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    d.setSorting(QDir::DirsFirst);

    QStringList *ls = new QStringList ;
    QFileInfoList infols = d.entryInfoList() ;
    QFileInfo fff ;
    for( int i = 0 ; i < infols.size() ; i++){
        fff = infols.at(i) ;

        if( fff.fileName() == "." || fff.fileName() == ".." ) continue ;
        else if( fff.isDir() ) SearchDir( hash , dir+"/"+fff.fileName(),suffix ) ;
        else if( suffix.indexOf(fff.suffix().toLower()) < 0 ) continue ; //不是已指定后缀结尾
        else if( OutFilelist.indexOf( LOLI_OS_QSTRING(dir+"/"+fff.fileName() ) ) >= 0 ) continue ;
        else ls->append( fff.fileName() );
    }

    if( !ls->isEmpty() ){
        //hash[ LOLI_OS_QSTRING(dir) ] = ls ;
        hash[ dir ] = ls ;
        return true ;
    }
    else return false ;
}

//从树结构中生成hash
void BKEproject::SearchTree(BkeFilesHash &hash, QTreeWidgetItem *dest,const QString &dir)
{
    QTreeWidgetItem *le ;
    QStringList *ls = new QStringList ;
    for( int i = 0 ; i < dest->childCount() ; i++){
        le = dest->child(i) ;
        if( le->childCount() > 0) SearchTree(hash,le,dir+"/"+le->text(0));
        else ls->append( le->text(0) );
        //要检测文件夹图标？
    }

    if( !ls->isEmpty() ){
        QStringList *ts = hash.value( LOLI_OS_QSTRING(dir) ) ;
        hash[ LOLI_OS_QSTRING(dir) ] = ls ;
        if( ts != 0) delete ts ;  //删除旧的对象
    }
}

//设置图标
void BKEproject::SetIconFromSuffix(QTreeWidgetItem *dest,const QString suffix)
{
    QString temp = suffix.toLower() ;
    if( !temp.startsWith('.') && temp.lastIndexOf('.') > 0) temp = temp.right(temp.length()-temp.lastIndexOf('.')) ;
    if( QString(".jpg.jpeg.png.bmp").indexOf(temp) >= 0){
        dest->setIcon(0,*imgfileico);
        return ;
    }
    else if( QString(".ogg.wav.mp3.aac").indexOf(temp) >= 0){
        dest->setIcon(0,*volfileico);
        return ;
    }
    else if( QString(".mp4.avi.mpg").indexOf(temp) >= 0){
        dest->setIcon(0,*movfileico);
        return ;
    }
    else if( !temp.startsWith('.') ) dest->setIcon(0,*dirsico);
    else{
        dest->setIcon(0,*fileico);
        return ;
    }
}

bool BKEproject::WriteBkpFile()
{
    bkpAdmin = new QJsonObject ;
    bkpAdmin->insert("name",pname) ;
    bkpAdmin->insert("dir",pdir) ;

    bkpAdmin->insert("import",HashToJson(ImportHash)) ;
    bkpAdmin->insert("script",HashToJson(ScriptHash)) ;
    bkpAdmin->insert("source",HashToJson(SourceHash)) ;
    bkpAdmin->insert("version",QString("1.0")) ;

    QJsonDocument llm ;
    llm.setObject(*bkpAdmin);
    return LOLI::AutoWrite(FileDir()+"/"+BKE_PROJECT_NAME,llm.toJson()) ;
}

QJsonObject BKEproject::HashToJson(BkeFilesHash &hash)
{
    QJsonObject llm ;
    QStringList *ls ;
    for( auto ptr = hash.begin() ; ptr != hash.end() ; ptr++){
        ls = ptr.value() ;
        if( ls->isEmpty() ) continue ;
        llm.insert( ptr.key(),QJsonArray::fromStringList(*ls)) ;
    }
    return llm ;
}

void BKEproject::JsonToHash(BkeFilesHash &hash,QJsonObject llm)
{
    QStringList keyls = llm.keys() ;
    for( int i = 0 ; i < keyls.size() ; i++){
        QVariant ks = llm.value(keyls.at(i)).toVariant() ;
        QStringList *ls = new QStringList ;
        *ls = ks.toStringList() ;
        hash[ keyls.at(i) ] = ls ;
    }
}


QString BKEproject::IconKey(qint64 key)
{
    if( key == baseico->cacheKey()) return QString("@root") ;
    else if( key == importico->cacheKey()) return QString("@import") ;
    else if( key == fileico->cacheKey()) return QString("@file") ;
    else if( key == dirsico->cacheKey()) return QString("@dir") ;
    else if( key == bksdocico->cacheKey()) return QString("@script") ;
    else if( key == sourcedocico->cacheKey()) return QString("@source") ;
    else if( key == bksfileico->cacheKey()) return QString("@bksfile") ;
    else if( key == imgfileico->cacheKey()) return QString("@imgfile") ;
    else if( key == volfileico->cacheKey()) return QString("@volfile") ;
    else if( key == movfileico->cacheKey()) return QString("@movfile") ;
    else return QString("@file") ;
}




//从完整的文件路径插入文件
void BKEproject::AddFileToHash(BkeFilesHash *hash,const QString &filename)
{
    QString allfilename = filename ;   //全路径
    if( allfilename.indexOf(":") < 0) allfilename.prepend(FileDir()+"/") ;

    QFileInfo temp(allfilename) ;
    QString path = LOLI_OS_QSTRING( temp.path() );

    QStringList *list = hash->value(path, &emptylist) ;  //绝对路径是否已经在列表中
    if( list->isEmpty() ){       //不存在则创建
        list = new QStringList ;
        list->append( temp.fileName()) ;
        (*hash)[path] = list ;
    }
    else{
        list->append( temp.fileName())  ;   //存在则添加
    }
}

void BKEproject::removeFromHash(BkeFilesHash *hash,const QString &filename)
{
    QString allfilename = filename ;   //全路径
    if( allfilename.indexOf(":") < 0) allfilename.prepend(FileDir()+"/") ;

    QFileInfo temp(allfilename) ;
    QStringList *ls ;

    //没有.的被认为是目录，清空目录
    if( temp.fileName().indexOf('.') < 0 ){
        ls =  hash->value( LOLI_OS_QSTRING( allfilename) ) ;
        if( ls == 0) return ;
        ls->clear();
    }
    else{
        ls = hash->value( LOLI_OS_QSTRING( temp.path() ) ) ;
        if( ls == 0) return ;
        ls->removeOne(temp.fileName()) ;
    }

    if( ls->isEmpty() && temp.path().compare( FileDir(),Qt::CaseInsensitive ) != 0 ){
        hash->remove( LOLI_OS_QSTRING( temp.path() ) ) ;
        delete ls ;
    }

    return ;
}


//从hash中创建项目
void BKEproject::ItemFromHash(QTreeWidgetItem *dest,QHash<QString,QStringList*> &hash)
{
    QStringList *list ;
    QTreeWidgetItem *root ;

    for( auto ptr = hash.begin() ; ptr != hash.end() ; ptr++){
        list = ptr.value() ;
        if( list->size() < 1) continue ;

        root = FindItem(dest,ptr.key()) ;
        for(int i = 0 ; i < list->size() ; i++){
            QTreeWidgetItem *le = FindItem(root,list->at(i)) ;
            SetIconFromSuffix(le,list->at(i));
            root->addChild(le);
        }
    }
    SortTree( dest );
}


//排序,文件夹优先，大小写不敏感
void BKEproject::SortItem(QTreeWidgetItem *dest )
{
    QList<QTreeWidgetItem *> root = dest->takeChildren() ;
    QStringList dirlist,filelist ;
    QHash<QString,QTreeWidgetItem *> temp ;
    QTreeWidgetItem *le ;

    for( int i = 0 ; i < root.size() ; i++){
        le = root.at(i) ;
        if( le->icon(0).cacheKey() == dirsico->cacheKey()) dirlist.append(le->text(0));
        else filelist.append(le->text(0));
        temp[le->text(0)] = le ;
    }

    dirlist.sort(Qt::CaseInsensitive);
    filelist.sort(Qt::CaseInsensitive);
    for( int i = 0 ; i < dirlist.size() ; i++){
        dest->addChild(temp.value(dirlist.at(i)));
    }

    for( int i = 0 ; i < filelist.size() ; i++){
        dest->addChild(temp.value(filelist.at(i)));
    }
}

//排序整个目录
void BKEproject::SortTree(QTreeWidgetItem *tree)
{
    QTreeWidgetItem *le ;

    for( int i = 0 ; i < tree->childCount() ; i++){
        le = tree->child(i) ;
        if( le->childCount() > 0) SortTree(le);
    }
    SortItem(tree);
}

bool BKEproject::RemoveItem(QTreeWidgetItem *Item)
{
    QTreeWidgetItem *le ;
    for( int i = 0 ; i < Item->childCount() ;i++){
        if( Item->child(i)->childCount() > 0 ){
            RemoveItem(Item->child(i)) ;
        }
        delete Item->child(i) ;
    }

    le = Item->parent() ;
    if( le == 0) return true;
    le->removeChild(Item);
    return true ;
}

bool BKEproject::RemoveItem(ItemInfo &f)
{
    QTreeWidgetItem *le = FindItem(f.Root,f.FullName,false) ;
    if( le == 0) return false ;

    BkeFilesHash *h = typeHash(f.RootName) ;
    if( IconKey(f.IconKey) != "@dir" ){
        RemoveItem(le) ;
        delete le ;
        removeFromHash(h,f.FullName);
    }
    else{  //获取所有的子目录列表，从hash中移除
        QStringList ls = ItemDirs(le) ;
        ls << f.FullName ;
        RemoveItem(le) ;  //移除自身
        delete le ;

        for( int i = 0 ; i < ls.size() ; i++){
            removeFromHash(h,ls.at(i));
        }
    }
    return true ;
}



QHash<QString,QStringList*> *BKEproject::ItemToHashptr(const QTreeWidgetItem *root)
{
    if( root->text(0) == "初始化" ) return &ImportHash ;
    else if( root->text(0) == "脚本" ) return &ScriptHash ;
    else return &SourceHash ;
}

QStringList BKEproject::ListFiles(int type)
{
    QString path ;
    QStringList filelist ;
    QStringList *list ;
    QHash<QString,QStringList*> hash ;

    if( type == 0) hash = ImportHash ;
    else if( type == 1 ) hash = ScriptHash ;
    else hash = SourceHash ;

    for( auto ptr = hash.begin() ; ptr != hash.end() ; ptr++){
        path = ptr.key() ;
        list = ptr.value() ;
        for( int i = 0 ; i < list->size() ; i++){
            filelist << path + "/"+list->at(i) ;
        }
    }

    return filelist ;
}

QStringList BKEproject::AllScriptFiles()
{
    QStringList temp ;
    temp.append( ListFiles(0) );
    temp.append( ListFiles(1) );
    return temp ;
}

void BKEproject::copyStencil(const QString &file)
{

}

QTreeWidgetItem *BKEproject::FindItemAll(const QString &name)
{
    QTreeWidgetItem *le ;
    le = FindItem(Import,name,false) ;
    if( le == 0) FindItem(Script,name,false) ;
    else if( le == 0) FindItem(Source,name,false) ;
    return le ;
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

void BKEproject::SetTopLeveBold(bool t)
{
    QFont a = Root->font(0) ;
    a.setBold(t);
    Root->setFont(0,a);
}


bool BKEproject::WriteMarkFile(BkeMarkSupport *m)
{
    QString akb ;
    QStringList list = AllScriptFiles() ;
    BkeMarkList *mks ;
    for( int i = 0 ; i < list.size() ; i++ ){
        mks = m->GetBookMark(list.at(i),false) ;
        if( mks->isEmpty() ) continue ;

        akb.append(':'+list.at(i)+"\r\n") ;
        akb.append( MarksToString(mks) ) ;
    }

    return LOLI::AutoWrite(FileDir()+"/BkeProject.bmk",akb) ;
}

QString BKEproject::MarksToString(BkeMarkList *mk)
{
    QString result ;
    QString temp ;
    BkeMarkerBase *base ;
    for( int i = 0 ; i < mk->size() ; i++){
        base = mk->at(i) ;
        temp.setNum(base->Atpos) ;
        temp.append("/"+base->Information+"\r\n") ;
        result.append(temp) ;
    }
    return result ;
}

BkeFilesHash *BKEproject::typeHash(int type)
{
    if( type == 1) return &ImportHash ;
    else if( type == 2) return &ScriptHash ;
    else return &SourceHash ;
}

BkeFilesHash *BKEproject::typeHash(const QString &n)
{
    if( n == "初始化" ) return &ImportHash ;
    else if( n == "脚本" ) return &ScriptHash ;
    else return &SourceHash ;
}

//返回dest所有的子目录，不包括自身
QStringList BKEproject::ItemDirs(QTreeWidgetItem *dest)
{
    QStringList ls ;
    for( int i = 0 ; i < dest->childCount() ; i++){
        if( dest->child(i)->childCount() > 0) ls.append( ItemDirs(dest->child(i)));
    }
    return ls ;
}

void BKEproject::Addfiles(const QStringList &ls ,ItemInfo &f)
{
    QTreeWidgetItem *la ;
    BkeFilesHash *h1 ;
    workItem(&la,&h1,f);

    for( int i = 0 ; i < ls.size() ; i++){
        if( ls.at(i).endsWith(".bkscr") || ls.at(i).endsWith(".bkpsr") ){
            FindItem(la,ls.at(i)) ;
            AddFileToHash(h1,ls.at(i));
        }
        else{
            FindItem(Source,ls.at(i)) ;
            AddFileToHash(&SourceHash,ls.at(i));
        }
    }

    if( ls.size() > 0)  WriteBkpFile() ;
}

void BKEproject::AddDir(const QString &dir ,ItemInfo &f)
{
    BkeFilesHash k1,k2 ;
    SearchDir(k1,dir,".bkscr") ;
    SearchDir(k2,dir,".jpg.jpeg.png.bmp.ogg.wav.mp3.aac.mp4.avi.mpg") ;

    QTreeWidgetItem *la ;
    BkeFilesHash *h1 ;
    workItem(&la,&h1,f);

    if( !k1.isEmpty() ){
        ItemFromHash(la,k1);
        QTreeWidgetItem *le = FindItem(la,dir) ;
        SearchTree(*h1,le,dir);
    }
    if( !k2.isEmpty() ){
        ItemFromHash(Source,k2);
        QTreeWidgetItem *le = FindItem(Source,dir) ;
        SearchTree(SourceHash,le,dir);
    }
    WriteBkpFile() ;
}


void BKEproject::workItem(QTreeWidgetItem **la,BkeFilesHash **h1,ItemInfo &f)
{
    if( f.Layer < 1 || f.RootName == "资源"){
        *la = Script ;
        *h1 = &ScriptHash ;
    }
    else{
        *la = f.Root ;
        *h1 = typeHash(f.RootName) ;
    }
}


void BKEproject::CheckDir(BkeFilesHash *hash, const QString dirnow)
{
    QStringList ls = hash->keys() ;
    QString kdir = FileDir() ;

    BkeFilesHash th ;
    for( int i = 0 ; i < ls.size() ; i++){
        QString dd = BkeFullnameToName(kdir,ls.at(i)) ;
        th[ dirnow+"/"+dd ] = hash->value(ls.at(i)) ;
    }

    hash->clear();
    *hash = th ;
}
