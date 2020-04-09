#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"
#include <QTreeWidget>
#include "propmodel.h"
#include "diagramview.h"

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
    void AddLink(const QString &LinkName, const QString &sourceblock, const QString &targetblock, const QString &type, Edge* edge);
private:
    Ui::MainWindow *ui;
    System system;
    DiagramView* dView;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    void RefreshTreeView();
    string CreateNewName(string type);
    void PopulatePropertyTable(QuanSet* quanset);
    PropModel *propmodel = nullptr;
    void Populate_General_ToolBar();

private slots:
    void on_check_object_browser();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddlink();
    void onaddsource();
    void onaddparameter();
    void onaddentity();
    void preparetreeviewMenu(const QPoint &pos);
    void onTreeSelectionChanged(QTreeWidgetItem *current);
    void onzoomin();
    void onzoomout();
    void onzoomall();

};

#endif // MAINWINDOW_H
