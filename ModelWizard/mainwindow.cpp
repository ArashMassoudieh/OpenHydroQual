#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Wizard_Script.h"
#include "QDir"
#include "wizarddialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget->setFlow(QListView::LeftToRight);

    //Dynamically adjust contents
    ui->listWidget->setResizeMode(QListView::Adjust);

    //This is an arbitrary value, but it forces the layout into a grid
    ui->listWidget->setGridSize(QSize(150, 150));
    ui->listWidget->setIconSize(QSize(128, 128));
    //As an alternative to using setGridSize(), set a fixed spacing in the layout:
    ui->listWidget->setSpacing(5);

    //And the most important part:
    ui->listWidget->setViewMode(QListView::IconMode);
    QObject::connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
    PopulateListOfWizards();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::PopulateListOfWizards()
{

    QDir directory(QString::fromStdString(wizardsfolder));
    qDebug() << directory.absolutePath(); 
    qDebug() << directory.exists();
    QStringList wizardfiles = directory.entryList(QStringList() << "*.json" << "*.JSON",QDir::Files);

    foreach(QString filename, wizardfiles) {
        WizardScript wiz(QString::fromStdString(wizardsfolder)+filename);

        QListWidgetItem *item = new QListWidgetItem(wiz.Icon(),wiz.Name());
        item->setData(1000,QString::fromStdString(wizardsfolder)+filename);
        item->setToolTip(wiz.Description());

        ui->listWidget->addItem(item);
    }
}

void MainWindow::itemDoubleClicked(QListWidgetItem* wizitem)
{
    WizardDialog *wizDialog = new WizardDialog(this);
    WizardScript wiz(wizitem->data(1000).toString());
    wizDialog->CreateItems(&wiz);
    wizDialog->show();
    wizDialog->resizeEvent();
}

