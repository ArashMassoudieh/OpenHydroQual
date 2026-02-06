/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#define openhydroqual_version "2.0.3"
#define last_modified "December, 9, 2025"

#ifdef _WIN32
#include <windows.h>
#undef SetProp
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
#endif

#define RECENT "recentFiles.txt"

#ifndef max_num_recent_files
#define max_num_recent_files 15
#endif

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
#include "ProgressWindow.h"
#ifndef QCharts
#include "plotter.h"
#else
#include "qplotwindow.h"
#endif
#include <QInputDialog>
#include "wizard_select_dialog.h"
#ifndef Qt6
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgGenerator>
#include <QtSvg/QSvgRenderer>
#else
#include <QGraphicsSvgItem>
#include <QSvgGenerator>
#include <QSvgRenderer>
#endif
#include "options.h"
#include <QFileInfo>
#include "gridgenerator.h"
#include "metamodelhelpdialog.h"
#include "VisualizationDialog.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setStyleSheet(
        "QToolBar {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                 stop:0 #f8f9fa, stop:1 #e9ecef);"
        "    border: none;"
        "    border-bottom: 1px solid #dee2e6;"
        "    spacing: 3px;"
        "    padding: 4px;"
        "}"
        "QToolBar::separator {"
        "    background: #dee2e6;"
        "    width: 1px;"
        "    margin: 4px 8px;"
        "}"
        "QToolButton {"
        "    background: transparent;"
        "    border: 1px solid transparent;"
        "    border-radius: 4px;"
        "    padding: 6px;"
        "    margin: 2px;"
        "}"
        "QToolButton:hover {"
        "    background: #e3f2fd;"
        "    border: 1px solid #90caf9;"
        "}"
        "QToolButton:pressed {"
        "    background: #bbdefb;"
        "}"
        "QToolButton:checked {"
        "    background: #2196F3;"
        "    color: white;"
        "}"
    );
    
    ui->setupUi(this);

    removeToolBar(ui->BlocksToolBar);
    removeToolBar(ui->LinksToolBar);
    removeToolBar(ui->SourcesToolBar);
    removeToolBar(ui->mainToolBar);

    // Row 1: Blocks
    addToolBar(Qt::TopToolBarArea, ui->BlocksToolBar);

    // Row 2: Links  
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, ui->LinksToolBar);

    // Row 3: Sources and other categories (all on same row initially)
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, ui->SourcesToolBar);
    // Other toolbars will be added dynamically in BuildObjectsToolBar()

    // Hide mainToolBar
    ui->mainToolBar->setVisible(false);

    resource_directory = QString::fromStdString(RESOURCE_DIRECTORY);
    qDebug()<<"Resource Directory: " << resource_directory;
    QIcon mainicon(QString::fromStdString(RESOURCE_DIRECTORY) + "/Icons/Aquifolium.png");
    workingfolder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    setWindowIcon(mainicon);
    dView = new DiagramView(ui->centralWidget,this);
    dView->setObjectName(QStringLiteral("graphicsView"));
    ui->horizontalLayout->addWidget(dView);
    LogWindow = new logwindow(this);
    LogWindow->show();

    maintemplatefilename = RESOURCE_DIRECTORY + "/main_components.json";
    qDebug()<<QString::fromStdString(maintemplatefilename);
    entitiesfilename = RESOURCE_DIRECTORY + "/settings.json";
    system.DefaultTemplatePath() = RESOURCE_DIRECTORY + "/";
    qDebug()<<QString::fromStdString(entitiesfilename);
    Log("Default Template Location is set to '" + QString::fromStdString(system.DefaultTemplatePath()) + "'");
    if (system.GetQuanTemplate(maintemplatefilename)) //Read the template from modelfilename
    {
        Log("Template was successfully loaded from '" + QString::fromStdString(maintemplatefilename) + "'");
    }
    else {
        LogError("Template" + QString::fromStdString(maintemplatefilename) + "' was not loaded properly");
    }
    qDebug()<<"Main Template loaded";
    if (system.ReadSystemSettingsTemplate(entitiesfilename)) //Read the system settings
    {
        qDebug()<<"Setting successfully loaded";
        Log("Setting was successfully loaded from '" + QString::fromStdString(entitiesfilename) + "'");
    }
    else
    {
        LogError("Failed to load the setting file '" + QString::fromStdString(entitiesfilename) + "'");
    }
    qDebug()<<"Settings Loaded";
    qDebug()<<"Refreshing Tree...";
    RefreshTreeView();
    //connect(ui->treeWidget, SIGNAL(closeEvent()),ui->actionObject_Browser, SLOT())

    connect(ui->actionObject_Browser,SIGNAL(triggered()),this,SLOT(on_check_object_browser()));
    connect(ui->actionLog_Window,SIGNAL(triggered()),this,SLOT(on_check_showlogwindow()));
    connect(ui->dockWidget_3,SIGNAL(visibilityChanged(bool)),this,SLOT(on_object_browser_closed(bool)));
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    BuildObjectsToolBar();
    ReCreateObjectsMenu();
    connect(ui->treeWidget,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(preparetreeviewMenu(const QPoint&)));
    connect(ui->treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(onTreeSelectionChanged(QTreeWidgetItem*)));

    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(onopen()));
    connect(ui->actionImport, SIGNAL(triggered()), this, SLOT(onimport()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(onsave()));
    connect(ui->actionSave_as,SIGNAL(triggered()),this,SLOT(onsaveas()));
    connect(ui->actionSave_State_to_Json,SIGNAL(triggered()),this,SLOT(onsaveasJson()));
    connect(ui->actionLoad_Json,SIGNAL(triggered()),this,SLOT(onloadJson()));
    connect(ui->actionExport_to_SVG,SIGNAL(triggered()),this,SLOT(onexporttosvg()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(onabout()));
    connect(ui->actionUndo,SIGNAL(triggered()),this,SLOT(on_Undo()));
    connect(ui->actionRedo,SIGNAL(triggered()),this,SLOT(on_Redo()));
    connect(ui->actionCreate_2D_Array,SIGNAL(triggered()),this,SLOT(onCreate2dArray()));
    connect(this,SIGNAL(closed()),this,SLOT(onclosed()));
    connect(ui->actionLoad_a_new_template,SIGNAL(triggered()),this,SLOT(loadnewtemplate()));
    connect(ui->actionAddPlugin,SIGNAL(triggered()),this,SLOT(addplugin()));
    connect(ui->actionAdd_plugin,SIGNAL(triggered()),this,SLOT(adddefaultpluging()));
    connect(ui->actionOptions,SIGNAL(triggered()),this,SLOT(optionsdialog()));
    connect(ui->actionOpen_Results,SIGNAL(triggered()),this,SLOT(loadresults()));
    connect(ui->actionNew_Project,SIGNAL(triggered()),this,SLOT(onnewproject()));
    connect(ui->actionVisualize, &QAction::triggered, this, &MainWindow::onVisualize);
    ui->actionVisualize->setEnabled(false);
    connect(ui->actionComponent_description, &QAction::triggered,this, &MainWindow::oncomponentdescriptions);
    PropertiesWidget = new ItemPropertiesWidget(ui->dockWidgetContents_3);
    PropertiesWidget->tableView()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->verticalLayout->addWidget(PropertiesWidget);
    connect(PropertiesWidget->tableView(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(tablePropShowContextMenu(const QPoint&)));

    PropertiesWidget->tableView()->setItemDelegateForColumn(1,new Delegate(this,this));
    Populate_General_ToolBar();
    readRecentFilesList();
    undoData = UndoData(this);
    undoData.AppendtoLast(&system);
    if (undoData.active==0) InactivateUndo();
    if (undoData.active==undoData.Systems.size()-1) InactivateRedo();
    qDebug()<<graphsClipboard.size();
    graphsClipboard.clear();


}

void MainWindow::InactivateUndo(bool yes)
{
    ui->actionUndo->setEnabled(!yes);
    ui->actionUndo->setDisabled(yes);
}

void MainWindow::InactivateRedo(bool yes)
{
    ui->actionRedo->setEnabled(!yes);
    ui->actionRedo->setDisabled(yes);
}


void MainWindow::ResetSystem()
{
    system = System();
    system.clear();
    system.addedtemplates.clear();
    addedtemplatefilenames = system.addedtemplates;

    maintemplatefilename = RESOURCE_DIRECTORY + "/main_components.json";
    entitiesfilename = RESOURCE_DIRECTORY + "/settings.json";
    system.DefaultTemplatePath() = RESOURCE_DIRECTORY + "/";


    Log("Default Template Location is set to '" + QString::fromStdString(system.DefaultTemplatePath()) + "'");
    if (system.GetQuanTemplate(maintemplatefilename)) //Read the template from modelfilename
    {
        Log("Template was successfully loaded from '" + QString::fromStdString(maintemplatefilename) + "'");
    }
    else {
        LogError("Template" + QString::fromStdString(maintemplatefilename) + "' was not loaded properly");
    }
    if (system.ReadSystemSettingsTemplate(entitiesfilename)) //Read the system settings
    {
        Log("Setting was successfully loaded from '" + QString::fromStdString(entitiesfilename) + "'");
    }
    else
    {
        LogError("Failed to load the setting file '" + QString::fromStdString(entitiesfilename) + "'");
    }

    RefreshTreeView();


}

MainWindow::~MainWindow()
{
    if (rtw)
        rtw->close();
    LogWindow->close();
    delete ui;
    exit; 
}

void MainWindow::tablePropShowContextMenu(const QPoint&pos)
{
    QModelIndex i1 = PropertiesWidget->tableView()->indexAt(pos);
    int row = i1.row();
    QModelIndex i2 = i1.sibling(row, 1);
    tableitemrightckicked = i2;
    if (i1.column() == 0)
    {
        menu = std::unique_ptr<QMenu>(new QMenu(this));

        QMenu *estimatesMenu = new QMenu("Parameters");
        menu->addMenu(estimatesMenu);
        estimatesMenu->setEnabled(false);
        if (i2.data(CustomRoleCodes::Role::EstimateCode).toBool())
        {
            for (unsigned int i=0 ; i< GetSystem()->ParametersCount(); i++)
                estimatesMenu->addAction(QString::fromStdString(GetSystem()->Parameters()[i]->GetName()));// , this, SLOT(addParameter()));
            //addParameterIndex(i1); // tableProp->indexAt(pos));
            connect(estimatesMenu, SIGNAL(triggered(QAction*)), this, SLOT(addParameter(QAction*)));
            estimatesMenu->setEnabled(true);
        }

        menu->exec(PropertiesWidget->tableView()->mapToGlobal(pos));
    }
    if (i1.column() == 1)
    {
        menu = std::unique_ptr<QMenu>(new QMenu(this));

        if (PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::TypeRole).toString().contains("ComboBox"))
        {
            QAction* action = menu->addAction("Clear");
            connect(action,SIGNAL(triggered()),this, SLOT(clearcombobox()));
            menu->exec(PropertiesWidget->tableView()->mapToGlobal(pos));
        }
        if (PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::TypeRole).toString().contains("DateTime"))
        {
            QAction* action = menu->addAction("Insert in numeric format");
            connect(action,SIGNAL(triggered()),this, SLOT(insertnumberasdate()));
            menu->exec(PropertiesWidget->tableView()->mapToGlobal(pos));
        }
        if (PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::TypeRole).toString().contains("Browser"))
        {
            QAction* action = menu->addAction("Plot");
            connect(action,SIGNAL(triggered()),this, SLOT(PlotTimeSeries()));
            menu->exec(PropertiesWidget->tableView()->mapToGlobal(pos));
        }
    }
}

void MainWindow::clearcombobox()
{
    PropertiesWidget->tableView()->model()->setData(tableitemrightckicked, "");
}

void MainWindow::insertnumberasdate()
{
    double x = QInputDialog::getDouble(this,"Date/Time Value: ","Date/Time value", PropertiesWidget->tableView()->model()->data(tableitemrightckicked).toDouble(),0,20000,2);
    PropertiesWidget->tableView()->model()->setData(tableitemrightckicked, QString::number(x));
}

void MainWindow::PlotTimeSeries()
{
    QString timeseriesfilename = PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::fullFileNameRole).toString();
    QString objectname = PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::ObjectName).toString();
    QString varname = PropertiesWidget->tableView()->model()->data(tableitemrightckicked,CustomRoleCodes::VariableName).toString();

    Quan* quan = GetSystem()->object(objectname.toStdString())->Variable(varname.toStdString());

    if (quan->GetTimeSeries() != nullptr)
    {
#ifndef QCharts
        Plotter* plot = Plot(*quan->GetTimeSeries());
#else
        QPlotWindow* plot = Plot(*quan->GetTimeSeries(), quan);  // Pass the Quan object
#endif
        plot->SetYAxisTitle(varname);
    }



}

void MainWindow::addParameter(QAction* item)
{
    QString parameter = item->text();
    PropertiesWidget->tableView()->model()->setData(tableitemrightckicked, parameter, CustomRoleCodes::setParamRole);
    menu->hide();
    menu->setVisible(false);

}

QModelIndex MainWindow::addParameterIndex(const QModelIndex &index)
{
    static QModelIndex parameterIndex;
    if (index != QModelIndex()) parameterIndex = index;
    return parameterIndex;
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
    return true;
}

void MainWindow::on_check_object_browser()
{
    //ui->actionObject_Browser->setChecked(!ui->actionObject_Browser->isChecked());
    if (ui->actionObject_Browser->isChecked())
        ui->dockWidget_3->show();
    else
        ui->dockWidget_3->hide();

}

void MainWindow::on_check_showlogwindow()
{
    //ui->actionObject_Browser->setChecked(!ui->actionObject_Browser->isChecked());
    if (ui->actionLog_Window->isChecked())
        LogWindow->show();
    else
        LogWindow->hide();

}

void MainWindow::on_object_browser_closed(bool visible)
{
    ui->actionObject_Browser->setChecked(visible);
}

bool MainWindow::BuildObjectsToolBar()
{
    ui->mainToolBar->clear();
    ui->BlocksToolBar->clear();
    ui->LinksToolBar->clear();
    ui->SourcesToolBar->clear();

    // Clear any existing category toolbars
    for (QToolBar* toolbar : categoryToolbars_.values())
    {
        removeToolBar(toolbar);
        delete toolbar;
    }
    categoryToolbars_.clear();

    // Set smaller icon size and text style for standard toolbars
    for (QToolBar* toolbar : { ui->BlocksToolBar, ui->LinksToolBar, ui->SourcesToolBar }) {
        toolbar->setIconSize(QSize(20, 20));
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        toolbar->setVisible(true);
        toolbar->setStyleSheet(
            "QToolBar {"
            "    spacing: 2px;"
            "}"
            "QToolButton {"
            "    padding: 2px;"
            "    margin: 1px;"
            "    border-radius: 3px;"
            "}"
            "QToolButton:hover {"
            "    background: #e3f2fd;"
            "    border: 1px solid #90caf9;"
            "}"
            "QToolButton:pressed {"
            "    background: #bbdefb;"
            "}"
            "QToolButton:checked {"
            "    background: #2196F3;"
            "    color: white;"
            "}"
        );
    }

    // Add compact labels to toolbars
    addToolbarLabel(ui->BlocksToolBar, "Blocks:");
    addToolbarLabel(ui->LinksToolBar, "Links:");
    addToolbarLabel(ui->SourcesToolBar, "Sources:");

    // Build Blocks toolbar
    for (unsigned int i = 0; i < system.GetAllBlockTypes().size(); i++)
    {
        QAction* action = new QAction(this);
        action->setObjectName(QString::fromStdString(system.GetAllBlockTypes()[i]));
        QIcon icon;

        QString iconFileName = QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->IconFileName());
        QString iconPath;

        if (iconFileName.contains("/"))
            iconPath = iconFileName;
        else
            iconPath = QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/") + iconFileName;

        if (!QFile::exists(iconPath))
            LogError("Icon file '" + iconPath + "' was not found!");
        else
            icon.addFile(iconPath, QSize(), QIcon::Normal, QIcon::Off);

        action->setIcon(icon);
        action->setToolTip(QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->Description()));
        action->setText(QString::fromStdString(system.GetAllBlockTypes()[i]));
        ui->BlocksToolBar->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(onaddblock()));
    }

    // Build Links toolbar
    for (unsigned int i = 0; i < system.GetAllLinkTypes().size(); i++)
    {
        QAction* action = new QAction(this);
        action->setCheckable(true);
        action->setObjectName(QString::fromStdString(system.GetAllLinkTypes()[i]));
        QIcon icon;

        QString iconFileName = QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->IconFileName());
        QString iconPath;

        if (iconFileName.contains("/"))
            iconPath = iconFileName;
        else
            iconPath = QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/") + iconFileName;

        if (!QFile::exists(iconPath))
            LogError("Icon file '" + iconPath + "' was not found!");
        else
            icon.addFile(iconPath, QSize(), QIcon::Normal, QIcon::Off);

        action->setIcon(icon);
        action->setToolTip(QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->Description()));
        action->setText(QString::fromStdString(system.GetAllLinkTypes()[i]));
        ui->LinksToolBar->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(onaddlink()));
    }

    // Build other category toolbars - each category gets its own toolbar
    for (unsigned int j = 0; j < system.QGetAllCategoryTypes().size(); j++)
    {
        string typecategory = system.QGetAllCategoryTypes()[j].toStdString();

        if (typecategory != "Blocks" && typecategory != "Connectors" && typecategory != "Settings" && !typecategory.empty())
        {
            if (system.GetAllTypesOf(typecategory).size() > 0)
            {
                QToolBar* categoryToolbar = nullptr;
                QString categoryName = QString::fromStdString(typecategory);

                // Use SourcesToolBar for Sources, create new toolbars for others
                if (typecategory == "Sources")
                {
                    categoryToolbar = ui->SourcesToolBar;
                }
                else
                {
                    // Create new toolbar for this category
                    categoryToolbar = new QToolBar(categoryName, this);
                    categoryToolbar->setObjectName(categoryName + "ToolBar");
                    categoryToolbar->setIconSize(QSize(20, 20));
                    categoryToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
                    categoryToolbar->setMovable(true);
                    categoryToolbar->setFloatable(true);
                    categoryToolbar->setStyleSheet(
                        "QToolBar {"
                        "    spacing: 2px;"
                        "}"
                        "QToolButton {"
                        "    padding: 2px;"
                        "    margin: 1px;"
                        "    border-radius: 3px;"
                        "}"
                        "QToolButton:hover {"
                        "    background: #e3f2fd;"
                        "    border: 1px solid #90caf9;"
                        "}"
                        "QToolButton:pressed {"
                        "    background: #bbdefb;"
                        "}"
                        "QToolButton:checked {"
                        "    background: #2196F3;"
                        "    color: white;"
                        "}"
                    );

                    // Add to main window on the same row as Sources (no break)
                    addToolBar(Qt::TopToolBarArea, categoryToolbar);

                    // Store in map for cleanup later
                    categoryToolbars_[categoryName] = categoryToolbar;

                    // Add label to this toolbar
                    addToolbarLabel(categoryToolbar, categoryName + ":");
                }

                // Add items to the category toolbar
                for (unsigned int i = 0; i < system.GetAllTypesOf(typecategory).size(); i++)
                {
                    string type = system.GetAllTypesOf(typecategory)[i];
                    QAction* action = new QAction(this);
                    action->setCheckable(false);
                    action->setObjectName(QString::fromStdString(type));
                    QIcon icon;

                    QString iconFileName = QString::fromStdString(system.GetModel(type)->IconFileName());
                    QString iconPath;

                    if (iconFileName.contains("/"))
                        iconPath = iconFileName;
                    else
                        iconPath = QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/") + iconFileName;

                    if (!QFile::exists(iconPath))
                        LogError("Icon file '" + iconPath + "' was not found!");
                    else
                        icon.addFile(iconPath, QSize(), QIcon::Normal, QIcon::Off);

                    action->setIcon(icon);
                    action->setToolTip(QString::fromStdString(system.GetModel(type)->Description()));
                    action->setText(QString::fromStdString(type));

                    categoryToolbar->addAction(action);

                    if (typecategory == "Sources")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddsource()));
                    else if (typecategory == "Parameters")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddparameter()));
                    else if (typecategory == "Objective Functions")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddobjectivefunction()));
                    else if (typecategory == "Observations")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddobservation()));
                    else if (typecategory == "Constituents")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddconstituent()));
                    else if (typecategory == "Reactions")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddreaction()));
                    else if (typecategory == "Reaction Parameters")
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddreactionparameter()));
                    else
                        connect(action, SIGNAL(triggered()), this, SLOT(onaddentity()));
                }
            }
        }
    }

    // Hide mainToolBar as it's not needed
    ui->mainToolBar->setVisible(false);

    return true;
}

// Compact helper method:
void MainWindow::addToolbarLabel(QToolBar* toolbar, const QString& text)
{
    QLabel* label = new QLabel(text, this);
    label->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    color: #2196F3;"
        "    padding: 0 5px;"
        "    font-size: 9pt;"
        "}"
    );
    toolbar->addWidget(label);
    toolbar->addSeparator();
}

bool MainWindow::ReCreateObjectsMenu()
{
    ui->menuBlocks->clear();
    ui->menuLinks->clear();
    ui->menuChemistry->clear();
    ui->menuSources->clear();
    for (unsigned int i = 0; i < system.GetAllBlockTypes().size(); i++)
    {
        //qDebug() << QString::fromStdString(system.GetAllBlockTypes()[i]);
        QAction* action = new QAction(this);
        action->setObjectName(QString::fromStdString(system.GetAllBlockTypes()[i]));
        action->setText(QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->Description()));
        QIcon icon;
        if (QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()).contains("/"))
        {
            if (!QFile::exists(QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->IconFileName())))
                LogError("Icon file '" + QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()) + "' was not found!");
            else
                icon.addFile(QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {

            if (!QFile::exists(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllBlockTypes()[i])->IconFileName())))
                LogError("Icon file '" + QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()) + "' was not found!");
            else
                icon.addFile(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }
        action->setIcon(icon);
        action->setToolTip(QString::fromStdString(system.GetModel(system.GetAllBlockTypes()[i])->Description()));
        ui->menuBlocks->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(onaddblock()));

    }

    for (unsigned int i = 0; i < system.GetAllLinkTypes().size(); i++)
    {
        //qDebug() << QString::fromStdString(system.GetAllLinkTypes()[i]);
        QAction* action = new QAction(this);
        action->setCheckable(true);
        action->setObjectName(QString::fromStdString(system.GetAllLinkTypes()[i]));
        action->setText(QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->Description()));
        QIcon icon;
        if (QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()).contains("/"))
        {
            if (!QFile::exists(QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->IconFileName())))
                LogError("Icon file '" + QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()) + "' was not found!");
            else
                icon.addFile(QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {

            if (!QFile::exists(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllLinkTypes()[i])->IconFileName())))
                LogError("Icon file '" + QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()) + "' was not found!");
            else
                icon.addFile(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(system.GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        action->setIcon(icon);
        action->setToolTip(QString::fromStdString(system.GetModel(system.GetAllLinkTypes()[i])->Description()));
        ui->menuLinks->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(onaddlink()));
    }

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
                action->setText(QString::fromStdString(system.GetModel(type)->Description()));
                QIcon icon;

                icon.addFile(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + system.GetModel(type)->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
                if (QString::fromStdString(system.GetModel(type)->IconFileName()).contains("/"))
                {
                    if (!QFile::exists(QString::fromStdString(system.GetModel(type)->IconFileName())))
                        LogError("Icon file '" + QString::fromStdString(system.GetModel(type)->IconFileName()) + "' was not found!");
                    else
                        icon.addFile(QString::fromStdString(system.GetModel(type)->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
                }
                else
                {

                    if (!QFile::exists(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/" + system.GetModel(type)->IconFileName())))
                        LogError("Icon file '" + QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/" + system.GetModel(type)->IconFileName()) + "' was not found!");
                    else
                        icon.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/" + system.GetModel(type)->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
                }


                action->setIcon(icon);
                action->setToolTip(QString::fromStdString(system.GetModel(type)->Description()));
                if (typecategory=="Sources")
                    ui->menuSources->addAction(action);
                else if (typecategory == "Constituents" || typecategory == "Reactions" || typecategory == "Reaction Parameters")
                    ui->menuChemistry->addAction(action);
                action->setText(QString::fromStdString(system.GetModel(type)->Description()));
                if (typecategory=="Sources")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddsource()));
                else if (typecategory == "Parameters")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddparameter()));
                else if (typecategory == "Objective Functions")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddobjectivefunction()));
                else if (typecategory == "Observations")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddobservation()));
                else if (typecategory == "Constituents")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddconstituent()));
                else if (typecategory == "Reactions")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddreaction()));
                else if (typecategory == "Reaction Parameters")
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddreactionparameter()));
                else
                    connect(action, SIGNAL(triggered()), this, SLOT(onaddentity()));
            }

    }

    return true;
}


void MainWindow::onaddblock()
{

    QObject* obj = sender();
    Block block;
    //qDebug() << "Setting Quantities";
    if (!block.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString()))
    {
        LogError(QString::fromStdString(block.lasterror()));
        return;
    }
    //qDebug() << "Quantities Set";

    block.SetType(obj->objectName().toStdString());
    string name = CreateNewName(obj->objectName().toStdString());
    block.SetName(name);
    //qDebug() << "Adding Block to the system";
    undoData.SetActiveSystem(&system);
    system.AddBlock(block);
    system.object(name)->SetName(name);
    Node *node = new Node(dView,&system);
    //qDebug() << "Node Created!";
    dView->UpdateSceneRect();
    dView->repaint();
    //qDebug() << "DiagramView Repainted!";
    system.object(name)->AssignRandomPrimaryKey();
    system.AddAllConstituentRelateProperties(system.block(name));
    system.SetVariableParents();
    node->SetObject(system.object(name));
    RefreshTreeView();
    LogAddDelete("Block '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendAfterActive(&system);
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

bool MainWindow::AddLink(const QString &LinkName, const QString &sourceblock, const QString &targetblock, const QString &type,  Edge* edge)
{
    Link link;
    if (!link.SetQuantities(system.GetMetaModel(),type.toStdString()))
    {
        LogError(QString::fromStdString(link.lasterror()));
        LogError(QString::fromStdString(system.lasterror()));
        //RecreateGraphicItemsFromSystem();
        return false;
    }
    undoData.AppendAfterActive(&system);
    link.SetType(type.toStdString());
    link.SetName(LinkName.toStdString());
    undoData.SetActiveSystem(&system);
    if (!system.AddLink(link, sourceblock.toStdString(), targetblock.toStdString()))
    {
        qDebug() << QString::fromStdString(system.lasterror());
        QMessageBox::question(this, "Link does not match the connected block!", QString::fromStdString(system.lasterror()), QMessageBox::Ok);
        LogError(QString::fromStdString(system.lasterror()));
        //RecreateGraphicItemsFromSystem();
        return false; 
    }
    system.object(LinkName.toStdString())->SetName(LinkName.toStdString());
    system.object(LinkName.toStdString())->AssignRandomPrimaryKey();
    edge->SetObject(system.object(LinkName.toStdString()));
    foreach (QAction* action, ui->LinksToolBar->actions())
    {
        action->setChecked(false);
    }

    foreach (QAction* action, ui->menuLinks->actions())
    {
        action->setChecked(false);
    }
    RefreshTreeView();
    system.AddAllConstituentRelateProperties(system.link(LinkName.toStdString()));
    system.SetVariableParents();

    LogAddDelete("Link '" + LinkName + "' was added!");
    undoData.AppendAfterActive(&system);
    return true;
}


void MainWindow::onaddsource()
{
    QObject* obj = sender();
    Source source;
    source.SetQuantities(system.GetMetaModel(),obj->objectName().toStdString());
    source.SetType(obj->objectName().toStdString());
    string name = CreateNewName(obj->objectName().toStdString());
    source.SetName(name);
    undoData.AppendAfterActive(&system);
    system.AddSource(source);
    //qDebug() << "source added! " << obj->objectName();
    system.object(name)->SetName(name);
    system.AddAllConstituentRelateProperties(system.source(name));
    RefreshTreeView();
    LogAddDelete("Source '" + QString::fromStdString(name) + "' was added!");
}

void MainWindow::onaddparameter()
{
    QObject* obj = sender();
    Parameter parameter;
    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString());
        objectname = obj->objectName().toStdString();
    }
    else
    {   name = CreateNewName("Parameter");
        objectname = "Parameter";
    }
    parameter.SetQuantities(system.GetMetaModel(),objectname);
    parameter.SetType(objectname);
    parameter.SetName(name);
    undoData.SetActiveSystem(&system);
    system.Parameters().Append(name,parameter);
    //qDebug() << "parameter added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
    LogAddDelete("Parameter '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);
}

void MainWindow::onaddobjectivefunction()
{
    QObject* obj = sender();
    Objective_Function objective_function;

    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString());
        objectname = obj->objectName().toStdString();
    }
    else
    {
        name = CreateNewName("Objective Function");
        objectname = "Objective_Function";
    }

    objective_function.SetQuantities(system.GetMetaModel(),objectname);
    objective_function.SetType(objectname);

    objective_function.SetName(name);
    undoData.SetActiveSystem(&system);
    system.AppendObjectiveFunction(name,objective_function);
    //qDebug() << "objective function added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
    LogAddDelete("Objective Function '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);

}

void MainWindow::onaddobservation()
{
    QObject* obj = sender();
    Observation observation;

    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString());
        objectname = obj->objectName().toStdString();
    }
    else
    {
        name = CreateNewName("Observation");
        objectname = "Observation";
    }

    observation.SetQuantities(system.GetMetaModel(),objectname);
    observation.SetType(objectname);

    observation.SetName(name);
    undoData.SetActiveSystem(&system);
    system.AddObservation(observation);
    system.object(name)->SetName(name);
    //qDebug() << "observation added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
    LogAddDelete("Observation '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);
}

void MainWindow::onaddconstituent()
{
    QObject* obj = sender();
    Constituent constituent;

    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString(),false);
        objectname = obj->objectName().toStdString();
    }
    else
    {
        name = CreateNewName("Constituent",false);
        objectname = "Constituent";
    }

    constituent.SetQuantities(system.GetMetaModel(),objectname);
    constituent.SetType(objectname);

    constituent.SetName(name);
    undoData.SetActiveSystem(&system);
    system.AddConstituent(constituent);
    system.object(name)->SetName(name);
    //qDebug() << "Constituent added! " << obj->objectName();
    //system.object(name)->SetName(name);
    system.AddConstituentRelateProperties(system.constituent(name));
    RefreshTreeView();
    LogAddDelete("Constituent '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);
}

void MainWindow::onaddreaction()
{
    QObject* obj = sender();
    Reaction reaction;

    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString());
        objectname = obj->objectName().toStdString();
    }
    else
    {
        name = CreateNewName("Reaction");
        objectname = "Reaction";
    }

    reaction.SetQuantities(system.GetMetaModel(),objectname);
    reaction.SetType(objectname);

    reaction.SetName(name);
    undoData.SetActiveSystem(&system);
    system.AddReaction(reaction);
    system.object(name)->SetName(name);
    system.AddAllConstituentRelateProperties(system.reaction(name));
    system.AddConstituentRelateProperties(system.reaction(name));
    //qDebug() << "Reaction added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
    LogAddDelete("Reaction '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);

}

void MainWindow::onaddreactionparameter()
{
    QObject* obj = sender();
    RxnParameter reactionparameter;

    string name;
    string objectname;
    if (obj->objectName()!="")
    {   name = CreateNewName(obj->objectName().toStdString(),false);
        objectname = obj->objectName().toStdString();
    }
    else
    {
        name = CreateNewName("Reaction Parameter",false);
        objectname = "ReactionParameter";
    }

    reactionparameter.SetQuantities(system.GetMetaModel(),objectname);
    reactionparameter.SetType(objectname);

    reactionparameter.SetName(name);
    undoData.SetActiveSystem(&system);
    system.AddReactionParameter(reactionparameter);
    system.object(name)->SetName(name);
    system.AddConstituentRelateProperties(system.reactionparameter(name));
    //qDebug() << "Reaction Parameter added! " << obj->objectName();
    //system.object(name)->SetName(name);
    RefreshTreeView();
    LogAddDelete("Reaction '" + QString::fromStdString(name) + "' was added!");
    undoData.AppendtoLast(&system);
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
    if (propmodel != nullptr)
        delete  propmodel;
    propmodel = nullptr;
    for (unsigned int i=0; i<system.SettingsCount(); i++)
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
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setText(0,QString::fromStdString(system.Setting(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.BlockCount(); i++)
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
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setText(0,QString::fromStdString(system.block(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.LinksCount(); i++)
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
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setText(0,QString::fromStdString(system.link(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.SourcesCount(); i++)
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
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.source(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ParametersCount(); i++)
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
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setText(0,QString::fromStdString(system.Parameters()[i]->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ObjectiveFunctionsCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.ObjectiveFunctions()[i]->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.ObjectiveFunctions()[i]->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.ObjectiveFunctions()[i]->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ObservationsCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.observation(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.observation(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.observation(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ConstituentsCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.constituent(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.constituent(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.constituent(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ReactionParametersCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.reactionparameter(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.reactionparameter(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.reactionparameter(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }

    for (unsigned int i=0; i<system.ReactionsCount(); i++)
    {
        QString TypeCategory = QString::fromStdString(system.reaction(i)->TypeCategory());
        QList<QTreeWidgetItem*> MatchedItems = ui->treeWidget->findItems(QString::fromStdString(system.reaction(i)->TypeCategory()),Qt::MatchExactly);
        if (MatchedItems.size()==0)
            qDebug() << "No category called '" + TypeCategory + "' was found!";
        else if (MatchedItems.size()>1)
            qDebug() << "More than one category called '" + TypeCategory + "' was found!";
        else {
            QTreeWidgetItem *treeitem = ui->treeWidget->findItems(TypeCategory,Qt::MatchExactly)[0];
            QTreeWidgetItem *treechlditem = new QTreeWidgetItem(treeitem);
            treechlditem->setData(0,CustomRoleCodes::Role::TypeRole,TypeCategory);
            treechlditem->setData(0,Qt::UserRole,"child");
            treechlditem->setText(0,QString::fromStdString(system.reaction(i)->GetName()));
            treeitem->addChild(treechlditem);
        }
    }
}

string MainWindow::CreateNewName(string type, bool allow_parathesis)
{
    int i=1;
    string newname; 
    if (allow_parathesis)
    {
        newname = type + " (" + aquiutils::numbertostring(i) + ")";
        while (system.object(newname) != nullptr)
        {
            newname = type + " (" + aquiutils::numbertostring(i++) + ")";
        }
    }
    else
    {
        newname = type + "_" + aquiutils::numbertostring(i);
        while (system.object(newname) != nullptr)
        {
            newname = type+ "_" + aquiutils::numbertostring(i++);
        }
    }
    return newname;

}

void MainWindow::preparetreeviewMenu(const QPoint &pos)
{
    menu = nullptr;
    QTreeWidget *tree = ui->treeWidget;

    QTreeWidgetItem *nd = tree->itemAt( pos );

    //qDebug()<<nd->data(0,Qt::UserRole);

    //qDebug()<<pos<<nd->text(0);

    if (nd->data(0,Qt::UserRole)=="main")
    {
        if (nd->text(0)=="Parameters" || nd->text(0)=="Objective Functions" || nd->text(0)=="Reactions" || nd->text(0)=="Reaction Parameters" || nd->text(0)=="Constituents" || nd->text(0)=="Observations")
        {
            menu = std::unique_ptr<QMenu>(new QMenu(this));
            QAction *AddAct = new QAction(QIcon(":/Resource/warning32.ico"), "Add " + nd->text(0), this);
            AddAct->setProperty("group",nd->text(0));
            AddAct->setStatusTip("Append " + nd->text(0));
            connect(AddAct, SIGNAL(triggered()), this, SLOT(onAddItemThroughTreeViewRightClick()));
            menu->addAction(AddAct);
            QPoint pt(pos);
        }
        if (nd->data(0,CustomRoleCodes::Role::TypeRole).toString() == "Objective Functions")
        {
            QMenu* results = menu->addMenu("Results");
            timeseriestobeshown = "Time Series";
            QAction* graphaction = results->addAction(timeseriestobeshown);
            QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.objectivefunction(nd->text(0).toStdString())->GetOutputItem()));
            graphaction->setData(v);
            //called_by_clicking_on_graphical_object = true;
            connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
        }
        if (nd->data(0,CustomRoleCodes::Role::TypeRole).toString() == "Observations")
        {
            QMenu* results = menu->addMenu("Results");
            timeseriestobeshown = "Modeled vs Measured";
            QAction* graphaction = results->addAction(timeseriestobeshown);
            QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.observation(nd->text(0).toStdString())->GetOutputItem()));
            graphaction->setData(v);
            graphaction->setProperty("object",QString::fromStdString(system.observation(nd->text(0).toStdString())->GetName()));
            //called_by_clicking_on_graphical_object = true;
            connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
        }
        if (nd->data(0, CustomRoleCodes::Role::TypeRole).toString() == "Sources")
        {
            menu = std::unique_ptr<QMenu>(new QMenu(this));
            timeseriestobeshown = "Precipitation";
            if (GetSystem()->source(nd->text(0).toStdString())->Variable("timeseries")!=nullptr)
            {
                if (GetSystem()->source(nd->text(0).toStdString())->Variable("timeseries")->GetTimeSeries()!=nullptr)
                {   QAction* graphaction = menu->addAction(timeseriestobeshown);
                    QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + nd->text(0));
                    graphaction->setData(v);
                    //called_by_clicking_on_graphical_object = true;
                    connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
                }
            }
        }
        
        if (menu)
            menu->exec( tree->mapToGlobal(pos) );

        return;

    }

    if (nd->data(0,Qt::UserRole)=="child")
    {
        QAction *DeleteAct = new QAction(QIcon(":/Resource/warning32.ico"), "Delete" , this);
        DeleteAct->setStatusTip("Delete");
        DeleteAct->setData(nd->text(0));
        connect(DeleteAct, SIGNAL(triggered()), this, SLOT(onDeleteItem()));
        QMenu menu(this);
        menu.addAction(DeleteAct);
        QMenu* results = menu.addMenu("Results");
        if (system.object(nd->text(0).toStdString())==nullptr)
        {
            RefreshTreeView();
            return;
        }
        for (unsigned int i = 0; i < system.object(nd->text(0).toStdString())->ItemswithOutput().size(); i++)
        {
            timeseriestobeshown = QString::fromStdString(system.object(nd->text(0).toStdString())->ItemswithOutput()[i]);
            QAction* graphaction = results->addAction(timeseriestobeshown);
            QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.object(nd->text(0).toStdString())->Variable(timeseriestobeshown.toStdString())->GetOutputItem()));
            graphaction->setData(v);
            //called_by_clicking_on_graphical_object = true;
            connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
        }


        if (system.object(nd->text(0).toStdString())->GetType()=="Parameter")
        {
            QMenu* posterior_results = menu.addMenu("Posterior results");
            timeseriestobeshown = "distribution";
            if (system.parameter(nd->text(0).toStdString())->GetPosteriorDistribution().size()!=0)
            {
                QAction* graphaction = posterior_results->addAction("Distribution");
                QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.object(nd->text(0).toStdString())->GetName()));
                graphaction->setData(v);
                connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
            }
            if (system.parameter(nd->text(0).toStdString())->GetMCMCSamples().size()!=0)
            {
                QAction* graphaction = posterior_results->addAction("Marcov Chain");
                QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.object(nd->text(0).toStdString())->GetName()));
                graphaction->setData(v);
                connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
            }

        }

        if (system.object(nd->text(0).toStdString())->GetType()=="Observation")
        {
            QMenu* posterior_results = menu.addMenu("Posterior results");
            timeseriestobeshown = "posterior";
            if (system.observation(nd->text(0).toStdString())->Realizations().size()!=0)
            {
                QAction* graphaction = posterior_results->addAction("Realizations");
                QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.object(nd->text(0).toStdString())->GetName()));
                graphaction->setData(v);
                connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
            }
            if (system.observation(nd->text(0).toStdString())->Percentile95().size()!=0)
            {
                QAction* graphaction = posterior_results->addAction("95 percent bracket");
                QVariant v = QVariant::fromValue(timeseriestobeshown + ";" + QString::fromStdString(system.object(nd->text(0).toStdString())->GetName()));
                graphaction->setData(v);
                connect(graphaction, SIGNAL(triggered()), this, SLOT(showgraph()));
            }

        }
        menu.exec( tree->mapToGlobal(pos) );
    }
}

void MainWindow::showgraph()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QStringList keys = act->data().toString().split(";");
    QString key2 = keys[0];
    QString item = keys[1];
    if (key2 == "Precipitation")
    {
        if (GetSystem()->source(item.toStdString())->Variable("timeseries")->GetTimeSeries() != nullptr)
        {
#ifndef QCharts
            Plotter* plot = Plot(*GetSystem()->source(item.toStdString())->Variable("timeseries")->GetTimeSeries());
#else
            QPlotWindow* plot = Plot(*GetSystem()->source(item.toStdString())->Variable("timeseries")->GetTimeSeries());
#endif
            plot->SetYAxisTitle(act->text());
        }
    return;
    }
    if (key2 == "Modeled vs Measured")
    {
        QString object = act->property("object").toString();
        if (GetSystem()->observation(object.toStdString())->Variable("observed_data")->GetTimeSeries() != nullptr)
        {
            if (GetSystem()->GetOutputs().Contains(item.toStdString()))
            {
#ifndef QCharts
                Plotter* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()],*GetSystem()->observation(object.toStdString())->Variable("observed_data")->GetTimeSeries());
#else
                QPlotWindow* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()],*GetSystem()->observation(object.toStdString())->Variable("observed_data")->GetTimeSeries());
#endif
                plot->SetYAxisTitle(act->text());
            }
            else
            {
#ifndef QCharts
                Plotter* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()],*GetSystem()->observation(object.toStdString())->Variable("observed_data")->GetTimeSeries());
#else
                QPlotWindow* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()],*GetSystem()->observation(object.toStdString())->Variable("observed_data")->GetTimeSeries());
#endif

                plot->SetYAxisTitle(act->text());
            }
        }
        else if (GetSystem()->GetOutputs().Contains(item.toStdString()))
        {
#ifndef QCharts
            Plotter* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()]);
#else
            QPlotWindow* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()]);
#endif
            plot->SetYAxisTitle(act->text());
        }
    }
    else if (key2 == "distribution")
    {
        if (act->text()=="Distribution")
        {
#ifndef QCharts
            Plotter* plot = Plot(GetSystem()->parameter(item.toStdString())->GetPosteriorDistribution(),false);
#else
            QPlotWindow* plot = Plot(GetSystem()->parameter(item.toStdString())->GetPosteriorDistribution(),false);
#endif
            plot->AddData(GetSystem()->parameter(item.toStdString())->PriorDistribution());
            plot->SetYAxisTitle("Density");
            plot->SetXAxisTitle(item);
        }
        else if (act->text()=="Marcov Chain")
        {
#ifndef QCharts
            Plotter* plot = Plot(GetSystem()->parameter(item.toStdString())->GetMCMCSamples(),false);
#else
            QPlotWindow* plot = Plot(GetSystem()->parameter(item.toStdString())->GetMCMCSamples(),false);
#endif
            plot->SetYAxisTitle(item);
            plot->SetXAxisTitle("Sample");
        }
    }
    else if (key2 == "posterior")
    {
        if (act->text()=="Realizations")
        {
#ifndef QCharts
            Plotter* plot = Plot(GetSystem()->observation(item.toStdString())->Realizations());
#else
            QPlotWindow* plot = Plot(GetSystem()->observation(item.toStdString())->Realizations());
#endif
            plot->SetYAxisTitle(QString::fromStdString(GetSystem()->observation(item.toStdString())->GetName()));
            plot->SetLegend(false);
            plot->SetXAxisTitle("t");
        }
        else if (act->text()=="95 percent bracket")
        {
#ifndef QCharts
            Plotter* plot = Plot(GetSystem()->observation(item.toStdString())->Percentile95());
#else
            QPlotWindow* plot = Plot(GetSystem()->observation(item.toStdString())->Percentile95());
#endif

            qDebug()<<"Aadding axis titles";
            plot->SetYAxisTitle(QString::fromStdString(GetSystem()->observation(item.toStdString())->GetName()));
            plot->SetXAxisTitle("t");
        }
    }
    else
    {
        if (GetSystem()->GetOutputs()[item.toStdString()].size()==0)
        {
            QMessageBox::question(this, "Time Series is empty!", "The result for this quantity is empty!", QMessageBox::Ok);
            return;
        }

        // Parse the output name: ObjectName_PropertyName
        Quan* quan = nullptr;

        QStringList parts = item.split('_');

        // Try each possible split point (at least 1 part for object, 1 for property)
        for (int i = 1; i < parts.size(); i++)
        {
            QString objectName = parts.mid(0, i).join('_');
            QString quanName = parts.mid(i).join('_');

            qDebug() << "Trying split:";
            qDebug() << "  Object:" << objectName;
            qDebug() << "  Quantity:" << quanName;

            // Check if this object exists AND has this quantity
            if (GetSystem()->object(objectName.toStdString()))
            {
                if (GetSystem()->object(objectName.toStdString())->HasQuantity(quanName.toStdString()))
                {
                    quan = GetSystem()->object(objectName.toStdString())->Variable(quanName.toStdString());
                    qDebug() << "  ✓ Found valid object and property!";
                    break;  // Stop at first valid match
                }
            }
        }

        if (!quan)
        {
            qDebug() << "  No valid object/property combination found";
        }

#ifndef QCharts
        Plotter* plot = Plot(GetSystem()->GetOutputs()[item.toStdString()]);
#else
        QPlotWindow* plot;
        if (quan)
            plot = Plot(GetSystem()->GetOutputs()[item.toStdString()], quan);
        else
            plot = Plot(GetSystem()->GetOutputs()[item.toStdString()]);
#endif
        plot->SetYAxisTitle(act->text());
    }

}

void MainWindow::onDeleteItem()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QString item = act->data().toString();
    undoData.SetActiveSystem(&system);
    dView->deleteselectednode(item);
    
}

void MainWindow::on_Undo()
{
    if (undoData.CanUndo())
        system = *undoData.Undo();
    PopulatePropertyTable(nullptr);
    RecreateGraphicItemsFromSystem(false);
    RefreshTreeView();
}

void MainWindow::on_Redo()
{
    if (undoData.CanRedo())
        system = *undoData.Redo();
    PopulatePropertyTable(nullptr);
    RecreateGraphicItemsFromSystem(false);
    RefreshTreeView();
}


void MainWindow::onTreeSelectionChanged(QTreeWidgetItem *current)
{
    //qDebug()<<current->data(0,Qt::UserRole);
    //qDebug()<< current->text(0);

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
    {   propmodel = new PropModel(quanset,this,this);
        SetPropertyWindowTitle(QString::fromStdString(quanset->Parent()->GetName())+":"+QString::fromStdString(quanset->Description()));
        QString iconfilename;
        if (quanset->IconFileName()!="")
        {
            if (QString::fromStdString(quanset->IconFileName()).contains("/"))
            {
                if (!QFile::exists(QString::fromStdString(quanset->IconFileName())))
                    LogError("Icon file '" + QString::fromStdString(quanset->IconFileName()) + "' was not found!");
                else
                    iconfilename = QString::fromStdString(quanset->IconFileName());
            }
            else
            {
                if (!QFile::exists(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + quanset->IconFileName())))
                    LogError("Icon file '" + QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + quanset->IconFileName() + "' was not found!"));
                else
                    iconfilename = QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + quanset->IconFileName());
            }

            SetPropertyWindowIcon(iconfilename);
        }
        else
            SetPropertyWindowIcon("");
    }
    else
        propmodel = nullptr;
    PropertiesWidget->tableView()->setModel(propmodel);
}

void MainWindow::SetPropertyWindowIcon(const QString &iconfilename)
{
    PropertiesWidget->setIcon(iconfilename);
}

void MainWindow::Populate_General_ToolBar()
{
    //Normal Mode
    QAction* actionnormalmode = new QAction(this);
    actionnormalmode->setObjectName("Normal Mode");
    QIcon iconnormalmodel;

    iconnormalmodel.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/arrow_cursor.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionnormalmode->setIcon(iconnormalmodel);
    ui->GeneraltoolBar->addAction(actionnormalmode);
    connect(actionnormalmode, SIGNAL(triggered()), this, SLOT(onnormalmode()));
    // Save //
    QAction* actionsave = new QAction(this);
    actionsave->setObjectName("Save");
    QIcon iconsave;

    iconsave.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/Save.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionsave->setIcon(iconsave);
    ui->GeneraltoolBar->addAction(actionsave);
    actionsave->setText("Save");
    actionsave->setToolTip("Save");
    connect(actionsave, SIGNAL(triggered()), this, SLOT(onsave()));
    // Open //
    QAction* actionopen = new QAction(this);
    actionopen->setObjectName("Open");
    QIcon iconopen;

    iconopen.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/open.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionopen->setIcon(iconopen);
    ui->GeneraltoolBar->addAction(actionopen);
    actionopen->setText("Open");
    actionopen->setToolTip("Open");
    connect(actionopen, SIGNAL(triggered()), this, SLOT(onopen()));
    // ZoomAll //
    QAction* actionzoomall = new QAction(this);
    actionzoomall->setObjectName("Zoom All");
    actionopen->setToolTip("Zoom All");
    QIcon iconzoomall;

    iconzoomall.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/Full_screen_view.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionzoomall->setIcon(iconzoomall);
    ui->GeneraltoolBar->addAction(actionzoomall);
    actionzoomall->setText("Zoom Extends");
    actionzoomall->setToolTip("Zoom All");
    actionzoomall->setObjectName("Zoom All");
    connect(actionzoomall, SIGNAL(triggered()), this, SLOT(onzoomall()));
    // ZoomIn //
    QAction* actionzoomin = new QAction(this);
    actionzoomin->setObjectName("Zoom_In");
    QIcon iconzoomin;

    iconzoomin.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/zoom_in.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionzoomin->setIcon(iconzoomin);
    ui->GeneraltoolBar->addAction(actionzoomin);
    actionzoomin->setText("Zoom In");
    actionzoomin->setToolTip("Zoom In");
    connect(actionzoomin, SIGNAL(triggered()), this, SLOT(onzoomin()));
    // ZoomOut //
    QAction* actionzoomout = new QAction(this);
    actionzoomall->setObjectName("Zoom_Out");
    QIcon iconzoomout;

    iconzoomout.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/zoom_out.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionzoomout->setIcon(iconzoomout);
    ui->GeneraltoolBar->addAction(actionzoomout);
    actionzoomout->setText("Zoom Out");
    actionzoomout->setToolTip("Zoom Out");
    connect(actionzoomout, SIGNAL(triggered()), this, SLOT(onzoomout()));
    actionzoomwindow = new QAction(this);
    actionzoomwindow->setObjectName("Zoom Window");
    actionzoomwindow->setToolTip("Zoom Window");
    QIcon iconzoomwindow;

    iconzoomwindow.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/WindowZoom.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionzoomwindow->setCheckable(true);
    actionzoomwindow->setIcon(iconzoomwindow);
    ui->GeneraltoolBar->addAction(actionzoomwindow);
    actionzoomwindow->setText("Zoom Window");
    connect(actionzoomwindow, SIGNAL(triggered()), this, SLOT(onzoomwindowtriggered()));
    QAction* seperator = new QAction(this);
    seperator->setSeparator(true);
    ui->GeneraltoolBar->addAction(seperator);



    // Pan //
    actionpan = new QAction(this);
    actionpan->setObjectName("Pan");
    actionpan->setToolTip("Pan");
    QIcon iconpan;

    iconpan.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/pan2.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionpan->setCheckable(true);
    actionpan->setIcon(iconpan);
    ui->GeneraltoolBar->addAction(actionpan);
    actionpan->setText("Pan");
    connect(actionpan, SIGNAL(triggered()), this, SLOT(onpantriggered()));
    seperator->setSeparator(true);
    ui->GeneraltoolBar->addAction(seperator);


    // Run
    QIcon iconrun;

    iconrun.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/runmodel.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionrun = new QAction(this);
    actionrun->setIcon(iconrun);
    ui->GeneraltoolBar->addAction(actionrun);
    actionrun->setText("Run Model");
    actionrun->setToolTip("Run Model");
    actionrun->setObjectName("Run Model");
    connect(actionrun, SIGNAL(triggered()), this, SLOT(onrunmodel()));
    //optimize
    QIcon iconoptimize;

    iconoptimize.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/optimize.png"), QSize(), QIcon::Normal, QIcon::Off);

    QAction* actionoptimize = new QAction(this);
    actionoptimize->setIcon(iconoptimize);
    ui->GeneraltoolBar->addAction(actionoptimize);
    actionoptimize->setText("Optimize");
    actionoptimize->setToolTip("Optimize");
    connect(actionoptimize, SIGNAL(triggered()), this, SLOT(onoptimize()));
    //inverse run
    QIcon iconinverserun;

    iconinverserun.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/inverserun.png"), QSize(), QIcon::Normal, QIcon::Off);

    QAction* actioninverse = new QAction(this);
    actioninverse->setIcon(iconinverserun);
    ui->GeneraltoolBar->addAction(actioninverse);
    actioninverse->setText("Invese Run");
    actioninverse->setToolTip("Inverse Run");
    connect(actioninverse, SIGNAL(triggered()), this, SLOT(oninverserun()));
    //mcmc

    QIcon iconmcmc;
    iconmcmc.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/MCMC.png"), QSize(), QIcon::Normal, QIcon::Off);

    QAction* actionmcmc = new QAction(this);
    actionmcmc->setIcon(iconmcmc);
    ui->GeneraltoolBar->addAction(actionmcmc);
    actionmcmc->setText("MCMC parameter estimation");
    actionmcmc->setToolTip("MCMC parameter estimation");
    connect(actionmcmc, SIGNAL(triggered()), this, SLOT(onmcmc()));

    QIcon iconviz;
    iconviz.addFile(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/Visualize.png"), QSize(), QIcon::Normal, QIcon::Off);

    actionviz = new QAction(this);
    actionviz->setIcon(iconviz);
    ui->GeneraltoolBar->addAction(actionviz);
    actionviz->setText("Visualize");
    actionviz->setToolTip("Visualize results");
    connect(actionviz, SIGNAL(triggered()), this, SLOT(onVisualize()));
    actionviz->setEnabled(false);
}

void MainWindow::onzoomin()
{
    dView->scaleView(1.25);
}

void MainWindow::onzoomout()
{
    dView->scaleView(1 / 1.25);
}

void MainWindow::onzoomall(bool openornew)
{
    QRectF newRect = dView->MainGraphicsScene->itemsBoundingRect();
    float width = float(newRect.width());
    float height = float(newRect.height());
    float scale = float(1.1);
    newRect.setLeft(newRect.left() - float(scale - 1) / 2 * float(width));
    newRect.setTop(newRect.top() - (scale - 1) / 2 * height);
    newRect.setWidth(qreal(width * scale));
    newRect.setHeight(qreal(height * scale));
    //qDebug()<<newRect;
    if (width>dView->MainGraphicsScene->sceneRect().width() || height>dView->MainGraphicsScene->sceneRect().height() || openornew )
        dView->MainGraphicsScene->setSceneRect(newRect);
    dView->fitInView(newRect,Qt::KeepAspectRatio);
}

void MainWindow::onabout()
{
    AboutDialog* abtdlg = new AboutDialog(this);
    
	abtdlg->SetVersion(QString(openhydroqual_version));
    abtdlg->SetLastModified(QString(last_modified));
    
    abtdlg->AppendText("Plugins added:");
    for (unsigned int i=0; i<addedtemplatefilenames.size(); i++)
        abtdlg->AppendText(QString::fromStdString("    ") + QString::fromStdString(addedtemplatefilenames[i]));
    abtdlg->show();
}

void MainWindow::onpantriggered()
{
    if (actionpan->isChecked())
    {
        dView->setMode(Operation_Modes::Pan);
    }
    else
    {
        dView->setMode(Operation_Modes::NormalMode);
    }


    dView->setModeCursor();
}

void MainWindow::onzoomwindowtriggered()
{
    if (actionzoomwindow->isChecked())
    {
        dView->setMode(Operation_Modes::ZoomWindow);
    }
    else
    {
        dView->setMode(Operation_Modes::NormalMode);
    }

    dView->setModeCursor();
}

void MainWindow::onsaveas()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save As"), workingfolder,
            tr("OpenHydroQual files (*.ohq)"),nullptr,QFileDialog::DontUseNativeDialog);
    qDebug() << workingfolder;
    if (fileName!="")
    {
        //qDebug() << fileName.split('.');

        if (!fileName.contains("."))
            fileName = fileName + ".ohq";
        else if (fileName.split('.')[fileName.split('.').size()-1]!="ohq" )
        {
            fileName = fileName + ".ohq";
        }
        system.SavetoScriptFile(fileName.toStdString(),maintemplatefilename, addedtemplatefilenames);

        workingfolder = QFileInfo(fileName).canonicalPath();
        SetFileName(fileName);
        addToRecentFiles(fileName,true);
    }

}

void MainWindow::onsaveasJson()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"), workingfolder,
                                                    tr("Json files (*.json)"),nullptr,QFileDialog::DontUseNativeDialog);
    qDebug() << workingfolder;
    if (fileName!="")
    {
        //qDebug() << fileName.split('.');

        if (!fileName.contains("."))
            fileName = fileName + ".json";
        else if (fileName.split('.')[fileName.split('.').size()-1]!="json" )
        {
            fileName = fileName + ".json";
        }
        system.SavetoJson(fileName.toStdString(), addedtemplatefilenames, false);

    }
}
void MainWindow::onloadJson()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open"), workingfolder,
                                                    tr("Json files (*.json);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {
        system.clear();
        system.SetWorkingFolder(QFileInfo(fileName).canonicalPath().toStdString()+"/");
        //qDebug() <<"Working Folder: " << QString::fromStdString(system.GetWorkingFolder());
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr,
                                  "Error opening file",
                                  "Could not open JSON file:\n" + fileName);
            return;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            QMessageBox::critical(nullptr,
                                  "JSON Parse Error",
                                  "Failed to parse JSON file:\n" + fileName +
                                      "\nError: " + parseError.errorString());
            return; // return empty
        }
        system.LoadfromJson(doc);
        SetFileName("");


    }
    addedtemplatefilenames = system.addedtemplates;
    PopulatePropertyTable(nullptr);
    dView->DeleteAllItems();
    RecreateGraphicItemsFromSystem();
    RefreshTreeView();
    BuildObjectsToolBar();
    ReCreateObjectsMenu();
    LogAllSystemErrors();
    undoData = UndoData(this);
    undoData.AppendtoLast(&system);
    if (undoData.active==0) InactivateUndo();
    if (undoData.active==undoData.Systems.size()-1) InactivateRedo();
    ui->actionVisualize->setEnabled(false);
    actionviz->setEnabled(false);
}

void MainWindow::onexporttosvg()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save"), workingfolder,
            tr("svg files (*.svg)"),nullptr,QFileDialog::DontUseNativeDialog);
    QString svgfileName;
    if (fileName!="")
    {
        //qDebug() << fileName.split('.');

        if (!fileName.contains("."))
            svgfileName = fileName + ".svg";
        else if (fileName.split('.')[fileName.split('.').size()-1]!="svg" )
        {
            svgfileName = fileName + ".svg";
        }
        else
            svgfileName = fileName;
        saveSceneToSvg(svgfileName);
    }

}

void MainWindow::onsave()
{

    if (filename!="" && filename!="unnamed.ohq")
    {   system.SavetoScriptFile(filename.toStdString(),maintemplatefilename, addedtemplatefilenames);
        addToRecentFiles(filename,true);
    }
    else
        onsaveas();

}

void MainWindow::onnormalmode()
{
    foreach (QAction* action, ui->mainToolBar->actions())
        action->setChecked(false);

    dView->setMode(Operation_Modes::NormalMode);

}


void MainWindow::onopen()
{

    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), workingfolder,
            tr("OpenHydroQual files (*.ohq);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {

        ResetSystem();
        Script scr(fileName.toStdString(),&system);
        system.clear();
        system.SetWorkingFolder(QFileInfo(fileName).canonicalPath().toStdString()+"/");
        qDebug() <<"Working Folder: " << QString::fromStdString(system.GetWorkingFolder());
        system.CreateFromScript(scr,entitiesfilename);
        workingfolder = QFileInfo(fileName).canonicalPath();
        SetFileName(fileName);
        qDebug() <<"Main Working Folder: " << workingfolder;
        addToRecentFiles(fileName,true);
    }
    addedtemplatefilenames = system.addedtemplates; 
    PopulatePropertyTable(nullptr);
    dView->DeleteAllItems();
    RecreateGraphicItemsFromSystem();
    RefreshTreeView();
    BuildObjectsToolBar();
    ReCreateObjectsMenu();
    LogAllSystemErrors();
    undoData = UndoData(this);
    undoData.AppendtoLast(&system);
    if (undoData.active==0) InactivateUndo();
    if (undoData.active==undoData.Systems.size()-1) InactivateRedo();
    ui->actionVisualize->setEnabled(false);
    actionviz->setEnabled(false);

}


void MainWindow::oncomponentdescriptions()
{
    MetaModel* metamodel = system.GetMetaModel();
    MetaModelHelpDialog *componentdescriptiondialog = new MetaModelHelpDialog(metamodel, resource_directory, this);
    componentdescriptiondialog->setAttribute(Qt::WA_DeleteOnClose);
    componentdescriptiondialog->show();
}

void MainWindow::onnewproject()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "OpenHydroQual",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Cancel)
        return;

    ResetSystem();
    QString fileName = "unnamed.ohq";
    SetFileName(fileName);
    PopulatePropertyTable(nullptr);
    dView->DeleteAllItems();
    RecreateGraphicItemsFromSystem();
    RefreshTreeView();
    BuildObjectsToolBar();
    ReCreateObjectsMenu();
    LogAllSystemErrors();
    undoData = UndoData(this);
    undoData.AppendtoLast(&system);
    if (undoData.active==0) InactivateUndo();
    if (undoData.active==undoData.Systems.size()-1) InactivateRedo();
    ui->actionVisualize->setEnabled(false);
    actionviz->setEnabled(false);
}

bool MainWindow::LoadModel(QString fileName)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "OpenHydroQual",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Cancel)
        return false;

    bool success = true;
    if (fileName!="")
    {
        Script scr(fileName.toStdString(),&system);
        ResetSystem();
        workingfolder = QFileInfo(fileName).canonicalPath();
        qDebug()<<"Main window path: " << workingfolder;
        system.SetWorkingFolder(workingfolder.toStdString()+"/");
        qDebug()<<"System path: " << QString::fromStdString(system.GetWorkingFolder());
        system.CreateFromScript(scr,entitiesfilename);

        SetFileName(fileName);
        addToRecentFiles(fileName,true);
        addedtemplatefilenames = system.addedtemplates;
        PopulatePropertyTable(nullptr);
        RecreateGraphicItemsFromSystem();
        RefreshTreeView();
        BuildObjectsToolBar();
        ReCreateObjectsMenu();
        LogAllSystemErrors();
        return success;
    }
    else
        return  false;


}


void MainWindow::LogAllSystemErrors(ErrorHandler *errs)
{
    if (errs==nullptr)
    {   for (int i=0; i<system.GetErrorHandler()->Count(); i++)
            LogError(QString::fromStdString(system.GetErrorHandler()->at(i)->description));
        system.GetErrorHandler()->clear();
    }
    else
        for (int i=0; i<errs->Count(); i++)
             LogError(QString::fromStdString(errs->at(i)->description));

}

void MainWindow::RecreateGraphicItemsFromSystem(bool zoom_all)
{
    dView->DeleteAllItems();
    for (unsigned int i=0; i<system.BlockCount(); i++)
    {
        Node *node = new Node(dView,&system);
        system.block(i)->AssignRandomPrimaryKey();
        node->SetObject(system.block(i));
        node->setX(system.block(i)->GetVal("x"));
        node->setY(system.block(i)->GetVal("y"));
        node->setWidth(system.block(i)->GetVal("_width"));
        node->setHeight(system.block(i)->GetVal("_height"));

    }
    for (unsigned int i=0; i<system.LinksCount(); i++)
    {
        Node *s_node = dView->node(QString::fromStdString(system.block(system.link(i)->s_Block_No())->GetName()));
        Node *e_node = dView->node(QString::fromStdString(system.block(system.link(i)->e_Block_No())->GetName()));
        if (s_node && e_node)
        {
            Edge *edge = new Edge(s_node,e_node,dView );
            system.link(i)->AssignRandomPrimaryKey();
            edge->SetObject(system.link(i));
        }
    }
    dView->repaint();
    if (zoom_all)
        onzoomall(true);
}

void MainWindow::SetPropertyWindowTitle(const QString &title)
{
    int width = ui->dockWidget_3->size().width();
    PropertiesWidget->SetTitleText(title);


}

void MainWindow::onrunmodel()
{
    onsave();
    actionrun->setEnabled(false);
    ErrorHandler errs = system.VerifyAllQuantities();
    if (errs.Count()!=0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "There are errors in the values assigned to some of the variables. Check the log window for more details.", QMessageBox::Ok);
        actionrun->setEnabled(true);
        return;
    }
    System copiedsystem(system);
    copiedsystem.SetSystemSettings();
    qDebug()<<"Working folder: " << workingfolder;
    if (copiedsystem.GetSolverSettings().write_solution_details)
        copiedsystem.SetSolutionLogger(workingfolder.toStdString() + "/solution_details.txt");
    rtw = new ProgressWindow(this);
	rtw->setWindowTitle("Running Model");
    rtw->SetStatus("Running Model");
    rtw->SetPrimaryChartXAxisTitle("Time");
    rtw->SetPrimaryChartYAxisTitle("Time-step size");
	rtw->SetPrimaryChartTitle("Time-step size vs Time");
    rtw->show();
    copiedsystem.SetProgressWindow(rtw);
    copiedsystem.WriteOutPuts(); 
    copiedsystem.Solve(true);
    rtw->AppendLog("Saving outputs in '" + workingfolder + "'");
    qDebug()<<"Working folder" << workingfolder;
    QCoreApplication::processEvents();
    if (copiedsystem.OutputFileName() != "")
    {
        rtw->AppendLog(std::string("Writing all outputs ... "));
        if (QString::fromStdString(copiedsystem.OutputFileName()).contains("/") || QString::fromStdString(copiedsystem.OutputFileName()).contains("\\"))
            if (copiedsystem.WriteIntermittently())
                copiedsystem.GetOutputs().appendtofile(copiedsystem.OutputFileName(),true);
            else
                copiedsystem.GetOutputs().write(copiedsystem.OutputFileName());
        else
            if (copiedsystem.WriteIntermittently())
                copiedsystem.GetOutputs().appendtofile(workingfolder.toStdString() + "/" + copiedsystem.OutputFileName(),true);
            else
                copiedsystem.GetOutputs().write(workingfolder.toStdString() + "/" + copiedsystem.OutputFileName());
    }
    if (copiedsystem.ObservedOutputFileName() != "")
    {
        rtw->AppendLog(std::string("Writing observations ... "));
        if (QString::fromStdString(copiedsystem.ObservedOutputFileName()).contains("/") || QString::fromStdString(copiedsystem.ObservedOutputFileName()).contains("\\"))
            if (copiedsystem.WriteIntermittently())
                copiedsystem.GetObservedOutputs().appendtofile(copiedsystem.ObservedOutputFileName(),true);
            else
                copiedsystem.GetObservedOutputs().write(copiedsystem.ObservedOutputFileName());

        else
            if (copiedsystem.WriteIntermittently())
                copiedsystem.GetObservedOutputs().appendtofile(workingfolder.toStdString() + "/" + copiedsystem.ObservedOutputFileName(),true);
            else
                copiedsystem.GetObservedOutputs().write(workingfolder.toStdString() + "/" + copiedsystem.ObservedOutputFileName());
    }

    copiedsystem.ObjectiveFunctionSet()->GetTimeSeriesSet().write(workingfolder.toStdString() + "/Objective_Function_TimeSeries.txt");
    if (copiedsystem.WriteIntermittently())
    {
        rtw->AppendLog(std::string("Writing objective functions ... "));
        if (copiedsystem.OutputFileName() != "")
        {   if (QString::fromStdString(copiedsystem.OutputFileName()).contains("/") || QString::fromStdString(copiedsystem.OutputFileName()).contains("\\"))
                copiedsystem.GetOutputs().read(copiedsystem.OutputFileName(),true);
            else
                copiedsystem.GetOutputs().read(workingfolder.toStdString() + "/" + copiedsystem.OutputFileName(),true);
        }

        if (QString::fromStdString(copiedsystem.ObservedOutputFileName()).contains("/") || QString::fromStdString(copiedsystem.ObservedOutputFileName()).contains("\\"))
            copiedsystem.GetObservedOutputs().read(copiedsystem.ObservedOutputFileName(),true);
        else
            copiedsystem.GetObservedOutputs().read(workingfolder.toStdString() + "/" + copiedsystem.ObservedOutputFileName(),true);
    }
    copiedsystem.errorhandler.Write(workingfolder.toStdString() + "/errors.txt");
    if (copiedsystem.GetSolutionLogger())
        copiedsystem.GetSolutionLogger()->Close();
    rtw->AppendLog("Saving model json file in '" + workingfolder + "/state.json'" );
    qDebug()<<"Start time was set to "<<copiedsystem.GetTime();
    copiedsystem.SetVal("tstart",copiedsystem.GetTime());
    copiedsystem.SetProp("tstart",copiedsystem.GetTime());
    copiedsystem.SetSystemSettingsObjectProperties("simulation_start_time",QString::number(copiedsystem.GetTime()).toStdString());
    copiedsystem.SavetoJson(workingfolder.toStdString() + "/state.json", addedtemplatefilenames,false, false);
    system.TransferResultsFrom(&copiedsystem);
    system.SetOutputItems();
    CVector FitMeasures(3*copiedsystem.ObservationsCount());
    copiedsystem.ObjectiveFunctionSet()->Calculate();
    CVector ObjectiveFunctionValues = copiedsystem.ObjectiveFunctionSet()->Objective_Values();
    TimeSeriesSet<double> mapped_modeled_results;
    for (unsigned int i=0; i<copiedsystem.ObservationsCount();  i++)
    {
        copiedsystem.observation(i)->CalcMisfit();
        for (unsigned int j=0; j<3; j++)
            if (copiedsystem.observation(i)->fit_measures.size()==3)
                FitMeasures[i*3+j] = copiedsystem.observation(i)->fit_measures[j];
        if (copiedsystem.observation(i)->GetModeledTimeSeries()!=nullptr)
            if (copiedsystem.observation(i)->Variable("observed_data")->GetTimeSeries()!=nullptr)
                mapped_modeled_results.append(copiedsystem.observation(i)->GetModeledTimeSeries()->interpol(copiedsystem.observation(i)->Variable("observed_data")->GetTimeSeries()),copiedsystem.observation(i)->GetName());

    }

    ObjectiveFunctionValues.writetofile(workingfolder.toStdString() + "/" + "objective_function_values.txt");
    FitMeasures.writetofile(workingfolder.toStdString() + "/" + "fit_measures.txt");
    mapped_modeled_results.write(workingfolder.toStdString() + "/" + "mapped_modeled_results.txt");
    actionrun->setEnabled(true);
    rtw->AppendLog(std::string("All tasks finished!"));
    ui->actionVisualize->setEnabled(true);
    actionviz->setEnabled(true);
    rtw->SetStatus("Finished!");
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "OpenHydroQual",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        system.stop_triggered = true;
        if (rtw) rtw->close();
        if (LogWindow) LogWindow->close();
        event->accept();
    }
    exit; 
}

void MainWindow::onoptimize()
{
    ErrorHandler errs = system.VerifyAllQuantities();
    if (system.ParametersCount()==0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "No parameters have been defined!", QMessageBox::Ok);
        return;
    }
    if (errs.Count()!=0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "There are errors in the values assigned to some of the variables. Check the log window for more details.", QMessageBox::Ok);
        return;
    }
    system.SetSystemSettings();
    optimizer = new CGA<System>(&system);
    optimizer->SetParameters(system.object("Optimizer"));
    optimizer->filenames.pathname = workingfolder.toStdString() + "/";
    system.SetAllParents();
    rtw = new ProgressWindow(this);
    rtw->SetSecondaryProgressVisible(true);
    rtw->SetPrimaryChartXAxisTitle("Generation");
    rtw->SetPrimaryChartYAxisTitle("Objective Function Value");
    rtw->SetPrimaryChartTitle("OFV vs Generation");
    rtw->SetStatus("Optimization");
    rtw->SetPrimaryChartXRange(0, optimizer->GA_params.nGen);
    rtw->SetSecondaryProgressVisible(true);
    rtw->show();
    rtw->AppendLog(std::string("Optimization Started ..."));
    rtw->SetPrimaryChartXRange(0,optimizer->GA_params.nGen);
    system.SetProgressWindow(nullptr);
    optimizer->SetProgressWindow(rtw);
    system.SetParameterEstimationMode(parameter_estimation_options::optimize);
    optimizer->optimize();
    optimizer->Model_out.GetOutputs().write(workingfolder.toStdString() + "/outputs.txt");
    optimizer->Model_out.GetObservedOutputs().write(workingfolder.toStdString() + "/observedoutputs.txt");
    optimizer->Model_out.errorhandler.Write(workingfolder.toStdString() + "/errors.txt");

    system.TransferResultsFrom(&optimizer->Model_out);
    system.Parameters() = optimizer->Model_out.Parameters();
    system.SetOutputItems();
    system.SetParameterEstimationMode();
    rtw->AppendLog(std::string("Optimization Finished!"));
    rtw->SetStatus("Finished!");
}

void MainWindow::oninverserun()
{
    ErrorHandler errs = system.VerifyAllQuantities();
    if (system.ParametersCount()==0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "No parameters have been defined!", QMessageBox::Ok);
        return;
    }
    if (errs.Count()!=0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "There are errors in the values assigned to some of the variables. Check the log window for more details.", QMessageBox::Ok);
        return;
    }
    system.SetSystemSettings();
    optimizer = new CGA<System>(&system);
    optimizer->SetParameters(system.object("Optimizer"));
    optimizer->filenames.pathname = workingfolder.toStdString() + "/";
    system.SetAllParents();
    rtw = new ProgressWindow(this);
	rtw->SetPrimaryChartXAxisTitle("Generation");
    rtw->SetPrimaryChartYAxisTitle("-Log Likelihood");
    rtw->SetPrimaryChartTitle("-LL vs Generation");
	rtw->SetStatus("Parameter Estimation");
    rtw->SetPrimaryChartXRange(0, optimizer->GA_params.nGen);
	rtw->SetSecondaryProgressVisible(true);
    rtw->show();
    rtw->AppendLog(std::string("Parameter Estimation Started ..."));
    rtw->SetPrimaryChartXRange(0,optimizer->GA_params.nGen);
    system.SetProgressWindow(nullptr);
    system.SetParameterEstimationMode(parameter_estimation_options::inverse_model);
    optimizer->SetProgressWindow(rtw);
    optimizer->optimize();
    system.TransferResultsFrom(&optimizer->Model_out);
    system.Parameters() = optimizer->Model_out.Parameters();
    system.SetParameterEstimationMode();
    system.SetOutputItems();
    rtw->AppendLog(std::string("Parameter Estimation Finished!"));
    rtw->SetStatus("Finished!");
}

void MainWindow::onmcmc()
{
    ErrorHandler errs = system.VerifyAllQuantities();
    if (system.ParametersCount()==0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "No parameters have been defined!", QMessageBox::Ok);
        return;
    }
    if (errs.Count()!=0)
    {
        LogAllSystemErrors(&errs);
        QMessageBox::question(this, "Errors!", "There are errors in the values assigned to some of the variables. Check the log window for more details.", QMessageBox::Ok);
        return;
    }
    system.SetSystemSettings();
    mcmc = new CMCMC<System>(&system);
    mcmc->FileInformation.outputpath = workingfolder.toStdString() + "/";
    mcmc->SetParameters(system.object("MCMC"));
    system.SetAllParents();
    rtw = new ProgressWindow(this);
    rtw->SetSecondaryChartVisible(true);
    rtw->show();
	rtw->SetStatus("Bayesian Parameter Estimation");
    rtw->AppendLog(std::string("Parameter Estimation Started ..."));
    rtw->SetPrimaryChartXRange(0,mcmc->MCMC_Settings.total_number_of_samples);
	rtw->SetPrimaryChartXAxisTitle("Sample Number");
	rtw->SetPrimaryChartYAxisTitle("Acceptance Rate");
	rtw->SetPrimaryChartTitle("Acceptance Rate vs Sample Number");
    rtw->SetPrimaryChartKeepMinYAtZero(true);  // Keep minimum Y at zero
    rtw->SetPrimaryChartAutoScale(true);        // Enable auto-scaling for max
    rtw->SetPrimaryChartYRange(0, 1);
    rtw->SetSecondaryChartXRange(0,mcmc->MCMC_Settings.total_number_of_samples);
    rtw->SetSecondaryChartYRange(0, 1);
    rtw->SetSecondaryChartAutoScale(true);
	rtw->SetSecondaryChartXAxisTitle("Sample Number");
	rtw->SetSecondaryChartYAxisTitle("Perturbation coefficient");
	rtw->SetSecondaryChartTitle("Perturbation Coefficient vs Sample Number");
    rtw->SetSecondaryChartKeepMinYAtZero(true);  // Keep minimum Y at zero
    rtw->SetSecondaryChartAutoScale(true);        // Enable auto-scaling for max
    system.SetProgressWindow(nullptr);
    system.SetParameterEstimationMode(parameter_estimation_options::inverse_model);
    mcmc->SetProgressWindow(rtw);
    mcmc->Perform();

    rtw->AppendLog(std::string("Parameter Estimation Finished!"));
    rtw->SetStatus("Finished");
}

#ifndef QCharts
Plotter* MainWindow::Plot(TimeSeries<timeseriesprecision>& plotitem, bool allowtime)
{
    Plotter* plotter = new Plotter(this);
    plotter->PlotData(plotitem,allowtime);
    plotter->show();
    return plotter;
}

Plotter* MainWindow::Plot(TimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime)
{
    Plotter* plotter = new Plotter(this);
    qDebug()<<"plotting ...";
    plotter->PlotData(plotitem, allowtime);
    qDebug()<<"Showing plot";
    plotter->show();
    return plotter;
}

Plotter* MainWindow::Plot(TimeSeries<timeseriesprecision>& plotmodeled, TimeSeries<timeseriesprecision>& plotobserved)
{
    Plotter* plotter = new Plotter(this);
    if (plotmodeled.n>0)
        plotter->PlotData(plotmodeled,false, "line");
    if (plotobserved.n>0)
    plotter->AddData(plotobserved,false, "dots");
    plotter->show();
    return plotter;
}
#else
QPlotWindow* MainWindow::Plot(TimeSeries<timeseriesprecision>& plotitem, bool allowtime)
{
    QPlotWindow* plotter = new QPlotWindow(this);
    plotter->PlotData(plotitem,allowtime);
    plotter->show();
    return plotter;
}

QPlotWindow* MainWindow::Plot(TimeSeries<timeseriesprecision>& plotitem, Quan* quan, bool allowtime)
{
    QPlotWindow* plotter = new QPlotWindow(this);
    plotter->SetQuantity(quan);  // Set the Quan before plotting
    plotter->PlotData(plotitem, allowtime);
    plotter->show();
    return plotter;
}

QPlotWindow* MainWindow::Plot(TimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime)
{
    QPlotWindow* plotter = new QPlotWindow(this);
    qDebug()<<"plotting ...";
    plotter->PlotData(plotitem, allowtime);
    qDebug()<<"Showing plot";
    plotter->show();
    return plotter;
}

QPlotWindow* MainWindow::Plot(TimeSeries<timeseriesprecision>& plotmodeled, TimeSeries<timeseriesprecision>& plotobserved)
{
    QPlotWindow* plotter = new QPlotWindow(this);
    if (plotmodeled.size()>0)
        plotter->PlotData(plotmodeled,false, "line");
    if (plotobserved.size()>0)
    plotter->AddData(plotobserved,false, "dots");
    plotter->show();
    return plotter;
}
#endif


void MainWindow::onAddItemThroughTreeViewRightClick()
{
    QObject* obj = sender();
    if (obj->property("group")=="Parameters")
        onaddparameter();
    if (obj->property("group")=="Objective Functions")
        onaddobjectivefunction();
    if (obj->property("group")=="Reactions")
        onaddreaction();
    if (obj->property("group")=="Reaction Parameters")
        onaddreactionparameter();
    if (obj->property("group")=="Constituents")
        onaddconstituent();
    if (obj->property("group")=="Observations")
        onaddobservation();


    //counts[obj->objectName()] = counts[obj->objectName()] + 1;
        //qDebug() << "entity added! " << obj->objectName();
        //Entity* item = new Entity(obj->objectName(), obj->objectName() + QString::number(counts[obj->objectName()]), diagramview, QString::fromStdString(system.GetMetaModel()->GetItem(obj->objectName().toStdString())->CategoryType()));


}

void MainWindow::loadnewtemplate()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), QString::fromStdString(RESOURCE_DIRECTORY),
            tr("Script files (*.json);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {
        system.clear();
        system.GetMetaModel()->Clear();
        system.GetQuanTemplate(fileName.toStdString());
        system.ReadSystemSettingsTemplate(entitiesfilename);
        ui->mainToolBar->clear();
        BuildObjectsToolBar();
        ReCreateObjectsMenu();
        RefreshTreeView();
        maintemplatefilename = fileName.toStdString();
    }

}

void MainWindow::addplugin()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Script files (*.json);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {
        if (system.AppendQuanTemplate(fileName.toStdString()))
        {   ui->mainToolBar->clear();
            BuildObjectsToolBar();
            ReCreateObjectsMenu();
            RefreshTreeView();
            addedtemplatefilenames.push_back(fileName.toStdString());
        }
        else
        {
            LogError("Error in file '" + filename + "' :" +  QString::fromStdString(system.GetMetaModel()->GetLastError()));
        }
    }

}

void MainWindow::addplugin(const QString &fileName)
{

    if (fileName!="")
    {
        if (system.AppendQuanTemplate(fileName.toStdString()))
        {   ui->mainToolBar->clear();
            system.AddConstituentRelatePropertiestoMetalModel();
            BuildObjectsToolBar();
            ReCreateObjectsMenu();
            RefreshTreeView();
            addedtemplatefilenames.push_back(fileName.toStdString());
        }
        else
        {
            LogError("Error in file '" + filename + "' :" +  QString::fromStdString(system.GetMetaModel()->GetLastError()));
        }
    }

}

void MainWindow::adddefaultpluging()
{
    Wizard_select_dialog *Wizard_Select_Form = new Wizard_select_dialog(this);
    Wizard_Select_Form->show();
}

void MainWindow::optionsdialog()
{
    OptionsDialog *Options_Dialog = new OptionsDialog(this);
    Options_Dialog->show();

}

void MainWindow::loadresults()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Output files (*.txt);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    try {
        TimeSeriesSet<double> outputs(fileName.toStdString(), true);
        TimeSeriesSet<double> past_output = system.GetOutputs();
        system.GetOutputs() = outputs;

        if (!system.SetLoadedOutputItems())
        {
            system.GetOutputs() = past_output;
            QApplication::restoreOverrideCursor();  // Restore before dialog
            QMessageBox::critical(this, "Output file not correct!",
                                  "The file does not contains the results of the model",
                                  QMessageBox::Ok);
        }
        else
        {
            QApplication::restoreOverrideCursor();  // Restore before dialog
            QMessageBox::information(this, "Output file loaded!",
                                     "Output file loaded successfully!",
                                     QMessageBox::Ok);
            ui->actionVisualize->setEnabled(true);
            actionviz->setEnabled(true);
        }
    }
    catch (...) {
        QApplication::restoreOverrideCursor();  // Restore on exception
        throw;  // Re-throw
    }

}


bool MainWindow::Log(const QString &s)
{
    LogWindow->AppendText(s);
    return true;
}

bool MainWindow::LogError(const QString &s)
{
    LogWindow->AppendError(s);
    ui->actionLog_Window->setChecked(true);
    LogWindow->show();
    return true;
}

bool MainWindow::LogAddDelete(const QString &s)
{
    LogWindow->AppendBlue(s);
    return true;
}

void MainWindow::SetFileName(const QString &_filename)
{
    filename = _filename;
    if (filename.split('/').size()>0)
        setWindowTitle("OpenHydroQual: " + filename.split('/')[filename.split('/').size()-1]);

}

void MainWindow::readRecentFilesList()
{
//	//qDebug() << localAppFolderAddress();
//	QString add = localAppFolderAddress();
    ifstream file(localAppFolderAddress().toStdString()+RECENT);
    int count = 0;
    if (file.good())
    {
        string line;
        while (!file.eof())
        {
            getline(file, line);
            count++;
        }
        file.close();
    }

    file.open(localAppFolderAddress().toStdString() + RECENT);
    int n = 0;
    if (file.good())
    {
        string line;
        while (!file.eof())
        {
            getline(file, line);
            n++;
            QString fileName = QString::fromStdString(line);
            //qDebug() << fileName; QString::fromStdString(line);
            if (n>count-max_num_recent_files)
                addToRecentFiles(fileName, false);

        }
        file.close();

    }
}
void MainWindow::addToRecentFiles(QString fileName, bool addToFile)
{
    bool rewriteFile = false;
    if (recentFiles.contains(fileName) && fileName.trimmed() != "")
        if (recentFiles.indexOf(fileName) != recentFiles.count()-1)
        {
            ui->menuRecent->removeAction(ui->menuRecent->actions()[recentFiles.size() - 1 - recentFiles.indexOf(fileName)]);
            recentFiles.removeOne(fileName);
            addToFile = false;
            rewriteFile = true;
        }

    if (!recentFiles.contains(fileName) && fileName.trimmed() != "")
    {
        recentFiles.append(fileName);
        //		QAction * a = ui->menuRecent->addAction(fileName);// , this, SLOT(recentItem()));
        QAction * fileNameAction = new QAction(fileName, nullptr);
        if (ui->menuRecent->actions().size())
            ui->menuRecent->insertAction(ui->menuRecent->actions()[0], fileNameAction);
        else
            ui->menuRecent->addAction(fileNameAction);
        QObject::connect(fileNameAction, SIGNAL(triggered()), this, SLOT(on_actionRecent_triggered()));

        if (addToFile)
        {
            CreateFileIfDoesNotExist(localAppFolderAddress() + RECENT);
            ofstream file(localAppFolderAddress().toStdString() + RECENT, fstream::app);
            if (file.good())
                file << fileName.toStdString() << std::endl;
            file.close();
        }
        if (rewriteFile)
            writeRecentFilesList();
    }
}

void MainWindow::writeRecentFilesList()
{
    ofstream file(localAppFolderAddress().toStdString() + RECENT);
    if (file.good())
    {
        foreach (QString fileName , recentFiles)
            file << fileName.toStdString() << std::endl;
    }
    file.close();
}

QString localAppFolderAddress() {
    #ifdef _WIN32
    TCHAR szPath[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPath(NULL,
        CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
        NULL,
        0,
        szPath)))
    {
        return QString("%1/").arg(QString::fromStdWString(szPath));
        //PathAppend(szPath, TEXT("New Doc.txt"));
        //HANDLE hFile = CreateFile(szPath, ...);
    }
#else
    return QString();
#endif
}

void MainWindow::on_actionRecent_triggered()
{
    QAction* a = static_cast<QAction*> (QObject::sender());
    QString fileName = a->text();
    if (LoadModel(fileName))
    {
        addToRecentFiles(fileName, false);
    }
    undoData = UndoData(this);
    undoData.AppendtoLast(&system);
    if (undoData.active==0) InactivateUndo();
    if (undoData.active==undoData.Systems.size()-1) InactivateRedo();

}

void MainWindow::removeFromRecentList(QAction* selectedFileAction)
{
    recentFiles.removeAll(selectedFileAction->text());
    ui->menuRecent->removeAction(selectedFileAction);
    writeRecentFilesList();
}

void MainWindow::saveSceneToSvg(const QString &filename) {

    QSize sceneSize = GetDiagramView()->MainGraphicsScene->sceneRect().size().toSize();

    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(sceneSize);
    generator.setViewBox(QRect(0, 0, sceneSize.width(), sceneSize.height()));
    generator.setDescription(QObject::tr("My canvas exported to Svg"));
    generator.setTitle(filename);
    QPainter painter;
    painter.begin(&generator);
    GetDiagramView()->MainGraphicsScene->render(&painter);
    painter.end();


}

bool MainWindow::CreateFileIfDoesNotExist(QString fileName)
{
    QFileInfo check_file(fileName);
    bool success = false; 
    if (!check_file.exists())
    {
        QFile filetobecreated(fileName);
        success = filetobecreated.open(QIODevice::WriteOnly);
        filetobecreated.close();
    }
    return success; 
}

void MainWindow::AddStatetoUndoData()
{
    undoData.AppendtoLast(&system);
}


void MainWindow::SetActiveUndo()
{
    undoData.SetActiveSystem(&system);
}

void MainWindow::onCreate2dArray()
{
    GridGenerator *gridgenerator = new GridGenerator(this);
    gridgenerator->exec();
    delete gridgenerator;

}

// Add this to your MainWindow class header (.h file):
// 
// In the public slots section:
//     void onimport();
// 
// In the private section:
//     struct NameConflict {
//         QString objectType;
//         QString objectName;
//         QString suggestedNewName;
//     };
//     QList<NameConflict> checkForNameConflicts(System* importSystem);
//     bool resolveNameConflicts(Script& importScript, const QList<NameConflict>& conflicts);

// ========================================================================
// Implementation code for MainWindow.cpp:
// ========================================================================

void MainWindow::onimport()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Model"), workingfolder,
        tr("OpenHydroQual files (*.ohq);; All files (*.*)"),
        nullptr, QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty())
        return;

    // Create a temporary system to load the import file
    System importSystem;
    Script importScript(fileName.toStdString(), &importSystem);
    importSystem.SetWorkingFolder(QFileInfo(fileName).canonicalPath().toStdString() + "/");

    // Execute the script to populate the import system
    bool scriptSuccess = importSystem.CreateFromScript(importScript, "");
    if (!scriptSuccess)
    {
        // Check if at least one object was created
        bool hasObjects = (importSystem.BlockCount() > 0 ||
            importSystem.LinksCount() > 0 ||
            importSystem.SourcesCount() > 0 ||
            importSystem.ObservationsCount() > 0 ||
            importSystem.ConstituentsCount() > 0 ||
            importSystem.ReactionsCount() > 0 ||
            importSystem.ReactionParametersCount() > 0 ||
            importSystem.ParametersCount() > 0 ||
            importSystem.ObjectiveFunctions().size() > 0);

        if (!hasObjects)
        {
            QMessageBox::critical(this, tr("Import Error"),
                tr("Failed to load the import file. No objects were created."));
            LogAllSystemErrors();
            return;
        }
        else
        {
            QMessageBox::warning(this, tr("Import Warning"),
                tr("The import file had errors, but some objects were created successfully. "
                    "Import will continue with the objects that loaded correctly."));
        }
    }

    QList<NameConflict> conflicts = checkForNameConflicts(&importSystem);

    if (!conflicts.isEmpty())
    {
        // Show conflict resolution dialog
        if (!resolveNameConflicts(importScript, conflicts))
        {
            // User cancelled the import
            return;
        }

        // Recreate the import system with renamed objects
        importSystem.clear();
        importSystem.addedtemplates.clear();

        // Re-execute the modified script
        bool scriptSuccess = importSystem.CreateFromScript(importScript, "");

        // Check if at least one object was created
        bool hasObjects = (importSystem.BlockCount() > 0 ||
            importSystem.LinksCount() > 0 ||
            importSystem.SourcesCount() > 0 ||
            importSystem.ObservationsCount() > 0 ||
            importSystem.ConstituentsCount() > 0 ||
            importSystem.ReactionsCount() > 0 ||
            importSystem.ReactionParametersCount() > 0 ||
            importSystem.ParametersCount() > 0 ||
            importSystem.ObjectiveFunctions().size() > 0);

        if (!hasObjects)
        {
            QMessageBox::critical(this, tr("Import Error"),
                tr("Failed to apply name changes. Import cancelled."));
            return;
        }
        else if (!scriptSuccess)
        {
            QMessageBox::warning(this, tr("Import Warning"),
                tr("Some errors occurred while applying changes, but objects were created. Import will continue."));
        }
    }

    bool hasPositionConflict = false;
    double maxShift = 0.0;

    for (unsigned int i = 0; i < importSystem.BlockCount(); i++)
    {
        Block* importBlock = importSystem.block(i);
        if (!importBlock || !importBlock->HasQuantity("x") || !importBlock->HasQuantity("y"))
            continue;

        double importX = importBlock->GetVal("x");
        double importY = importBlock->GetVal("y");
        double importWidth = importBlock->HasQuantity("_width") ? importBlock->GetVal("_width") : 0.0;
        double importHeight = importBlock->HasQuantity("_height") ? importBlock->GetVal("_height") : 0.0;
        double importThreshold = sqrt(importWidth * importWidth + importHeight * importHeight);

        // Check against all blocks in the host system
        for (unsigned int j = 0; j < system.BlockCount(); j++)
        {
            Block* hostBlock = system.block(j);
            if (!hostBlock || !hostBlock->HasQuantity("x") || !hostBlock->HasQuantity("y"))
                continue;

            double hostX = hostBlock->GetVal("x");
            double hostY = hostBlock->GetVal("y");
            double hostWidth = hostBlock->HasQuantity("_width") ? hostBlock->GetVal("_width") : 0.0;
            double hostHeight = hostBlock->HasQuantity("_height") ? hostBlock->GetVal("_height") : 0.0;
            double hostThreshold = sqrt(hostWidth * hostWidth + hostHeight * hostHeight);

            // Calculate distance between blocks
            double dx = importX - hostX;
            double dy = importY - hostY;
            double distance = sqrt(dx * dx + dy * dy);

            // Use the maximum threshold of the two blocks
            double threshold = std::max(importThreshold, hostThreshold);

            // If blocks are too close, we have a conflict
            if (distance < threshold)
            {
                hasPositionConflict = true;
                // Calculate how far right we need to shift to clear this block
                double requiredShift = hostX + hostWidth + importWidth - importX + 50.0; // Add 50 unit padding
                maxShift = std::max(maxShift, requiredShift);
            }
        }
    }

    // If there's a position conflict, translate the import system
    if (hasPositionConflict)
    {
        importSystem.Translate(maxShift, 0.0);
        qDebug() << "Translated import model by" << maxShift << "units to avoid position conflicts";
    }

    // Add templates from import file if they don't exist
    for (const string& templateName : importSystem.addedtemplates)
    {
        if (aquiutils::lookup(system.addedtemplates, templateName) == -1)
        {
            // Try to add the template
            if (!system.AppendQuanTemplate(templateName))
            {
                // Try with just the filename in the default template path
                if (!system.AppendQuanTemplate(system.DefaultTemplatePath() +
                    aquiutils::GetOnlyFileName(templateName)))
                {
                    QMessageBox::warning(this, tr("Template Warning"),
                        tr("Template '%1' could not be loaded. Some objects may not function correctly.")
                        .arg(QString::fromStdString(templateName)));
                }
            }
        }
    }

    // Now merge the objects from importSystem into the main system
    // Blocks
    for (unsigned int i = 0; i < importSystem.BlockCount(); i++)
    {
        Block* importBlock = importSystem.block(i);
        Block newBlock = *importBlock;  // Copy the block
        system.AddBlock(newBlock);

        // Copy all properties
        Block* addedBlock = system.block(newBlock.GetName());
        if (addedBlock)
        {
            *addedBlock = *importBlock;
        }
    }

    // Links
    for (unsigned int i = 0; i < importSystem.LinksCount(); i++)
    {
        Link* importLink = importSystem.link(i);

        // Get source and target block names
        Object* sourceBlock = importLink->GetConnectedBlock(Expression::loc::source);
        Object* targetBlock = importLink->GetConnectedBlock(Expression::loc::destination);

        if (!sourceBlock || !targetBlock)
        {
            QMessageBox::warning(this, tr("Import Warning"),
                tr("Link '%1' has invalid connections and will be skipped.")
                .arg(QString::fromStdString(importLink->GetName())));
            continue;
        }

        string fromBlock = sourceBlock->GetName();
        string toBlock = targetBlock->GetName();

        // Create a new link with proper connections in the target system
        Link newLink;
        newLink.SetName(importLink->GetName());
        newLink.SetType(importLink->GetType());

        if (system.AddLink(newLink, fromBlock, toBlock))
        {
            // Now copy the quantities/properties without copying pointers
            Link* addedLink = system.link(newLink.GetName());
            if (addedLink)
            {
                addedLink->CopyQuantitiesFrom(importLink);
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Import Warning"),
                tr("Link '%1' could not be added. Check that source and target blocks exist.")
                .arg(QString::fromStdString(newLink.GetName())));
        }
    }

    // Sources
    for (unsigned int i = 0; i < importSystem.SourcesCount(); i++)
    {
        Source* importSource = importSystem.source(i);
        Source newSource = *importSource;
        system.AddSource(newSource);

        Source* addedSource = system.source(newSource.GetName());
        if (addedSource)
        {
            *addedSource = *importSource;
        }
    }

    // Observations
    for (unsigned int i = 0; i < importSystem.ObservationsCount(); i++)
    {
        Observation* importObs = importSystem.observation(i);
        Observation newObs = *importObs;
        system.AddObservation(newObs);

        Observation* addedObs = system.observation(newObs.GetName());
        if (addedObs)
        {
            *addedObs = *importObs;
        }
    }

    // Constituents
    for (unsigned int i = 0; i < importSystem.ConstituentsCount(); i++)
    {
        Constituent* importConst = importSystem.constituent(i);
        Constituent newConst = *importConst;
        system.AddConstituent(newConst);

        Constituent* addedConst = system.constituent(newConst.GetName());
        if (addedConst)
        {
            *addedConst = *importConst;
        }
    }

    // Reactions
    for (unsigned int i = 0; i < importSystem.ReactionsCount(); i++)
    {
        Reaction* importRxn = importSystem.reaction(i);
        Reaction newRxn = *importRxn;
        system.AddReaction(newRxn);

        Reaction* addedRxn = system.reaction(newRxn.GetName());
        if (addedRxn)
        {
            *addedRxn = *importRxn;
        }
    }

    // Reaction Parameters
    for (unsigned int i = 0; i < importSystem.ReactionParametersCount(); i++)
    {
        RxnParameter* importRxnParam = importSystem.reactionparameter(i);
        RxnParameter newRxnParam = *importRxnParam;
        system.AddReactionParameter(newRxnParam);

        RxnParameter* addedRxnParam = system.reactionparameter(newRxnParam.GetName());
        if (addedRxnParam)
        {
            *addedRxnParam = *importRxnParam;
        }
    }

    // Parameters
    for (unsigned int i = 0; i < importSystem.ParametersCount(); i++)
    {
        Parameter* importParam = importSystem.GetParameter(i);
        if (importParam && system.parameter(importParam->GetName()) == nullptr)
        {
            system.AppendParameter(importParam->GetName(), *importParam);
        }
    }

    // Objective Functions
    for (unsigned int i = 0; i < importSystem.ObjectiveFunctions().size(); i++)
    {
        Objective_Function* importObjFunc = importSystem.objectivefunction(i);
        if (importObjFunc && system.objectivefunction(importObjFunc->GetName()) == nullptr)
        {
            system.AppendObjectiveFunction(importObjFunc->GetName(), *importObjFunc);
        }
    }

    // Update the system
    system.SetAllParents();
    system.SetVariableParents();

    // Update the added templates list
    addedtemplatefilenames = system.addedtemplates;

    // Refresh the UI
    PopulatePropertyTable(nullptr);
    dView->DeleteAllItems();
    RecreateGraphicItemsFromSystem();
    RefreshTreeView();
    BuildObjectsToolBar();
    ReCreateObjectsMenu();
    LogAllSystemErrors();

    // Update undo system
    undoData.AppendtoLast(&system);
    if (undoData.active == 0) InactivateUndo();
    if (undoData.active == undoData.Systems.size() - 1) InactivateRedo();

    QMessageBox::information(this, tr("Import Complete"),
        tr("Model imported successfully."));
}

QList<MainWindow::NameConflict> MainWindow::checkForNameConflicts(System* importSystem)
{
    QList<NameConflict> conflicts;

    // Check blocks
    for (unsigned int i = 0; i < importSystem->BlockCount(); i++)
    {
        Block* importBlock = importSystem->block(i);
        if (system.block(importBlock->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Block";
            conflict.objectName = QString::fromStdString(importBlock->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check links
    for (unsigned int i = 0; i < importSystem->LinksCount(); i++)
    {
        Link* importLink = importSystem->link(i);
        if (system.link(importLink->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Link";
            conflict.objectName = QString::fromStdString(importLink->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check sources
    for (unsigned int i = 0; i < importSystem->SourcesCount(); i++)
    {
        Source* importSource = importSystem->source(i);
        if (system.source(importSource->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Source";
            conflict.objectName = QString::fromStdString(importSource->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check observations
    for (unsigned int i = 0; i < importSystem->ObservationsCount(); i++)
    {
        Observation* importObs = importSystem->observation(i);
        if (system.observation(importObs->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Observation";
            conflict.objectName = QString::fromStdString(importObs->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check constituents
    for (unsigned int i = 0; i < importSystem->ConstituentsCount(); i++)
    {
        Constituent* importConst = importSystem->constituent(i);
        if (system.constituent(importConst->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Constituent";
            conflict.objectName = QString::fromStdString(importConst->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check reactions
    for (unsigned int i = 0; i < importSystem->ReactionsCount(); i++)
    {
        Reaction* importRxn = importSystem->reaction(i);
        if (system.reaction(importRxn->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Reaction";
            conflict.objectName = QString::fromStdString(importRxn->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check reaction parameters
    for (unsigned int i = 0; i < importSystem->ReactionParametersCount(); i++)
    {
        RxnParameter* importRxnParam = importSystem->reactionparameter(i);
        if (system.reactionparameter(importRxnParam->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Reaction Parameter";
            conflict.objectName = QString::fromStdString(importRxnParam->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check parameters
    for (unsigned int i = 0; i < importSystem->ParametersCount(); i++)
    {
        Parameter* importParam = importSystem->GetParameter(i);
        if (importParam && system.parameter(importParam->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Parameter";
            conflict.objectName = QString::fromStdString(importParam->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    // Check objective functions
    for (unsigned int i = 0; i < importSystem->ObjectiveFunctions().size(); i++)
    {
        Objective_Function* importObjFunc = importSystem->objectivefunction(i);
        if (importObjFunc && system.objectivefunction(importObjFunc->GetName()) != nullptr)
        {
            NameConflict conflict;
            conflict.objectType = "Objective Function";
            conflict.objectName = QString::fromStdString(importObjFunc->GetName());
            conflict.suggestedNewName = conflict.objectName + "_imported";
            conflicts.append(conflict);
        }
    }

    return conflicts;
}

bool MainWindow::resolveNameConflicts(Script& importScript, const QList<NameConflict>& conflicts)
{
    // Create a dialog to show conflicts and get new names
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Resolve Name Conflicts"));
    dialog.setMinimumWidth(600);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    QLabel* headerLabel = new QLabel(tr("The following objects have name conflicts with existing objects.\n"
        "Please provide new names for the imported objects:"));
    mainLayout->addWidget(headerLabel);

    // Create a table to show conflicts
    QTableWidget* table = new QTableWidget(conflicts.size(), 3, &dialog);
    table->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Original Name") << tr("New Name"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    QMap<QString, QString> nameMapping;  // Original name -> New name

    for (int i = 0; i < conflicts.size(); i++)
    {
        QTableWidgetItem* typeItem = new QTableWidgetItem(conflicts[i].objectType);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(i, 0, typeItem);

        QTableWidgetItem* originalItem = new QTableWidgetItem(conflicts[i].objectName);
        originalItem->setFlags(originalItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(i, 1, originalItem);

        QTableWidgetItem* newNameItem = new QTableWidgetItem(conflicts[i].suggestedNewName);
        table->setItem(i, 2, newNameItem);
    }

    mainLayout->addWidget(table);

    // Add buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
    {
        return false;  // User cancelled
    }

    // Collect the new names
    for (int i = 0; i < conflicts.size(); i++)
    {
        QString newName = table->item(i, 2)->text().trimmed();
        if (newName.isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid Name"),
                tr("New name for %1 '%2' cannot be empty.")
                .arg(conflicts[i].objectType)
                .arg(conflicts[i].objectName));
            return false;
        }
        nameMapping[conflicts[i].objectName] = newName;
    }

    // Now modify the script commands to use the new names
    for (int i = 0; i < importScript.CommandsCount(); i++)
    {
        Command* cmd = importScript[i];

        // Get reference to assignments map using the accessor method
        // NOTE: You must add GetAssignments() method to Command.h:
        //       map<string, string>& GetAssignments() { return assignments; }
        map<string, string>& assignments = cmd->GetAssignments();

        // Check if this command creates an object with a conflicting name
        if (aquiutils::tolower(cmd->Keyword()) == "create")
        {
            if (assignments.count("name") > 0)
            {
                QString originalName = QString::fromStdString(assignments["name"]);
                if (nameMapping.contains(originalName))
                {
                    // Update the name in the command
                    assignments["name"] = nameMapping[originalName].toStdString();
                }
            }
        }

        // Also need to update references in other commands (like setvalue, setasparameter, etc.)
        if (assignments.count("object") > 0)
        {
            QString objectName = QString::fromStdString(assignments["object"]);
            if (nameMapping.contains(objectName))
            {
                assignments["object"] = nameMapping[objectName].toStdString();
            }
        }

        // Update link from/to references
        if (assignments.count("from") > 0)
        {
            QString fromName = QString::fromStdString(assignments["from"]);
            if (nameMapping.contains(fromName))
            {
                assignments["from"] = nameMapping[fromName].toStdString();
            }
        }

        if (assignments.count("to") > 0)
        {
            QString toName = QString::fromStdString(assignments["to"]);
            if (nameMapping.contains(toName))
            {
                assignments["to"] = nameMapping[toName].toStdString();
            }
        }
    }

    return true;
}

void MainWindow::onVisualize()
{

    if (GetSystem()->GetOutputs().size()>0)
    {
        // Create and show the visualization dialog
        VisualizationDialog* vizDialog = new VisualizationDialog(GetSystem(), this);
        vizDialog->setAttribute(Qt::WA_DeleteOnClose);
        vizDialog->setModal(false);  // Allow interaction with main window
        vizDialog->setAttribute(Qt::WA_DeleteOnClose);  // Auto-delete when closed
        vizDialog->show();
    }
}
