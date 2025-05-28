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


#include "constituent.h"
#include "System.h"
Constituent::Constituent(): Object::Object()
{
    SetObjectType(object_type::constituent);
}

Constituent::~Constituent()
{
    //dtor
}

Constituent::Constituent(const Constituent& other):Object::Object(other)
{
    SetObjectType(object_type::constituent);
    Object::operator=(other);
}

Constituent& Constituent::operator=(const Constituent& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    return *this;
}

bool Constituent::SetName(const string &newname, bool setprop)
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



