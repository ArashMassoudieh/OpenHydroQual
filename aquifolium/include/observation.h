/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
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


#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "Expression.h"
#include "TimeSeries.h"
#include "Object.h"
#include "TimeSeriesSet.h"

class System;

class Observation: public Object
{
    public:
        Observation();
        Observation(System *_system) ;
        virtual ~Observation();
        Observation(const Observation& other);
        Observation(System *_system, const Expression &expr, const string &loc);
        Observation& operator=(const Observation& other);
        double GetValue(const Expression::timing &tmg = Expression::timing::present); //return the current value of the objective function
        void append_value(double t, double val); //append a value to the modeled time series
        void append_value(double t); //append the corresponding modeled value to the modeled time series
        void SetSystem(System *_system) {system = _system;}
        string GetLastError() {return lasterror;}
        void SetLastError(const string &lerror) {lasterror = lerror;}
        TimeSeries<timeseriesprecision> *GetTimeSeries() {return &modeled_time_series;}
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
        double CalcMisfit();
        void SetModeledTimeSeries(const TimeSeries<timeseriesprecision> &X) {modeled_time_series = X;}
        TimeSeries<timeseriesprecision>* GetModeledTimeSeries() {return &modeled_time_series;}
        void SetRealizations(const TimeSeriesSet<double>& rlztions) {realizations = rlztions;}
        TimeSeriesSet<double>& Realizations() {return realizations;}
        void SetPercentile95(const TimeSeriesSet<double>& rpct95) {percentile95 = rpct95;}
        TimeSeriesSet<double>& Percentile95() {return percentile95;}
        vector<double> fit_measures;

        // Effective-information scale for this observation's likelihood.
        // Residuals are serially correlated (sensor noise AND model structural
        // error), so N observations carry only N/tau_int independent pieces of
        // information. CalcMisfit divides the log-likelihood by this factor so
        // the posterior width reflects information actually present rather than
        // the raw sample count. 1.0 = no correction (default; previous
        // behaviour). Set externally per cycle -- it is a property of the
        // residual series, not of the model file, so it is deliberately NOT a
        // Quan and does not appear in templates.
        void SetLikelihoodScale(double s) { likelihood_scale = (s > 0.0 ? s : 1.0); }
        double GetLikelihoodScale() const { return likelihood_scale; }

    protected:

    private:
        string location; // location at which the objective function will be evaluated
        Expression expression; // Function
        TimeSeries<timeseriesprecision> modeled_time_series; // Stored time series values;
        TimeSeries<timeseriesprecision> observed_time_series; // Stored time series values;
        System *system; // pointer to the system the observation is evaluated at
        string lasterror;
        double current_value=0;
        double likelihood_scale=1.0; // tau_int; divides the log-likelihood
        string outputitem="";
        TimeSeriesSet<double> realizations;
        TimeSeriesSet<double> percentile95;

};
#endif // OBSERVATION_H
