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


#include "precalculatedfunction.h"

PreCalculatedFunction::PreCalculatedFunction() : CTimeSeries<double>::CTimeSeries()
{
    //ctor
}

PreCalculatedFunction::~PreCalculatedFunction()
{
    //dtor
}

PreCalculatedFunction::PreCalculatedFunction(const PreCalculatedFunction& other):CTimeSeries<double>::CTimeSeries(other)
{
    x_max=other.x_max;
    x_min=other.x_min;
    logarithmic = other.logarithmic;
    indepenentvariable=other.indepenentvariable;
    CTimeSeries::operator=(other);
}

PreCalculatedFunction& PreCalculatedFunction::operator=(const PreCalculatedFunction& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    CTimeSeries<double>::operator=(rhs);
    x_max=rhs.x_max;
    x_min=rhs.x_min;
    logarithmic = rhs.logarithmic;
    indepenentvariable=rhs.indepenentvariable;
    return *this;
}

