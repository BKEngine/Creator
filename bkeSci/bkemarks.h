#ifndef COMPILEPROBLEM_H
#define COMPILEPROBLEM_H
#include "weh.h"
#include <Qsci/qsciscintilla.h>

class BkeMarkerBase
{
public:
    QString FullName ; //全路径
    QString Name ;  //相对路径
    QString Information ;   //对于问题，是信息，对于标签是用户输入标记名称
    int Type ;
    int Atpos ;

};

typedef QList<BkeMarkerBase*> BkeMarkList ;
typedef QHash<QString,BkeMarkList*> BkeMarkHash ;


//统管bke记号类
class BkeMarkSupport
{
public:

    enum MARKTYPE{
        BKE_MARK_PROBLEEM = 3 ,
        BKE_MARK_BOOKMARK = 4 ,
        BKE_MARK_MARKER = 8
    };
    int errorcount ;

    void SetFile(const QString &file) ;

    void SetProblemText() ;
    void AddProblem(QString text) ;
    void AddBookMark(const QString &info,int pos,const QString &dir) ;
    void ProblemsFromText(const QString &dir,const QString &text) ;
    void BookMarksFromText(const QString &text,const QString &dir) ;
    void MarksFromText(const QString &text) ;

    BkeMarkList *GetPrombleMark(const QString &file = QString(),bool all = false) ;
    BkeMarkList *GetBookMark(const QString &file = QString(),bool all = false) ;
    BkeMarkList *GetMarks(const QString &file = QString(),bool all = false) ;
    BkeMarkList* OutListFromType(int type) ;

private:
    QString Name ;
    BkeMarkHash bookmarkhash ;
    BkeMarkHash problemhash ;
    BkeMarkHash markhash ;
    BkeMarkList* currentproblemlist ;
    BkeMarkList* currentbookmarklist ;
    BkeMarkList* currentmarklist ;
    //对外输出*bkemarklist时，总是改变outlist，相当于一个缓存
    BkeMarkList outproblemlist ;
    BkeMarkList outbookmarkerlist ;
    BkeMarkList outmarklist ;

    BkeMarkList* MarkListOf(QString &file,int type) ;
    BkeMarkHash* HashFromType(int type) ;

    void ClearMarks(int type) ;
    QString ReadProblem(int &pos,QString &problem) ;
    BkeMarkList* GetFileMarker(const QString &file,int type,bool showall = false) ;


};

#endif // COMPILEPROBLEM_H
