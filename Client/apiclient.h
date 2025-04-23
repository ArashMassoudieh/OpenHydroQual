#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QMap>
#include <QString>

struct TimeSeries {
    QList<double> t;
    QList<double> value;
};

using TimeSeriesMap = QMap<QString, TimeSeries>;

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(const QUrl& endpoint, QObject* parent = nullptr);
    void fetchAllSeries(const QJsonObject& requestData);

signals:
    void dataReady(const TimeSeriesMap& data);
    void requestFailed(const QString& error);

private slots:
    void handleReply(QNetworkReply* reply);

private:
    QNetworkAccessManager manager;
    QUrl url;
};

#endif // APICLIENT_H
