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


#include "wizardcriteria.h"

WizardCriteria::WizardCriteria()
{

}

WizardCriteria::WizardCriteria(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        if (it.key()=="less")
        {
            less = Wizard_Argument(json_obj["less"].toString().toStdString());
        }
        if (it.key() == "greater")
        {
            greater = Wizard_Argument(json_obj["greater"].toString().toStdString());
        }
        if (it.key() == "errormessage")
        {
            errormessage = json_obj["errormessage"].toString();
        }
    }
}


WizardCriteria::WizardCriteria(const WizardCriteria &WPG){
    less = WPG.less;
    greater = WPG.greater;
    name = WPG.name;
    errormessage = WPG.errormessage;
}


WizardCriteria& WizardCriteria::operator=(const WizardCriteria& WPG)
{
    less = WPG.less;
    greater = WPG.greater;
    name = WPG.name;
    errormessage = WPG.errormessage;
    return *this;
}

bool WizardCriteria::Check(QMap<QString, WizardParameter> *Parameters)
{
    if (less.calc(Parameters)<greater.calc(Parameters))
        return true;
    else
        return false;
}
