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

// LM.hpp: implementation of the CLM class.
////////////////////////////////////////////////////////////////////////

#include "LM.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#ifndef mac_version
#include <omp.h>
#endif
#include "Utilities.h"

#ifdef Q_GUI_SUPPORT
    #include "ProgressWindow.h"
    #include <QCoreApplication>
#endif

template<class T>
CLM<T>::CLM()
{
}

template<class T>
CLM<T>::CLM(T *model)
{
    Model = model;
    filenames.pathname = Model->OutputPath();
}

template<class T>
CLM<T>::~CLM()
{
}

template<class T>
void CLM<T>::Log(const string &s) const
{
    cout << "[LM] " << s << std::endl;
#ifdef Q_GUI_SUPPORT
    if (rtw) rtw->AppendLog("[LM] " + s);
#endif
}

template<class T>
bool CLM<T>::SetProperty(const string &varname, const string &value)
{
    const string v = aquiutils::tolower(varname);
    if (v == "max_iterations")   { LM_params.max_iterations = aquiutils::atoi(value); return true; }
    if (v == "lambda0")          { LM_params.lambda0 = aquiutils::atof(value); return true; }
    if (v == "lambda_up")        { LM_params.lambda_up = aquiutils::atof(value); return true; }
    if (v == "lambda_down")      { LM_params.lambda_down = aquiutils::atof(value); return true; }
    if (v == "lambda_max")       { LM_params.lambda_max = aquiutils::atof(value); return true; }
    if (v == "max_rejections")   { LM_params.max_rejections = aquiutils::atoi(value); return true; }
    if (v == "fd_step")          { LM_params.fd_step = aquiutils::atof(value); return true; }
    if (v == "tol_objective")    { LM_params.tol_objective = aquiutils::atof(value); return true; }
    if (v == "tol_step")         { LM_params.tol_step = aquiutils::atof(value); return true; }
    if (v == "tol_gradient")     { LM_params.tol_gradient = aquiutils::atof(value); return true; }
    // lm_numthreads / lm_outputfile are the names the LM settings object uses.
    // The settings machinery resolves a model's "setvalue object=system" against
    // whichever settings block owns a quantity of that name FIRST, and jsoncpp
    // iterates the blocks in sorted key order, which puts "LM" ahead of
    // "optimizer". Plain "numthreads" and "outputfile" would therefore be stolen
    // from the genetic algorithm by every existing model. The bare names are
    // still accepted here so "setvalue object=LM, quantity=outputfile" and the
    // OHQ-LM command line keep working.
    if (v == "lm_numthreads" || v == "numthreads")
    {   LM_params.numthreads = aquiutils::atoi(value); numberOfThreads = LM_params.numthreads; return true; }
    if (v == "lm_outputfile" || v == "outputfile") { filenames.outputfilename = value; return true; }
    if (v == "covariancefile")   { filenames.covariancefilename = value; return true; }
    // The GUI's combo boxes deliver "Yes"/"No"; scripts and the command line use
    // "true"/"1". Accept all of them so a setting means the same thing wherever
    // it was set.
    if (v == "central_differences") { LM_params.central_differences = aquiutils::tolower(value)=="yes" || aquiutils::tolower(value)=="true" || value=="1"; return true; }
    if (v == "profile_sigma")       { LM_params.profile_sigma       = aquiutils::tolower(value)=="yes" || aquiutils::tolower(value)=="true" || value=="1"; return true; }
    if (v == "write_covariance")    { LM_params.write_covariance    = aquiutils::tolower(value)=="yes" || aquiutils::tolower(value)=="true" || value=="1"; return true; }

    last_error = "Property '" + varname + "' was not found!";
    return false;
}

template<class T>
void CLM<T>::SetParameters(Object *obj)
{
    if (obj == nullptr) return;
    for (unordered_map<string,Quan>::iterator it=obj->GetVars()->begin(); it!=obj->GetVars()->end(); it++)
        SetProperty(it->first, it->second.GetProperty());
}

// ---------------------------------------------------------------------------
// A parameter counts as a sigma parameter when EVERY location/quantity pair it
// drives is an observation's error_standard_deviation. A parameter that drives
// a sigma AND something else cannot be profiled out (its optimal value is not
// the analytic RMS residual), so it stays in the LM vector and is reported.
// ---------------------------------------------------------------------------
template<class T>
void CLM<T>::IdentifySigmaParameters()
{
    sigma_params.clear();
    sigma_observations.clear();
    if (!LM_params.profile_sigma) return;

    for (unsigned int i=0; i<Model->Parameters().size(); i++)
    {
        Parameter *p = Model->GetParameter(int(i));
        const vector<string> locs  = p->GetLocations();
        const vector<string> quans = p->GetQuans();
        if (locs.empty()) continue;

        vector<int> obs_indices;
        bool all_sigma = true;
        for (unsigned int j=0; j<locs.size(); j++)
        {
            if (j >= quans.size() || aquiutils::tolower(quans[j]) != "error_standard_deviation")
            {   all_sigma = false; break; }
            int found = -1;
            for (unsigned int k=0; k<Model->ObservationsCount(); k++)
                if (Model->observation(k)->GetName() == locs[j]) { found = int(k); break; }
            if (found < 0) { all_sigma = false; break; }
            obs_indices.push_back(found);
        }
        if (all_sigma && !obs_indices.empty())
        {
            sigma_params.push_back(int(i));
            sigma_observations.push_back(obs_indices);
        }
    }
}

template<class T>
bool CLM<T>::Initialize()
{
    if (Model == nullptr) { last_error = "No model was assigned to the optimizer."; return false; }
    if (Model->Parameters().size() == 0) { last_error = "No parameters have been defined."; return false; }
    if (Model->ObservationsCount() == 0)
    {
        last_error = "Levenberg-Marquardt calibrates against observations, and this model "
                     "defines none. Add observations with observed data. Scoring a design "
                     "objective-function set instead is what Optimize is for.";
        return false;
    }

    const int nP = int(Model->Parameters().size());
    loged.assign(nP,0);
    minval.assign(nP,0.0);
    maxval.assign(nP,0.0);
    x.assign(nP,0.0);
    paramname.clear();

    for (int i=0; i<nP; i++)
    {
        Parameter *p = Model->GetParameter(i);
        const string prior = p->GetPriorDistribution();
        const bool islog = (prior=="lognormal" || prior=="log-normal");
        loged[i] = islog ? 1 : 0;

        const double low = p->GetVal("low");
        const double high = p->GetVal("high");
        if (islog)
        {
            if (low <= 0 || high <= 0)
            {
                last_error = "Parameter '" + Model->Parameters().getKeyAtIndex(i) +
                             "' has a lognormal prior but a non-positive range, so it cannot "
                             "be searched in log space.";
                return false;
            }
            minval[i] = log10(low);
            maxval[i] = log10(high);
        }
        else
        {
            minval[i] = low;
            maxval[i] = high;
        }
        if (!(maxval[i] > minval[i]))
        {
            last_error = "Parameter '" + Model->Parameters().getKeyAtIndex(i) +
                         "' has an empty or inverted range [" + aquiutils::numbertostring(low) +
                         ", " + aquiutils::numbertostring(high) + "].";
            return false;
        }

        // LM is local: it starts where the parameters currently are. After a
        // GA run those are the GA's estimates, which is exactly the hybrid we
        // want, and it needs no coupling between the two classes. A parameter
        // that has never been given a value starts at the middle of its range
        // (the geometric middle when it is searched in log space).
        double v = p->GetValue();
        if (!std::isfinite(v) || v == 0)
            x[i] = 0.5*(minval[i]+maxval[i]);
        else
            x[i] = ToSearchSpace(i, v);

        paramname.push_back(Model->Parameters().getKeyAtIndex(i));
    }
    Clamp(x);

    IdentifySigmaParameters();

    active.clear();
    for (int i=0; i<nP; i++)
        if (std::find(sigma_params.begin(), sigma_params.end(), i) == sigma_params.end())
            active.push_back(i);

    if (active.empty())
    {
        last_error = "Every calibrated parameter is an error standard deviation being profiled "
                     "out, so there is nothing left for Levenberg-Marquardt to optimise.";
        return false;
    }

    numberOfThreads = max(1, LM_params.numthreads);
    return true;
}

template<class T>
void CLM<T>::Clamp(vector<double> &x_t) const
{
    for (unsigned int i=0; i<x_t.size(); i++)
        x_t[i] = min(max(x_t[i], minval[i]), maxval[i]);
}

template<class T>
double CLM<T>::Evaluate(const vector<double> &x_t, T &model, vector<double> &r)
{
    for (unsigned int i=0; i<x_t.size(); i++)
        model.SetParameterValue(int(i), ToModelSpace(int(i), x_t[i]));
    model.ApplyParameters();
    // Called from the parallel Jacobian sweep, so the counter is atomic.
#ifndef NO_OPENMP
#pragma omp atomic
#endif
    model_solves++;
    model.Solve();

    r.clear();
    if (model.GetSolutionFailed())
        return 1e18;

    vector<ResidualBlock> blocks;
    string offender;
    if (!model.ResidualVector(r, blocks, &offender))
        return 1e18;   // caller has already refused this case in optimize()

    double f = 0;
    for (unsigned int k=0; k<blocks.size(); k++)
        f += blocks[k].NegLogLikelihood();
    return f;
}

// ---------------------------------------------------------------------------
// sigma_hat minimises A/(2 s^2) + c*log(s) over the observations a parameter
// drives, where A = sum_k sigma_k^2 * ||r_k||^2 (the residual sum of squares
// with the current sigma divided back out) and c = sum_k log_sigma_coeff_k.
// The minimiser is s^2 = A/c: the effective-sample-size-weighted RMS residual.
// ---------------------------------------------------------------------------
template<class T>
bool CLM<T>::ProfileSigmas(T &model, const vector<ResidualBlock> &blocks)
{
    bool changed = false;
    for (unsigned int s=0; s<sigma_params.size(); s++)
    {
        double A = 0, c = 0;
        for (unsigned int j=0; j<sigma_observations[s].size(); j++)
        {
            const int k = sigma_observations[s][j];
            if (k >= int(blocks.size())) continue;
            if (blocks[k].kind != ResidualBlock::Kind::sum_of_squares) continue;
            // Neff*mse is the residual sum of squares with the current sigma
            // divided back out, matching NegLogLikelihood()'s own arithmetic.
            A += blocks[k].log_sigma_coeff*blocks[k].mse;
            c += blocks[k].log_sigma_coeff;
        }
        if (c <= 0 || A <= 0) continue;

        const int pi = sigma_params[s];
        const double s_hat = min(max(sqrt(A/c), ToModelSpace(pi,minval[pi])),
                                 ToModelSpace(pi,maxval[pi]));
        const double x_new = ToSearchSpace(pi, s_hat);
        if (fabs(x_new - x[pi]) > 1e-14*max(1.0,fabs(x[pi]))) changed = true;
        x[pi] = x_new;
        model.SetParameterValue(pi, s_hat);
    }
    return changed;
}

template<class T>
bool CLM<T>::ComputeJacobian(const vector<double> &x_t, const vector<double> &r0,
                             arma::mat &J)
{
    const int na = int(active.size());
    const int m  = int(r0.size());
    const bool central = LM_params.central_differences;
    const int n_eval = central ? 2*na : na;

    // Perturbed points, forward first then backward when central.
    vector<vector<double>> points(n_eval, x_t);
    vector<double> h(na, 0.0);
    for (int a=0; a<na; a++)
    {
        const int i = active[a];
        double step = LM_params.fd_step*(maxval[i]-minval[i]);
        // A forward step that would leave the box is taken backwards instead,
        // so a parameter sitting on a bound still gets a one-sided derivative
        // instead of a zero column.
        if (x_t[i] + step > maxval[i]) step = -step;
        h[a] = step;
        points[a][i] = min(max(x_t[i]+step, minval[i]), maxval[i]);
        if (central)
        {
            points[na+a][i] = min(max(x_t[i]-step, minval[i]), maxval[i]);
            // The actual two-sided width, in case a bound truncated one side.
            h[a] = 0.5*(points[a][i] - points[na+a][i]);
        }
        else
        {
            h[a] = points[a][i] - x_t[i];
        }
        if (h[a] == 0)
        {
            last_error = "Parameter '" + paramname[i] + "' has a finite-difference step of "
                         "zero, which means its range is degenerate.";
            return false;
        }
    }

    Models.clear();
    Models.resize(n_eval);
    vector<vector<double>> R(n_eval);
    vector<double> F(n_eval, 1e18);
    vector<char> ok(n_eval, 0);

    for (int e=0; e<n_eval; e++)
    {
        Models[e] = *Model;
        Models[e].SetSilent(true);
        Models[e].SetRecordResults(false);
        Models[e].SetNumThreads(1);
    }

#ifndef NO_OPENMP
    omp_set_num_threads(numberOfThreads);
#pragma omp parallel for
#endif
    for (int e=0; e<n_eval; e++)
    {
        F[e] = Evaluate(points[e], Models[e], R[e]);
        ok[e] = (!Models[e].GetSolutionFailed() && int(R[e].size()) == m) ? 1 : 0;
#ifdef Q_GUI_SUPPORT
#ifndef NO_OPENMP
#pragma omp critical
#endif
        {
            if (rtw)
            {
#ifndef NO_OPENMP
                if (omp_get_thread_num() == 0)
#endif
                {
                    rtw->SetSecondaryProgress(double(e+1)/double(n_eval));
                    QCoreApplication::processEvents();
                }
            }
        }
#endif
    }

    for (int e=0; e<n_eval; e++)
    {
        if (ok[e]) continue;
        if (Models[e].GetSolutionFailed())
            last_error = "The model failed to solve while perturbing '" +
                         paramname[active[e % na]] + "' for the Jacobian.";
        else
            last_error = "Perturbing '" + paramname[active[e % na]] + "' changed the number of "
                         "residuals from " + aquiutils::numbertostring(m) + " to " +
                         aquiutils::numbertostring(int(R[e].size())) + ". The comparison points "
                         "themselves moved with the parameters, so a Jacobian is not defined "
                         "here; use the genetic algorithm for this model.";
        Models.clear();
        return false;
    }

    J.set_size(m, na);
    for (int a=0; a<na; a++)
        for (int i=0; i<m; i++)
            J(i,a) = central ? (R[a][i]-R[na+a][i])/(2.0*h[a])
                             : (R[a][i]-r0[i])/h[a];

    Models.clear();
    return true;
}

template<class T>
void CLM<T>::WriteHeader(FILE *f) const
{
    if (!f) return;
    const time_t now = time(nullptr);
    char stamp[64]; strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "# OpenHydroQual -- Levenberg-Marquardt calibration log\n");
    fprintf(f, "# started            : %s\n", stamp);
    fprintf(f, "# max iterations     : %d\n", LM_params.max_iterations);
    fprintf(f, "# initial lambda     : %g (x%g rejected, /%g accepted)\n",
            LM_params.lambda0, LM_params.lambda_up, LM_params.lambda_down);
    fprintf(f, "# finite differences : %s, step = %g of each parameter's range\n",
            LM_params.central_differences ? "central" : "forward", LM_params.fd_step);
    fprintf(f, "# tolerances         : objective %g, step %g, gradient %g\n",
            LM_params.tol_objective, LM_params.tol_step, LM_params.tol_gradient);
    fprintf(f, "#\n# optimised parameters (%d):\n", int(active.size()));
    for (unsigned int a=0; a<active.size(); a++)
        fprintf(f, "#   %-24s searched in %s space, within [%g, %g]\n",
                paramname[active[a]].c_str(), (loged[active[a]]==1 ? "log10" : "linear"),
                ToModelSpace(active[a], minval[active[a]]),
                ToModelSpace(active[a], maxval[active[a]]));
    if (!sigma_params.empty())
    {
        fprintf(f, "#\n# error standard deviations profiled out analytically (%d):\n",
                int(sigma_params.size()));
        for (unsigned int s=0; s<sigma_params.size(); s++)
            fprintf(f, "#   %s\n", paramname[sigma_params[s]].c_str());
    }
    fprintf(f, "#\n# One row per iteration. 'status' is accept when the step reduced the\n");
    fprintf(f, "# negative log-likelihood and reject when lambda had to be raised and the\n");
    fprintf(f, "# step retried. Parameter columns are in MODEL space, not search space.\n#\n");

    fprintf(f, "%-5s, %-14s, %-11s, %-7s", "iter", "neg_log_lik", "lambda", "status");
    for (unsigned int i=0; i<paramname.size(); i++)
        fprintf(f, ", %s", paramname[i].c_str());
    fprintf(f, "\n");
    fflush(f);
}

template<class T>
void CLM<T>::WriteIteration(FILE *f, int iter, double f_now, double lambda,
                            const vector<double> &x_t, const char *status) const
{
    if (!f) return;
    fprintf(f, "%-5d, %- 14.7e, %- 11.4e, %-7s", iter, f_now, lambda, status);
    for (unsigned int i=0; i<x_t.size(); i++)
        fprintf(f, ", %.8g", ToModelSpace(int(i), x_t[i]));
    fprintf(f, "\n");
    fflush(f);
}

// ---------------------------------------------------------------------------
// At the solution the Gauss-Newton approximation of the NLL's Hessian is J'J,
// so (J'J)^-1 is the parameter covariance. The residuals already carry 1/sigma,
// which is what makes this a covariance and not an unscaled curvature.
// Everything is in the SEARCH space, so an entry for a lognormal parameter is
// the variance of its base-10 logarithm.
// ---------------------------------------------------------------------------
template<class T>
void CLM<T>::WriteCovariance() const
{
    if (!LM_params.write_covariance || !covariance_valid) return;
    string name = filenames.covariancefilename;
    if (!aquiutils::contains(name,"/")) name = filenames.pathname + name;
    FILE *f = fopen(name.c_str(), "w");
    if (!f) return;

    fprintf(f, "# OpenHydroQual -- Levenberg-Marquardt parameter covariance\n");
    fprintf(f, "# (J'J)^-1 at the solution, in the space each parameter was searched in:\n");
    fprintf(f, "# a lognormal parameter's entries refer to log10(value), not to the value.\n");
    fprintf(f, "# negative log-likelihood at the solution: %.8e\n#\n", final_objective);

    fprintf(f, "# estimate and standard error\n");
    fprintf(f, "%-24s, %-8s, %-14s, %-14s, %-14s, %-14s\n",
            "parameter", "space", "estimate", "std_error", "ci95_low", "ci95_high");
    for (unsigned int a=0; a<active.size(); a++)
    {
        const int i = active[a];
        const double xi = x[i];
        const double se = standard_error[a];
        fprintf(f, "%-24s, %-8s, %- 14.7g, %- 14.7g, %- 14.7g, %- 14.7g\n",
                paramname[i].c_str(), (loged[i]==1 ? "log10" : "linear"),
                ToModelSpace(i, xi), se,
                ToModelSpace(i, xi-1.96*se), ToModelSpace(i, xi+1.96*se));
    }

    fprintf(f, "\n# correlation matrix\n%-24s", "");
    for (unsigned int a=0; a<active.size(); a++)
        fprintf(f, ", %-10s", paramname[active[a]].c_str());
    fprintf(f, "\n");
    for (unsigned int a=0; a<active.size(); a++)
    {
        fprintf(f, "%-24s", paramname[active[a]].c_str());
        for (unsigned int b=0; b<active.size(); b++)
            fprintf(f, ", %- 10.4f", correlation(a,b));
        fprintf(f, "\n");
    }
    fclose(f);
}

template<class T>
int CLM<T>::optimize()
{
#ifdef Q_GUI_SUPPORT
    QCoreApplication::processEvents();
#endif
    if (!Initialize())
    {
        Log("Cannot start: " + last_error);
        return -1;
    }

    // Refuse before doing any work if the likelihood has no sum-of-squares
    // form, instead of differentiating a Kolmogorov-Smirnov distance and
    // reporting a covariance that means nothing.
    {
        T probe = *Model;
        probe.SetSilent(true);
        probe.SetRecordResults(false);
        probe.SetNumThreads(1);
        vector<double> r_probe;
        vector<ResidualBlock> blocks_probe;
        string offender;
        for (unsigned int i=0; i<x.size(); i++)
            probe.SetParameterValue(int(i), ToModelSpace(int(i), x[i]));
        probe.ApplyParameters();
        probe.Solve();
        if (probe.GetSolutionFailed())
        {
            last_error = "The model failed to solve at the starting parameter values, so "
                         "Levenberg-Marquardt has no point to start from. Fix the model, or "
                         "run the genetic algorithm first to find a feasible region.";
            Log("Cannot start: " + last_error);
            return -1;
        }
        if (!probe.ResidualVector(r_probe, blocks_probe, &offender))
        {
            last_error = "Observation '" + offender + "' uses the 'Similarity' comparison "
                         "method, which is an autocorrelation and Kolmogorov-Smirnov distance "
                         "and not a sum of squares. Levenberg-Marquardt has no Gauss-Newton "
                         "form for it; use the genetic algorithm for this model.";
            Log("Cannot start: " + last_error);
            return -1;
        }
        if (r_probe.empty())
        {
            last_error = "No residuals: the model's observations carry no observed data inside "
                         "the simulated time range.";
            Log("Cannot start: " + last_error);
            return -1;
        }
        if (int(r_probe.size()) < int(active.size()))
        {
            last_error = "There are " + aquiutils::numbertostring(int(r_probe.size())) +
                         " residuals but " + aquiutils::numbertostring(int(active.size())) +
                         " parameters to estimate. The problem is underdetermined and the "
                         "covariance would be singular.";
            Log("Cannot start: " + last_error);
            return -1;
        }
    }

    string RunFileName = filenames.outputfilename;
    if (!aquiutils::contains(RunFileName,"/")) RunFileName = filenames.pathname + RunFileName;
    FILE *FileOut = fopen(RunFileName.c_str(), "w");
    WriteHeader(FileOut);

    T current = *Model;
    current.SetSilent(true);
    current.SetRecordResults(false);
    current.SetNumThreads(1);

    vector<double> r;
    double f_now = Evaluate(x, current, r);

    // With sigma profiled the first evaluation used whatever sigma the model
    // came with, which is usually not its maximiser; set it and re-score so the
    // first reported value is on the same footing as every later one.
    if (!sigma_params.empty())
    {
        vector<double> r_tmp; vector<ResidualBlock> blocks; string dummy;
        current.ResidualVector(r_tmp, blocks, &dummy);
        if (ProfileSigmas(current, blocks))
            f_now = Evaluate(x, current, r);
    }

    Log("Starting from a negative log-likelihood of " + aquiutils::numbertostring(f_now) +
        " with " + aquiutils::numbertostring(int(r.size())) + " residuals and " +
        aquiutils::numbertostring(int(active.size())) + " free parameters.");
    WriteIteration(FileOut, 0, f_now, LM_params.lambda0, x, "start");

#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        rtw->SetPrimaryChartYRange(0, f_now*1.1);
        rtw->AddPrimaryChartPoint(0, f_now);
        rtw->ReplotPrimaryChart();
    }
#endif

    double lambda = LM_params.lambda0;
    const int na = int(active.size());
    arma::mat J;
    int iter = 0;
    string stop_reason = "reached the iteration limit";

    for (iter = 1; iter <= LM_params.max_iterations; iter++)
    {
        if (!ComputeJacobian(x, r, J))
        {
            Log("Stopped: " + last_error);
            stop_reason = last_error;
            break;
        }

        // Normal equations. JtJ is na-by-na and na is the number of calibrated
        // parameters -- a handful -- so forming it explicitly costs nothing
        // next to the model solves that produced J.
        const arma::mat &Jm = J;
        arma::vec  rv(r.size());
        for (unsigned int i=0; i<r.size(); i++) rv(i) = r[i];
        arma::mat  JtJ = Jm.t()*Jm;
        arma::vec  Jtr = Jm.t()*rv;

        const double grad_inf = arma::norm(Jtr, "inf");
        if (grad_inf < LM_params.tol_gradient)
        {
            stop_reason = "the gradient is below tol_gradient";
            Log("Converged: " + stop_reason + " (|J'r|_inf = " +
                aquiutils::numbertostring(grad_inf) + ").");
            break;
        }

        arma::vec diag_JtJ = JtJ.diag();
        // A parameter the residuals do not respond to gives a zero diagonal and
        // damping it by a multiple of zero leaves the system singular. Fall back
        // to the largest diagonal so such a parameter is simply held still
        // instead of taking the whole solve down.
        const double dmax = diag_JtJ.max();
        for (int a=0; a<na; a++)
            if (diag_JtJ(a) <= 0) diag_JtJ(a) = (dmax > 0 ? dmax : 1.0);

        bool accepted = false;
        bool converged = false;
        int  rejections = 0;
        vector<double> x_try(x), r_try;
        double f_try = 0, step_rel = 0;
        const double f_before = f_now;

        while (!accepted)
        {
            arma::mat A = JtJ;
            A.diag() += lambda*diag_JtJ;

            arma::vec delta;
            if (!arma::solve(delta, A, -Jtr, arma::solve_opts::no_approx))
            {
                // Only reached when the damped system is still singular, which
                // means the parameters are exactly redundant at this point.
                if (!arma::solve(delta, A, -Jtr, arma::solve_opts::allow_ugly))
                {
                    lambda *= LM_params.lambda_up;
                    if (lambda > LM_params.lambda_max)
                    {
                        stop_reason = "the damped normal equations stayed singular up to "
                                      "lambda_max, which means two or more parameters are "
                                      "indistinguishable from the data";
                        break;
                    }
                    continue;

                }
            }

            x_try = x;
            for (int a=0; a<na; a++) x_try[active[a]] += delta(a);
            Clamp(x_try);

            // Predicted reduction from the local quadratic model, used to decide
            // how much to trust it. Computed on the ACTUAL step taken, after the
            // box projection, so a clipped step is judged on what it really did.
            arma::vec d_actual(na);
            for (int a=0; a<na; a++) d_actual(a) = x_try[active[a]] - x[active[a]];
            const double predicted = -arma::dot(d_actual, Jtr) - 0.5*arma::dot(d_actual, JtJ*d_actual);

            double xn = 0, dn = 0;
            for (int a=0; a<na; a++) { xn += pow(x[active[a]],2); dn += pow(d_actual(a),2); }
            step_rel = sqrt(dn)/max(sqrt(xn), 1e-12);

            if (sqrt(dn) == 0)
            {
                // The damped step rounds to nothing in double precision, or the
                // box projection cancelled it entirely. More damping only makes
                // it smaller, so this is as far as the search goes.
                stop_reason = "the step underflowed to zero, so the search cannot move "
                              "further from this point";
                converged = true;
                break;
            }

            T trial = *Model;
            trial.SetSilent(true);
            trial.SetRecordResults(false);
            trial.SetNumThreads(1);
            f_try = Evaluate(x_try, trial, r_try);

            if (!sigma_params.empty() && !trial.GetSolutionFailed())
            {
                // Profiling sigma at the trial point is what makes the accept
                // test compare like with like: both points are scored at their
                // own best sigma, which is the profile likelihood LM is
                // actually descending.
                vector<double> r_tmp; vector<ResidualBlock> blocks; string dummy;
                trial.ResidualVector(r_tmp, blocks, &dummy);
                vector<double> x_saved = x;
                x = x_try;
                if (ProfileSigmas(trial, blocks))
                {
                    x_try = x;
                    f_try = Evaluate(x_try, trial, r_try);
                }
                x = x_saved;
            }

            const double actual = f_now - f_try;
            const double rho = (predicted > 0) ? actual/predicted : (actual > 0 ? 1.0 : -1.0);

            if (actual > 0 && rho > 0)
            {
                accepted = true;
                x = x_try;
                r = r_try;
                f_now = f_try;
                current = trial;
                // A step the quadratic model predicted well earns more trust.
                lambda = max(lambda/((rho > 0.75) ? LM_params.lambda_down : 1.0),
                             1e-12);
            }
            else
            {
                rejections++;
                lambda *= LM_params.lambda_up;
                WriteIteration(FileOut, iter, f_try, lambda, x_try, "reject");

                // A step that fails to improve the objective by more than
                // tol_objective has not hit a badly scaled trust region, it has
                // hit the floor set by the finite-difference resolution and the
                // solver's own tolerance. Raising lambda cannot get below that
                // floor, and each attempt costs a full model solve, so stop.
                if (fabs(actual)/max(fabs(f_now), 1e-300) < LM_params.tol_objective)
                {
                    stop_reason = "no step changes the objective by more than tol_objective, "
                                  "which is the resolution the finite-difference gradient can "
                                  "see";
                    converged = true;
                    break;
                }
                if (rejections >= LM_params.max_rejections)
                {
                    stop_reason = "no step reduced the objective in " +
                                  aquiutils::numbertostring(rejections) + " attempts at "
                                  "increasing damping, so this is a local minimum to within "
                                  "the finite-difference resolution";
                    converged = true;
                    break;
                }
                if (lambda > LM_params.lambda_max)
                {
                    stop_reason = "no step reduced the objective even at maximum damping, so "
                                  "this is a local minimum to within the finite-difference "
                                  "resolution";
                    converged = true;
                    break;
                }
            }
        }

        if (!accepted)
        {
            Log(string(converged ? "Converged at iteration " : "Stopped at iteration ") +
                aquiutils::numbertostring(iter) + ": " + stop_reason);
            break;
        }

        WriteIteration(FileOut, iter, f_now, lambda, x, "accept");
        Log("iteration " + aquiutils::numbertostring(iter) + ": -logL = " +
            aquiutils::numbertostring(f_now) + ", lambda = " + aquiutils::numbertostring(lambda));

#ifdef Q_GUI_SUPPORT
        if (rtw)
        {
            rtw->SetProgress(double(iter)/double(LM_params.max_iterations));
            rtw->AddPrimaryChartPoint(double(iter), f_now);
            rtw->ReplotPrimaryChart();
            QCoreApplication::processEvents();
        }
#endif

        const double rel_improvement = (f_before - f_now)/max(fabs(f_before), 1e-300);
        if (rel_improvement < LM_params.tol_objective)
        {
            stop_reason = "the objective improved by less than tol_objective";
            Log("Converged: " + stop_reason + " (relative improvement " +
                aquiutils::numbertostring(rel_improvement) + ").");
            break;
        }

        if (step_rel < LM_params.tol_step)
        {
            stop_reason = "the step is below tol_step";
            Log("Converged: " + stop_reason + ".");
            break;
        }
    }

    // The for loop leaves iter one past the limit when it runs to completion;
    // every other exit breaks out on the iteration being reported.
    if (iter > LM_params.max_iterations) iter = LM_params.max_iterations;

    // -----------------------------------------------------------------
    // Covariance from the final Jacobian. Recomputed at the solution rather
    // than reusing the last iteration's, which was taken one step away from it.
    // -----------------------------------------------------------------
    covariance_valid = false;
    if (ComputeJacobian(x, r, J))
    {
        const arma::mat &Jm = J;
        arma::mat JtJ = Jm.t()*Jm;
        arma::mat C;
        if (arma::inv_sympd(C, JtJ) || arma::inv(C, JtJ))
        {
            covariance = C;
            correlation.set_size(na, na);
            standard_error.assign(na, 0.0);
            for (int a=0; a<na; a++)
                standard_error[a] = (C(a,a) > 0) ? sqrt(C(a,a)) : 0.0;
            for (int a=0; a<na; a++)
                for (int b=0; b<na; b++)
                    correlation(a,b) = (standard_error[a] > 0 && standard_error[b] > 0)
                                     ? C(a,b)/(standard_error[a]*standard_error[b]) : 0.0;
            covariance_valid = true;
        }
        else
        {
            Log("The Gauss-Newton curvature matrix is singular at the solution, so no "
                "covariance is reported: at least one parameter, or one combination of "
                "parameters, the data cannot distinguish.");
        }
    }

    // Final full run with results recorded, so Model_out carries outputs the
    // caller can plot -- the same contract CGA::optimize() ends on.
    final_params.resize(x.size());
    for (unsigned int i=0; i<x.size(); i++)
        final_params[i] = ToModelSpace(int(i), x[i]);

    T best = *Model;
    for (unsigned int i=0; i<x.size(); i++)
        best.SetParameterValue(int(i), final_params[i]);
    best.ApplyParameters();
    best.Solve();
    final_objective = best.GetObjectiveFunctionValue();
    Model_out = best;
    Model_out.TransferResultsFrom(&best);

    if (FileOut)
    {
        fprintf(FileOut, "\n# stopped after %d iteration(s) and %ld model solves: %s\n",
                iter, model_solves, stop_reason.c_str());
        fprintf(FileOut, "# final negative log-likelihood: %.8e\n#\n", final_objective);
        fprintf(FileOut, "# final estimates\n");
        for (unsigned int i=0; i<final_params.size(); i++)
            fprintf(FileOut, "%-24s, %.10g%s\n", paramname[i].c_str(), final_params[i],
                    (std::find(sigma_params.begin(), sigma_params.end(), int(i)) != sigma_params.end())
                        ? "   # profiled analytically" : "");
        fclose(FileOut);
    }

    WriteCovariance();

    Log("Finished after " + aquiutils::numbertostring(iter) + " iteration(s) and " +
        aquiutils::numbertostring(int(model_solves)) + " model solves: " + stop_reason +
        ". Final -logL = " + aquiutils::numbertostring(final_objective) + ".");

#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        rtw->SetProgress(1.0);
        QCoreApplication::processEvents();
    }
#endif

    return iter;
}
