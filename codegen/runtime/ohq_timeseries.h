/*
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
    // d[i] = time from sample i to the LAST sample of the constant run that
    // starts at i (one sample spacing where the value changes right away), so a
    // step from inside a dry spell stops on its last dry sample rather than on
    // the first wet one. interpolD interpolates d, floored at the local spacing.
    // The solver clamps dt to the minimum over every forcing series, as does
    // System::GetMinimumNextTimeStepSize.
    mutable std::vector<double> d;
    void assignD() const
    {
        const size_t n = t.size();
        d.assign(n, 0.0);
        if (n == 0) return;
        std::vector<size_t> runEnd(n, n - 1);   // last sample of i's constant run
        for (size_t i = n - 1; i-- > 0; )
            runEnd[i] = (c[i + 1] == c[i]) ? runEnd[i + 1] : i;
        for (size_t i = 0; i < n; ++i) {
            double counter = 0.0;
            if (n == 1)          counter = 100.0;
            else if (i + 1 == n) counter = t[n - 1] - t[n - 2];
            else counter = (runEnd[i] > i) ? t[runEnd[i]] - t[i] : t[i + 1] - t[i];
            if (counter == 0.0)  counter = (i > 0) ? t[i] - t[i - 1] : t[0];
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
