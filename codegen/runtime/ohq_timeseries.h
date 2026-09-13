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
