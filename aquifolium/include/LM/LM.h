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

#pragma once

// ---------------------------------------------------------------------------
// Levenberg-Marquardt calibration.
//
// Minimises the SAME negative log-likelihood the genetic algorithm minimises
// (System::GetObjectiveFunctionValue()), but uses its residual decomposition
// (System::ResidualVector()) to build a Gauss-Newton step instead of scoring a
// population. Where the GA needs maxpop*ngen model solves and returns a point
// estimate, LM needs roughly nParam solves per iteration and returns a point
// estimate WITH a parameter covariance matrix, because at the solution
// (J^T J)^-1 is the Gauss-Newton approximation of the NLL's inverse Hessian.
//
// LM is a local method. It starts from each parameter's CURRENT value, so
// running it after an Inverse Run automatically continues from the GA's
// estimate (that action writes its result back into the system's parameters),
// and running it on a multimodal problem from a poor starting point will find a
// local optimum.
// That is the intended division of labour: GA to find the basin, LM to
// converge inside it and quantify the uncertainty.
//
// Parameter space is the GA's: log10 for parameters with a lognormal prior,
// linear otherwise, bounded by each parameter's [low, high]. Steps are
// projected onto that box.
//
// Requires every observation to use a comparison method with a sum-of-squares
// form; "Similarity" has none, and optimize() refuses instead of silently
// differentiating something that is not a likelihood of that shape.
// ---------------------------------------------------------------------------

#include <string>
#include <vector>
#include "Matrix_arma.h"
#include "Vector_arma.h"
#include "Object.h"
#include "observation.h"
#include "Parameter.h"

// GUI
class ProgressWindow;

using namespace std;

struct LM_Parameters
{
    int    max_iterations = 50;

    // Marquardt damping. The step solves (J'J + lambda*diag(J'J)) d = -J'r, so
    // lambda -> 0 is Gauss-Newton and lambda -> inf is a short steepest-descent
    // step. Scaling by diag(J'J) instead of by the identity makes the damping
    // invariant to how the parameters are scaled, which matters here because
    // conductivities and porosities differ by many orders of magnitude.
    double lambda0    = 1e-2;
    double lambda_up  = 3.0;    // multiply after a rejected step
    double lambda_down= 3.0;    // divide after an accepted step
    double lambda_max = 1e12;   // past this the step is numerically zero: stop

    // Cap on consecutive rejected steps within one iteration. Each rejection
    // costs a full model solve, and raising lambda by lambda_up at a time takes
    // ~30 of them to walk from O(1) to lambda_max. Past this many, the point is
    // a local minimum to within the finite-difference resolution and there is
    // nothing to gain from grinding lambda upwards.
    int max_rejections = 10;

    // Finite-difference step, as a fraction of each parameter's [low,high]
    // range in the transformed space. The model is solved with an adaptive
    // time step, so the objective is only piecewise smooth: a perturbation
    // smaller than the solver's own error yields a gradient of numerical
    // noise. 1e-3 of the range is large enough to clear that on typical
    // settings and small enough to stay in the linear regime.
    double fd_step = 1e-3;
    bool   central_differences = false; // 2*nParam solves per iteration, less bias

    // Convergence. All three are tested; the first one met stops the run.
    double tol_objective = 1e-8;  // relative decrease in the NLL
    double tol_step      = 1e-8;  // relative length of the accepted step
    double tol_gradient  = 1e-8;  // max |J'r| component

    // Calibrating error_standard_deviation turns the objective into
    // A/(2 s^2) + c*log(s), which is not a sum of squares. Rather than give up,
    // profile it: s is set to its analytic maximiser at every iteration and
    // dropped from the LM parameter vector. The reported estimate is the
    // profile-likelihood one, which is what the GA converges to as well.
    // Setting this false leaves sigma in the vector and LM then only sees the
    // sum-of-squares part of its gradient, which converges poorly; it exists
    // to make the difference measurable, not because it is a good idea.
    bool profile_sigma = true;

    int  numthreads = 8;
    bool write_covariance = true;
};

struct _LM_filenames
{
    string pathname;
    string outputfilename = "LM_output.txt";
    string covariancefilename = "LM_covariance.txt";
};

template<class T>
class CLM
{
public:
    CLM();
    CLM(T *model);
    virtual ~CLM();

    LM_Parameters   LM_params;
    _LM_filenames   filenames;

    // Optimises and leaves the best model in Model_out, exactly as CGA does, so
    // the callers that already know how to harvest a GA result need no changes.
    // Returns the number of iterations taken, or -1 with last_error set.
    int optimize();

    bool SetProperty(const string &varname, const string &value);
    void SetParameters(Object *obj);
    string last_error;

    T  Model_out;
    T *Model = nullptr;

    // Results ------------------------------------------------------------
    vector<double> final_params;   // in model space (de-logged)
    vector<string> paramname;
    double final_objective = 0;    // the NLL at the solution
    // Forward model solves the run consumed. The directly comparable number for
    // a GA run is maxpop*ngen, so this is what says whether LM was worth it.
    long   model_solves = 0;

    // (J'J)^-1 at the solution, in the TRANSFORMED space LM searches in.
    // Meaningful only when the run converged; empty otherwise.
    //
    // arma::mat instead of the project's CMatrix_arma on purpose: System.h
    // macro-redefines CMatrix_arma to the non-Armadillo CMatrix under -DDEBUG,
    // which every current build sets, and LM's linear algebra needs the real
    // Armadillo type for solve()/inv_sympd().
    arma::mat covariance;
    arma::mat correlation;
    vector<double> standard_error;    // sqrt of the diagonal, transformed space
    bool covariance_valid = false;

#ifdef Q_GUI_SUPPORT
    void SetProgressWindow(ProgressWindow *_rtw) {rtw=_rtw;}
#endif

private:
    // Parameter bookkeeping, mirroring CGA's: one entry per MODEL parameter.
    vector<int>    loged;     // 1 = searched in log10 space
    vector<double> minval, maxval;  // bounds in the transformed space
    vector<double> x;         // current point, transformed space, all parameters
    // Indices into the above of the parameters LM actually varies: every model
    // parameter except the error standard deviations being profiled out.
    vector<int>    active;
    // Indices of parameters that drive one or more observations'
    // error_standard_deviation, together with the observations they drive.
    vector<int>            sigma_params;
    vector<vector<int>>    sigma_observations;

    vector<T> Models;         // scratch copies for the parallel Jacobian sweep

    bool  Initialize();
    void  IdentifySigmaParameters();
    // Applies `x_t` to `model` and solves. Fills `r` with the residual vector
    // and returns the negative log-likelihood, +1e18 when the solve failed.
    double Evaluate(const vector<double> &x_t, T &model, vector<double> &r);
    // Perturbs one active parameter at a time, in parallel, and fills J
    // (n_residual by n_active) column by column. Returns false if the residual
    // vector changed length under perturbation, which means the comparison
    // points themselves moved and a Jacobian is not defined.
    bool  ComputeJacobian(const vector<double> &x_t, const vector<double> &r0,
                          arma::mat &J);
    // Sets each profiled sigma to its analytic maximiser given the current
    // residuals. Returns true if any value changed.
    bool  ProfileSigmas(T &model, const vector<ResidualBlock> &blocks);
    void  Clamp(vector<double> &x_t) const;
    double ToModelSpace(int i, double xi) const { return loged[i]==1 ? pow(10,xi) : xi; }
    double ToSearchSpace(int i, double v) const { return loged[i]==1 ? log10(max(v,1e-300)) : v; }
    void  WriteHeader(FILE *f) const;
    void  WriteIteration(FILE *f, int iter, double f_now, double lambda,
                         const vector<double> &x_t, const char *status) const;
    void  WriteCovariance() const;
    void  Log(const string &s) const;

    int numberOfThreads = 8;
#ifdef Q_GUI_SUPPORT
    ProgressWindow *rtw = nullptr;
#endif
};

#include "LM.hpp"
