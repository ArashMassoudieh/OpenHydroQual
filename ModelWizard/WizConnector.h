#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include <QVector> 
#include "Wizard_Entity.h"

enum class connector_type {m2o,o2m,o2o,m2m};
enum class connector_config {d2u, u2d,l2r, r2l};

class Connector: public Wizard_Entity
{
public:
    Connector();
    Connector(const QJsonObject& jsonobject);
    Connector(const Connector& WS);
    Connector& operator=(const Connector& WS);
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
    connector_type GetConnectorType() {return ConnectorType;}
    connector_config GetConnectorConfig() {return ConnectorConfig;}
    QString From() {return from;}
    QString To() {return to;}
private:
    connector_type ConnectorType;
    connector_config ConnectorConfig;
    QString from;
    QString to;

};

connector_type ConnectType(QString x)
{
    if (x=="m2o")
        return connector_type::m2o;
    if (x=="m2m")
        return connector_type::m2m;
    if (x=="o2m")
        return connector_type::o2m;
    if (x=="o2o")
        return connector_type::o2o;
}

connector_config ConnectConfig(QString x)
{
    if (x=="d2u")
        return connector_config::d2u;
    if (x=="u2d")
        return connector_config::u2d;
    if (x=="l2r")
        return connector_config::l2r;
    if (x=="r2l")
        return connector_config::r2l;

}
