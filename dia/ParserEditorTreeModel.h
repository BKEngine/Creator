#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ParserEditorTreeItem;
class QBkeVariable;
class QUndoStack;
//! [0]
class ParserEditorTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ParserEditorTreeModel(QObject *parent = 0);
    ~ParserEditorTreeModel();

    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	virtual QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;

	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	//virtual bool insertRows(int row, const QList<ParserEditorTreeItem *> &items, const QModelIndex &parent = QModelIndex());

	bool duplicate(int row, const QModelIndex &parent = QModelIndex());

	/**
	 *	该条目是否可以被编辑
	 */
	bool editable(const QModelIndex &index) const;
	/**
	 *	该条目是否可以添加子条目
	 */
	bool addable(const QModelIndex &index) const;
	/**
	 *	该条目底下是否可以插入新条目
	 */
	bool insertable(const QModelIndex &index) const;
	/**
	 *	该条目是否可以被移除
	 */
	bool removeable(const QModelIndex &index) const;

	ParserEditorTreeItem *item(const QModelIndex &index) const;

	void clear();

	void buildRoot(const QBkeVariable &var);
	QUndoStack *undoStack() const { return _undoStack; }

private:
	ParserEditorTreeItem *add(const QBkeVariable &var, ParserEditorTreeItem *parent);
	ParserEditorTreeItem *itemFromVariable(const QBkeVariable &var);

	//UndoCommand实现，以下接口只有UndoCommand才能调用，真正的修改数据
private:
	void insertRowsInternal(int row, int count, const QModelIndex &parent);
	void insertDataInternal(int row, const QList<ParserEditorTreeItem *> &items, const QModelIndex &parent);
	void removeRowsInternal(int row, int count, const QModelIndex &parent);
	void setDataInternal(const QModelIndex &index, const QVariant &data);
	void replaceItemInternal(const QModelIndex &index, ParserEditorTreeItem *item);
	QList<ParserEditorTreeItem *> itemsForRows(int row, int count, const QModelIndex &parent) const;
	friend class InsertRowsCommand;
	friend class RemoveRowsCommand;
	friend class InsertDataCommand;
	friend class ModifyDataCommand;
	friend class ChangeTypeCommand;

private:
    ParserEditorTreeItem *root;
	QStringList header;
	QUndoStack *_undoStack;
};
//! [0]
