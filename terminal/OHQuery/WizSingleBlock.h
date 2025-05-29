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
#include <QVector> 
#include "Wizard_Entity.h"

class SingleBlock: public Wizard_Entity
{
public:
    SingleBlock();
    SingleBlock(const QJsonObject& jsonobject);
    SingleBlock(const SingleBlock& WS);
    SingleBlock& operator=(const SingleBlock& WS);
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
private:



};
