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


#include "Wizard_Entity.h"
Wizard_Entity::Wizard_Entity() 
{

}
Wizard_Entity::Wizard_Entity(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it != json_obj.end(); it++)
    {
        if (it.key() == "name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key() == "entity")
        {
            entity = json_obj["entity"].toString();
        }
        if (it.key() == "type")
        {
            type = json_obj["type"].toString();
        }
        else
        {
            QString full_expression_string = it.value().toString();
            QString expression = full_expression_string;
            QString unit;
            if (full_expression_string.contains(";"))
            {
                unit = full_expression_string.split(";")[1];
                expression = full_expression_string.split(";")[0];
            }
            QString key = it.key();

            Wizard_Argument arg(expression.toStdString(), unit.toStdString());
            Arguments[key] = arg;
        }
    }
}
Wizard_Entity::Wizard_Entity(const Wizard_Entity& MB) 
{
    name = MB.name;
    type = MB.type;
    entity = MB.entity; 
    Arguments = MB.Arguments;
    
}
Wizard_Entity& Wizard_Entity::operator=(const Wizard_Entity& MB)
{
    name = MB.name;
    type = MB.type;
    entity = MB.entity;
    Arguments = MB.Arguments;
    return *this;
}

QString Wizard_Entity::Name()
{
    return name;
}

QString Wizard_Entity::Type()
{
    return type;
}

QString Wizard_Entity::Entity()
{
    return entity;
}

QStringList Wizard_Entity::GenerateScript(QMap<QString, WizardParameter>* params)
{
    QStringList out; 
    QString line;
    line += "create " + entity + "; type = " + Type() + ", name = " + Name();
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
    {
        if (it.key() != "name" && it.key()!="entity")
            line += "," + it.key() + "=" + it.value().Calc(params);
    }
    out << line; 
    return out; 
}

void Wizard_Entity::SetAllParents()
{
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
        it->SetWizardEntity(this);
}

