// TimeSeriesLoader.cpp
#include "timeseriesloader.h"
#include <QDebug>

TimeSeriesLoader::TimeSeriesLoader(QObject* parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

void TimeSeriesLoader::load(const QUrl& url)
{
    QNetworkRequest request(url);
    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &TimeSeriesLoader::onReplyFinished);
}

void TimeSeriesLoader::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QMap<QString, TimeSeries> tsMap;

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
    for (const QString& key : obj.keys()) {
        QJsonObject tsObj = obj[key].toObject();
        QJsonArray tArray = tsObj["t"].toArray();
        QJsonArray valArray = tsObj["value"].toArray();

        TimeSeries ts;
        for (int i = 0; i < tArray.size() && i < valArray.size(); ++i) {
            ts.t.append(tArray[i].toDouble());
            ts.value.append(valArray[i].toDouble());
        }
        tsMap[key] = ts;
    }

    emit timeSeriesLoaded(tsMap);
    reply->deleteLater();
}
