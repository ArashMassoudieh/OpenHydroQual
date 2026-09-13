/*
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

} // namespace ohq

#endif // OHQ_LINALG_H
