#ifndef WSCLIENT_H
#define WSCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonObject>
#include <QMap>
#include <QString>

struct TimeSeries {
    QList<double> t;
    QList<double> value;
};

using TimeSeriesMap = QMap<QString, TimeSeries>;

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
private:
    QWebSocket m_webSocket;
    QUrl m_url;
};

#endif // WSCLIENT_H
