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


#include "WizSingleBlock.h"
#include <QDebug>

SingleBlock::SingleBlock():Wizard_Entity()
{

}
SingleBlock::SingleBlock(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        else if (it.key()=="type")
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
SingleBlock::SingleBlock(const SingleBlock& MB) :Wizard_Entity(MB)
{    

}
SingleBlock& SingleBlock::operator=(const SingleBlock& MB)
{
    Wizard_Entity::operator=(MB);
    return *this;
}


QStringList SingleBlock::GenerateScript(QMap<QString, WizardParameter> *params)
{
    QStringList output;    
    QString line;
    line += "create block;";
    line += "type = " + Type();
    line += ", name =" +Name();
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
    {
        line += "," + it.key() + "=" + it.value().Calc(params)+it.value().UnitText();
    }
    output << line;
    qDebug() << output; 
    return output; 
    
}
