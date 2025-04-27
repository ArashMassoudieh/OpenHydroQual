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
    emit connected();
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
    if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(json);
        QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        m_webSocket.sendTextMessage(jsonString);
        qDebug() << "Sent message:" << jsonString;
    } else {
        qWarning() << "WebSocket not connected!";
    }
}



