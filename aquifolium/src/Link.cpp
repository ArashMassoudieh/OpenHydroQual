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


#include "Link.h"
#include "Block.h"
#include "Object.h"

Link::Link():Object::Object()
{
    SetObjectType(object_type::link);
}

Link::~Link()
{
    //dtor
}

Link::Link(const Link& other):Object::Object(other)
{

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
    string out = "from=" + Object::GetConnectedBlock(ExpressionNode::loc::source)->GetName() + "," + "to=" + Object::GetConnectedBlock(ExpressionNode::loc::destination)->GetName() + ",";
    out += Object::toCommand();
    return out;
}

vector<string> Link::GetAllRequieredStartingBlockProperties() const
{
    vector<string> s; 
    for (unordered_map<string, Quan>::const_iterator it = GetVars()->cbegin(); it!= GetVars()->cend(); it++)
    {
        for (unsigned int i = 0; i< it->second.GetAllRequieredStartingBlockProperties().size(); i++)
        {
            s.push_back(it->second.GetAllRequieredStartingBlockProperties()[i]);
        }
    }
    return s; 
}
vector<string> Link::GetAllRequieredDestinationBlockProperties() const
{
    vector<string> s;
    for (unordered_map<string, Quan>::const_iterator it = GetVars()->cbegin(); it != GetVars()->cend(); it++)
    {
        for (unsigned int i = 0; i< it->second.GetAllRequieredEndingBlockProperties().size(); i++)
        {
            s.push_back(it->second.GetAllRequieredEndingBlockProperties()[i]);
        }
    }
    return s;
}

bool Link::ShiftLinkedBlock(int shift, ExpressionNode::loc loc)
{
    if (loc == ExpressionNode::loc::source)
        SetSBlockNo( s_Block_No() + shift);
    if (loc == ExpressionNode::loc::destination)
        SetEBlockNo(e_Block_No() + shift);

    return true; 
}
