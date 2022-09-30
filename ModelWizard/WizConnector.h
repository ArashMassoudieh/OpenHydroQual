#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include <QVector> 
#include "Wizard_Entity.h"

enum class connector_type {m2o,o2m,o2o,m2m};
enum class connector_config {d2u, u2d,l2r, r2l, d2d, u2u, l2l,r2r};

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
    Wizard_Entity *GetFromEntity();
    Wizard_Entity *GetToEntity();
private:
    connector_type ConnectorType;
    connector_config ConnectorConfig;
    QString from;
    QString to;

};

connector_type ConnectType(QString x);
connector_config ConnectConfig(QString x);
