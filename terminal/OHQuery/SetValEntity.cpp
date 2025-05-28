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


#include "SetValEntity.h"

SetVal_Entity::SetVal_Entity() :Wizard_Entity()
{

}
SetVal_Entity::SetVal_Entity(const QJsonObject& json_obj) :Wizard_Entity(json_obj)
{
    
}
SetVal_Entity::SetVal_Entity(const SetVal_Entity& MB) :Wizard_Entity(MB)
{

}
SetVal_Entity& SetVal_Entity::operator=(const SetVal_Entity& MB)
{
    Wizard_Entity::operator=(MB);
   
    return *this;
}

QStringList SetVal_Entity::GenerateScript(QMap<QString, WizardParameter>* params)
{
    QStringList out;
    QString line;
    line += "setvalue; ";
    int i = 0; 
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
    {
        if (it.key() != "name")
        {
            line += (i == 0 ? "" : ",") + it.key() + "=" + it.value().Calc(params);
            i++;
        }
    }
    out << line;
    return out;

}
