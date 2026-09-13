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
#include "ohq_linalg.h"
#include "ohq_solver.h"   // SolverSettings

namespace ohq {

template <class Model>
class MassBalanceSolver {
public:
    explicit MassBalanceSolver(Model& m, SolverSettings s = SolverSettings{})
        : m_(m), s_(s) {}

    SolverSettings& settings() { return s_; }
    double landtozero = 0.0;   // matches settings.json default

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
    }

    double time() const { return t_; }
    double dt()   const { return dt_; }
    double storage(int b) const { return storage_[b]; }
    bool   isLimited(int b) const { return limited_[b] != 0; }
    int    lastIterations() const { return last_iters_; }

    bool step()
    {
        const double dt_min = dt0_ * s_.dt_min_factor;
        int stepFails = 0;
        for (;;) {
            for (int b = 0; b < n_; ++b) past_[b] = storage_[b];
            std::vector<char> limitedAtStart = limited_;
            std::vector<double> factorAtStart = factor_;
            tnew_ = t_ + dt_;
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
                    } else if (X_[b] >= 1.0 && limited_[b]) {
                        limited_[b] = 0; allow_[b] = 0; switched = true;
                    } else if (X_[b] < 0.0 && limited_[b]) {
                        factor_[b] = 0.0;
                    }
                }
                if (switched) ++attempts;
            }

            if (ok && !switched) {
                for (int b = 0; b < n_; ++b) {
                    if (limited_[b]) { factor_[b] = X_[b]; storage_[b] = past_[b] * landtozero; }
                    else             { storage_[b] = X_[b]; }
                }
                t_ += dt_;
                if (iters_last_ > s_.iter_upper)      dt_ *= s_.dt_reduce;
                else if (iters_last_ < s_.iter_lower) dt_ *= s_.dt_grow;
                if (dt_ > dt0_ * s_.dt_max_factor) dt_ = dt0_ * s_.dt_max_factor;
                return true;
            }
            // failed: restore limited state, shrink dt, retry
            limited_ = limitedAtStart; factor_ = factorAtStart;
            if (++stepFails > s_.max_step_failures) return false;
            dt_ *= s_.dt_reduce_fail;
            if (dt_ < dt_min) return false;
        }
    }

    bool runTo(double t_target)
    {
        while (t_ < t_target - 1e-30) {
            if (t_ + dt_ > t_target) dt_ = t_target - t_;
            if (!step()) return false;
        }
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
                F[b] = -past_[b] * (1.0 - landtozero) / dt_ - inflow;
            } else {
                F[b] = (X[b] - past_[b]) / dt_ - inflowOwn_[b];
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
        assemble(X_.data(), F_.data());
        double err = norm(F_), err_ini = err, xnorm = norm(X_) + 1e-30;
        if (err < s_.abs_floor) { iters = 0; iters_last_ = 0; return true; }
        std::vector<double> J(n_ * n_), Jc(n_ * n_), Fc(n_), dx(n_), Xtry(n_), F0(n_);
        for (iters = 1; iters <= s_.max_iterations; ++iters) {
            // numerical Jacobian (FD), matching the interpreter's default column kernel
            assemble(X_.data(), F0.data());
            for (int j = 0; j < n_; ++j) {
                const double eps = -1e-6 * (std::fabs(X_[j]) + 1.0);
                const double save = X_[j]; X_[j] += eps;
                assemble(X_.data(), F_.data());
                for (int i = 0; i < n_; ++i) J[i * n_ + j] = (F_[i] - F0[i]) / eps;
                X_[j] = save;
            }
            Jc = J; Fc = F0;
            if (!solveInPlace(n_, Jc.data(), Fc.data(), dx.data())) return false;
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

    static double norm(const std::vector<double>& v)
    { double s = 0; for (double x : v) s += x*x; return std::sqrt(s); }

    Model& m_;
    SolverSettings s_;
    int n_ = 0, nl_ = 0, last_iters_ = 0, iters_last_ = 0;
    double t_ = 0, dt_ = 0, dt0_ = 0, tnew_ = 0;
    std::vector<double> storage_, past_, factor_, X_, F_, eff_, flowRaw_, inflowOwn_;
    std::vector<char> limited_, allow_;
    std::vector<std::vector<int>> linksFrom_, linksTo_;
};

} // namespace ohq

#endif // OHQ_MASSBALANCE_H
