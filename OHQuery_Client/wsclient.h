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


#ifndef WSCLIENT_H
#define WSCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QTimer>

class WSClient : public QObject
{
    Q_OBJECT

public:
    explicit WSClient(const QUrl &url, QObject *parent = nullptr);
    void sendJson(const QJsonObject &json);
    bool HasConnectedOnce() {return m_hasConnectedOnce;}
private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void reconnect();
signals:
    void dataReady(const QJsonDocument& data);
    void connected();
    void disconnected();
    void socketError(QAbstractSocket::SocketError error);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    QTimer m_reconnectTimer;
    QTimer m_pingTimer;
    bool m_hasConnectedOnce = false;
};

#endif // WSCLIENT_H
