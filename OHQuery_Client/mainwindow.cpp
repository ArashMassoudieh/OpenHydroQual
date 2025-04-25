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
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::handleData);
    connect(wsClient, &WSClient::connected, this, &MainWindow::RecieveTemplate);


}

void MainWindow::RecieveTemplate()
{
    QJsonObject ModelFile;
    ModelFile["FileName"] = "StormwaterPond.json";
    QJsonObject response;
    response["Model"] = ModelFile;
    wsClient->sendJson(response);  // now async
}

void MainWindow::sendParameters(const QJsonDocument& jsonDoc)
{
    wsClient->sendJson(jsonDoc.object());  // now async
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleData(const QJsonDocument &JsonDoc)
{
    WizardScript wiz;
    wiz.GetFromJsonDoc(JsonDoc);
    WizardDialog *wizDialog = new WizardDialog(this);
    wizDialog->setWindowTitle(wiz.Description());
    wizDialog->CreateItems(&wiz);
    ui->horizontalLayout->addWidget(wizDialog);
    connect(wsClient, &WSClient::dataReady, this, &MainWindow::handleData);
    connect(wizDialog, &WizardDialog::model_generate_requested, this, &MainWindow::sendParameters);
}



