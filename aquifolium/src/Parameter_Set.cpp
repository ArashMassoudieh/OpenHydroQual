#include "Parameter_Set.h"

Parameter_Set::Parameter_Set()
{
    //ctor
}

Parameter_Set::~Parameter_Set()
{
    //dtor
}

Parameter_Set::Parameter_Set(const Parameter_Set& other)
{
    parameters = other.parameters;
    lasterror = other.lasterror;
}

Parameter_Set& Parameter_Set::operator=(const Parameter_Set& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    parameters = rhs.parameters;
    lasterror = rhs.lasterror;
    return *this;
}

void Parameter_Set::Append(const string &name, const Parameter &param)
{
    parameters.push_back(param);
    parameters[unsigned int( parameters.size()-1)].SetName(name);
    return;
}
Parameter* Parameter_Set::operator[](string name)
{
    for (int i=0; i<parameters.size(); i++)
        if (parameters[i].GetName() == name)
            return &parameters[i];

     lasterror = "Parameter " + name + " does not exist!";
     return nullptr;

}

Parameter* Parameter_Set::operator[](int i)
{
    return &parameters[i];
}

string Parameter_Set::getKeyAtIndex (int index){
    return parameters[index].GetName();
}

void Parameter_Set::clear()
{
    parameters.clear();
}

bool Parameter_Set::erase(int i)
{
    if (i >= parameters.size()) return false; 
    parameters.erase(parameters.begin() + i);

    return true; 
}
bool Parameter_Set::erase(const string& s)
{
    for (unsigned int i = 0; i < parameters.size(); i++)
        if (parameters[i].GetName() == s)
        {
            erase(i);
            return true;
        }
    return false; 

}



