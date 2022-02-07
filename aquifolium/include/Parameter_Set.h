#ifndef PARAMETER_SET_H
#define PARAMETER_SET_H

#include "Parameter.h"
#include <map>

class System;

class Parameter_Set
{
    public:
        Parameter_Set();
        virtual ~Parameter_Set();
        Parameter_Set(const Parameter_Set& other);
        Parameter_Set& operator=(const Parameter_Set& other);
        void Append(const string &name, const Parameter &param);
        Parameter* operator[](string name);
        Parameter* operator[](int i);
        string LastError() {return lasterror;}
        bool Contains(const string &name) {
            for (int i=0; i<parameters.size(); i++)
                if (parameters[i].GetName()==name)
                    return true;
            return false;
        }
        bool ApplyParameters(System *system);
        unsigned int size()
        {
            return parameters.size();
        }
        string getKeyAtIndex (int index);
        int count(const string &s) {
            int j=0;
            for (unsigned int i=0; i<parameters.size(); i++)
                if (parameters[i].GetName()==s)
                    j++;
            return  j;
        }
        void clear();
        bool erase(int i);
        bool erase(const string& s);
    protected:

    private:
        SafeVector<Parameter> parameters;
        string lasterror;
};

#endif // PARAMETER_SET_H
