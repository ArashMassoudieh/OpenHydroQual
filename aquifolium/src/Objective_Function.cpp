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


#include "Objective_Function.h"
#include "System.h"


Objective_Function::Objective_Function(): Object::Object()
{
    SetObjectType(object_type::objective_function);
}

Objective_Function::Objective_Function(System *_system)
{
    system = _system;
    SetObjectType(object_type::objective_function);
}

Objective_Function::Objective_Function(System *_system, const Expression &expr, const string &loc)
{
    SetType("Objective_Function");
    system = _system;
    expression = expr;
    location = loc;
    SetObjectType(object_type::objective_function);
    
}

Objective_Function::~Objective_Function()
{
    //dtor
}

Objective_Function::Objective_Function(const Objective_Function& other)
{
    Object::operator=(other);
    expression = other.expression;
    location = other.location;
    type = other.type;
    stored_time_series = other.stored_time_series;
}

Objective_Function& Objective_Function::operator=(const Objective_Function& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    expression = rhs.expression;
    location = rhs.location;
    type = rhs.type;
    stored_time_series.clear();
    return *this;
}

bool Objective_Function::SetProperty(const string &prop, const string &val)
{
    if (aquiutils::tolower(prop)=="expression")
    {
        expression = Expression(val);

    }
    if (aquiutils::tolower(prop)=="location" || aquiutils::tolower(prop)=="object")
    {
        location = val;

    }
    if (aquiutils::tolower(prop)=="method")
    {
        if (aquiutils::tolower(val)=="integrate") {type = objfunctype::Integrate;}
        if (aquiutils::tolower(val)=="value") {type = objfunctype::Value;}
        if (aquiutils::tolower(val) == "maximum") { type = objfunctype::Maximum; }
        if (aquiutils::tolower(val) == "variance") { type = objfunctype::Variance; }
        if (aquiutils::tolower(val) == "exceedance") { type = objfunctype::Exceedance; }
        lasterror = "Type '" + val + "' was not recognized!";
    }
    return Object::SetProperty(prop,val);
    return false;
}

double Objective_Function::GetValue(const Timing &tmg)
{
    if (expression.param_constant_expression == "") 
        expression = Variable("expression")->GetProperty();
    
    if (system->block(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->block(location),tmg);
        return current_value;
    }
    if (system->link(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->link(location),tmg,true);
        return current_value;
    }
    lasterror = "Location " + location + "was not found in the system!";
    return 0;
}

void Objective_Function::append_value(double t, double val)
{
    stored_time_series.append(t,val);
    return;
}

void Objective_Function::append_value(double t)
{
    stored_time_series.append(t,GetValue(Timing::present));
    return;
}

vector<string> Objective_Function::ItemswithOutput()
{
    vector<string> s = Object::ItemswithOutput();
    s.push_back("Time Series");
    return s;
}

double Objective_Function::GetObjective()
{
    SetProperty("method", Variable("method")->GetProperty());
    stored_time_series = stored_time_series.make_uniform(Parent()->dt0());
    if (type == objfunctype::Integrate)
    {   objective_value = stored_time_series.integrate();
        return objective_value;
    }
    else if (type == objfunctype::Value)
    {
        objective_value = GetValue(Timing::present);
        return objective_value;
    }
    else if (type == objfunctype::Maximum)
    {
        objective_value = stored_time_series.maxC();
        return objective_value;
    }
    else if (type == objfunctype::Variance)
    {   objective_value = stored_time_series.variance();
        return objective_value;

    }
    else if (type == objfunctype::Exceedance)
    {   objective_value = stored_time_series.percentile(1-Percentile());
        return objective_value;

    }
    else
    {   objective_value = 0;
        return objective_value;
    }
}

double Objective_Function::Weight()
{
    if (GetVars()->Count("weight")>0)
    {
        return Variable("weight")->GetVal();
    }
    else
        return 1;
}

double Objective_Function::Percentile()
{
    if (GetVars()->Count("exceedance_probability")>0)
    {
        return Variable("exceedance_probability")->GetVal();
    }
    else
        return 0.5;
}




