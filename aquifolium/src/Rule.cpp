#include "Rule.h"

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

void Rule::Append(const string &condition, const string &result)
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

double Rule::calc(Object *W, const Expression::timing &tmg)
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

string Rule::ToString(int _tabs) const
{
    string out = aquiutils::tabs(_tabs) + "{\n";
    for (unsigned int i=0; i<rules.size(); i++)
        out += aquiutils::tabs(_tabs) + rules[i].condition.ToString(_tabs+1) + ":" + rules[i].result.ToString()+ "\n";
    out += aquiutils::tabs(_tabs) + "}\n";
    return out;
}
