#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QDialog>
#include <QDockWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include "bkeSci/bkescintilla.h"
#include <QSplitter>
#include <weh.h>

class SearchBox : public QDockWidget
{
    Q_OBJECT
public:
    explicit SearchBox(QWidget *parent = 0);
    void SetSci(BkeScintilla *sci);

signals:


public slots :
	void onDocChanged();
	void onSelectionChanged();

public slots:
    void FindNext() ;
    void FindLast() ;
    void SearchModel() ;
    void ReplaceModel() ;
    void ChangeModel() ;
    void ReplaceText() ;
    void ReplaceAllText() ;
    void Show_() ;
private:
    QVBoxLayout *h1 ;
    QVBoxLayout *h2 ;
    QHBoxLayout *v1 ;

    QPushButton *btnsearchlast ;
    QPushButton *btnsearchnext ;
    QPushButton *btnreplacemodel ;
    QPushButton *btnreplace ;
    QPushButton *btnreplaceall ;
    QCheckBox *iscase ;
    QCheckBox *isregular ;
    QCheckBox *isword ;
    QCheckBox *isalwaysbegin ;
    QLineEdit *edit ;
    QLineEdit *edit1 ;
    QLabel *lable1 ;
    QLabel *lable2 ;
    BkeScintilla *sciedit ;
    QString fstr ;
    bool firstshow ;
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // SEARCHBOX_H
