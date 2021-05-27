#include "Parameter.h"
#include "Expression.h"
#include "System.h"



Parameter::Parameter(): Object::Object()
{
    //ctor
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

    for (int i=0; i<_quan.size(); i++ ) { out +=  _quan[i]; if (i<_quan.size()-1) out += ","; }
    out += "}\n";
    out += aquiutils::tabs(_tabs) + "locations: {";
    for (int i=0; i<_location.size(); i++ ) { out +=  _location[i]; if (i<_location.size()-1) out += ","; }
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
        for (int i=0; i<_location.size(); i++ ) { out +=  _location[i]; if (i<_location.size()-1) out += ","; }
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
        for (int i=0; i<_quan.size(); i++ ) { out +=  _quan[i]; if (i<_quan.size()-1) out += ","; }
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
    for (int i=0; i<_location.size(); i++)
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
