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


class WSServerOps : public QObject
{
    Q_OBJECT
public:
    explicit WSServerOps(QObject *parent = nullptr);
    void Start(quint16 port);
    ~WSServerOps();
    QJsonDocument Execute(const QJsonObject &instructions);
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
private slots:
    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();
    void sendMessageToClient(QWebSocket *client, const QString &message);



private:
    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
    QString modelFile;
    QString workingDirectory;

};

#endif // WEBSOCKETSERVER_H


