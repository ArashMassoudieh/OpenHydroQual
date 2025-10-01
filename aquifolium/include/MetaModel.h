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


#ifndef METAMODEL_H
#define METAMODEL_H

#include "QuanSet.h"
#include <map>

class MetaModel
{
    public:
        MetaModel();
        virtual ~MetaModel();
        MetaModel(const MetaModel& other);
        MetaModel& operator=(const MetaModel& other);
        bool Append(const string &s, const QuanSet &q);
        unsigned long Count(const string &s) {return metamodel.count(s);}
        map<string, QuanSet> *GetMetaModel() {return &metamodel;}
        QuanSet* operator[] (const string &typ);
        QuanSet* GetItem(const string &typ);
        std::map<string,QuanSet>::iterator find(const string &name) {return metamodel.find(name);}
        std::map<string,QuanSet>::iterator end() {return metamodel.end();}
        std::map<string,QuanSet>::iterator begin() {return metamodel.begin();}
        unsigned long size() {return metamodel.size();}
        bool GetFromJsonFile(const string &filename);
		bool AppendFromJsonFile(const string& filename);
        void Clear();
        string ToString(int _tabs=0);
        vector<string> solvevariableorder;
        string GetLastError() {return last_error; }
        void RenameConstituent(const string &oldname, const string &newname);
    protected:

    private:
        map<string, QuanSet> metamodel;
        string last_error;
        vector<string> errors;
};

#endif // METAMODEL_H
