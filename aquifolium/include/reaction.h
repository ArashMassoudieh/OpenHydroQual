#ifndef REACTION_H
#define REACTION_H

#include "Object.h"
#include "Expression.h"
#include <map>

class Reaction : public Object
{
public:
    Reaction();
    Reaction(System *parent);
    Reaction(const Reaction& other);
    Reaction& operator=(const Reaction& rhs);
    virtual ~Reaction();
    Expression *RateExpression() {
        if (Variable("rate_expression")==nullptr)
            return nullptr;
        else
            return Variable("rate_expression")->GetExpression();

    }
    Expression *Stoichiometric_Constant(const string &constituent)
    {
         if (Variable(constituent+":stoichiometric_constant")==nullptr)
             return nullptr;
         else
             return Variable(constituent+":stoichiometric_constant")->GetExpression();
    }
    bool RenameConstituents(const string& oldname, const string& newname);

private:



};

#endif // REACTION_H
