#include "ParserEditorTreeItem.h"
#include "ParserEditorTreeModel.h"
#include <QStringList>
#include <QUndoStack>
#include "ParserHelper/ParserHelper.h"
#include "ParserEditorUndoCommand.h"

//! [0]
ParserEditorTreeModel::ParserEditorTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	root = NULL;
	header << "键" << "类型" << "值";
	_undoStack = new QUndoStack();
}
//! [0]

//! [1]
ParserEditorTreeModel::~ParserEditorTreeModel()
{
    delete root;
}
//! [1]

//! [2]
int ParserEditorTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ParserEditorTreeItem*>(parent.internalPointer())->columnCount();
    else
        return header.count();
}

bool ParserEditorTreeModel::removeRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	_undoStack->push(new RemoveRowsCommand(this, row, count, parent));
	return true;
}

bool ParserEditorTreeModel::insertRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	if (row < 0)
		row += item->childCount() + 1;
	_undoStack->push(new InsertRowsCommand(this, row, count, parent));
	return true;
}

static QPair<QString, QString> splitNumber(const QString &str)
{
	for (int i = str.size() - 1; i >= 0; i--)
	{
		if (str[i] < '0' || str[i] > '9')
		{
			if (i == str.size() - 1)
			{
				return {str, QString()};
			}
			else
			{
				return { str.left(i + 1), str.right(str.size() - 1 - i) };
			}
		}
	}
	return{ QString(), str };
}

bool ParserEditorTreeModel::duplicate(int row, const QModelIndex &parent /*= QModelIndex()*/)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	ParserEditorTreeItem *duplicated = item->child(row)->duplicate();
	QPair<QString, QString> nameSplited = splitNumber(duplicated->key());
	if (nameSplited.second.isEmpty())
	{
		duplicated->setKey(duplicated->key() + "2");
	}
	else
	{
		duplicated->setKey(nameSplited.first + QString::number(nameSplited.second.toInt() + 1));
	}
	_undoStack->push(new InsertDataCommand(this, row + 1, { duplicated }, parent));
	return true;
}

bool ParserEditorTreeModel::editable(const QModelIndex &index) const
{
	ParserEditorTreeItem *childItem = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	ParserEditorTreeItem *parentItem = childItem->parentItem();
	if (parentItem == 0)
		return false;
	if (parentItem->type() == ParserEditorTreeItem::ARRAY && index.column() == 0)
		return false;
	if ((childItem->type() == ParserEditorTreeItem::VOID || childItem->type() == ParserEditorTreeItem::ARRAY || childItem->type() == ParserEditorTreeItem::DICTIONARY) && index.column() == 2)
		return false;
	return true;
}

bool ParserEditorTreeModel::addable(const QModelIndex &index) const
{
	ParserEditorTreeItem *childItem = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	return childItem->type() == ParserEditorTreeItem::DICTIONARY || childItem->type() == ParserEditorTreeItem::ARRAY;
}

bool ParserEditorTreeModel::insertable(const QModelIndex &index) const
{
	ParserEditorTreeItem *childItem = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	ParserEditorTreeItem *parentItem = childItem->parentItem();
	if (parentItem == 0) //表示这是root 不可插入
		return false;
	return true; //只要是个条目都在字典或者数组下 所以都可以插入
}

bool ParserEditorTreeModel::removeable(const QModelIndex &index) const
{
	//可插入就可以被删除
	return insertable(index);
}

void ParserEditorTreeModel::clear()
{
	beginResetModel();
	delete root;
	root = NULL;
	endResetModel();
}

ParserEditorTreeItem *ParserEditorTreeModel::item(const QModelIndex &index) const
{
	ParserEditorTreeItem *childItem = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	if (!childItem)
	{
		return root;
	}
	return childItem;
}

void ParserEditorTreeModel::buildRoot(const QBkeVariable &var)
{
	beginResetModel();
	root = add(var, NULL);
	root->setKey("root");
	endResetModel();
}

ParserEditorTreeItem *ParserEditorTreeModel::add(const QBkeVariable &var, ParserEditorTreeItem *parent)
{
	beginResetModel();
	auto item = itemFromVariable(var);
	if (parent != NULL)
	{
		parent->appendChild(item);
	}
	endResetModel();
	return item;
}

ParserEditorTreeItem * ParserEditorTreeModel::itemFromVariable(const QBkeVariable &var)
{
	ParserEditorTreeItem *parent = NULL;
	if (var.isArray())
	{
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::ARRAY, QString());
		for (int i = 0; i < var.getCount(); i++)
		{
			ParserEditorTreeItem *item = itemFromVariable(var[i]);
			item->setKey(QString::number(i));
			parent->appendChild(item);
		}
	}
	else if (var.isDic())
	{
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::DICTIONARY, QString());
		QBkeDictionary dic = var.toBkeDic();
		for (auto it = dic.begin(); it != dic.end(); it++)
		{
			ParserEditorTreeItem *item = itemFromVariable(it.value());
			item->setKey(it.key());
			parent->appendChild(item);
		}
	}
	else if (var.isString())
	{
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::STRING, var.toString());
	}
	else if (var.isVoid())
	{
		parent = new ParserEditorTreeItem();
	}
	else if (var.isNum())
	{
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::NUMBER, QString::number(var.toReal()));
	}
	return parent;
}

void ParserEditorTreeModel::insertRowsInternal(int row, int count, const QModelIndex & parent)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	beginInsertRows(parent, row, row + count - 1);
	for (int i = 0; i < count; i++)
	{
		item->insertChildBefore(row + i, new ParserEditorTreeItem());
	}
	endInsertRows();
}

void ParserEditorTreeModel::insertDataInternal(int row, const QList<ParserEditorTreeItem*>& items, const QModelIndex & parent)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	int count = items.count();
	beginInsertRows(parent, row, row + count - 1);
	for (int i = 0; i < count; i++)
	{
		item->insertChildBefore(row + i, items[i]);
	}
	endInsertRows();
}

void ParserEditorTreeModel::removeRowsInternal(int row, int count, const QModelIndex & parent)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	beginRemoveRows(parent, row, row + count - 1);
	item->removeChildrenAt(row, count);
	endRemoveRows();
}

void ParserEditorTreeModel::setDataInternal(const QModelIndex & index, const QVariant & data)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	//�������ͱ任
	if (index.column() == 1)
	{
		auto type = item->type();
		if (item->setTypeString(data.toString())) 
		{
			if (type == ParserEditorTreeItem::DICTIONARY || type == ParserEditorTreeItem::ARRAY)
			{
				if (item->childCount())
				{
					beginRemoveRows(index, 0, item->childCount() - 1);
					item->clear();
					endRemoveRows();
				}
			}
			else if (item->type() == ParserEditorTreeItem::DICTIONARY || item->type() == ParserEditorTreeItem::ARRAY)
			{
				item->setValue(QString());
			}
			else if (item->type() == ParserEditorTreeItem::NUMBER)
			{
				item->setValue(QString::number(item->value().toDouble()));
			}
			else if(item->type() == ParserEditorTreeItem::VOID)
			{
				item->setValue(QString());
			}
			QModelIndex index1 = this->index(index.row(), 0, index.parent());
			QModelIndex index2 = this->index(index.row(), 2, index.parent());
			emit dataChanged(index1, index2);
		}
	}
	else
	{
		item->setData(index.column(), data);
		emit dataChanged(index, index);
	}
}

void ParserEditorTreeModel::replaceItemInternal(const QModelIndex & index, ParserEditorTreeItem *newitem)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
	ParserEditorTreeItem *parent = item->parentItem();
	parent->replaceChildAt(item->row(), newitem);
}

QList<ParserEditorTreeItem *> ParserEditorTreeModel::itemsForRows(int row, int count, const QModelIndex &parent) const
{
	QList<ParserEditorTreeItem*> items;
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	for (int i = row; i < row + count; i++)
	{
		items << item->child(i);
	}
	return items;
}

//! [2]

//! [3]
QVariant ParserEditorTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
	
    if (role != Qt::DisplayRole)
        return QVariant();

    ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(index.internalPointer());
    return item->data(index.column());
}

bool ParserEditorTreeModel::setData(const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */)
{
	if (role == Qt::EditRole)
	{
		if (index.column() == 1)
			_undoStack->push(new ChangeTypeCommand(this, index, value));
		else
			_undoStack->push(new ModifyDataCommand(this, index, value));
		return true;
	}
	return false;
}

//! [3]

//! [4]
Qt::ItemFlags ParserEditorTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	if (editable(index))
	{
		return flags | Qt::ItemIsEditable;
	}
	return flags;
}
//! [4]

//! [5]
QVariant ParserEditorTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header.value(section);

    return QVariant();
}
//! [5]

//! [6]
QModelIndex ParserEditorTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ParserEditorTreeItem *parentItem;

	if (!parent.isValid())
	{
		return createIndex(row, column, root);
	}
    else
        parentItem = static_cast<ParserEditorTreeItem*>(parent.internalPointer());

    ParserEditorTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//! [7]
QModelIndex ParserEditorTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ParserEditorTreeItem *childItem = static_cast<ParserEditorTreeItem*>(index.internalPointer());
    ParserEditorTreeItem *parentItem = childItem->parentItem();

    if (parentItem == NULL)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
//! [7]

//! [8]
int ParserEditorTreeModel::rowCount(const QModelIndex &parent) const
{
    ParserEditorTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

	if (!parent.isValid())
		if (!root)
			return 0;
		else
			return 1;
    else
        parentItem = static_cast<ParserEditorTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void ParserEditorTreeModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{

}
