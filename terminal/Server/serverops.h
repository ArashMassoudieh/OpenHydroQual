#ifndef SERVEROPS_H
#define SERVEROPS_H

#include <QObject>

#ifdef signals
#undef signals
#endif
#include "crow.h"
#include <QJsonObject>
#include "cors_middleware.h"
#include <QFile>
#include <QJsonParseError>

class ServerOps : public QObject
{
    Q_OBJECT
public:
    explicit ServerOps(QObject *parent = nullptr);
    void Start(quint16 port);
    ~ServerOps();
    crow::response StatementReceived(const crow::request& req);
    QJsonDocument Execute(const QJsonObject &instructions = QJsonObject());
    void SetModelFile(const QString &modelfile)
    {
        qDebug()<< "Model file was set to '" <<modelfile<<"'";
        modelFile = modelfile;
    }

    void SetWorkingDirectory(const QString &workingdirectory)
    {
        qDebug()<< "Working directory was set to '" <<workingdirectory<<"'";
        workingDirectory = workingdirectory;
    }

private slots:




private:
    crow::App<CORS> app;
    QString modelFile;
    QString workingDirectory;
};

QJsonObject loadJsonObjectFromFile(const QString& filePath);
#endif // WEBSOCKETSERVER_H
