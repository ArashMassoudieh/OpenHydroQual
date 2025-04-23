#include "apiclient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

ApiClient::ApiClient(const QUrl& endpoint, QObject* parent)
    : QObject(parent), url(endpoint)
{
    connect(&manager, &QNetworkAccessManager::finished, this, &ApiClient::handleReply);
}

void ApiClient::fetchAllSeries(const QJsonObject& requestData)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(requestData);
    QByteArray postData = doc.toJson();

    manager.post(request, postData);
}

void ApiClient::handleReply(QNetworkReply* reply)
{
    TimeSeriesMap seriesMap;

    if (reply->error() != QNetworkReply::NoError) {
        emit requestFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit requestFailed("JSON parse error: " + parseError.errorString());
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

