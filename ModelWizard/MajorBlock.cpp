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
        else
        {
            Wizard_Assigned_Value arg;
            if (it.value().toString().contains("param["))
            {
                arg.SetAsParam(true);
                arg.SetParam(it.value().toString().split("[")[1].split("]")[0]);
            }
            else
                arg.SetValue(it.value().toString());
        }
    }
}
MajorBlock::MajorBlock(const MajorBlock& MB)
{
    name = MB.name;
    type = MB.type;
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    Arguments = MB.Arguments;
}
MajorBlock& MajorBlock::operator=(const MajorBlock& MB)
{
    name = MB.name;
    type = MB.type;
    v_connector_type = MB.v_connector_type;
    h_connector_type = MB.h_connector_type;
    Arguments = MB.Arguments;
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
