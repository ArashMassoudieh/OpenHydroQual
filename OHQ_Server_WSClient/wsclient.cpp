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


WSClient::WSClient(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WSClient::onTextMessageReceived);
    m_webSocket.open(m_url);
}

void WSClient::onConnected()
{
    qDebug() << "WebSocket connected!";
}

void WSClient::onTextMessageReceived(const QString &message)
{
    TimeSeriesMap seriesMap;

    qDebug() << "Received response:" << message;QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug()<<"JSON parse error: " + parseError.errorString();
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    for (const QString& key : rootObj.keys()) {
        QJsonObject variable = rootObj[key].toObject();
        QJsonArray tArr = variable["t"].toArray();
        QJsonArray valArr = variable["value"].toArray();

        TimeSeries ts;
        for (int i = 0; i < tArr.size(); ++i) {
            ts.t.append(tArr[i].toDouble());
            ts.value.append(valArr[i].toDouble());
        }

        seriesMap[key] = ts;
    }
    emit dataReady(seriesMap);
}

void WSClient::sendJson(const QJsonObject &json)
{
    if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(json);
        QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        m_webSocket.sendTextMessage(jsonString);
        qDebug() << "Sent message:" << jsonString;
    } else {
        qWarning() << "WebSocket not connected!";
    }
}



