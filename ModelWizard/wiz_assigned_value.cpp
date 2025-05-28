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


#include "wiz_assigned_value.h"
#include "wiz_entity.h"


wiz_assigned_value::wiz_assigned_value()
{
}



wiz_assigned_value::wiz_assigned_value(QString s)
{
	if (s.split("=").size() < 2)
	{
		_last_error = "Syntax error in '" + s + "'";
		return;
	}
	entity = s.split("=")[0].trimmed();
	if (s.split("=")[1].split("[").size() == 1)
	{
		value_param_expression = "value";
		value = s.split("=")[1].trimmed().split(";")[0];
		if (s.split("=")[1].trimmed().split(";").size() > 1)
			unit = s.split("=")[1].split("[")[1].toLower().trimmed().split(";")[1];
	}
	else if (s.split("=")[1].split("[").size() == 2)
	{
		if (s.split("=")[1].split("[")[0].toLower().trimmed() == "value")
		{
			value_param_expression = "value";
			value = s.split("=")[1].split("[")[1].split("]")[0].trimmed().split(";")[0];
			if (s.split("=")[1].split("[")[1].split("]")[0].toLower().trimmed().split(";").size() > 1)
				unit = s.split("=")[1].split("[")[1].toLower().trimmed().split(";")[1];
		}
		else if (s.split("=")[1].split("[")[0].toLower().trimmed() == "param")
		{
			value_param_expression = "parameter";
			parameter_name = extract_between(s.split("=")[1].trimmed(), '[', ']').trimmed();
		}
		else if (s.split("=")[1].split("[")[0].toLower().trimmed() == "expression")
		{
			value_param_expression = "expression";
			value = extract_between(s.split("=")[1].trimmed(), '[', ']').trimmed();
		}
	}
	else
	{
		_last_error = "Syntax error in '" + s.split("=")[1] + "'";
		return;
	}

		

}

wiz_assigned_value::wiz_assigned_value(const wiz_assigned_value & s)
{
	entity = s.entity;
	parameter_name = s.parameter_name;
	unit = s.unit;
	value = s.value;
	value_param_expression = s.value_param_expression;
	_last_error = s._last_error;
}

QString wiz_assigned_value::toQString()
{
	QString out;
	out += entity + "=";
	if (value_param_expression.toLower() == "value")
	{
		out += "value(";
		out += value;
		if (unit != "")
		{
			out += "[";
			out += unit;
			out += "]";
		}
		out += ")";
	}
	if (value_param_expression.toLower() == "param")
	{
		out += "param(";
		out += parameter_name;
		out += ")";
	}
	if (value_param_expression.toLower() == "expression")
	{
		out += "expression(";
		out += value;
		out += ")";
	}
	
	if (_last_error != "")
	{
		out += "error: " + _last_error;
	}
	return out; 
}



wiz_assigned_value::~wiz_assigned_value()
{
}

QString extract_between(QString str, QString del1, QString del2)
{
	unsigned first = str.indexOf(del1);
    unsigned last = str.indexOf(del2,first+1);
	QString out = str.mid(first+1, last - first-1);
	return out;
}

QString extract_between(QString str, char del1, char del2)
{
	unsigned first = str.indexOf(QString(del1));
    unsigned last = str.indexOf(QString(del2),first+1);
	QString out = str.mid(first+1, last - first-1);
	return out;
}
