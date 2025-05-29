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


#include "wizardparametergroup.h"
#include "QJsonArray"

WizardParameterGroup::WizardParameterGroup()
{

}

WizardParameterGroup::WizardParameterGroup(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="description")
        {
            description = json_obj["description"].toString();
        }
        if (it.key() == "parameters")
        {
            QJsonArray params = json_obj["parameters"].toArray();
            for (int i=0; i<params.count(); i++)
            {
                parameters<<params[i].toString();
            }
        }
        if (it.key() == "criteria")
        {
            QJsonArray crits = json_obj["criteria"].toArray();
            for (int i=0; i<crits.count(); i++)
            {
                criteria<<WizardCriteria(crits[i].toObject());
            }
        }
    }
}

WizardParameterGroup::WizardParameterGroup(const WizardParameterGroup &WPG)
{
    parameters = WPG.parameters;
    criteria = WPG.criteria;
    name = WPG.name;
    description = WPG.description;
}

WizardParameterGroup& WizardParameterGroup::operator=(const WizardParameterGroup& WPG)
{
    parameters = WPG.parameters;
    criteria = WPG.criteria;
    name = WPG.name;
    description = WPG.description;
    return *this;
}

QStringList WizardParameterGroup::CheckCriteria(QMap<QString, WizardParameter> *Parameters)
{
    QStringList Errors;
    for (int i=0; i<criteria.count(); i++)
    {
        if (!criteria[i].Check(Parameters))
        {
            Errors.append(criteria[i].ErrorMessage());
        }
    }
    return Errors;

}
