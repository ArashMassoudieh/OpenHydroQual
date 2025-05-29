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


#include "wiz_parameter_group.h"



wiz_parameter_group::wiz_parameter_group()
{
}


wiz_parameter_group::~wiz_parameter_group()
{
}

wiz_parameter_group::wiz_parameter_group(const wiz_parameter_group & wiz_p_g)
{
	items = wiz_p_g.items;
	name = wiz_p_g.name;
	parameters = wiz_p_g.parameters;
	parent = wiz_p_g.parent;
	_last_error = wiz_p_g._last_error;
}

QString wiz_parameter_group::toQString()
{
	QString out;
	out += "parameter: {";
	for (int i=0; i<parameters.count(); i++)
	{
		out += "parameter = " + parameters[i];
		if (i < parameters.count() - 1) out += ",";
	}

	for (QString key : items.keys())
	{
		out += items[key].toQString(); 
		if (key != items.lastKey()) out += ",";
	}

	out += ", name = " + name;
	if (_last_error != "")
	{
		out += ", error = " + _last_error;
	}
	return out;
}

wiz_parameter_group::wiz_parameter_group(QString & s, Wizard_Script_Reader *_parent )
{
	parent = _parent; 
	QStringList params = extract_between(s.split(":")[1], QString("{"), QString("}")).split(",");
	for (int i = 0; i < params.size(); i++)
	{
		wiz_assigned_value entty(params[i]);
		if (entty._last_error == "")
		{
			if ((entty.entity.toLower().trimmed() != "parameter") && (entty.entity.toLower().trimmed() !="note"))
				items[entty.entity] = entty;
			else
				parameters.append(entty.value);
			
		}
		else
		{
			_last_error = _last_error + ";" + entty._last_error;
			break;
		}
	}

	if (get_parameter("name").value == "")
		_last_error = "No entity name";
	else
		name = get_name();
}
