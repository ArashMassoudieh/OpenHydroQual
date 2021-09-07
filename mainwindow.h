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
#include "logwindow.h"
#include "aboutdialog.h"



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
    void PopulatePropertyTable(QuanSet* quanset);
    void RecreateGraphicItemsFromSystem(bool zoom_all=true);
    void RefreshTreeView();
    Plotter* Plot(CTimeSeries& plotitem);
    Plotter* Plot(CTimeSeries& plotmodeled, CTimeSeries& plotobserved);
    string CreateNewName(string type);
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
private:
    logwindow *LogWindow;
    Ui::MainWindow *ui;
    System system;
    DiagramView* dView;
    QAction* actionpan = nullptr;
    QAction* actionzoomwindow = nullptr;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    PropModel *propmodel = nullptr;
    void Populate_General_ToolBar();
    string maintemplatefilename;
    vector<string> addedtemplatefilenames;
    string entitiesfilename;
    RunTimeWindow *rtw = nullptr;
    void closeEvent (QCloseEvent *event) override;
    QString workingfolder;
    QModelIndex addParameterIndex(const QModelIndex &index = QModelIndex());
    QModelIndex tableitemrightckicked;
    QMenu *menu = nullptr;
    CGA<System> *optimizer;
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
    void onzoomall();
    void onabout();
    void onpantriggered();
    void onzoomwindowtriggered();
    void onsave();
    void onsaveas();
    void onexporttosvg();
    void onopen();
    void onnewproject();
    void onrunmodel();
    void onoptimize();
    void oninverserun();
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
};

QString localAppFolderAddress();

#endif // MAINWINDOW_H
