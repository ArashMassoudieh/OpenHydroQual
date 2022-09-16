#include "wizardparameter.h"

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
    }
}
WizardParameter::WizardParameter(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
}
WizardParameter& WizardParameter::operator=(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
    return *this;
}
QString WizardParameter::Name()
{
    return name;
}
QString WizardParameter::Delegate()
{
    return delegate;
}
QString WizardParameter::Question()
{
    return question;
}
