#pragma once
#include <qstring.h>
#include <qmap.h>
#include "wiz_assigned_value.h"
#include "XString.h"

class Wizard_Script_Reader; 

class wiz_entity
{
public:
	wiz_entity();
	wiz_entity(QString &s, Wizard_Script_Reader *_parent);
	wiz_entity(const wiz_entity & we);
	QString get_configuration();
	QString get_direction();
    XString get_value(wiz_assigned_value x);
    XString get_value(QString x);
	XString get_nh();
	XString get_nv();
	~wiz_entity();
	Wizard_Script_Reader *parent; 
    QString toQString();
	QString& entity()
	{
		return _entity;
	}
	QString& get_last_error()
	{
		return _last_error; 
	}

	wiz_assigned_value &get_parameter(const QString name)
	{
		return parameters[name];
	}

	bool has_parameter(const QString name)
	{
		return parameters.contains(name);
	}

	QString &name()
	{
		return parameters["name"].value;
	}

	QString x()
	{
		if (parameters.count("x") > 0)
			return get_value(parameters["x"]);
		else
			return QString();
	}

	QString y()
	{
		if (parameters.count("y") > 0)
			return get_value(parameters["y"]);
		else
			return QString();
	}

	QString first_index()
	{
		if (parameters.count("first_index") > 0)
			return get_value(parameters["first_index"]);
		else
			return QString("1");
	}

	QString first_index_x()
	{
		if (parameters.count("first_index_x") > 0)
			return get_value(parameters["first_index_x"]);
		else
			return QString("1");
	}

	QString first_index_y()
	{
		if (parameters.count("first_index_y") > 0)
			return get_value(parameters["first_index_y"]);
		else
			return QString("1");
	}

	QString type()
	{
		if (parameters.count("type") > 0)
			return parameters["type"].value;
		if (parameters.count("Type") > 0)
			return parameters["Type"].value;
		else
			return QString("*");
	}

	QString subtype()
	{
		if (parameters.count("subtype") > 0)
			return parameters["subtype"].value;
		else
			return QString("*");
	}

	QMap<QString, wiz_assigned_value> &get_parameters()
	{
		return parameters; 
	}

private:
	QString _entity; 
	QMap<QString,wiz_assigned_value> parameters; 
	QString _last_error;
	
};


