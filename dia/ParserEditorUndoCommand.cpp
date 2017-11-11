#include "ParserEditorUndoCommand.h"
#include "ParserEditorTreeModel.h"
#include "ParserEditorTreeItem.h"

InsertRowsCommand::InsertRowsCommand(ParserEditorTreeModel *model, int row, int count, const QModelIndex &parent)
	: model(model)
	, row(row)
	, count(count)
	, parent(parent)
{
}

void InsertRowsCommand::undo()
{
	model->removeRowsInternal(row, count, parent);
}

void InsertRowsCommand::redo()
{
	model->insertRowsInternal(row, count, parent);
}

RemoveRowsCommand::RemoveRowsCommand(ParserEditorTreeModel *model, int row, int count, const QModelIndex &parent)
	: model(model)
	, row(row)
	, count(count)
	, parent(parent)
{
	items = model->itemsForRows(row, count, parent);
	for (auto &&item : items)
	{
		item = item->duplicate();
	}
}

RemoveRowsCommand::~RemoveRowsCommand()
{
	qDeleteAll(items);
}

void RemoveRowsCommand::undo()
{
	QList<ParserEditorTreeItem *> items;
	for (auto item : this->items)
	{
		items << item->duplicate();
	}
	model->insertDataInternal(row, items, parent);
}

void RemoveRowsCommand::redo()
{
	model->removeRowsInternal(row, count, parent);
}

InsertDataCommand::InsertDataCommand(ParserEditorTreeModel * model, int row, const QList<ParserEditorTreeItem*>& items, const QModelIndex & parent)
	: model(model)
	, row(row)
	, items(items)
	, parent(parent)
{
}

InsertDataCommand::~InsertDataCommand()
{
	qDeleteAll(items);
}

void InsertDataCommand::undo()
{
	model->removeRowsInternal(row, items.count(), parent);
}

void InsertDataCommand::redo()
{
	QList<ParserEditorTreeItem *> items;
	for (auto item : this->items)
	{
		items << item->duplicate();
	}
	model->insertDataInternal(row, items, parent);
}

ModifyDataCommand::ModifyDataCommand(ParserEditorTreeModel *model, const QModelIndex &index, const QVariant &data)
	: model(model)
	, index(index)
	, data(data)
{
	oldData = model->data(index, Qt::DisplayRole);
}

void ModifyDataCommand::undo()
{
	model->setDataInternal(index, oldData);
}

void ModifyDataCommand::redo()
{
	model->setDataInternal(index, data);
}