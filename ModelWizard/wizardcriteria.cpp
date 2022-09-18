#include "wizardcriteria.h"

WizardCriteria::WizardCriteria()
{

}

WizardCriteria::WizardCriteria(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="less")
        {
            less = json_obj["less"].toString();
        }
        if (it.key() == "greater")
        {
            greater = json_obj["greater"].toString();
        }
        if (it.key() == "errormessage")
        {
            errormessage = json_obj["errormessage"].toString();
        }
    }
}


WizardCriteria::WizardCriteria(const WizardCriteria &WPG){
    less = WPG.less;
    greater = WPG.greater;
    name = WPG.name;
    errormessage = WPG.errormessage;
}


WizardCriteria& WizardCriteria::operator=(const WizardCriteria& WPG)
{
    less = WPG.less;
    greater = WPG.greater;
    name = WPG.name;
    errormessage = WPG.errormessage;
    return *this;
}
