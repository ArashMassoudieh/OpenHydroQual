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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"
#include <QTreeWidget>
#include "propmodel.h"
#include "diagramview.h"
#include "runtimewindow.h"
#ifndef QCharts
#include "plotter.h"
#else
#include "qplotwindow.h"
#endif
#include "GA.h"
#include "MCMC.h"
#include "logwindow.h"
#include "aboutdialog.h"
#include "undodata.h"
#include "ItemPropertiesWidget.h"

#ifdef windows_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../../resources"
#endif

#ifdef ubuntu_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../../resources"
#endif

#ifdef mac_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../resources"
#endif


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    PropModel *propModel() {return propmodel;}
    void resetPropModel() { propmodel = nullptr ;}
    System *GetSystem() {return &system;}
    bool AddLink(const QString &LinkName, const QString &sourceblock, const QString &targetblock, const QString &type, Edge* edge);
    UndoData undoData;
    void PopulatePropertyTable(QuanSet* quanset);
    void RecreateGraphicItemsFromSystem(bool zoom_all=true);
    void SetPropertyWindowTitle(const QString &title);
    void RefreshTreeView();
    QString resource_directory;
#ifndef QCharts
    Plotter* Plot(TimeSeries<timeseriesprecision>& plotitem, bool allowtime = true);
    Plotter* Plot(TimeSeries<timeseriesprecision>& plotmodeled, TimeSeries<timeseriesprecision>& plotobserved);
    Plotter* Plot(TimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime=true);
#else
    QPlotWindow* Plot(TimeSeries<timeseriesprecision>& plotitem, bool allowtime = true);
    QPlotWindow* Plot(TimeSeries<timeseriesprecision>& plotmodeled, TimeSeries<timeseriesprecision>& plotobserved);
    QPlotWindow* Plot(TimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime=true);
#endif
    string CreateNewName(string type, bool allow_paranthesis = true);
    DiagramView* GetDiagramView() { return dView; }
    bool Log(const QString &s);
    bool LogError(const QString &s);
    bool LogAddDelete(const QString &s);
    void LogAllSystemErrors(ErrorHandler *errs=nullptr);
#ifndef QCharts
    QMap<QCPGraph *, plotformat> graphsClipboard;
#else
    QMap<QString, TimeSeries<timeseriesprecision>*> graphsClipboard = QMap<QString, TimeSeries<timeseriesprecision>*>();
#endif
    void SetPan(bool panmode) {actionpan->setChecked(panmode); if (!panmode) dView->setMode(Operation_Modes::NormalMode); dView->setModeCursor(); }
    void SetZoomWindow(bool panmode) {actionzoomwindow->setChecked(panmode); if (!panmode) dView->setMode(Operation_Modes::NormalMode); dView->setModeCursor(); }
    void addplugin(const QString &fileName);
    void ResetSystem();
    void InactivateUndo(bool yes=true);
    void InactivateRedo(bool yes=true);
    void AddStatetoUndoData();
    void SetActiveUndo();
    void SetPropertyWindowIcon(const QString &iconfilename);
    QString* GetWorkingFolder()
    {
        return &workingfolder;
    }
private:
    logwindow *LogWindow;
    Ui::MainWindow *ui;
    System system;
    DiagramView* dView;
    QAction* actionpan = nullptr;
    QAction* actionzoomwindow = nullptr;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    bool ReCreateObjectsMenu();
    PropModel *propmodel = nullptr;
    void Populate_General_ToolBar();
    string maintemplatefilename;
    vector<string> addedtemplatefilenames;
    string entitiesfilename;
    RunTimeWindow *rtw = nullptr;
    void closeEvent (QCloseEvent *event) override;
    QString workingfolder = ".";
    QModelIndex addParameterIndex(const QModelIndex &index = QModelIndex());
    QModelIndex tableitemrightckicked;
    std::unique_ptr<QMenu> menu;
    CGA<System> *optimizer;
    CMCMC<System> *mcmc;
    QString timeseriestobeshown;
    QString filename="";
    void SetFileName(const QString &_filename);
    void readRecentFilesList();
    void addToRecentFiles(QString fileName, bool addToFile);
    void writeRecentFilesList();
    QStringList recentFiles;
    void removeFromRecentList(QAction* selectedFileAction);
    bool LoadModel(QString fileName);
    void saveSceneToSvg(const QString &filename);
    bool CreateFileIfDoesNotExist(QString fileName);
    QAction* actionrun = nullptr;
    ItemPropertiesWidget *PropertiesWidget = nullptr;
private slots:
    void on_check_object_browser();
    void on_check_showlogwindow();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddlink();
    void onaddsource();
    void onaddparameter();
    void onaddentity();
    void onaddobjectivefunction();
    void onaddobservation();
    void onaddconstituent();
    void onaddreaction();
    void onaddreactionparameter();
    void preparetreeviewMenu(const QPoint &pos);
    void onTreeSelectionChanged(QTreeWidgetItem *current);
    void onzoomin();
    void onzoomout();
    void onzoomall(bool open_new=false);
    void onabout();
    void onpantriggered();
    void onzoomwindowtriggered();
    void onsave();
    void onnormalmode();
    void onsaveas();
    void onsaveasJson();
    void onloadJson();
    void onexporttosvg();
    void onopen();
    void onnewproject();
    void onrunmodel();
    void onoptimize();
    void oninverserun();
    void onmcmc();
    void onAddItemThroughTreeViewRightClick();
    void tablePropShowContextMenu(const QPoint&);
    void addParameter(QAction* item);
    void clearcombobox();
    void insertnumberasdate();
    void PlotTimeSeries();
    void loadnewtemplate();
    void addplugin();
    void adddefaultpluging();
    void optionsdialog();
    void loadresults();
    void showgraph();
    void onDeleteItem();
    void on_actionRecent_triggered();
    void on_Undo();
    void on_Redo();
    void onCreate2dArray();// Is called when 2D Array action is triggered
    void oncomponentdescriptions();
};

QString localAppFolderAddress();

#endif // MAINWINDOW_H
