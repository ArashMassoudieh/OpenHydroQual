#include "Objective_Function_Set.h"

Objective_Function_Set::Objective_Function_Set()
{
    //ctor
}

Objective_Function_Set::~Objective_Function_Set()
{
    //dtor
}

Objective_Function_Set::Objective_Function_Set(const Objective_Function_Set& other)
{
    objectivefunctions = other.objectivefunctions;
}

Objective_Function_Set& Objective_Function_Set::operator=(const Objective_Function_Set& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    objectivefunctions = rhs.objectivefunctions;
    return *this;
}

void Objective_Function_Set::Append(const string &name, const Objective_Function &o_func, double weight)
{
    objectivefunctions.push_back(o_func);
    objectivefunctions[objectivefunctions.size()-1].SetVal("weight",weight);
    objectivefunctions[objectivefunctions.size()-1].SetName(name);
    return;
}
Objective_Function* Objective_Function_Set::operator[](string name)
{
    for (unsigned int i=0; i<objectivefunctions.size(); i++)
        if (objectivefunctions[i].GetName() == name)
            return &objectivefunctions[i];

     lasterror = "Objective Function " + name + " does not exist!";
     return nullptr;
}

double Objective_Function_Set::Calculate()
{
    double out = 0;
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        out += objectivefunctions[i].GetObjective()*objectivefunctions[i].Weight();

    return out;
}

void Objective_Function_Set::ClearStoredTimeSeries()
{
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].GetTimeSeries()->clear();
}

CTimeSeriesSet<double> Objective_Function_Set::TimeSeries()
{
    CTimeSeriesSet<double> output(objectivefunctions.size());
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
    {
        output.BTC[i] = *objectivefunctions[i].GetTimeSeries();
        output.setname(i,objectivefunctions[i].GetName());
    }
    return output;
}

CVector Objective_Function_Set::Objective_Values()
{
    CVector values(objectivefunctions.size());
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        values[i] = objectivefunctions[i].GetObjectiveValue();
    return values;
}

void Objective_Function_Set::Update(double t)
{

    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].append_value(t);
    return;
}

CTimeSeriesSet<timeseriesprecision> Objective_Function_Set::GetTimeSeriesSet()
{
    CTimeSeriesSet<timeseriesprecision> out;
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        out.append(*objectivefunctions[i].GetTimeSeries(),objectivefunctions[i].GetName());
    return out;
}

void Objective_Function_Set::SetSystem(System* s)
{
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].SetSystem(s);
}

void Objective_Function_Set::clear()
{
    objectivefunctions.clear();
}

bool Objective_Function_Set::erase(int i)
{
    if (i >= objectivefunctions.size()) return false;
    objectivefunctions.erase(objectivefunctions.begin() + i);

    return true;
}


