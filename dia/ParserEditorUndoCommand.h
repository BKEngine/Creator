#pragma once

#include <QModelIndex>
#include <QUndoCommand>
#include <QVariant>
#include <QPersistentModelIndex>

/*
��ɾɾ�����У�QModelIndex����֤��Ч��������undo�Ĺ�����ɾ��ĳ���У�
��redo��ʱ������е�ʱ��itemָ���Ѿ��仯�˵���QModelIndexʧЧ����
����QPersistentModelIndex��֤��Ч��
QPersistentModelIndex����Ч����Modelά����
*/

class ParserEditorTreeModel;
class ParserEditorTreeItem;
class InsertRowsCommand : public QUndoCommand
{
	Q_DISABLE_COPY(InsertRowsCommand)
	ParserEditorTreeModel *model;
	int row;
	int count;
	QPersistentModelIndex parent;
public:
	InsertRowsCommand(ParserEditorTreeModel *model, int row, int count, const QModelIndex &parent);
	virtual void undo() override;
	virtual void redo() override;
};

class RemoveRowsCommand : public QUndoCommand
{
	Q_DISABLE_COPY(RemoveRowsCommand)
	ParserEditorTreeModel *model;
	int row;
	int count;
	QPersistentModelIndex parent;
	QList<ParserEditorTreeItem *> items;
public:
	RemoveRowsCommand(ParserEditorTreeModel *model, int row, int count, const QModelIndex &parent);
	~RemoveRowsCommand();
	virtual void undo() override;
	virtual void redo() override;
};

class InsertDataCommand : public QUndoCommand
{
	Q_DISABLE_COPY(InsertDataCommand)
	ParserEditorTreeModel *model;
	int row;
	QList<ParserEditorTreeItem *> items;
	QPersistentModelIndex parent;
public:
	InsertDataCommand(ParserEditorTreeModel *model, int row, const QList<ParserEditorTreeItem *> &items, const QModelIndex &parent);
	~InsertDataCommand();
	virtual void undo() override;
	virtual void redo() override;
};

class ModifyDataCommand : public QUndoCommand
{
	Q_DISABLE_COPY(ModifyDataCommand)
	ParserEditorTreeModel *model;
	QPersistentModelIndex index;
	QVariant data;
	QVariant oldData;
public:
	ModifyDataCommand(ParserEditorTreeModel *model, const QModelIndex &index, const QVariant &data);
	virtual void undo() override;
	virtual void redo() override;
};
