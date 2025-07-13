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


#include "Rule.h"
#include "Utilities.h"

Rule::Rule()
{
    //ctor
}

Rule::~Rule()
{
    //dtor
}

Rule::Rule(const Rule &S)
{
    rules = S.rules;
}

Rule& Rule::operator=(const Rule& S)
{
    rules = S.rules;
    return *this;
}

void Rule::Append(const std::string &condition, const std::string &result)
{
    _condplusresult x;
    x.condition = Condition(condition);
    x.result = Expression(result);
    rules.push_back(x);
}

void Rule::Append(const Condition &condition, const Expression &result)
{
    _condplusresult x;
    x.condition = condition;
    x.result = result;
    rules.push_back(x);
}

double Rule::calc(Object *W, const Timing &tmg)
{
    for (unsigned int i=0;i<rules.size(); i++)
    {
        if (rules[i].condition.calc(W,tmg))
        {
            return rules[i].result.calc(W,tmg);
        }
    }
    return 0;
}

std::string Rule::ToString(int _tabs) const
{
    std::string out = aquiutils::tabs(_tabs) + "{\n";
    for (unsigned int i=0; i<rules.size(); i++)
        out += aquiutils::tabs(_tabs) + rules[i].condition.ToString(_tabs+1) + ":" + rules[i].result.ToString()+ "\n";
    out += aquiutils::tabs(_tabs) + "}\n";
    return out;
}
