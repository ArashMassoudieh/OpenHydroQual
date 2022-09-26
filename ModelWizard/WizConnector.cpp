#include "WizConnector.h"

Connector::Connector():Wizard_Entity()
{

}
Connector::Connector(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        else if (it.key()=="type")
        {
            type = json_obj["type"].toString();
        }
        else if (it.key()=="connectiontype")
        {
            ConnectorType = ConnectType(json_obj["connectiontype"].toString());
        }
        else if (it.key()=="connectionconfig")
        {
            ConnectorConfig = ConnectConfig(json_obj["connectionconfig"].toString());
        }
        else if (it.key() == "from")
        {
            from = json_obj["from"].toString();
        }
        else if (it.key() == "to")
        {
            from = json_obj["to"].toString();
        }
        else
        {
            QString full_expression_string = it.value().toString();
            QString expression = full_expression_string; 
            QString unit; 
            if (full_expression_string.contains(";"))
            {
                unit = full_expression_string.split(";")[1];
                expression = full_expression_string.split(";")[0];
            }
            QString key = it.key();

            Wizard_Argument arg(expression.toStdString(), unit.toStdString());
            Arguments[key] = arg;
        }
    }
}
Connector::Connector(const Connector& MB) :Wizard_Entity(MB)
{    
    ConnectorConfig = MB.ConnectorConfig;
    ConnectorType = MB.ConnectorType;
    from = MB.from;
    to = MB.to;
}
Connector& Connector::operator=(const Connector& MB)
{
    ConnectorConfig = MB.ConnectorConfig;
    ConnectorType = MB.ConnectorType;
    from = MB.from;
    to = MB.to;
    Wizard_Entity::operator=(MB);
    return *this;
}


QStringList Connector::GenerateScript(QMap<QString, WizardParameter> *params)
{
    QStringList output;    
    QString line;
    line += "create link;";
    line += "type = " + Type();
    line += ", name =" +Name();
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
    {
        line += "," + it.key() + "=" + QString::number(it.value().calc(params));
    }
    output << line;
    qDebug() << output; 
    return output; 
    
}

connector_type ConnectType(QString x)
{
    if (x == "m2o")
        return connector_type::m2o;
    if (x == "m2m")
        return connector_type::m2m;
    if (x == "o2m")
        return connector_type::o2m;
    if (x == "o2o")
        return connector_type::o2o;
}

connector_config ConnectConfig(QString x)
{
    if (x == "d2u")
        return connector_config::d2u;
    if (x == "u2d")
        return connector_config::u2d;
    if (x == "l2r")
        return connector_config::l2r;
    if (x == "r2l")
        return connector_config::r2l;

}
