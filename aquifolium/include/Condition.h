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


#ifndef CONDITION_H
#define CONDITION_H
#include "Expression.h"


class Object;

enum class _oprtr{lessthan, greaterthan};

class Condition
{
    public:
        Condition();
        Condition(const string &str);
        Condition(const Condition &S);
        Condition& operator=(const Condition&);
        virtual ~Condition();
        bool calc(Object *W, const Expression::timing &tmg);
        string GetLastError() {return last_error;}
        string ToString(int _tabs = 0) const;
        unsigned int Count() { return exr.size();  }
    protected:

    private:
        vector<Expression> exr;
        vector<_oprtr> oprtr;
        string last_error;
};

#endif // CONDITION_H
