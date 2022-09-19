#ifndef WIZARDPARAMETER_H
#define WIZARDPARAMETER_H

#include <qstring.h>
#include <qjsonobject.h>
#include <QVector2D>

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
    QVector2D Range();
    QStringList Units();
    QStringList ComboItems();
private:
    QString name;
    QString delegate;
    QString question;
    QVector2D range;
    QStringList units;
    QStringList comboitems;
};


#endif // WIZARDPARAMETER_H
