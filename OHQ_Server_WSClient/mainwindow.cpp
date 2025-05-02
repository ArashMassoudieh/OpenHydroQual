#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QUrl url("ws://localhost:12345");  // Change the port to match your server
    wsClient = new WSClient(url);
    // Connect button click
    connect(ui->pushButtonProcess, &QPushButton::clicked, this, &MainWindow::onProcessTriggered);

    // Connect async response
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::handleData);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProcessTriggered()
{
    QJsonObject ModelFile;
    ModelFile["FileName"] = "StormwaterPond.json";
    QJsonObject response;
    response["Model"] = ModelFile;
    wsClient->sendJson(response);  // now async
}

void MainWindow::handleData(const TimeSeriesMap& allSeries)
{
    for (const QChartView* item : chartviews)
        delete item;

    chartviews.clear();

    for (const QString& key : allSeries.keys()) {
        const TimeSeries& ts = allSeries[key];
        QLineSeries* series = new QLineSeries();
        for (int i = 0; i < ts.t.size(); ++i)
            series->append(ts.t[i], ts.value[i]);

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(key);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setTitleText("Time");
        chart->axes(Qt::Vertical).first()->setTitleText(key);

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartviews[key] = chartView;

        ui->ChartsLayout->addWidget(chartView);
    }
}

void MainWindow::handleError(const QString& error)
{
    qWarning() << "API Request Failed:" << error;
    // Optional: show in UI (e.g., QLabel or QMessageBox)
}
