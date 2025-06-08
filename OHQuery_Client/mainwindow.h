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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "qlabel.h"
#include "wsclient.h"
#include <QtCharts/QChartView>
#include <QTextBrowser>
#include "timeseriesloader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void RecieveTemplate();
    void SetModelTemplate(const QString &jsonfile) {modeltemplate = jsonfile;}
    void PopulatePrecipTextBrowser();
    void showErrorWindow(const QString& message);
private:
    Ui::MainWindow *ui;
    void PopulateListOfWizards();
    WSClient * wsClient = nullptr;
    void sendParameters(const QJsonDocument& jsonDoc); //Send Parameters
    QMap<QString, QChartView*> chartviews;
    void downloadFileAndTriggerBrowserSave(const QUrl& fileUrl, const QString& downloadName, QObject* parent = nullptr);
    void saveLocalFileToBrowser(const QString& sourceFilePath, const QString& downloadName);
    QString TemporaryFolderName;
    QMap<QString,QString> DownloadedTimeSeriesData;
    QPushButton* DownloadModelButton = nullptr;
    QTextBrowser* DownloadPrecipTextBrowser = nullptr;
    QTextBrowser* DownloadOutputTextBrowser = nullptr;
    QString modeltemplate;
    QLabel *errorBanner = new QLabel();
    TimeSeriesLoader* loader = nullptr;
public slots:
    void handleData(const QJsonDocument &JsonDoc); //Handle the model output data recieved
    void TemplateRecieved(const QJsonDocument &JsonDoc); //Template Recieved
    void onError(QAbstractSocket::SocketError error);
    void onDownloadModel();
    void handleLoadedTimeSeries(const QMap<QString, TimeSeries>& tsMap);


};

QDateTime excelToQDateTime(double excelDate);
QString socketErrorToString(QAbstractSocket::SocketError error);

#endif // MAINWINDOW_H
