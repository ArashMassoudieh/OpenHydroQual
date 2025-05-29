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


#include "wiz_parameter.h"
#include "qstring.h"
#include "qstringlist.h"
#include "wiz_assigned_value.h"



wiz_parameter::wiz_parameter()
{
}

wiz_parameter::wiz_parameter(const wiz_parameter & wiz_param)
{
	name = wiz_param.name;
	parameters = wiz_param.parameters;
	parent = wiz_param.parent;
	value = wiz_param.value;
	_last_error = wiz_param._last_error;
}

QString wiz_parameter::toQString()
{
	QString out; 
	out += "parameter: {";
	for (QString key : parameters.keys())
	{
		out += parameters[key].toQString(); 
	}
	out += ", value = " + value; 
	out += ", name = " + name; 
	if (_last_error != "")
	{
		out += ", error = " + _last_error;
	}
	return out; 
}

wiz_parameter::~wiz_parameter()
{
}

wiz_parameter::wiz_parameter(QString & s, Wizard_Script_Reader *_parent)
{
	parent = _parent;
	QStringList params = extract_between(s.split(":")[1], QString("{"), QString("}")).split(",");
	for (int i = 0; i < params.size(); i++)
	{
		wiz_assigned_value entty(params[i]);
		if (entty._last_error == "")
			parameters[entty.entity] = entty;
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
