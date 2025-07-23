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


#include "Condition.h"
#include <QDebug>

using namespace aquiutils;

#include "Utilities.h"

Condition::Condition()
{
    //ctor
}

Condition::~Condition()
{
    //dtor
}

Condition::Condition(const Condition &S)
{
    exr = S.exr;
    oprtr = S.oprtr;
    last_error = S.last_error;
}

Condition& Condition::operator=(const Condition& S)
{
    exr = S.exr;
    oprtr = S.oprtr;
    last_error = S.last_error;

    return *this;
}

Condition::Condition(const string &str)
{
    if (aquiutils::split(str,'<').size()>1)
    {
        for (unsigned int i=0; i< aquiutils::split(str,'<').size(); i++)
        {
            exr.push_back(Expression(aquiutils::split(str,'<')[i]));
        }
        for (unsigned int i=0; i< aquiutils::split(str,'<').size()-1; i++)
            oprtr.push_back(_oprtr::lessthan);
        return;
    }
    else if (aquiutils::split(str,'>').size()>1)
    {
        for (unsigned int i=0; i< aquiutils::split(str,'>').size(); i++)
        {
            exr.push_back(Expression(aquiutils::split(str,'>')[i]));
        }
        for (unsigned int i=0; i< aquiutils::split(str,'>').size()-1; i++)
            oprtr.push_back(_oprtr::greaterthan);
        return;
    }
    else
        last_error = "Expression (" + str + ") does not contain a inequality operator!";
    return;

}

bool Condition::calc(Object *W, const Timing &tmg)
{
    bool out = true;
    for (unsigned int i=0; i<oprtr.size(); i++)
    {
        //qDebug()<<QString::fromStdString(this->ToString());
        if (oprtr[i] == _oprtr::greaterthan)
        {
            //qDebug()<<QString::fromStdString(exr[i].ToString())+">"+QString::fromStdString(exr[i+1].ToString());
            if (!(exr[i].calc(W, tmg)>exr[i+1].calc(W,tmg))) out = false;
        }
        if (oprtr[i] == _oprtr::lessthan)
        {
            //qDebug()<<QString::fromStdString(exr[i].ToString())+"<"+QString::fromStdString(exr[i+1].ToString());
            if (!(exr[i].calc(W, tmg)<exr[i+1].calc(W,tmg))) out = false;
        }
        //qDebug()<<QString::fromStdString(this->ToString());
    }
    return out;
}

string Condition::ToString(int _tabs) const
{
    string s = aquiutils::tabs(_tabs+1);
    for (unsigned int i=0; i<oprtr.size(); i++)
    {
        s += exr[i].ToString();
        if (oprtr[i]==_oprtr::lessthan) s+= "<";
        if (oprtr[i]==_oprtr::greaterthan) s+= ">";
    }
    s += exr[exr.size()-1].ToString();
    return s;
}
