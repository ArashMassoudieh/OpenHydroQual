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


#ifndef OBJECTIVE_FUNCTION_H
#define OBJECTIVE_FUNCTION_H

#include "Expression.h"
#include "TimeSeries.h"
#include "Object.h"

enum class objfunctype {Integrate, Value, Maximum, Variance, Exceedance};

class System;

class Objective_Function: public Object
{
    public:
        Objective_Function();
        Objective_Function(System *_system) ;
        virtual ~Objective_Function();
        Objective_Function(const Objective_Function& other);
        Objective_Function(System *_system, const Expression &expr, const string &loc);
        Objective_Function& operator=(const Objective_Function& other);
        double GetValue(const Timing &tmg = Timing::present); //return the current value of the objective function
        double GetObjective(); //calculates the objective function
        void append_value(double t, double val); //append a value to the stored time-series;
        void append_value(double t); //append the evaluated expression value to the stored time-series;
        void SetSystem(System *_system) {system = _system;}
        string GetLastError() {return lasterror;}
        void SetLastError(const string &lerror) {lasterror = lerror;}
        TimeSeries<timeseriesprecision> *GetTimeSeries() {return &stored_time_series;}
        void SetTimeSeries(const TimeSeries<timeseriesprecision> &X)
        {
            stored_time_series = X;
        }
        double Weight();
        double Percentile();
        void SetLocation(const string &loc) {location = loc;}
        string GetLocation() {
            if (Variable("object"))
            {
                if (location!=Variable("object")->GetProperty())
                    location = Variable("object")->GetProperty();
            }
            return location;
        }
        void SetExpression(const Expression &exp) {expression = exp;}
        bool SetProperty(const string &prop, const string &val);
        double Value() {return current_value;}
        void SetOutputItem(const string& s) { outputitem = s; }
        string GetOutputItem() { return outputitem; }
        vector<string> ItemswithOutput();
        double GetObjectiveValue() {
            return objective_value;
        }
    protected:

    private:
        string location; // location at which the objective function will be evaluated
        Expression expression; // Function
        TimeSeries<timeseriesprecision> stored_time_series; // Stored time series values;
        System *system; // pointer to the system the objective function is evaluated at
        string lasterror;
        objfunctype type = objfunctype::Integrate;
        double current_value=0;
        string outputitem=""; 
        double objective_value=0;
         //the last error that occurred
};

#endif // OBJECTIVE_FUNCTION_H
