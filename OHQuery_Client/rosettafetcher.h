#ifndef ROSETTAFETCHER_H
#define ROSETTAFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QJsonObject>

class RosettaFetcher : public QObject
{
    Q_OBJECT

public:
    explicit RosettaFetcher(QObject* parent = nullptr);
    void fetchJson(const QUrl& url);
    QMap<QString, QMap<QString, double>> parseJsonToMap(const QJsonObject& data);
    QMap<QString, QMap<QString, double>> getData() {return data;}
    QStringList getTextureClasses() const {return data.keys();}
signals:
    void dataReady(const QStringList& keys);
    void errorOccurred(const QString& message);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
    QMap<QString, QMap<QString, double>> data;
};

#endif // ROSETTAFETCHER_H
