/*
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
        : m_(&m), s_(s) {}
    // re-point at the owning model (after a copy); see the m_ comment
    void rebind(Model& m) { m_ = &m; }
    const Model* model() const { return m_; }

    SolverSettings& settings() { return s_; }

    void initialize()
    {
        n_  = m_->nMass();
        nc_ = m_->nConst();
        nl_ = m_->nLinksT();
        mass_.assign(n_, 0.0); past_.assign(n_, 0.0);
        F_.assign(n_, 0.0);
        mt_.assign(nl_ * nc_ > 0 ? nl_ * nc_ : 1, 0.0);
        inflow_.assign(n_, 0.0);
        Q_.assign(n_, 0.0);
        m_->initialMass(mass_.data());
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
        m_->computeTransportFluxes(X, t_, mt_.data(), inflow_.data());
        // Q_ = each (block, constituent)'s own throughput, for the per-block
        // convergence test (System::GetResiduals_TR, block_flux_scale)
        for (int i = 0; i < n_; ++i) {
            const double storageRate = (X[i] - past_[i]) / dt_;
            F[i] = storageRate - inflow_[i];
            Q_[i] = std::fabs(storageRate) + std::fabs(inflow_[i]);
        }
        for (int l = 0; l < nl_; ++l) {
            const int s = m_->linkSrcT(l), e = m_->linkDstT(l);
            for (int j = 0; j < nc_; ++j) {
                const double q = mt_[l * nc_ + j];
                F[s * nc_ + j] += q;
                F[e * nc_ + j] -= q;
                Q_[s * nc_ + j] += std::fabs(q);
                Q_[e * nc_ + j] += std::fabs(q);
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
        // System.cpp OneStepSolve -- the stored Jacobian holds 1/dt on its diagonal:
        // refresh it when the applied dt is more than jac_dt_refresh_factor away
        // from the dt it was assembled with
        if (s_.jac_dt_refresh_factor > 1.0 && jacDt_ > 0
            && std::fabs(std::log(dt_ / jacDt_)) > std::log(s_.jac_dt_refresh_factor))
            updateJac_ = true;
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
        // NOTE: do NOT skip the solve when X_norm == 0. The interpreter has no
        // such guard (System.cpp:2431): with X_norm == 0 the loop's
        // dx_norm/X_norm is 1/0 = +inf > 1e-10, so it iterates normally. A state
        // that legitimately starts at zero -- an age tracer, or a dry catchment
        // at t=0 -- would otherwise never be solved at all, and since it then
        // stays zero the guard latches for the whole run.
        std::vector<double> dx(n_), X1(n_), F1(n_);
        // Per-block test, as MassBalanceSolver::newtonInterp, per (block,
        // constituent) with the floor taken per constituent (System.cpp
        // OneStepSolve, blocks_balanced).
        auto blocksBalanced = [&]() -> bool {
            if (s_.block_tolerance <= 0) return true;
            const int ng = nc_ > 0 ? nc_ : 1;
            std::vector<double> Q_max(ng, 0.0);
            for (int i = 0; i < n_; ++i) Q_max[i % ng] = std::max(Q_max[i % ng], Q_[i]);
            for (int i = 0; i < n_; ++i) {
                if (std::fabs(F_[i]) <= s_.block_tolerance * (Q_[i] + 1e-3 * Q_max[i % ng]) + 1e-12) continue;
                if (last_iters_ > 0 && std::fabs(dx[i]) <= 1e-10 * std::fabs(Xit[i])) continue;
                return false;
            }
            return true;
        };
        while (err > 1e-12
               && ((err / (err_ini + 1e-8 * X_norm) > s_.tolerance && dx_norm / X_norm > 1e-10)
                   || !blocksBalanced()))
        {
            ++last_iters_;
            if (s_.update_jacobian_every_iteration) updateJac_ = true;
            if (updateJac_) {
                std::vector<double> F0(F_);
                if (!assembleJacobian(F0)) return false;
                updateJac_ = false;
                jacDt_ = dt_;
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

    // A POINTER, not a reference: the generated kernel is copied per MCMC
    // chain, and a copy must re-bind this to ITSELF (rebind()). A reference
    // also deletes copy-assignment, which the hosts need.
    Model* m_;
    SolverSettings s_;
    int last_iters_ = 0;
    int n_ = 0, nc_ = 0, nl_ = 0;
    double t_ = 0, dt_ = 0;
    std::vector<double> mass_, past_, F_, mt_, inflow_;
    std::vector<double> Q_;   // per-entry throughput, see assemble()
    // interpreter-parity Newton state (persists across steps, as in System.cpp)
    std::vector<double> Jfac_;
    std::vector<int> piv_;
    bool updateJac_ = true;
    double nrCoeff_ = 1.0;
    long stepCounter_ = 0;
    double jacDt_ = 0;   // dt the stored Jacobian was assembled with (0 = none)
};

} // namespace ohq

#endif // OHQ_TRANSPORT_H
