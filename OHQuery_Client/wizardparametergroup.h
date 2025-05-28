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


#ifndef WIZARDPARAMETERGROUP_H
#define WIZARDPARAMETERGROUP_H

#include <QStringList>
#include <QJsonObject>
#include "wizardcriteria.h"

class WizardParameterGroup
{
public:
    WizardParameterGroup();
    WizardParameterGroup(const QJsonObject& jsonobj);
    WizardParameterGroup(const WizardParameterGroup &WPG);
    WizardParameterGroup& operator=(const WizardParameterGroup& WPG);
    QString& Parameter(int i){
        if (i<parameters.size())
            return parameters[i];
    }
    WizardCriteria& Criteria(int i){
        if (i<criteria.size())
            return criteria[i];
    }
    QString Name() {return name;}
    QString Description() {return description;}
    int ParametersCount() {return parameters.count();}
    int CriteriaCount() {return criteria.count();}
    QStringList CheckCriteria(QMap<QString, WizardParameter> *Parameters);
private:
    QStringList parameters;
    QList<WizardCriteria> criteria;
    QString name;
    QString description;
};



#endif // WIZARDPARAMETERGROUP_H
