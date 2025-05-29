/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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

