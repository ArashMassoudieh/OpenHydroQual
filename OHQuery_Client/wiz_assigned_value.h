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
class wiz_assigned_value
{
public:
	wiz_assigned_value();
	wiz_assigned_value(QString s);
	wiz_assigned_value(const wiz_assigned_value &s);
	QString toQString();
	~wiz_assigned_value();
	QString value_param_expression; 
	QString unit; 
	QString entity; 
	QString parameter_name; 
	QString value; 
	QString _last_error;
};

QString extract_between(QString str, QString del1, QString del2);
QString extract_between(QString str, char del1, char del2);
