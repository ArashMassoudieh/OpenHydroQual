#include "RxnParameter.h"
#include "System.h"

RxnParameter::RxnParameter(): Object::Object()
{
    SetObjectType(object_type::reaction_parameter);
}

RxnParameter::~RxnParameter()
{
    //dtor
}

RxnParameter::RxnParameter(const RxnParameter& other):Object::Object(other)
{
    Object::operator=(other);
}

RxnParameter& RxnParameter::operator=(const RxnParameter& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    return *this;
}

bool RxnParameter::SetName(const string &newname, bool setprop)
{
    if (newname.find('(') != std::string::npos || newname.find(')') != std::string::npos)
    {
        return false;
    }
    if (GetParent()!=nullptr)
    {
        if (newname!=GetName())
            GetParent()->RenameConstituents(GetName(),newname);
    }

    Object::SetName(newname,setprop);
    return true;
}
