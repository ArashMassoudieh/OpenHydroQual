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


#include "Wizard_Script_Reader.h"
#include "qfile.h"
#include "qmessagebox.h"
#include "qtextstream.h"
#include "qdatetime.h"
#include "utility_funcs.h"

Wizard_Script_Reader::Wizard_Script_Reader()
{
	script_specific_params << "configuration" << "type" << "name";
}

Wizard_Script_Reader::Wizard_Script_Reader(const Wizard_Script_Reader & WSR)
{
	error_list = WSR.error_list;
	icon_file = WSR.icon_file;
	major_blocks = WSR.major_blocks;
	major_connections = WSR.major_connections;
	project_settings = WSR.project_settings;
	change_properties = WSR.change_properties;
	script_name = WSR.script_name;
	entities = WSR.entities;
	script_specific_params = WSR.script_specific_params;
	wiz_parameter_groups = WSR.wiz_parameter_groups;
	wiz_params = WSR.wiz_params;
	description = WSR.description;
}

QStringList  Wizard_Script_Reader::toQStingList()
{
	QStringList out; 
	out << "script name = " + script_name;
	out << "icon file = " + icon_file;
	out << "description = " + description;
	for (QString key : major_blocks.keys())
	{
		out << "major_block: " + major_blocks[key].toQString(); 
	}

	for (QString key : major_connections.keys())
	{
		out << "major_connection: " + major_connections[key].toQString();
	}

	for (QString key : change_properties.keys())
	{
		out << "change_property: " + change_properties[key].toQString();
	}

	for (QString key : entities.keys())
	{
		out << "entity: " + entities[key].toQString();
	}

	for (QString key : project_settings.keys())
	{
		out << "project setting: " + project_settings[key].toQString();
	}

	for (QString key : wiz_params.keys())
	{
		out << "parameter: " + wiz_params[key].toQString();
	}

	for (QString key : wiz_parameter_groups.keys())
	{
		out << "parameter_group: " + wiz_parameter_groups[key].toQString();
	}

	return out; 
}

Wizard_Script_Reader::Wizard_Script_Reader(QString filename)
{
	script_specific_params << "configuration" << "type" << "name" << "x" << "y" << "first_index";
	QFile file(filename); 
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox msgBox;
		msgBox.setText("File '" + filename + "' was not found");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		int ret = msgBox.exec();
	}
	else
	{
		QTextStream wizard_stream(&file);
		while (!wizard_stream.atEnd())
		{
			add_command(wizard_stream.readLine());
		}
	}
	file.close();

}

QStringList Wizard_Script_Reader::validate()
{
    QStringList out;
    for (QString key : criteria.keys())
    {
        if (criteria[key].expressiongreater.calc(this)<=criteria[key].expressionless.calc(this))
            out.append(criteria[key].ErrorMessage);

    }
    return out;
}


wiz_entity *Wizard_Script_Reader::operator[](const QString &name)
{
	if (major_blocks.count(name) == 1)
		return &major_blocks[name];
	else
		return nullptr;
}

wiz_entity *Wizard_Script_Reader::Get(const QString &name)
{
	if (major_blocks.count(name) == 1)
		return &major_blocks[name];
	else
		return nullptr;
}

bool Wizard_Script_Reader::add_command(QString line)
{
	if (line.split(":").size() == 1) return false;
	if (line.split(":")[0].toLower().trimmed() == "name") script_name = line.split(":")[1].trimmed();
	if (line.split(":")[0].toLower().trimmed() == "icon") icon_file = line.split(":")[1].trimmed();
	if (line.split(":")[0].toLower().trimmed() == "description") description = line.split(":")[1].trimmed().replace("#",",");

	if (line.split(":")[0].toLower().trimmed() == "settings" || line.split(":")[0].toLower().trimmed() == "major_block" || line.split(":")[0].toLower().trimmed() == "major_connection" || line.split(":")[0].toLower().trimmed() == "entity" || line.split(":")[0].toLower().trimmed() == "change_property" || line.split(":")[0].toLower().trimmed() == "criteria")
	{
		wiz_entity entty(line, this); 
		if (entty.name() == "" && entty.entity() == "settings")
		{
			entty.name() = "settings_" + QString::number(project_settings.count());
			entty.get_last_error() = "";
		}

		if (entty.get_last_error() == "")
		{
			if (entty.entity() == "settings")
			{
				project_settings[entty.name()] = entty;
			}
			else if (entty.entity() == "major_block")
			{
				major_blocks[entty.name()] = entty;

			}
			else if (entty.entity() == "major_connection")
			{
				major_connections[entty.name()] = entty;
			}

			else if (entty.entity() == "change_property")
			{
				change_properties[entty.name()] = entty;
			}

			else if (entty.entity() == "entity")
			{
				entities[entty.name()] = entty;
			}

            else if (entty.entity() == "criteria")
            {
                wiz_criterion w;
                w.expressionless = entty.get_parameter("less").value;
                w.expressiongreater = entty.get_parameter("greater").value;
                w.ErrorMessage = entty.get_parameter("ErrorMessage").value;
                criteria[entty.name()] = w;
            }

		}
		else
			error_list.append("Error in '" + line + "':" + entty.get_last_error());
			
	}
	else if (line.split(":")[0].toLower().trimmed() == "parameter")
	{
		wiz_parameter p(line, this); 
		if (p.get_last_error() == "")
			wiz_params[p.get_name()] = p;
		else
		{
			error_list.append("Parameter '" + p.get_name() + "':" + p.get_last_error());
		}
	}
	else if (line.split(":")[0].toLower().trimmed() == "parameter_group")
	{
		wiz_parameter_group pg(line, this);
		if (pg.get_last_error() == "")
			wiz_parameter_groups[pg.get_name()] = pg;
		else
		{
			error_list.append("Parameter group '" + pg.get_name() + "' :" + pg.get_last_error());
		}
	}

}

Wizard_Script_Reader::~Wizard_Script_Reader()
{
}


CCommand Wizard_Script_Reader::get_script_commands_entities(wiz_entity *wiz_ent)
{
	CCommand command; 
	command.command = "add";
	command.values.append(wiz_ent->type());

	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Entity";
	_filter.ObjectType = wiz_ent->type();
	_filter.SubType = wiz_ent->subtype();
	mPropList m = mproplist->filter(_filter);

	for (wiz_assigned_value item : wiz_ent->get_parameters())
	{
		command.parameters["Name"] = wiz_ent->name();
		if (!script_specific_params.contains(item.entity))
		{
			if (m.VariableNames_w_abv().contains(item.entity))
				command.parameters[item.entity] = wiz_ent->get_value(item);
			if (item.entity.toLower()=="rate" || item.entity.toLower().left(4)=="stch")
				command.parameters[item.entity] = wiz_ent->get_value(item);
		}
	}
	
	return command; 

}

CCommand Wizard_Script_Reader::get_script_commands_project_settings(wiz_entity *wiz_ent)
{
	CCommand command;
	command.command = "setprop";
	command.values.append(wiz_ent->type());

	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Entity";
	_filter.ObjectType = wiz_ent->type();
	_filter.SubType = wiz_ent->subtype();
	mPropList m = mproplist->filter(_filter);

	for (wiz_assigned_value item : wiz_ent->get_parameters())
	{
		if (!script_specific_params.contains(item.entity))
		{
			_filter.VariableName = item.entity;
			mPropList m1 = m.filter(_filter);
			if (m.VariableNames_w_abv().contains(item.entity))
			{
				if (m1[0].Delegate != "DateTime")
					command.parameters[item.entity] = wiz_ent->get_value(item);
				else
				{
					QDateTime QDT = QDateTime::fromString(wiz_ent->get_value(item),"M/d/yyyy hh:mm AP");
					//qDebug() << QDT << QDT.toString() << QDT.isValid();
					command.parameters[item.entity] = QString::number(QDate2Xldate(QDT)); 
				}
			}
		}
	}

	return command;
}

QList<CCommand> Wizard_Script_Reader::get_script_change_properties(wiz_entity *wiz_ent)
{
	QList<CCommand> commands; 
	if (wiz_ent->has_parameter("index_h") && !wiz_ent->has_parameter("index_v"))
	{ 
		wiz_entity* source = Get(wiz_ent->name());
		for (unsigned int i = 0; i < source->get_nv().toInt(); i++)
		{
			CCommand command;
			command.command = "setprop";
			command.values.append(wiz_ent->name() + " (" + wiz_ent->get_value("index_h") + "," + QString::number(i+source->first_index_y().toInt()) + ")");
			
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Block";
			_filter.ObjectType = wiz_ent->type();
			_filter.SubType = wiz_ent->subtype();
			mPropList m = mproplist->filter(_filter);

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					_filter.VariableName = item.entity;
					mPropList m1 = m.filter(_filter);
					if (m.VariableNames_w_abv().contains(item.entity))
					{
						if (m1[0].Delegate != "DateTime")
							command.parameters[item.entity] = wiz_ent->get_value(item);
						else
						{
							QDateTime QDT = QDateTime::fromString(wiz_ent->get_value(item), "M/d/yyyy hh:mm AP");
							//qDebug() << QDT << QDT.toString() << QDT.isValid();
							command.parameters[item.entity] = QString::number(QDate2Xldate(QDT));
						}
					}
				}
			}

			commands.append(command);

		}
	}
	else if (!wiz_ent->has_parameter("index_h") && wiz_ent->has_parameter("index_v"))
	{
		wiz_entity* source = Get(wiz_ent->name());
		for (unsigned int i = 0; i < source->get_nh().toInt(); i++)
		{
			CCommand command;
			command.command = "setprop";
			command.values.append(wiz_ent->name() + " (" + QString::number(i+source->first_index_x().toInt()) + "," + wiz_ent->get_value("index_v") + ")");

			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Block";
			_filter.ObjectType = wiz_ent->type();
			_filter.SubType = wiz_ent->subtype();
			mPropList m = mproplist->filter(_filter);

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					_filter.VariableName = item.entity;
					mPropList m1 = m.filter(_filter);
					if (m.VariableNames_w_abv().contains(item.entity))
					{
						if (m1[0].Delegate != "DateTime")
							command.parameters[item.entity] = wiz_ent->get_value(item);
						else
						{
							QDateTime QDT = QDateTime::fromString(wiz_ent->get_value(item), "M/d/yyyy hh:mm AP");
							//qDebug() << QDT << QDT.toString() << QDT.isValid();
							command.parameters[item.entity] = QString::number(QDate2Xldate(QDT));
						}
					}
				}
			}

			commands.append(command);
		}
	
	}
	else
	{
		CCommand command;
		command.command = "setprop";
		if (wiz_ent->has_parameter("index_h"))
			command.values.append(wiz_ent->name() + " (" + wiz_ent->get_value("index_h") + "," + wiz_ent->get_value("index_v") + ")");
		else if (wiz_ent->has_parameter("index"))
			command.values.append(wiz_ent->name() + " (" + wiz_ent->get_value("index") + ")");
		else
			command.values.append(wiz_ent->name());

		mProp _filter;
		_filter.setstar();
		_filter.GuiObject = "Block";
		_filter.ObjectType = wiz_ent->type();
		_filter.SubType = wiz_ent->subtype();
		mPropList m = mproplist->filter(_filter);

		for (wiz_assigned_value item : wiz_ent->get_parameters())
		{
			if (!script_specific_params.contains(item.entity))
			{
				_filter.VariableName = item.entity;
				mPropList m1 = m.filter(_filter);
				if (m.VariableNames_w_abv().contains(item.entity))
				{
					if (m1[0].Delegate != "DateTime")
						command.parameters[item.entity] = wiz_ent->get_value(item);
					else
					{
						QDateTime QDT = QDateTime::fromString(wiz_ent->get_value(item), "M/d/yyyy hh:mm AP");
						//qDebug() << QDT << QDT.toString() << QDT.isValid();
						command.parameters[item.entity] = QString::number(QDate2Xldate(QDT));
					}
				}
			}
		}
	
		commands.append(command);
		
	}
	return commands;
}


QList<CCommand> Wizard_Script_Reader::get_script_commands_major_blocks(wiz_entity *wiz_ent, int &x, int &y)
{
	QList<CCommand> commands;
	
	QString configuration = wiz_ent->get_configuration().toLower();
	QString direction = wiz_ent->get_direction();
	
	

	if (wiz_ent->x() != "")
		x = wiz_ent->x().toDouble();

	if (wiz_ent->y() != "")
		y = wiz_ent->y().toDouble();

	if (configuration == "")
	{
		int n = 1;
		CCommand command;
		command.command = "add";
		command.values.append(wiz_ent->type());
		

		mProp _filter;
		_filter.setstar();
		_filter.GuiObject = "Block";
		_filter.ObjectType = wiz_ent->type();
		mPropList m = mproplist->filter(_filter);

		for (wiz_assigned_value item : wiz_ent->get_parameters())
		{
			command.parameters["Name"] = wiz_ent->name();
			command.parameters["x"] = x;
			command.parameters["y"] = y;
			if (!script_specific_params.contains(item.entity))
			{
				if (m.VariableNames_w_abv().contains(XString::reform(item.entity)))
					command.parameters[item.entity] = wiz_ent->get_value(item);
				
			}
		}
		commands.append(command);
		x += h_big_interval;
	}
	if ((configuration == "1dv") || (configuration == "1dh") || (configuration == "1dr"))
	{
		commands.append(do_1dvh(configuration, wiz_ent, x, y));
	}
	if (configuration == "2dv")
	{
		commands.append(do_2dv(configuration, wiz_ent, x, y));
	}
	if (configuration == "2dh")
	{
		commands.append(do_2dh(configuration, wiz_ent, x, y));
	}
	if (configuration == "2dr")
	{
		commands.append(do_2dr(configuration, wiz_ent, x, y));
	}



	return commands; 
}


QList<CCommand> Wizard_Script_Reader::get_script_commands()
{
	int x, y;
	x = x_base; 
	y = y_base; 

	QList<CCommand> commands;
	
	for (QString key : get_project_settings().keys())
	{
		commands.append(get_script_commands_project_settings(&get_project_settings()[key]));
	}
	for (QString key : get_major_blocks().keys())
	{
		commands.append(get_script_commands_major_blocks(&get_major_blocks()[key],x,y));
	}
	for (QString key : get_major_connections().keys())
	{
		commands.append(get_script_commands_major_connections(&get_major_connections()[key], x, y));
	}
	for (QString key : get_entities().keys())
	{
		commands.append(get_script_commands_entities(&get_entities()[key]));
	}
	for (QString key : get_change_props().keys())
	{
		commands.append(get_script_change_properties(&get_change_props()[key]));
	}

	return commands; 

}

QList<CCommand> Wizard_Script_Reader::get_script_commands_major_connections(wiz_entity *wiz_ent, int &x, int &y)
{
	QList<CCommand> commands;

	QString configuration = wiz_ent->get_configuration();
	wiz_entity *source = &major_blocks[wiz_ent->get_parameter("source").value];
	wiz_entity *target = &major_blocks[wiz_ent->get_parameter("target").value];
	if (source == nullptr)
	{
		error_list.append(wiz_ent->get_parameter("source").value + " was not found!");
		return commands;
	}
	if (target == nullptr)
	{
		error_list.append(wiz_ent->get_parameter("target").value + " was not found!");
		return commands;
	}

	if (source->get_configuration() != "2dv" && source->get_configuration() != "2dh" && source->get_configuration() != "2dr" && target->get_configuration() != "2dv" && target->get_configuration() != "2dh" && target->get_configuration() != "2dr")
	{	if (configuration == "")
		{
			CCommand command;
			command.command = "connect";
			command.values.append(source->name());
			command.values.append(target->name());
			if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
			if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name();
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			_filter.ObjectType = wiz_ent->type();
			mPropList m = mproplist->filter(_filter);

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
					if (item.entity.toLower() == "area")
						command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
				}
			}
			commands.append(command);
		}
		if (configuration == "e2s" || configuration == "s2e" || configuration == "s2s" || configuration == "e2e")
		{
			CCommand command;
			command.command = "connect";
			if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
			if (configuration.left(1) == "e")
			{
				if (source->get_value(source->get_parameter("n")).toInt() > 1)
					command.values.append(source->name() + " (" + QString::number(source->get_value(source->get_parameter("n")).toInt() + source->first_index().toInt() - 1) + ")");
				else
					command.values.append(source->name());
			}
			else if (configuration.left(1) == "s")
			{
				if (source->get_value(source->get_parameter("n")).toInt() > 1)
					command.values.append(source->name() + " (" + source->first_index() + ")");
				else
					command.values.append(source->name());
			}



			if (configuration.right(1) == "e")
			{
				if (target->get_value(target->get_parameter("n")).toInt() > 1)
					command.values.append(target->name() + " (" + QString::number(target->get_value(target->get_parameter("n")).toInt() + source->first_index().toInt() - 1) + ")");
				else
					command.values.append(target->name());
			}
			else
			{
				if (target->get_value(target->get_parameter("n")).toInt() > 1)
					command.values.append(target->name() + " (" + target->first_index() + ")");
				else
					command.values.append(target->name());

			}

            command.parameters["Name"] = wiz_ent->name();
            mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			_filter.ObjectType = wiz_ent->type();
			mPropList m = mproplist->filter(_filter);

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
					if (item.entity.toLower() == "area")
						command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
				}
			}
			commands.append(command);
		}
		if (configuration == "a2a")
		{
			int n_source = source->get_value(QString("n")).toInt();
			int n_target = target->get_value(QString("n")).toInt();
			if (n_source != n_target)
				error_list.append("Number of cells in source and target are different");
			else
			{
				for (int i = 0; i < n_source; i++)
				{
					CCommand command;
					command.command = "connect";
					command.values.append(source->name() + " (" + QString::number(i + source->first_index().toInt()) + ")");
					command.values.append(target->name() + " (" + QString::number(i + source->first_index().toInt()) + ")");
					if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
                    int j=0;
                    if (source->first_index()==target->first_index()) j=source->first_index().toInt()-1;
                    if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i+j) + ")";
					mProp _filter;
					_filter.setstar();
					_filter.GuiObject = "Connector";
					_filter.ObjectType = wiz_ent->type();
					mPropList m = mproplist->filter(_filter);

					for (wiz_assigned_value item : wiz_ent->get_parameters())
					{
						if (!script_specific_params.contains(item.entity))
						{
							if (m.VariableNames_w_abv().contains(item.entity))
								command.parameters[item.entity] = wiz_ent->get_value(item);
							if (item.entity.toLower() == "area")
								command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
						}
					}
					commands.append(command);

				}
			}

		}
		if (configuration == "s2a" || configuration == "e2a")
		{
			int n_source = source->get_value(QString("n")).toInt();
			if (n_source == 0) n_source = 1; 
			int n_target = target->get_value(QString("n")).toInt();
						
			for (int i = 0; i < n_target; i++)
			{
				CCommand command;
				command.command = "connect";
				if (source->get_configuration()=="")
					command.values.append(source->name());
				else if (configuration == "s2a")
					command.values.append(source->name() + " (" + QString::number(source->first_index().toInt()) + ")");
				else if (configuration == "e2a")
					command.values.append(source->name() + " (" + QString::number(n_source - 1 + source->first_index().toInt()) + ")");

				command.values.append(target->name() + " (" + QString::number(i + target->first_index().toInt()) + ")");
				if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
				
				int j = target->first_index().toInt() - 1;
				if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i + j) + ")";
				mProp _filter;
				_filter.setstar();
				_filter.GuiObject = "Connector";
				_filter.ObjectType = wiz_ent->type();
				mPropList m = mproplist->filter(_filter);

				for (wiz_assigned_value item : wiz_ent->get_parameters())
				{
					if (!script_specific_params.contains(item.entity))
					{
						if (target->get_configuration() == "1dr")
						{
							double inradius = target->get_value(target->get_parameter("inradius")).toDouble();
							QString radius_unit = target->get_value(target->get_parameter("inradius")).unit;
							double outradius = target->get_value(target->get_parameter("outradius")).toDouble();
							double length = (outradius - inradius) / n_target;
							command.parameters["Interface/cross sectional area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));
							command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";

						}
						if (m.VariableNames_w_abv().contains(item.entity))
							command.parameters[item.entity] = wiz_ent->get_value(item);
						if (item.entity.toLower() == "area")
							command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
					}
				}
				commands.append(command);

			}
			
		}

		if (configuration == "a2s" || configuration == "a2e")
		{
			int n_source = source->get_value(QString("n")).toInt();
			int n_target = target->get_value(QString("n")).toInt();
			if (n_target == 0) n_target = 1;

			for (int i = 0; i < n_source; i++)
			{
				CCommand command;
				command.command = "connect";
				command.values.append(source->name() + " (" + QString::number(i + source->first_index().toInt()) + ")");
				if (target->get_configuration() == "")
					command.values.append(target->name());
				else if (configuration == "a2s")
					command.values.append(target->name() + " (" + QString::number(target->first_index().toInt()) + ")");
				else if (configuration == "a2e")
					command.values.append(target->name() + " (" + QString::number(n_target - 1 + target->first_index().toInt()) + ")");

				
				if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();

				int j = source->first_index().toInt() - 1;
				if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i + j) + ")";
				mProp _filter;
				_filter.setstar();
				_filter.GuiObject = "Connector";
				_filter.ObjectType = wiz_ent->type();
				mPropList m = mproplist->filter(_filter);

				for (wiz_assigned_value item : wiz_ent->get_parameters())
				{
					if (!script_specific_params.contains(item.entity))
					{
						if (source->get_configuration() == "1dr")
						{
							double inradius = source->get_value(source->get_parameter("inradius")).toDouble();
							QString radius_unit = source->get_value(source->get_parameter("inradius")).unit;
							double outradius = source->get_value(source->get_parameter("outradius")).toDouble();
							double length = (outradius - inradius) / n_source;
							command.parameters["Interface/cross sectional area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));
							command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";

						}
						if (m.VariableNames_w_abv().contains(item.entity))
							command.parameters[item.entity] = wiz_ent->get_value(item);
						if (item.entity.toLower() == "area")
							command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
					}
				}
				commands.append(command);

			}

		}

	}
	else if ((source->get_configuration() == "2dv" || source->get_configuration() == "2dr") && target->get_configuration() != "2dv" && target->get_configuration() != "2dh" && target->get_configuration() != "2dr")
	{
		if (configuration == "l2a" || configuration == "r2a" || configuration == "t2a" || configuration == "b2a" || configuration == "l2s" || configuration == "r2s" || configuration == "t2s" || configuration=="b2s" || configuration == "l2e" || configuration == "r2e" || configuration == "t2e" || configuration == "b2e")
		{
			int n_source;
			if (configuration == "l2a" || configuration == "r2a" || configuration == "l2s" || configuration == "r2s" || configuration == "l2e" || configuration == "r2e")
				n_source = source->get_value(QString("nv")).toInt();
            if (configuration == "t2a" || configuration == "b2a" || configuration == "t2s" || configuration == "b2s" || configuration == "t2e" || configuration == "b2e")
				n_source = source->get_nh().toInt();

			int n_target = target->get_value(QString("n")).toInt();
			if (!target->has_parameter("n") || target->get_value("n")=="") 
				n_target = 1; 
			if (n_source != n_target && n_target!=1 && (configuration=="r2a" || configuration=="l2a" || configuration=="t2a" || configuration=="b2a"))
				error_list.append("Number of cells in source and target are different");
			else
			{
				for (int i = 0; i < n_source; i++)
				{
					CCommand command;
					command.command = "connect";
					if (configuration == "l2a" || configuration == "l2e" || configuration == "l2s")
						command.values.append(source->name() + " (" + source->first_index_x() + "," + QString::number(i + source->first_index_y().toInt()) + ")");
					if (configuration == "r2a" || configuration == "r2e" || configuration == "r2s")
						command.values.append(source->name() + " (" + QString::number(source->get_nh().toInt() + source->first_index_x().toInt() - 1) + "," + QString::number(i + source->first_index_y().toInt()) + ")");
					if (configuration == "t2a" || configuration == "t2e" || configuration == "t2s")
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt() + i) + "," + QString::number(source->first_index_y().toInt()) + ")");
					if (configuration == "b2a" || configuration == "b2e" || configuration == "b2s")
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt() + i) + "," + QString::number(source->get_value(QString("nv")).toInt() + source->first_index_y().toInt() - 1) + ")");
					if (n_target>1 && configuration.right(1)==QString("a"))
						command.values.append(target->name() + " (" + QString::number(i + target->first_index().toInt()) + ")");
					else if (n_target>1 && configuration.right(1) == QString("s"))
						command.values.append(target->name() + " (" + QString::number(target->first_index().toInt()) + ")");
					else if (n_target>1 && configuration.right(1) == QString("e"))
						command.values.append(target->name() + " (" + QString::number(target->get_value(QString("n")).toInt() + target->first_index().toInt()-1) + ")");
					else if (n_target==1)
						command.values.append(target->name());

					if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();

                    int j=0;
                    if (target->first_index_x()==source->first_index() && (configuration.left(1)=="b" || configuration.left(1)=="t"))
                        j=target->first_index_x().toInt()-1;
                    if (target->first_index_y()==source->first_index() && (configuration.left(1)=="l" || configuration.left(1)=="r"))
                        j=target->first_index_y().toInt()-1;

                    if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i+1+j) + ")";
					mProp _filter;
					_filter.setstar();
					_filter.GuiObject = "Connector";
					_filter.ObjectType = wiz_ent->type();
					mPropList m = mproplist->filter(_filter);

					if ((configuration == "s2t" || configuration == "s2b" || configuration == "e2t" || configuration == "e2b") && target->get_configuration() == "2dr")
					{
						double inradius = target->get_value(target->get_parameter("inradius")).toDouble();
						QString radius_unit = target->get_value(target->get_parameter("inradius")).unit;
						double outradius = target->get_value(target->get_parameter("outradius")).toDouble();
						double length = (outradius - inradius) / n_target;
						command.parameters["Interface/cross sectional area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));
						command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";
					}

					if ((configuration == "t2s" || configuration == "b2s" || configuration == "b2e" || configuration == "t2e") && source->get_configuration() == "2dr")
					{
						double inradius = source->get_value(source->get_parameter("inradius")).toDouble();
						QString radius_unit = source->get_value(source->get_parameter("inradius")).unit;
						double outradius = source->get_value(source->get_parameter("outradius")).toDouble();
						double length = (outradius - inradius) / n_source;
						command.parameters["Interface/cross sectional area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));
						command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";
					}
					
					for (wiz_assigned_value item : wiz_ent->get_parameters())
					{
						if (!script_specific_params.contains(item.entity))
						{
							if (m.VariableNames_w_abv().contains(item.entity))
								command.parameters[item.entity] = wiz_ent->get_value(item);
							if (item.entity.toLower() == "area")
								command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
						}
					}
					commands.append(command);

				}
			}
		}
	}
    else if ((target->get_configuration() == "2dv" ||  target->get_configuration() == "2dr") && source->get_configuration() != "2dv" && source->get_configuration() != "2dh" && source->get_configuration() != "2dr")
    {
        if (configuration == "a2l" || configuration == "a2r" || configuration == "a2t" || configuration == "a2b" || configuration == "s2l" || configuration == "s2r" || configuration == "s2t" || configuration=="s2b" || configuration == "e2l" || configuration == "e2r" || configuration == "e2t" || configuration == "e2b")
        {
            int n_target;
            if (configuration == "a2l" || configuration == "a2r" || configuration == "s2l" || configuration == "s2r" || configuration == "e2l" || configuration == "e2r")
                n_target = target->get_value(QString("nv")).toInt();
            if (configuration == "a2t" || configuration == "a2b" || configuration == "s2t" || configuration == "s2b" || configuration == "e2b" || configuration == "e2t")
                n_target = target->get_nh().toInt();

            int n_source = source->get_value(QString("n")).toInt();
			if (!source->has_parameter("n") || source->get_value("n")=="") n_source = 1;
            if (n_source != n_target && n_source!=1 && (configuration=="a2r" || configuration=="a2l" || configuration=="a2t" || configuration=="a2b"))
                error_list.append("Number of cells in source and target are different");
            else
            {
                for (int i = 0; i < n_target; i++)
                {
                    CCommand command;
                    command.command = "connect";

                    if (n_source>1 && configuration.left(1)==QString("a"))
                        command.values.append(source->name() + " (" + QString::number(i + source->first_index().toInt()) + ")");
                    else if (n_source>1 && configuration.left(1) == QString("s"))
                        command.values.append(source->name() + " (" + QString::number(source->first_index().toInt()) + ")");
                    else if (n_source>1 && configuration.left(1) == QString("e"))
                        command.values.append(source->name() + " (" + QString::number(source->get_value(QString("n")).toInt() + source->first_index().toInt()-1) + ")");
                    else if (n_source==1)
                        command.values.append(source->name());


                    if (configuration == "a2l" || configuration == "e2l" || configuration == "s2l")
                        command.values.append(target->name() + " (" + target->first_index_x() + "," + QString::number(i + target->first_index_y().toInt()) + ")");
                    if (configuration == "a2r" || configuration == "e2r" || configuration == "s2r")
                        command.values.append(target->name() + " (" + QString::number(target->get_nh().toInt() + target->first_index_x().toInt() - 1) + "," + QString::number(i + target->first_index_y().toInt()) + ")");
                    if (configuration == "a2t" || configuration == "e2t" || configuration == "s2t")
                        command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt() + i) + "," + QString::number(target->first_index_y().toInt()) + ")");
                    if (configuration == "a2b" || configuration == "e2b" || configuration == "s2b")
                        command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt() + i) + "," + QString::number(target->get_value(QString("nv")).toInt() + target->first_index_y().toInt() - 1) + ")");

                    if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
                    int j=0;
                    if (target->first_index_x()==source->first_index() && (configuration.right(1)=="b" || configuration.right(1)=="t"))
                        j=target->first_index_x().toInt()-1;
                    if (target->first_index_y()==source->first_index() && (configuration.right(1)=="l" || configuration.right(1)=="r"))
                        j=target->first_index_y().toInt()-1;

                    if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i+1+j) + ")";
                    mProp _filter;
                    _filter.setstar();
                    _filter.GuiObject = "Connector";
                    _filter.ObjectType = wiz_ent->type();
                    mPropList m = mproplist->filter(_filter);

					if ((configuration == "s2t" || configuration == "s2b" || configuration == "e2b" || configuration == "e2t") && target->get_configuration() == "2dr")
					{
						double inradius = target->get_value(target->get_parameter("inradius")).toDouble();
						QString radius_unit = target->get_value(target->get_parameter("inradius")).unit;
						double outradius = target->get_value(target->get_parameter("outradius")).toDouble();
						double length = (outradius - inradius) / n_target;
						command.parameters["Interface/cross sectional area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));
						command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";
					}

					for (wiz_assigned_value item : wiz_ent->get_parameters())
                    {
                        if (!script_specific_params.contains(item.entity))
                        {
                            if (m.VariableNames_w_abv().contains(item.entity))
                                command.parameters[item.entity] = wiz_ent->get_value(item);
                            if (item.entity.toLower() == "area")
                                command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
                        }
                    }
                    commands.append(command);

                }
            }
        }
    }
	else if ((target->get_configuration() == "2dv" || target->get_configuration() == "2dr") && (source->get_configuration() == "2dv" || source->get_configuration() == "2dh" || source->get_configuration() == "2dr"))
	{
		if (configuration == "r2l" || configuration == "l2r" || configuration == "b2t" || configuration == "t2b")
		{
			int n_target;
			if (configuration == "r2l" || configuration == "l2r")
				n_target = target->get_value(QString("nv")).toInt();
			if (configuration == "b2t" || configuration == "t2b")
				n_target = target->get_nh().toInt();

			int n_source = n_target;
			
			if (n_source != n_target)
				error_list.append("Number of cells in source and target are different");
			else
			{
				for (int i = 0; i < n_target; i++)
				{
					CCommand command;
					command.command = "connect";

					if (configuration.left(1) == QString("l"))
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt()) + "," + QString::number(source->first_index_y().toInt() + i) + ")");
					else if (configuration.left(1) == QString("r"))
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt() + source->get_nh().toInt() - 1) + "," + QString::number(source->first_index_y().toInt() + i) + ")");
					else if (configuration.left(1) == QString("t"))
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt() + i) + "," + QString::number(source->first_index_y().toInt()) + ")");
					else if (configuration.left(1) == QString("b"))
						command.values.append(source->name() + " (" + QString::number(source->first_index_x().toInt() + i) + "," + QString::number(source->first_index_y().toInt() + source->get_value(QString("nv")).toInt() - 1) + ")");
						
					if (configuration.right(1) == QString("l"))
						command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt()) + "," + QString::number(target->first_index_y().toInt() + i) + ")");
					else if (configuration.right(1) == QString("r"))
						command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt() + target->get_nh().toInt() - 1) + "," + QString::number(target->first_index_y().toInt() + i) + ")");
					else if (configuration.right(1) == QString("t"))
						command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt() + i) + "," + QString::number(target->first_index_y().toInt()) + ")");
					else if (configuration.right(1) == QString("b"))
						command.values.append(target->name() + " (" + QString::number(target->first_index_x().toInt() + i) + "," + QString::number(target->first_index_y().toInt() + target->get_value(QString("nv")).toInt() - 1) + ")");


					if (wiz_ent->type() != "*") command.parameters["Type"] = wiz_ent->type();
					int j = 0;
					if (target->first_index_x() == source->first_index() && (configuration.right(1) == "b" || configuration.right(1) == "t"))
						j = target->first_index_x().toInt() - 1;
					if (target->first_index_y() == source->first_index() && (configuration.right(1) == "l" || configuration.right(1) == "r"))
						j = target->first_index_y().toInt() - 1;

					if (wiz_ent->name() != "*") command.parameters["Name"] = wiz_ent->name() + " (" + QString::number(i + 1 + j) + ")";
					mProp _filter;
					_filter.setstar();
					_filter.GuiObject = "Connector";
					_filter.ObjectType = wiz_ent->type();
					mPropList m = mproplist->filter(_filter);

					for (wiz_assigned_value item : wiz_ent->get_parameters())
					{
						if (!script_specific_params.contains(item.entity))
						{
							if (m.VariableNames_w_abv().contains(item.entity))
								command.parameters[item.entity] = wiz_ent->get_value(item);
							if (item.entity.toLower() == "area")
								command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);
						}
					}
					commands.append(command);

				}
			}
		}
	}
	return commands; 
}

XString Wizard_Script_Reader::get_parameter_value(QString &parameter_name)
{
	return get_parameters()[parameter_name].get_value();
}

QList<CCommand> Wizard_Script_Reader::do_1dvh(QString configuration, wiz_entity *wiz_ent, int &x, int &y)
{
	
	int first_index;
	if (wiz_ent->first_index() != "")
		first_index = wiz_ent->first_index().toInt();
	else
		first_index = 1;
	
	QList<CCommand> commands;
	QString direction = wiz_ent->get_direction();
	
	int n = wiz_ent->get_value(wiz_ent->get_parameter("n")).toInt();
	
	double z0 = 0;
	double Depth;
    QString Depth_unit;
    QString z0_unit;
	double length = wiz_ent->get_value(wiz_ent->get_parameter("Length")).toDouble();
    QString length_unit = wiz_ent->get_value(wiz_ent->get_parameter("Length")).unit;
    double width = wiz_ent->get_value(wiz_ent->get_parameter("Width")).toDouble();
    QString width_unit = wiz_ent->get_value(wiz_ent->get_parameter("Width")).unit;
	double inradius = wiz_ent->get_value(wiz_ent->get_parameter("inradius")).toDouble();
	QString radius_unit = wiz_ent->get_value(wiz_ent->get_parameter("inradius")).unit;
	double outradius = wiz_ent->get_value(wiz_ent->get_parameter("outradius")).toDouble();
	
	if (configuration == "1dr") length = (outradius - inradius) / n; 

    double slope = wiz_ent->get_value(wiz_ent->get_parameter("Slope")).toDouble();
	bool has_z0 = false;
	bool has_depth = false;
	if (wiz_ent->has_parameter("Bottom elevation"))
	{
		z0 = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).toDouble();
        z0_unit = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).unit;
        has_z0 = true;
	}
	if (wiz_ent->has_parameter("Depth"))
	{
		Depth = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).toDouble();
        Depth_unit = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).unit;
        has_depth = true;
	}
	if (wiz_ent->has_parameter("depth"))
	{
		Depth = wiz_ent->get_value(wiz_ent->get_parameter("depth")).toDouble();
		Depth_unit = wiz_ent->get_value(wiz_ent->get_parameter("depth")).unit;
		has_depth = true;
	}


	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Block";
	_filter.ObjectType = wiz_ent->type();
	mPropList m = mproplist->filter(_filter);

	for (int i = 0; i < n; i++)
	{
		CCommand command;
		command.command = "add";
		command.values.append(wiz_ent->type());
		if (n > 1)
			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index) + ")");
		else
			command.parameters["Name"] = XString(wiz_ent->name());

		command.parameters["x"] = x;
		command.parameters["y"] = y;
		if (configuration == "1dv")
		{
			if (direction == "up")
				y -= v_interval;
			else
				y += v_interval;
		}
		else if (configuration == "1dh" || configuration == "1dr")
		{
			if (direction == "left")
				x -= h_interval;
			else
				x += h_interval;
		}
		for (wiz_assigned_value item : wiz_ent->get_parameters())
		{
			if (!script_specific_params.contains(item.entity))
			{
				if (m.VariableNames_w_abv().contains(XString::reform(item.entity)))
					command.parameters[item.entity] = wiz_ent->get_value(item);
			}
		}
		if (has_depth)
		{
			if (configuration == "1dv")
			{
				if (direction == "up")
                {	command.parameters["Bottom elevation"] = z0 + i * Depth;
                    command.parameters["Bottom elevation"].unit = Depth_unit;
                }
				else
                {	command.parameters["Bottom elevation"] = z0 + (n - i - 1)*Depth;
                    command.parameters["Bottom elevation"].unit = Depth_unit;
                }
			}
		}
		if (configuration == "1dh" || configuration == "1dr")
        {	command.parameters["Bottom elevation"] = z0 - i*length*slope;
            command.parameters["Bottom elevation"].unit = z0_unit;
        }
		if (configuration == "1dr")
			command.parameters["Bottom area"] = PI * (pow((i + 1)*length + inradius, 2) - pow(i*length + inradius, 2));

		commands.append(command);

	}

	for (int i = 0; i < n - 1; i++)
	{
		CCommand command;
		command.command = "connect";
		command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index) + ")");
		command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index + 1) + ")");
		mProp _filter;
		_filter.setstar();
		_filter.GuiObject = "Connector";
		mPropList m = mproplist->filter(_filter);

		command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index+1) + ")");
        if (has_depth && configuration == "1dv")
        {   command.parameters["Length"] = Depth;
            command.parameters["Length"].unit = Depth_unit;
        }
        if (configuration == "1dh" || configuration == "1dr")
        {   command.parameters["Length"] = length;
            command.parameters["Length"].unit = length_unit;
        }
		for (wiz_assigned_value item : wiz_ent->get_parameters())
		{
			if (!script_specific_params.contains(item.entity))
			{
				if (m.VariableNames_w_abv().contains(item.entity))
					command.parameters[item.entity] = wiz_ent->get_value(item);
				if ((item.entity.toLower() == "bottom area" || item.entity.toLower() == "area") && configuration == "1dv")
					command.parameters["Interface/cross sectional area"] = wiz_ent->get_value(item);

			}
		}

		if (configuration == "1dh")
        {	if (has_depth)
            {   command.parameters["Interface/cross sectional area"] = width*Depth;
                if (width_unit.contains("ft") && Depth_unit.contains("ft"))
                    command.parameters["Interface/cross sectional area"].unit = "ft~^2";
            }
        }

		if (configuration == "1dr")
		{
			if (has_depth)
			{
				command.parameters["Interface/cross sectional area"] = ((i+1)*length+inradius) * Depth *2*PI;
				command.parameters["Interface/cross sectional area"].unit = radius_unit + "~^2";
			}
			else
			{
				command.parameters["width"] = ((i + 1)*length + inradius) * 2 * PI;
				command.parameters["width"].unit = radius_unit + "~^2";
			}
		}

		commands.append(command);

	}

	if (configuration == "1dv")
	{
		y = y_base;
		x += h_big_interval;
	}
	else if (configuration == "1dh")
	{
		x = x_base;
		y += v_big_interval;
	}

	return commands; 
}

QList<CCommand> Wizard_Script_Reader::do_2dv(QString configuration, wiz_entity *wiz_ent, int &x, int &y)
{
	QList<CCommand> commands;
	QString direction = wiz_ent->get_direction();

	int nh = wiz_ent->get_value(wiz_ent->get_parameter("nh")).toInt();
	int nv = wiz_ent->get_value(wiz_ent->get_parameter("nv")).toInt();

	int first_index_x, first_index_y;
	if (wiz_ent->first_index() != "")
		first_index_x = wiz_ent->first_index_x().toInt();
	else
		first_index_x = 1;

	if (wiz_ent->first_index_y() != "")
		first_index_y = wiz_ent->first_index_y().toInt();
	else
		first_index_y = 1;

	double z0 = 0;
	double Depth;
    QString Depth_unit;
    QString length_unit;
    QString Width_unit;
    QString z0_unit;
	double length = wiz_ent->get_value(wiz_ent->get_parameter("Length")).toDouble();
    length_unit = wiz_ent->get_value(wiz_ent->get_parameter("Length")).unit;
	double Width = wiz_ent->get_value(wiz_ent->get_parameter("Width")).toDouble();
    Width_unit = wiz_ent->get_value(wiz_ent->get_parameter("Width")).unit;
	double slope = wiz_ent->get_value(wiz_ent->get_parameter("Slope")).toDouble();
	bool has_z0 = false;
	bool has_depth = false;
	if (wiz_ent->has_parameter("Bottom elevation"))
	{
		z0 = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).toDouble();
        z0_unit = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).unit;
        has_z0 = true;
	}
	if (wiz_ent->has_parameter("Depth"))
	{
		Depth = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).toDouble();
        Depth_unit = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).unit;
		has_depth = true;
	}

	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Block";
	_filter.ObjectType = wiz_ent->type();
	mPropList m = mproplist->filter(_filter);
	
	for (int i = 0; i < nh; i++)
	{
		for (int j = 0; j < nv; j++)
		{
			CCommand command;
			command.command = "add";
			command.values.append(wiz_ent->type());

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");


			command.parameters["x"] = x;
			command.parameters["y"] = y;

			if (direction.contains("up"))
				y -= v_interval;
			else
				y += v_interval;


			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(XString::reform(item.entity)))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
			if (has_depth)
			{
				{
					if (direction == "up")
                    {	command.parameters["Bottom elevation"] = z0 + j * Depth;
                        command.parameters["Bottom elevation"].unit = Depth_unit;
                    }
					else
                    {	command.parameters["Bottom elevation"] = z0 + (nv - j - 1)*Depth;
                        command.parameters["Bottom elevation"].unit = Depth_unit;
                    }
				}
			}


			commands.append(command);

		}
		if (direction.contains("left"))
			x -= h_interval;
		else
			x += h_interval;
		if (wiz_ent->get_parameters().count("y"))
			y = wiz_ent->get_value("y").toDouble();
		else
			y = y_base;
		z0 -= slope*length;
	}
	
	for (int i = 0; i < nh-1; i++)
	{
		for (int j = 0; j < nv; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");
			
			
			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}

			command.parameters["Length"] = length;
            command.parameters["Length"].unit = length_unit;
			command.parameters["Interface/cross sectional area"] = Width*Depth;

			commands.append(command);

		}
	}

	for (int i = 0; i < nh; i++)
	{
		for (int j = 0; j < nv-1; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");
			
			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
			command.parameters["Length"] = Depth;
			commands.append(command);

		}
	}

	y = y_base;
	x += h_big_interval;

	return commands;
	
}

QList<CCommand> Wizard_Script_Reader::do_2dh(QString configuration, wiz_entity *wiz_ent, int &x, int &y)
{
	QList<CCommand> commands;
	QString direction = wiz_ent->get_direction();

	int nx = wiz_ent->get_value(wiz_ent->get_parameter("nx")).toInt();
	int ny = wiz_ent->get_value(wiz_ent->get_parameter("ny")).toInt();

	int first_index_x, first_index_y;
	if (wiz_ent->first_index() != "")
		first_index_x = wiz_ent->first_index_x().toInt();
	else
		first_index_x = 1;

	if (wiz_ent->first_index_y() != "")
		first_index_y = wiz_ent->first_index_y().toInt();
	else
		first_index_y = 1;

	double z0 = 0;
	double Depth;
	double length = wiz_ent->get_value(wiz_ent->get_parameter("Length")).toDouble();
    QString lenght_unit = wiz_ent->get_value(wiz_ent->get_parameter("Length")).unit;
    double Width = wiz_ent->get_value(wiz_ent->get_parameter("Width")).toDouble();
    QString Width_unit = wiz_ent->get_value(wiz_ent->get_parameter("Width")).unit;
	double slope_x = wiz_ent->get_value(wiz_ent->get_parameter("X_Slope")).toDouble();
	double slope_y = wiz_ent->get_value(wiz_ent->get_parameter("Y_Slope")).toDouble();
	bool has_z0 = false;
	bool has_depth = false;
    QString Depth_unit;
    QString z0_unit;
	if (wiz_ent->has_parameter("Bottom elevation"))
	{
		z0 = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).toDouble();
        z0_unit = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).unit;
        has_z0 = true;

	}
	if (wiz_ent->has_parameter("Depth"))
	{
		Depth = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).toDouble();
        Depth_unit = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).unit;
        has_depth = true;
	}

	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Block";
	_filter.ObjectType = wiz_ent->type();
	mPropList m = mproplist->filter(_filter);

	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			CCommand command;
			command.command = "add";
			command.values.append(wiz_ent->type());

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");


			command.parameters["x"] = x;
			command.parameters["y"] = y;

			if (direction.contains("up"))
				y -= v_interval;
			else
				y += v_interval;


			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(XString::reform(item.entity)))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
			
			command.parameters["Bottom elevation"] = z0 - i*slope_x - j*slope_y;
            command.parameters["Bottom elevation"].unit = z0_unit;

			commands.append(command);
		}
						
		if (direction.contains("left"))
			x -= h_interval;
		else
			x += h_interval;
		if (wiz_ent->get_parameters().count("y"))
			y = wiz_ent->get_value("y").toDouble();
		else
			y = y_base;
		
	}

	for (int i = 0; i < nx - 1; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");


			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}

			command.parameters["Length"] = length;
            command.parameters["Length"].unit = lenght_unit;
			command.parameters["Width"] = Width;
            command.parameters["Width"].unit = Width_unit;
			command.parameters["Interface/cross sectional area"] = Width*Depth;
            if (lenght_unit == Width_unit)
                command.parameters["Interface/cross sectional area"].unit = lenght_unit + "~^2";
			commands.append(command);

		}
	}

	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny - 1; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
            command.parameters["Length"] = length;
            command.parameters["Length"].unit = lenght_unit;
            command.parameters["Width"] = Width;
            command.parameters["Width"].unit = Width_unit;
			command.parameters["Interface/cross sectional area"] = length*Depth;
            if (lenght_unit == Depth_unit)
                command.parameters["Interface/cross sectional area"].unit = lenght_unit + "~^2";
			commands.append(command);

		}
	}

	y = y_base;
	x += h_big_interval;
	

	return commands;

}


QList<CCommand> Wizard_Script_Reader::do_2dr(QString configuration, wiz_entity *wiz_ent, int &x, int &y)
{
	QList<CCommand> commands;
	QString direction = wiz_ent->get_direction();

	int nr = wiz_ent->get_value(wiz_ent->get_parameter("nr")).toInt();
	int nv = wiz_ent->get_value(wiz_ent->get_parameter("nv")).toInt();

	int first_index_x, first_index_y;
	if (wiz_ent->first_index() != "")
		first_index_x = wiz_ent->first_index_x().toInt();
	else
		first_index_x = 1;

	if (wiz_ent->first_index_y() != "")
		first_index_y = wiz_ent->first_index_y().toInt();
	else
		first_index_y = 1;

	double z0 = 0;
	double Depth;
	QString Depth_unit;
	QString Radious_unit;
	QString Width_unit;
	QString z0_unit;
	double in_radius = wiz_ent->get_value(wiz_ent->get_parameter("inradius")).toDouble();
	double out_radius = wiz_ent->get_value(wiz_ent->get_parameter("outradius")).toDouble();
	double length = (out_radius - in_radius) / nr; 

	Radious_unit = wiz_ent->get_value(wiz_ent->get_parameter("inradius")).unit;
	
	double slope = wiz_ent->get_value(wiz_ent->get_parameter("Slope")).toDouble();
	bool has_z0 = false;
	bool has_depth = false;
	if (wiz_ent->has_parameter("Bottom elevation"))
	{
		z0 = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).toDouble();
		z0_unit = wiz_ent->get_value(wiz_ent->get_parameter("Bottom elevation")).unit;
		has_z0 = true;
	}
	if (wiz_ent->has_parameter("Depth"))
	{
		Depth = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).toDouble();
		Depth_unit = wiz_ent->get_value(wiz_ent->get_parameter("Depth")).unit;
		has_depth = true;
	}

	mProp _filter;
	_filter.setstar();
	_filter.GuiObject = "Block";
	_filter.ObjectType = wiz_ent->type();
	mPropList m = mproplist->filter(_filter);

	for (int i = 0; i < nr; i++)
	{
		for (int j = 0; j < nv; j++)
		{
			CCommand command;
			command.command = "add";
			command.values.append(wiz_ent->type());

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");


			command.parameters["x"] = x;
			command.parameters["y"] = y;
			command.parameters["Bottom area"] = (pow((i + 1)*length + in_radius, 2) - pow(i*length + in_radius, 2))*PI;

			if (direction.contains("up"))
				y -= v_interval;
			else
				y += v_interval;


			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(XString::reform(item.entity)))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
			if (has_depth)
			{
				{
					if (direction == "up")
					{
						command.parameters["Bottom elevation"] = z0 + j * Depth;
						command.parameters["Bottom elevation"].unit = Depth_unit;
					}
					else
					{
						command.parameters["Bottom elevation"] = z0 + (nv - j - 1)*Depth;
						command.parameters["Bottom elevation"].unit = Depth_unit;
					}
				}
			}


			commands.append(command);

		}
		if (direction.contains("left"))
			x -= h_interval;
		else
			x += h_interval;
		if (wiz_ent->get_parameters().count("y"))
			y = wiz_ent->get_value("y").toDouble();
		else
			y = y_base;
		z0 -= slope * length;
	}

	for (int i = 0; i < nr - 1; i++)
	{
		for (int j = 0; j < nv; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x + 1) + "," + QString::number(j + first_index_y) + ")");


			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}

			command.parameters["Length"] = length;
			command.parameters["Length"].unit = Radious_unit;
			command.parameters["Interface/cross sectional area"] = ((i+1)*length + in_radius)*2*PI*Depth;

			commands.append(command);

		}
	}

	for (int i = 0; i < nr; i++)
	{
		for (int j = 0; j < nv - 1; j++)
		{
			CCommand command;
			command.command = "connect";
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ")");
			command.values.append(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");
			mProp _filter;
			_filter.setstar();
			_filter.GuiObject = "Connector";
			mPropList m = mproplist->filter(_filter);

			command.parameters["Name"] = XString(wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y) + ") - " + wiz_ent->name() + " (" + QString::number(i + first_index_x) + "," + QString::number(j + first_index_y + 1) + ")");

			for (wiz_assigned_value item : wiz_ent->get_parameters())
			{
				if (!script_specific_params.contains(item.entity))
				{
					if (m.VariableNames_w_abv().contains(item.entity))
						command.parameters[item.entity] = wiz_ent->get_value(item);
				}
			}
			command.parameters["Length"] = Depth;
			commands.append(command);

		}
	}

	y = y_base;
	x += h_big_interval;

	return commands;

}