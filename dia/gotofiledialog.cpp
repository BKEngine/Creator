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
	connect(ui->cancelButton, &QPushButton::clicked, this, &GotoFileDialog::reject);
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
	QPinyin::ExtractPinyinToMap(files, map);
	QStringList keys = map.keys();
	for (auto &s : keys)
	{
		if (s.endsWith(".bkscr"))
		{
			auto newkey = s.right(s.size() - 6);
			map.insert(newkey, map[s]);
		}
	}
	matcher.reset(new QFuzzyMatcher(map.keys()));
	LineEditTextChanged(QString());
}

void GotoFileDialog::LineEditTextChanged(QString s)
{
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
		QString path = qs[i];
		QString filename = QFileInfo(path).fileName();
		if (i < rows)
		{
			ui->tableWidget->item(i, 0)->setText(filename);
			ui->tableWidget->item(i, 1)->setText(path);
		}
		else
		{
			ui->tableWidget->setItem(i, 0, new QTableWidgetItem(filename));
			ui->tableWidget->setItem(i, 1, new QTableWidgetItem(path));
		}
	}
	ui->tableWidget->setCurrentCell(0, 0);
	tableContents = qs;
}

void GotoFileDialog::OnOK()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0)
	{
		emit GotoFile(tableContents[row]);
		this->accept();
		return;
	}
	this->accept();
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
				reject();
				return true;
			}
		}
	}
	return false;
}
