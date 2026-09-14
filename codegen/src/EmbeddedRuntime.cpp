/*
 * OpenHydroQual - Codegen: embedded header-only runtime (GENERATED FILE)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Produced by codegen/tools/embed_runtime.py from the codegen/runtime headers.
 * Do not edit by hand -- edit the runtime headers and re-run the script.
 */
#include "EmbeddedRuntime.h"

namespace ohqcg {

static const RuntimeFile kRuntimeFiles[] = {
    { "ohq_intrinsics.h",
R"OHQRT(/*
 * OpenHydroQual - Codegen runtime: scalar intrinsics
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Header-only, dependency-free (only <cmath>). These reproduce the exact
 * semantics of aquifolium/src/Expression.cpp Expression::func(...) so that a
 * generated, hard-coded solver is numerically identical to the interpreter.
 *
 * Interpreter name  ->  runtime function
 *   _exp _log _abs _sgn _sqr _sqt _pos _hsd   (one argument)
 *   _min _max _mon _mbs _lpw                   (two arguments)
 *   _ups _bkw                                  (three arguments)
 *   ^ operator                                 -> ohq::powr
 * The stateful/topological kernels _ekr and _gkr live in ohq_timeseries.h.
 */
#ifndef OHQ_INTRINSICS_H
#define OHQ_INTRINSICS_H

#include <cmath>

namespace ohq {

// ---- one-argument intrinsics -------------------------------------------------
inline double pos(double v)  { return (v + std::fabs(v)) * 0.5; }      // _pos
inline double hsd(double v)  { return v >= 0.0 ? 1.0 : 0.0; }          // _hsd (Heaviside)
inline double sgn(double v)  { return v > 0.0 ? 1.0 : -1.0; }          // _sgn
inline double f_exp(double v){ return std::exp(v); }                    // _exp
inline double f_log(double v){ return v > 0.0 ? std::log(v) : -1e12; }  // _log (guarded)
inline double f_abs(double v){ return std::fabs(v); }                   // _abs
inline double f_sqr(double v){ return std::sqrt(pos(v)); }              // _sqr = sqrt of positive part
inline double f_sqt(double v)                                           // _sqt = signed regularized sqrt
{
    const double a = v * v / (std::fabs(v) + 1e-4);
    return v > 0.0 ? std::sqrt(a) : -std::sqrt(a);
}

// ---- two-argument intrinsics -------------------------------------------------
inline double f_min(double a, double b){ return a < b ? a : b; }        // _min
inline double f_max(double a, double b){ return a > b ? a : b; }        // _max
inline double mon(double a, double b)  { return a / (a + b); }          // _mon  (Monod-like)
inline double mbs(double a, double b)  { return std::fabs(a) / (std::fabs(a) + b); } // _mbs
inline double lpw(double a, double b)                                   // _lpw
{
    return std::pow(std::fabs(a), 1.0 - (1.0 - b) * a / (1e-6 + a));
}

// ---- three-argument intrinsics ----------------------------------------------
inline double ups(double cond, double a, double b){ return cond >= 0.0 ? a : b; } // _ups
inline double bkw(double cond, double a, double b){ return cond >= 0.0 ? a : b; } // _bkw

// ---- operator '^' ------------------------------------------------------------
// Expression::oprt: pow(aquiutils::Pos(val1), val2) -- the base is clamped at 0.
inline double powr(double a, double b){ return std::pow(a > 0.0 ? a : 0.0, b); }

// ---- operator '/' ------------------------------------------------------------
// Expression::oprt: val1 / (val2 + 1e-23). The regularizer makes 0/0 -> 0
// (a missing quantity is 0 in the interpreter), which models rely on.
inline double div(double a, double b){ return a / (b + 1e-23); }

} // namespace ohq

#endif // OHQ_INTRINSICS_H
)OHQRT" },
    { "ohq_linalg.h",
R"OHQRT(/*
 * OpenHydroQual - Codegen runtime: tiny dense linear solver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Dependency-free Gaussian elimination with partial pivoting for the small,
 * dense Newton systems produced per state-variable group. Row-major storage.
 * For large sparse systems a CSR/SuperLU backend can be swapped in behind the
 * same solveInPlace() signature; kept header-only and allocation-light here so
 * the generated library embeds without external linear-algebra dependencies.
 */
#ifndef OHQ_LINALG_H
#define OHQ_LINALG_H

#include <vector>
#include <cmath>

namespace ohq {

// Solve J * x = b for x, with J row-major n*n. J and b are overwritten.
// Returns false if the matrix is singular. x receives the solution.
inline bool solveInPlace(int n, double* J, double* b, double* x)
{
    for (int col = 0; col < n; ++col) {
        // partial pivot
        int piv = col;
        double best = std::fabs(J[col * n + col]);
        for (int r = col + 1; r < n; ++r) {
            double v = std::fabs(J[r * n + col]);
            if (v > best) { best = v; piv = r; }
        }
        if (best < 1e-300) return false;
        if (piv != col) {
            for (int k = 0; k < n; ++k) std::swap(J[col * n + k], J[piv * n + k]);
            std::swap(b[col], b[piv]);
        }
        // eliminate below
        const double diag = J[col * n + col];
        for (int r = col + 1; r < n; ++r) {
            const double f = J[r * n + col] / diag;
            if (f == 0.0) continue;
            for (int k = col; k < n; ++k) J[r * n + k] -= f * J[col * n + k];
            b[r] -= f * b[col];
        }
    }
    // back substitution
    for (int r = n - 1; r >= 0; --r) {
        double s = b[r];
        for (int k = r + 1; k < n; ++k) s -= J[r * n + k] * x[k];
        x[r] = s / J[r * n + r];
    }
    return true;
}

// LU factorisation with partial pivoting, so one Jacobian can serve several
// Newton iterations (the interpreter's lagged-Jacobian / chord scheme).
// A is overwritten with its LU factors; piv receives the row swaps.
inline bool luFactor(int n, double* A, int* piv)
{
    for (int col = 0; col < n; ++col) {
        int p = col; double best = std::fabs(A[col * n + col]);
        for (int r = col + 1; r < n; ++r) {
            double v = std::fabs(A[r * n + col]);
            if (v > best) { best = v; p = r; }
        }
        if (best < 1e-300) return false;
        piv[col] = p;
        if (p != col) for (int k = 0; k < n; ++k) std::swap(A[col * n + k], A[p * n + k]);
        const double diag = A[col * n + col];
        for (int r = col + 1; r < n; ++r) {
            const double f = A[r * n + col] / diag;
            A[r * n + col] = f;                       // store the multiplier (L)
            if (f == 0.0) continue;
            for (int k = col + 1; k < n; ++k) A[r * n + k] -= f * A[col * n + k];
        }
    }
    return true;
}

// Solve using factors from luFactor. b is not modified; x receives the solution.
inline void luSolve(int n, const double* LU, const int* piv, const double* b, double* x)
{
    for (int i = 0; i < n; ++i) x[i] = b[i];
    for (int col = 0; col < n; ++col) {
        const int p = piv[col];
        if (p != col) std::swap(x[col], x[p]);
        for (int r = col + 1; r < n; ++r) x[r] -= LU[r * n + col] * x[col];
    }
    for (int r = n - 1; r >= 0; --r) {
        double v = x[r];
        for (int k = r + 1; k < n; ++k) v -= LU[r * n + k] * x[k];
        x[r] = v / LU[r * n + r];
    }
}

} // namespace ohq

#endif // OHQ_LINALG_H
)OHQRT" },
    { "ohq_massbalance.h",
R"OHQRT(/*
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
    // outflow, mirroring System::S)OHQRT"
R"OHQRT(etLimitedOutFlow.
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
)OHQRT" },
    { "ohq_solver.h",
R"OHQRT(/*
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
    // Interpreter Solve-loop floors: the APPLIED step is >= dt0/timestepminfactor
    // and dt_base after a shrink is >= minimum_timestep (System.cpp ~1142/1594).
    double dt_floor_factor    = 1e-5;   // applied dt floor = dt0 * dt_floor_factor  (1/timestepminfactor)
    double dt_abs_min         = 1e-6;   // minimum_timestep
    double nr_coefficient     = 1.0;    // Newton damping (line-search scale)
    int    max_step_failures  = 20;
    // Lagged ("chord") Jacobian, matching System.cpp: assemble once, reuse the
    // factors for subsequent iterations, refresh only when the step stops
    // improving (updatejacobian=false at System.cpp:2440, restored in
    // AdjustNRCoefficient). Cheaper per iteration but more iterations; the
    // iteration count then feeds dt adaptation exactly as in the interpreter.
    bool   lag_jacobian       = false;
    // ---- interpreter Newton parity (System::OneStepSolve + ComputeNewtonStep +
    // AdjustNRCoefficient): the interpreter's iterate path instead of a fresh
    // line search per iteration -- Jacobian assembled on demand with factors
    // PERSISTING ACROSS STEPS, damping coefficient adapted by its rules.
    //
    // ON by default: with it, codegen reproduces the interpreter's per-iteration
    // residual sequence to 8 significant digits, and the breakthrough curves at
    // the model's own settings go from +10.6% to -1.0% of the interpreter's.
    // It is also ~10x faster than the undamped path, because the chord Jacobian
    // is genuinely reused instead of being rebuilt every iteration.
    bool   interpreter_newton = true;
    bool   optimize_lambda    = true;   // System.h:86 (not exposed in settings.json)
    double nr_coeff_reduction = 0.8;    // NR_coeff_reduction_factor, System.h:69
    bool   update_jacobian_every_iteration = false;
    int    jac_refresh_every  = 50;     // System.cpp:1138, counter % 50
    // ---- oscillation control (System.h:112-140, System.cpp:1531-1610) --------
    // A step can satisfy the Newton tolerance and still be wrong: with a step
    // coarse relative to the fastest reaction the scheme oscillates and the run
    // finishes non-monotone with no error at all. Off by default here as in the
    // interpreter -- genuinely oscillatory problems exist.
    bool   oscillation_control   = false;
    double oscillation_tolerance = 0.01;   // reversal size, relative to the variable's scale
    bool   oscillation_rewind    = true;   // rewind to the restore point vs. just slow down
    int    oscillation_relax_after = 40;   // clean steps before the ceiling doubles; 0 = never
    int    oscillation_max_reductions = 4; // budget, refunded when the ceiling relaxes
    unsigned restore_interval       = 200; // save a restore point every N steps
    unsigned restore_point_max_uses = 2;   // rewinds allowed per saved point
    // Evaluate time-dependent expressions at the step's START time, as the
    // interpreter does (System.cpp:1494 advances t only after OneStepSolve).
    bool   forcing_at_step_start = true;
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
)OHQRT" },
    { "ohq_sparse.h",
R"OHQRT(/*
 * OpenHydroQual - Codegen runtime: compact sparse linear solver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Header-only, dependency-free CSR sparse matrix with an ILU(0)-preconditioned
 * BiCGSTAB solver. Used by the mass-balance/transport Newton steps to replace
 * dense O(n^3) elimination on large, sparse network/grid models. Backward-Euler
 * puts 1/dt on the diagonal (strongly diagonally dominant for the clamped dt),
 * so ILU(0)+BiCGSTAB converges in a few iterations; a dense fallback covers the
 * rare non-convergent case.
 */
#ifndef OHQ_SPARSE_H
#define OHQ_SPARSE_H

#include <vector>
#include <cmath>
#include "ohq_linalg.h"   // dense fallback

namespace ohq {

struct SparseCSR {
    int n = 0;
    std::vector<int> rowptr;      // size n+1
    std::vector<int> colidx;      // size nnz
    std::vector<double> val;      // size nnz
    std::vector<int> diag;        // index into val/colidx of the diagonal per row

    // Build the structure from per-row sorted column lists (must include the
    // diagonal). Values are zeroed; call setZero()+adder to fill.
    void build(const std::vector<std::vector<int>>& cols)
    {
        n = static_cast<int>(cols.size());
        rowptr.assign(n + 1, 0);
        for (int i = 0; i < n; ++i) rowptr[i + 1] = rowptr[i] + (int)cols[i].size();
        const int nnz = rowptr[n];
        colidx.resize(nnz); val.assign(nnz, 0.0); diag.assign(n, -1);
        for (int i = 0; i < n; ++i) {
            int p = rowptr[i];
            for (int c : cols[i]) { colidx[p] = c; if (c == i) diag[i] = p; ++p; }
        }
    }
    void setZero() { std::fill(val.begin(), val.end(), 0.0); }
    // Add into entry (i,j); returns false if (i,j) not in the pattern.
    bool add(int i, int j, double v)
    {
        for (int p = rowptr[i]; p < rowptr[i + 1]; ++p)
            if (colidx[p] == j) { val[p] += v; return true; }
        return false;
    }
    void mv(const double* x, double* y) const
    {
        for (int i = 0; i < n; ++i) { double s = 0; for (int p = rowptr[i]; p < rowptr[i+1]; ++p) s += val[p]*x[colidx[p]]; y[i] = s; }
    }
};

// ILU(0): factorization sharing A's pattern. luVal is a copy of A.val, overwritten.
inline bool ilu0(const SparseCSR& A, std::vector<double>& luVal)
{
    luVal = A.val;
    const int n = A.n;
    std::vector<int> firstUpper(n);
    for (int i = 0; i < n; ++i) {
        for (int p = A.rowptr[i]; p < A.rowptr[i + 1]; ++p) {
            int k = A.colidx[p];
            if (k >= i) break;                        // only strict-lower part
            const double diagk = luVal[A.diag[k]];
            if (std::fabs(diagk) < 1e-300) return false;
            const double mult = luVal[p] / diagk;
            luVal[p] = mult;
            // subtract mult * U(k, j) from A(i, j) for j>k in row i that also in row k
            for (int q = A.diag[k] + 1; q < A.rowptr[k + 1]; ++q) {
                int j = A.colidx[q];
                // find (i,j)
                for (int r = p + 1; r < A.rowptr[i + 1]; ++r)
                    if (A.colidx[r] == j) { luVal[r] -= mult * luVal[q]; break; }
            }
        }
        if (std::fabs(luVal[A.diag[i]]) < 1e-300) return false;
    }
    (void)firstUpper;
    return true;
}

// Apply M^{-1} r -> z using the ILU(0) factors in luVal (unit lower L, upper U).
inline void iluApply(const SparseCSR& A, const std::vector<double>& luVal,
                     const double* r, double* z)
{
    const int n = A.n;
    // forward solve L y = r  (unit diagonal)
    for (int i = 0; i < n; ++i) {
        double s = r[i];
        for (int p = A.rowptr[i]; p < A.diag[i]; ++p) s -= luVal[p] * z[A.colidx[p]];
        z[i] = s;
    }
    // back solve U z = y
    for (int i = n - 1; i >= 0; --i) {
        double s = z[i];
        for (int p = A.diag[i] + 1; p < A.rowptr[i + 1]; ++p) s -= luVal[p] * z[A.colidx[p]];
        z[i] = s / luVal[A.diag[i]];
    }
}

// Solve A x = b with ILU(0)-preconditioned BiCGSTAB. Returns true on convergence.
inline bool bicgstab(const SparseCSR& A, const double* b, double* x,
                     int maxit = 500, double tol = 1e-10)
{
    const int n = A.n;
    std::vector<double> luVal;
    const bool haveM = ilu0(A, luVal);
    std::vector<double> r(n), rhat(n), p(n, 0), v(n, 0), s(n), t(n), ph(n), sh(n), y(n);
    A.mv(x, r.data());
    double bnorm = 0;
    for (int i = 0; i < n; ++i) { r[i] = b[i] - r[i]; rhat[i] = r[i]; bnorm += b[i]*b[i]; }
    bnorm = std::sqrt(bnorm); if (bnorm < 1e-300) bnorm = 1.0;
    double rho = 1, alpha = 1, omega = 1, rho_prev = 1;
    auto dot = [&](const std::vector<double>& a, const std::vector<double>& c){ double d=0; for (int i=0;i<n;++i) d+=a[i]*c[i]; return d; };
    for (int it = 0; it < maxit; ++it) {
        double rn = 0; for (int i=0;i<n;++i) rn += r[i]*r[i]; rn = std::sqrt(rn);
        if (rn / bnorm < tol) return true;
        rho = dot(rhat, r);
        if (std::fabs(rho) < 1e-300) return false;
        const double beta = (rho / rho_prev) * (alpha / omega);
        for (int i = 0; i < n; ++i) p[i] = r[i] + beta * (p[i] - omega * v[i]);
        if (haveM) iluApply(A, luVal, p.data(), ph.data()); else ph = p;
        A.mv(ph.data(), v.data());
        alpha = rho / dot(rhat, v);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];
        if (haveM) iluApply(A, luVal, s.data(), sh.data()); else sh = s;
        A.mv(sh.data(), t.data());
        double tt = dot(t, t); omega = tt > 1e-300 ? dot(t, s) / tt : 0.0;
        for (int i = 0; i < n; ++i) { x[i] += alpha * ph[i] + omega * sh[i]; r[i] = s[i] - omega * t[i]; }
        rho_prev = rho;
        if (std::fabs(omega) < 1e-300) return false;
    }
    return false;
}

} // namespace ohq

#endif // OHQ_SPARSE_H
)OHQRT" },
    { "ohq_timeseries.h",
R"OHQRT(/*
 * OpenHydroQual - Codegen runtime: time series + forecast kernels
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * A minimal, dependency-free time series (only STL) providing:
 *   - interpol(t)      : linear interpolation with flat extrapolation
 *   - ekr(t, lambda)   : forward exponential kernel  (interpreter _ekr)
 *   - gkr(t, mu, sigma): Gaussian kernel             (interpreter _gkr)
 *
 * The kernels reproduce aquifolium/src/TimeSeries.hpp
 * Exponential_Kernel / Gaussian_Kernel so a generated forecast matches the
 * interpreter bit-for-bit (same discretization).
 */
#ifndef OHQ_TIMESERIES_H
#define OHQ_TIMESERIES_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace ohq {

class TimeSeries {
public:
    std::vector<double> t;   // ascending sample times
    std::vector<double> c;   // sample values

    TimeSeries() = default;

    std::size_t size() const { return t.size(); }
    bool empty() const { return t.empty(); }

    void push(double ti, double ci) { t.push_back(ti); c.push_back(ci); }

    // ---- interpreter parity: TimeSeries::assign_D / interpol_D --------------
    // d[i] = time from sample i until the series next CHANGES value (at least one
    // sample spacing). System::GetMinimumNextTimeStepSize takes the minimum of
    // interpol_D over the precipitation series so dt is only clamped where the
    // forcing actually changes (dry spells are stepped over in big steps).
    mutable std::vector<double> d;
    void assignD() const
    {
        const size_t n = t.size();
        d.assign(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            double counter = 0.0;
            for (size_t j = i + 1; j < n; ++j) {
                counter += t[j] - t[j - 1];
                if (c[j] != c[i]) break;
            }
            if (i + 1 == n && n > 1) counter = t[n - 1] - t[n - 2];
            else if (n == 1)         counter = 100.0;
            if (counter == 0.0)      counter = (i > 0) ? t[i] - t[i - 1] : t[0];
            d[i] = std::fabs(counter);
        }
    }
    double interpolD(double x) const
    {
        const int n = static_cast<int>(t.size());
        if (n == 0) return 0.0;
        if (d.size() != t.size()) assignD();          // lazily, also after a setter replaced the data
        if (x <= t.front()) return d.front();
        if (x >= t.back())  return d.back();
        int i = idxAt(x);
        if (i >= n - 1) return d.back();
        const double dt = t[i + 1] - t[i];
        const double interp = d[i] + (d[i + 1] - d[i]) * (x - t[i]) / dt;
        return std::max(interp, dt);
    }

    // ---- interpreter parity: TimeSeries<double>::make_uniform ---------------
    // (aquifolium/src/TimeSeries.hpp:1533 -- the live class behind
    // TimeSeriesSet<outputtimeseriesprecision>; the CTimeSeries in BTC.hpp is
    // the legacy one and walks the grid differently.)
    // System::FinalizeOutputs resamples every recorded series onto a grid of
    // step `increment` anchored at t[0] before writing, so an output row is an
    // interpolation between two accepted steps, never a step itself. The step
    // stays adaptive; only the OUTPUT is uniform. Reproduced point for point:
    // non-finite samples dropped first, the grid advanced by repeated addition
    // (so the same rounding walk), the bracket advanced while t[i+1] < t_grid
    // but never past last-1, and every grid point emitted unconditionally.
    TimeSeries makeUniform(double increment) const
    {
        TimeSeries out;
        TimeSeries src;
        for (std::size_t k = 0; k < t.size() && k < c.size(); ++k)
            if (std::isfinite(t[k]) && std::isfinite(c[k])) src.push(t[k], c[k]);
        if (src.size() < 2) return out;
        if (!(increment > 0.0)) return out;

        const std::size_t last = src.size() - 1;
        std::size_t i = 0;
        const double t_end = src.t[last];
        for (double cur = src.t[0]; cur <= t_end; cur += increment) {
            while (i + 1 < last && src.t[i + 1] < cur) ++i;
            const double dt = src.t[i + 1] - src.t[i];
            const double ratio = (dt == 0.0) ? 0.5 : (cur - src.t[i]) / dt;
            out.push(cur, src.c[i] + ratio * (src.c[i + 1] - src.c[i]));
        }
        return out;
    }

    // Index of the last sample with t[i] <= x (clamped into [0, size-1]).
    // Mirrors CTimeSeries::GetElementNumberAt semantics closely enough for the
    // kernels, which only use it to bound the integration window.
    int idxAt(double x) const
    {
        const int n = static_cast<int>(t.size());
        if (n == 0) return 0;
        if (x <= t.front()) return 0;
        if (x >= t.back())  return n - 1;
        // largest i with t[i] <= x
        int lo = 0, hi = n - 1;
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            if (t[mid] <= x) lo = mid; else hi = mid - 1;
        }
        return lo;
    }

    // Linear interpolation with flat extrapolation outside the range.
    double interpol(double x) const
    {
        const int n = static_cast<int>(t.size());
        if (n == 0) return 0.0;
        if (x <= t.front()) return c.front();
        if (x >= t.back())  return c.back();
        int i = idxAt(x);
        if (i >= n - 1) return c.back();
        const double dt = t[i + 1] - t[i];
        if (dt == 0.0) return c[i];
        const double w = (x - t[i]) / dt;
        return c[i] * (1.0 - w) + c[i + 1] * w;
    }

    // Forward exponential kernel: reproduces TimeSeries.hpp Exponential_Kernel.
    // forecast(t) ~ integral_t^inf c(tau) * lambda * exp(-lambda (tau - t)) dtau
    double ekr(double time, double lambda) const
    {
        if (t.empty()) return 0.0;
        const int n = static_cast<int>(t.size());
        int initial_i = idxAt(time);
        int last_i = std::min(idxAt(time + 2.0 / lambda), n - 1);
        double sum = 0.0;
        for (int i = initial_i; i < last_i; ++i) {
            const double t_i   = t[i];
            const double t_ip1 = t[i + 1];
            const double c_i   = c[i];
            const double delta = t_ip1 - t_i;
            sum += c_i * lambda * std::exp(-lambda * (t_i   - time)) * delta;
            sum += c_i * lambda * std::exp(-lambda * (t_ip1 - time)) * delta;
        }
        return sum;
    }

    // Gaussian kernel: reproduces TimeSeries.hpp Gaussian_Kernel.
    double gkr(double time, double mu, double stdev) const
    {
        const int n = static_cast<int>(t.size());
        if (n < 2) return 0.0;
        const double sqrt_2pi = std::sqrt(2.0 * 3.14159265358979323846);
        const double var = stdev * stdev;
        int initial_i = std::max(idxAt(time - 2.0 * stdev + mu), 0);
        int last_i    = std::min(idxAt(time + 2.0 * stdev + mu), n - 2);
        double sum = 0.0;
        for (int i = initial_i; i <= last_i; ++i) {
            const double t1 = t[i],   t2 = t[i + 1];
            const double c1 = c[i],   c2 = c[i + 1];
            const double delta = t2 - t1;
            const double w1 = std::exp(-std::pow(t1 - time - mu, 2) / (2 * var)) / (sqrt_2pi * stdev);
            const double w2 = std::exp(-std::pow(t2 - time - mu, 2) / (2 * var)) / (sqrt_2pi * stdev);
            sum += 0.5 * (c1 * w1 + c2 * w2) * delta;
        }
        return sum;
    }

    // Load a 2- or 3-column CSV: "t,value" or "t_start,t_end,value"
    // (3-column uses the interval midpoint as the sample time, matching the
    // precipitation file convention).
    bool loadCSV(const std::string& path)
    {
        std::ifstream f(path);
        if (!f) return false;
        t.clear(); c.clear();
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::vector<double> cols;
            std::string cell;
            while (std::getline(ss, cell, ',')) {
                try { cols.push_back(std::stod(cell)); } catch (...) { cols.clear(); break; }
            }
            if (cols.size() == 2)      push(cols[0], cols[1]);
            else if (cols.size() >= 3) push(0.5 * (cols[0] + cols[1]), cols[2]);
        }
        return !t.empty();
    }
};

} // namespace ohq

#endif // OHQ_TIMESERIES_H
)OHQRT" },
    { "ohq_transport.h",
R"OHQRT(/*
 * OpenHydroQual - Codegen runtime: constituent transport solver (advection)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * The second solve phase. After the flow phase fixes storages and link flows,
 * this advances constituent masses over the same step, reproducing
 * System::GetResiduals_TR (advective mass balance; reactions come later):
 *
 *   F[b*nC+j] = (mass - mass_past)/dt - inflowOwn[b,j]
 *   per link l, constituent j: F[src]+= masstransfer[l,j]; F[dst]-= masstransfer[l,j]
 *
 * The model supplies the flow-phase results (cached) and the compiled transport
 * expressions via computeTransportFluxes(). No outflow limiting here.
 *
 * Required Model interface:
 *   int  nMass() const;            // nBlocks * nConst
 *   int  nConst() const;
 *   int  nLinksT() const;
 *   int  linkSrcT(int l) const;    // source block index
 *   int  linkDstT(int l) const;    // destination block index
 *   void initialMass(double* m);
 *   void computeTransportFluxes(const double* mass, double t,
 *                               double* massTransfer, double* inflowOwn) const;
 *        // massTransfer[nLinks*nConst], inflowOwn[nBlocks*nConst]
 */
#ifndef OHQ_TRANSPORT_H
#define OHQ_TRANSPORT_H

#include <vector>
#include <cmath>
#include "ohq_linalg.h"
#include "ohq_solver.h"   // SolverSettings

namespace ohq {

template <class Model>
class TransportSolver {
public:
    explicit TransportSolver(Model& m, SolverSettings s = SolverSettings{})
        : m_(m), s_(s) {}

    SolverSettings& settings() { return s_; }

    void initialize()
    {
        n_  = m_.nMass();
        nc_ = m_.nConst();
        nl_ = m_.nLinksT();
        mass_.assign(n_, 0.0); past_.assign(n_, 0.0);
        F_.assign(n_, 0.0);
        mt_.assign(nl_ * nc_ > 0 ? nl_ * nc_ : 1, 0.0);
        inflow_.assign(n_, 0.0);
        m_.initialMass(mass_.data());
    }

    double mass(int i) const { return mass_[i]; }
    // G5: restart the constituent masses from state values
    void setMass(const double* m) { for (int i = 0; i < n_; ++i) { mass_[i] = m[i]; past_[i] = m[i]; } }

    // Advance one step of size dt at time t, using the model's cached flow-phase
    // storages/flows.
    // force a Jacobian refresh (System::SetUpdateJacobian(true))
    void requestJacobianUpdate() { updateJac_ = true; }
    int lastIterations() const { return last_iters_; }
    bool step(double t, double dt)
    {
        t_ = t; dt_ = dt;
        for (int i = 0; i < n_; ++i) past_[i] = mass_[i];
        return newton();
    }

private:
    void assemble(const double* X, double* F)
    {
        m_.computeTransportFluxes(X, t_, mt_.data(), inflow_.data());
        for (int i = 0; i < n_; ++i) F[i] = (X[i] - past_[i]) / dt_ - inflow_[i];
        for (int l = 0; l < nl_; ++l) {
            const int s = m_.linkSrcT(l), e = m_.linkDstT(l);
            for (int j = 0; j < nc_; ++j) {
                const double q = mt_[l * nc_ + j];
                F[s * nc_ + j] += q;
                F[e * nc_ + j] -= q;
            }
        }
    }

    // FD Jacobian of the transport residual, factorised for reuse across
    // iterations AND steps (SolverTempVars.updatejacobian persists likewise).
    bool assembleJacobian(const std::vector<double>& F0)
    {
        Jfac_.assign(n_ * n_, 0.0);
        std::vector<double> Fp(n_);
        for (int jc = 0; jc < n_; ++jc) {
            double eps = -1e-6 * (std::fabs(mass_[jc]) + 1.0);
            const double save = mass_[jc]; mass_[jc] += eps;
            assemble(mass_.data(), Fp.data());
            for (int i = 0; i < n_; ++i) Jfac_[i * n_ + jc] = (Fp[i] - F0[i]) / eps;
            mass_[jc] = save;
            // System::Jacobian (System.cpp:3269): a column that comes back
            // non-finite, or whose diagonal is exactly zero, is recomputed with
            // the opposite perturbation sign.
            bool redo = (Jfac_[jc * n_ + jc] == 0.0);
            for (int i = 0; i < n_ && !redo; ++i) if (!std::isfinite(Jfac_[i * n_ + jc])) redo = true;
            if (redo) {
                eps = +1e-6 * (std::fabs(mass_[jc]) + 1.0);
                mass_[jc] += eps;
                assemble(mass_.data(), Fp.data());
                for (int i = 0; i < n_; ++i) Jfac_[i * n_ + jc] = (Fp[i] - F0[i]) / eps;
                mass_[jc] = save;
            }
        }
        piv_.assign(n_, 0);
        return luFactor(n_, Jfac_.data(), piv_.data());
    }

    bool newton()
    {
        if (s_.interpreter_newton) return newtonInterp();
        return newtonLineSearch();
    }

    // Same port as MassBalanceSolver::newtonInterp -- see the comment there. The
    // interpreter runs this identical loop for the constituent state variable,
    // only with a different residual and Jacobian.
    bool newtonInterp()
    {
        if (s_.jac_refresh_every > 0 && (stepCounter_ + 1) % s_.jac_refresh_every == 0)
            updateJac_ = true;                       // System.cpp:1136-1138 (1-based)
        ++stepCounter_;
        const double X_norm = norm(mass_);
        double dx_norm = X_norm * 10 + 1;
        // mass_ = the state the model actually holds (the FULL damped step, what
        // the step commits); Xit = the local iterate AdjustNRCoefficient may move
        // to the half-step. See the comment in MassBalanceSolver::newtonInterp --
        // this phase exits after one iteration via the dx_norm escape, so
        // committing the half-step here halves every accepted increment.
        std::vector<double> Xit = mass_, X_past = mass_;
        assemble(mass_.data(), F_.data());
        double err_ini = norm(F_);
        double err = err_ini, err_p = err_ini;
        double error_increase_counter = 0;
        nrCoeff_ = 1.0;
        last_iters_ = 0;
        if (X_norm <= 0.0) return true;
        std::vector<double> dx(n_), X1(n_), F1(n_);
        while (err / (err_ini + 1e-8 * X_norm) > s_.tolerance && err > 1e-12
               && dx_norm / X_norm > 1e-10)
        {
            ++last_iters_;
            if (s_.update_jacobian_every_iteration) updateJac_ = true;
            if (updateJac_) {
                std::vector<double> F0(F_);
                if (!assembleJacobian(F0)) return false;
                updateJac_ = false;
            }
            luSolve(n_, Jfac_.data(), piv_.data(), F_.data(), dx.data());
            for (int i = 0; i < n_; ++i) dx[i] *= nrCoeff_;
            dx_norm = norm(dx);
            for (int i = 0; i < n_; ++i) mass_[i] = Xit[i] - dx[i];   // full step
            if (s_.optimize_lambda) {
                for (int i = 0; i < n_; ++i) X1[i] = mass_[i] + 0.5 * dx[i];
                assemble(X1.data(), F1.data());
            }
            assemble(mass_.data(), F_.data());
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
                    Xit = mass_;
                }
                if (std::min(err2, err) > err_p) error_increase_counter++;
            } else {
                if (err > err_p * 0.9) {
                    nrCoeff_ = std::max(nrCoeff_ * s_.nr_coeff_reduction, 0.05);
                    updateJac_ = true;
                    Xit = X_past;
                } else {
                    Xit = mass_;
                }
                if (err > err_p) error_increase_counter++;
                else if (err < err_p / 2.0) {
                    if (nrCoeff_ < 0.99) updateJac_ = true;
                    nrCoeff_ = std::max(std::min(nrCoeff_ / s_.nr_coeff_reduction, 1.0), 0.05);
                }
            }
            if (error_increase_counter > 10) return false;
            if (last_iters_ > s_.max_iterations)  return false;
        }
        return true;
    }

    bool newtonLineSearch()
    {
        assemble(mass_.data(), F_.data());
        double err = norm(F_), err_ini = err, xnorm = norm(mass_) + 1e-30;
        last_iters_ = 0;
        if (err < s_.abs_floor) return true;
        std::vector<double> J(n_ * n_), Jc(n_ * n_), Fc(n_), dx(n_), Xtry(n_), F0(n_);
        std::vector<int> piv(n_);
        bool needJac = true;              // lagged mode: assemble on demand
        for (int it = 1; it <= s_.max_iterations; ++it) {
            assemble(mass_.data(), F0.data());
            if (!s_.lag_jacobian || needJac) {
                for (int jc = 0; jc < n_; ++jc) {
                    const double eps = -1e-6 * (std::fabs(mass_[jc]) + 1.0);
                    const double save = mass_[jc]; mass_[jc] += eps;
                    assemble(mass_.data(), F_.data());
                    for (int i = 0; i < n_; ++i) J[i * n_ + jc] = (F_[i] - F0[i]) / eps;
                    mass_[jc] = save;
                }
                if (s_.lag_jacobian) {
                    Jc = J;
                    if (!luFactor(n_, Jc.data(), piv.data())) return false;
                    needJac = false;
                }
            }
            if (s_.lag_jacobian) {
                luSolve(n_, Jc.data(), piv.data(), F0.data(), dx.data());
            } else {
                Jc = J; Fc = F0;
                if (!solveInPlace(n_, Jc.data(), Fc.data(), dx.data())) return false;
            }
            double lambda = s_.nr_coefficient, err_try = err;
            int ls_used = 0;
            for (int ls = 0; ls < 12; ++ls) {
                ls_used = ls;
                for (int i = 0; i < n_; ++i) Xtry[i] = mass_[i] - lambda * dx[i];
                assemble(Xtry.data(), F_.data());
                err_try = norm(F_);
                if (err_try < err || err_try < s_.abs_floor) break;
                lambda *= 0.5;
            }
            // A damped step means the stored factors no longer describe the local
            // slope well; refresh, as AdjustNRCoefficient does (System.cpp:6468).
            if (s_.lag_jacobian && (ls_used > 0 || err_try > err * 0.9)) needJac = true;
            mass_ = Xtry;
            double dxn = 0; for (int i = 0; i < n_; ++i) dxn += (lambda*dx[i])*(lambda*dx[i]);
            dxn = std::sqrt(dxn); xnorm = norm(mass_) + 1e-30; err = err_try;
            last_iters_ = it;
            if (err/(err_ini+1e-8*xnorm) < s_.tolerance || err < s_.abs_floor || dxn/xnorm < 1e-10)
                return true;
        }
        return false;
    }

    static double norm(const std::vector<double>& v)
    { double s = 0; for (double x : v) s += x*x; return std::sqrt(s); }

    Model& m_;
    SolverSettings s_;
    int last_iters_ = 0;
    int n_ = 0, nc_ = 0, nl_ = 0;
    double t_ = 0, dt_ = 0;
    std::vector<double> mass_, past_, F_, mt_, inflow_;
    // interpreter-parity Newton state (persists across steps, as in System.cpp)
    std::vector<double> Jfac_;
    std::vector<int> piv_;
    bool updateJac_ = true;
    double nrCoeff_ = 1.0;
    long stepCounter_ = 0;
};

} // namespace ohq

#endif // OHQ_TRANSPORT_H
)OHQRT" }
};

const RuntimeFile* runtimeFiles(int& count)
{
    count = static_cast<int>(sizeof(kRuntimeFiles) / sizeof(kRuntimeFiles[0]));
    return kRuntimeFiles;
}

} // namespace ohqcg
