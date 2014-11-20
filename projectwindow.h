#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QDialogButtonBox>
#include "weh.h"
#include "topbarwindow.h"
#include "bkeproject.h"
#include "dia/bkeconfiguimodel.h"

extern QList<BKEproject*> projectlist ;


// 项目树窗口
class ProjectWindow : public QTreeWidget
{
    Q_OBJECT
public:
    enum{
        btn_active,
        btn_compile ,
        btn_release ,
        btn_insertdir ,
        btn_preview ,
        btn_newscript ,
        btn_newimport ,
        btn_addfile ,
        btn_adddir ,
        btn_showindir ,
        btn_search ,
        btn_remove ,
        btn_close ,
        BTN_COUNT
    };


    ProjectWindow(QWidget *parent = 0);
    BKEproject *FindPro(const QString &proname) ;
    BKEproject *FindFileProject(const QString &file) ;
    BKEproject *FindProjectFromDir(const QString &dir) ;
    void OpenProject(const QString &file) ;
    QList<BKEproject*> ProjectList(){ return projectlist ; }
    bool hasProject(){ return projectlist.size() > 0 ; }

    BKEproject *workpro ;

signals:
    void OpenThisFile(const QString &file,const QString &prodir) ;
    void DirWillBeInsert(const QString &text) ;
    void CheckFileOpen(const QString &file,bool &IsOpen) ;
    void ImportFileChange(const QString &text,int type) ;
    void FileNameChange(const QString &oldname,const QString &newname,bool &c) ;
    void CurrentProChange(BKEproject *pro) ;
    void Compile() ;
    void TextToMarks(const QString &text,const QString &dir,int type) ;


public slots:
    void NewProject() ;
    void OpenProject() ;
    void ItemDoubleClick(QTreeWidgetItem * item, int column) ;
    void ShowRmenu( const QPoint & pos ) ;
    void SetCurrentItem(const QString &file) ;
    void ReName() ;
    void OpenFile() ;
    void ActionAdmin() ;


private:
    //QList<BKEproject*> projectlist ;
    BKEproject *temppro ;
    ItemInfo info ;


    QAction *btns[BTN_COUNT] ;

    bool ReadItemInfo(QTreeWidgetItem *dest,ItemInfo &f) ;
    void BkeChangeCurrentProject(BKEproject *p) ;
    void ConfigProject(BkeProjectConfig *config) ;
    void NewFile(const ItemInfo &f, int type) ;
    void DeleteFile(const ItemInfo &f) ;
    void Addfiles(const ItemInfo &f) ;
    void AddDir(const ItemInfo &f) ;
    void PreviewFile(const ItemInfo &f) ;
    void CloseProject(const ItemInfo &f);
    void Active(const ItemInfo &f);
    void ShowInDir(const ItemInfo &f);
    QTreeWidgetItem *findFileInProject(const QString &name) ;
};



#endif // PROJECTWINDOW_H
