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
        void Append(const string &condition, const string &result);
        void Append(const Condition &condition, const Expression &result);
        double calc(Object *W, const Expression::timing &tmg);
        string GetLastError() {return last_error;}
        _condplusresult *operator[](int i) {return &rules[i];}
        string ToString(int _tabs = 0) const;
    protected:

    private:
        vector<_condplusresult> rules;
        string last_error;
};

#endif // RULE_H
