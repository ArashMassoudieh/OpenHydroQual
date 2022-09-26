#include "WizBlockArray.h"

BlockArray::BlockArray():Wizard_Entity()
{

}
BlockArray::BlockArray(const QJsonObject& json_obj)
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
        else if (it.key() == "v_connector_type")
        {
            v_connector_type = json_obj["v_connector_type"].toString();
        }
        else if (it.key() == "h_connector_type")
        {
            h_connector_type = json_obj["h_connector_type"].toString();
        }
        else if (it.key() == "grid_type")
        {
            gridtype = json_obj["grid_type"].toString();
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
            if (key.contains("."))
            {
                key=it.key().split(".")[0];
                if (it.key().split(".")[1]=="h")
                    Arguments_H[key] = arg;
                else if (it.key().split(".")[1]=="v")
                    Arguments_V[key] = arg;
            }
            else
                Arguments[key] = arg;
        }
    }
}
BlockArray::BlockArray(const BlockArray& MB) :Wizard_Entity(MB)
{
    
    name = MB.name;
    type = MB.type;
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    gridtype = MB.gridtype;
    Arguments = MB.Arguments;
    Arguments_H = MB.Arguments_H;
    Arguments_V = MB.Arguments_V;
}
BlockArray& BlockArray::operator=(const BlockArray& MB)
{
    Wizard_Entity::operator=(MB);
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    gridtype = MB.gridtype;
    Arguments_H = MB.Arguments_H;
    Arguments_V = MB.Arguments_V;
    return *this;
}

QString BlockArray::V_ConnectorType()
{
    return v_connector_type;
}
QString BlockArray::H_ConnectorType()
{
    return h_connector_type;
}

QString BlockArray::GridType()
{
    return gridtype; 
}

QStringList BlockArray::GenerateScript(QMap<QString, WizardParameter> *params)
{
    QStringList output; 
    nx = Arguments["n_x"].calc(params);
    ny = Arguments["n_y"].calc(params);
    if (gridtype == "2D-Rect")
    {
        for (int i = 0; i < Arguments["n_x"].calc(params); i++)
        {
            params->operator[]("i").SetValue(QString::number(i));
            for (int j = 0; j < Arguments["n_y"].calc(params); j++)
            {
                params->operator[]("j").SetValue(QString::number(j));
                QString line; 
                line += "create block;";
                line += "type = " + Type();
                line += ", name =" +Name() + "(" + QString::number(i) + ":" + QString::number(j) + ")";
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    if (it.key()!="n_x" && it.key()!="n_y")
                        line += "," + it.key() + "=" + QString::number(it.value().calc(params));
                }
                output << line; 
            }
        }
        for (int i = 0; i < Arguments["n_x"].calc(params)-1; i++)
        {
            params->operator[]("i").SetValue(QString::number(i+0.5));
            for (int j = 0; j < Arguments["n_y"].calc(params); j++)
            {
                params->operator[]("j").SetValue(QString::number(j));
                QString line;
                line += "create link;";
                line += " from = " + Name() + "(" + QString::number(i) + ":" + QString::number(j) + "),";
                line += " to = " + Name() + "(" + QString::number(i+1) + ":" + QString::number(j) + "),";
                line += " type = " + H_ConnectorType();
                line += ", name =" +Name() + "(" + QString::number(i) + ":" + QString::number(j) + ")-(" + QString::number(i+1) + ":" + QString::number(j) + ")";

                for (QMap<QString, Wizard_Argument>::iterator it = Arguments_H.begin(); it != Arguments_H.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params));
                }
                output << line;
            }
        }
        for (int i = 0; i < Arguments["n_x"].calc(params); i++)
        {
            params->operator[]("i").SetValue(QString::number(i));
            for (int j = 0; j < Arguments["n_y"].calc(params)-1; j++)
            {
                params->operator[]("j").SetValue(QString::number(j+0.5));
                QString line;
                line += "create link;";
                line += " from = " + Name() + "(" + QString::number(i) + ":" + QString::number(j) + ")";
                line += ", to = " + Name() + "(" + QString::number(i) + ":" + QString::number(j+1) + ")";
                line += ", type = " + V_ConnectorType();
                line += ", name =" + Name() + "(" + QString::number(i) + ":" + QString::number(j) + ")-(" + QString::number(i) + ":" + QString::number(j+1) + ")";

                for (QMap<QString, Wizard_Argument>::iterator it = Arguments_V.begin(); it != Arguments_V.end(); it++)
                {
                    line += "," + it.key() + "=" + QString::number(it.value().calc(params));
                }
                output << line;
            }
        }
    }
    qDebug() << output; 
    return output; 
    
}
