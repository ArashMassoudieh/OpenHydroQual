#include "Source.h"
#include "Object.h"

Source::Source() : Object::Object()
{
    //ctor
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
