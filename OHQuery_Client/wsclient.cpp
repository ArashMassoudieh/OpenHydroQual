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


#include "wsclient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QSettings>


WSClient::WSClient(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WSClient::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WSClient::onTextMessageReceived);

    #if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    connect(&m_webSocket, &QWebSocket::errorOccurred, this, &WSClient::socketError);
    #else
    //connect(&m_webSocket.socket(), &QAbstractSocket::errorOccurred, this, &WSClient::socketError);
    #endif

    QString clientId;

    QSettings settings("OpenHydroQual", "Client");
    clientId = settings.value("client_id").toString();
    if (clientId.isEmpty()) {
        clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue("client_id", clientId);
    }

    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, [this](QAbstractSocket::SocketError error) {
                emit socketError(error);

            });

    m_pingTimer.setInterval(10000);  // every 10 seconds
    connect(&m_pingTimer, &QTimer::timeout, [&]() {
        if (m_webSocket.state() == QAbstractSocket::ConnectedState)
            m_webSocket.ping();
    });
    m_pingTimer.start();

    // Setup reconnect timer
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WSClient::reconnect);

    m_webSocket.open(m_url);
}

void WSClient::onConnected()
{
    qDebug() << "WebSocket connected!";
    emit connected();

    // Send a special reconnect message if this is not the first connection
    if (m_hasConnectedOnce) {
        QJsonObject reconnectMsg;
        QSettings settings("OpenHydroQual", "Client");
        reconnectMsg["client_id"] = settings.value("client_id").toString();
        reconnectMsg["Reconnected"] = true;

        QJsonDocument doc(reconnectMsg);
        m_webSocket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));

        qDebug() << "[WSClient] Sent Reconnected message to server";
    } else {
        m_hasConnectedOnce = true;
    }
}

void WSClient::onDisconnected()
{
    emit disconnected();
    qDebug() << "WebSocket disconnected. Reconnecting in 3 seconds...";
    m_reconnectTimer.start(3000);  // 3-second delay

}

void WSClient::reconnect()
{
    qDebug() << "Reconnecting WebSocket to:" << m_url.toString();
    m_webSocket.open(m_url);
}

void WSClient::onTextMessageReceived(const QString &message)
{
    //qDebug() << "Received response:" << message;
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug()<<"JSON parse error: " + parseError.errorString();
        return;
    }

    emit dataReady(jsonDoc);
}

void WSClient::sendJson(const QJsonObject &json)
{
    QJsonObject enrichedJson = json;

    // Inject persistent client_id
    QSettings settings("OpenHydroQual", "Client");
    QString clientId = settings.value("client_id").toString();
    if (!clientId.isEmpty())
        enrichedJson["client_id"] = clientId;

    QJsonDocument doc(enrichedJson);
    QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
        m_webSocket.sendTextMessage(jsonString);
        qDebug() << "[WSClient] Sent message:" << jsonString;
    } else {
        qWarning() << "[WSClient] WebSocket not connected! Dropping message:" << jsonString;

        // Optional: Queue and resend later
        // pendingMessages.enqueue(jsonString);
    }
}



