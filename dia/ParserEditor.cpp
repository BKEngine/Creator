#include "ParserEditor.h"
#include "ui_ParserEditor.h"
#include "ParserHelper/ParserHelper.h"
#include <QMessageBox>
#include "loli/loli_island.h"
#include "ParserEditorTreeItem.h"
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>

class StringDelegate : public QStyledItemDelegate
{
public:
	explicit StringDelegate(QObject *parent = 0) :QStyledItemDelegate(parent) {}
	QWidget *createEditor(QWidget *parent,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QLineEdit *edit = new QLineEdit(parent);
		return edit;
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QLineEdit *edit = (QLineEdit *)editor;
		ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
		edit->setText(item->data(index.column()).toString());
	}
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QLineEdit *edit = (QLineEdit *)editor;
		ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
		item->setData(index.column(), edit->text());
	}

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		editor->setGeometry(option.rect);
	}
};

class TypeDelegate : public QStyledItemDelegate
{
public:
	explicit TypeDelegate(QObject *parent = 0) :QStyledItemDelegate(parent) {}
	QWidget *createEditor(QWidget *parent,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QComboBox *box = new QComboBox(parent);
		box->addItems(ParserEditorTreeItem::allTypeStrings());
		return box;
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QComboBox *box = (QComboBox *)editor;
		ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
		box->setCurrentText(item->typeString());
	}
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		QComboBox *box = (QComboBox *)editor;
		ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
		item->setTypeString(box->currentText());
	}

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const Q_DECL_OVERRIDE
	{
		editor->setGeometry(option.rect);
	}
};

void ParserEditor::removeButtons()
{
	x->setVisible(false);
	a->setVisible(false);
	e->setVisible(false);
}

void ParserEditor::addButtons(const QModelIndex &index)
{
	QRect rect = ui->treeView->visualRect(index);
	int y = rect.top() + (rect.height() - 15) / 2;
	int x1 = rect.right() - 15;
	if (model->removeable(index))
	{
		x->move(x1, y);
		x->setVisible(true);
		x1 -= 16;
	}
	if (model->addable(index))
	{
		a->move(x1, y);
		a->setVisible(true);
		x1 -= 16;
	}
	if (model->editable(index))
	{
		e->move(x1, y);
		e->setVisible(true);
	}
	
}

void ParserEditor::buildToolButtons()
{
	QWidget *w = ui->treeView->viewport();
	{
		x = new QToolButton(w);
		x->setIcon(QIcon(":/pedit/source/x.png"));
		x->setFixedSize(QSize(15, 15));
		x->setVisible(false);
		connect(x, &QToolButton::clicked, this, &ParserEditor::onXClicked);
		x->setWhatsThis("删除该条目");
	}
	{
		a = new QToolButton(w);
		a->setIcon(QIcon(":/pedit/source/+.png"));
		a->setFixedSize(QSize(15, 15));
		a->setVisible(false);
		connect(a, &QToolButton::clicked, this, &ParserEditor::onAClicked);
		a->setWhatsThis("在该字典/数组中新增一个条目");
	}
	{
		e = new QToolButton(w);
		e->setIcon(QIcon(":/pedit/source/edit.png"));
		e->setFixedSize(QSize(15, 15));
		e->setVisible(false);
		connect(e, &QToolButton::clicked, this, &ParserEditor::onEClicked);
		e->setWhatsThis("编辑该项值");
	}
}

void ParserEditor::onTreeViewRClick(const QPoint &pt)
{
	QModelIndex index = ui->treeView->indexAt(pt);
	ParserEditorTreeItem *item = model->item(index);
	QMenu mu;
	if (model->editable(index))
	{
		QAction *e = new QAction("编辑", &mu);
		connect(e, &QAction::triggered, [this,&index]() {
			ui->treeView->edit(index);
		});
		mu.addAction(e);
		mu.addSeparator();
	}
	if (model->insertable(index))
	{
		QAction *e = new QAction("插入条目", &mu);
		connect(e, &QAction::triggered, [this, &index]() {
			model->insertRow(index.row() + 1, model->parent(index));
		});
		mu.addAction(e);
	}
	if (model->addable(index))
	{
		QAction *e = new QAction("添加子条目", &mu);
		connect(e, &QAction::triggered, [this, &index]() {
			model->insertRow(-1, index);
			ui->treeView->expand(index);
		});
		mu.addAction(e);
	}
	if (model->insertable(index))
	{
		QAction *e = new QAction("克隆", &mu);
		connect(e, &QAction::triggered, [this, &index]() {
			model->duplicate(index.row(), model->parent(index));
		});
		mu.addAction(e);
	}
	mu.addSeparator();
	mu.exec(QCursor::pos());
}

void ParserEditor::onIndexEntered(const QModelIndex &index)
{
	if (_lastEnteredIndex.isValid())
	{
		removeButtons();
	}
	if (index.column()==1)
	{
		_lastEnteredIndex = QModelIndex();
		return;
	}
	_lastEnteredIndex = index;
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	addButtons(index);
}

void ParserEditor::onViewportEntered()
{
	if (_lastEnteredIndex.isValid())
	{
		removeButtons();
		_lastEnteredIndex = QModelIndex();
	}
}

void ParserEditor::onXClicked()
{
	model->removeRow(_lastEnteredIndex.row(), model->parent(_lastEnteredIndex));
	removeButtons();
}

void ParserEditor::onAClicked()
{
	model->insertRow(-1, _lastEnteredIndex);
	ui->treeView->expand(_lastEnteredIndex);
}

void ParserEditor::onEClicked()
{
	ui->treeView->edit(_lastEnteredIndex);
}

ParserEditor::ParserEditor(const QString &filepath, QWidget *parent) :
    QMainWindow(parent),
	filepath(filepath), 
    ui(new Ui::ParserEditor)
{
    ui->setupUi(this);
	QTreeView *treeView = ui->treeView;
	model = new ParserEditorTreeModel(treeView);
	treeView->setModel(model);
	//treeView->setStyleSheet("QTreeView::branch {image:none;}");
	/*treeView->setAlternatingRowColors(true);
	treeView->setStyleSheet("QTreeView{background-color: rgb(255,255,255);"
		"alternate-background-color: rgb(200,200,200);}");*/
	treeView->setItemDelegateForColumn(1, new TypeDelegate(treeView));
	auto d = new StringDelegate(treeView);
	treeView->setItemDelegateForColumn(0, d);
	treeView->setItemDelegateForColumn(2, d);
	treeView->setMouseTracking(true);
	connect(treeView, &QTreeView::customContextMenuRequested, this, &ParserEditor::onTreeViewRClick);
	connect(treeView, &QTreeView::entered, this, &ParserEditor::onIndexEntered);
	connect(treeView, &QTreeView::viewportEntered, this, &ParserEditor::onViewportEntered);
	buildToolButtons();
}

void ParserEditor::load()
{
	QString text;
	if (!LOLI::AutoRead(text, filepath))
	{
		_error = "文件" + filepath + "打开失败";
		return;
	}
	
	QBkeVariable var = QBkeVariable::fromString(text);
	if (!var.isArray() && !var.isDic())
	{
		_error = "根节点应该是Dictionary或者Array。";
		return;
	}
	model->clear();
	model->buildRoot(var);
	ui->treeView->expandAll();
}

