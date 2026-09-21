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


#include "observation.h"
#include "TimeSeries.h"
#include "System.h"



Observation::Observation(): Object::Object()
{
    SetObjectType(object_type::observation);
    //ctor
}

Observation::Observation(System *_system)
{
    SetObjectType(object_type::observation);
    system = _system;
}

Observation::Observation(System *_system, const Expression &expr, const string &loc)
{
    SetType("Observation");
    SetObjectType(object_type::observation);
    system = _system;
    expression = expr;
    location = loc;

}

Observation::~Observation()
{
    modeled_time_series.clear();
    observed_time_series.clear(); 
}

Observation::Observation(const Observation& other)
{
    Object::operator=(other);
    expression = other.expression;
    location = other.location;
    modeled_time_series = other.modeled_time_series;
    observed_time_series = other.observed_time_series;
    likelihood_scale = other.likelihood_scale;   // must survive System copies:
                                                 // every MCMC chain works on a copy
    weighting_expression = other.weighting_expression;
    emc_flux_series = other.emc_flux_series;
    emc_weight_series = other.emc_weight_series;
    modeled_emc = other.modeled_emc;
    observed_emc = other.observed_emc;
}

Observation& Observation::operator=(const Observation& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    expression = rhs.expression;
    location = rhs.location;
    observed_time_series = rhs.observed_time_series;
    likelihood_scale = rhs.likelihood_scale;
    weighting_expression = rhs.weighting_expression;
    ClearModeled();
    return *this;
}

bool Observation::SetProperty(const string &prop, const string &val)
{
    if (aquiutils::tolower(prop)=="expression")
    {
        expression = Expression(val);

    }
    if (aquiutils::tolower(prop)=="location" || aquiutils::tolower(prop)=="object")
    {
        location = val;

    }
    if (aquiutils::tolower(prop)=="emc_weighting_expression")
    {
        weighting_expression = Expression(val);
    }

    return Object::SetProperty(prop,val);
    return false;
}

double Observation::GetValue(const Expression::timing &tmg)
{
    if (expression.param_constant_expression == "")
        expression = Variable("expression")->GetProperty();

    if (system->block(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->block(location),tmg);
        return current_value;
    }
    if (system->link(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->link(location),tmg,true);
        return current_value;
    }
    lasterror = "Location " + location + "was not found in the system!";
    return 0;
}

double Observation::CalcMisfit()
{
    //qDebug()<<"Inside the misfit function";
    if (Variable("observed_data")->GetTimeSeries()!=nullptr)
    {
        fit_measures.clear();
        if (Variable("comparison_method")->GetProperty()=="Least Squared")
        {
            double fit_mse = 0;
            double _R2 = 0;
            double Nash_Sutcliffe_efficiency = 0;
            if (Variable("error_structure")->GetProperty()=="normal")
            {
                fit_mse = diff2(modeled_time_series,Variable("observed_data")->GetTimeSeries());
                _R2 = R2(&modeled_time_series,Variable("observed_data")->GetTimeSeries());
                Nash_Sutcliffe_efficiency = NSE(&modeled_time_series,Variable("observed_data")->GetTimeSeries());
                fit_measures.push_back(fit_mse);
                fit_measures.push_back(_R2);
                fit_measures.push_back(Nash_Sutcliffe_efficiency);
                // Gaussian negative log-likelihood, constants dropped:
                //   -log L = N*log(sigma) + sum_i r_i^2 / (2 sigma^2)
                // diff2() returns the MEAN squared error, so sum_i r_i^2 =
                // N * fit_mse and the whole thing is N*(MSE/(2 sigma^2) +
                // log sigma). The factor 2 is what makes sigma's MAP the
                // RMS residual; without it sigma is inflated by sqrt(2).
                return (Variable("observed_data")->GetTimeSeries()->size()/likelihood_scale)*(fit_mse/(2.0*pow(Variable("error_standard_deviation")->GetVal(),2))+log(Variable("error_standard_deviation")->GetVal()));

            }
            else if (Variable("error_structure")->GetProperty()=="log-normal" || Variable("error_structure")->GetProperty()=="lognormal")
            {
                fit_mse = diff2(modeled_time_series.log(1e-8),Variable("observed_data")->GetTimeSeries()->log(1e-8));
                _R2 = R2(modeled_time_series.log(1e-8),Variable("observed_data")->GetTimeSeries()->log(1e-8));
                Nash_Sutcliffe_efficiency = NSE(modeled_time_series.log(1e-8),Variable("observed_data")->GetTimeSeries()->log(1e-8));
                fit_measures.push_back(fit_mse);
                fit_measures.push_back(_R2);
                fit_measures.push_back(Nash_Sutcliffe_efficiency);
                // Same Gaussian NLL as the normal branch, in log space.
                return (Variable("observed_data")->GetTimeSeries()->size()/likelihood_scale)*(fit_mse/(2.0*pow(Variable("error_standard_deviation")->GetVal(),2))+log(Variable("error_standard_deviation")->GetVal()));
            }
            else
                return 0;
        }
        // -----------------------------------------------------------------
        // EMC: one flow-weighted event mean concentration per event window,
        // compared with the observed EMC through the same Gaussian NLL as
        // Least Squared (N = number of events with data).
        // -----------------------------------------------------------------
        else if (IsEMC())
        {
            fit_measures.clear();
            if (!CalcEMCs() || observed_emc.size() == 0)
            {
                fit_measures.resize(3);
                return 0;
            }
            const bool lognormal = (Variable("error_structure")->GetProperty()=="log-normal" || Variable("error_structure")->GetProperty()=="lognormal");
            const size_t n = observed_emc.size();
            vector<double> m(n), o(n);
            for (size_t i=0; i<n; i++)
            {
                m[i] = modeled_emc.getValue(i);
                o[i] = observed_emc.getValue(i);
                if (lognormal)
                {
                    m[i] = log(max(m[i],1e-8));
                    o[i] = log(max(o[i],1e-8));
                }
            }
            double mean_o = 0, mean_m = 0;
            for (size_t i=0; i<n; i++) { mean_o += o[i]; mean_m += m[i]; }
            mean_o /= n; mean_m /= n;
            double ss_res = 0, ss_tot = 0, s_mo = 0, s_mm = 0;
            for (size_t i=0; i<n; i++)
            {
                ss_res += pow(o[i]-m[i],2);
                ss_tot += pow(o[i]-mean_o,2);
                s_mo += (m[i]-mean_m)*(o[i]-mean_o);
                s_mm += pow(m[i]-mean_m,2);
            }
            const double fit_mse = ss_res/n;
            const double _R2 = (ss_tot>0 && s_mm>0) ? s_mo*s_mo/(ss_tot*s_mm) : 0;
            const double Nash_Sutcliffe_efficiency = (ss_tot>0) ? 1.0 - ss_res/ss_tot : 0;
            fit_measures.push_back(fit_mse);
            fit_measures.push_back(_R2);
            fit_measures.push_back(Nash_Sutcliffe_efficiency);
            const double sigma = Variable("error_standard_deviation")->GetVal();
            return (n/likelihood_scale)*(fit_mse/(2.0*pow(sigma,2))+log(sigma));
        }
        // -----------------------------------------------------------------
        // Weighted Least Squared:
        // Same Gaussian negative-log-likelihood form as Least Squared, but
        // residuals are summed under a temporal kernel that downweights
        // older observations relative to the most recent one. The kernel
        // is anchored at t_now = last observed time, so the freshest
        // observation gets weight 1 and older observations are downweighted
        // by a log-power tail past Delta0. The effective sample size
        // sum_w (returned via weighted_mse's out-param) replaces N in the
        // likelihood, and the standard deviation enters as in the LS case
        // so that sigma remains identifiable when calibrated jointly.
        // R2/NSE are reported unweighted as diagnostics.
        // -----------------------------------------------------------------
        else if (Variable("comparison_method")->GetProperty()=="Weighted Least Squared")
        {
            TimeSeries<double>* obs = Variable("observed_data")->GetTimeSeries();
            if (obs->size() == 0)
            {
                fit_measures.resize(3);
                return 0;
            }

            const double t_now  = obs->getTime(obs->size() - 1);
            const double Delta0 = Variable("kernel_Delta0")->GetVal();
            const double tau    = Variable("kernel_tau")->GetVal();
            const double alpha  = Variable("kernel_alpha")->GetVal();
            const double sigma  = Variable("error_standard_deviation")->GetVal();

            double fit_mse = 0;
            double sum_w   = 0;
            double _R2     = 0;
            double Nash_Sutcliffe_efficiency = 0;

            if (Variable("error_structure")->GetProperty()=="normal")
            {
                fit_mse = weighted_mse(*obs, modeled_time_series, t_now, Delta0, tau, alpha, &sum_w);
                _R2 = R2(&modeled_time_series, obs);
                Nash_Sutcliffe_efficiency = NSE(&modeled_time_series, obs);
            }
            else if (Variable("error_structure")->GetProperty()=="log-normal" || Variable("error_structure")->GetProperty()=="lognormal")
            {
                TimeSeries<double> obs_log = obs->log(1e-8);
                TimeSeries<double> mod_log = modeled_time_series.log(1e-8);
                fit_mse = weighted_mse(obs_log, mod_log, t_now, Delta0, tau, alpha, &sum_w);
                _R2 = R2(mod_log, obs_log);
                Nash_Sutcliffe_efficiency = NSE(mod_log, obs_log);
            }
            else
            {
                fit_measures.resize(3);
                return 0;
            }

            fit_measures.push_back(fit_mse);
            fit_measures.push_back(_R2);
            fit_measures.push_back(Nash_Sutcliffe_efficiency);

            // Kernel-weighted Gaussian NLL, constants dropped:
            //   -log L = sum_w*log(sigma) + sum_i w_i r_i^2 / (2 sigma^2)
            // weighted_mse() returns the WEIGHTED MEAN squared error, so
            // sum_i w_i r_i^2 = sum_w * fit_mse. sum_w is the kernel
            // effective sample size, replacing N.
            return (sum_w / likelihood_scale) * (fit_mse / (2.0 * pow(sigma, 2)) + log(sigma));
        }
        else
        {
            double auto_correlation_diff = 0;
            double CDF_diff = 0;
            double time_span = Variable("autocorrelation_time-span")->GetVal();
            double increment = time_span/20.0;
            TimeSeries<double> autocorr_measured = Variable("observed_data")->GetTimeSeries()->ConvertToNormalScore().AutoCorrelation(time_span,increment);
            TimeSeries<double> autocorr_modeled = modeled_time_series.ConvertToNormalScore().AutoCorrelation(time_span,increment);
            auto_correlation_diff =  diff2(autocorr_measured, autocorr_modeled);
            if (Variable("error_structure")->GetProperty()=="normal")
            {

                //qDebug()<<"Calculating Misfit Normal";
                CDF_diff = KolmogorovSmirnov(Variable("observed_data")->GetTimeSeries(),&modeled_time_series);

            }
            else
            {
                //qDebug()<<"Calculating Misfit Log-Normal";
                CDF_diff = KolmogorovSmirnov(Variable("observed_data")->GetTimeSeries()->log(1e-8),modeled_time_series.log(1e-8));
                //qDebug()<<"CDF diff calculated";
            }
            fit_measures.push_back(auto_correlation_diff + CDF_diff);
            fit_measures.push_back(auto_correlation_diff);
            fit_measures.push_back(CDF_diff);
            //qDebug()<<"Misfit vector populated";
            return auto_correlation_diff + CDF_diff;
        }
    }
    else
    {
        fit_measures.resize(3);
        return 0;
    }
}
void Observation::append_value(double t, double val)
{
    modeled_time_series.append(t,val);
    return;
}

void Observation::append_value(double t)
{
    current_value = GetValue(Expression::timing::present);
    modeled_time_series.append(t,current_value);
    if (IsEMC())
    {
        const double w = GetWeightingValue(Expression::timing::present);
        emc_flux_series.append(t,current_value*w);
        emc_weight_series.append(t,w);
    }
    return;
}

bool Observation::IsEMC()
{
    return Variable("comparison_method")!=nullptr && Variable("comparison_method")->GetProperty()=="EMC";
}

void Observation::ClearModeled()
{
    modeled_time_series.clear();
    emc_flux_series.clear();
    emc_weight_series.clear();
    modeled_emc.clear();
    observed_emc.clear();
}

double Observation::GetWeightingValue(const Expression::timing &tmg)
{
    if (Variable("emc_weighting_expression")==nullptr)
    {
        lasterror = "Observation '" + GetName() + "' has no 'emc_weighting_expression' property";
        return 0;
    }
    if (weighting_expression.param_constant_expression == "")
        weighting_expression = Expression(Variable("emc_weighting_expression")->GetProperty());

    string wloc = GetLocation();
    if (Variable("emc_weighting_object")!=nullptr && aquiutils::trim(Variable("emc_weighting_object")->GetProperty())!="")
        wloc = Variable("emc_weighting_object")->GetProperty();

    if (system->block(wloc) != nullptr)
        return weighting_expression.calc(system->block(wloc),tmg);
    if (system->link(wloc) != nullptr)
        return weighting_expression.calc(system->link(wloc),tmg,true);
    lasterror = "Weighting object " + wloc + " was not found in the system!";
    return 0;
}

// Trapezoidal integral of a piecewise-linear series over [a,b], one pass.
static double window_integral(const TimeSeries<timeseriesprecision> &ts, double a, double b)
{
    double sum = 0;
    for (size_t i=1; i<ts.size(); i++)
    {
        const double t0 = ts.getTime(i-1), t1 = ts.getTime(i);
        if (t1 <= a) continue;
        if (t0 >= b) break;
        if (t1 <= t0) continue;
        const double c0 = ts.getValue(i-1), c1 = ts.getValue(i);
        const double lo = max(t0,a), hi = min(t1,b);
        const double v_lo = c0 + (c1-c0)*(lo-t0)/(t1-t0);
        const double v_hi = c0 + (c1-c0)*(hi-t0)/(t1-t0);
        sum += 0.5*(v_lo+v_hi)*(hi-lo);
    }
    return sum;
}

bool Observation::CalcEMCs()
{
    modeled_emc.clear();
    observed_emc.clear();
    TimeSeries<timeseriesprecision> *obs = Variable("observed_data")->GetTimeSeries();
    TimeSeries<timeseriesprecision> *events = (Variable("emc_events")!=nullptr) ? Variable("emc_events")->GetTimeSeries() : nullptr;
    if (obs == nullptr || events == nullptr || events->size() == 0)
    {
        lasterror = "Observation '" + GetName() + "': EMC comparison requires an event file (emc_events) with rows 't_start, t_end'";
        return false;
    }
    if (emc_weight_series.size() < 2)
    {
        lasterror = "Observation '" + GetName() + "': no weighting (flow) values were recorded for the EMC calculation";
        return false;
    }
    const double t_first = emc_weight_series.getTime(0);
    const double t_last = emc_weight_series.getTime(emc_weight_series.size()-1);
    for (size_t k=0; k<events->size(); k++)
    {
        const double a = events->getTime(k);
        const double b = events->getValue(k);
        if (b <= a || b <= t_first || a >= t_last) continue; // empty or outside the simulated period

        // observed EMC: the observed value(s) time-stamped within the window
        double obs_sum = 0, t_obs = 0;
        int obs_n = 0;
        for (size_t j=0; j<obs->size(); j++)
            if (obs->getTime(j) >= a && obs->getTime(j) <= b)
            {
                if (obs_n == 0) t_obs = obs->getTime(j);
                obs_sum += obs->getValue(j);
                obs_n++;
            }
        if (obs_n == 0) continue;

        const double volume = window_integral(emc_weight_series, a, b);
        double emc;
        if (fabs(volume) > 1e-30)
            emc = window_integral(emc_flux_series, a, b)/volume;
        else
        {   // no modeled flow during the event: EMC undefined, use the time-averaged value
            const double lo = max(a,t_first), hi = min(b,t_last);
            emc = window_integral(modeled_time_series, lo, hi)/(hi-lo);
            lasterror = "Observation '" + GetName() + "': zero weighting (flow) during an event; time-averaged value used";
        }
        modeled_emc.append(t_obs, emc);
        observed_emc.append(t_obs, obs_sum/obs_n);
    }
    return true;
}

TimeSeries<timeseriesprecision> Observation::MappedModeledSeries()
{
    if (IsEMC())
    {
        CalcEMCs();
        return modeled_emc;
    }
    if (Variable("observed_data")->GetTimeSeries()==nullptr)
        return TimeSeries<timeseriesprecision>();
    return modeled_time_series.interpol(Variable("observed_data")->GetTimeSeries());
}

vector<string> Observation::ItemswithOutput()
{
    vector<string> s = Object::ItemswithOutput();
    s.push_back("Time Series");
    return s;
}






