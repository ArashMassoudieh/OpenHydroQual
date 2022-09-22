#include "wizardparameter.h"
#include "QJsonArray"

WizardParameter::WizardParameter()
{

}


WizardParameter::WizardParameter(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="delegate")
        {
            delegate = json_obj["delegate"].toString();
        }
        if (it.key() == "question")
        {
            question = json_obj["question"].toString();
        }
        if (it.key() == "unit")
        {
            units.clear();
            units=it.value().toString().split(";");
        }
        if (it.key() == "range")
        {
            QJsonArray items = json_obj["range"].toArray();
            range[0]=items[0].toString().toInt();
            range[1]=items[1].toString().toInt();
        }
        if (it.key() == "comboitems")
        {
            comboitems.clear();
            QJsonArray items = json_obj["combotems"].toArray();
            for (int i=0; i<items.size(); i++)
            {
                comboitems = it.value().toString().split(";");
            }
        }

    }
}
WizardParameter::WizardParameter(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
    range = WS.range;
    comboitems = WS.comboitems;
    units = WS.units;
}
WizardParameter& WizardParameter::operator=(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
    range = WS.range;
    comboitems = WS.comboitems;
    units = WS.units;
    return *this;
}
QString WizardParameter::Name()
{
    return name;
}
void WizardParameter::SetName(const QString &_name)
{
    name = _name; 
}
QString WizardParameter::Delegate()
{
    return delegate;
}
QString WizardParameter::Question()
{
    return question;
}

QVector2D WizardParameter::Range()
{
    return range;
}
QStringList WizardParameter::Units()
{
    return units;
}
QStringList WizardParameter::ComboItems()
{
    return comboitems;
}
