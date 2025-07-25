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


#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
#include <vector>
#include "Object.h"
#include "TimeSeriesSet.h"

using namespace std;
struct Range
{
    double low = 0;
    double high = 0;
};

class Parameter : public Object
{
    public:
        Parameter();
        virtual ~Parameter();
        Parameter(const Parameter& other);
        Parameter& operator=(const Parameter& other);
        void SetLocations(const vector<string> &loc) {_location = loc;}
        void AppendLocationQuan(const string &loc, const string &quan) {_location.push_back(loc); _quan.push_back(quan);}
        bool RemoveLocationQuan(const string &loc, const string &quan);
        vector<string> GetLocations() {return _location;}
        vector<string> GetQuans() {return _quan;}
        void SetRange(double low, double high) {Object::SetVal("low",low); Object::SetVal("high",high);}
        Range GetRange()
        {
            Range prior_range;
            prior_range.low = Object::GetVal("low");
            prior_range.high = Object::GetVal("high");
            return  prior_range;
        }
        void SetPriorDistribution(const string &prior) {Object::SetProperty("prior_distribution",prior); }
        string GetPriorDistribution()
        {
            if (!Object::HasQuantity("prior_distribution"))
                return "normal";
            return Object::Variable("prior_distribution")->GetStringValue();
        }
        double mean()
        {
            return (GetRange().high+GetRange().low)/2.0;
        }
        double geomean()
        {
            return sqrt(GetRange().high*GetRange().low);
        }
        double std()
        {
            return (GetRange().high-GetRange().low)/4.0;
        }
        double geostd()
        {
            return (log(GetRange().high)-log(GetRange().low))/4.0;
        }
        double ExpandedLow(const double &factor=1.5);
        double ExpandedHigh(const double &factor=1.5);
        TimeSeries<double> PriorDistribution(unsigned int nbins = 100);
        double CalcPriorProbability(const double &x);
        double CalcLogPriorProbability(const double &x);
        void SetValue(const double &val) {value = val; SetProperty("value", aquiutils::numbertostring(val));}
        double GetValue() {return Object::GetVal("value");}
        string LastError() {return last_error;}
        bool SetProperty(const string &s, const string &val, bool check_criteria=true)
        {
            if (s=="low") { Object::SetProperty("low",val);}
            if (s=="high") { Object::SetProperty("high",val);}
            if (s=="value") {value = atof(val.c_str());}
            return Object::SetProperty(s,val,false,check_criteria);

        }
        string toString(int _tabs = 0);
        bool HasQuantity(const string &qntty);
        string variable(const string &qntty);
        string TypeCategory() {return "Parameters";}
        bool SetName(string s);
        void SetMCMCSamples(const TimeSeriesSet<double> &mcmc_smpl_vals) {mcmc_sampled_values = mcmc_smpl_vals;}
        TimeSeriesSet<double>& GetMCMCSamples() {return mcmc_sampled_values;}
        void SetPosteriorDistribution(const TimeSeries<double> &posterior_dist) {posterior_distribution = posterior_dist;}
        TimeSeries<double>& GetPosteriorDistribution() {return posterior_distribution;};

    protected:

    private:
        vector<string> _location;
        vector<string> _quan;
        //Range prior_range;
        double low() {return GetVars()->GetVar("low").GetVal();}
        double high() {return GetVars()->GetVar("high").GetVal();}
        //string prior_distribution;
        double value;
        string last_error;
        TimeSeriesSet<double> mcmc_sampled_values;
        TimeSeries<double> posterior_distribution;

};

#endif // PARAMETER_H
