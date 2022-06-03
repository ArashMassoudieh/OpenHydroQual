#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"
#include <QTreeWidget>
#include "propmodel.h"
#include "diagramview.h"
#include "runtimewindow.h"
#include "plotter.h"
#include "GA.h"
#include "MCMC.h"
#include "logwindow.h"
#include "aboutdialog.h"
#include "undodata.h"

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
    void RefreshTreeView();
    Plotter* Plot(CTimeSeries<timeseriesprecision>& plotitem, bool allowtime = true);
    Plotter* Plot(CTimeSeries<timeseriesprecision>& plotmodeled, CTimeSeries<timeseriesprecision>& plotobserved);
    Plotter* Plot(CTimeSeriesSet<timeseriesprecision>& plotitem, bool allowtime=true);
    string CreateNewName(string type, bool allow_paranthesis = true);
    DiagramView* GetDiagramView() { return dView; }
    bool Log(const QString &s);
    bool LogError(const QString &s);
    bool LogAddDelete(const QString &s);
    void LogAllSystemErrors(ErrorHandler *errs=nullptr);
    QMap<QCPGraph *, plotformat> graphsClipboard;
    void SetPan(bool panmode) {actionpan->setChecked(panmode); if (!panmode) dView->setMode(Operation_Modes::NormalMode); dView->setModeCursor(); }
    void SetZoomWindow(bool panmode) {actionzoomwindow->setChecked(panmode); if (!panmode) dView->setMode(Operation_Modes::NormalMode); dView->setModeCursor(); }
    void addplugin(const QString &fileName);
    void ResetSystem();
    void InactivateUndo(bool yes=true);
    void InactivateRedo(bool yes=true);
    void AddStatetoUndoData();
    void SetActiveUndo();
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
    void showgraph();
    void onDeleteItem();
    void on_actionRecent_triggered();
    void on_Undo();
    void on_Redo();
    void onCreate2dArray();// Is called when 2D Array action is triggered
};

QString localAppFolderAddress();

#endif // MAINWINDOW_H
