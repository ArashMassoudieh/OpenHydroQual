#ifndef SERVEROPS_H
#define SERVEROPS_H

#include <QObject>

#ifdef signals
#undef signals
#endif
#include "crow.h"

class ServerOps : public QObject
{
    Q_OBJECT
public:
    explicit ServerOps(quint16 port, QObject *parent = nullptr);
    ~ServerOps();
    crow::response StatementReceived(const crow::request& req);
    QJsonDocument Execute();
private slots:




private:
    crow::SimpleApp app;

};

#endif // WEBSOCKETSERVER_H
