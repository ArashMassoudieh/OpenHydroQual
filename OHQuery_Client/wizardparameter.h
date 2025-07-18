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


#ifndef WIZARDPARAMETER_H
#define WIZARDPARAMETER_H

#include <qstring.h>
#include <qjsonobject.h>
#include <QVector2D>
#include <QStringList>

enum class parameter_type {numeric, string, date, api, combofromapi};

class WizardParameter
{
public:
    WizardParameter();
    WizardParameter(const QJsonObject& jsonobject);
    WizardParameter(const WizardParameter& WS);
    WizardParameter& operator=(const WizardParameter& WS);
    QString Name();
    void SetName(const QString &_name);
    QString Delegate();
    QString Question();
    QString Default();
    QVector2D Range();
    QStringList Units();
    QStringList ComboItems();
    void SetEntryItem(QWidget *item) {entryitem = item;}
    QWidget *EntryItem() {return entryitem;}
    void SetValue(const QString &val) {value = val;}
    QString Value() {return value; }
    parameter_type ParameterType() { return Parameter_Type; }
    QStringList GetKeys() const;
private:
    QString name;
    QString delegate;
    QString question;
    QString default_value;
    QVector2D range;
    QStringList units;
    QStringList comboitems;
    QWidget *entryitem = nullptr;
    QString value;
    parameter_type Parameter_Type = parameter_type::numeric; 
};


#endif // WIZARDPARAMETER_H
