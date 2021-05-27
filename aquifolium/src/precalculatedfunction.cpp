#include "precalculatedfunction.h"

PreCalculatedFunction::PreCalculatedFunction() : CTimeSeries::CTimeSeries()
{
    //ctor
}

PreCalculatedFunction::~PreCalculatedFunction()
{
    //dtor
}

PreCalculatedFunction::PreCalculatedFunction(const PreCalculatedFunction& other):CTimeSeries::CTimeSeries(other)
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
    CTimeSeries::operator=(rhs);
    x_max=rhs.x_max;
    x_min=rhs.x_min;
    logarithmic = rhs.logarithmic;
    indepenentvariable=rhs.indepenentvariable;
    return *this;
}

