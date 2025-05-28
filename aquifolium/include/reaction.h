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


#ifndef REACTION_H
#define REACTION_H

#include "Object.h"
#include "Expression.h"
#include <map>

class Reaction : public Object
{
public:
    Reaction();
    Reaction(System *parent);
    Reaction(const Reaction& other);
    Reaction& operator=(const Reaction& rhs);
    virtual ~Reaction();
    Expression *RateExpression() {
        if (Variable("rate_expression")==nullptr)
            return nullptr;
        else
            return Variable("rate_expression")->GetExpression();

    }
    Expression *Stoichiometric_Constant(const string &constituent)
    {
         if (Variable(constituent+":stoichiometric_constant")==nullptr)
             return nullptr;
         else
             return Variable(constituent+":stoichiometric_constant")->GetExpression();
    }
    bool RenameConstituents(const string& oldname, const string& newname);

private:



};

#endif // REACTION_H
