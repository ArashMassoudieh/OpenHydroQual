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
#include "wiz_assigned_value.h"
#include <qmap.h>

class Wizard_Script_Reader;

class wiz_parameter_group
{
public:
	wiz_parameter_group();
	~wiz_parameter_group();
	wiz_parameter_group(const wiz_parameter_group &wiz_p_g);
	QString toQString();
	wiz_parameter_group(QString &s, Wizard_Script_Reader *_parent);
	Wizard_Script_Reader *parent;
	QString& get_name()
	{
		name = items["name"].value.trimmed();
		return name;
	}
	QString& get_last_error()
	{
		return _last_error;
	}

	wiz_assigned_value &get_parameter(const QString name)
	{
		return items[name];
	}

	QMap<QString, wiz_assigned_value> &get_items()
	{
		return items;
	}

	QList<QString> &get_parameters()
	{
		return parameters;
	}

    QString pg_name()
	{
		name = items["name"].value.trimmed();
		return items["name"].value.trimmed();
	}

	QString &get_description()
	{
		if (items["description"].value.trimmed()=="") 
			 items["description"].value = items["name"].value;
		QString x = items["description"].value.trimmed();
		return items["description"].value;
		
	}
private:
	QString name;
	QMap<QString, wiz_assigned_value> items;
	QList<QString> parameters;
	QString _last_error;
};

