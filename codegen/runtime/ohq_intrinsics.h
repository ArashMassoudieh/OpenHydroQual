/*
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
inline double powr(double a, double b){ return std::pow(a, b); }

} // namespace ohq

#endif // OHQ_INTRINSICS_H
