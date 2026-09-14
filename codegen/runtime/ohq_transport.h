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

    bool newton()
    {
        assemble(mass_.data(), F_.data());
        double err = norm(F_), err_ini = err, xnorm = norm(mass_) + 1e-30;
        if (err < s_.abs_floor) return true;
        std::vector<double> J(n_ * n_), Jc(n_ * n_), Fc(n_), dx(n_), Xtry(n_), F0(n_);
        for (int it = 1; it <= s_.max_iterations; ++it) {
            assemble(mass_.data(), F0.data());
            for (int jc = 0; jc < n_; ++jc) {
                const double eps = -1e-6 * (std::fabs(mass_[jc]) + 1.0);
                const double save = mass_[jc]; mass_[jc] += eps;
                assemble(mass_.data(), F_.data());
                for (int i = 0; i < n_; ++i) J[i * n_ + jc] = (F_[i] - F0[i]) / eps;
                mass_[jc] = save;
            }
            Jc = J; Fc = F0;
            if (!solveInPlace(n_, Jc.data(), Fc.data(), dx.data())) return false;
            double lambda = s_.nr_coefficient, err_try = err;
            for (int ls = 0; ls < 12; ++ls) {
                for (int i = 0; i < n_; ++i) Xtry[i] = mass_[i] - lambda * dx[i];
                assemble(Xtry.data(), F_.data());
                err_try = norm(F_);
                if (err_try < err || err_try < s_.abs_floor) break;
                lambda *= 0.5;
            }
            mass_ = Xtry;
            double dxn = 0; for (int i = 0; i < n_; ++i) dxn += (lambda*dx[i])*(lambda*dx[i]);
            dxn = std::sqrt(dxn); xnorm = norm(mass_) + 1e-30; err = err_try;
            if (err/(err_ini+1e-8*xnorm) < s_.tolerance || err < s_.abs_floor || dxn/xnorm < 1e-10)
                return true;
        }
        return false;
    }

    static double norm(const std::vector<double>& v)
    { double s = 0; for (double x : v) s += x*x; return std::sqrt(s); }

    Model& m_;
    SolverSettings s_;
    int n_ = 0, nc_ = 0, nl_ = 0;
    double t_ = 0, dt_ = 0;
    std::vector<double> mass_, past_, F_, mt_, inflow_;
};

} // namespace ohq

#endif // OHQ_TRANSPORT_H
