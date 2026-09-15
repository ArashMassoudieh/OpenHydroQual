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
