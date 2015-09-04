#include "cskinoption.h"
#include "ui_cskinoption.h"

CSkinOption::CSkinOption(QWidget *parent) :
QWidget(parent),
ui(new Ui::CSkinOption)
{
	ui->setupUi(this);
	setLayout(ui->gridLayout);
	isChange = false;

	ui->stylenamebox->addItems(StyleList());
	ui->textEdit->setFontPointSize(10);

	connect(ui->stylenamebox, SIGNAL(currentIndexChanged(QString)), this, SLOT(StyleChange(QString)));
	connect(ui->unitbox, SIGNAL(currentTextChanged(QString)), this, SLOT(UnitChange(QString)));
	connect(ui->textEdit, SIGNAL(undoAvailable(bool)), ui->btnno, SLOT(setEnabled(bool)));
	connect(ui->textEdit, SIGNAL(undoAvailable(bool)), ui->btnyes, SLOT(setEnabled(bool)));
	connect(ui->btnno, SIGNAL(clicked()), ui->textEdit, SLOT(undo()));
	connect(ui->btnyes, SIGNAL(clicked()), this, SLOT(SaveStyle()));
	connect(ui->btnnew, SIGNAL(clicked()), this, SLOT(NewStyle()));
	connect(ui->btndel, SIGNAL(clicked()), this, SLOT(DelStyle()));

	ui->stylenamebox->setCurrentText(BKE_SKIN_CURRENT);
	UnitChange(ui->unitbox->currentText());
}

CSkinOption::~CSkinOption()
{
	delete ui;
}

QStringList CSkinOption::StyleList()
{
	QStringList temp = BKE_SKIN_SETTING->childGroups();
	QStringList res;
	for (int i = 0; i < temp.size(); i++){
		if (BKE_SKIN_SETTING->value(temp.at(i) + "/type", false).toBool()) res.append(temp.at(i));
	}
	return res;
}

QStringList CSkinOption::StyleUnit(QString stylename)
{
	BKE_SKIN_SETTING->beginGroup(stylename);
	QStringList temp = BKE_SKIN_SETTING->childKeys();
	temp.removeOne("type");
	BKE_SKIN_SETTING->endGroup();
	return temp;
}

void CSkinOption::SaveStyle()
{
	save();
}

void CSkinOption::StyleChange(const QString &stylename)
{
	if (stylename.isEmpty()) return;
	UnitChange(ui->unitbox->currentText());
	BKE_SKIN_CURRENT = stylename;
}

void CSkinOption::UnitChange(const QString &part)
{
	ui->textEdit->setText(TypeText(BKE_SKIN_SETTING->value(ui->stylenamebox->currentText() + "/" + part).toString()));
	ui->btnno->setEnabled(false);
	ui->btnyes->setEnabled(false);
}

QString CSkinOption::TypeText(QString text)
{
	for (int i = 0; i < text.length() - 2; i++){
		if (text.at(i) == QChar('{') || text.at(i) == QChar('}') || text.at(i) == QChar(';')){
			text.insert(i + 1, "\r\n");
		}
	}
	return text;

}

void CSkinOption::NewStyle()
{
	QString NewStyleName;
	bool gww;
	do{
		NewStyleName = QInputDialog::getText(this, "", "请输入新外观样式的名字", QLineEdit::Normal, "", &gww);
		if (!gww) return;
		if (ui->stylenamebox->findText(NewStyleName) >= 0){
			QMessageBox::information(this, "", "重复的类型名字");
			NewStyleName.clear();
		}
	} while (NewStyleName.isEmpty());

	BKE_SKIN_SETTING->setValue(NewStyleName + "/type", true);
	ui->stylenamebox->addItem(NewStyleName);
	ui->stylenamebox->setCurrentText(NewStyleName);
}

void CSkinOption::DelStyle()
{
	if (ui->stylenamebox->currentText() == "默认"){
		QMessageBox::information(this, "外观", "默认的外观不可删除！");
		return;
	}

	BKE_SKIN_SETTING->beginGroup(ui->stylenamebox->currentText());
	BKE_SKIN_SETTING->remove("");
	BKE_SKIN_SETTING->endGroup();
	ui->stylenamebox->removeItem(ui->stylenamebox->currentIndex());
}

void CSkinOption::save()
{
	QString temp = ui->textEdit->toPlainText();
	temp = temp.replace(QRegExp("\r"), "");
	temp = temp.replace(QRegExp("\n"), "");
	ui->btnyes->setEnabled(false);
	BKE_SKIN_SETTING->setValue(ui->stylenamebox->currentText() + "/" + ui->unitbox->currentText(), temp);
}

void CSkinOption::load()
{
	ui->stylenamebox->setCurrentText(BKE_SKIN_CURRENT);
	UnitChange(ui->unitbox->currentText());
}

void CSkinOption::reset()
{
	BKE_SKIN_CURRENT = "默认";
	ui->stylenamebox->setCurrentText(BKE_SKIN_CURRENT);
	UnitChange(ui->unitbox->currentText());
}
