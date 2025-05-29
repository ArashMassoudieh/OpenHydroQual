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


#ifndef WIZARDCRITERIA_H
#define WIZARDCRITERIA_H

#include <QString>
#include <QJsonObject>
#include "wizardparameter.h"
#include "Wizard_Argument.h"

class WizardCriteria
{
public:
    WizardCriteria();
    WizardCriteria(const QJsonObject& jsonobj);
    WizardCriteria(const WizardCriteria &WPG);
    WizardCriteria& operator=(const WizardCriteria& WPG);
    QString Name() {return name;}
    QString ErrorMessage() {return errormessage;}
    Wizard_Argument Less() {return less;}
    Wizard_Argument Greater() {return greater;}
    bool Check(QMap<QString, WizardParameter> *Parameters);
private:
    Wizard_Argument less;
    Wizard_Argument greater;
    QString name;
    QString errormessage;

};

#endif // WIZARDCRITERIA_H
