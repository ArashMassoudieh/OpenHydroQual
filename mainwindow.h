#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"
#include <QTreeWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    System system;
    bool Populate_TreeWidget();
    bool BuildObjectsToolBar();
    void RefreshTreeView();
    string CreateNewName(string type);
    void PopulatePropertyTable(QuanSet* quanset);
private slots:
    void on_check_object_browser();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddsource();
    void onaddentity();
    void preparetreeviewMenu(const QPoint &pos);
    void onTreeSelectionChanged(QTreeWidgetItem *current);

};

#endif // MAINWINDOW_H
