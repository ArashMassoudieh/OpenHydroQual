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
#include "qmessagebox.h"
#include "ui_mainwindow.h"
#include "Wizard_Script.h"
#include "QDir"
#include "wizarddialog.h"
#include <QJsonDocument>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include "timeseriesloader.h"

#ifdef Q_OS_WASM
#include <emscripten.h>
#else
#include <QFileDialog>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->chartTabs->hide();
    this->setStyleSheet("background-color: white;");

    errorBanner = new QLabel(this);
    errorBanner->setStyleSheet("background-color:#fee; color:red; font-weight:bold; border: 1px solid red; padding: 5px;");
    errorBanner->setAlignment(Qt::AlignCenter);
    errorBanner->hide(); // initially hidden
    ui->verticalLayout_2->insertWidget(0, errorBanner); // insert at top of main layout

    qApp->setStyleSheet(R"(
    * {
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        font-size: 14px;
    }

    QWidget {
        background-color: white;
    }

    QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QPushButton, QDateEdit {
        padding: 6px;
        color: black;
        background-color: white;
        border: 1px solid #ccc;
    }

    QLabel {
        color: black;
    }

    QTabWidget::pane {
        border: 1px solid #ccc;
        background-color: white;
    }

    QTabBar::tab {
        background: #e0e0e0;
        color: black;
        padding: 6px 12px;
        border: 1px solid #bbb;
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
    }

    QTabBar::tab:selected {
        background: white;
        border-color: #aaa;
        font-weight: bold;
    }

    QTabBar::tab:hover {
        background: #f0f0f0;
    }
)");

#ifdef LOCALHOST
    QUrl url("ws://localhost:12345");  // Change the port to match your server
#else
    #ifdef HTTPs
        QUrl url("wss://greeninfraiq.com/ws/");  // Change the port to match your server
    #else
        QUrl url("ws://greeninfraiq.com:12345");  // Change the port to match your server
    #endif
#endif

    wsClient = new WSClient(url);

    // Connect async response
    connect(wsClient, &WSClient::connected, this, &MainWindow::RecieveTemplate);
    connect(wsClient, &WSClient::socketError, this, &MainWindow::onError);

}

void MainWindow::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "WebSocket error occurred:" << error;
    showErrorWindow("The output data was failed to be retrieved from the server: WebSocket error occurred:" + socketErrorToString(error));

    return;

}

void MainWindow::showErrorWindow(const QString& message)
{
    errorBanner->setText(message);
    errorBanner->show();
}

QString socketErrorToString(QAbstractSocket::SocketError error)
{
    switch (error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        return "Connection refused by the server";
    case QAbstractSocket::RemoteHostClosedError:
        return "Remote host closed the connection";
    case QAbstractSocket::HostNotFoundError:
        return "Host not found";
    case QAbstractSocket::SocketAccessError:
        return "Socket access error";
    case QAbstractSocket::SocketResourceError:
        return "Socket resource error (too many open sockets?)";
    case QAbstractSocket::SocketTimeoutError:
        return "Socket operation timed out";
    case QAbstractSocket::DatagramTooLargeError:
        return "Datagram too large to send";
    case QAbstractSocket::NetworkError:
        return "Network error occurred";
    case QAbstractSocket::AddressInUseError:
        return "Address already in use";
    case QAbstractSocket::SocketAddressNotAvailableError:
        return "Socket address not available";
    case QAbstractSocket::UnsupportedSocketOperationError:
        return "Unsupported socket operation";
    case QAbstractSocket::UnfinishedSocketOperationError:
        return "Unfinished socket operation";
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return "Proxy authentication required";
    case QAbstractSocket::SslHandshakeFailedError:
        return "SSL/TLS handshake failed";
    case QAbstractSocket::ProxyConnectionRefusedError:
        return "Proxy connection was refused";
    case QAbstractSocket::ProxyConnectionClosedError:
        return "Proxy connection closed unexpectedly";
    case QAbstractSocket::ProxyConnectionTimeoutError:
        return "Proxy connection timed out";
    case QAbstractSocket::ProxyNotFoundError:
        return "Proxy server not found";
    case QAbstractSocket::ProxyProtocolError:
        return "Proxy protocol error";
    case QAbstractSocket::OperationError:
        return "Operation error (an operation is in progress)";
    case QAbstractSocket::SslInternalError:
        return "Internal SSL error occurred";
    case QAbstractSocket::SslInvalidUserDataError:
        return "Invalid SSL user data";
    case QAbstractSocket::TemporaryError:
        return "Temporary error (try again)";
    case QAbstractSocket::UnknownSocketError:
    default:
        return "Unknown socket error";
    }
}

void MainWindow::RecieveTemplate()
{
    if (templateAlreadyRequested)
        return;

    if (!wsClient->HasConnectedOnce())
    {
        templateAlreadyRequested = true;
        QJsonObject ModelFile;
        ModelFile["FileName"] = modeltemplate;
        QJsonObject response;
        response["Model"] = ModelFile;
        wsClient->sendJson(response);  // now async
        disconnect(wsClient, &WSClient::socketError, this, &MainWindow::onError);
        connect(wsClient, &WSClient::dataReady, this, &MainWindow::TemplateRecieved);
    }
    else
        disconnect(wsClient, &WSClient::dataReady, this, &MainWindow::RecieveTemplate);
}

void MainWindow::sendParameters(const QJsonDocument& jsonDoc)
{
    if (jsonDoc.isNull())
        return;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
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
    qDebug()<<"Template Recieved: "<<JsonDoc;
    wiz.GetFromJsonDoc(JsonDoc);
    WizardDialog *wizDialog = new WizardDialog(this);
    wizDialog->setWindowTitle(wiz.Description());
    wizDialog->CreateItems(&wiz);
    ui->label->hide();
    ui->horizontalLayout->addWidget(wizDialog);
    disconnect(wsClient, &WSClient::dataReady, this, &MainWindow::TemplateRecieved);
    connect(wizDialog, &WizardDialog::model_generate_requested, this, &MainWindow::sendParameters);
}

void MainWindow::handleData(const QJsonDocument &JsonDoc)
{
    qDebug()<<"Handling data ...";
    if (!JsonDoc.isObject()) {
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("Error");
        msgBox->setText("The output data was failed to be retrieved from the server");
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->open();
        return;
    }

    TimeSeriesMap allSeries;

    QJsonObject rootObj = JsonDoc.object();

    qDebug()<<"Message Recieved: "<<rootObj;
    for (const QString& key : rootObj.keys()) {
        if (key == "TemporaryFolderName")
        {
            TemporaryFolderName = rootObj[key].toString();
        }
        else if (key == "DownloadedTimeSeriesData")
        {
            QJsonObject TimeSeriesInfo = rootObj[key].toObject();
            for (const QString& key_TS : TimeSeriesInfo.keys()) {
                 DownloadedTimeSeriesData[key_TS] = TimeSeriesInfo[key_TS].toString();
            }
        }
#ifdef LOCALHOST
        else
        {
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
#else

#endif
    }
    qDebug()<<"Attempting to download the results ...";
    loader = new TimeSeriesLoader(this);
    QString cleanedFilePath = TemporaryFolderName.remove("/home/ubuntu/OHQueryTemporaryFolder/");
    QString url = "https://www.greeninfraiq.com/modeldata/" + cleanedFilePath + "/output.json";
    qDebug()<<"Loading data from " + url;
    loader->load(QUrl(url));
    connect(loader, &TimeSeriesLoader::timeSeriesLoaded, this, &MainWindow::handleLoadedTimeSeries);
    connect(loader, &TimeSeriesLoader::loadFailed, this, &MainWindow::showErrorWindow);

}

void MainWindow::handleLoadedTimeSeries(const QMap<QString, TimeSeries>& tsMap)
{
    if (resultsRead) return;
    resultsRead = true;
    qDebug()<<"Processing loaded time series data ... ";
    for (const QChartView* item : chartviews)
        delete item;

    qDebug()<<"Previous charts deleted";
    chartviews.clear();
    qDebug()<<"Clear!";

    for (const QString& key : tsMap.keys()) {
        qDebug()<<"Chart: " << key;
        const TimeSeries& ts = tsMap[key];
        QLineSeries* series = new QLineSeries();

        for (int i = 0; i < ts.t.size(); ++i)
        {
            QDateTime dt = excelToQDateTime(ts.t[i]);
            series->append(dt.toMSecsSinceEpoch(), ts.value[i]);
        }

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(convertToSuperscript(key));

        // X-axis as DateTimeAxis
        QDateTimeAxis* axisX = new QDateTimeAxis;
        axisX->setFormat("yyyy-MM-dd");
        axisX->setTitleText("Time");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        // Y-axis
        QValueAxis* axisY = new QValueAxis;
        axisY->setTitleText(convertToSuperscript(key));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartviews[key] = chartView;

        ui->chartTabs->addTab(chartView, convertToSuperscript(key));
    }

    if (!DownloadModelButton)
    {   DownloadModelButton = new QPushButton(this);
        DownloadPrecipTextBrowser = new QTextBrowser(this);
        DownloadOutputTextBrowser = new QTextBrowser(this);
        DownloadModelButton->setText("Download the OpenHydroQual Model");

        DownloadPrecipTextBrowser->setText("Download timeseries data");
        DownloadOutputTextBrowser->setText("Download timeseries data");
        ui->horizontalLayout_buttons->addWidget(DownloadModelButton);
        ui->horizontalLayout_buttons->addWidget(DownloadPrecipTextBrowser);
        ui->horizontalLayout_buttons->addWidget(DownloadOutputTextBrowser);
        connect(DownloadModelButton, &QPushButton::clicked, this, &MainWindow::onDownloadModel);
    }
    PopulatePrecipTextBrowser();
    ui->chartTabs->show();
    QGuiApplication::restoreOverrideCursor();
    disconnect(loader, &TimeSeriesLoader::timeSeriesLoaded, this, &MainWindow::handleLoadedTimeSeries);
    qDebug()<<"Charts created!";
}

void MainWindow::PopulatePrecipTextBrowser()
{

    DownloadPrecipTextBrowser->clear();
    DownloadOutputTextBrowser->clear();
    DownloadPrecipTextBrowser->setOpenExternalLinks(true);
    DownloadOutputTextBrowser->setOpenExternalLinks(true);
    QString html = "<h3>Download Time Series Data</h3><ul>";
    QString cleanedFilePath = TemporaryFolderName.remove("/home/ubuntu/OHQueryTemporaryFolder/");
    for (const QString& key : DownloadedTimeSeriesData.keys())
    {
        QString title = key;
        QString url = "https://www.greeninfraiq.com/modeldata/" + cleanedFilePath + "/" + DownloadedTimeSeriesData[key];
        html += QString("<li><a href='%1'>%2</a></li>").arg(url, title);
    }
    DownloadPrecipTextBrowser->setHtml(html);

    html = "<h3>Download Model Outputs</h3><ul>";
    QString url = "https://www.greeninfraiq.com/modeldata/" + cleanedFilePath + "/observedoutput.txt";
    QString title = "Model Output";
    html += QString("<li><a href='%1'>%2</a></li>").arg(url, title);
    DownloadOutputTextBrowser->setHtml(html);

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

void MainWindow::downloadFileAndTriggerBrowserSave(const QUrl& fileUrl, const QString& downloadName, QObject* parent)
{
    auto* manager = new QNetworkAccessManager(parent);
    QNetworkRequest request(fileUrl);
    QNetworkReply* reply = manager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply, downloadName]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Download failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        reply->deleteLater();

#ifdef Q_OS_WASM
        // Use Emscripten to trigger browser download
        QString base64 = QString::fromLatin1(data.toBase64());
        QString js = QString(R"(
            const a = document.createElement('a');
            a.href = 'data:application/octet-stream;base64,%1';
            a.download = '%2';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        )").arg(base64, downloadName);

        emscripten_run_script(js.toUtf8().constData());
#else \
    // Native fallback: prompt with QFileDialog (won’t work in WASM)
        QString filename = QFileDialog::getSaveFileName(nullptr, "Save File", downloadName);
        if (!filename.isEmpty()) {
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
            }
        }
#endif
    });
}

void MainWindow::saveLocalFileToBrowser(const QString& sourceFilePath, const QString& downloadName)
{
    QFile file(sourceFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << sourceFilePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

#ifdef Q_OS_WASM
    // Use Emscripten to trigger download via browser
    QString base64 = QString::fromLatin1(data.toBase64());
    QString js = QString(R"(
        const a = document.createElement('a');
        a.href = 'data:application/octet-stream;base64,%1';
        a.download = '%2';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
    )").arg(base64, downloadName);

    emscripten_run_script(js.toUtf8().constData());

#else
    // Native fallback using QFileDialog
    QString filename = QFileDialog::getSaveFileName(nullptr, "Save File As", downloadName);
    if (!filename.isEmpty()) {
        QFile outFile(filename);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(data);
            outFile.close();
        } else {
            qWarning() << "Could not save file to" << filename;
        }
    }
#endif
}

void MainWindow::onDownloadModel()
{
#ifdef LOCALHOST
    saveLocalFileToBrowser(TemporaryFolderName + "/System.ohq","model.ohq");
#else

    QString cleanedFilePath = TemporaryFolderName.remove("/home/ubuntu/OHQueryTemporaryFolder/");
    downloadFileAndTriggerBrowserSave(QUrl("https://www.greeninfraiq.com/modeldata/" + cleanedFilePath + "/System.ohq"),"model.ohq");
#endif
}

QString convertToSuperscript(const QString& input) {
    // Superscript character map
    static const QMap<QChar, QChar> superscriptMap = {
        {'0', QChar(0x2070)},
        {'1', QChar(0x00B9)},
        {'2', QChar(0x00B2)},
        {'3', QChar(0x00B3)},
        {'4', QChar(0x2074)},
        {'5', QChar(0x2075)},
        {'6', QChar(0x2076)},
        {'7', QChar(0x2077)},
        {'8', QChar(0x2078)},
        {'9', QChar(0x2079)},
        {'+', QChar(0x207A)},
        {'-', QChar(0x207B)},
        {'=', QChar(0x207C)},
        {'(', QChar(0x207D)},
        {')', QChar(0x207E)},
        {'n', QChar(0x207F)}
        // Add more as needed
    };

    QString result;
    bool inSuperscript = false;

    for (int i = 0; i < input.length(); ++i) {
        QChar current = input[i];

        if (current == '^') {
            inSuperscript = true;
            continue;
        }

        if (inSuperscript) {
            if (superscriptMap.contains(current)) {
                result.append(superscriptMap[current]);
            } else {
                // If no superscript equivalent, append as-is or skip
                result.append(current);
            }
            inSuperscript = false;
        } else {
            result.append(current);
        }
    }

    return result;
}
