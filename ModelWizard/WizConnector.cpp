#include "WizConnector.h"
#include "Wizard_Script.h"
#include <QDebug>

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
            to = json_obj["to"].toString();
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
    
    
    if (ConnectorType == connector_type::m2m)
    {   if (ConnectorConfig==connector_config::u2d || ConnectorConfig==connector_config::d2u)
        {
            if (static_cast<BlockArray*>(GetFromEntity())->Nx() != static_cast<BlockArray*>(GetToEntity())->Nx())
            {
                qDebug() << "Number of columns in the connected arrays must be the same";
                return QStringList();
            }
            else
            {
                BlockArray *fromBlock = static_cast<BlockArray*>(GetFromEntity());
                BlockArray *toBlock = static_cast<BlockArray*>(GetToEntity());
                for (int i = 0; i < static_cast<BlockArray*>(GetFromEntity())->Nx(); i++)
                {
                    QString line;
                    line += "create link;";
                    if (ConnectorConfig==connector_config::u2d) {
                        line += " from = " + fromBlock->BlockName(i,fromBlock->Ny()-1);
                        line += ", to = " + toBlock->BlockName(i,0);
                    }
                    else if (ConnectorConfig==connector_config::d2u) {
                        line += " from = " + fromBlock->BlockName(i,0);
                        line += ", to = " + toBlock->BlockName(i,toBlock->Ny()-1);
                    }
                    else if (ConnectorConfig==connector_config::d2d) {
                        line += " from = " + fromBlock->BlockName(i,fromBlock->Ny()-1);
                        line += ", to = " + toBlock->BlockName(i,toBlock->Ny()-1);
                    }
                    else if (ConnectorConfig==connector_config::u2u) {
                        line += " from = " + fromBlock->BlockName(i,0);
                        line += ", to = " + toBlock->BlockName(i,0);
                    }
                    line += ", type = " + Type();
                    line += ", name =" + Name() + "(" + QString::number(i) + ")";
                    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                    {
                        line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                    }
                    output << line;
                }
            }

        }
        else if (ConnectorConfig == connector_config::l2r || ConnectorConfig == connector_config::r2l)
        {
            if (static_cast<BlockArray*>(GetFromEntity())->Ny() != static_cast<BlockArray*>(GetToEntity())->Ny())
            {
                qDebug() << "Number of rows in the connected arrays must be the same";
                return QStringList();
            }
            else
            {
                BlockArray* fromBlock = static_cast<BlockArray*>(GetFromEntity());
                BlockArray* toBlock = static_cast<BlockArray*>(GetToEntity());
                for (int j = 0; j < static_cast<BlockArray*>(GetFromEntity())->Ny(); j++)
                {
                    QString line;
                    line += "create link;";
                    if (ConnectorConfig == connector_config::l2r) {
                        line += " from = " + fromBlock->BlockName(fromBlock->Nx() - 1, j);
                        line += ", to = " + toBlock->BlockName(0, j);
                    }
                    else if (ConnectorConfig == connector_config::r2l) {
                        line += " from = " + fromBlock->BlockName(0, j);
                        line += ", to = " + toBlock->BlockName(toBlock->Nx() - 1, j);
                    }
                    else if (ConnectorConfig == connector_config::r2r) {
                        line += " from = " + fromBlock->BlockName(0, j);
                        line += ", to = " + toBlock->BlockName(0, j);
                    }
                    else if (ConnectorConfig == connector_config::l2l) {
                        line += " from = " + fromBlock->BlockName(fromBlock->Nx() - 1, j);
                        line += ", to = " + toBlock->BlockName(toBlock->Nx() - 1, j);
                    }
                    line += ", type = " + Type();
                    line += ", name =" + Name()+ "(" + QString::number(j) + ")";
                    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                    {
                        line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                    }
                    output << line;
                }
            }
        }
    }
    if (ConnectorType == connector_type::m2o)
    {
        if (ConnectorConfig == connector_config::u2d || ConnectorConfig == connector_config::d2u)
        {
            
            BlockArray* fromBlock = static_cast<BlockArray*>(GetFromEntity());
            SingleBlock* toBlock = static_cast<SingleBlock*>(GetToEntity());
            for (int i = 0; i < static_cast<BlockArray*>(GetFromEntity())->Nx(); i++)
            {
                QString line;
                line += "create link;";
                if (ConnectorConfig == connector_config::u2d || ConnectorConfig == connector_config::d2d) {
                    line += " from = " + fromBlock->BlockName(i, fromBlock->Ny() - 1);
                    line += ", to = " + toBlock->Name();
                }
                else if (ConnectorConfig == connector_config::d2u || ConnectorConfig == connector_config::u2u) {
                    line += " from = " + fromBlock->BlockName(i, 0);
                    line += ", to = " + toBlock->Name();
                }

                line += ", type = " + Type();
                line += ", name =" + Name()+ "(" + QString::number(i) + ")";
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                }
                output << line;
            }
        }
        else if (ConnectorConfig == connector_config::r2l || ConnectorConfig == connector_config::l2r)
        {

            BlockArray* fromBlock = static_cast<BlockArray*>(GetFromEntity());
            SingleBlock* toBlock = static_cast<SingleBlock*>(GetToEntity());
            for (int j = 0; j < static_cast<BlockArray*>(GetFromEntity())->Ny(); j++)
            {
                QString line;
                line += "create link;";
                if (ConnectorConfig == connector_config::l2r || ConnectorConfig == connector_config::l2l) {
                    line += " from = " + fromBlock->BlockName(fromBlock->Nx() - 1, j);
                    line += ", to = " + toBlock->Name();
                }
                else if (ConnectorConfig == connector_config::r2l || ConnectorConfig == connector_config::r2r) {
                    line += " from = " + fromBlock->BlockName(0, j);
                    line += ", to = " + toBlock->Name();
                }
                line += ", type = " + Type();
                line += ", name =" + Name();
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                }
                output << line;
            }
        }
    }
    if (ConnectorType == connector_type::o2m)
    {
        if (ConnectorConfig == connector_config::u2d || ConnectorConfig == connector_config::d2u)
        {
            
            SingleBlock* fromBlock = static_cast<SingleBlock*>(GetFromEntity());
            BlockArray* toBlock = static_cast<BlockArray*>(GetToEntity());
            for (int i = 0; i < static_cast<BlockArray*>(GetToEntity())->Nx(); i++)
            {
                QString line;
                line += "create link;";
                if (ConnectorConfig == connector_config::u2d || ConnectorConfig == connector_config::d2d) {
                    line += " from = " + fromBlock->Name();
                    line += ", to = " + toBlock->BlockName(i, 0);
                }
                else if (ConnectorConfig == connector_config::d2u || ConnectorConfig == connector_config::u2u) {
                    line += " from = " + fromBlock->Name();
                    line += ", to = " + toBlock->BlockName(i, toBlock->Ny() - 1);
                }
                line += ", type = " + Type();
                line += ", name =" + Name()+ "(" + QString::number(i) + ")";
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                }
                output << line;
            }
        }
        else if (ConnectorConfig == connector_config::l2r || ConnectorConfig == connector_config::r2l)
        {
            SingleBlock* fromBlock = static_cast<SingleBlock*>(GetFromEntity());
            BlockArray* toBlock = static_cast<BlockArray*>(GetToEntity());
            for (int j = 0; j < static_cast<BlockArray*>(GetFromEntity())->Ny(); j++)
            {
                QString line;
                line += "create link;";
                if (ConnectorConfig == connector_config::l2r || ConnectorConfig == connector_config::r2r) {
                    line += " from = " + fromBlock->Name();
                    line += ", to = " + toBlock->BlockName(0, j);
                }
                else if (ConnectorConfig == connector_config::r2l || ConnectorConfig == connector_config::l2l) {
                    line += " from = " + fromBlock->Name();
                    line += ", to = " + toBlock->BlockName(toBlock->Nx() - 1, j);
                }
                line += ", type = " + Type();
                line += ", name =" + Name()+ "(" + QString::number(j) + ")";
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
                }
                output << line;
            }
        }

    }
    if (ConnectorType == connector_type::o2o)
    {
        SingleBlock* fromBlock = static_cast<SingleBlock*>(GetFromEntity());
        SingleBlock* toBlock = static_cast<SingleBlock*>(GetToEntity());

        QString line;
        line += "create link;";

        line += " from = " + fromBlock->Name();
        line += ", to = " + toBlock->Name();

        line += ", type = " + Type();
        line += ", name =" + Name();
        for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
        {
            line += "," + it.key() + "=" + QString::number(it.value().calc(params))+it.value().UnitText();
        }
        output << line;
        
    }
    qDebug() << output; 
    return output; 
    
}

Wizard_Entity *Connector::GetFromEntity()
{
    return GetWizardScript()->FindEntity(from);
}

Wizard_Entity *Connector::GetToEntity()
{
    return GetWizardScript()->FindEntity(to);
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
    if (x == "u2u")
        return connector_config::u2u;
    if (x == "d2d")
        return connector_config::d2d;
    if (x == "r2r")
        return connector_config::r2r;
    if (x == "l2l")
        return connector_config::l2l;

}
