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


#ifndef LINK_H
#define LINK_H
#include <string>
#include <map>
#include "Quan.h"
#include "Object.h"

using namespace std;

class Block;
class System;

class Link: public Object
{
    public:
        Link();
        Link(System *parent);
        virtual ~Link();
        Link(const Link& other);
        Link& operator=(const Link& other);
        Object* GetConnectedBlock(ExpressionNode::loc l) 
        {
            if (l == ExpressionNode::loc::source)
                return Get_s_Block();
            if (l == ExpressionNode::loc::destination)
                return Get_e_Block();
            return nullptr;
        };
        string toCommand();
        vector<string> GetAllRequieredStartingBlockProperties(); 
        vector<string> GetAllRequieredDestinationBlockProperties();
        bool ShiftLinkedBlock(int shift, ExpressionNode::loc loc);
    protected:

    private:


};

#endif // LINK_H
