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


#include "wizardparameter.h"
#include "QJsonArray"

WizardParameter::WizardParameter()
{

}


WizardParameter::WizardParameter(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="delegate")
        {
            delegate = json_obj["delegate"].toString();
            if (delegate == "FileBrowser" || "ComboBox")
            {
                Parameter_Type = parameter_type::string;
            }
            else if (delegate == "DateBox")
            {
                Parameter_Type = parameter_type::date;
            }
            else if (delegate.contains("FromAPI"))
            {
                Parameter_Type = parameter_type::api;
            }
            else if (delegate.contains("CombofromAPI"))
            {
                Parameter_Type = parameter_type::combofromapi;
            }

        }
        if (it.key() == "question")
        {
            question = json_obj["question"].toString();
        }
        if (it.key() == "default")
        {
            default_value = json_obj["default"].toString();
        }
        if (it.key() == "unit")
        {
            units.clear();
            units=it.value().toString().split(";");
        }
        if (it.key() == "range")
        {
            QJsonArray items = json_obj["range"].toArray();
            range[0]=items[0].toString().toInt();
            range[1]=items[1].toString().toInt();
        }
        if (it.key() == "comboitems")
        {
            comboitems.clear();
            QJsonArray items = json_obj["combotems"].toArray();
            for (int i=0; i<items.size(); i++)
            {
                comboitems = it.value().toString().split(";");
            }
        }

    }
}
WizardParameter::WizardParameter(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
    range = WS.range;
    comboitems = WS.comboitems;
    Parameter_Type = WS.Parameter_Type;
    default_value = WS.default_value;
    units = WS.units;
}
WizardParameter& WizardParameter::operator=(const WizardParameter& WS)
{
    name = WS.name;
    delegate = WS.delegate;
    question = WS.question;
    range = WS.range;
    comboitems = WS.comboitems;
    units = WS.units;
    Parameter_Type = WS.Parameter_Type;
    default_value = WS.default_value;
    return *this;
}
QString WizardParameter::Name()
{
    return name;
}
void WizardParameter::SetName(const QString &_name)
{
    name = _name; 
}
QString WizardParameter::Delegate()
{
    return delegate;
}
QString WizardParameter::Default()
{
    return default_value;
}
QString WizardParameter::Question()
{
    return question;
}

QVector2D WizardParameter::Range()
{
    return range;
}
QStringList WizardParameter::Units()
{
    return units;
}
QStringList WizardParameter::ComboItems()
{
    return comboitems;
}
QStringList WizardParameter::GetKeys() const
{

}
