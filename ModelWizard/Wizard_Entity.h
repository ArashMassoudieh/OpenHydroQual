#pragma once
#include <qstring.h>
#include "Wizard_Argument.h"
#include "qjsonarray.h"
class Wizard_Entity
{
public:
    Wizard_Entity();
    Wizard_Entity(const QJsonObject& jsonobject);
    Wizard_Entity(const Wizard_Entity& WS);
    Wizard_Entity& operator=(const Wizard_Entity& WS);
    QString Name();
    QString Type();
    virtual QStringList GenerateScript(QMap<QString, WizardParameter>* params);
protected:
    QMap<QString, Wizard_Argument> Arguments;
    QString name;
    QString type;
    QString entity; 
private:
    
};

