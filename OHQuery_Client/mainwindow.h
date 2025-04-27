#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "wsclient.h"
#include <QtCharts/QChartView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using TimeSeriesMap = QMap<QString, TimeSeries>;

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
    QMap<QString, QChartView*> chartviews;
public slots:
    void handleData(const QJsonDocument &JsonDoc); //Handle the model output data recieved
    void TemplateRecieved(const QJsonDocument &JsonDoc); //Template Recieved

};

QDateTime excelToQDateTime(double excelDate);

#endif // MAINWINDOW_H
