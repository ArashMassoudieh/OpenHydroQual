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