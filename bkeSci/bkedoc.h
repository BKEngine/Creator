#ifndef BKEDOC_H
#define BKEDOC_H

#include "QHash"
#include "bkescintilla.h"
#include "dia/compileproblem.h"
#include "bkeproject.h"

class BKEdoc
{
public:
    BKEdoc();
    BkeScintilla *WorkingFile(const QString &file,bool mkNull = true ) ;
    void          SetFileProject(const QString &file, BKEproject *pro) ;
    BKEproject*   FileProject(const QString &file) ;
    bool          CloseFile(QWidget *win) ;
    QString       WinOfFile(QWidget *win) ;
    void          SetCurrentFile(const QString &file) ;
    QStringList   WorkFileList( int &currentpos) ;


    int FileCount(){ return sciHash.size(); }
    QString GetError(){ return errors ; }

private:
    QHash<QString,BkeScintilla*> sciHash ;
    QHash<QWidget*,QString> winHash ;
    QHash<QString,BKEproject*> proHash ;
    QStringList FileList ;
    QString currentfile ;
    QString errors ;

};

#endif // BKEDOC_H
