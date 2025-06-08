#ifndef FILECHECKER_H
#define FILECHECKER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QObject>
#include <QTimer>
#include <QDebug>

class FileChecker : public QObject {
    Q_OBJECT

public:
    explicit FileChecker(QObject *parent = nullptr) : QObject(parent) {
        connect(&manager, &QNetworkAccessManager::finished, this, &FileChecker::onReplyFinished);
    }

    void checkFileExists(const QUrl &url) {
        QNetworkRequest request(url);
        QNetworkReply* reply = manager.head(request);

        // Optional: Timeout protection
        QTimer::singleShot(5000, this, [reply]() {
            if (reply->isRunning()) {
                qWarning() << "Request timed out";
                reply->abort();
            }
        });
    }

signals:
    void fileExists();
    void fileNotFound();
    void checkFailed(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (reply->error() == QNetworkReply::NoError && statusCode.isValid()) {
            int status = statusCode.toInt();
            if (status == 200) {
                emit fileExists();
            } else if (status == 404) {
                emit fileNotFound();
            } else {
                emit checkFailed(QString("Unexpected status code: %1").arg(status));
            }
        } else {
            emit checkFailed(reply->errorString());
        }
        reply->deleteLater();
    }

private:
    QNetworkAccessManager manager;
};


#endif // FILECHECKER_H
