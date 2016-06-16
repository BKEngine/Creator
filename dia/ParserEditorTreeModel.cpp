#include "ParserEditorTreeItem.h"
#include "ParserEditorTreeModel.h"
#include <QStringList>
#include "ParserHelper/ParserHelper.h"

//! [0]
ParserEditorTreeModel::ParserEditorTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	root = NULL;
	header << "��" << "����" << "ֵ";
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
	beginRemoveRows(parent, row, row + count - 1);
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	item->removeChildrenAt(row, count);
	endRemoveRows();
	return true;
}

bool ParserEditorTreeModel::insertRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	ParserEditorTreeItem *item = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	if (row < 0)
		row += item->childCount();
	beginInsertRows(parent, row + 1, row + count);
	while (count--)
	{
		item->insertChildBefore(row, new ParserEditorTreeItem(QString(), ParserEditorTreeItem::VOID, QString()));
		row++;
	}
	endInsertRows();
	return true;
}

bool ParserEditorTreeModel::duplicate(int row, const QModelIndex &parent /*= QModelIndex()*/)
{
	ParserEditorTreeItem *parentItem = static_cast<ParserEditorTreeItem*>(parent.internalPointer());
	ParserEditorTreeItem *item = parentItem->child(row);
	ParserEditorTreeItem *duplicated = item->duplicate();
	beginInsertRows(parent, row + 1, row + 1);
	parentItem->insertChildBefore(row + 1, duplicated);
	endInsertRows();
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
	if (parentItem == 0) //��ʾ����root ���ɲ���
		return false;
	return true; //ֻҪ�Ǹ���Ŀ�����ֵ���������� ���Զ����Բ���
}

bool ParserEditorTreeModel::removeable(const QModelIndex &index) const
{
	//�ɲ���Ϳ��Ա�ɾ��
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
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::VOID, QString());
	}
	else if (var.isNum())
	{
		parent = new ParserEditorTreeItem(QString(), ParserEditorTreeItem::NUMBER, QString::number(var.toReal()));
	}
	return parent;
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

