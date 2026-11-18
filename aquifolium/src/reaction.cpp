/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
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


#include "reaction.h"
#include "System.h"

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

// A reaction's "copytoblocks" quantities (e.g. surface_release's release_coefficient) live in every block
// as "<reaction name>:<quantity>". Renaming the reaction must rename them too, as constituents and
// reaction parameters do; otherwise the saved model refers to properties that a reload does not create.
bool Reaction::SetName(const string &newname, bool setprop)
{
    if (GetParent()!=nullptr && !GetName().empty() && newname!=GetName())
        GetParent()->RenameConstituents(GetName(),newname);

    return Object::SetName(newname,setprop);
}
