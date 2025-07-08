/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


// BTC.cpp: implementation of the CTimeSeries class.
//
//////////////////////////////////////////////////////////////////////



#include "TimeSeries.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>
//#include "StringOP.h"
#include "Utilities.h"
#include "NormalDist.h"
#include <chrono>
#include <ctime>
#ifdef Q_version
#include "qfile.h"
#include "qdatastream.h"
#include <qdebug.h>
#endif

#ifdef GSL
#include <gsl/gsl_cdf.h>
#endif
#include <QJsonArray>
#include <QJsonObject>


using namespace std;


// Default constructor
template<typename T>
TimeSeries<T>::TimeSeries()
    : Base() {}

#ifdef GSL
template<typename T>
TimeSeries<T>::~TimeSeries() {
    if (r_ != nullptr) {
        gsl_rng_free(r_);
        r_ = nullptr;
    }
}
#else
template<typename T>
TimeSeries<T>::~TimeSeries() = default;
#endif


// Construct from vector<DataPoint>
template<typename T>
TimeSeries<T>::TimeSeries(const Base& points)
    : Base(points) {
    detectStructure();
}

// Construct from separate t and c vectors
template<typename T>
TimeSeries<T>::TimeSeries(const std::vector<T>& t, const std::vector<T>& c) {
    size_t n = std::min(t.size(), c.size());
    this->reserve(n);
    for (size_t i = 0; i < n; ++i)
        this->emplace_back(DataPoint{t[i], c[i], std::nullopt});
    detectStructure();
}

// Construct with zero elements
template<typename T>
TimeSeries<T>::TimeSeries(size_t n) {
    this->reserve(n);
    for (size_t i = 0; i < n; ++i)
        this->emplace_back(DataPoint{T{0}, T{0}, std::nullopt});

    // With all t = 0, we cannot assume it's structured
    structured_ = false;
    dt_ = T{};
}

// Copy constructor
template<typename T>
TimeSeries<T>::TimeSeries(const TimeSeries<T>& other)
    : Base(other) {
    structured_ = other.structured_;
    dt_ = other.dt_;
}

// Copy constructor with interval
template<typename T>
TimeSeries<T>::TimeSeries(const std::vector<T>& data, int writeInterval) {
    this->reserve((data.size() + writeInterval - 1) / writeInterval); // conservative prealloc

    for (size_t i = 0; i < data.size(); ++i) {
        if (i % writeInterval == 0)
            this->emplace_back(DataPoint{static_cast<T>(i), data[i], std::nullopt});
    }

    detectStructure(); // auto-sets structured_ and dt_
}

#ifdef _arma
template<typename T>
TimeSeries<T>::TimeSeries(const arma::mat& x, const arma::mat& y) {
    if (x.n_elem != y.n_elem || x.n_cols != 1 || y.n_cols != 1)
        return;

    this->reserve(x.n_elem);
    for (size_t i = 0; i < x.n_elem; ++i) {
        this->emplace_back(DataPoint{
            static_cast<T>(x(i, 0)),
            static_cast<T>(y(i, 0)),
            std::nullopt
        });
    }

    detectStructure();
#ifdef GSL
    ensureGSLInitialized();
#endif
}
#endif

template<typename T>
TimeSeries<T>::TimeSeries(const std::string& filename)
    : fileNotFound(false), fileNotCorrect(false), structured_(true)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        fileNotFound = true;
        return;
    }

    std::string line;
    std::vector<T> tvec, cvec;

    while (std::getline(file, line)) {
        auto s = aquiutils::split(line, ',');
        if (s.size() < 2) continue;

        std::string& token0 = s[0];
        if (token0.substr(0, 2) == "//" || aquiutils::tolower(token0) == "names" || aquiutils::trim(token0).empty())
            continue;

        try {
            T tval = static_cast<T>(std::stod(aquiutils::trim(s[0])));
            T cval = static_cast<T>(std::stod(aquiutils::trim(s[1])));
            tvec.push_back(tval);
            cvec.push_back(cval);
        } catch (...) {
            fileNotCorrect = true;
            return;
        }
    }

    file.close();

    if (tvec.empty()) {
        fileNotCorrect = true;
        return;
    }

    this->reserve(tvec.size());
    for (size_t i = 0; i < tvec.size(); ++i)
        this->emplace_back(DataPoint{tvec[i], cvec[i], std::nullopt});

    detectStructure();

#ifdef GSL
    ensureGSLInitialized();
#endif
}


template<typename T>
void TimeSeries<T>::clear() {
    Base::clear();
    structured_ = true;
    dt_ = T{};
}

// Move constructor
template<typename T>
TimeSeries<T>::TimeSeries(TimeSeries<T>&& other) noexcept
    : Base(std::move(other))
{
    structured_ = other.structured_;
    dt_ = other.dt_;
}

// Copy assignment
template<typename T>
TimeSeries<T>& TimeSeries<T>::operator=(const TimeSeries<T>& other) {
    if (this != &other) {
        Base::operator=(other);
    }
    return *this;
}

// Move assignment
template<typename T>
TimeSeries<T>& TimeSeries<T>::operator=(TimeSeries<T>&& other) noexcept {
    if (this != &other) {
        Base::operator=(std::move(other));
    }
    return *this;
}

template<typename T>
T TimeSeries<T>::mean() const {
    if (this->empty()) return T{};
    T sum{};
    for (const auto& pt : *this)
        sum += pt.c;
    return sum / static_cast<T>(this->size());
}

template<typename T>
T TimeSeries<T>::mint() const {
    if (this->empty()) return T{};
    T min_t = (*this)[0].t;
    for (const auto& pt : *this)
        if (pt.t < min_t) min_t = pt.t;
    return min_t;
}

template<typename T>
T TimeSeries<T>::maxt() const {
    if (this->empty()) return T{};
    T max_t = (*this)[0].t;
    for (const auto& pt : *this)
        if (pt.t > max_t) max_t = pt.t;
    return max_t;
}

template<typename T>
T TimeSeries<T>::minC() const {
    if (this->empty()) return T{};
    T min_val = (*this)[0].c;
    for (const auto& pt : *this)
        if (pt.c < min_val) min_val = pt.c;
    return min_val;
}

template<typename T>
T TimeSeries<T>::maxC() const {
    if (this->empty()) return T{};
    T max_val = (*this)[0].c;
    for (const auto& pt : *this)
        if (pt.c > max_val) max_val = pt.c;
    return max_val;
}

template<typename T>
T TimeSeries<T>::interpol(const T& x) const {
    if (this->empty()) return T{};
    if (x <= this->front().t) return this->front().c;
    if (x >= this->back().t) return this->back().c;

    if (structured_ && this->size() > 1) {
        T t0 = this->front().t;
        int i = static_cast<int>((x - t0) / dt_);

        if (i < 0) return this->front().c;
        if (i >= static_cast<int>(this->size()) - 1)
            return this->back().c;

        const auto& p1 = (*this)[i];
        const auto& p2 = (*this)[i + 1];
        T ratio = (x - p1.t) / (p2.t - p1.t);
        return p1.c + ratio * (p2.c - p1.c);
    }

    // fallback for unstructured case
    for (size_t i = 0; i < this->size() - 1; ++i) {
        const auto& p1 = (*this)[i];
        const auto& p2 = (*this)[i + 1];
        if (p1.t <= x && x <= p2.t) {
            T ratio = (x - p1.t) / (p2.t - p1.t);
            return p1.c + ratio * (p2.c - p1.c);
        }
    }

    return this->back().c;
}


template<typename T>
void TimeSeries<T>::detectStructure() {
    if (this->size() < 3) {
        structured_ = true;
        dt_ = (this->size() >= 2) ? ((*this)[1].t - (*this)[0].t) : T{};
        return;
    }

    T dt0 = (*this)[1].t - (*this)[0].t;
    structured_ = true;
    for (size_t i = 2; i < this->size(); ++i) {
        T dt_i = (*this)[i].t - (*this)[i - 1].t;
        if (std::abs(dt_i - dt0) > 1e-10 * std::abs(dt0)) {
            structured_ = false;
            dt_ = 0;
            return;
        }
    }
    dt_ = dt0;
}

template<typename T>
void TimeSeries<T>::addPoint(T t, T c, std::optional<T> d) {
    this->emplace_back(DataPoint{t, c, d});

    size_t n = this->size();
    if (!structured_ || n < 3)
        return;

    // Check dt consistency
    const T dt_prev = (*this)[n - 2].t - (*this)[n - 3].t;
    const T dt_curr = (*this)[n - 1].t - (*this)[n - 2].t;

    if (n == 3) {
        dt_ = dt_curr;
    } else {
        T rel_diff = std::abs(dt_curr - dt_) / std::max(std::abs(dt_), T(1e-12));
        if (rel_diff > 1e-6) {
            structured_ = false;
            dt_ = T{}; // reset since not valid anymore
        }
    }
}

template<typename T>
void TimeSeries<T>::setNumPoints(size_t n) {
    this->resize(n, DataPoint{T{0}, T{0}, std::nullopt});
    structured_ = false;
    dt_ = T{};
}

template<typename T>
bool TimeSeries<T>::setTime(size_t i, const T& value) {
    if (i < this->size()) {
        (*this)[i].t = value;
        return true;
    }
    return false;
}

template<typename T>
bool TimeSeries<T>::setValue(size_t i, const T& value) {
    if (i < this->size()) {
        (*this)[i].c = value;
        return true;
    }
    return false;
}

template<typename T>
bool TimeSeries<T>::setDistance(size_t i, const T& value) {
    if (i < this->size()) {
        (*this)[i].d = value;
        return true;
    }
    return false;
}

template<typename T>
T TimeSeries<T>::getTime(size_t i) const {
    if (i < this->size())
        return (*this)[i].t;
    return T{};
}

template<typename T>
T TimeSeries<T>::getValue(size_t i) const {
    if (i < this->size())
        return (*this)[i].c;
    return T{};
}

template<typename T>
std::optional<T> TimeSeries<T>::getDistance(size_t i) const {
    if (i < this->size() && (*this)[i].d.has_value())
        return (*this)[i].d.value();
    return T{};
}

template<typename T>
TimeSeries<T> TimeSeries<T>::log() const {
    TimeSeries<T> out;
    out.reserve(this->size());
    for (const auto& pt : *this) {
        out.emplace_back(DataPoint{pt.t, std::log(pt.c), pt.d});
    }
    out.structured_ = this->structured_;
    out.dt_ = this->dt_;
    return out;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::log(T minval) const {
    TimeSeries<T> out;
    out.reserve(this->size());
    for (const auto& pt : *this) {
        T safe_c = std::max(pt.c, minval);
        out.emplace_back(DataPoint{pt.t, std::log(safe_c), pt.d});
    }
    out.structured_ = this->structured_;
    out.dt_ = this->dt_;
    return out;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::movingAverageSmooth(int span) const {
    TimeSeries<T> out;
    out.reserve(this->size());

    for (size_t i = 0; i < this->size(); ++i) {
        int i_start = std::max<int>(0, static_cast<int>(i) - span);
        int i_end   = std::min<int>(this->size() - 1, i + span);
        int count   = i_end - i_start + 1;

        T sum{};
        for (int j = i_start; j <= i_end; ++j)
            sum += (*this)[j].c;

        T average = sum / static_cast<T>(count);
        out.emplace_back(DataPoint{(*this)[i].t, average, (*this)[i].d});
    }

    out.structured_ = this->structured_;
    out.dt_ = this->dt_;
    return out;
}

template<typename T>
T TimeSeries<T>::interpol_D(const T& x) const {
    if (this->empty())
        return T{};

    if (x <= this->front().t)
        return this->front().d.value_or(this->front().t - x);

    if (x >= this->back().t)
        return this->back().d.value_or(T{});

    if (structured_ && this->size() >= 2) {
        int i = static_cast<int>((x - this->front().t) / dt_);
        if (i < 0) return this->front().d.value_or(T{});
        if (static_cast<size_t>(i + 1) >= this->size()) return this->back().d.value_or(T{});

        const auto& p1 = (*this)[i];
        const auto& p2 = (*this)[i + 1];

        if (!p1.d || !p2.d)
            return p1.d.value_or(p2.d.value_or(T{}));

        T dt = p2.t - p1.t;
        T interpolated = p1.d.value() + (p2.d.value() - p1.d.value()) * (x - p1.t) / dt;
        return std::max(interpolated, dt);
    }

    // Unstructured mode
    for (size_t i = 0; i < this->size() - 1; ++i) {
        const auto& p1 = (*this)[i];
        const auto& p2 = (*this)[i + 1];

        if (p1.t <= x && x <= p2.t) {
            if (!p1.d || !p2.d)
                return p1.d.value_or(p2.d.value_or(p2.t - p1.t));

            T dt = p2.t - p1.t;
            T interpolated = p1.d.value() + (p2.d.value() - p1.d.value()) * (x - p1.t) / dt;
            return std::max(interpolated, dt);
        }
    }

    return this->back().d.value_or(T{});
}

template<typename T>
TimeSeries<T> TimeSeries<T>::interpol(const std::vector<T>& x) const {
    TimeSeries<T> out;
    out.reserve(x.size());

    for (const auto& ti : x)
        out.emplace_back(DataPoint{ti, this->interpol(ti), std::nullopt});

    out.structured_ = false;
    out.dt_ = T{};
    return out;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::interpol(const TimeSeries<T>& x) const {
    TimeSeries<T> out;
    out.reserve(x.size());

    for (const auto& pt : x)
        out.emplace_back(DataPoint{pt.t, this->interpol(pt.t), std::nullopt});

    out.structured_ = x.structured_;
    out.dt_ = x.dt_;
    return out;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::interpol(const TimeSeries<T>* x) const {
    TimeSeries<T> out;

    if (!x || x->empty() || this->empty())
        return out;

    out.reserve(x->size());

    for (const auto& pt : *x)
        out.emplace_back(DataPoint{pt.t, this->interpol(pt.t), std::nullopt});

    out.structured_ = x->structured_;
    out.dt_ = x->dt_;
    return out;
}

// Scalar * TimeSeries
template<typename T>
TimeSeries<T> operator*(T alpha, const TimeSeries<T>& ts) {
    TimeSeries<T> result;
    result.reserve(ts.size());

    for (const auto& pt : ts)
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, alpha * pt.c, pt.d});

    result.structured_ = ts.structured_;
    result.dt_ = ts.dt_;
    return result;
}

// TimeSeries * Scalar
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts, T alpha) {
    return alpha * ts;  // reuse the first version
}

// TimeSeries / Scalar
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T alpha) {
    TimeSeries<T> result;
    result.reserve(ts.size());

    for (const auto& pt : ts)
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, pt.c / alpha, pt.d});

    result.structured_ = ts.structured_;
    result.dt_ = ts.dt_;
    return result;
}

// TimeSeries / TimeSeries (interpolated)
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    result.reserve(ts1.size());

    for (const auto& pt : ts1) {
        T divisor = ts2.interpol(pt.t);
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, pt.c / divisor, pt.d});
    }

    result.structured_ = false;
    result.dt_ = T{};
    return result;
}

// TimeSeries - TimeSeries (interpolated)
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    result.reserve(ts1.size());

    for (const auto& pt : ts1) {
        T diff = pt.c - ts2.interpol(pt.t);
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, diff, pt.d});
    }

    result.structured_ = false;
    result.dt_ = T{};
    return result;
}

// Pointwise multiplication using interpolation
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    result.reserve(ts1.size());

    for (const auto& pt : ts1) {
        T interpolated = ts2.interpol(pt.t);
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, pt.c * interpolated, pt.d});
    }

    result.structured_ = false;
    result.dt_ = T{};
    return result;
}

// TimeSeries minus a constant
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts, T a) {
    TimeSeries<T> result;
    result.reserve(ts.size());

    for (const auto& pt : ts)
        result.emplace_back(typename TimeSeries<T>::DataPoint{pt.t, pt.c - a, pt.d});

    result.structured_ = ts.structured_;
    result.dt_ = ts.dt_;
    return result;
}

// Pointwise division (no interpolation) — assumes aligned timestamps
template<typename T>
TimeSeries<T> operator%(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    size_t n = std::min(ts1.size(), ts2.size());
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        result.emplace_back(typename TimeSeries<T>::DataPoint{
            ts1[i].t,
            ts1[i].c / ts2[i].c,
            ts1[i].d
        });
    }

    result.structured_ = false;
    result.dt_ = T{};
    return result;
}

// Pointwise addition (no interpolation) — assumes aligned timestamps
template<typename T>
TimeSeries<T> operator&(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    size_t n = std::min(ts1.size(), ts2.size());
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        result.emplace_back(typename TimeSeries<T>::DataPoint{
            ts1[i].t,
            ts1[i].c + ts2[i].c,
            ts1[i].d
        });
    }

    result.structured_ = false;
    result.dt_ = T{};
    return result;
}


namespace TimeSeriesMetrics {

    template<typename T>
    T ADD(const TimeSeries<T>& modeled, const TimeSeries<T>& observed) {
        if (observed.empty())
            return T{};

        T sum = T{};

        for (const auto& pt : observed) {
            T y_obs = pt.c;
            T y_mod = modeled.interpol(pt.t);

            if (std::abs(y_obs) < T(1e-3))
                sum += std::abs(y_obs - y_mod);
            else
                sum += std::abs(y_obs - y_mod) / y_obs;
        }

        return sum / static_cast<T>(observed.size());
    }

    template<typename T>
    T diff_relative(const TimeSeries<T>& A, const TimeSeries<T>& B, T threshold) {
        T sum = T{};
        size_t n = std::min(A.size(), B.size());

        for (size_t i = 0; i < n; ++i) {
            T denom = std::abs(A[i].c);
            T diff_val = std::abs(B[i].c - A.interpol(B[i].t));
            sum += (denom < threshold) ? diff_val : diff_val / denom;
        }

        return sum;
    }

    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, double scale) {
        T sum = T{};
        T norm = std::sqrt(1.0 + scale * scale);

        for (const auto& pt : observed) {
            T pred = predicted.interpol(pt.t);
            T err = pt.c - pred;
            sum += (pt.c > pred ? scale * err * err : err * err) / norm;
        }

        return sum;
    }

    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
        if (predicted.empty() || observed.empty()) return T{};

        T sum = T{};
        for (const auto& pt : observed) {
            T err = pt.c - predicted.interpol(pt.t);
            sum += err * err;
        }

        return sum;
    }

    template<typename T>
    T diff_abs(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
        T sum = T{};
        for (const auto& pt : observed) {
            sum += std::abs(pt.c - predicted.interpol(pt.t));
        }
        return sum;
    }

    template<typename T>
    T diff_log(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, T lowlim) {
        T sum = T{};
        for (const auto& pt : observed) {
            T pred = std::max(predicted.interpol(pt.t), lowlim);
            T obs  = std::max(pt.c, lowlim);
            sum += std::pow(std::log(obs) - std::log(pred), 2);
        }
        return sum;
    }

    template<typename T>
    T diff2(const TimeSeries<T>* predicted, const TimeSeries<T>& observed) {
        if (!predicted || observed.empty()) return T{};

        T sum = T{};
        for (const auto& pt : observed) {
            sum += std::pow(pt.c - predicted->interpol(pt.t), 2);
        }

        return sum / static_cast<T>(observed.size());
    }

    template<typename T>
    T diff2(const TimeSeries<T>& predicted, const TimeSeries<T>* observed) {
        if (!observed || predicted.empty()) return T{};

        T sum = T{};
        int count = 0;

        for (const auto& pt : *observed) {
            if (pt.t > predicted.mint() && pt.t < predicted.maxt()) {
                sum += std::pow(pt.c - predicted.interpol(pt.t), 2);
                ++count;
            }
        }

        return (count > 0) ? sum / static_cast<T>(count) : T{};
    }

    template<typename T>
    T diff2(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
        if (predicted.empty() || observed.empty()) return T{};

        T sum = T{};
        int count = 0;

        for (const auto& pt : observed) {
            if (pt.t > predicted.mint() && pt.t < predicted.maxt()) {
                sum += std::pow(pt.c - predicted.interpol(pt.t), 2);
                ++count;
            }
        }

        return (count > 0) ? sum / static_cast<T>(count) : T{};
    }

    template<typename T>
    T diff_norm(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
        if (observed.empty()) return T{};

        T sum = 0;
        T sumvar1 = 0;
        T sumvar2 = 0;

        size_t n = observed.size();

        for (const auto& pt : observed) {
            T pred = predicted.interpol(pt.t);
            T err = pt.c - pred;
            sum += err * err / n;
            sumvar1 += pt.c * pt.c / n;
            sumvar2 += pred * pred / n;
        }

        return sum / std::sqrt(sumvar1 * sumvar2);
    }

    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, const TimeSeries<T>& weights) {
        T sum = 0;

        for (const auto& pt : observed) {
            T pred = predicted.interpol(pt.t);
            T weight = weights.interpol(pt.t);
            T err = pt.c - pred;
            sum += err * err * weight * weight;
        }

        return sum;
    }

} // namespace TimeSeriesMetrics





