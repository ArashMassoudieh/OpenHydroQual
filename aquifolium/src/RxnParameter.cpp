#include "RxnParameter.h"

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
