/*
 * OpenHydroQual - Codegen runtime: mass-balance solver with outflow limiting
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * A header-only, templated transient solver that reproduces the interpreter's
 * finite-volume mass balance AND its `applylimit` outflow-limiting complementarity
 * (aquifolium System::GetResiduals + the SetLimitedOutFlow switching loop in
 * OneStepSolve). No virtual dispatch; the Model's hooks are inlined.
 *
 * When a block would go negative, it flips to "limited" mode: its storage is
 * pinned to past*landtozero (~0) and its state unknown becomes an outflow
 * factor in [0,1] that scales its outflows so it drains exactly to empty. The
 * flag propagates through rigid neighbours. An outer loop re-solves until the
 * limited set is stable (mirrors OneStepSolve's `attempts` loop).
 *
 * Required Model interface (all non-virtual):
 *   int  nBlocks() const;
 *   int  nLinks()  const;
 *   int  linkSrc(int l) const;      // source block index of link l
 *   int  linkDst(int l) const;      // destination block index of link l
 *   bool rigid(int b) const;        // is block b rigid for the state variable
 *   void initialStorage(double* s); // initial storages [nBlocks]
 *   void precomputeStep(double t);  // per-step (state-independent) values
 *   void computeFluxes(const double* effStorage, double t,
 *                      double* flowRaw, double* inflowOwn) const;
 *        // flowRaw[nLinks]  : raw link flows (before limiting factors)
 *        // inflowOwn[nBlocks]: sum of each block's own inflow quantities
 */
#ifndef OHQ_MASSBALANCE_H
#define OHQ_MASSBALANCE_H

#include <vector>
#include <cmath>
#include <algorithm>
#include "ohq_linalg.h"
#include "ohq_sparse.h"
#include "ohq_solver.h"   // SolverSettings
#include "ohq_timeseries.h"   // dt clamp series (interpol_D)

namespace ohq {

template <class Model>
class MassBalanceSolver {
public:
    explicit MassBalanceSolver(Model& m, SolverSettings s = SolverSettings{})
        : m_(m), s_(s) {}

    SolverSettings& settings() { return s_; }
    double landtozero = 0.0;   // matches settings.json default

    // Forcing breakpoints (sorted): time-series sample times. dt is clamped so a
    // step never crosses one, matching the interpreter's GetMinimumNextTimeStepSize
    // so spiky forcing (e.g. a rainfall pulse) is never stepped over.
    std::vector<double> breakpoints;
    void setBreakpoints(const double* a, int n) { breakpoints.assign(a, a + n); }
    // Interpreter dt policy (System::Solve loop + GetMinimumNextTimeStepSize):
    // the APPLIED step is max(min(dt_base, min_series interpol_D(t)), dt0/timestepminfactor)
    // over the registered series -- GetTimeSeries(true): PRECIPITATION series only --
    // while dt_base (dt_) is the adaptive quantity that grows/shrinks on its own.
    std::vector<const TimeSeries*> clampSeries_;
    void addClampSeries(const TimeSeries* ts) { clampSeries_.push_back(ts); }
    double minNextDt() const
    {
        double x = 1e12;
        // empty series (not loaded / not injected yet) are not registered by the
        // interpreter at all; an injected one becomes an active clamp automatically
        for (const TimeSeries* s : clampSeries_) if (!s->empty()) x = std::min(x, s->interpolD(t_));
        return std::max(x, 0.001);
    }
    double lastDt() const { return lastDt_; }   // size of the last ACCEPTED step
    // Hard stop time: dt is clamped so a step never crosses it (used by the
    // two-phase transport runTo so both phases land exactly on t_end).
    double stopTime_ = 1e300;
    void setStop(double t) { stopTime_ = t; }

    void initialize(double tstart, double dt0)
    {
        n_ = m_.nBlocks();
        nl_ = m_.nLinks();
        t_ = tstart; dt0_ = dt0; dt_ = dt0;
        storage_.assign(n_, 0.0); past_.assign(n_, 0.0);
        factor_.assign(n_, 1.0); limited_.assign(n_, 0); allow_.assign(n_, 1);
        X_.assign(n_, 0.0); F_.assign(n_, 0.0); eff_.assign(n_, 0.0);
        flowRaw_.assign(nl_ > 0 ? nl_ : 1, 0.0);
        inflowOwn_.assign(n_, 0.0);
        // adjacency for OutFlowCanOccur / propagation
        linksFrom_.assign(n_, {}); linksTo_.assign(n_, {});
        for (int l = 0; l < nl_; ++l) {
            linksFrom_[m_.linkSrc(l)].push_back(l);
            linksTo_[m_.linkDst(l)].push_back(l);
        }
        m_.initialStorage(storage_.data());
        last_iters_ = 0;

        // sparse Jacobian pattern: block adjacency (self + link-connected blocks)
        nbr_.assign(n_, {});
        std::vector<std::vector<int>> pattern(n_);
        for (int b = 0; b < n_; ++b) {
            std::vector<int> cols{b};
            for (int l : linksFrom_[b]) cols.push_back(m_.linkDst(l));
            for (int l : linksTo_[b])   cols.push_back(m_.linkSrc(l));
            std::sort(cols.begin(), cols.end());
            cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
            pattern[b] = cols;
            for (int c : cols) if (c != b) nbr_[b].push_back(c);
        }
        useSparse_ = (n_ >= 100);
        if (useSparse_) {
            jac_.build(pattern);
            // Greedy distance-2 coloring so columns sharing no residual row are
            // perturbed together: the FD Jacobian then costs ~(#colors) residual
            // evals instead of n. (Two columns conflict if within graph distance 2.)
            color_.assign(n_, -1);
            for (int j = 0; j < n_; ++j) {
                std::vector<int> used;
                auto note = [&](int k){ if (color_[k] >= 0) used.push_back(color_[k]); };
                note(j);
                for (int a : nbr_[j]) { note(a); for (int b : nbr_[a]) note(b); }
                std::sort(used.begin(), used.end());
                int c = 0; for (int u : used) { if (u == c) ++c; else if (u > c) break; }
                color_[j] = c;
            }
            int ncol = 0; for (int c : color_) ncol = std::max(ncol, c + 1);
            colorGroups_.assign(ncol, {});
            for (int j = 0; j < n_; ++j) colorGroups_[color_[j]].push_back(j);
        }
    }

    double time() const { return t_; }
    double dt()   const { return dt_; }
    double storage(int b) const { return storage_[b]; }
    bool   isLimited(int b) const { return limited_[b] != 0; }
    int    lastIterations() const { return last_iters_; }
    // Committed link flow (raw flow * outflow-limit factor) from the last step;
    // consumed by the transport phase.
    double linkFlow(int l) const { return committedFlow_.empty() ? 0.0 : committedFlow_[l]; }
    double limitFactor(int b) const { return factor_[b]; }

    // G5: restart from state VALUES (a snapshot's storages and outflow-limiting
    // state) at time t. dt_base restarts from dt0, as System::Solve does at the
    // start of every solve. limited / factor may be null (none / 1.0).
    void setState(double t, const double* storage, const int* limited, const double* factor)
    {
        t_ = t; dt_ = dt0_; lastDt_ = 0.0; last_iters_ = 0;
        for (int b = 0; b < n_; ++b) {
            storage_[b] = storage[b]; past_[b] = storage[b];
            limited_[b] = limited ? (limited[b] ? 1 : 0) : 0;
            factor_[b]  = factor ? factor[b] : 1.0;
            allow_[b]   = 1;
        }
        committedFlow_.assign(nl_ > 0 ? nl_ : 1, 0.0);
    }

    // dt ceiling imposed by an oscillation rewind (System.cpp:1140): clamps the
    // adaptive base so it cannot climb straight back to where it oscillated.
    // force a Jacobian refresh (System::SetUpdateJacobian(true))
    void requestJacobianUpdate() { updateJac_ = true; }
    void   setDtCeiling(double v) { dtCeiling_ = v; }
    double dtCeiling() const      { return dtCeiling_; }
    double dtBase() const         { return dt_; }
    void   setDtBase(double v)    { dt_ = v; }

    bool step()
    {
        int stepFails = 0;
        if (dtCeiling_ > 0) dt_ = std::min(dt_, dtCeiling_);
        for (;;) {
            // applied step (see clampSeries_ comment); dt_ stays the adaptive base
            double dta = std::min(dt_, minNextDt());
            dta = std::max(dta, dt0_ * s_.dt_floor_factor);
            if (!breakpoints.empty()) {   // optional hard breakpoints (not emitted by the generator)
                auto it = std::upper_bound(breakpoints.begin(), breakpoints.end(), t_ + 1e-12);
                if (it != breakpoints.end() && t_ + dta > *it) dta = *it - t_;
            }
            if (stopTime_ - t_ > 1e-30 && t_ + dta > stopTime_) dta = stopTime_ - t_;
            dtApplied_ = dta;
            // System.cpp:1138 -- every 50th accepted step the Jacobian is refreshed
            // unconditionally, whatever the chord scheme thinks
            // System.cpp:1136-1138 increments `counter` BEFORE testing it, so the
            // refresh lands on the 50th, 100th, ... accepted step (1-based).
            if (s_.jac_refresh_every > 0 && (stepCounter_ + 1) % s_.jac_refresh_every == 0)
                updateJac_ = true;
            for (int b = 0; b < n_; ++b) past_[b] = storage_[b];
            std::vector<char> limitedAtStart = limited_;
            std::vector<double> factorAtStart = factor_;
            // Interpreter convention (System.cpp:1494): SolverTempVars.t is advanced
            // in HandleSolveSuccess, AFTER OneStepSolve, so every time-dependent
            // expression evaluated during the Newton solve reads GetTime() = the
            // step's START time. Backward Euler in the state, forcing at t_n.
            // Evaluating at t_n+1 instead shifts the whole residual sequence by one
            // step, which is what put the two codes' Jacobians on different dt.
            tnew_ = s_.forcing_at_step_start ? t_ : t_ + dta;
            m_.precomputeStep(tnew_);
            for (int b = 0; b < n_; ++b) allow_[b] = 1;

            bool ok = true, switched = true;
            int attempts = 0;
            while (attempts < n_ + 1 && switched) {
                switched = false;
                for (int b = 0; b < n_; ++b) X_[b] = limited_[b] ? factor_[b] : past_[b];
                int iters = 0;
                if (!newton(iters)) { ok = false; break; }
                last_iters_ = iters;
                // switching rules (mirror OneStepSolve)
                for (int b = 0; b < n_; ++b) {
                    if (X_[b] < -1e-13 && !limited_[b] && outflowCanOccur(b)) {
                        setLimited(b, true);
                        switched = true;
                        updateJac_ = true;            // System.cpp:2505
                    } else if (X_[b] >= 1.0 && limited_[b]) {
                        limited_[b] = 0; allow_[b] = 0; switched = true;
                        updateJac_ = true;            // System.cpp:2525
                    } else if (X_[b] < 0.0 && limited_[b]) {
                        factor_[b] = 0.0;
                    }
                }
                if (switched) ++attempts;
            }

            if (ok && !switched) {
                // refresh fluxes at the converged solution, then record committed
                // link flows (raw flow * limiting factor) for the transport phase
                assemble(X_.data(), F_.data());
                committedFlow_.assign(nl_ > 0 ? nl_ : 1, 0.0);
                for (int l = 0; l < nl_; ++l) {
                    const int s = m_.linkSrc(l), e = m_.linkDst(l);
                    const double q = flowRaw_[l];
                    double factor = 1.0;
                    if (limited_[s] && q > 0)      factor = X_[s];
                    else if (limited_[e] && q < 0) factor = X_[e];
                    committedFlow_[l] = q * factor;
                }
                for (int b = 0; b < n_; ++b) {
                    if (limited_[b]) { factor_[b] = X_[b]; storage_[b] = past_[b] * landtozero; }
                    else             { storage_[b] = X_[b]; }
                }
                t_ += dta; lastDt_ = dta;
                // post-success adaptation of dt_base (System.cpp ~1591-1605): shrink from the
                // APPLIED dt (floored at minimum_timestep), grow the base (capped)
                ++stepCounter_;
                // System.cpp:1620 -- an iteration overshoot also forces a refresh
                // and resets the damping coefficient
                if (iters_last_ > s_.iter_upper) { updateJac_ = true; nrCoeff_ = 1.0; }
                if (iters_last_ > s_.iter_upper)      dt_ = std::max(dta * s_.dt_reduce, s_.dt_abs_min);
                else if (iters_last_ < s_.iter_lower) dt_ = std::min(dt_ * s_.dt_grow, dt0_ * s_.dt_max_factor);
                return true;
            }
            // failed (HandleSolveFailure): restore limited state, shrink dt_base, floor, retry
            limited_ = limitedAtStart; factor_ = factorAtStart;
            updateJac_ = true;                      // System.cpp:1399 (HandleSolveFailure)
            if (++stepFails > s_.max_step_failures) return false;
            dt_ = std::max(dt_ * s_.dt_reduce_fail, 0.5 * dt0_ * s_.dt_floor_factor);
        }
    }

    bool runTo(double t_target)
    {
        const double prevStop = stopTime_;
        stopTime_ = t_target;
        while (t_ < t_target - 1e-30) {
            if (!step()) { stopTime_ = prevStop; return false; }
        }
        stopTime_ = prevStop;
        return true;
    }

private:
    void assemble(const double* X, double* F)
    {
        for (int b = 0; b < n_; ++b)
            eff_[b] = limited_[b] ? past_[b] * landtozero : X[b];
        m_.computeFluxes(eff_.data(), tnew_, flowRaw_.data(), inflowOwn_.data());

        for (int b = 0; b < n_; ++b) {
            if (m_.rigid(b)) {
                F[b] = -inflowOwn_[b];
            } else if (limited_[b]) {
                double inflow = inflowOwn_[b];
                if (inflow < 0) inflow *= X[b];
                F[b] = -past_[b] * (1.0 - landtozero) / dtApplied_ - inflow;
            } else {
                F[b] = (X[b] - past_[b]) / dtApplied_ - inflowOwn_[b];
            }
        }
        for (int l = 0; l < nl_; ++l) {
            const int s = m_.linkSrc(l), e = m_.linkDst(l);
            const double q = flowRaw_[l];
            double factor = 1.0;
            if (limited_[s] && q > 0)      factor = X[s];
            else if (limited_[e] && q < 0) factor = X[e];
            const double lf = q * factor;
            F[s] += lf; F[e] -= lf;
        }
        for (int b = 0; b < n_; ++b)
            if (limited_[b] && !outflowCanOccur(b)) F[b] = X[b] - 1.1;
    }

    bool outflowCanOccur(int b)
    {
        const double tol = 0.0;
        if (inflowOwn_[b] < -tol) return true;         // negative own inflow = outflow
        for (int l : linksFrom_[b]) if (flowRaw_[l] > tol) return true;  // leaves b forward
        for (int l : linksTo_[b])   if (flowRaw_[l] < -tol) return true; // leaves b in reverse
        return false;
    }

    // Flag block b limited and propagate through rigid neighbours reached by an
    // outflow, mirroring System::SetLimitedOutFlow.
    void setLimited(int b, bool on)
    {
        std::vector<char> visited(n_, 0);
        setLimitedRec(b, on, visited);
    }
    void setLimitedRec(int b, bool on, std::vector<char>& visited)
    {
        if (visited[b]) return; visited[b] = 1;
        limited_[b] = on ? 1 : 0;
        factor_[b] = on ? 0.9999 : 1.0;
        if (!on) return;
        for (int l : linksFrom_[b]) {
            int nb = m_.linkDst(l);
            if (m_.rigid(nb) && flowRaw_[l] > 0 && allow_[nb]) setLimitedRec(nb, on, visited);
        }
        for (int l : linksTo_[b]) {
            int nb = m_.linkSrc(l);
            if (m_.rigid(nb) && flowRaw_[l] < 0 && allow_[nb]) setLimitedRec(nb, on, visited);
        }
    }

    bool newton(int& iters)
    {
        if (s_.interpreter_newton) return newtonInterp(iters);
        return newtonLineSearch(iters);
    }

    // Faithful port of System::OneStepSolve's Newton loop (System.cpp:2361-2492)
    // together with ComputeNewtonStep (6320-6325) and AdjustNRCoefficient (6450).
    // Every quirk is deliberate and load-bearing for parity:
    //   - X_norm is taken ONCE from the initial guess and never refreshed;
    //   - the convergence test runs BEFORE each iteration, on the previous err;
    //   - dx = NR_coefficient * J^-1 F, and F is the residual at the PREVIOUS
    //     iterate -- after X is replaced by the half-step X1 the stored F is not
    //     recomputed, so the next step is taken from a stale right-hand side;
    //   - the Jacobian flag persists across steps unless something requests it.
    bool newtonInterp(int& iters)
    {
        const double X_norm = norm(X_) + 0.0;
        double dx_norm = X_norm * 10 + 1;
        // The interpreter keeps two things that are easy to conflate:
        //   X_    -- the state last written into the model by GetResiduals, i.e.
        //            the FULL damped step. This is what the step commits.
        //   Xit   -- the local iterate, which AdjustNRCoefficient may set to the
        //            half-step X1 (or back to X_past). It only feeds the NEXT
        //            iteration; it is never written into the model.
        // Committing X1 instead of X_ doubles the accepted increment whenever the
        // loop exits right after the half-step was adopted -- which is every step
        // in the transport phase, where the dx_norm escape stops it at one
        // iteration. (System.cpp:2457-2492 / 6417-6440.)
        std::vector<double> Xit = X_, X_past = X_;
        assemble(X_.data(), F_.data());
        double err_ini = norm(F_);
        double err = err_ini, err_p = err_ini;
        double error_increase_counter = 0;
        nrCoeff_ = 1.0;                       // System.cpp:2375, reset per attempt
        iters = 0;
        if (X_norm <= 0.0) { iters_last_ = 0; return true; }
        std::vector<double> dx(n_), X1(n_), F1(n_);
        while (err / (err_ini + 1e-8 * X_norm) > s_.tolerance && err > 1e-12
               && dx_norm / X_norm > 1e-10)
        {
            ++iters;
            if (s_.update_jacobian_every_iteration) updateJac_ = true;
            if (updateJac_) {
                std::vector<double> F0(F_);
                if (!assembleJacobian(F0)) { iters_last_ = iters; return false; }
                updateJac_ = false;
            }
            if (!solveCached(F_, dx)) { iters_last_ = iters; return false; }
            for (int i = 0; i < n_; ++i) dx[i] *= nrCoeff_;
            dx_norm = norm(dx);
            for (int i = 0; i < n_; ++i) X_[i] = Xit[i] - dx[i];     // full step
            if (s_.optimize_lambda) {
                for (int i = 0; i < n_; ++i) X1[i] = X_[i] + 0.5 * dx[i];
                assemble(X1.data(), F1.data());
            }
            assemble(X_.data(), F_.data());   // model ends the iteration at X_
            err_p = err;
            err = norm(F_);
            if (s_.optimize_lambda) {
                const double err2 = norm(F1);
                if (err2 < err) {
                    nrCoeff_ = std::max(nrCoeff_ / 2.0, 0.05);
                    updateJac_ = true;
                    Xit = X1;
                } else {
                    nrCoeff_ = std::max(std::min(nrCoeff_ * 1.25, 1.0), 0.05);
                    Xit = X_;
                }
                if (std::min(err2, err) > err_p) error_increase_counter++;
            } else {
                if (err > err_p * 0.9) {
                    nrCoeff_ = std::max(nrCoeff_ * s_.nr_coeff_reduction, 0.05);
                    updateJac_ = true;
                    Xit = X_past;
                } else {
                    Xit = X_;
                }
                if (err > err_p) error_increase_counter++;
                else if (err < err_p / 2.0) {
                    if (nrCoeff_ < 0.99) updateJac_ = true;
                    nrCoeff_ = std::max(std::min(nrCoeff_ / s_.nr_coeff_reduction, 1.0), 0.05);
                }
            }
            if (error_increase_counter > 10) { iters_last_ = iters; return false; }
            if (iters > s_.max_iterations)    { iters_last_ = iters; return false; }
        }
        iters_last_ = iters;
        return true;
    }

    bool newtonLineSearch(int& iters)
    {
        assemble(X_.data(), F_.data());
        double err = norm(F_), err_ini = err, xnorm = norm(X_) + 1e-30;
        if (err < s_.abs_floor) { iters = 0; iters_last_ = 0; return true; }
        std::vector<double> dx(n_), Xtry(n_), F0(n_);
        for (iters = 1; iters <= s_.max_iterations; ++iters) {
            assemble(X_.data(), F0.data());
            if (!computeStep(F0, dx)) return false;
            double lambda = s_.nr_coefficient, err_try = err;
            for (int ls = 0; ls < 12; ++ls) {
                for (int i = 0; i < n_; ++i) Xtry[i] = X_[i] - lambda * dx[i];
                assemble(Xtry.data(), F_.data());
                err_try = norm(F_);
                if (err_try < err || err_try < s_.abs_floor) break;
                lambda *= 0.5;
            }
            X_ = Xtry;
            double dxn = 0; for (int i = 0; i < n_; ++i) dxn += (lambda*dx[i])*(lambda*dx[i]);
            dxn = std::sqrt(dxn); xnorm = norm(X_) + 1e-30; err = err_try;
            if (err/(err_ini+1e-8*xnorm) < s_.tolerance || err < s_.abs_floor || dxn/xnorm < 1e-10) {
                iters_last_ = iters; return true;
            }
        }
        iters_last_ = iters; return false;
    }

    // ---- Jacobian: assembly and solve, kept separate so the factors can be
    // reused across iterations AND across steps, as SolverTempVars.updatejacobian
    // does in the interpreter. Sparse (ILU0-BiCGSTAB over the block-adjacency
    // pattern) for large systems, dense LU for small ones.
    bool assembleJacobian(const std::vector<double>& F0)
    {
        std::vector<double> Fp(n_);
        if (useSparse_) {
            jac_.setZero();
            std::vector<double> eps(n_, 0.0);
            for (const auto& group : colorGroups_) {
                for (int j : group) { eps[j] = -1e-6 * (std::fabs(X_[j]) + 1.0); X_[j] += eps[j]; }
                assemble(X_.data(), Fp.data());
                for (int j : group) {
                    jac_.add(j, j, (Fp[j] - F0[j]) / eps[j]);
                    for (int i : nbr_[j]) jac_.add(i, j, (Fp[i] - F0[i]) / eps[j]);
                }
                for (int j : group) X_[j] -= eps[j];   // restore
            }
            haveSparseJac_ = true;
            // Keep a dense LU of the SAME matrix. BiCGSTAB fails routinely on this
            // Jacobian (badly scaled rows), and the fallback must solve the matrix
            // we assembled -- re-assembling a fresh one there silently turns the
            // chord scheme into true Newton, which is exactly what made codegen
            // converge in one iteration where the interpreter needs twenty.
            dense_.assign(n_ * n_, 0.0);
            for (int i = 0; i < n_; ++i)
                for (int k = jac_.rowptr[i]; k < jac_.rowptr[i + 1]; ++k)
                    dense_[i * n_ + jac_.colidx[k]] = jac_.val[k];
            piv_.assign(n_, 0);
            return luFactor(n_, dense_.data(), piv_.data());
        }
        dense_.assign(n_ * n_, 0.0);
        for (int j = 0; j < n_; ++j) {
            const double eps = -1e-6 * (std::fabs(X_[j]) + 1.0);
            const double save = X_[j]; X_[j] += eps;
            assemble(X_.data(), Fp.data());
            for (int i = 0; i < n_; ++i) dense_[i * n_ + j] = (Fp[i] - F0[i]) / eps;
            X_[j] = save;
        }
        piv_.assign(n_, 0);
        haveSparseJac_ = false;
        return luFactor(n_, dense_.data(), piv_.data());
    }

    // dx = J^-1 * rhs, using the stored Jacobian / factors.
    bool solveCached(const std::vector<double>& rhs, std::vector<double>& dx)
    {
        if (haveSparseJac_) {
            std::fill(dx.begin(), dx.end(), 0.0);
            std::vector<double> b(rhs);
            if (bicgstab(jac_, b.data(), dx.data())) return true;
            // BiCGSTAB stalled: direct solve of the SAME cached matrix
            luSolve(n_, dense_.data(), piv_.data(), rhs.data(), dx.data());
            return true;
        }
        luSolve(n_, dense_.data(), piv_.data(), rhs.data(), dx.data());
        return true;
    }

    // Newton step (legacy path): fresh FD Jacobian + solve, every iteration.
    bool computeStep(const std::vector<double>& F0, std::vector<double>& dx)
    {
        if (!assembleJacobian(F0)) return false;
        return solveCached(F0, dx);
    }

    static double norm(const std::vector<double>& v)
    { double s = 0; for (double x : v) s += x*x; return std::sqrt(s); }

    Model& m_;
    SolverSettings s_;
    int n_ = 0, nl_ = 0, last_iters_ = 0, iters_last_ = 0;
    double t_ = 0, dt_ = 0, dt0_ = 0, tnew_ = 0;
    double dtApplied_ = 0, lastDt_ = 0;   // applied step of the current / last accepted step
    std::vector<double> storage_, past_, factor_, X_, F_, eff_, flowRaw_, inflowOwn_;
    std::vector<double> committedFlow_;
    std::vector<char> limited_, allow_;
    std::vector<std::vector<int>> linksFrom_, linksTo_;
    // sparse Jacobian machinery
    SparseCSR jac_;
    std::vector<std::vector<int>> nbr_;   // block adjacency (excludes self)
    std::vector<int> color_;
    std::vector<std::vector<int>> colorGroups_;
    bool useSparse_ = false;
    // interpreter-parity Newton state; updateJac_ deliberately PERSISTS across
    // steps, as SolverTempVars.updatejacobian does
    std::vector<double> dense_;
    std::vector<int> piv_;
    bool haveSparseJac_ = false;
    bool updateJac_ = true;
    double nrCoeff_ = 1.0;
    long stepCounter_ = 0;
    double dtCeiling_ = 0;
};

} // namespace ohq

#endif // OHQ_MASSBALANCE_H
