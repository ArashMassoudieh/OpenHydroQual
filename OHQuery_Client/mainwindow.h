#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "wsclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void RecieveTemplate();
private:
    Ui::MainWindow *ui;
    void PopulateListOfWizards();
    WSClient * wsClient = nullptr;

public slots:
    void handleData(const QJsonDocument &JsonDoc);
    void sendParameters(const QJsonDocument& jsonDoc);
};
#endif // MAINWINDOW_H
