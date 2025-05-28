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


#include "Parameter.h"
#include "Expression.h"
#include "System.h"



Parameter::Parameter(): Object::Object()
{
    SetObjectType(object_type::parameter);
}

Parameter::~Parameter()
{
    //dtor
}

Parameter::Parameter(const Parameter& other):Object::Object(other)
{
    Object::operator=(other);
    _location = other._location;
    _quan = other._quan;
    value = other.value;
    last_error = other.last_error;
}

Parameter& Parameter::operator=(const Parameter& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    _location = rhs._location;
    _quan = rhs._quan;
    value = rhs.value;
    last_error = rhs.last_error;
    return *this;
}

string Parameter::toString(int _tabs)
{
    string out = aquiutils::tabs(_tabs) + "Name: " + GetName() + "\n";
    out += aquiutils::tabs(_tabs) + "low: " + aquiutils::numbertostring(Object::GetVal("low")) + "\n";
    out += aquiutils::tabs(_tabs) + "high: " + aquiutils::numbertostring(Object::GetVal("high")) + "\n";
    out += aquiutils::tabs(_tabs) + "quans: {";

    for (unsigned int i=0; i<_quan.size(); i++ ) { out +=  _quan[i]; if (i<_quan.size()-1) out += ","; }
    out += "}\n";
    out += aquiutils::tabs(_tabs) + "locations: {";
    for (unsigned int i=0; i<_location.size(); i++ ) { out +=  _location[i]; if (i<_location.size()-1) out += ","; }
    out += "}\n";
    out += aquiutils::tabs(_tabs) + "prior distribution: " + Object::Variable("prior_distribution")->GetStringValue() + "\n";
    out += aquiutils::tabs(_tabs) + "value: " + aquiutils::numbertostring(value) + "\n";
    return out;
}

bool Parameter::HasQuantity(const string &qntty)
{
    if (aquiutils::tolower(qntty)!="location" && aquiutils::tolower(qntty)!="quan" && aquiutils::tolower(qntty)!="range" && aquiutils::tolower(qntty)!="low" && aquiutils::tolower(qntty)!="prior_distribution" && aquiutils::tolower(qntty)!="name" && aquiutils::tolower(qntty)!="value")
        return false;
    else
        return true;
}

string Parameter::variable(const string &qntty)
{
    if (aquiutils::tolower(qntty)=="location")
    {
        string out;
        out += "{";
        for (unsigned int i=0; i<_location.size(); i++ ) { out +=  _location[i]; if (i<_location.size()-1) out += ","; }
        out += "}";
        return out;
    }
    if (aquiutils::tolower(qntty)=="name")
        return GetName();

    if (aquiutils::tolower(qntty)=="low")
        return aquiutils::numbertostring(Object::GetVal("low"));
    if (aquiutils::tolower(qntty)=="high")
        return aquiutils::numbertostring(Object::GetVal("low"));

    if (aquiutils::tolower(qntty)=="quan")
    {
        string out;
        out += "{";
        for (unsigned int i=0; i<_quan.size(); i++ ) { out +=  _quan[i]; if (i<_quan.size()-1) out += ","; }
        out += "}";
        return out;
    }

    if (aquiutils::tolower(qntty)=="prior_distribution")
        return Object::Variable("prior_distribution")->GetStringValue();

    if (aquiutils::tolower(qntty)=="value")
        return aquiutils::numbertostring(value);

    if (aquiutils::tolower(qntty)=="range")
        return "{" + aquiutils::numbertostring(Object::GetVal("low")) +"," + aquiutils::numbertostring(Object::GetVal("high")) + "}";

    return "";
}

bool Parameter::SetName(string s)
{
    Object::SetName(s);
    return true;
}

bool Parameter::RemoveLocationQuan(const string &loc, const string &quan)
{
    for (unsigned int i=0; i<_location.size(); i++)
    {
        if (_location[i]==loc && _quan[i]==quan)
        {
            _location.erase(_location.begin()+i);
            _quan.erase(_quan.begin()+i);
            return true;
        }
    }
    return false;


}

double Parameter::ExpandedLow(const double &factor)
{
    if (GetPriorDistribution()!="log-normal")
    {
        return GetRange().low - (GetRange().high-GetRange().low)*(factor-1);
    }
    else
    {
        return exp(log(GetRange().low) - (log(GetRange().high)-log(GetRange().low))*(factor-1));
    }
}

double Parameter::ExpandedHigh(const double &factor)
{
    if (GetPriorDistribution()!="log-normal")
    {
        return GetRange().high + (GetRange().high-GetRange().low)*(factor-1);
    }
    else
    {
        return exp(log(GetRange().high) + (log(GetRange().high)-log(GetRange().low))*(factor-1));
    }
}

CTimeSeries<double> Parameter::PriorDistribution(unsigned int nbins)
{
    CTimeSeries<double> prior_dist;
    prior_dist.name = "Prior density";
    for (unsigned int i=0; i<nbins; i++)
    {
        double x = ExpandedLow() + i*(ExpandedHigh() - ExpandedLow())/double(nbins);
        prior_dist.append(x,CalcPriorProbability(x));
    }
    return prior_dist;
}

double Parameter::CalcPriorProbability(const double &x)
{
    if (GetPriorDistribution()=="normal")
        return 1.0/std()/sqrt(2*PI)*exp(-pow((x-mean())/std(),2)/2.0);
    if (GetPriorDistribution()=="log-normal")
    {   if (x>0)
            return 1.0/(geostd()*sqrt(2*PI)*x)*exp(-pow((log(x)-log(geomean()))/geostd(),2)/2.0);
        else
            return 1e-30;
    }
    if (GetPriorDistribution()=="uniform")
    {   if (x<Range().high && x>Range().low)
            return 1.0/(Range().high-Range().low);
        else
            return 1e-30;
    }
}

double Parameter::CalcLogPriorProbability(const double &x)
{
    if (GetPriorDistribution()=="normal")
        return -log(std()/sqrt(2*PI))-pow((x-mean())/std(),2)/2.0;
    if (GetPriorDistribution()=="log-normal")
    {   if (x>0)
            return -log((geostd()*sqrt(2*PI)*x))-pow((log(x)-log(geomean()))/geostd(),2)/2.0;
        else
            return log(1e-30);
    }
    if (GetPriorDistribution()=="uniform")
    {   if (x<Range().high && x>Range().low)
            return -log((Range().high-Range().low));
        else
            return log(1e-30);
    }
}
