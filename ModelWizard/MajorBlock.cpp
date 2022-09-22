#include "MajorBlock.h"

MajorBlock::MajorBlock()
{

}
MajorBlock::MajorBlock(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="type")
        {
            type = json_obj["type"].toString();
        }
        if (it.key() == "v_connector_type")
        {
            v_connector_type = json_obj["v_connector_type"].toString();
        }
        if (it.key() == "h_connector_type")
        {
            h_connector_type = json_obj["h_connector_type"].toString();
        }
        if (it.key() == "grid_type")
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
            Wizard_Argument arg(expression.toStdString(), unit.toStdString());
            Arguments[it.key()] = arg; 
        }
    }
}
MajorBlock::MajorBlock(const MajorBlock& MB)
{
    name = MB.name;
    type = MB.type;
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    gridtype = MB.gridtype;
    Arguments = MB.Arguments;
}
MajorBlock& MajorBlock::operator=(const MajorBlock& MB)
{
    name = MB.name;
    type = MB.type;
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    Arguments = MB.Arguments;
    gridtype = MB.gridtype;
    return *this;
}
QString MajorBlock::Name()
{
    return name;
}

QString MajorBlock::Type()
{
    return type;
}
QString MajorBlock::V_ConnectorType()
{
    return v_connector_type;
}
QString MajorBlock::H_ConnectorType()
{
    return h_connector_type;
}

QString MajorBlock::GridType()
{
    return gridtype; 
}

QStringList MajorBlock::GenerateScript(QMap<QString, WizardParameter> *params)
{
    QStringList output; 
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
                for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
                {
                    if (it.key() == "name")
                    {
                        line += "," + it.key() + "=" +Name() + "(" + QString::number(i) + ":" + QString::number(j) + ")";
                    }
                    else
                        line += "," + it.key() + "=" + QString::number(it.value().calc(params));
                }
                output << line; 
            }
        }
    }
    qDebug() << output; 
    return output; 
    
}
