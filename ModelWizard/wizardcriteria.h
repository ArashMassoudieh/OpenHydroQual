#ifndef WIZARDCRITERIA_H
#define WIZARDCRITERIA_H

#include <QString>
#include <QJsonObject>

class WizardCriteria
{
public:
    WizardCriteria();
    WizardCriteria(const QJsonObject& jsonobj);
    WizardCriteria(const WizardCriteria &WPG);
    WizardCriteria& operator=(const WizardCriteria& WPG);
    QString Name() {return name;}
    QString ErrorMessage() {return errormessage;}
    QString Less() {return less;}
    QString Greater() {return greater;}
private:
    QString less;
    QString greater;
    QString name;
    QString errormessage;

};

#endif // WIZARDCRITERIA_H
