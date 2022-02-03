#include "reaction.h"

Reaction::Reaction() : Object::Object()
{
    SetObjectType(object_type::reaction);
}

Reaction::~Reaction()
{
    //dtor
}

Reaction::Reaction(const Reaction& other):Object::Object(other)
{

    Object::operator=(other);

}

Reaction& Reaction::operator=(const Reaction& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    return *this;
}

bool Reaction::RenameConstituents(const string& oldname, const string& newname)
{
    bool succeed = true;
    succeed &= GetVars()->RenameQuantity(oldname, newname);
    Object::RenameConstituents(oldname, newname);
    return succeed;
}

