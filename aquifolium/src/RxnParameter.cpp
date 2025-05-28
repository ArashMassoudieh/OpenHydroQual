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
