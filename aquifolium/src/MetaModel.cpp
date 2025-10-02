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


#include "MetaModel.h"
#include <json/json.h>
#include <QDebug>


MetaModel::MetaModel():map<string, QuanSet>()
{
    //ctor
}

MetaModel::~MetaModel()
{
    //dtor
}

MetaModel::MetaModel(const MetaModel& other):map<string, QuanSet>(other)
{

}

MetaModel& MetaModel::operator=(const MetaModel& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    map<string, QuanSet>::operator=(rhs);
    return *this;
}

bool MetaModel::Append(const string &s, const QuanSet &q)
{
    if (count(s)>0)
    {
        last_error = "Object type " + s + " Already exist";
        errors.push_back(last_error);
        return false;
    }
    else
        map<string, QuanSet>::operator[](s) = q;
    return true;
}

QuanSet& MetaModel::at(const string &typ)
{
    if (count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        static QuanSet empty;
        return empty;
    }
    else
        return map<string, QuanSet>::at(typ);
}

const QuanSet& MetaModel::at(const string &typ) const
{
    if (count(typ)==0)
    {
        cout << "Type " + typ + " was not found!";
        static QuanSet empty;
        return empty;
    }
    else
        return map<string, QuanSet>::at(typ);
}

QuanSet* MetaModel::operator[] (const string &typ)
{
    if (count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(this->at(typ));
}

QuanSet* MetaModel::GetItem(const string &typ)
{
    if (count(typ)==0)
    {
        last_error = "Type " + typ + " was not found!";
        return nullptr;
    }
    else
        return &(this->at(typ));
}

#pragma warning(disable : 4996)
bool MetaModel::GetFromJsonFile(const string &filename)
{
    //qDebug()<<"Clearing!";
    Clear();
    //qDebug()<<"Cleared!";
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    std::ifstream file(filename);
    if (!file.good())
    {
        //cout << "File " + filename + " was not found!";
        return false;
    }
    //else
    //    qDebug()<<"File is good!";

    //qDebug()<<"File loaded!";
    if(!Json::parseFromStream(builder, file, &root, &errs)){
        std::cout  << "Failed to parse configuration\n"
                  << errs;
        last_error = "Failed to parse configuration\n";
        return false;
    }

    //qDebug()<<"Parsed!";


    for (Json::ValueIterator object_types=root.begin(); object_types!=root.end(); ++object_types)
    {
        //qDebug()<<QString::fromStdString(object_types.key().asString());

        if (object_types.key().asString()=="solutionorder")
        {
            for (Json::Value::ArrayIndex i = 0; i != root["solutionorder"].size(); i++)
            {
                solvevariableorder.push_back(root["solutionorder"][i].asString());
            }
        }
        QuanSet quanset(object_types);
        //qDebug()<<QString::fromStdString(object_types.key().asString());
        Append(object_types.key().asString(),quanset);
    }
    return true;
}

bool MetaModel::AppendFromJsonFile(const string &filename)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    std::ifstream file(filename);
    if (!file.good())
    {
        std::cout << "File " + filename + " was not found!";
        return false;
    }

    if (!Json::parseFromStream(builder, file, &root, &errs)) {
        last_error = errs;
        return false;
    }

    for (Json::ValueIterator object_types=root.begin(); object_types!=root.end(); ++object_types)
    {
        QuanSet quanset(object_types);
        Append(object_types.key().asString(),quanset);
    }
    return true;
}


void MetaModel::Clear()
{
    solvevariableorder.clear();
    clear();
    errors.clear();
}

string MetaModel::ToString(int _tabs) const
{
    string out = aquiutils::tabs(_tabs) + "root:\n";
    out += aquiutils::tabs(_tabs) + "{\n";
    for (map<string, QuanSet>::const_iterator it = cbegin(); it!=cend(); it++)
        out += map<string, QuanSet>::at(it->first).ToString(_tabs+1) + "\n";
    out += "}\n";
    return out;
}

void MetaModel::RenameConstituent(const string &oldname, const string &newname)
{
    vector<string> oldfullname;
    vector<string> newfullname;
    for (map<string,QuanSet>::iterator model = begin(); model!=end(); model++ )
    {   for (unordered_map<string, Quan>::iterator it = model->second.begin(); it != model->second.end(); it++)
        {
            if (aquiutils::split(it->first,':').size()==2)
            {   if (aquiutils::split(it->first,':')[0]==oldname)
                {
                    if (aquiutils::lookup(oldfullname,it->first)==-1)
                    {   oldfullname.push_back(it->first);
                        newfullname.push_back(newname + ":" + aquiutils::split(it->first,':')[1]);
                    }
                }
            }
        }
    }

    for (map<string,QuanSet>::iterator model = begin(); model!=end(); model++ )
    {   for (unsigned int i=0; i<oldfullname.size(); i++)
        {
            model->second.RenameProperty(oldfullname[i],newfullname[i]);
            if (model->second.count(newfullname[i])>0)
            {   model->second.at(newfullname[i]).Description() = newname + ":" + aquiutils::split(newfullname[i],':')[1];
                model->second.at(newfullname[i]).Description(true) = newname + ":" + aquiutils::split(newfullname[i],':')[1];
            }
        }
    }


}
