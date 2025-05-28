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

    m_clients.append(socket);
    qDebug() << "New client connected!";


}

void WSServerOps::onTextMessageReceived(QString message)
{
    QWebSocket *senderSocket = qobject_cast<QWebSocket *>(sender());
    if (senderSocket) {
        qDebug() << "Received message from client:" << message;

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << parseError.errorString();
        } else {
            qDebug() << "Parsed JSON:" << jsonDoc;
        }

        QJsonObject obj = jsonDoc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QString key = it.key();
            QJsonValue value = it.value();
            qDebug() << "Key:" << key << ", Value:" << value;
        }

        QJsonDocument responseDoc = Execute(obj);

        QString jsonString = QString::fromUtf8(responseDoc.toJson(QJsonDocument::Compact));
        sendMessageToClient(senderSocket, jsonString);

    }
}

void WSServerOps::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
        qDebug() << "Client disconnected!";
    }
}


void WSServerOps::sendMessageToClient(QWebSocket *client, const QString &message)
{
    if (client) {
        client->sendTextMessage(message);
        qDebug() << "Sent message to client:" << message;
    }
}

QJsonDocument WSServerOps::Execute(const QJsonObject &instructions)
{
    System *system=new System();
    cout<<"Reading script ..."<<endl;
    string defaulttemppath = QCoreApplication::applicationDirPath().toStdString() + "/../../resources/";
    cout << "Default Template path = " + defaulttemppath +"\n";
    system->SetDefaultTemplatePath(defaulttemppath);
    system->SetWorkingFolder(QFileInfo(modelFile).canonicalPath().toStdString() + "/");

    qDebug()<<"Model File: " << modelFile;
    string settingfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";
    Script scr(modelFile.toStdString(),system);
    cout<<"Executing script ..."<<endl;
    system->CreateFromScript(scr,settingfilename);
    system->SetSilent(false);
    for (QJsonObject::const_iterator entity = instructions.constBegin(); entity != instructions.constEnd(); ++entity)
    {
        if (system->object(entity.key().toStdString()))
        {
            QJsonObject properties = entity.value().toObject();
            qDebug()<<entity.key()<<":"<<entity.value();
            for (QJsonObject::const_iterator property = properties.constBegin(); property != properties.constEnd(); ++property)
            {
                qDebug()<<property.key()<<":"<<property.value();
                system->object(entity.key().toStdString())->SetProperty(property.key().toStdString(), property.value().toString().toStdString());
            }
        }
    }

    cout<<"Solving ..."<<endl;
    system->Solve();
    system->SavetoJson("System.json",system->addedtemplates);
    System system2;

    system2.SetDefaultTemplatePath(defaulttemppath);
    system2.SetWorkingFolder(QFileInfo(workingDirectory).canonicalPath().toStdString() + "/");
    qDebug()<<"Working Folder: "<< QString::fromStdString(system2.GetWorkingFolder());
    system2.ReadSystemSettingsTemplate(settingfilename);

    QFile file("System.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);


    system2.LoadfromJson(doc);
    system2.SavetoScriptFile("Recreated.ohq");
    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + system->OutputFileName() +"'";
    system->GetObservedOutputs().writetofile(system->GetWorkingFolder() + system->OutputFileName());
    return QJsonDocument(system->GetObservedOutputs().toJson());
}




