#include "weh.h"
#include "gotofiledialog.h"
#include "ui_gotofiledialog.h"
#include "QPinyin/QPinyin.h"
#include "qnofocusitemdelegate.h"
#include <QKeyEvent>
#include <QHeaderView>

GotoFileDialog::GotoFileDialog(const QStringList &files, QWidget *parent /*= 0*/) :
	QDialog(parent, Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint),
	ui(new Ui::GotoFileDialog)
{
	ui->setupUi(this);
	ui->tableWidget->setColumnCount(2); //设置列数
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "文件" << "路径");
	ui->tableWidget->setVerticalHeaderLabels(QStringList());
	ui->tableWidget->horizontalHeader()->setDefaultSectionSize(150);
	ui->tableWidget->horizontalHeader()->setSectionsClickable(false);
	ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tableWidget->setItemDelegate(new QNoFocusItemDelegate());
	connect(ui->okButton, &QPushButton::clicked, this, &GotoFileDialog::OnOK);
	connect(ui->cancelButton, &QPushButton::clicked, this, &GotoFileDialog::close);
	connect(ui->lineEdit, &QLineEdit::textChanged, this, &GotoFileDialog::LineEditTextChanged);
	ui->lineEdit->installEventFilter(this);
	setFiles(files);
}

GotoFileDialog::~GotoFileDialog()
{
	delete ui;
}

void GotoFileDialog::setFiles(const QStringList &files)
{
	this->files = files;
	matches = QPinyin::ExtractPinyin(files, map);
	matcher.reset(new QFuzzyMatcher(matches));
	LineEditTextChanged(QString());
}

void GotoFileDialog::LineEditTextChanged(QString s)
{
	if (s.startsWith("*"))
	{
		s = s.right(s.length() - 1);
	}
	QStringList qs;
	if (s.isEmpty())
	{
		qs = files;
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
}

void GotoFileDialog::OnOK()
{
	auto item = ui->tableWidget->currentItem();
	if (item)
	{
		emit GotoFile(item->text());
	}
	this->close();
}

bool GotoFileDialog::eventFilter(QObject *obj, QEvent *event)
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
				close();
				return true;
			}
		}
	}
	return false;
}
