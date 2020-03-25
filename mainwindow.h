#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "System.h"

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
private slots:
    void on_check_object_browser();
    void on_object_browser_closed(bool visible);
    void onaddblock();
    void onaddsource();
    void onaddentity();

};

#endif // MAINWINDOW_H
