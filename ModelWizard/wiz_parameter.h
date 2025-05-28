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
#include "XString.h"

class Wizard_Script_Reader;

class wiz_parameter
{
public:
	wiz_parameter();
	wiz_parameter(const wiz_parameter &wiz_param);
	QString toQString();
	~wiz_parameter();
	wiz_parameter(QString &s, Wizard_Script_Reader *_parent);
	Wizard_Script_Reader *parent;
    QString get_name()
	{
		name = parameters["name"].value.trimmed();
		return name;
	}
    QString get_last_error()
	{
		return _last_error;
	}

    QString get_role()
	{
		return parameters["delegate"].value.trimmed(); 
	}

    QString get_units()
    {
        if (parameters.count("unit")>0)
            return XString::reform(parameters["unit"].value.trimmed());
        else
            return "";
    }

	bool get_bold()
	{
		if (parameters["bold"].value.trimmed() == "true") return true;
		if (parameters["Bold"].value.trimmed() == "true") return true;
		return false; 
	}

	wiz_assigned_value &get_parameter(const QString name)
	{
		return parameters[name];
	}

	QMap<QString, wiz_assigned_value> &get_parameters()
	{
		return parameters;
	}

    QString param_name()
	{
		name = parameters["name"].value.trimmed();
		return parameters["name"].value.trimmed();
	}

    QString get_question()
	{
		if (parameters["question"].value.trimmed() == "")
			parameters["question"].value = parameters["name"].value;

		return parameters["question"].value.replace("#",",");
	}

    XString get_value()
	{
		return value;
	}

	void set_value(XString &val)
	{
		value = val;
	}


private:
	QString name;
	XString value;
	QMap<QString, wiz_assigned_value> parameters;
	QString _last_error;

};

