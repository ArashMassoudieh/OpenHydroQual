#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QDebug>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifndef Win_Version
    string modelfilename = qApp->applicationDirPath().toStdString() + "/../../resources/power_reservoirs_rules_source.json";
    string entitiesfilename = qApp->applicationDirPath().toStdString() + "/../../resources/entities.json";
#else
    string modelfilename = qApp->applicationDirPath().toStdString() + "/resources/power_reservoirs_rules_source.json";
    string entitiesfilename = qApp->applicationDirPath().toStdString() + "/resources/entities.json";
#endif // !Win_Version
    system.GetQuanTemplate(modelfilename);
    Populate_TreeWidget();
    //connect(ui->treeWidget, SIGNAL(closeEvent()),ui->actionObject_Browser, SLOT())

    connect(ui->actionObject_Browser,SIGNAL(triggered()),this,SLOT(on_check_object_browser()));
    connect(ui->dockWidget_3,SIGNAL(visibilityChanged(bool)),this,SLOT(on_object_browser_closed(bool)));

    BuildObjectsToolBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::Populate_TreeWidget()
{
    ui->treeWidget->clear();
    QStringList treeitems = system.QGetAllCategoryTypes();
    for (int i=0; i<treeitems.count(); i++)
    {
        QTreeWidgetItem *Item = new QTreeWidgetItem(ui->treeWidget);
        QTreeWidgetItem(ui->treeWidget);
        Item->setText(0,treeitems[i]);
        ui->treeWidget->addTopLevelItem(Item);
    }

}

void MainWindow::on_check_object_browser()
{
    //ui->actionObject_Browser->setChecked(!ui->actionObject_Browser->isChecked());
    if (ui->actionObject_Browser->isChecked())
        ui->dockWidget_3->show();
    else
        ui->dockWidget_3->hide();

}

void MainWindow::on_object_browser_closed(bool visible)
{
    ui->actionObject_Browser->setChecked(visible);
}

bool MainWindow::BuildObjectsToolBar()
{
    for (unsigned int i = 0; i < system.GetAllBlockTypes().size(); i++)
    {
        qDebug() << QString::fromStdString(system.GetAllBlockTypes()[i]);
        QAction* action = new QAction(this);
        action->setObjectName(QString::fromStdString(system.GetAllBlockTypes()[i]));
        QIcon icon;
        icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        action->setIcon(icon);
        ui->mainToolBar->addAction(action);
        action->setText(QString::fromStdString(system.GetAllBlockTypes()[i]));
        connect(action, SIGNAL(triggered()), this, SLOT(onaddblock()));
    }
    ui->mainToolBar->addSeparator();
    for (unsigned int i = 0; i < system.GetAllLinkTypes().size(); i++)
    {
        qDebug() << QString::fromStdString(system.GetAllLinkTypes()[i]);
        QAction* action = new QAction(this);
        action->setCheckable(true);
        action->setObjectName(QString::fromStdString(system.GetAllLinkTypes()[i]));
        QIcon icon;
        icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        action->setIcon(icon);
        ui->mainToolBar->addAction(action);
        action->setText(QString::fromStdString(system.GetAllLinkTypes()[i]));
        connect(action, SIGNAL(triggered()), this, SLOT(onaddlink()));
    }
    ui->mainToolBar->addSeparator();
    for (unsigned int j = 0; j < system.QGetAllCategoryTypes().size(); j++)
    {
        string typecategory = system.QGetAllCategoryTypes()[j].toStdString();
        if (typecategory!="Blocks" && typecategory !="Connectors")
            for (unsigned int i = 0; i < system.GetAllTypesOf(typecategory).size(); i++)
            {
                string type = system.GetAllTypesOf(typecategory)[i];
                QAction* action = new QAction(this);
                action->setCheckable(false);
                action->setObjectName(QString::fromStdString(type));
                QIcon icon;
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system.GetModel(type)->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
                action->setIcon(icon);
                ui->mainToolBar->addAction(action);
                action->setText(QString::fromStdString(type));
                connect(action, SIGNAL(triggered()), this, SLOT(onaddentity()));
            }
        ui->mainToolBar->addSeparator();
    }

    return true;
}


void MainWindow::onaddblock()
{
    QObject* obj = sender();
    Block block;
    block.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString());
    block.SetType(obj->objectName().toStdString());
    block.SetName(CreateNewName(obj->objectName().toStdString()));
    system.AddBlock(block);
    RefreshTreeView();
    //Node* item = new Node(diagramview,obj->objectName(),obj->objectName() + QString::number(counts[obj->objectName()]),int(diagramview->scene()->width()/2), int(diagramview->scene()->height()/2));


}

void MainWindow::onaddsource()
{
    QObject* obj = sender();
    Source source;
    source.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString());
    system.AddSource(source);
    qDebug() << "source added! " << obj->objectName();
    //Entity* item = new Entity(obj->objectName(), obj->objectName() + QString::number(counts[obj->objectName()]), diagramview,"Sources");

}

void MainWindow::onaddentity()
{
    //QObject* obj = sender();
    //counts[obj->objectName()] = counts[obj->objectName()] + 1;
    //qDebug() << "entity added! " << obj->objectName();
    //Entity* item = new Entity(obj->objectName(), obj->objectName() + QString::number(counts[obj->objectName()]), diagramview, QString::fromStdString(system.GetMetaModel()->GetItem(obj->objectName().toStdString())->CategoryType()));

}

void MainWindow::RefreshTreeView()
{
    Populate_TreeWidget();
    for (int i=0; i<system.BlockCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.block(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.block(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setText(0,QString::fromStdString(system.block(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }
}

string MainWindow::CreateNewName(string type)
{
    int i=1;
    string newname = type + " (" + aquiutils::numbertostring(i) + ")";
    while (system.object(newname)!=nullptr)
    {
        newname = type + " (" + aquiutils::numbertostring(i++) + ")";
    }
    return newname;

}
