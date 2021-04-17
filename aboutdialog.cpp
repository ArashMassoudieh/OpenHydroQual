#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::AppendText(const QString &text)
{
    ui->textBrowser->append(text);
}
void AboutDialog::ClearText()
{
    ui->textBrowser->clear();
}
