#ifndef TIMESERIESLOADER_H
#define TIMESERIESLOADER_H
// TimeSeriesLoader.h
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

struct TimeSeries {
    QList<double> t;
    QList<double> value;
};

using TimeSeriesMap = QMap<QString, TimeSeries>;

class TimeSeriesLoader : public QObject
{
    Q_OBJECT

public:
    explicit TimeSeriesLoader(QObject* parent = nullptr);

    void load(const QUrl& url);

signals:
    void timeSeriesLoaded(QMap<QString, TimeSeries> data);
    void loadFailed(QString error);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* manager;
};
#endif TIMESERIESLOADER_H
