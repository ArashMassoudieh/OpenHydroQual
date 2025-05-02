#ifndef WIZARDCRITERIA_H
#define WIZARDCRITERIA_H

#include <QString>
#include <QJsonObject>
#include "wizardparameter.h"
#include "Wizard_Argument.h"

class WizardCriteria
{
public:
    WizardCriteria();
    WizardCriteria(const QJsonObject& jsonobj);
    WizardCriteria(const WizardCriteria &WPG);
    WizardCriteria& operator=(const WizardCriteria& WPG);
    QString Name() {return name;}
    QString ErrorMessage() {return errormessage;}
    Wizard_Argument Less() {return less;}
    Wizard_Argument Greater() {return greater;}
    bool Check(QMap<QString, WizardParameter> *Parameters);
private:
    Wizard_Argument less;
    Wizard_Argument greater;
    QString name;
    QString errormessage;

};

#endif // WIZARDCRITERIA_H
