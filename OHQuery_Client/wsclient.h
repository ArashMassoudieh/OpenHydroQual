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
#include "timeseriesloader.h"

class WSClient : public QObject
{
    Q_OBJECT

public:
    explicit WSClient(const QUrl &url, QObject *parent = nullptr);
    void sendJson(const QJsonObject &json);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);

signals:
    void dataReady(const QJsonDocument& data);
    void connected();
    void socketError(QAbstractSocket::SocketError error);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
};

#endif // WSCLIENT_H
