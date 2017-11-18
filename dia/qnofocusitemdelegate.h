#ifndef QNOFOCUSITEMDELEGATE_H
#define QNOFOCUSITEMDELEGATE_H

#include <QStyledItemDelegate>

class QNoFocusItemDelegate : public QStyledItemDelegate
{
public:
	explicit QNoFocusItemDelegate(QObject *parent = 0);
    void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const override;
};

#endif // QNOFOCUSITEMDELEGATE_H
