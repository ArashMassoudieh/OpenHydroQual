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
