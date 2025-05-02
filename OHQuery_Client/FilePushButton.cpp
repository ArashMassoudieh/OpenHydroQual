#include "FilePushButton.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

FilePushButton::FilePushButton(QWidget *parent)
	: QPushButton(parent)
{
    setText("Browse...");
    setMinimumWidth(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setVisible(true);
    QObject::connect(this, SIGNAL(clicked()), this, SLOT(browserClicked()));
}

FilePushButton::~FilePushButton()
{
}

void FilePushButton::browserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open"), "",
        tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName != "")
    {
        if (QFile(fileName).exists())
            setText(fileName);
        else {
            QMessageBox::information(nullptr, "File not found!", "File " + fileName + " was not found!", QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        }
    }
    else
        setText("");

}
