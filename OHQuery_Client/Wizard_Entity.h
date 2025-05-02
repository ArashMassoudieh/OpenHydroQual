#pragma once
#include <qstring.h>
#include "Wizard_Argument.h"
#include "qjsonarray.h"
#include <QMap>

class WizardScript;

class Wizard_Entity
{
public:
    Wizard_Entity();
    Wizard_Entity(const QJsonObject& jsonobject);
    Wizard_Entity(const Wizard_Entity& WS);
    Wizard_Entity& operator=(const Wizard_Entity& WS);
    QString Name();
    QString Type();
    QString Entity();
    virtual QStringList GenerateScript(QMap<QString, WizardParameter>* params);
    void SetWizardScript(WizardScript* wizscript) { wiz_script = wizscript; }
    WizardScript* GetWizardScript() { return wiz_script; }
    
protected:
    QMap<QString, Wizard_Argument> Arguments;
    QString name;
    QString type;
    QString entity; 
private:
    WizardScript* wiz_script; 
    
};

