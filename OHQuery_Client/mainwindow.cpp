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
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>

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
    ui->label->setText("WebSocket error occurred:" + socketErrorToString(error));

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
    QJsonObject ModelFile;
    ModelFile["FileName"] = modeltemplate;
    QJsonObject response;
    response["Model"] = ModelFile;
    wsClient->sendJson(response);  // now async
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::TemplateRecieved);
}

void MainWindow::sendParameters(const QJsonDocument& jsonDoc)
{
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
    qDebug()<<JsonDoc;
    wiz.GetFromJsonDoc(JsonDoc);
    WizardDialog *wizDialog = new WizardDialog(this);
    wizDialog->setWindowTitle(wiz.Description());
    wizDialog->CreateItems(&wiz);
    ui->label->hide();
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
        if (key == "TemporaryFolderName")
        {
            TemporaryFolderName = rootObj[key].toString();
        }
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

        ui->ChartsLayout->addWidget(chartView);
    }

    if (!DownloadModelButton)
    {   DownloadModelButton = new QPushButton(this);
        DownloadPrecipButton = new QPushButton(this);
        DownloadModelButton->setText("Download the OpenHydroQual Model");
        DownloadPrecipButton->setText("Download precipitation data");
        ui->horizontalLayout_buttons->addWidget(DownloadModelButton);
        ui->horizontalLayout_buttons->addWidget(DownloadPrecipButton);
        connect(DownloadModelButton, &QPushButton::clicked, this, &MainWindow::onDownloadModel);
    }
    QGuiApplication::restoreOverrideCursor();

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
