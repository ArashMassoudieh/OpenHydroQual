#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "Expression.h"
#include "BTC.h"
#include "Object.h"
#include "BTCSet.h"

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
        CTimeSeries<timeseriesprecision> *GetTimeSeries() {return &modeled_time_series;}
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
        CTimeSeries<timeseriesprecision>* GetModeledTimeSeries() {return &modeled_time_series;}
        void SetRealizations(const CTimeSeriesSet<double>& rlztions) {realizations = rlztions;}
        CTimeSeriesSet<double>& Realizations() {return realizations;}
        void SetPercentile95(const CTimeSeriesSet<double>& rpct95) {percentile95 = rpct95;}
        CTimeSeriesSet<double>& Percentile95() {return percentile95;}
        double fit_mse = 0;
    protected:

    private:
        string location; // location at which the objective function will be evaluated
        Expression expression; // Function
        CTimeSeries<timeseriesprecision> modeled_time_series; // Stored time series values;
        CTimeSeries<timeseriesprecision> observed_time_series; // Stored time series values;
        System *system; // pointer to the system the observation is evaluated at
        string lasterror;
        double current_value=0;
        string outputitem="";
        CTimeSeriesSet<double> realizations;
        CTimeSeriesSet<double> percentile95;

};
#endif // OBSERVATION_H
