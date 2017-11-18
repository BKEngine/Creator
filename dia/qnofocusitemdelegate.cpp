#include "qnofocusitemdelegate.h"

QNoFocusItemDelegate::QNoFocusItemDelegate(QObject *parent)
	:QStyledItemDelegate(parent)
{

}

void QNoFocusItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (option.state & QStyle::State_HasFocus)
	{
		QStyleOptionViewItem itemOption(option);
		itemOption.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(painter, itemOption, index);
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}    
}

