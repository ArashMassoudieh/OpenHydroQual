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
        void ClearStoredTimeSeries();
        CTimeSeriesSet<double> TimeSeries();
        CVector Objective_Values();
        void Update(double t);
        string LastError() {return lasterror;}
        CTimeSeriesSet<timeseriesprecision> GetTimeSeriesSet();
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
        double EvaluateExpression(const std::string& expression);
        void AppendExpression(const std::string& name, const std::string& expression);
        std::map<std::string, double> EvaluateAllExpressions();
    protected:

    private:
        vector<Objective_Function> objectivefunctions;
        string lasterror;
        std::map<std::string, std::string> expression_map;


};

double EvaluateMathExpression(const std::string& expr);
int precedence(char op);
bool isRightAssociative(char op);
double applyOp(double a, double b, char op);
bool isFunctionChar(char c);
double EvaluateFunction(const std::string& name, const std::vector<double>& args);
double parseNumber(std::istringstream& input);
std::string parseIdentifier(std::istringstream& input);
std::vector<double> parseFunctionArgs(std::istringstream& input);
double EvaluateMathExpression(const std::string& expr);




#endif // OBJECTIVE_FUNCTION_SET_H
