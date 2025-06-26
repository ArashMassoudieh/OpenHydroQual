#include "rosettafetcher.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

RosettaFetcher::RosettaFetcher(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &RosettaFetcher::onReplyFinished);
}

void RosettaFetcher::fetchJson(const QUrl& url)
{
    QNetworkRequest request(url);
    m_networkManager->get(request);
}

void RosettaFetcher::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray readdata = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(readdata, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        int errPos = parseError.offset;

        // Display context around error
        int contextSize = 40;
        int start = std::max(0, errPos - contextSize);
        int end = std::min(int(readdata.size()), errPos + contextSize);
        QByteArray context = readdata.mid(start, end - start);

        qWarning() << "Failed to parse JSON at offset:" << errPos;
        qWarning() << "Error message:" << parseError.errorString();
        qWarning() << "Context:" << context;

        emit errorOccurred("Failed to parse JSON: " + parseError.errorString());
        return;
    }

    QJsonObject rootObj = doc.object();
    QStringList keys = rootObj.keys();
    data = parseJsonToMap(rootObj);
    emit dataReady(keys);
}

QMap<QString, QMap<QString, double>> RosettaFetcher::parseJsonToMap(const QJsonObject& rootObj)
{
    QMap<QString, QMap<QString, double>> result;
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        QString textureClass = it.key();
        QJsonObject obj = it.value().toObject();
        QMap<QString, double> properties;

        for (auto propIt = obj.begin(); propIt != obj.end(); ++propIt)
        {
            if (propIt.value().isDouble())
                properties[propIt.key()] = propIt.value().toDouble();
        }

        result[textureClass] = properties;
    }

    return result;
}

