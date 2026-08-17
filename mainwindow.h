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
#include "ProgressWindow.h"
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
#include <QDir>
#include <QSplitter>
#include <QDockWidget>

#ifdef windows_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../../resources"
#endif

#ifdef ubuntu_version
    #define RESOURCE_DIRECTORY \
        (QDir(QString::fromStdString(qApp->applicationDirPath().toStdString()+"/../resources")).exists() ? \
        qApp->applicationDirPath().toStdString()+"/../resources" : \
        qApp->applicationDirPath().toStdString()+"/../../resources")
#endif

#ifdef mac_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../resources"
#endif


namespace Ui {
class MainWindow;
}

class Node;
class Edge;
class QLabel;

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
    /**
     * @brief The name of the node that represents an object in the diagram
     *
     * For a composite member this is the owning composite, which is what the
     * diagram actually draws; for anything else it is the object itself.
     */
    QString NodeNameFor(const std::string &objectname);
    void SetPropertyWindowTitle(const QString &title);
    void RefreshTreeView();
    QString resource_directory;
#ifdef QCharts
    QPlotWindow* Plot(TimeSeries<timeseriesprecision>& plotitem, bool allowtime = true);
    QPlotWindow* Plot(TimeSeries<timeseriesprecision>& plotitem, Quan* quan, bool allowtime = true);
    QPlotWindow* Plot(TimeSeries<timeseriesprecision>& plotmodeled, TimeSeries<timeseriesprecision>& plotobserved);
    QPlotWindow* Plot(TimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime=true);
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
    /**
     * @brief Flags the model as having unsaved changes (or as freshly saved/loaded)
     *
     * Updates the window title, where unsaved changes are shown with a trailing '*'.
     */
    void SetModified(bool modified = true);
    bool IsModified() const { return modelModified; }
    /**
     * @brief Shows what the user is doing or what is selected, on the left of the status bar
     *
     * Cursor coordinates live in their own permanent label on the right, so these two
     * no longer overwrite each other.
     */
    void ShowStatusHint(const QString &hint);
    void ShowCursorPosition(int x, int y);
    /**
     * @brief Puts the selected link's type and endpoints in the status bar
     * @param edge the selected link, or nullptr to clear
     */
    void ShowSelectedEdge(Edge *edge);
    void ShowSelectedNode(Node *node);
    /** @brief Unchecks every link tool and drops the armed link type */
    void ClearLinkMode();
    QString* GetWorkingFolder()
    {
        return &workingfolder;
    }
private:
    logwindow *LogWindow;
    QDockWidget *logDock = nullptr;
    QLabel *cursorPositionLabel = nullptr;
    void SetupLogDock();
    void SetupStatusBar();
    /**
     * @brief Applies the translucent look while the log is floating over the diagram
     *
     * Window opacity only has an effect on top level windows, so it is applied when the
     * dock is undocked and dropped again when it is re docked.
     */
    void UpdateLogDockOpacity(bool floating);
    Ui::MainWindow *ui;
    System system;
    DiagramView* dView;
    QAction* actionpan = nullptr;
    QAction* actionzoomwindow = nullptr;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    void addToolbarLabel(QToolBar* toolbar, const QString& text);
    bool ReCreateObjectsMenu();
    PropModel *propmodel = nullptr;
    void Populate_General_ToolBar();
    string maintemplatefilename;
    vector<string> addedtemplatefilenames;
    string entitiesfilename;
    ProgressWindow *rtw = nullptr;
    /**
     * @brief Creates the progress window for a run, discarding the one from the previous run
     *
     * The window is deliberately not self-deleting on close: the solver holds this
     * pointer for the whole run, so it stays alive until the next run replaces it.
     */
    ProgressWindow *CreateProgressWindow();
    void closeEvent (QCloseEvent *event) override;
    QString workingfolder = ".";
    bool modelModified = false;
    void UpdateWindowTitle();
    /**
     * @brief Offers to save a modified model before it is discarded
     * @return false if the user cancelled, in which case the caller must abort
     */
    bool MaybeSaveChanges();
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
    QAction* actionviz = nullptr;
    ItemPropertiesWidget *PropertiesWidget = nullptr;
    QSplitter *browserSplitter = nullptr;
    void SetupObjectBrowserSplitter();
    void SaveObjectBrowserSplitterState();
    QMap<QString, QToolBar*> categoryToolbars_;
    /**
     * @brief The object-creation actions rebuilt whenever the template set changes
     *
     * They are parented to the main window, so clearing the toolbars and menus that
     * show them does not destroy them; they are tracked here and deleted on rebuild.
     */
    QList<QAction*> toolbarObjectActions_;
    QList<QAction*> menuObjectActions_;

    struct NameConflict {
        QString objectType;      // e.g., "Block", "Link", "Parameter"
        QString objectName;      // Original conflicting name
        QString suggestedNewName; // Suggested replacement name
    };

    // Helper methods for import functionality
    QList<NameConflict> checkForNameConflicts(System* importSystem);
    bool resolveNameConflicts(Script& importScript, const QList<NameConflict>& conflicts);

private slots:
    void on_check_object_browser();
    void on_check_showlogwindow();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddcomposite();
    void onungroupcomposite();
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
    void onimport();
    void onVisualize();
};

QString localAppFolderAddress();

#endif // MAINWINDOW_H
