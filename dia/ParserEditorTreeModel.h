/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TREEMODEL_H
#define TREEMODEL_H

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

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool insertRows(int row, const QList<ParserEditorTreeItem *> &items, const QModelIndex &parent = QModelIndex());

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

#endif // TREEMODEL_H
