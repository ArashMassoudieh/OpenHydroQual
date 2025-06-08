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


#ifndef WSSERVEROP_H
#define WSSERVEROP_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>

class System;

class WSServerOps : public QObject
{
    Q_OBJECT
public:
#ifdef HTTPS
    explicit WSServerOps(const QString &certPath, const QString &keyPath, QObject *parent = nullptr);
#else
    explicit WSServerOps(QObject *parent = nullptr);
#endif
    void Start(quint16 port);
    ~WSServerOps();
    QJsonObject Execute(System *sys);
    void SetModelFile(const QString &modelfile)
    {
        qDebug()<< "Model file was set to '" <<modelfile<<"'";
        modelFile = modelfile;
    }

    void SetWorkingDirectory(const QString &workingdirectory)
    {
        qDebug()<< "Working directory was set to '" <<workingdirectory<<"'";
        workingDirectory = workingdirectory;
    }
    QJsonDocument SendModelTemplate(const QString &TemplateName);
private slots:
    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();
    void sendMessageToClient(QWebSocket *client, const QString &message);


private:
    QWebSocketServer *m_server;
    QMap<QString, QWebSocket*> m_clients;
    QMap<QString, QString> client_state;
    QMap<QString, QString> message_to_be_sent;
    QString m_certPath;
    QString m_keyPath;
    QString modelFile;
    QString workingDirectory;
    QString TemplateFile_Fullpath;


};

#endif // WEBSOCKETSERVER_H


