#include "wiz_entity.h"
#include "Wizard_Script_Reader.h"
#include "qstring.h"
#include "Expression.h"


wiz_entity::wiz_entity()
{
}

wiz_entity::wiz_entity(const wiz_entity & we)
{
	parameters = we.parameters;
	parent = we.parent;
	_entity = we._entity;
	_last_error = we._last_error;
}

QString wiz_entity::toQString()
{
	QString out; 
	out += "entity: " + _entity + "\n";
	int i = 0; 
	for (QString key : parameters.keys())
	{
		out += parameters[key].toQString();
		if (i < parameters.count() - 1) out + ", ";
		i++;
	}
	out += "\n";
	out += "errors: " + _last_error + "\n";

	return out; 
}

wiz_entity::wiz_entity(QString &s, Wizard_Script_Reader *_parent)
{
	QStringList gofirst; gofirst << "name" << "type" << "subtype";
	parent = _parent; 
	_entity = s.split(":")[0].trimmed().toLower();
	QStringList params = extract_between(s.split(":")[1],QString("{"),QString("}")).split(",");
	
	for (int i = 0; i < params.size(); i++)
	{
		wiz_assigned_value entty(params[i]);
		if (gofirst.contains(entty.entity))
			parameters[entty.entity] = entty;
	}

	for (int i = 0; i < params.size(); i++)
	{
		wiz_assigned_value entty(params[i]);
		if (entty._last_error == "")
		{
			if (!gofirst.contains(entty.entity))
				parameters[entty.entity] = entty;
		}
		else
		{
			_last_error = _last_error + ";" + entty._last_error;
			break;
		}
	}

	if (get_parameter("name").value == "")
		_last_error = "No entity name";
	
}



QString wiz_entity::get_configuration()
{
	QString s; 
	if (parameters.count("configuration") > 0)
		s = parameters["configuration"].value.toLower().trimmed();

	return s;
}

QString wiz_entity::get_direction()
{
	QString s;
	if (get_configuration() == "1dv") s = "down"; else s = "right";
	if (parameters.count("direction") > 0)
		s = parameters["direction"].value;

	return s;
}

XString wiz_entity::get_value(wiz_assigned_value x)
{
	XString X; 
	//qDebug() << x.entity << " count: " << parameters.count(x.entity) << endl; 
	if (parameters.count(x.entity) > 0)
	{
		if (parameters[x.entity].value_param_expression == "value")
			X.setString(parameters[x.entity].value);
		if (parameters[x.entity].value_param_expression == "parameter")
			X = parent->get_parameters()[x.parameter_name].get_value();
		if (parameters[x.entity].value_param_expression == "expression")
		{
			CExpression expr(parameters[x.entity].value); 
			X = expr.calc(parent);
			X.unit = expr.unit;
		};
	}
	return X; 

}

XString wiz_entity::get_value(QString x)
{
	if (parameters.count(x) > 0)
		return get_value(parameters[x]);
	else
		return XString(); 
}

XString wiz_entity::get_nh()
{
	if (parameters.count("nh") > 0)
		return get_value(parameters["nh"]);
	else if (parameters.count("nr") > 0)
		return get_value(parameters["nr"]);
	else	
		return XString();
}

XString wiz_entity::get_nv()
{
	if (parameters.count("nv") > 0)
		return get_value(parameters["nv"]);
	else
		return XString();
}

wiz_entity::~wiz_entity()
{
}



