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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#ifndef RULE_H
#define RULE_H
#include "Condition.h"

struct _condplusresult
{
    Condition condition;
    Expression result;
};

class Rule
{
    public:
        Rule();
        virtual ~Rule();
        Rule(const Rule &S);
        Rule& operator=(const Rule& S);
        void Append(const std::string &condition, const std::string &result);
        void Append(const Condition &condition, const Expression &result);
        double calc(Object *W, const Timing &tmg);
        std::string GetLastError() {return last_error;}
        _condplusresult *operator[](int i) {return &rules[i];}
        std::string ToString(int _tabs = 0) const;
    protected:

    private:
        std::vector<_condplusresult> rules;
        std::string last_error;
};

#endif // RULE_H
