#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"
#include <QVector> 
#include "Wizard_Entity.h"

class SetVal_Entity : public Wizard_Entity
{
public:
    SetVal_Entity();
    SetVal_Entity(const QJsonObject& jsonobject);
    SetVal_Entity(const SetVal_Entity& WS);
    SetVal_Entity& operator=(const SetVal_Entity& WS);
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
private:


};