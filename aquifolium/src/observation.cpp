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

// ---------------------------------------------------------------------------
// Residuals (observed - modeled) at the observed points that lie within the
// modeled series' time span. An observation timestamped outside the simulated
// window has no modeled counterpart -- interpol() would return the first or
// last modeled value, which is an extrapolation dressed up as data -- so it is
// skipped and does not enter the mean.
//
// The bounds are INCLUSIVE. diff2(), which this replaced, tested them strictly,
// so an observation timestamped exactly at the simulation start was always
// discarded. At t == mint() interpol() returns the first modeled value, which
// is a genuine model output, not an extrapolation, and that point is often the
// most informative one in the series: it is where an initial condition is
// measured directly. The same holds at t == maxt().
//
// The caller must supply both series already in the comparison space (raw for
// "normal", logs for "log-normal"), because the test is applied to the series
// actually being compared.
// ---------------------------------------------------------------------------
static vector<double> in_range_residuals(const TimeSeries<timeseriesprecision> &modeled,
                                         const TimeSeries<timeseriesprecision> &observed)
{
    vector<double> d;
    if (modeled.empty() || observed.empty()) return d;
    const double t_min = modeled.mint();
    const double t_max = modeled.maxt();
    d.reserve(observed.size());
    for (const auto &pt : observed)
        if (pt.t >= t_min && pt.t <= t_max)
            d.push_back(pt.c - modeled.interpol(pt.t));
    return d;
}

double Observation::CalcMisfit()
{
    //qDebug()<<"Inside the misfit function";
    if (Variable("observed_data")==nullptr || Variable("observed_data")->GetTimeSeries()==nullptr)
    {
        fit_measures.resize(3);
        return 0;
    }

    // The three sum-of-squares methods share one implementation with the
    // Levenberg-Marquardt residual vector, so the misfit the GA minimises and
    // the residuals LM differentiates are by construction the same quantity.
    // ResidualVector() fills fit_measures on the way through.
    const string method = Variable("comparison_method")->GetProperty();
    if (method=="Least Squared" || method=="Weighted Least Squared" || IsEMC())
        return ResidualVector().NegLogLikelihood();

    // -----------------------------------------------------------------
    // Similarity: an autocorrelation distance plus a Kolmogorov-Smirnov
    // distance between the modeled and observed distributions. This is not a
    // sum of squares over per-point residuals and has no Gauss-Newton form,
    // which is why ResidualVector() reports it as not_decomposable and LM
    // refuses to run on a model that uses it.
    // -----------------------------------------------------------------
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
    fit_measures.clear();
    fit_measures.push_back(auto_correlation_diff + CDF_diff);
    fit_measures.push_back(auto_correlation_diff);
    fit_measures.push_back(CDF_diff);
    //qDebug()<<"Misfit vector populated";
    return auto_correlation_diff + CDF_diff;
}

// ---------------------------------------------------------------------------
// Residual decomposition of this observation's negative log-likelihood; see
// the ResidualBlock comment in observation.h. Every branch below reproduces
// the corresponding CalcMisfit() branch term by term:
//
//   -log L = (Neff/tau)*( MSE/(2 sigma^2) + log sigma )
//          =  0.5 * sum_i r_i^2  +  (Neff/tau) * log sigma
//
// so the residual scaling is whatever makes sum_i r_i^2 equal
// (Neff/tau)*MSE/sigma^2.
// ---------------------------------------------------------------------------
ResidualBlock Observation::ResidualVector()
{
    ResidualBlock block;
    fit_measures.clear();

    if (Variable("observed_data")==nullptr || Variable("observed_data")->GetTimeSeries()==nullptr)
    {
        fit_measures.resize(3);
        return block; // kind == empty
    }

    TimeSeries<timeseriesprecision>* obs = Variable("observed_data")->GetTimeSeries();
    const string method    = Variable("comparison_method")->GetProperty();
    const string structure = Variable("error_structure")->GetProperty();
    const bool lognormal   = (structure=="log-normal" || structure=="lognormal");
    const bool normal      = (structure=="normal");
    const double tau       = likelihood_scale;

    if (method=="Least Squared")
    {
        if (!normal && !lognormal)
        {
            // CalcMisfit scored an unrecognised error structure as 0 and still
            // does: no residuals, no log-sigma term.
            fit_measures.resize(3);
            return block;
        }

        vector<double> d;
        double fit_mse = 0, _R2 = 0, nse = 0;
        if (normal)
        {
            d   = in_range_residuals(modeled_time_series, *obs);
            _R2 = R2(&modeled_time_series, obs);
            nse = NSE(&modeled_time_series, obs);
        }
        else
        {
            const TimeSeries<timeseriesprecision> mod_log = modeled_time_series.log(1e-8);
            const TimeSeries<timeseriesprecision> obs_log = obs->log(1e-8);
            d   = in_range_residuals(mod_log, obs_log);
            _R2 = R2(mod_log, obs_log);
            nse = NSE(mod_log, obs_log);
        }

        double ss = 0;
        for (double v : d) ss += pow(v,2);
        const size_t count = d.size();
        fit_mse = count ? ss/double(count) : 0.0;
        fit_measures.push_back(fit_mse);
        fit_measures.push_back(_R2);
        fit_measures.push_back(nse);

        // Gaussian negative log-likelihood, constants dropped:
        //   -log L = (n/tau) * ( MSE/(2 sigma^2) + log sigma )
        // The factor 2 is what makes sigma's MAP the RMS residual; without it
        // sigma is inflated by sqrt(2).
        //
        // n is the number of points actually COMPARED, which is what MSE is
        // averaged over. Before this was fixed the prefactor used the full
        // observed count while the mean was taken over the compared subset, so
        // the whole likelihood was inflated by N/count. That left the point
        // estimate alone -- scaling an objective does not move its minimum --
        // but it over-weighted observations whose data extends past the
        // simulated window against those fully inside it, and it scaled the
        // Gauss-Newton curvature by the same factor, so parameter standard
        // errors came out too narrow by sqrt(count/N), in LM and in MCMC alike.
        const double sigma = Variable("error_standard_deviation")->GetVal();
        block.sigma = sigma;
        block.log_sigma_coeff = double(count)/tau;
        block.mse = fit_mse;
        block.kind = ResidualBlock::Kind::sum_of_squares;
        if (count)
        {
            const double scale = 1.0/(sigma*sqrt(tau));
            block.r.reserve(count);
            for (double v : d) block.r.push_back(v*scale);
        }
        return block;
    }

    // -----------------------------------------------------------------
    // EMC: one flow-weighted event mean concentration per event window,
    // compared with the observed EMC through the same Gaussian NLL as
    // Least Squared (N = number of events with data).
    // -----------------------------------------------------------------
    if (IsEMC())
    {
        if (!CalcEMCs() || observed_emc.size() == 0)
        {
            fit_measures.resize(3);
            return block;
        }
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

        // (n/tau)*(ss_res/n)/(2 sigma^2) = ss_res/(2 tau sigma^2), so the MSE's
        // 1/n cancels the prefactor's n and each residual is simply scaled by
        // 1/(sigma*sqrt(tau)).
        const double sigma = Variable("error_standard_deviation")->GetVal();
        block.sigma = sigma;
        block.log_sigma_coeff = double(n)/tau;
        block.mse = fit_mse;
        block.kind = ResidualBlock::Kind::sum_of_squares;
        const double scale = 1.0/(sigma*sqrt(tau));
        block.r.reserve(n);
        for (size_t i=0; i<n; i++) block.r.push_back((o[i]-m[i])*scale);
        return block;
    }

    // -----------------------------------------------------------------
    // Weighted Least Squared:
    // Same Gaussian negative-log-likelihood form as Least Squared, but
    // residuals are summed under a temporal kernel that downweights
    // older observations relative to the most recent one. The kernel
    // is anchored at t_now = last observed time, so the freshest
    // observation gets weight 1 and older observations are downweighted
    // by a log-power tail past Delta0. The effective sample size
    // sum_w replaces N in the likelihood, and the standard deviation
    // enters as in the LS case so that sigma remains identifiable when
    // calibrated jointly. R2/NSE are reported unweighted as diagnostics.
    // -----------------------------------------------------------------
    if (method=="Weighted Least Squared")
    {
        if (obs->size() == 0 || (!normal && !lognormal))
        {
            fit_measures.resize(3);
            return block;
        }

        const double t_now  = obs->getTime(obs->size() - 1);
        const double Delta0 = Variable("kernel_Delta0")->GetVal();
        const double ktau   = Variable("kernel_tau")->GetVal();
        const double alpha  = Variable("kernel_alpha")->GetVal();
        const double sigma  = Variable("error_standard_deviation")->GetVal();

        const TimeSeries<timeseriesprecision> obs_c = normal ? *obs : obs->log(1e-8);
        const TimeSeries<timeseriesprecision> mod_c = normal ? modeled_time_series : modeled_time_series.log(1e-8);

        double sum_w = 0;
        const double fit_mse = weighted_mse(obs_c, mod_c, t_now, Delta0, ktau, alpha, &sum_w);
        const double _R2 = normal ? R2(&modeled_time_series, obs) : R2(mod_c, obs_c);
        const double nse = normal ? NSE(&modeled_time_series, obs) : NSE(mod_c, obs_c);

        fit_measures.push_back(fit_mse);
        fit_measures.push_back(_R2);
        fit_measures.push_back(nse);

        // Kernel-weighted Gaussian NLL, constants dropped:
        //   -log L = sum_w*log(sigma) + sum_i w_i r_i^2 / (2 sigma^2)
        // sum_w is the kernel effective sample size, replacing N. Unlike the
        // Least Squared branch, weighted_mse compares at EVERY observed time
        // (interpolating the modeled series), so there is no in-range subset
        // and no N/count correction.
        block.sigma = sigma;
        block.log_sigma_coeff = sum_w/tau;
        block.mse = fit_mse;
        block.kind = ResidualBlock::Kind::sum_of_squares;
        block.r.reserve(obs_c.size());
        for (size_t i=0; i<obs_c.size(); i++)
        {
            const double t_i = obs_c.getTime(i);
            const double d   = obs_c.getValue(i) - mod_c.interpol(t_i);
            const double w   = recency_kernel_weight(t_now - t_i, Delta0, ktau, alpha);
            block.r.push_back(d*sqrt(w/tau)/sigma);
        }
        return block;
    }

    // "Similarity" and anything unrecognised.
    fit_measures.resize(3);
    block.kind = ResidualBlock::Kind::not_decomposable;
    return block;
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






