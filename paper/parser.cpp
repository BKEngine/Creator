#include "parser.h"

QStringList SYSlist ;

QStringList KEYlist ;

BkeParser::BkeParser(QObject *parent)
    :QObject(parent)
{
    wordadmin = 0 ;
    ErrorFlag = false ;
    IndentLayer = 0 ;

#ifndef PARSER_PRO
    ParserText(BKE_API_FILE);  //载入默认的api列表
#endif

    SetVarList("..");
    SetCommandList("..");
}

BkeParser::~BkeParser()
{
}

void BkeParser::ParserText( const QString &t)
{
    if( t.isEmpty() ) return ;
    wordadmin = new WordSupport ;
    wordadmin->setText(t);
    IndentLayer = 0 ;

    QString key ;
    while( wordadmin->more ){

        key = wordadmin->NextWord2() ;
        if( key == "@" ){
            CommandStart() ;
        }
        else if( key == "##" ){
            ParserModel(false);
            wordadmin->SetParserModel(false);
        }
        else if( key == "*") defLabel();
        else ParserModel2();

        wordadmin->NextLine();  //转到下一行
        ErrorFlag = false ;
    }

    delete wordadmin ;
    #ifdef PARSER_PRO
    ErrorSub(-1,"测试完成");
    #endif
}

void BkeParser::ParserModel(bool issub)
{
    errorid = 0 ;
    wordadmin->SetParserModel(true);
    int k ;
    wordadmin->NextWord();
    while( wordadmin->more ){

        if( wordadmin->cWord == "##" )
            break ;
        else if( wordadmin->cWord == "function" ) defFunction();
        else if( wordadmin->cWord == "}" && issub) break ;
        else if( wordadmin->cWord == "var" ) SetVariable(-1,true);
        else if( wordadmin->cWord == "=" ){
            if( wordadmin->nWord == "%" ) defDictionary(&VariablesBase);
            else RightExp() ;
        }

        if( errorid < 0 ){
            wordadmin->NextLine();
        }

        if( wordadmin->IsAtEnd() ) wordadmin->NextLine();
        else wordadmin->NextWord();
    }
    return ;
}

void BkeParser::ParserModel2()
{
    while( wordadmin->more && !wordadmin->IsAtEnd() ){
        if( wordadmin->cWord == "function"  ) defFunction();
        else if( wordadmin->cWord == "var" ) SetVariable(-1,true);
        wordadmin->NextWord();
    }

    //分界符总是在末尾
    if( wordadmin->cWord == "{"  ) IndentLayer++ ;
    else if( wordadmin->cWord == "}"  ) IndentLayer-- ;
}

void BkeParser::PushParserText( const QString &t)
{
    StackWordadmin.push(wordadmin);
    ParserText(t);
    wordadmin = StackWordadmin.pop() ;
}

void BkeParser::CommandStart()
{
    QString name = wordadmin->NextWord2() ;
//    if( !wordadmin->IsCurrentSpace() ){
//        ErrorSub(wordadmin->ErrorPos(),"命令："+name+"后必须使用空格");
//        return ;
//    }

    int a = wordadmin->NowAt() ;
    while( !wordadmin->atLineEnd ){
        CLeftExp() ;
    }

    if( ErrorFlag ) return ;
    else if( name == "macro") defCommand(a);
    else if( name == "import" ) ImportFile();
    else if( name == "if" || name == "elseif" || name == "else" || name == "for") IndentLayer++ ;
    else if( name == "endif" || name == "next" ) IndentLayer-- ;

}

void BkeParser::ErrorSub(int pos,const QString &info)
{
    emit ErrorAt(pos,info);
    errorid = -1 ;
}

//@左表达式
bool BkeParser::CLeftExp()
{
    int ptype = -1 ;

    while( !wordadmin->atLineEnd ){
        wordadmin->NextWord();

        if( wordadmin->cWord.isEmpty() ) return true;
        //进入右值表达式模式
        else if( wordadmin->nWord == "=" && wordadmin->IsCurrentSpace()){
            ErrorSub(wordadmin->ErrorPos(),"“=”号之前不能使用空格");
        }
        else if( wordadmin->cWord == "=" && wordadmin->IsCurrentSpace()){
            ErrorSub(wordadmin->ErrorPos(),"“=”号之后的值不能为空");
        }
        else if( wordadmin->cWord == "=" ) CLeftExp() ;
        else if( wordadmin->cWord == "(" || wordadmin->cWord == "[") RightExp() ;
        else if( wordadmin->ctype == ptype && ptype == WordSupport::BKE_WORD_TYPE_OPEATOR ){
            ErrorSub(wordadmin->ErrorPos(),"运算符叠加");
        }

        if( wordadmin->IsCurrentSpace() ) return true ; //现在位置的后一个符号为空格，返回
        ptype = wordadmin->ctype ;
    }
    return true ;
}

//@右值表达式
bool BkeParser::CRightExp()
{
	return false;
}

//右表达式
int  BkeParser::RightExp(bool next)
{
    if( wordadmin->cWord == "=" || wordadmin->cWord == "(" || wordadmin->cWord == "[")
        wordadmin->NextWord();

    CompleteBase *le ;
    QString czb ;
    static int ea = 0 ;
    static int eb = 0 ;
    //af:从这里开始解析变量
    int af = wordadmin->NowAt() ;

    while( !wordadmin->IsAtEnd() ){

        //递归类
        if( wordadmin->cWord == "(" ){ ea++; RightExp(); }
        else if( wordadmin->cWord == "[" ){ eb++; RightExp();}
        else if( wordadmin->cWord == "{" ) ParserModel(true);
        //递归的返回
        else if( wordadmin->cWord == ")" || wordadmin->cWord == "]") return 0;
        else if( wordadmin->cWord == "}" ) return 1;

//        else if( wordadmin->cWord == "%" && wordadmin->nWord == "[" ){
//            le = VariablesBase.AddChild(czb,CompleteBase::BKE_TYPE_DICTIONARIES) ;
//            defDictionary(le);
//        }
        //过程错误
        else if( wordadmin->ptype == wordadmin->ctype ) ErrorSub(wordadmin->ErrorPos(),"语法错误：类型叠加");
        //只处理显式的定义
        else if( wordadmin->cWord == "var" ) SetVariable(-1,true);



        //错误标记
        if( wordadmin->IsAtEnd() && ea > 0) ErrorSub(wordadmin->ErrorPos(),"“(”号不匹配");
        if( wordadmin->IsAtEnd() && eb > 0) ErrorSub(wordadmin->ErrorPos(),"“]”号不匹配");
        if( errorid < 0 ) return -1 ;

        czb = wordadmin->pWord ; //更前一个
        wordadmin->NextWord();
    }
}


void BkeParser::defCommand(int pos)
{
    if( pos != -1 ) wordadmin->Gopos(pos);
    WordSupport w ;
    w.setText(wordadmin->WordUntilLineEnd(pos));

    if( w.NextWord2() != "name"){
        ErrorSub(w.ErrorPos()+pos,"必须使用name关键字");
        return ;
    }
    else if( w.NextWord2() != "=" ){
        ErrorSub(w.ErrorPos()+pos,"必须使用“=”号");
        return ;
    }

    QString name = w.NextWord2() ;

    if( !name.startsWith("\"") && !name.endsWith("\"")){
        ErrorSub(w.ErrorPos()+pos,"名称必须使用字符串");
        return ;
    }

    QString cinfo1("命令:") ;
    name = name.mid(1,name.length()-2) ;
    CompleteBase *le ;
    int i ;
    if(  (i=name.indexOf('#')) >= 0){
        le = CommandBase.AddChild(name.left(i),CompleteBase::BKE_TYPE_COMMAND) ;
        cinfo1.append(le->Name) ;
        name = name.right(name.length()-i-1).replace(QRegExp("\\\\"),"") ;
        le = le->AddChild(name,CompleteBase::BKE_TYPE_COMMAND) ;
        cinfo1.append(" " + le->Name+" 参数:") ;
    }
    else{
        le = CommandBase.AddChild(name,CompleteBase::BKE_TYPE_COMMAND) ;
        cinfo1.append(le->Name+" 参数:") ;
    }

    //添加定义说明
    QString cinfo = w.GetRightLine() ;
    cinfo.replace(QRegExp("//"),"\n") ;
    le->functioninfo = cinfo1 + cinfo ;

    StackWordadmin.push(wordadmin);
    wordadmin = &w ;
    while( !w.atLineEnd ){
        name = w.NextWord2();
        if( w.cWord.isEmpty() ) break ;

        if( w.nWord == "=" ){
            w.NextWord();
            //RightExp() ;
            CLeftExp() ;
        }
        else if( w.nWord == "#" ){
            w.NextWord();
        }

        le->AddChild(name,CompleteBase::BKE_TYPE_COMMAND) ;
    }

    wordadmin = StackWordadmin.pop() ;
}


const char* BkeParser::GetCommandList( const QString &t)
{
    if( apiBase == 0 ) return 0 ;

    QStringList list = apiBase->TheWords() ;
    if( list.isEmpty() ) return 0 ;
    else if( t.isEmpty() ){
        list.sort(Qt::CaseInsensitive);
        outbyte = list.join(" ").toUtf8() ;
        outlist = list ;
    }
    else{
        QStringList s ;
        for( int i = 0 ; i < list.size() ; i++){
            if( list.at(i).startsWith(t) ) s.append(list.at(i));
        }
        s.sort(Qt::CaseInsensitive);
        outbyte = s.join(" ").toUtf8() ;
        outlist = s ;
    }

    return outbyte.constData() ;
}

const char* BkeParser::GetValList( const QString &t)
{

    if( KeyFlag ) valBase = &EmptyBase ;

    if( valBase == 0 ) return 0 ;

    QStringList list = valBase->TheWords() ;
    if( valBase == &VariablesBase ) list.append( KEYlist );
    else list.append( SYSlist );

    if( t.isEmpty() ){
        list.sort(Qt::CaseInsensitive);
        outbyte = list.join(" ").toUtf8() ;
    }
    else{
        QStringList s = list.filter(QRegExp("^"+t,Qt::CaseInsensitive)) ;
        s.sort(Qt::CaseInsensitive);
        outbyte = s.join(" ").toUtf8() ;
        outlist = s ;
    }
    return outbyte.constData() ;
}

const char* BkeParser::GetLabelList( const QString &t)
{
    QStringList ls = VariablesBase.TheWords() ;
    QRegExp exp(QString(t+".*[?]%1").arg(CompleteBase::BKE_TYPE_LABEL),Qt::CaseInsensitive) ;
    ls = ls.filter(exp) ;
    outbyte = ls.join(" ").toUtf8() ;
    outlist = ls ;
    return outbyte.constData() ;
}


//通常获取
const char* BkeParser::GetList(const QString &t)
{

    if( showtype == SHOW_NULL ) return 0 ;
    if( showtype == SHOW_USEVALLIST || showtype == SHOW_AUTOVALLIST) return GetValList(t) ;
    else if( showtype == SHOW_LABEL ) return GetLabelList(t) ;
    else if( showtype == SHOW_SYS ) return GetValList(t) ;
    else return GetCommandList(t) ;
}

void BkeParser::SetCommandList(const QString &t)
{
    if( t == ".." ) apiBase = &CommandBase ;
    else if( apiBase != 0){
        apiBase = apiBase->indexOf(t,CompleteBase::BKE_TYPE_COMMAND) ;
    }
}

void BkeParser::SetVarList(const QString &t)
{
    CompleteBase *le ;
    if( t == ".." ) valBase = &VariablesBase ;
    else if( valBase != 0 ){
        le = valBase->indexOf(t,CompleteBase::BKE_TYPE_NORMAL) ;
        if( le == 0) le = valBase->indexOf(t,CompleteBase::BKE_TYPE_DICTIONARIES) ;
        else if( le == 0) le = valBase->indexOf(t,CompleteBase::BKE_TYPE_FUNCTION) ;
        valBase = le ;
    }
}

bool BkeParser::IfHasThenSet(const QString &t)
{
    CompleteBase *le = apiBase->indexOf(t,CompleteBase::BKE_TYPE_COMMAND) ;
    if( le == 0 || !le->hasChild() ) return false ; //不存在的树枝，或者不存在子树枝的树枝
    else apiBase = le ;
    return true ;
}


bool BkeParser::HasTheChileOf(const QString &t)
{
    CompleteBase *le = apiBase->indexOf(t,CompleteBase::BKE_TYPE_COMMAND) ;
    if( le == 0 || !le->hasChild() ) return false ; //不存在的树枝，或者不存在子树枝的树枝
    else return true ;
}

//通常的加入api
void BkeParser::SetVariable(int pos,bool toend)
{
    if( pos < 0) pos = wordadmin->NowAt() ;
    int k = wordadmin->NowAt() ;

    wordadmin->Gopos(pos);
    if( wordadmin->cWord == "var" ) wordadmin->NextWord();
    wordadmin->NextWord();  //使得pword为关键字

    CompleteBase *le = &VariablesBase ;
    while( wordadmin->ptype == WordSupport::BKE_WORD_TYPE_ID ){

        if( wordadmin->cWord == "(" ){
            le = le->AddChild(wordadmin->pWord+"()",CompleteBase::BKE_TYPE_FUNCTION) ;
            RightExp() ;
        }
        else if( wordadmin->cWord == "=" && wordadmin->nWord == "%" ){
            le = VariablesBase.AddChild(wordadmin->pWord,CompleteBase::BKE_TYPE_DICTIONARIES) ;
            defDictionary(le);
        }

        else le = le->AddChild(wordadmin->pWord,CompleteBase::BKE_TYPE_NORMAL) ;

        if( wordadmin->cWord == "[" ) RightExp() ;
        if( wordadmin->cWord != "." ) break ;

        wordadmin->NextTwoWord();
    }


    if( !toend ) wordadmin->Gopos(k);
    return ;
}

void BkeParser::defFunction()
{
    wordadmin->NextWord();
    if( !wordadmin->IsCurrentType(WordSupport::BKE_WORD_TYPE_ID) ) return ;
    else if( wordadmin->nWord != "(" ) return ;
    VariablesBase.AddChild(wordadmin->cWord+"()",CompleteBase::BKE_TYPE_FUNCTION) ;
    RightExp() ;
    return ;
}

void BkeParser::defDictionary(CompleteBase *le)
{
    if( wordadmin->cWord == "=" && wordadmin->nWord == "%")
        le = le->AddChild(wordadmin->pWord,CompleteBase::BKE_TYPE_DICTIONARIES) ;
    while( wordadmin->cWord == "=" || wordadmin->cWord == "%"
           ||wordadmin->cWord == "[") wordadmin->NextWord();

    while( !wordadmin->cWord.isEmpty() ){

        if( wordadmin->cWord == "=" && wordadmin->nWord == "%"){
            le = le->AddChild(wordadmin->pWord,CompleteBase::BKE_TYPE_DICTIONARIES) ;
            defDictionary(le);
        }
        else if(wordadmin->cWord == "=" && wordadmin->nWord == ">"){
            le->AddChild(wordadmin->pWord,CompleteBase::BKE_TYPE_NORMAL) ;
        }
        else if( wordadmin->cWord == "]" ) return ;
        else if( wordadmin->cWord == ";" || wordadmin->cWord == "}" ){
            ErrorSub(wordadmin->ErrorPos(),"定义字典出现了异常的结尾");
            return ;
        }

        else wordadmin->NextWord();
    }

}

//定义标签
void BkeParser::defLabel()
{
    wordadmin->NextWord();
    if( !wordadmin->IsCurrentType(WordSupport::BKE_WORD_TYPE_ID) ) return ;
    VariablesBase.AddChild(wordadmin->cWord,CompleteBase::BKE_TYPE_LABEL) ;

}

void BkeParser::ParserFile(const QString &file,const QString &dirfrom)
{
    if( !QFile(file).exists() ) return ;
    dir = dirfrom ;
    importfile.clear();

    QString t ;
    LOLI::AutoRead(t,file) ;
    if( t.isEmpty() ) return ;

    ParserText( t );
}

void BkeParser::ImportFile()
{
    wordadmin->NextTwoWord();
    QString f = wordadmin->NextWord2();
    if( f.startsWith('\"') ) f = f.right(f.length()-1 ) ;
    if( f.endsWith('\"') ) f = f.left(f.length()-1) ;

    QString ka ;
    if( !LOLI::AutoRead( ka,dir+"/"+f  ) ) return ;

    importfile << f ;
    PushParserText(ka);
}

//文本被改变
void BkeParser::TextBeChange(BkeModifiedBase *modbase,QsciScintilla *sciedit)
{
    showtype = SHOW_NULL ;
    KeyFlag = false ;
    if( sciedit->hasSelectedText() ) return ;

    //如果有自动完成列表，则不显示

    if( !(modbase->type&QsciScintilla::SC_MOD_INSERTTEXT) ) return ; //不是插入文本不分析
    if( modbase->lineadd > 0){
        QString t ;
        //有多行被改变时，自动检查前几行
        for( int i = modbase->line - modbase->lineadd ; i < modbase->line ; i++){
            t.append( sciedit->text(i) ) ;
        }
        ParserText(t);
        return ;
    }
    else if( modbase->text.length() > 1) return ; //改变多个字，并没有增加新行，不做检查
    else if( modbase->text == " " ) return ; //输入的是空格，什么也不会发生

    QString t = sciedit->text( modbase->line ) ;
    WordSupport wow ;
    //光标右侧不是一个有效的分割符，则不自动完成
    if( modbase->EndPos() < t.length()-2 && !wow.IsSeparate(t[modbase->EndPos()]) ){
        return ;
    }

    //优先检查光标左侧，确定显示的具体内容
    QStringList ls = wow.context(t,modbase->index-1) ;
    if( ls.size() > 0 && ls.at(0) == "*" ){   //显示标签，不是严谨的界定
        showtype = SHOW_LABEL ;
        return ;
    }


    SetVarList("..");
    int st = ls.size() - 1  ;
    if( modbase->text == "." ){
        st++ ;  //如果输入的是.那么填充的参数为列表最后
        showtype = SHOW_USEVALLIST ;
    }
    else showtype = SHOW_AUTOVALLIST ;
    for( int i = 1 ; i < st ; i++) {
        SetVarList(ls.at(i)) ;
    }

    if( ls.size() > 1 && valBase == 0){
        showtype = SHOW_SYS ;
        KeyFlag = true ;
    }


    //=============================
    t = t.left(modbase->index+1) ;
    wow.setText(t);
    if( wow.NextWord2() != "@" && wow.cWord != "[" ) return ;

    SetCommandList("..");
    QString key1 = wow.NextWord2() ;
    QString key2 = wow.NextWord2() ;

    if( showtype == SHOW_NULL ) showtype = SHOW_AUTOCOMMANDLIST ;

    if( key1.isEmpty() ) showtype = SHOW_USECOMMANDLIST ;

    if( !key1.isEmpty() && IfHasThenSet(key1) && !key2.isEmpty() && IfHasThenSet(key2))  ;
    if( key2.isEmpty() ) showtype = SHOW_AUTOCOMMANDLIST ;

    if( modbase->text == ";" ) showtype = SHOW_USECOMMANDLIST ;
    else if( ls.size() > 0 && ls.at(0) == ";" ) showtype = SHOW_AUTOCOMMANDLIST ;

}

//取得缩进层次
int  BkeParser::GetIndentLayer(QsciScintilla *edit,int line)
{
    QString t = edit->text(line).trimmed() ;

    WordSupport w ;
    w.setText(t);
    w.NextWord();

    if( t.endsWith("{") ) return 1 ;
    else if( w.cWord == "}"  ) return GetLessIndent(edit,line) ;
    else if( w.cWord == "@" ){
        if( w.nWord =="if" || w.nWord == "for" ) return 1 ;
        else if( w.nWord == "elseif" || w.nWord == "next" || w.nWord == "else"
                 || w.nWord == "endif"){
                QString tt ;
                if( line > 1 ) tt = edit->text(line-1).trimmed() ;

                if( tt.isEmpty() ) return 0 ;
                else if( tt.startsWith("@if") || tt.startsWith("@for") ) return 0 ;
                else return GetLessIndent(edit,line) ;
        }
        else return 0 ;
    }
    else return 0 ;
}

int  BkeParser::GetLessIndent(QsciScintilla *edit,int line)
{
    int pos_f , pos_n ;
    if( line < 1) pos_f = 0 ;
    else pos_f = edit->indentation(line-1) ;
    pos_n = edit->indentation(line) ;

    if( pos_n >= pos_f ) return -1 ;
    else return 0 ;
}

//取得信息
QString BkeParser::GetInfo(const QString &t)
{
    if( t.isEmpty() ) return QString();

    WordSupport w ;
    w.setText(t);

    if( w.NextWord2() != "@" ) return QString() ;

    CompleteBase *le = CommandBase.indexOf(w.NextWord2(),CompleteBase::BKE_TYPE_COMMAND) ;
    if( le == 0) return QString() ;
    CompleteBase *le1 = le->indexOf( w.NextWord2(),CompleteBase::BKE_TYPE_COMMAND ) ;
    if( le1 != 0 && le1->hasChild() ) le = le1 ;

    return le->functioninfo ;
}
