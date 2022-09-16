#ifndef WIZARDPARAMETER_H
#define WIZARDPARAMETER_H

#include <qstring.h>
#include <qjsonobject.h>

class WizardParameter
{
public:
    WizardParameter();
    WizardParameter(const QJsonObject& jsonobject);
    WizardParameter(const WizardParameter& WS);
    WizardParameter& operator=(const WizardParameter& WS);
    QString Name();
    QString Delegate();
    QString Question();
private:
    QString name;
    QString delegate;
    QString question;
};


#endif // WIZARDPARAMETER_H
