#ifndef SERVEROPS_H
#define SERVEROPS_H

#include <QObject>

#ifdef signals
#undef signals
#endif
#include "crow.h"
#include <QJsonObject>
#include "cors_middleware.h"

class ServerOps : public QObject
{
    Q_OBJECT
public:
    explicit ServerOps(quint16 port, QObject *parent = nullptr);
    ~ServerOps();
    crow::response StatementReceived(const crow::request& req);
    QJsonDocument Execute(const QJsonObject &instructions = QJsonObject());
private slots:




private:
    crow::App<CORS> app;

};

#endif // WEBSOCKETSERVER_H
