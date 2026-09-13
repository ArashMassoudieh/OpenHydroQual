/*
 * OpenHydroQual - Codegen runtime: transient solver driver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * A header-only, templated damped Newton + adaptive-timestep driver. It is
 * templated on the generated Model type so every callback is inlined and there
 * is NO virtual dispatch in the hot loop (extreme-efficiency requirement).
 *
 * The generated Model must provide these non-virtual members (see ohq_model.h
 * for the concept and demo/two_pond_model.h for a worked example):
 *
 *   int    nStates() const;
 *   void   initialValues();                      // set present state at t=tstart
 *   void   getState(double* X) const;            // present state  -> X[nStates]
 *   void   setState(const double* X);            // X -> present state
 *   void   snapshotPast();                       // past := present
 *   void   setDt(double dt);                     // store dt for the residual
 *   void   precomputeStep(double t_new);         // per-step (state-independent) values
 *   void   residual(const double* X, double* F) const;   // F[nStates]
 *   void   jacobian(const double* X, double* J) const;    // row-major nStates*nStates
 *
 * SolverSettings mirror the meaningful knobs of aquifolium solversettings.
 */
#ifndef OHQ_SOLVER_H
#define OHQ_SOLVER_H

#include <vector>
#include <cmath>
#include "ohq_linalg.h"

namespace ohq {

struct SolverSettings {
    double tolerance          = 1e-6;   // relative Newton tolerance
    double abs_floor          = 1e-12;  // absolute residual floor
    int    max_iterations     = 40;     // NR_niteration_max
    int    iter_lower         = 3;      // grow dt below this
    int    iter_upper         = 10;     // shrink dt above this
    double dt_reduce          = 0.5;    // NR_timestep_reduction_factor
    double dt_reduce_fail     = 0.4;    // NR_timestep_reduction_factor_fail
    double dt_grow            = 1.5;
    double dt_min_factor      = 1e-6;   // dt floor = dt0 * dt_min_factor
    double dt_max_factor      = 100.0;  // dt ceil  = dt0 * dt_max_factor
    double nr_coefficient     = 1.0;    // Newton damping (line-search scale)
    int    max_step_failures  = 20;
};

template <class Model>
class TransientSolver {
public:
    explicit TransientSolver(Model& m, SolverSettings s = SolverSettings{})
        : m_(m), s_(s) {}

    SolverSettings& settings() { return s_; }

    void initialize(double tstart, double dt0)
    {
        t_    = tstart;
        dt0_  = dt0;
        dt_   = dt0;
        const int n = m_.nStates();
        X_.assign(n, 0.0); F_.assign(n, 0.0);
        J_.assign(n * n, 0.0); dx_.assign(n, 0.0);
        m_.initialValues();
        last_iters_ = 0;
    }

    double time() const { return t_; }
    double dt()   const { return dt_; }

    // Advance exactly one accepted step; dt is chosen adaptively and may be
    // retried internally. Returns false only if the step floor is hit without
    // convergence. This is the external "one-step solve" entry point.
    bool step()
    {
        const double dt_min = dt0_ * s_.dt_min_factor;
        int failures = 0;
        for (;;) {
            m_.snapshotPast();
            m_.setDt(dt_);
            m_.precomputeStep(t_ + dt_);
            int iters = 0;
            if (newton(iters)) {
                t_ += dt_;
                adaptOnSuccess(iters);
                last_iters_ = iters;
                return true;
            }
            // failed: shrink and retry
            if (++failures > s_.max_step_failures) return false;
            dt_ *= s_.dt_reduce_fail;
            if (dt_ < dt_min) return false;
        }
    }

    // Run until t_target (inclusive within one dt). Returns false on failure.
    bool runTo(double t_target)
    {
        while (t_ < t_target - 1e-30) {
            if (t_ + dt_ > t_target) dt_ = t_target - t_;   // land exactly on target
            if (!step()) return false;
        }
        return true;
    }

    int lastIterations() const { return last_iters_; }

private:
    // One damped Newton solve for the current dt/past/precompute context.
    bool newton(int& iters)
    {
        const int n = m_.nStates();
        m_.getState(X_.data());
        m_.residual(X_.data(), F_.data());
        double err = norm(F_);
        double err_ini = err;
        double xnorm = norm(X_) + 1e-30;
        if (err < s_.abs_floor) { iters = 0; return true; }

        for (iters = 1; iters <= s_.max_iterations; ++iters) {
            m_.jacobian(X_.data(), J_.data());
            // solveInPlace overwrites J and F; keep F for damping check
            std::vector<double> Jcopy = J_;
            std::vector<double> Fcopy = F_;
            if (!solveInPlace(n, Jcopy.data(), Fcopy.data(), dx_.data())) return false;

            // damped update with a simple backtracking line search
            double lambda = s_.nr_coefficient;
            std::vector<double> Xtry(n);
            double err_try = err;
            for (int ls = 0; ls < 12; ++ls) {
                for (int i = 0; i < n; ++i) Xtry[i] = X_[i] - lambda * dx_[i];
                m_.setState(Xtry.data());
                m_.residual(Xtry.data(), F_.data());
                err_try = norm(F_);
                if (err_try < err || err_try < s_.abs_floor) break;
                lambda *= 0.5;
            }
            X_ = Xtry;
            double dxnorm = 0.0;
            for (int i = 0; i < n; ++i) dxnorm += (lambda * dx_[i]) * (lambda * dx_[i]);
            dxnorm = std::sqrt(dxnorm);
            xnorm = norm(X_) + 1e-30;
            err = err_try;

            const bool converged =
                err / (err_ini + 1e-8 * xnorm) < s_.tolerance ||
                err < s_.abs_floor ||
                dxnorm / xnorm < 1e-10;
            if (converged) { m_.setState(X_.data()); return true; }
        }
        return false;
    }

    void adaptOnSuccess(int iters)
    {
        if (iters > s_.iter_upper)      dt_ *= s_.dt_reduce;
        else if (iters < s_.iter_lower) dt_ *= s_.dt_grow;
        const double dt_max = dt0_ * s_.dt_max_factor;
        if (dt_ > dt_max) dt_ = dt_max;
    }

    static double norm(const std::vector<double>& v)
    {
        double s = 0.0; for (double x : v) s += x * x; return std::sqrt(s);
    }

    Model& m_;
    SolverSettings s_;
    double t_ = 0.0, dt_ = 0.0, dt0_ = 0.0;
    int last_iters_ = 0;
    std::vector<double> X_, F_, J_, dx_;
};

} // namespace ohq

#endif // OHQ_SOLVER_H
