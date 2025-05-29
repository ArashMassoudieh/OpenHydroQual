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
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#include "Script.h"
#include <iostream>
#include <fstream>
#include "System.h"

Script::Script()
{
   FillMustBeSpecified();
}

Script::~Script()
{
    if (system && systemwascreated) delete system;
}

Script::Script(const string &filename, System *sys)
{

    FillMustBeSpecified();
    if (system!=nullptr)
    {   system = sys;
        systemwascreated=false;
    }
    else
    {   system = new System();
        systemwascreated = true;
    }
    fstream file(filename);
    if (!file.good())
    {
        cout<< "File '" + filename + "' was not found!"<<std::endl;
        return;
    }
    while (!file.eof())
    {
        string s;
        getline(file,s);
        Command command(s);
        if (command.Syntax())
            Append(command);
        else
            errors.push_back(command.LastError());
    }
}

bool Script::CreateSystemFromQStringList(const QStringList &Script, System *sys)
{

    FillMustBeSpecified();
    if (system!=nullptr)
    {   system = sys;
        systemwascreated=false;
    }
    else
    {   system = new System();
        systemwascreated = true;
    }

    Command command("loadtemplate; filename = main_components.json");
    if (command.Syntax())
        Append(command);
    else
        errors.push_back(command.LastError());
    for (int i=0; i<Script.count(); i++)
    {
        Command command(Script[i].toStdString());
        if (command.Syntax())
            Append(command);
        else
            errors.push_back(command.LastError());
    }
    CreateSystem(sys);
    return systemwascreated;
}

bool Script::CreateSystem(System *system)
{

    for (unsigned int i=0; i<commands.size(); i++)
    {
        if (!commands[i].Execute(system))
        {
            system->errorhandler.Append("","Script","CreateSystem",commands[i].LastError(),6001);
            errors.push_back(commands[i].LastError());
        }
    }
    system->SetVariableParents();
    return true;
}


System* Script::CreateSystem()
{
    system = new System();
    for (unsigned int i=0; i<commands.size(); i++)
    {
        if (!commands[i].Execute(system))
        {
            system->errorhandler.Append("","Script","CreateSystem",commands[i].LastError(),6001);
            errors.push_back(commands[i].LastError());
        }
    }
    system->SetVariableParents();
    return system;
}

void Script::FillMustBeSpecified()
{
    mustbespecifiedatcreation["create"] = map<string, vector<string>>();
        mustbespecifiedatcreation["create"]["block"] = vector<string>();
            mustbespecifiedatcreation["create"]["block"].push_back("name");
            mustbespecifiedatcreation["create"]["block"].push_back("type");
        mustbespecifiedatcreation["create"]["link"] = vector<string>();
            mustbespecifiedatcreation["create"]["link"].push_back("name");
            mustbespecifiedatcreation["create"]["link"].push_back("type");
            mustbespecifiedatcreation["create"]["link"].push_back("from");
            mustbespecifiedatcreation["create"]["link"].push_back("to");
        mustbespecifiedatcreation["create"]["parameter"] = vector<string>();
            mustbespecifiedatcreation["create"]["parameter"].push_back("name");
            mustbespecifiedatcreation["create"]["parameter"].push_back("low");
            mustbespecifiedatcreation["create"]["parameter"].push_back("high");
        mustbespecifiedatcreation["create"]["objectivefunction"] = vector<string>();
            mustbespecifiedatcreation["create"]["objectivefunction"].push_back("name");
            mustbespecifiedatcreation["create"]["objectivefunction"].push_back("object");
            mustbespecifiedatcreation["create"]["objectivefunction"].push_back("expression");
        mustbespecifiedatcreation["create"]["observation"] = vector<string>();
            mustbespecifiedatcreation["create"]["observation"].push_back("name");
            mustbespecifiedatcreation["create"]["observation"].push_back("object");
            mustbespecifiedatcreation["create"]["observation"].push_back("expression");

            mustbespecifiedatcreation["create"]["source"] = vector<string>();
            mustbespecifiedatcreation["create"]["source"].push_back("name");
            mustbespecifiedatcreation["create"]["source"].push_back("type");
        mustbespecifiedatcreation["create"]["constituent"] = vector<string>();
            mustbespecifiedatcreation["create"]["constituent"].push_back("name");
        mustbespecifiedatcreation["create"]["reaction"] = vector<string>();
            mustbespecifiedatcreation["create"]["reaction"].push_back("name");
        mustbespecifiedatcreation["create"]["reaction_parameter"] = vector<string>();
            mustbespecifiedatcreation["create"]["reaction_parameter"].push_back("name");
    mustbespecifiedatcreation["loadtemplate"] = map<string, vector<string>>();
        mustbespecifiedatcreation["loadtemplate"]["*"] = vector<string>();
            mustbespecifiedatcreation["loadtemplate"]["*"].push_back("filename");
    mustbespecifiedatcreation["addtemplate"] = map<string, vector<string>>();
        mustbespecifiedatcreation["addtemplate"]["*"] = vector<string>();
            mustbespecifiedatcreation["addtemplate"]["*"].push_back("filename");
    mustbespecifiedatcreation["setasparameter"] = map<string, vector<string>>();
        mustbespecifiedatcreation["setasparameter"]["*"] = vector<string>();
            mustbespecifiedatcreation["setasparameter"]["*"].push_back("object");
            mustbespecifiedatcreation["setasparameter"]["*"].push_back("quantity");
            mustbespecifiedatcreation["setasparameter"]["*"].push_back("parametername");
    mustbespecifiedatcreation["echo"] = map<string, vector<string>>();
        mustbespecifiedatcreation["echo"]["*"] = vector<string>();
            mustbespecifiedatcreation["echo"]["*"].push_back("object");
    mustbespecifiedatcreation["setvalue"] = map<string, vector<string>>();
        mustbespecifiedatcreation["setvalue"]["*"] = vector<string>();
            mustbespecifiedatcreation["setvalue"]["*"].push_back("object");
            mustbespecifiedatcreation["setvalue"]["*"].push_back("quantity");
            mustbespecifiedatcreation["setvalue"]["*"].push_back("value");
    mustbespecifiedatcreation["solve"] = map<string, vector<string>>();
        mustbespecifiedatcreation["solve"]["*"] = vector<string>();
    mustbespecifiedatcreation["optimize"] = map<string, vector<string>>();
        mustbespecifiedatcreation["optimize"]["*"] = vector<string>();
	mustbespecifiedatcreation["writeoutput"] = map<string, vector<string>>();
		mustbespecifiedatcreation["write"]["outputs"] = vector<string>();
		mustbespecifiedatcreation["write"]["errors"] = vector<string>();
			mustbespecifiedatcreation["write"]["outputs"].push_back("filename");
			mustbespecifiedatcreation["write"]["errors"].push_back("filename");




}

void Script::Append(const Command &c)
{
    commands.push_back(c);
    commands[commands.size()-1].SetParent(this);
}
