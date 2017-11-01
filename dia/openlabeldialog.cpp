#include "weh.h"
#include "openlabeldialog.h"
#include "ui_openlabeldialog.h"
#include "QPinyin/QPinyin.h"
#include "qnofocusitemdelegate.h"
#include <QKeyEvent>
#include <QHeaderView>

OpenLabelDialog::OpenLabelDialog(const QSortedSet<QString> &labels, QWidget *parent /*= 0*/) :
	QDialog(parent, Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint),
	ui(new Ui::OpenLabelDialog)
{
	ui->setupUi(this); 
	ui->tableWidget->setColumnCount(1); //设置列数
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "标签");
	ui->tableWidget->setVerticalHeaderLabels(QStringList());
	ui->tableWidget->horizontalHeader()->setDefaultSectionSize(150);
	ui->tableWidget->horizontalHeader()->setSectionsClickable(false);
	ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tableWidget->setItemDelegate(new QNoFocusItemDelegate());
	connect(ui->okButton, &QPushButton::clicked, this, &OpenLabelDialog::OnOK);
	connect(ui->cancelButton, &QPushButton::clicked, this, &OpenLabelDialog::reject);
	connect(ui->lineEdit, &QLineEdit::textChanged, this, &OpenLabelDialog::LineEditTextChanged);
	connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &OpenLabelDialog::CellDoubleClicked);
	ui->lineEdit->installEventFilter(this);
	setLabels(labels);
}

OpenLabelDialog::~OpenLabelDialog()
{
    delete ui;
}

void OpenLabelDialog::setLabels(const QSortedSet<QString> &labels)
{
	this->labels = labels;
	QPinyin::ExtractPinyinToMap(labels, map);
	matcher.reset(new QFuzzyMatcher(map.keys()));
	LineEditTextChanged(QString());
}

void OpenLabelDialog::LineEditTextChanged(QString s)
{
	if (s.startsWith("*"))
	{
		s = s.right(s.length() - 1);
	}
	QStringList qs;
	if (s.isEmpty())
	{
		for (auto &i : labels)
			qs.append(i);
	}
	else
	{
		QVector<MatchResult> results = matcher->findMatches(s, options);
		for (auto && result : results)
		{
			auto &&ss = map[*result.value];
			if (!qs.contains(ss))
			{
				qs.append(ss);
			}
		}
	}
	int rows = ui->tableWidget->rowCount();
	ui->tableWidget->setRowCount(qs.length());
	for (int i = 0; i < qs.length(); i++)
	{
		QString label = "*" + qs[i];
		if (i < rows)
		{
			ui->tableWidget->item(i, 0)->setText(label);
		}
		else
		{
			ui->tableWidget->setItem(i, 0, new QTableWidgetItem(label));
		}
	}
	ui->tableWidget->setCurrentCell(0, 0);
	tableContents = qs;
}

void OpenLabelDialog::OnOK()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0)
	{
		emit GotoLabel(tableContents[row]);
		this->accept();
		return;
	}
	this->reject();
}

void OpenLabelDialog::CellDoubleClicked(int row, int)
{
	emit GotoLabel(tableContents[row]);
	this->accept();
	return;
}

bool OpenLabelDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui->lineEdit)
	{
		if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
			{
				return qApp->sendEvent(ui->tableWidget, keyEvent);
			}
			else if (keyEvent->key() == Qt::Key_Enter && event->type() == QEvent::KeyRelease)
			{
				OnOK();
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Escape && event->type() == QEvent::KeyRelease)
			{
				reject();
				return true;
			}
		}
	}
	return false;
}
