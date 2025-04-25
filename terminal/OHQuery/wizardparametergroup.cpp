#include "wizardparametergroup.h"
#include "QJsonArray"

WizardParameterGroup::WizardParameterGroup()
{

}

WizardParameterGroup::WizardParameterGroup(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="description")
        {
            description = json_obj["description"].toString();
        }
        if (it.key() == "parameters")
        {
            QJsonArray params = json_obj["parameters"].toArray();
            for (int i=0; i<params.count(); i++)
            {
                parameters<<params[i].toString();
            }
        }
        if (it.key() == "criteria")
        {
            QJsonArray crits = json_obj["criteria"].toArray();
            for (int i=0; i<crits.count(); i++)
            {
                criteria<<WizardCriteria(crits[i].toObject());
            }
        }
    }
}

WizardParameterGroup::WizardParameterGroup(const WizardParameterGroup &WPG)
{
    parameters = WPG.parameters;
    criteria = WPG.criteria;
    name = WPG.name;
    description = WPG.description;
}

WizardParameterGroup& WizardParameterGroup::operator=(const WizardParameterGroup& WPG)
{
    parameters = WPG.parameters;
    criteria = WPG.criteria;
    name = WPG.name;
    description = WPG.description;
    return *this;
}

QStringList WizardParameterGroup::CheckCriteria(QMap<QString, WizardParameter> *Parameters)
{
    QStringList Errors;
    for (int i=0; i<criteria.count(); i++)
    {
        if (!criteria[i].Check(Parameters))
        {
            Errors.append(criteria[i].ErrorMessage());
        }
    }
    return Errors;

}
