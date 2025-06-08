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


#include "wsserverop.h"
#include "System.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFileInfo>
#include "Script.h"
#include <QFile>
#include "Wizard_Script.h"
#include <QDir>
#ifdef HTTPS
#include <QSslKey>
#endif

#ifndef HTTPS
WSServerOps::WSServerOps(QObject *parent)
    : QObject(parent),
    m_server(new QWebSocketServer(QStringLiteral("Echo Server"),
                                  QWebSocketServer::NonSecureMode, this))
{


}

void WSServerOps::Start(quint16 port)
{
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "WebSocket server listening on port" << port;
        connect(m_server, &QWebSocketServer::newConnection,
                this, &WSServerOps::onNewConnection);
    }

}
#else

WSServerOps::WSServerOps(const QString &certPath, const QString &keyPath, QObject *parent)
    : QObject(parent),
    m_server(new QWebSocketServer(QStringLiteral("Echo Server"),
                                  QWebSocketServer::SecureMode, this))
    ,m_certPath(certPath), m_keyPath(keyPath)
{
    qDebug()<<"Certificate path: " <<m_certPath;
    qDebug()<<"Key path: " <<m_keyPath;
}

void WSServerOps::Start(quint16 port)
{
    // Load SSL configuration
    QSslConfiguration sslConfiguration;
    QFile certFile(m_certPath);
    QFile keyFile(m_keyPath);
    if (!certFile.open(QIODevice::ReadOnly) || !keyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open SSL certificate or key file!";
        return;
    }

    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);

    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);  // Don't verify clients
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1_2);

    m_server->setSslConfiguration(sslConfiguration);

    // Start listening
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "Secure WebSocket server listening on port" << port;
        connect(m_server, &QWebSocketServer::newConnection,
                this, &WSServerOps::onNewConnection);
    } else {
        qDebug() << "Failed to start WebSocket server on port" << port;
    }
}
#endif

WSServerOps::~WSServerOps()
{
    m_server->close();
    qDeleteAll(m_clients);
}

void WSServerOps::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();

    connect(socket, &QWebSocket::textMessageReceived,
            this, &WSServerOps::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
            this, &WSServerOps::onSocketDisconnected);

    m_server->setMaxPendingConnections(100);

    qDebug() << "New client socket connected, waiting for client_id...";

}

void WSServerOps::onTextMessageReceived(QString message)
{
    QWebSocket *senderSocket = qobject_cast<QWebSocket *>(sender());
    if (!senderSocket)
        return;

    qDebug() << "Received message from client:" << message;

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return;
    }

    QJsonObject root_obj = jsonDoc.object();

    QString client_id = root_obj.value("client_id").toString();
    if (client_id.isEmpty()) {
        qWarning() << "Missing client_id in message!";
        return;
    }

    // --- NEW: Register or update client ---
    if (!m_clients.contains(client_id)) {
        qDebug() << "New client registered: " << client_id;
        m_clients[client_id] = senderSocket;
    } else if (m_clients[client_id] != senderSocket) {
        qDebug() << "Client reconnected: " << client_id;
        m_clients[client_id] = senderSocket;
    }

    if (senderSocket) {
        qDebug() << "Received message from client:" << message;

        QJsonObject root_obj = jsonDoc.object();
        if (root_obj.keys().contains("Reconnected") && root_obj["Reconnected"].toBool() == true)
        {
            qDebug()<<"Seding message to the client: "<<message_to_be_sent[client_id];
            sendMessageToClient(senderSocket, message_to_be_sent[client_id]);
        }
        else if (root_obj.keys().contains("Model"))
        {
            QJsonDocument responseDoc = SendModelTemplate(root_obj["Model"].toObject()["FileName"].toString());
            qDebug()<<responseDoc;
            QString jsonString = QString::fromUtf8(responseDoc.toJson(QJsonDocument::Compact));
            client_state[client_id] = "Model Sent";
            message_to_be_sent[client_id] = jsonString;
            sendMessageToClient(senderSocket, jsonString);
        }
        else if (root_obj.keys().contains("Parameters"))
        {
            QJsonObject obj = root_obj["Parameters"].toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                QString key = it.key();
                QJsonValue value = it.value();
                qDebug() << "Key:" << key << ", Value:" << value;
            }
            WizardScript SelectedWizardScript(TemplateFile_Fullpath);
            SelectedWizardScript.AssignParameterValues(obj);

            QString randomFolderName = QUuid::createUuid().toString(QUuid::WithoutBraces);

            // Set the path where you want to create the folder
#ifndef LOCAL_HOST
            QString basePath = "/home/ubuntu";
#else
            QString basePath = QDir::homePath();
#endif
            QString newFolderPath = basePath + "/OHQueryTemporaryFolder/" + randomFolderName;

            // Create the directory
            QDir dir;

            QString MainFolderPath = basePath + "/OHQueryTemporaryFolder/";

            if (!dir.exists(MainFolderPath)) {
                if (dir.mkpath(MainFolderPath)) {
                    qDebug() << "Folder created successfully:" << MainFolderPath;
                } else {
                    qDebug() << "Failed to create folder:" << MainFolderPath;
                }
            } else {
                qDebug() << "Folder already exists:" << MainFolderPath;
            }

            if (dir.mkpath(newFolderPath)) {
                qDebug() << "Folder created successfully:" << newFolderPath;
            } else {
                qDebug() << "Failed to create folder.";
            }

            SelectedWizardScript.SetWorkingFolder(newFolderPath);

            Script script;
            System system;
            string defaulttemppath = QCoreApplication::applicationDirPath().toStdString() + "/../../resources/";
            system.ReadSystemSettingsTemplate(qApp->applicationDirPath().toStdString() + "/../../resources/settings.json");
            cout << "Default Template path = " + defaulttemppath +"\n";
            system.SetDefaultTemplatePath(defaulttemppath);
            system.SetWorkingFolder(newFolderPath.toStdString());
            script.CreateSystemFromQStringList(SelectedWizardScript.Script(),&system);
            QJsonObject responseObj = Execute(&system);
            QJsonObject TimeSeriesData;

            QMap<QString, QString> TimeSeriesDataMap = SelectedWizardScript.GetTimeSeriesData();
            for (QString tskey: TimeSeriesDataMap.keys())
            {
                TimeSeriesData[tskey] = TimeSeriesDataMap[tskey];
            }
            responseObj["DownloadedTimeSeriesData"] = TimeSeriesData;

            QJsonDocument responseDoc = QJsonDocument(responseObj);
            QString jsonString = QString::fromUtf8(responseDoc.toJson(QJsonDocument::Compact));
            qDebug()<<"Sending message of size " << jsonString.size() << " to the client";
            client_state[client_id] = "Model Simulation Ended";
            message_to_be_sent[client_id] = jsonString;
            sendMessageToClient(senderSocket, jsonString);
        }
    }
}

QJsonDocument WSServerOps::SendModelTemplate(const QString &TemplateName)
{
    QFile file(QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName);
    qDebug()<<QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
        return QJsonDocument(); // returns a null document
    }
    else
    {
        TemplateFile_Fullpath = QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return QJsonDocument();
    }

    return jsonDoc;
}

void WSServerOps::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (!socket)
        return;

    QString toRemoveKey;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == socket) {
            toRemoveKey = it.key();
            break;
        }
    }

    if (!toRemoveKey.isEmpty()) {
        m_clients.remove(toRemoveKey);
        client_state.remove(toRemoveKey);
        qDebug() << "Client disconnected and removed: " << toRemoveKey;
    }

    socket->deleteLater();
}


void WSServerOps::sendMessageToClient(QWebSocket *client, const QString &message)
{
    qDebug() << "Message to be sent to the client:" << message;
    if (client) {
        client->sendTextMessage(message);
        qDebug() << "Sent message to client:" << message;
    }
}

QJsonObject WSServerOps::Execute(System *system)
{

    string settingfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";

    cout<<"Executing script ..."<<endl;

    system->SetSilent(false);
    system->SavetoScriptFile(system->GetWorkingFolder() + "/" + "System.ohq");
    cout<<"Solving ..."<<endl;
    system->Solve();
    system->SavetoJson(system->GetWorkingFolder() + "/" + "System.json",system->addedtemplates);


    qDebug()<<QString::fromStdString(system->GetWorkingFolder()) + "/output.json";
    QFile file(QString::fromStdString(system->GetWorkingFolder()) + "/output.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
    }

    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + "/" + system->OutputFileName() +"'";
    system->GetObservedOutputs().writetofile(system->GetWorkingFolder() + "/" + system->ObservedOutputFileName());
    QJsonDocument doc(system->GetObservedOutputs().toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    system->GetOutputs().writetofile(system->GetWorkingFolder() + "/" + system->OutputFileName());
    QJsonObject output;
#ifdef LOCAL_HOST
    output = system->GetObservedOutputs().toJson();
#endif
    output["TemporaryFolderName"] = QString::fromStdString(system->GetWorkingFolder());

    return output;
}




