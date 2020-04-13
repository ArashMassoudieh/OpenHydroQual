#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QDebug>
#include "enums.h"
#include "propmodel.h"
#include "delegate.h"
#include "node.h"
#include "edge.h"
#include "Script.h"
#include "QFileDialog"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dView = new DiagramView(ui->centralWidget,this);
    dView->setObjectName(QStringLiteral("graphicsView"));
    ui->horizontalLayout->addWidget(dView);
#ifndef Win_Version
    string modelfilename = qApp->applicationDirPath().toStdString() + "/../../resources/power_reservoirs_rules_source.json";
    string entitiesfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";
#else
    string modelfilename = qApp->applicationDirPath().toStdString() + "/resources/power_reservoirs_rules_source.json";
    string entitiesfilename = qApp->applicationDirPath().toStdString() + "/resources/settings.json";
#endif // !Win_Version
    system.GetQuanTemplate(modelfilename);  //Read the template from modelfilename
    system.ReadSystemSettingsTemplate(entitiesfilename); //Read the system settings
    RefreshTreeView();
    //connect(ui->treeWidget, SIGNAL(closeEvent()),ui->actionObject_Browser, SLOT())

    connect(ui->actionObject_Browser,SIGNAL(triggered()),this,SLOT(on_check_object_browser()));
    connect(ui->dockWidget_3,SIGNAL(visibilityChanged(bool)),this,SLOT(on_object_browser_closed(bool)));
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    BuildObjectsToolBar();
    connect(ui->treeWidget,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(preparetreeviewMenu(const QPoint&)));
    connect(ui->treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(onTreeSelectionChanged(QTreeWidgetItem*)));
    ui->tableView->setItemDelegateForColumn(1,new Delegate(this,this));
    Populate_General_ToolBar();
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
        Item->setData(0,Qt::UserRole,"main");
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

        if (typecategory!="Blocks" && typecategory !="Connectors" && typecategory!="Settings")
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
                if (typecategory=="Sources")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddsource()));
                else if (typecategory == "Parameters")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddparameter()));
                else
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
    string name = CreateNewName(obj->objectName().toStdString());
    block.SetName(name);
    system.AddBlock(block);
    system.object(name)->SetName(name);
    Node *node = new Node(dView,&system);
    dView->repaint();
    system.object(name)->AssignRandomPrimaryKey();
    node->SetObject(system.object(name));
    RefreshTreeView();
 }

void MainWindow::onaddlink()
{
    QObject* obj = sender();
    dView->setconnectfeature(obj->objectName());
    foreach (QAction* action, ui->mainToolBar->actions())
    {
        if (action->objectName()!=obj->objectName())
            action->setChecked(false);
        else
            action->setChecked(true);
    }

}

void MainWindow::AddLink(const QString &LinkName, const QString &sourceblock, const QString &targetblock, const QString &type,  Edge* edge)
{
    Link link;
    link.SetQuantities(system.GetMetaModel(),type.toStdString());
    link.SetType(type.toStdString());
    link.SetName(LinkName.toStdString());
    system.AddLink(link,sourceblock.toStdString(),targetblock.toStdString());
    system.object(LinkName.toStdString())->SetName(LinkName.toStdString());
    system.object(LinkName.toStdString())->AssignRandomPrimaryKey();
    edge->SetObject(system.object(LinkName.toStdString()));
    foreach (QAction* action, ui->mainToolBar->actions())
    {
        action->setChecked(false);
    }
    RefreshTreeView();

}


void MainWindow::onaddsource()
{
    QObject* obj = sender();
    Source source;
    source.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString());
    source.SetType(obj->objectName().toStdString());
    string name = CreateNewName(obj->objectName().toStdString());
    source.SetName(name);
    system.AddSource(source);
    qDebug() << "source added! " << obj->objectName();
    system.object(name)->SetName(name);
    RefreshTreeView();
}

void MainWindow::onaddparameter()
{
    QObject* obj = sender();
    Parameter parameter;
    parameter.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString());
    parameter.SetType(obj->objectName().toStdString());
    string name = CreateNewName(obj->objectName().toStdString());
    parameter.SetName(name);
    system.Parameters().Append(name,parameter);
    qDebug() << "parameter added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
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

    for (int i=0; i<system.SettingsCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.Setting(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.Setting(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.Setting(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

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
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.block(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (int i=0; i<system.LinksCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.link(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.link(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.link(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (int i=0; i<system.SourcesCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.source(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.source(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.source(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (int i=0; i<system.ParametersCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.Parameters()[i]->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.Parameters()[i]->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.Parameters()[i]->GetName()));
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

void MainWindow::preparetreeviewMenu(const QPoint &pos)
{
    QTreeWidget *tree = ui->treeWidget;

    QTreeWidgetItem *nd = tree->itemAt( pos );

    qDebug()<<nd->data(0,Qt::UserRole);

    qDebug()<<pos<<nd->text(0);

    if (nd->data(0,Qt::UserRole)=="main")
    {
        QAction *AddAct = new QAction(QIcon(":/Resource/warning32.ico"), "Add " + nd->text(0), this);
        AddAct->setStatusTip("Append " + nd->text(0));
        connect(AddAct, SIGNAL(triggered()), this, SLOT(onAddItem()));
        QMenu menu(this);
        menu.addAction(AddAct);

        QPoint pt(pos);
        menu.exec( tree->mapToGlobal(pos) );

    }

    if (nd->data(0,Qt::UserRole)=="child")
    {
        QAction *DeleteAct = new QAction(QIcon(":/Resource/warning32.ico"), "Delete" , this);
        DeleteAct->setStatusTip("Delete");
        connect(DeleteAct, SIGNAL(triggered()), this, SLOT(onDeleteItem()));
        QMenu menu(this);
        menu.addAction(DeleteAct);

        QPoint pt(pos);
        menu.exec( tree->mapToGlobal(pos) );

    }
}

void MainWindow::onTreeSelectionChanged(QTreeWidgetItem *current)
{
    qDebug()<<current->data(0,Qt::UserRole);
    current->text(0);

    if (current->data(0,Qt::UserRole)=="child")
    {   if (system.object(current->text(0).toStdString())==nullptr)
        {
            RefreshTreeView();
            return;
        }
        PopulatePropertyTable(system.object(current->text(0).toStdString())->GetVars());
    }
}

void MainWindow::PopulatePropertyTable(QuanSet* quanset)
{
    if (propmodel != nullptr)
        delete  propmodel;
    if (quanset!=nullptr)
        propmodel = new PropModel(quanset,this,this);
    else
        propmodel = nullptr;
    ui->tableView->setModel(propmodel);
}

void MainWindow::Populate_General_ToolBar()
{
    // Save //
    QAction* actionsave = new QAction(this);
    actionsave->setObjectName("Save");
    QIcon iconsave;
    iconsave.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/Save.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionsave->setIcon(iconsave);
    ui->GeneraltoolBar->addAction(actionsave);
    actionsave->setText("Save");
    connect(actionsave, SIGNAL(triggered()), this, SLOT(onsave()));
    // Open //
    QAction* actionopen = new QAction(this);
    actionopen->setObjectName("Open");
    QIcon iconopen;
    iconopen.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/open.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionopen->setIcon(iconopen);
    ui->GeneraltoolBar->addAction(actionopen);
    actionopen->setText("Save");
    connect(actionopen, SIGNAL(triggered()), this, SLOT(onopen()));
    // ZoomAll //
    QAction* actionzoomall = new QAction(this);
    actionzoomall->setObjectName("Open");
    QIcon iconzoomall;
    iconzoomall.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/Full_screen_view.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionzoomall->setIcon(iconzoomall);
    ui->GeneraltoolBar->addAction(actionzoomall);
    actionzoomall->setText("Zoom Extends");
    connect(actionzoomall, SIGNAL(triggered()), this, SLOT(onzoomall()));
    // ZoomIn //
    QAction* actionzoomin = new QAction(this);
    actionzoomin->setObjectName("Zoom_In");
    QIcon iconzoomin;
    iconzoomin.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/zoom_in.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionzoomin->setIcon(iconzoomin);
    ui->GeneraltoolBar->addAction(actionzoomin);
    actionzoomin->setText("Zoom In");
    connect(actionzoomin, SIGNAL(triggered()), this, SLOT(onzoomin()));
    // ZoomOut //
    QAction* actionzoomout = new QAction(this);
    actionzoomall->setObjectName("Zoom_Out");
    QIcon iconzoomout;
    iconzoomout.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/zoom_out.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionzoomout->setIcon(iconzoomout);
    ui->GeneraltoolBar->addAction(actionzoomout);
    actionzoomout->setText("Zoom Out");
    connect(actionzoomout, SIGNAL(triggered()), this, SLOT(onzoomout()));

}

void MainWindow::onzoomin()
{
    dView->scaleView(1.25);
}

void MainWindow::onzoomout()
{
    dView->scaleView(1 / 1.25);
}

void MainWindow::onzoomall()
{
    QRectF newRect = dView->MainGraphicsScene->itemsBoundingRect();
    float width = float(newRect.width());
    float height = float(newRect.height());
    float scale = float(1.1);
    newRect.setLeft(newRect.left() - float(scale - 1) / 2 * float(width));
    newRect.setTop(newRect.top() - (scale - 1) / 2 * height);
    newRect.setWidth(qreal(width * scale));
    newRect.setHeight(qreal(height * scale));

    dView->fitInView(newRect,Qt::KeepAspectRatio);
}

void MainWindow::onsave()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save"), "",
            tr("script files (*.scr)"));
    if (fileName!="")
    {
        system.SavetoScriptFile(fileName.toStdString());
    }

}


void MainWindow::onopen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Script files (*.scr);; All files (*.*)"));
    if (fileName!="")
    {
        Script scr(fileName.toStdString(),&system);
        system.clear();
        system.CreateFromScript(scr);
    }
    RecreateGraphicItemsFromSystem();
    RefreshTreeView();
}

void MainWindow::RecreateGraphicItemsFromSystem()
{
    dView->DeleteAllItems();
    for (int i=0; i<system.BlockCount(); i++)
    {
        Node *node = new Node(dView,&system);
        dView->repaint();
        system.block(i)->AssignRandomPrimaryKey();
        node->SetObject(system.block(i));
    }
    for (int i=0; i<system.LinksCount(); i++)
    {
        Node *s_node = dView->node(QString::fromStdString(system.block(system.link(i)->s_Block_No())->GetName()));
        Node *e_node = dView->node(QString::fromStdString(system.block(system.link(i)->e_Block_No())->GetName()));
        if (s_node && e_node);
        {
            Edge *edge = new Edge(s_node,e_node,dView );
            system.link(i)->AssignRandomPrimaryKey();
            edge->SetObject(system.link(i));
        }
    }

}
