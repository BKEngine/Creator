#include "bkemarks.h"


void BkeMarkSupport::SetFile(const QString &file)
{
    Name = file ;
    QString cfile = LOLI_OS_QSTRING( file ) ;
    currentbookmarklist = MarkListOf(cfile,BKE_MARK_BOOKMARK) ;
    currentmarklist = MarkListOf(cfile,BKE_MARK_MARKER) ;
    currentproblemlist = MarkListOf(cfile,BKE_MARK_PROBLEEM) ;
}

//从编译输出文本中提取错误信息
void BkeMarkSupport::ProblemsFromText(const QString &dir,const QString &text)
{
    ClearMarks(BKE_MARK_PROBLEEM);
    QStringList tk = text.split("\r\n") ;
    QString temp ;

    errorcount = 0 ;
    for( int i = 0 ; i < tk.size() ; i++){
        temp = tk.at(i) ;
        if( temp.endsWith(".bkscr") || temp.endsWith(".bkpsr")){
            SetFile(dir+"/"+temp);
        }
        else if( temp.startsWith("error" )  ){
            AddProblem(temp);
            errorcount++ ;
        }
        else if( temp.startsWith("warning") ){
            AddProblem(temp);
        }
    }
}

void BkeMarkSupport::AddProblem(QString text)
{
    BkeMarkerBase *c = new BkeMarkerBase ;
    if( text.startsWith("error")) c->Type = 1 ;
    else c->Type = 2 ;

    c->FullName = Name ;
    int k = Name.lastIndexOf("/") ;
    if( k < 0) k = Name.lastIndexOf("\\") ;
    c->Name = Name.right(Name.length()-k-1) ;
    c->Information = ReadProblem(c->Atpos,text) ;
    currentproblemlist->append(c);
}

QString BkeMarkSupport::ReadProblem(int &pos,QString &problem)
{
    pos = 1 ;
    int k = problem.indexOf("：") ;
    if( k < 0) return QString();

    QString temp = problem.left(k) ;
    int a = temp.indexOf("第") ;
    if( a < 0) return problem.right(problem.length()-k-1);
    int b = temp.indexOf("行",a) ;
    if( b < 0) return problem.right(problem.length()-k-1) ;

    pos = temp.mid(a+1,b-a-1).toInt() ;
    return problem.right(problem.length()-k-1) ;
}

void BkeMarkSupport::AddBookMark(const QString &info,int pos,const QString &dir)
{
    BkeMarkerBase *abc = new BkeMarkerBase ;
    abc->FullName = Name ;
    abc->Information = info ;
    abc->Atpos = pos + 1 ;
    abc->Name = dir ;

    currentbookmarklist->append(abc);
}

//输出一个使用某outXXXlist的指针
BkeMarkList* BkeMarkSupport::GetFileMarker(const QString &file, int type, bool showall)
{
    BkeMarkHash *hash = HashFromType(type) ;
    QString cname = LOLI_OS_QSTRING(file) ;
    BkeMarkList *list = OutListFromType(type) ;
    list->clear();

    if( !showall ){
        BkeMarkList *ss = hash->value(cname,0) ;
        if( ss == 0) return list ; //文件没有标记，直接返回清理过后的空白对外输出
        //拷贝一份内容
        *list = *ss ;
        return list ;
    }

    //文件排在首位
    if( hash->contains(cname)) list->append(*(hash->value(cname)));

    for( auto ptr = hash->begin() ; ptr != hash->end() ; ptr++){
        if( ptr.key() == cname) continue ;
        list->append(*(ptr.value()));
    }
    return list ;
}


//根据文件及类型，返回list，不存在将创建
BkeMarkList* BkeMarkSupport::MarkListOf(QString &file,int type)
{
    BkeMarkHash *hash = HashFromType(type) ;
    if( hash->contains(file) ) return hash->value(file) ;
    else{
        BkeMarkList* temp = new BkeMarkList;
        (*hash)[file] = temp ;
        return temp ;
    }
}

//根据类型获取hash
BkeMarkHash* BkeMarkSupport::HashFromType(int type)
{
    if( type == BKE_MARK_BOOKMARK ) return &bookmarkhash ;
    else if( type == BKE_MARK_MARKER ) return &markhash ;
    else return &problemhash ;
}

//根据类型获取对外输出list
BkeMarkList* BkeMarkSupport::OutListFromType(int type)
{
    if( type == BKE_MARK_BOOKMARK ) return &outbookmarkerlist ;
    else if( type == BKE_MARK_MARKER ) return &outmarklist ;
    else return &outproblemlist ;
}

BkeMarkList *BkeMarkSupport::GetPrombleMark(const QString &file ,bool all )
{
    return GetFileMarker(file,BKE_MARK_PROBLEEM,all) ;
}

BkeMarkList *BkeMarkSupport::GetBookMark(const QString &file ,bool all )
{
    return GetFileMarker(file,BKE_MARK_BOOKMARK,all) ;
}

BkeMarkList *BkeMarkSupport::GetMarks(const QString &file ,bool all )
{
    return GetFileMarker(file,BKE_MARK_MARKER,all) ;
}

void BkeMarkSupport::ClearMarks(int type)
{
    BkeMarkHash *hash = HashFromType(type) ;
    BkeMarkList *list ;
    for( auto i = hash->begin() ; i != hash->end() ; i++ ){
        list = i.value() ;
        for( int k = 0 ; k < list->size() ; k++){
            delete list->at(k) ;  //删除markbase
        }
        list->clear();  //list不会被删除
    }
}

void BkeMarkSupport::BookMarksFromText(const QString &text,const QString &dir)
{
    QString ea = Name ;

    QString ep ;
    QStringList lz,ls = text.split("\r\n") ;
    for( int i = 0 ; i < ls.size() ; i++){
        if( ls.at(i).startsWith(':') ){
            ep = ls.at(i).right( ls.at(i).length()-1) ;
            SetFile(ep);
            ep = ep.right(ep.length()-dir.length()-1) ;
        }
        else if( ls.at(i).isEmpty() ) continue ;
        else{
            lz = ls.at(i).split('/') ;
            AddBookMark(lz.at(1),lz.at(0).toInt(),ep);
        }
    }
    //重置为原来的文件
    SetFile( ea ) ;
}

