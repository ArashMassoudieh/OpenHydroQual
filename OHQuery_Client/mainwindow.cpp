#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Wizard_Script.h"
#include "QDir"
#include "wizarddialog.h"
#include <QJsonDocument>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QDateTimeAxis>
#include <QValueAxis>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QUrl url("ws://localhost:12345");  // Change the port to match your server
    QUrl url("ws://ec2-34-221-236-134.us-west-2.compute.amazonaws.com:12345");  // Change the port to match your server

    wsClient = new WSClient(url);

    // Connect async response
    connect(wsClient, &WSClient::connected, this, &MainWindow::RecieveTemplate);


}

void MainWindow::RecieveTemplate()
{
    QJsonObject ModelFile;
    ModelFile["FileName"] = "StormwaterPond.json";
    QJsonObject response;
    response["Model"] = ModelFile;
    wsClient->sendJson(response);  // now async
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::TemplateRecieved);
}

void MainWindow::sendParameters(const QJsonDocument& jsonDoc)
{
    disconnect(wsClient, &WSClient::dataReady, this, &MainWindow::TemplateRecieved);
    wsClient->sendJson(jsonDoc.object());  // now async
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::handleData);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::TemplateRecieved(const QJsonDocument &JsonDoc)
{
    WizardScript wiz;
    qDebug()<<JsonDoc;
    wiz.GetFromJsonDoc(JsonDoc);
    WizardDialog *wizDialog = new WizardDialog(this);
    wizDialog->setWindowTitle(wiz.Description());
    wizDialog->CreateItems(&wiz);
    ui->horizontalLayout->addWidget(wizDialog);
    disconnect(wsClient, &WSClient::dataReady, this, &MainWindow::handleData);
    connect(wizDialog, &WizardDialog::model_generate_requested, this, &MainWindow::sendParameters);
}

void MainWindow::handleData(const QJsonDocument &JsonDoc)
{
    for (const QChartView* item : chartviews)
        delete item;

    TimeSeriesMap allSeries;

    QJsonObject rootObj = JsonDoc.object();
    for (const QString& key : rootObj.keys()) {
        QJsonObject variable = rootObj[key].toObject();
        QJsonArray tArr = variable["t"].toArray();
        QJsonArray valArr = variable["value"].toArray();

        TimeSeries ts;
        for (int i = 0; i < tArr.size(); ++i) {
            ts.t.append(tArr[i].toDouble());
            ts.value.append(valArr[i].toDouble());
        }

        allSeries[key] = ts;
    }

    chartviews.clear();

    for (const QString& key : allSeries.keys()) {
        const TimeSeries& ts = allSeries[key];
        QLineSeries* series = new QLineSeries();

        for (int i = 0; i < ts.t.size(); ++i)
        {
            QDateTime dt = excelToQDateTime(ts.t[i]);
            series->append(dt.toMSecsSinceEpoch(), ts.value[i]);
        }

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(key);

        // X-axis as DateTimeAxis
        QDateTimeAxis* axisX = new QDateTimeAxis;
        axisX->setFormat("yyyy-MM-dd");
        axisX->setTitleText("Time");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        // Y-axis
        QValueAxis* axisY = new QValueAxis;
        axisY->setTitleText(key);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartviews[key] = chartView;

        ui->ChartsLayout->addWidget(chartView);    }
}

QDateTime excelToQDateTime(double excelDate) {
    if (excelDate < 1.0) {
        qDebug() << "Invalid Excel date. Must be >= 1.0.";
        return QDateTime();
    }

    // Excel base date: January 1, 1900
    QDate excelBaseDate(1900, 1, 1);

    // Excel leap year bug: Skip February 29, 1900 (nonexistent day)
    int days = static_cast<int>(excelDate); // Integer part represents days
    if (days >= 60) {
        days -= 1; // Correct for Excel's leap year bug
    }

    // Add the integer days to the base date
    QDate convertedDate = excelBaseDate.addDays(days - 1);

    // Extract the fractional part for time
    double fractionalDay = excelDate - static_cast<int>(excelDate);
    int totalSeconds = static_cast<int>(fractionalDay * 86400); // Convert fraction to seconds
    QTime convertedTime = QTime(0, 0, 0).addSecs(totalSeconds);

    // Combine the date and time
    return QDateTime(convertedDate, convertedTime);
}

