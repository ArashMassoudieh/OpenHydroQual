#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QtCharts/QChartView>
#include "apiclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onProcessTriggered();
    void handleData(const TimeSeriesMap& seriesMap);
    void handleError(const QString& error);

private:
    Ui::MainWindow *ui;
    ApiClient* apiClient;
    QMap<QString, QChartView*> chartviews;
};

#endif // MAINWINDOW_H
