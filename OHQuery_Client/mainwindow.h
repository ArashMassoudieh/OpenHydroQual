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
    void sendParameters(const QJsonDocument& jsonDoc); //Send Parameters
public slots:
    void handleData(const QJsonDocument &JsonDoc); //Handle the model output data recieved
    void TemplateRecieved(const QJsonDocument &JsonDoc); //Template Recieved

};
#endif // MAINWINDOW_H
