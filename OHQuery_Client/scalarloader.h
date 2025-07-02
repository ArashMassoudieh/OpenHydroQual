#ifndef SCALARLOADER_H
#define SCALARLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

class ScalarLoader : public QObject
{
    Q_OBJECT

public:
    explicit ScalarLoader(QObject* parent = nullptr);

    void load(const QUrl& url);

signals:
    void scalarsLoaded(QMap<QString, double> data);
    void loadFailed(QString error);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* manager;
};

#endif // SCALARLOADER_H
