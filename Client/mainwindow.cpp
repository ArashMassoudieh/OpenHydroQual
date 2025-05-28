/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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

    // Initialize the API client with this as the parent
    //apiClient = new ApiClient(QUrl("http://localhost:8080/calculate"), this);
    apiClient = new ApiClient(QUrl("http://ec2-54-213-147-59.us-west-2.compute.amazonaws.com:8080/calculate"), this);

    // Connect button click
    connect(ui->pushButtonProcess, &QPushButton::clicked, this, &MainWindow::onProcessTriggered);

    // Connect async response
    connect(apiClient, &ApiClient::dataReady, this, &MainWindow::handleData);
    connect(apiClient, &ApiClient::requestFailed, this, &MainWindow::handleError);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProcessTriggered()
{
    QJsonObject kd;
    kd["base_value"] = ui->lineEditKd->text();
    QJsonObject request;
    request["kd"] = kd;

    apiClient->fetchAllSeries(request);  // now async
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
