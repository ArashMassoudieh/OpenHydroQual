/*
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
