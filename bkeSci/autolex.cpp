#include "autolex.h"

AutoLex::AutoLex()
{
    empty = true ;
    ReadMacroFile(BKE_CURRENT_DIR,"command.api");

    BuildRootHash();
}

//从程序安装路径下读取command.api，返回所有导入的列表
QString AutoLex::ReadMacroFile(const QString &dir,const QString &name)
{
    QString imfile = dir + "/" + name + "\r\n";
    QFile ai(dir+"/"+name) ;
    ai.open(QFile::ReadOnly) ;
    QTextStream tk(&ai) ;

    QString com = LOLI_MID(tk.readAll(),"*register","[return]") ;
    if( com.isEmpty()) return imfile ;
    QStringList comlist = com.split("\r\n") ;
    ai.close();

    QString cname ;
    for( int i = 0 ; i < comlist.size() ; i++){
        com = comlist.at(i).trimmed() ;
        cname = LOLI_MID(com,"\"","\"") ;

        if( com.startsWith("macro")){
            ApiHash[cname] = TypeFormat(com) ;
        }
        else if( com.startsWith("impor")){ //导入文件
            if( cname.indexOf(":") < 0){ //相对路径加上相对路径
                cname.append(dir+"/") ;
            }
            QString cdir = cname.left( cname.lastIndexOf("/") ) ;
            cname = cname.right(cname.length()-cname.lastIndexOf("/")-1) ;
            imfile.append( ReadMacroFile(cdir,cname)) ;
        }
    }

    return imfile ;
}

QString AutoLex::TypeFormat(const QString &text)
{
    QString result,temp ;
    QStringList tk = temp.split(" ") ;
    int pos ;
    for( int i = 0 ; i < tk.size() ; i++){
        temp = tk.at(i) ;
        if( temp.isEmpty()) continue ;
        if( (pos = temp.lastIndexOf("=")) >= 0 ){
            result += temp.left(pos)+"?1 " ;
        }
        else result += tk.at(i)+"?2 " ;
    }
    return result.trimmed() ;
}

//返回支持的单词列表
const char* AutoLex::WordListOf(const QString &startword)
{
    if( startword == ".."){
        WordListData = WordList.join(" ").toUtf8();
        return WordListData.constData() ;
    }

    QStringList list ;
    for( int i = 0 ; i < WordList.size() ; i++){
        if( WordList.at(i).startsWith(startword)) list << WordList.at(i) ;
    }

    if( list.size() < 1) return 0 ;
    WordListData = list.join(" ").toUtf8() ;
    return WordListData.constData() ;
}

//设置Api列表，成功返回真，失败返回假 、
bool AutoLex::SetApiListOf(const QString &startword)
{
//    //记录下上一次的根命令，以便外部使用，如果不是二级命令立即清除
//    if( RootCommand == "animate" || RootCommand == "action") LastRootCom = RootCommand ;
//    else LastRootCom.clear();

    RootCommand = startword ;
    WordList.clear();
    QString temp = ApiHash.value(startword,"#") ;
    if( temp == "#") return false ;
    WordList = temp.split(" ") ;
    return true ;
}

//历遍Hash，生成根节点
void AutoLex::BuildRootHash()
{
    QString root ;
    QHash<QString,QString>::const_iterator i ;

    for( i = ApiHash.begin() ; i != ApiHash.end() ; i++){
        if( i.key().indexOf("#") < 0 ) root.append( i.key() +"?1 ") ;
    }
    ApiHash["##06##"] = root.trimmed() ;
}

//从api列表中移除某一单词
bool AutoLex::RemoveWordFromList(const QString &word)
{
    int pos = WordList.indexOf(word+"?1") ;
    if( pos < 0) pos = WordList.indexOf(word+"?2") ;
    if( pos < 0 ) return false ;
    WordList.removeAt(pos);
    return true ;
}


 QStringList AutoLex::LineToWords(QString &text)
{

}
