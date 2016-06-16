#include <QStringList>

#include "ParserEditorTreeItem.h"

//! [0]
ParserEditorTreeItem::ParserEditorTreeItem(const QString &key, Type type, const QString &value)
	: _key(key)
	, _type(type)
	, _value(value)
	, m_parentItem(nullptr)
{
}
//! [0]

//! [1]
ParserEditorTreeItem::~ParserEditorTreeItem()
{
	clear();
}
//! [1]

//! [2]
void ParserEditorTreeItem::appendChild(ParserEditorTreeItem *item)
{
    m_childItems.append(item);
	item->m_parentItem = this;
	if (this->type() == ParserEditorTreeItem::ARRAY)
	{
		item->setKey(QString::number(m_childItems.count() - 1));
	}
}

void ParserEditorTreeItem::removeChildAt(int row)
{
	delete m_childItems[row];
	m_childItems.removeAt(row);
	if (this->type() == ParserEditorTreeItem::ARRAY)
	{
		this->rebuildArrayName();
	}
}

void ParserEditorTreeItem::removeChildrenAt(int row, int count)
{
	while (count--)
	{
		delete m_childItems[row];
		m_childItems.removeAt(row);
	}
	if (this->type() == ParserEditorTreeItem::ARRAY)
	{
		this->rebuildArrayName();
	}
}

void ParserEditorTreeItem::insertChildBefore(int pos, ParserEditorTreeItem *child)
{
	m_childItems.insert(m_childItems.begin() + pos, child);
	child->m_parentItem = this;
	if (this->type() == ParserEditorTreeItem::ARRAY)
	{
		this->rebuildArrayName();
	}
}

//! [2]

//! [3]
ParserEditorTreeItem *ParserEditorTreeItem::child(int row)
{
    return m_childItems.value(row);
}
//! [3]

//! [4]
int ParserEditorTreeItem::childCount() const
{
    return m_childItems.count();
}
//! [4]

//! [5]
int ParserEditorTreeItem::columnCount() const
{
    return 3;
}
//! [5]

//! [6]
QVariant ParserEditorTreeItem::data(int column) const
{
	switch (column)
	{
	case 0:
		return _key;
	case 1:
		return typeString();
	case 2:
		return _value;
	default:
		break;
	}
	return QVariant();
}

void ParserEditorTreeItem::setData(int column, const QVariant &data)
{
	switch (column)
	{
	case 0:
		_key = data.toString();
		break;
	case 1:
		setTypeString(data.toString());
		break;
	case 2:
		_value = data.toString();
		break;
	default:
		break;
	}
}

//! [6]

//! [7]
ParserEditorTreeItem *ParserEditorTreeItem::parentItem()
{
    return m_parentItem;
}

int ParserEditorTreeItem::level()
{
	int level = 0;
	auto parent = m_parentItem;
	while (parent)
	{
		level++;
		parent = parent->m_parentItem;
	}
	return level;
}

void ParserEditorTreeItem::clear()
{
	qDeleteAll(m_childItems);
	m_childItems.clear();
}

ParserEditorTreeItem * ParserEditorTreeItem::duplicate() const
{
	ParserEditorTreeItem *item = new ParserEditorTreeItem(_key, _type, _value);
	for (auto it : m_childItems)
	{
		item->appendChild(it->duplicate());
	}
	return item;
}

static QString _typeStrings[] = {
	"Void",
	"Number",
	"String",
	"Dictionary",
	"Array",
	"Expression"
};

QString ParserEditorTreeItem::typeString() const
{
	return _typeStrings[_type];
}

void ParserEditorTreeItem::setTypeString(const QString &str)
{
	int i = 0;
	for (auto && it : _typeStrings)
	{
		if (it == str)
		{
			_type = (Type)i;
			return;
		}
		i++;
	}
}

void ParserEditorTreeItem::rebuildArrayName()
{
	for (int i = 0; i < m_childItems.count(); i++)
	{
		m_childItems[i]->setKey(QString::number(i));
	}
}

QStringList ParserEditorTreeItem::allTypeStrings()
{
	QStringList a;
	for (auto &&s : _typeStrings)
	{
		a << s;
	}
	return a;
}

//! [7]

//! [8]
int ParserEditorTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ParserEditorTreeItem*>(this));

    return 0;
}
//! [8]
