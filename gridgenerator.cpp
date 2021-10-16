#include "gridgenerator.h"
#include "ui_gridgenerator.h"
#include "mainwindow.h"

GridGenerator::GridGenerator(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::GridGenerator)
{
    ui->setupUi(this);
    mainwindow = parent;
    PopulateBlocks();
    PopulateLinks(); 
}

GridGenerator::~GridGenerator()
{
    delete ui;
}

void GridGenerator::PopulateBlocks()
{

    ui->listWidgetBlocks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllBlockTypes().size(); i++)
    {

        //qDebug() << QString::fromStdString(system.GetAllBlockTypes()[i]);

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem *list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        ui->listWidgetBlocks->addItem(list_item);
    }

}
void GridGenerator::PopulateLinks()
{
    ui->listWidgetBlocks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllLinkTypes().size(); i++)
    {

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem* list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        ui->listWidgetLinks->addItem(list_item);
    }
}

System * GridGenerator::system()
{
    if (mainwindow!=nullptr)
        return mainwindow->GetSystem();
    else
        return nullptr;
}
