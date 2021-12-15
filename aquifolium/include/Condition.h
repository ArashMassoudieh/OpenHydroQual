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
