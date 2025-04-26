#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Wizard_Script.h"
#include "QDir"
#include "wizarddialog.h"
#include <QJsonDocument>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QUrl url("ws://localhost:12345");  // Change the port to match your server
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
    qDebug()<<JsonDoc;
}



