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


#ifndef SCRIPT_H
#define SCRIPT_H


#include "Command.h"
#include <vector>
#include "ErrorHandler.h"
#include "GA.h"

class System;

using namespace std;

class Script
{
    public:
        Script();
        Script(const string &filename, System *sys = nullptr);
        bool CreateSystemFromQStringList(const QStringList &Script, System *sys);
        System* CreateSystem();
        bool CreateSystem(System *system);
        virtual ~Script();
        Command* operator[](int i) {return &commands[i];}
        System *GetSystem() {return system;}
        void SetSystem(System* _system) {system = _system;}
        void FillMustBeSpecified();
        map<string, map<string, vector<string>>> *MustBeSpecified()
        {
            return &mustbespecifiedatcreation;
        }
        void Append(const Command &c);
        void SetGA(CGA<System> *G) {GA = G;}
        CGA<System> *GetGA() {return GA;}
        int CommandsCount() {return commands.size();}
        vector<string> &Errors() {return errors;}
    protected:

    private:
        vector<Command> commands;
        vector<string> errors;
        System *system = nullptr;
        map<string, map<string, vector<string>>> mustbespecifiedatcreation;
        ErrorHandler errorhandler;
        CGA<System> *GA = nullptr;
        bool systemwascreated = false;
};

#endif // SCRIPT_H
