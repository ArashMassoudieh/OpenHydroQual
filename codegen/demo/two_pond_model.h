/*
 * OpenHydroQual - Codegen demo / target model (HAND-WRITTEN)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * This is what the code generator should EMIT for a small model:
 *   pond0  --link-->  pond1  --outlet-->
 * with an inflow source on pond0. It is hand-written so we can (a) validate the
 * runtime and (b) have a concrete, benchmarkable compilation target.
 *
 * Illustrated codegen principles:
 *   - state (balance) variables live in a flat array, addressed by index enums
 *   - constants are folded into literals (AREA, K_LINK, ...)
 *   - a per-step value (inflow(t)) is computed once per step in precomputeStep()
 *   - per-iteration values (heads, link flux) are locals in residual()
 *   - the class exposes step()/runTo()/time()/value() for external callers
 */
#ifndef OHQ_TWO_POND_MODEL_H
#define OHQ_TWO_POND_MODEL_H

#include "../runtime/ohq_intrinsics.h"
#include "../runtime/ohq_timeseries.h"
#include "../runtime/ohq_solver.h"

class TwoPondModel {
public:
    // ----- generated state layout ------------------------------------------
    enum State { S0 = 0, S1 = 1, N_STATES = 2 };

    // ----- folded model constants (would be constexpr from the model file) --
    static constexpr double AREA0   = 100.0;   // m^2  pond 0 surface area
    static constexpr double AREA1   = 80.0;    // m^2  pond 1 surface area
    static constexpr double K_LINK  = 25.0;    // m^2/day  linear link conductance
    static constexpr double K_OUT   = 15.0;    // m^2/day  outlet conductance

    TwoPondModel() : solver_(*this) {}

    // ----- external API (the "compiled library" surface) -------------------
    void setInflowSeries(const ohq::TimeSeries& ts) { inflow_ = ts; }
    void setConstantInflow(double q) { inflow_.t = {0.0, 1e12}; inflow_.c = {q, q}; }

    void initialize(double tstart = 0.0, double dt0 = 0.01)
    {
        solver_.initialize(tstart, dt0);
    }
    bool step()               { return solver_.step(); }     // one adaptive step
    bool runTo(double t_end)  { return solver_.runTo(t_end); }// run up to a time
    double time() const       { return solver_.time(); }
    int    lastIterations() const { return solver_.lastIterations(); }

    // named output access
    double storage(int pond) const { return pond == 0 ? s_[S0] : s_[S1]; }
    double head(int pond) const
    {
        return pond == 0 ? s_[S0] / AREA0 : s_[S1] / AREA1;
    }

    // ----- solver callbacks (non-virtual; inlined by TransientSolver) -------
    int nStates() const { return N_STATES; }

    void initialValues()
    {
        s_[S0] = 500.0;   // initial storages
        s_[S1] = 100.0;
    }

    void getState(double* X) const { X[S0] = s_[S0]; X[S1] = s_[S1]; }
    void setState(const double* X) { s_[S0] = X[S0]; s_[S1] = X[S1]; }
    void snapshotPast() { sp_[S0] = s_[S0]; sp_[S1] = s_[S1]; }
    void setDt(double dt) { dt_ = dt; }

    // per-step (state-independent) precompute: the forcing at the new time
    void precomputeStep(double t_new)
    {
        qin_ = inflow_.interpol(t_new);
    }

    // residual: F = (S - S_past)/dt - net inflow
    void residual(const double* X, double* F) const
    {
        const double h0 = X[S0] / AREA0;
        const double h1 = X[S1] / AREA1;
        const double q01 = K_LINK * (h0 - h1);   // pond0 -> pond1
        const double qout = K_OUT * h1;          // pond1 -> outlet
        F[S0] = (X[S0] - sp_[S0]) / dt_ - (qin_ - q01);
        F[S1] = (X[S1] - sp_[S1]) / dt_ - (q01 - qout);
    }

    // analytical Jacobian (row-major 2x2): dF_i/dX_j
    void jacobian(const double* /*X*/, double* J) const
    {
        const double dq01_dS0 =  K_LINK / AREA0;
        const double dq01_dS1 = -K_LINK / AREA1;
        const double dqout_dS1 = K_OUT / AREA1;
        // F0 = (S0-S0p)/dt - qin + q01
        J[0 * 2 + 0] = 1.0 / dt_ + dq01_dS0;   // dF0/dS0
        J[0 * 2 + 1] =            dq01_dS1;     // dF0/dS1
        // F1 = (S1-S1p)/dt - q01 + qout
        J[1 * 2 + 0] = -dq01_dS0;              // dF1/dS0
        J[1 * 2 + 1] = 1.0 / dt_ - dq01_dS1 + dqout_dS1; // dF1/dS1
    }

private:
    double s_[N_STATES]  = {0, 0};   // present state
    double sp_[N_STATES] = {0, 0};   // past state
    double dt_  = 0.01;
    double qin_ = 0.0;               // per-step forcing
    ohq::TimeSeries inflow_;
    ohq::TransientSolver<TwoPondModel> solver_;
};

#endif // OHQ_TWO_POND_MODEL_H
