#ifndef OBJECTIVE_FUNCTION_SET_H
#define OBJECTIVE_FUNCTION_SET_H

#include "Objective_Function.h"
#include <map>
#include "BTCSet.h"

class System;


class Objective_Function_Set
{
    public:
        Objective_Function_Set();
        virtual ~Objective_Function_Set();
        Objective_Function_Set(const Objective_Function_Set& other);
        Objective_Function_Set& operator=(const Objective_Function_Set& other);
        void Append(const string &name, const Objective_Function &o_func, double weight = 1);
        Objective_Function* operator[](string name);
        Objective_Function* operator[](int i) {return &objectivefunctions[i];}
        double Calculate();
        void Update(double t);
        string LastError() {return lasterror;}
        CTimeSeriesSet GetTimeSeriesSet();
        void SetSystem(System* s);
        void clear();
        bool erase(int i);
        unsigned int size()
        {
            return objectivefunctions.size();
        }
        bool Contains(const string &name) {
            for (unsigned int i=0; i<objectivefunctions.size(); i++)
                if (objectivefunctions[i].GetName()==name)
                    return true;
            return false;
        }
        int count(const string &s) {
            int j=0;
            for (unsigned int i=0; i<objectivefunctions.size(); i++)
                if (objectivefunctions[i].GetName()==s)
                    j++;
            return  j;
        }
    protected:

    private:
        vector<Objective_Function> objectivefunctions;
        string lasterror;


};

#endif // OBJECTIVE_FUNCTION_SET_H
