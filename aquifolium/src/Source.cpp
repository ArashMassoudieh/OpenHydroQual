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


#include "Source.h"
#include "Object.h"

Source::Source() : Object::Object()
{
    SetObjectType(object_type::source);
}

Source::~Source()
{
    //dtor
}

Source::Source(const Source& other):Object::Object(other)
{

}

Source& Source::operator=(const Source& rhs)
{

    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    return *this;
}

double Source::GetValue(Object *obj)
{
    double coeff;
    if (Variable("coefficient")==nullptr)
        coeff = 1;
    else
        coeff = Variable("coefficient")->CalcVal(obj,Expression::timing::present);

    double value;
    if (Variable("timeseries")!=nullptr)
        value = Variable("timeseries")->CalcVal(Expression::timing::present);
    else
        value = 1;

    double rate;
    if (Variable("rate")!=nullptr)
        rate = Variable("rate")->CalcVal(Expression::timing::present);
    else
        rate = 1;



    return coeff*value*rate;

}
