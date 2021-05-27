#include "Condition.h"

using namespace aquiutils;

Condition::Condition()
{
    //ctor
}

Condition::~Condition()
{
    //dtor
}

Condition::Condition(const Condition &S)
{
    exr = S.exr;
    oprtr = S.oprtr;
    last_error = S.last_error;
}

Condition& Condition::operator=(const Condition& S)
{
    exr = S.exr;
    oprtr = S.oprtr;
    last_error = S.last_error;

    return *this;
}

Condition::Condition(const string &str)
{
    if (split(str,'<').size()>1)
    {
        for (unsigned int i=0; i<split(str,'<').size(); i++)
        {
            exr.push_back(Expression(split(str,'<')[i]));
        }
        for (unsigned int i=0; i<split(str,'<').size()-1; i++)
            oprtr.push_back(_oprtr::lessthan);
        return;
    }
    else if (split(str,'>').size()>1)
    {
        for (unsigned int i=0; i<split(str,'>').size(); i++)
        {
            exr.push_back(Expression(split(str,'>')[i]));
        }
        for (unsigned int i=0; i<split(str,'>').size()-1; i++)
            oprtr.push_back(_oprtr::greaterthan);
        return;
    }
    else
        last_error = "Expression (" + str + ") does not contain a inequality operator!";
    return;

}

bool Condition::calc(Object *W, const Expression::timing &tmg)
{
    bool out = true;
    for (unsigned int i=0; i<oprtr.size(); i++)
    {
        if (oprtr[i] == _oprtr::greaterthan)
        {
            if (!(exr[i].calc(W, tmg)>exr[i+1].calc(W,tmg))) out = false;
        }
        if (oprtr[i] == _oprtr::lessthan)
        {
            if (!(exr[i].calc(W, tmg)<exr[i+1].calc(W,tmg))) out = false;
        }
    }
    return out;
}

string Condition::ToString(int _tabs) const
{
    string s = tabs(_tabs+1);
    for (unsigned int i=0; i<oprtr.size(); i++)
    {
        s += exr[i].ToString();
        if (oprtr[i]==_oprtr::lessthan) s+= "<";
        if (oprtr[i]==_oprtr::greaterthan) s+= ">";
    }
    s += exr[exr.size()-1].ToString();
    return s;
}
