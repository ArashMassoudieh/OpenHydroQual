#include "Link.h"
#include "Block.h"
#include "Object.h"

Link::Link():Object::Object()
{
    //ctor
}

Link::~Link()
{
    //dtor
}

Link::Link(const Link& other):Object::Object(other)
{
    //copy ctor
}

Link& Link::operator=(const Link& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    //assignment operator
    return *this;
}


string Link::toCommand()
{
    string out = "from=" + Object::GetConnectedBlock(Expression::loc::source)->GetName() + "," + "to=" + Object::GetConnectedBlock(Expression::loc::destination)->GetName() + ",";
    out += Object::toCommand();
    return out;
}

vector<string> Link::GetAllRequieredStartingBlockProperties()
{
    vector<string> s; 
    for (map<string, Quan>::iterator it = GetVars()->begin(); it!= GetVars()->end(); it++)
    {
        for (unsigned int i = 0; i< it->second.GetAllRequieredStartingBlockProperties().size(); i++)
        {
            s.push_back(it->second.GetAllRequieredStartingBlockProperties()[i]);
        }
    }
    return s; 
}
vector<string> Link::GetAllRequieredDestinationBlockProperties()
{
    vector<string> s;
    for (map<string, Quan>::iterator it = GetVars()->begin(); it != GetVars()->end(); it++)
    {
        for (unsigned int i = 0; i< it->second.GetAllRequieredEndingBlockProperties().size(); i++)
        {
            s.push_back(it->second.GetAllRequieredEndingBlockProperties()[i]);
        }
    }
    return s;
}

bool Link::ShiftLinkedBlock(int shift, Expression::loc loc)
{
    if (loc == Expression::loc::source)
        SetSBlockNo( s_Block_No() + shift);
    if (loc == Expression::loc::destination)
        SetEBlockNo(e_Block_No() + shift);

    return true; 
}
