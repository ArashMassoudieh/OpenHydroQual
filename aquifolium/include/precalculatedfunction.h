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


#ifndef PRECALCULATEDFUNCTION_H
#define PRECALCULATEDFUNCTION_H

#include "BTC.h"

class PreCalculatedFunction : public CTimeSeries<double>
{
public:
    PreCalculatedFunction();
    PreCalculatedFunction(const PreCalculatedFunction& other);
    PreCalculatedFunction& operator=(const PreCalculatedFunction& rhs);
    virtual ~PreCalculatedFunction();
    double xmax() const {return x_max;}
    double xmin() const {return x_min;}
    bool setminmax(const double &xmin, const double &xmax)
    {
        if (xmax>xmin)
        {
            x_min = xmin;
            x_max = xmax;
            return true;
        }
        return false;
    }
    void SetLogarithmic(bool x) {logarithmic = x;}
    bool Logarithmic() const {return logarithmic;}
    bool SetIndependentVariable(const string &indvar) {indepenentvariable = indvar;return true;}
    string IndependentVariable() const {return indepenentvariable;}
    bool Initiated() const {return initiated;}
    void SetInitiated(bool x) {initiated = x;}
private:
    string indepenentvariable = "";
    bool logarithmic=false;
    double x_min;
    double x_max;
    bool initiated = false;
};

#endif // PRECALCULATEDFUNCTION_H
