#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"
#include <QTreeWidget>
#include "propmodel.h"

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
    System *GetSystem() {return &system;}
private:
    Ui::MainWindow *ui;
    System system;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    void RefreshTreeView();
    string CreateNewName(string type);
    void PopulatePropertyTable(QuanSet* quanset);
    PropModel *propmodel = nullptr;
private slots:
    void on_check_object_browser();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddsource();
    void onaddparameter();
    void onaddentity();
    void preparetreeviewMenu(const QPoint &pos);
    void onTreeSelectionChanged(QTreeWidgetItem *current);

};

#endif // MAINWINDOW_H
