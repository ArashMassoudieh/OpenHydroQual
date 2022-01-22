#include "observation.h"

#include "System.h"


Observation::Observation(): Object::Object()
{
    //ctor
}

Observation::Observation(System *_system)
{
    system = _system;
}

Observation::Observation(System *_system, const Expression &expr, const string &loc)
{
    SetType("Observation");
    system = _system;
    expression = expr;
    location = loc;

}

Observation::~Observation()
{
    modeled_time_series.clear();
    observed_time_series.clear(); 
}

Observation::Observation(const Observation& other)
{
    Object::operator=(other);
    expression = other.expression;
    location = other.location;
    modeled_time_series = other.modeled_time_series;
    observed_time_series = other.observed_time_series;
}

Observation& Observation::operator=(const Observation& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    expression = rhs.expression;
    location = rhs.location;
    observed_time_series = rhs.observed_time_series;
    modeled_time_series.clear();
    return *this;
}

bool Observation::SetProperty(const string &prop, const string &val)
{
    if (aquiutils::tolower(prop)=="expression")
    {
        expression = Expression(val);

    }
    if (aquiutils::tolower(prop)=="location" || aquiutils::tolower(prop)=="object")
    {
        location = val;

    }

    return Object::SetProperty(prop,val);
    return false;
}

double Observation::GetValue(const Expression::timing &tmg)
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

double Observation::CalcMisfit()
{
    if (Variable("observed_data")->GetTimeSeries()!=nullptr)
    {
        if (Variable("error_structure")->GetProperty()=="normal")
        {
            Variable("observed_data")->GetTimeSeries()->writefile("observed.txt");
            modeled_time_series.writefile("modeled.txt");
            return diff2(Variable("observed_data")->GetTimeSeries(),modeled_time_series)/pow(Variable("error_standard_deviation")->GetVal(),2)+log(Variable("error_standard_deviation")->GetVal());
        }
        else if (Variable("error_structure")->GetProperty()=="log-normal" || Variable("error_structure")->GetProperty()=="lognormal")
        {
            return diff2(Variable("observed_data")->GetTimeSeries()->Log(1e-8),modeled_time_series.Log(1e-8))/pow(Variable("error_standard_deviation")->GetVal(),2)+log(Variable("error_standard_deviation")->GetVal());
        }
        else
            return 0;
    }
    else return 0;

}

void Observation::append_value(double t, double val)
{
    modeled_time_series.append(t,val);
    return;
}

void Observation::append_value(double t)
{
    modeled_time_series.append(t,GetValue(Expression::timing::present));
    return;
}

vector<string> Observation::ItemswithOutput()
{
    vector<string> s = Object::ItemswithOutput();
    s.push_back("Time Series");
    return s;
}






