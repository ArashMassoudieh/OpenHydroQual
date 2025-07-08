#include "scalarloader.h"
#include "qnetworkrequest.h"
#include <QDebug>

ScalarLoader::ScalarLoader(QObject* parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

void ScalarLoader::load(const QUrl& url)
{
    QNetworkRequest request(url);
    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ScalarLoader::onReplyFinished);
}

void ScalarLoader::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QMap<QString, double> scalarMap;

    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        emit loadFailed("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit loadFailed("JSON parse error: " + parseError.errorString());
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().isDouble()) {
            scalarMap.insert(it.key(), it.value().toDouble());
        }
    }

    emit scalarsLoaded(scalarMap);
    reply->deleteLater();
}
