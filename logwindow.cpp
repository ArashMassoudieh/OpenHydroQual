#include "logwindow.h"
#include "ui_logwindow.h"

logwindow::logwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logwindow)
{
    ui->setupUi(this);
}

logwindow::~logwindow()
{
    delete ui;
}

void logwindow::AppendText(const QString &s)
{
    ui->textBrowser->append(s);

}

void logwindow::AppendError(const QString &s)
{
    ui->textBrowser->setFontWeight( QFont::DemiBold );
    ui->textBrowser->setTextColor( QColor( "red" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );

}

void logwindow::AppendBlue(const QString &s)
{
    ui->textBrowser->setTextColor( QColor( "blue" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );

}
