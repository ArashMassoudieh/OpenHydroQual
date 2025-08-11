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


// TimeSeries.cpp: implementation of the CTimeSeries class.
//
//////////////////////////////////////////////////////////////////////

#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>
//#include "StringOP.h"
#include "Utilities.h"
#include "NormalDist.h"
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef Q_JSON_SUPPORT
#include "qfile.h"
#include "qdatastream.h"
#include <qdebug.h>
#endif

#ifdef GSL
#include <gsl/gsl_cdf.h>
#endif
#include <QJsonArray>
#include <QJsonObject>
#include <mutex>



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
        this->emplace_back(DataPoint<T>{t[i], c[i], std::nullopt});
    detectStructure();
}

// Construct with zero elements
template<typename T>
TimeSeries<T>::TimeSeries(size_t n) {
    this->reserve(n);
    for (size_t i = 0; i < n; ++i)
        this->emplace_back(DataPoint<T>{T{0}, T{0}, std::nullopt});

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
    name_ = other.name_;
    unit_ = other.unit_;
    filename_ = other.filename_;

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

// Constructor using linear transformation: y = a + b * x[i]
template<class T>
TimeSeries<T>::TimeSeries(T a, T b, const std::vector<T>& x) {
    this->reserve(x.size());
    for (const auto& xi : x)
        this->emplace_back(DataPoint<T>{xi, a + b * xi, std::nullopt});
    detectStructure();
}

// Constructor using linear transformation applied to another TimeSeries's time vector
template<class T>
TimeSeries<T>::TimeSeries(T a, T b, const TimeSeries<T>& btc)
    : TimeSeries<T>(a, b, btc.getTimes()) {
    name_ = btc.name_;
    unit_ = btc.unit_;
}

template<typename T>
T& TimeSeries<T>::lastD() {
    if (this->empty())
        throw std::out_of_range("Cannot access lastD() of empty TimeSeries.");
    if (!(*this).back().d.has_value())
        (*this).back().d = T{};  // Initialize if not already set
    return (*this).back().d.value();
}

template<typename T>
T& TimeSeries<T>::lastC() {
    if (this->empty())
        throw std::out_of_range("Cannot access lastC() of empty TimeSeries.");
    return (*this).back().c;
}

template<typename T>
T& TimeSeries<T>::lastt() {
    if (this->empty())
        throw std::out_of_range("Cannot access lastt() of empty TimeSeries.");
    return (*this).back().t;
}

template<typename T>
T TimeSeries<T>::Exponential_Kernel(const T& t, const T& lambda) const {
    if (this->empty()) return T{};

    size_t initial_i = GetElementNumberAt(t);
    size_t last_i = std::min(GetElementNumberAt(t + T(2.0) / lambda), int(this->size() - 1));

    T sum = 0;
    for (size_t i = initial_i; i < last_i; ++i) {
        T t_i = (*this)[i].t;
        T t_ip1 = (*this)[i + 1].t;
        T c_i = (*this)[i].c;

        T delta = t_ip1 - t_i;
        sum += c_i * lambda * std::exp(-lambda * (t_i - t)) * delta;
        sum += c_i * lambda * std::exp(-lambda * (t_ip1 - t)) * delta;
    }

    return sum;
}

template<typename T>
T TimeSeries<T>::Gaussian_Kernel(const T& t, const T& mu, const T& stdev) const {
    if (this->size() < 2) return T{};

    const T sqrt_2pi = std::sqrt(2.0 * 3.14159265358979323846);
    const T var = stdev * stdev;

    size_t initial_i = std::max(GetElementNumberAt(t - 2.0 * stdev + mu), int(size_t(0)));
    size_t last_i = std::min(GetElementNumberAt(t + 2.0 * stdev + mu), int(this->size() - 2));

    T sum = 0;
    for (size_t i = initial_i; i <= last_i; ++i) {
        T t1 = (*this)[i].t;
        T t2 = (*this)[i + 1].t;
        T c1 = (*this)[i].c;
        T c2 = (*this)[i + 1].c;
        T delta = t2 - t1;

        T w1 = std::exp(-std::pow(t1 - t - mu, 2) / (2 * var)) / (sqrt_2pi * stdev);
        T w2 = std::exp(-std::pow(t2 - t - mu, 2) / (2 * var)) / (sqrt_2pi * stdev);

        sum += 0.5 * (c1 * w1 + c2 * w2) * delta;
    }

    return sum;
}

template<typename T>
int TimeSeries<T>::GetElementNumberAt(const T& x) const {
    size_t n = this->size();
    if (n == 0)
        return 0;
    if (n == 1)
        return 0;

    if (!structured_) {
        for (size_t i = 0; i < n - 1; ++i) {
            if ((*this)[i].t <= x && (*this)[i + 1].t >= x)
                return static_cast<int>(i);
        }
        if (x > (*this)[n - 1].t) return static_cast<int>(n - 1);
        if (x < (*this)[0].t)     return 0;
    }
    else {
        T t0 = (*this)[0].t;
        T dt = dt_;
        if (x < t0) return 0;
        if (x > (*this)[n - 1].t) return static_cast<int>(n - 1);
        return static_cast<int>((x - t0) / dt);
    }

    return 0;  // fallback
}

template<typename T>
TimeSeries<T> TimeSeries<T>::LogTransformX() const {
    TimeSeries<T> out;
    out.reserve(this->size());

    for (const auto& pt : *this) {
        if (pt.t > T(0)) { // Avoid log of zero or negative
            out.emplace_back(DataPoint<T>{std::log(pt.t), pt.c, pt.d});
        }
    }

    out.setStructured(false);  // After transform, time spacing is no longer uniform
    out.setName(this->getName());
    out.setUnit(this->getUnit());
    return out;
}

template<class T>
void TimeSeries<T>::CreatePeriodicStepFunction(const T& t_start, const T& t_end, const T& duration, const T& gap, const T& magnitude) {
    T t = t_start;
    while (t <= t_end) {
        append(t - 1e-6, T{ 0 });
        append(t, magnitude);
        append(t + duration, magnitude);
        append(t + duration + 1e-6, T{ 0 });
        t += duration + gap;
    }
    assign_D();
}

#ifdef GSL
template<class T>
void TimeSeries<T>::CreateOUProcess(const T& t_start, const T& t_end, const T& dt, const T& theta) {
    ensureGSLInitialized();
    T x = T{ 0 };
    for (T t = t_start; t <= t_end; t += dt) {
        x += -theta * x * dt + std::sqrt(2 * theta * dt) * gsl_ran_gaussian(r_, 1.0);
        append(t, x);
    }
}
#endif

#ifdef GSL
template<class T>
TimeSeries<T> TimeSeries<T>::MapfromNormalScoreToDistribution(const std::string& distribution, const std::vector<double>& parameters) {
    TimeSeries<T> out;
    for (const auto& pt : *this) {
        double u = gsl_cdf_ugaussian_P(pt.c);
        double value = 0.0;

        if (distribution == "lognormal" || distribution == "Lognormal") {
            value = std::exp(parameters[0] + parameters[1] * pt.c);
        }
        else if (distribution == "exp" || distribution == "Exp") {
            value = -parameters[0] * std::log(1.0 - u);
        }
        else if (distribution == "normal" || distribution == "Normal") {
            value = parameters[0] + parameters[1] * pt.c;
        }

        out.append(pt.t, static_cast<T>(value));
    }
    return out;
}
#endif

#ifdef GSL
template<class T>
TimeSeries<T> TimeSeries<T>::MapfromNormalScoreToDistribution(const TimeSeries<double>& distribution) {
    TimeSeries<T> out;
    for (const auto& pt : *this) {
        double u = gsl_cdf_ugaussian_P(pt.c);
        T mapped = static_cast<T>(distribution.interpol(u));
        out.append(pt.t, mapped);
    }
    return out;
}
#endif

#ifdef Q_JSON_SUPPORT
template<class T>
QJsonObject TimeSeries<T>::toJson() const {
    QJsonObject obj;
    QJsonArray tArray;
    QJsonArray cArray;

    for (const auto& dp : *this) {
        tArray.append(static_cast<double>(dp.t));
        cArray.append(static_cast<double>(dp.c));
    }

    obj["t"] = tArray;
    obj["value"] = cArray;

    return obj;
}

template<class T>
void TimeSeries<T>::fromJson(const QJsonObject& obj) {
    clear();

    QJsonArray tArray = obj["t"].toArray();
    QJsonArray cArray = obj["value"].toArray();

    int n = std::min(tArray.size(), cArray.size());
    this->reserve(n);

    for (int i = 0; i < n; ++i) {
        T tVal = static_cast<T>(tArray[i].toDouble());
        T cVal = static_cast<T>(cArray[i].toDouble());
        this->emplace_back(DataPoint<T>{tVal, cVal, std::nullopt});
    }

    detectStructure();
    computeMaxFabs();
}
#endif // Q_JSON_SUPPORT

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

#ifdef GSL
template<typename T>
void TimeSeries<T>::ensureGSLInitialized() {
    static std::once_flag gsl_init_flag;
    std::call_once(gsl_init_flag, []() {
        gsl_set_error_handler_off();  // Disable default error handler
        });

    if (A_ == nullptr) {
        A_ = gsl_rng_default;
    }

    if (r_ == nullptr) {
        r_ = gsl_rng_alloc(A_);
        unsigned long seed = static_cast<unsigned long>(std::time(nullptr));
        gsl_rng_set(r_, seed);
    }
}
#endif

template<typename T>
const std::string& TimeSeries<T>::name() const {
    return name_;
}

template<typename T>
const std::string& TimeSeries<T>::unit() const {
    return unit_;
}

template<typename T>
void TimeSeries<T>::setName(const std::string& name) {
    name_ = name;
}

template<typename T>
void TimeSeries<T>::setUnit(const std::string& unit) {
    unit_ = unit;
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
    name_ = std::move(other.name_);
    unit_ = std::move(other.unit_);
    filename_ = std::move(other.filename_);
}

// Copy assignment
template<typename T>
TimeSeries<T>& TimeSeries<T>::operator=(const TimeSeries<T>& other) {
    if (this != &other) {
        Base::operator=(other);
        name_ = other.name_;
        unit_ = other.unit_;
		structured_ = other.structured_;
        filename_ = other.filename_;
    }
    return *this;
}

// Move assignment
template<typename T>
TimeSeries<T>& TimeSeries<T>::operator=(TimeSeries<T>&& other) noexcept {
    if (this != &other) {
        Base::operator=(std::move(other));
        structured_ = other.structured_;
        dt_ = other.dt_;
        name_ = std::move(other.name_);
        unit_ = std::move(other.unit_);
    }
    return *this;
}

template<typename T>
bool TimeSeries<T>::readfile(const std::string& filename) {
    clear();
    fileNotFound = false;
    fileNotCorrect = false;

    std::ifstream file(filename);
    if (!file.good()) {
        fileNotFound = true;
        return false;
    }

    std::vector<std::string> s;
    T last_t = T{};
    bool first_valid = true;

    while (!file.eof()) {
        s = aquiutils::getline(file);
        if (s.empty()) continue;

        // Clean up malformed numbers
        while (!aquiutils::isnumber(s[0][0]) && s[0].size() > 1)
            s[0] = s[0].substr(1);

        if (s.size() > 1 && s[0].substr(0, 2) != "//" && !aquiutils::trim(s[0]).empty() && aquiutils::isnumber(s[0][0])) {
            if (aquiutils::isnumber(s[1][0])) {
                T t_val = aquiutils::atof(s[0]);
                T c_val = aquiutils::atof(s[1]);

                if (!first_valid && t_val < last_t) {
                    fileNotCorrect = true;
                    return false;
                }

                addPoint(t_val, c_val);
                last_t = t_val;
                first_valid = false;
            }
        }
    }

    file.close();
    filename_ = filename;
	detectStructure();
    return !this->empty();
}

template<typename T>
void TimeSeries<T>::writefile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.good()) return;

    file << "n " << this->size() << ", TimeSeries size " << this->size() << std::endl;
    for (const auto& dp : *this) {
        std::ostringstream time_str;
        time_str << std::fixed << std::setprecision(3) << dp.t;
        file << time_str.str() << "," << dp.c;
    }

    file.close();
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
T TimeSeries<T>::mean(int start_item) const {
    if (static_cast<size_t>(start_item) >= this->size()) return T{};

    T sum = T{};
    size_t count = this->size() - start_item;

    for (size_t i = start_item; i < this->size(); ++i) {
        sum += (*this)[i].c;
    }

    return count > 0 ? sum / static_cast<T>(count) : T{};
}

template<typename T>
T TimeSeries<T>::sum(int start_item) const {
    if (static_cast<size_t>(start_item) >= this->size()) return T{};

    T total = T{};
    for (size_t i = start_item; i < this->size(); ++i) {
        total += (*this)[i].c;
    }

    return total;
}

template<typename T>
T TimeSeries<T>::stddev(int start_item) const {
    if (static_cast<size_t>(start_item) >= this->size()) return T{};

    T mean_val = mean(start_item);
    T sum_sq_diff = T{};
    size_t count = this->size() - start_item;

    for (size_t i = start_item; i < this->size(); ++i) {
        T diff = (*this)[i].c - mean_val;
        sum_sq_diff += diff * diff;
    }

    return count > 0 ? std::sqrt(sum_sq_diff / static_cast<T>(count)) : T{};
}


template<typename T>
T TimeSeries<T>::interpol(const T& x) const {
    
    double dt = dt_; 
    if (structured_ && this->size() > 1)
        dt = (this->getTime(1) - this->getTime(0));
    if (this->empty()) return T{};
    if (x <= this->front().t) return this->front().c;
    if (x >= this->back().t) return this->back().c;

    if (structured_ && this->size() > 1) {
        T t0 = this->front().t;
        int i = static_cast<int>((x - t0) / dt);

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
T TimeSeries<T>::interpol(const T& x,
                          const TimeSeries<T> &CumulativeDistribution,
                          const double &correlationlength) const
{
    // Convert to normal scores
    TimeSeries<T> NormalScores = ConvertToNormalScore();

    // Case 1: x is before the first point
    if (x <= NormalScores.front().t) {
        // Extrapolate from first point toward prior mean (0 for normal scores)
        double rho = std::exp(-(NormalScores.front().t - x) / correlationlength);
        double mean = rho * NormalScores.front().c;  // shrinks toward 0
        double stdev = std::sqrt(1.0 - rho * rho);
        return mean + gsl_ran_ugaussian(r_) * stdev;
    }

    // Case 2: x is after the last point
    if (x >= NormalScores.back().t) {
        double rho = std::exp(-(x - NormalScores.back().t) / correlationlength);
        double mean = rho * NormalScores.back().c;
        double stdev = std::sqrt(1.0 - rho * rho);
        return mean + gsl_ran_ugaussian(r_) * stdev;
    }

    // Case 3: x is between two points
    for (size_t i = 0; i < NormalScores.size() - 1; ++i) {
        const auto& p1 = NormalScores[i];
        const auto& p2 = NormalScores[i + 1];
        if (p1.t <= x && x <= p2.t) {
            double rho1  = std::exp(-(x - p1.t) / correlationlength);
            double rho2  = std::exp(-(p2.t - x) / correlationlength);
            double rho12 = std::exp(-(p2.t - p1.t) / correlationlength);

            double mean =
                (rho1 - rho2 * rho12) / (1 - rho12 * rho12) * p1.c +
                (rho2 - rho1 * rho12) / (1 - rho12 * rho12) * p2.c;

            double var =
                1.0 - (rho1 * rho1 + rho2 * rho2 - 2.0 * rho1 * rho2 * rho12) /
                          (1.0 - rho12 * rho12);

            double stdev = std::sqrt(var);

            return mean + gsl_ran_ugaussian(r_) * stdev;
        }
    }

    // Should never get here
    throw std::runtime_error("Interpolation failed: x is out of range.");
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
    this->emplace_back(DataPoint<T>{t, c, d});

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

    T abs_c = std::fabs(c);
    if (!max_fabs_valid_ || abs_c > max_fabs_)
        max_fabs_ = abs_c;

    max_fabs_valid_ = true; // always true after append
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
		if (std::fabs(value) > max_fabs_) {
			max_fabs_ = std::fabs(value);
			max_fabs_valid_ = true;
		}
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
        out.emplace_back(DataPoint<T>{pt.t, std::log(safe_c), pt.d});
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
        out.emplace_back(DataPoint<T>{pt.t, this->interpol(pt.t), std::nullopt});

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

    result.setStructured(ts.isStructured());
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
        result.emplace_back(DataPoint<T>{pt.t, diff, pt.d});
    }

    result.setStructured(false);
    result.setdt(T{});
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

template<typename T>
T TimeSeries<T>::maxC() const {
    if (this->empty()) return T{};
    T max_val = (*this)[0].c;
    for (const auto& dp : *this)
        if (dp.c > max_val)
            max_val = dp.c;
    return max_val;
}

template<typename T>
T TimeSeries<T>::maxt() const {
    if (this->empty()) return T{};
    T max_time = (*this)[0].t;
    for (const auto& dp : *this)
        if (dp.t > max_time)
            max_time = dp.t;
    return max_time;
}

template<typename T>
T TimeSeries<T>::mint() const {
    if (this->empty()) return T{};
    T min_time = (*this)[0].t;
    for (const auto& dp : *this)
        if (dp.t < min_time)
            min_time = dp.t;
    return min_time;
}

template<typename T>
T TimeSeries<T>::minC() const {
    if (this->empty()) return T{};
    T min_val = (*this)[0].c;
    for (const auto& dp : *this)
        if (dp.c < min_val)
            min_val = dp.c;
    return min_val;
}

template<typename T>
T TimeSeries<T>::maxfabs() const {
    if (this->empty()) return T{};
    T max_val = std::fabs((*this)[0].c);
    for (const auto& dp : *this) {
        T abs_val = std::fabs(dp.c);
        if (abs_val > max_val)
            max_val = abs_val;
    }
    return max_val;
}

template<typename T>
T TimeSeries<T>::maxVal() const {
    return maxC();
}

template<typename T>
T TimeSeries<T>::minVal() const {
    return minC();
}

template<typename T>
T TimeSeries<T>::maxAbsVal() const {
    return maxfabs();
}

template<typename T>
T TimeSeries<T>::minAbsVal() {
    if (this->empty()) return T{};

    if (!max_fabs_valid_) {
        max_fabs_ = std::fabs((*this)[0].c);
        for (const auto& dp : *this) {
            T abs_val = std::fabs(dp.c);
            if (abs_val > max_fabs_)
                max_fabs_ = abs_val;
        }
        max_fabs_valid_ = true;
    }

    return max_fabs_;
}

// Numerical integration using trapezoidal rule over full domain
template<typename T>
T TimeSeries<T>::integrate() const {
    if (this->size() < 2) return T{};

    T sum = T{};
    for (size_t i = 1; i < this->size(); ++i) {
        const auto& p0 = (*this)[i - 1];
        const auto& p1 = (*this)[i];
        sum += (p0.c + p1.c) * (p1.t - p0.t) / 2.0;
    }
    return sum;
}

// Integration up to a specific time point tt
template<typename T>
T TimeSeries<T>::integrate(T tt) const {
    if (this->size() < 2) return T{};

    T sum = T{};
    for (size_t i = 1; i < this->size(); ++i) {
        const auto& p0 = (*this)[i - 1];
        const auto& p1 = (*this)[i];
        if (p1.t <= tt)
            sum += (p0.c + p1.c) * (p1.t - p0.t) / 2.0;
        else
            break;
    }
    return sum;
}

// Integration between two times using linear interpolation where necessary
template<typename T>
T TimeSeries<T>::integrate(T t1, T t2) const {
    if (this->size() < 2 || t2 <= t1) return T{};

    T sum = T{};
    for (size_t i = 1; i < this->size(); ++i) {
        const auto& p0 = (*this)[i - 1];
        const auto& p1 = (*this)[i];

        if (p1.t <= t1 || p0.t >= t2) continue;

        T a = std::max(p0.t, t1);
        T b = std::min(p1.t, t2);

        T c0 = interpol(a);
        T c1 = interpol(b);

        sum += (c0 + c1) * (b - a) / 2.0;
    }

    return sum;
}

// TimeSeries derivative: dC/dt using central difference
template<typename T>
TimeSeries<T> TimeSeries<T>::derivative() const {
    TimeSeries<T> out;
    if (this->size() < 2) return out;

    for (size_t i = 1; i < this->size(); ++i) {
        const auto& p0 = (*this)[i - 1];
        const auto& p1 = (*this)[i];

        T mid_t = (p0.t + p1.t) / 2.0;
        T deriv = (p1.c - p0.c) / (p1.t - p0.t);
        out.emplace_back(DataPoint<T>{mid_t, deriv, std::nullopt});
    }

    out.structured_ = false;
    return out;
}

// Weighted variance using trapezoidal integration
template<typename T>
T TimeSeries<T>::variance() const {
    if (this->size() < 2) return T{};

    T avg = mean();
    T sum = T{};

    for (size_t i = 1; i < this->size(); ++i) {
        const auto& p0 = (*this)[i - 1];
        const auto& p1 = (*this)[i];

        T avg_c = (p0.c + p1.c) / 2.0;
        T weight = p1.t - p0.t;
        sum += std::pow(avg_c - avg, 2) * weight;
    }

    return sum / (this->back().t - this->front().t);
}

// Find index of the interval where time falls (i.e., t[i] < query < t[i+1])
template<typename T>
int TimeSeries<T>::lookupt(T query_time) const {
    for (size_t i = 0; i + 1 < this->size(); ++i) {
        if ((*this)[i].t < query_time && (*this)[i + 1].t > query_time)
            return static_cast<int>(i);
    }
    return -1;
}

// Compute average over the entire time series (area under curve divided by duration)
template<typename T>
T TimeSeries<T>::average() const {
    if (this->empty()) return T{};
    T duration = this->back().t - this->front().t;
    return duration > T(0) ? integrate() / duration : T{};
}

// Compute average from t[0] up to a cutoff time tt
template<typename T>
T TimeSeries<T>::average(T tt) const {
    if (this->empty()) return T{};
    T duration = std::max(tt, this->back().t) - this->front().t;
    return duration > T(0) ? integrate(tt) / duration : T{};
}

// Return the slope between the last two points
template<typename T>
T TimeSeries<T>::slope() const {
    if (this->size() < 2) return T{};
    const auto& last = this->back();
    const auto& second_last = (*this)[this->size() - 2];
    return (last.c - second_last.c) / (last.t - second_last.t);
}

// Return the percentile value of the time series concentration (c) values
template<typename T>
T TimeSeries<T>::percentile(T quantile) const {
    if (this->empty()) return T{};
    std::vector<T> values;
    values.reserve(this->size());

    for (const auto& pt : *this)
        values.push_back(pt.c);

    std::sort(values.begin(), values.end());
    size_t index = static_cast<size_t>(quantile * values.size());
    index = std::min(index, values.size() - 1);

    return values[index];
}

template<typename T>
T TimeSeries<T>::percentile(T p, int start_index) const {
    if (start_index >= static_cast<int>(this->size()))
        return T{};  // Empty or invalid range

    std::vector<T> values;
    values.reserve(this->size() - start_index);

    for (size_t i = start_index; i < this->size(); ++i)
        values.push_back((*this)[i].c);

    std::sort(values.begin(), values.end());

    size_t index = static_cast<size_t>(p * values.size());
    if (index >= values.size()) index = values.size() - 1;

    return values[index];
}

template<typename T>
T TimeSeries<T>::mean_log(int start_index) const {
    if (start_index >= static_cast<int>(this->size()))
        return T{};

    T sum = T{};
    int count = 0;

    for (size_t i = start_index; i < this->size(); ++i) {
        sum += std::log((*this)[i].c);
        ++count;
    }

    return (count > 0) ? sum / static_cast<T>(count) : T{};
}

template<typename T>
bool TimeSeries<T>::append(T value) {
    bool increased_capacity = false;

    if (this->capacity() == this->size()) {
        this->reserve(this->size() + 1);
        increased_capacity = true;
    }

    this->emplace_back(DataPoint<T>{T{ 0 }, value, std::nullopt});

    // Update max absolute value
    T abs_val = std::fabs(value);
    if (!max_fabs_valid_ || abs_val > max_fabs_) {
        max_fabs_ = abs_val;
        max_fabs_valid_ = true;
    }

    // Assume structure unknown after unaligned append
    structured_ = false;

    return increased_capacity;
}

template<typename T>
bool TimeSeries<T>::append(T t_val, T c_val) {
    addPoint(t_val, c_val);
	return true; // Always true since we add a point
}

template<typename T>
void TimeSeries<T>::append(const TimeSeries<T>& other) {
    for (const DataPoint<T>& point : other) {
        this->addPoint(point.t, point.c, point.d);
    }
}

/*template<typename T>
void TimeSeries<T>::adjust_size(size_t new_size) {
    this->resize(new_size);
    detectStructure();  // Ensure internal consistency if structure depends on length
    max_fabs_ = computeMaxFabs();  // Recalculate cached max absolute value
}*/

template <typename T>
void TimeSeries<T>::removeNaNs() {
    Base cleaned;
    cleaned.reserve(this->size());

    for (const DataPoint<T>& pt : *this) {
        if (!std::isnan(pt.t) && !std::isnan(pt.c))
            cleaned.push_back(pt);
    }

    this->assign(cleaned.begin(), cleaned.end());
    detectStructure();
}

template <typename T>
TimeSeries<T> TimeSeries<T>::removeNaNs() const {
    TimeSeries<T> cleaned;
    cleaned.reserve(this->size());

    for (const auto& pt : *this) {
        if (!std::isnan(pt.t) && !std::isnan(pt.c))
            cleaned.push_back(pt);
    }

    cleaned.detectStructure();
    return cleaned;
}

template<typename T>
T TimeSeries<T>::computeMaxFabs() const {
    if (this->empty()) return T{};

    T max_val = T{};
    bool found_valid = false;

    for (const auto& pt : *this) {
        if (!std::isnan(pt.c)) {
            T abs_val = std::fabs(pt.c);
            if (!found_valid || abs_val > max_val) {
                max_val = abs_val;
                found_valid = true;
            }
        }
    }

    return found_valid ? max_val : T{};
}

template<typename T>
TimeSeries<T>& TimeSeries<T>::operator+=(const TimeSeries<T>& v) {
    for (auto& dp : *this) {
        dp.c += v.interpol(dp.t);
    }
    computeMaxFabs(); // Recompute cache
    return *this;
}

template<typename T>
TimeSeries<T>& TimeSeries<T>::operator%=(const TimeSeries<T>& v) {
    size_t n = std::min(this->size(), v.size());
    for (size_t i = 0; i < n; ++i) {
        (*this)[i].c += v[i].c;
    }
    computeMaxFabs(); // Recompute cache
    return *this;
}

template<typename T>
TimeSeries<T> operator+(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
    TimeSeries<T> result;
    result.reserve(ts1.size());

    for (const auto& pt : ts1) {
        T interpolated = ts2.interpol(pt.t);
        result.emplace_back(DataPoint<T>{pt.t, pt.c + interpolated, pt.d});
    }

    result.detectStructure();
    result.computeMaxFabs();
    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::make_uniform(T increment, bool assignD) const {
    TimeSeries<T> out;

	TimeSeries<T> nansRemoved = this->removeNaNs();
    if (this->size() < 2) return out;

    if (assignD) {
        // Ensure all distances are initialized
        for (DataPoint<T>& pt : const_cast<TimeSeries<T>&>(nansRemoved)) {
            if (!pt.d.has_value()) {
                pt.d = T{};
            }
        }
    }

    T t0 = this->front().t;
    T t_end = this->back().t;

    size_t i = 0;
    T current_t = t0;

    while (current_t <= t_end && i < nansRemoved.size() - 1) {
        const DataPoint<T>& p1 = nansRemoved[i];
        const DataPoint<T>& p2 = nansRemoved[i + 1];

        if (p1.t <= current_t && current_t <= p2.t) {
            T ratio = (current_t - p1.t) / (p2.t - p1.t);
            if (p2.t == p1.t) ratio = 0.5; 
            T c_interp = p1.c + ratio * (p2.c - p1.c);
            std::optional<T> d_interp = std::nullopt;

            if (assignD && p1.d.has_value() && p2.d.has_value()) {
                d_interp = p1.d.value() + ratio * (p2.d.value() - p1.d.value());
            }

            out.addPoint(current_t, c_interp, d_interp);
            current_t += increment;
        }
        else {
            ++i;
        }
    }
    out.setName(name());
	if (assignD) {
        out.assign_D();
    }
    
    out.setUnit(unit());
    out.structured_ = true;
    out.dt_ = increment;
    return out;
}


template<typename T>
T TimeSeries<T>::getLastValue() const {
    return this->empty() ? T{} : this->back().c;
}

template<typename T>
T TimeSeries<T>::getLastTime() const {
    return this->empty() ? T{} : this->back().t;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::extract(T t1, T t2) const {
    TimeSeries<T> out;
    out.reserve(this->size());

    for (const auto& pt : *this) {
        if (pt.t >= t1 && pt.t <= t2)
            out.addPoint(pt.t, pt.c, pt.d);
    }

    out.structured_ = false;
    out.dt_ = T{};
    out.computeMaxFabs();
    return out;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::distribution(int n_bins, int start_index) const {
    TimeSeries<T> out(n_bins + 2);  // histogram will have n_bins + 2 edges

    // Validate range
    if (this->size() <= static_cast<size_t>(start_index) || n_bins <= 0)
        return out;

    // Copy the values starting from start_index
    std::vector<T> values;
    values.reserve(this->size() - start_index);
    for (size_t i = start_index; i < this->size(); ++i)
        values.push_back((*this)[i].c);

    if (values.empty()) return out;

    // Determine bin parameters
    auto min_val = *std::min_element(values.begin(), values.end());
    auto max_val = *std::max_element(values.begin(), values.end()) * static_cast<T>(1.001);
    T bin_width = std::abs(max_val - min_val) / static_cast<T>(n_bins);

    if (bin_width == T{}) return out;

    // Initialize bin edges and zero values
    out[0].t = min_val - bin_width / 2;
    out[0].c = T{};

    for (int i = 1; i < n_bins + 2; ++i) {
        out[i].t = out[i - 1].t + bin_width;
        out[i].c = T{};
    }

    // Count normalized density
    for (const auto& val : values) {
        int bin = static_cast<int>((val - min_val) / bin_width) + 1;
        if (bin >= 0 && bin < static_cast<int>(out.size()))
            out[bin].c += T{ 1 } / static_cast<T>(values.size()) / bin_width;
    }

    out.structured_ = false;
    out.dt_ = T{};
    out.computeMaxFabs();
    return out;
}

template<typename T>
T TimeSeries<T>::mean_t() const {
    if (this->empty()) return T{};

    T sum = T{};
    for (const auto& pt : *this)
        sum += pt.t;

    return sum / static_cast<T>(this->size());
}

template<typename T>
std::vector<T> TimeSeries<T>::trend() const {
    std::vector<T> result(2, T{});

    if (this->size() < 2) return result;

    T x_bar = this->mean_t();
    T y_bar = this->mean();

    T sum_num = T{};
    T sum_denom = T{};

    for (const auto& pt : *this) {
        T dx = pt.t - x_bar;
        sum_num += dx * (pt.c - y_bar);
        sum_denom += dx * dx;
    }

    result[1] = (sum_denom != T{}) ? (sum_num / sum_denom) : T{};
    result[0] = y_bar - result[1] * x_bar;

    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::add_noise(T stddev, bool logd) const {
    TimeSeries<T> noisy;
    noisy.reserve(this->size());

#ifdef GSL
    ensureGSLInitialized();
#else
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<T> dist(0, stddev);
#endif

    for (const auto& pt : *this) {
#ifdef GSL
        T noise = gsl_ran_gaussian(r_, stddev);
#else
        T noise = dist(gen);
#endif
        T new_val = logd ? pt.c * std::exp(noise) : pt.c + noise;
        noisy.emplace_back(DataPoint<T>{pt.t, new_val, pt.d});
    }

    noisy.structured_ = this->structured_;
    noisy.dt_ = this->dt_;
    noisy.filename = this->filename;

    noisy.computeMaxFabs();  // Ensure internal max_fabs is up to date
    return noisy;
}

template<typename T>
T sum_interpolate(const std::vector<TimeSeries<T>>& series_list, T time) {
    T sum = T{};
    for (const auto& series : series_list) {
        sum += series.interpol(time);
    }
    return sum;
}

template<typename T>
void TimeSeries<T>::assign_D() {
    if (this->empty()) return;

    for (size_t i = 0; i < this->size(); ++i) {
        T counter = T{};

        for (size_t j = i + 1; j < this->size(); ++j) {
            if ((*this)[j].c == (*this)[i].c) {
                counter += (*this)[j].t - (*this)[j - 1].t;
            }
            else {
                counter += (*this)[j].t - (*this)[j - 1].t;
                break;
            }
        }

        if (i + 1 == this->size() && this->size() > 1)
            counter = (*this)[this->size() - 1].t - (*this)[this->size() - 2].t;
        else if (this->size() == 1)
            counter = T(100);

        if (counter == T{}) {
            counter = (i > 0) ? (*this)[i].t - (*this)[i - 1].t : (*this)[0].t;
        }

        (*this)[i].d = std::fabs(counter);
    }
}

template<typename T>
T TimeSeries<T>::wiggle() const {
    size_t n = this->size();
    if (n < 3) return T{};

    T t1 = (*this)[n - 1].t;
    T t2 = (*this)[n - 2].t;
    T t3 = (*this)[n - 3].t;

    T c1 = std::fabs((*this)[n - 1].c);
    T c2 = std::fabs((*this)[n - 2].c);
    T c3 = std::fabs((*this)[n - 3].c);

    T numerator = 3 * (c1 * (t2 - t3) - c2 * (t1 - t3) + c3 * (t1 - t2));
    T denominator = (t1 - t3) * std::max(max_fabs_, T(1e-7));

    return numerator / denominator;
}

template<typename T>
T TimeSeries<T>::wiggle_corr(int n_back) const {
    size_t n = this->size();
    if (n < static_cast<size_t>(n_back)) return T{};

    T C_m = T{};
    for (int i = 0; i < n_back; ++i)
        C_m += (*this)[n - 1 - i].c / T(n_back);

    T sum = T{}, var = T{};
    for (int i = 0; i < n_back - 1; ++i)
        sum += ((*this)[n - 1 - i].c - C_m) * ((*this)[n - 2 - i].c - C_m);

    for (int i = 0; i < n_back; ++i)
        var += std::pow((*this)[n - 1 - i].c - C_m, 2);

    return (var == T{}) ? T{} : sum / var;
}

template<typename T>
bool TimeSeries<T>::wiggle_sl(T tol) const {
    size_t n = this->size();
    if (n < 4) return false;

    T c1 = (*this)[n - 1].c;
    T c2 = (*this)[n - 2].c;
    T c3 = (*this)[n - 3].c;
    T c4 = (*this)[n - 4].c;

    T t1 = (*this)[n - 1].t;
    T t2 = (*this)[n - 2].t;
    T t3 = (*this)[n - 3].t;
    T t4 = (*this)[n - 4].t;

    T mean_val = (std::fabs(c1) + std::fabs(c2) + std::fabs(c3) + std::fabs(c4)) / T(4.0) + tol / T(100.0);

    T slope1 = (c1 - c2) / (t1 - t2) / mean_val;
    T slope2 = (c2 - c3) / (t2 - t3) / mean_val;
    T slope3 = (c3 - c4) / (t3 - t4) / mean_val;

    bool all_small = std::fabs(slope1) < tol && std::fabs(slope2) < tol && std::fabs(slope3) < tol;
    bool alternating = (slope1 * slope2 < 0) && (slope2 * slope3 < 0);

    return (!all_small && alternating);
}

template<typename T>
void TimeSeries<T>::knock_out(T cutoff_time) {
    size_t keep_count = 0;

    while (keep_count < this->size() && (*this)[keep_count].t <= cutoff_time) {
        ++keep_count;
    }

    this->resize(keep_count);
    max_fabs_valid_ = false;  // Invalidate cached max_fabs since data changed
}

template<typename T>
T TimeSeries<T>::AutoCor1(int k) const {
    size_t n = this->size();
    if (k <= 0 || static_cast<size_t>(k) >= n) k = static_cast<int>(n);

    T mean_val = mean();
    T sum_product = 0;
    T sum_sq = 0;

    for (size_t i = n - k; i < n - 1; ++i) {
        T diff_i = (*this)[i].c - mean_val;
        T diff_next = (*this)[i + 1].c - mean_val;
        sum_product += diff_i * diff_next;
        sum_sq += diff_i * diff_i;
    }

    return (sum_sq == T{}) ? T{} : sum_product / sum_sq;
}

template<typename T>
T TimeSeries<T>::AutoCorrelation(const T& distance) const {
    T mean_val = mean();
    T sum_product = 0;
    T sum_sq = 0;

    for (size_t i = 0; i < this->size(); ++i) {
        T ti = (*this)[i].t;
        if (ti + distance < this->back().t) {
            T ci = (*this)[i].c;
            T interpolated = interpol(ti + distance);
            T diff_i = ci - mean_val;
            T diff_j = interpolated - mean_val;
            sum_product += diff_i * diff_j;
            sum_sq += diff_i * diff_i;
        }
    }

    return (sum_sq == T{}) ? T{} : sum_product / sum_sq;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::AutoCorrelation(const T& span, const T& increment) const {
    TimeSeries<T> result;
    for (T d = T{}; d <= span; d += increment) {
        result.addPoint(d, AutoCorrelation(d));
    }
    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::ConvertToRanks() const {
    TimeSeries<T> ranked;
    ranked.reserve(this->size());

    for (const auto& pt : *this) {
        T score = Score(pt.c);
        ranked.addPoint(pt.t, score);
    }

    ranked.structured_ = this->structured_;
    ranked.dt_ = this->dt_;
    return ranked;
}

template<typename T>
T TimeSeries<T>::Score(const T& val) const {
    if (this->empty()) return T{};

    T score = T(0.5) / static_cast<T>(this->size());
    for (const auto& pt : *this) {
        if (pt.c < val)
            score += T(1.0) / static_cast<T>(this->size());
    }
    return score;
}

#ifdef GSL
template<typename T>
TimeSeries<T> TimeSeries<T>::ConvertToNormalScore() const {
    TimeSeries<T> ranked = ConvertToRanks();
    TimeSeries<T> result;
    result.reserve(this->size());

    for (const auto& pt : ranked) {
        T normal_val = static_cast<T>(gsl_cdf_gaussian_Pinv(pt.c, 1.0));
        result.addPoint(pt.t, normal_val);
    }

    result.structured_ = this->structured_;
    result.dt_ = this->dt_;
    return result;
}
#endif

template<typename T>
double TimeSeries<T>::AutoCorrelationCoeff() const {
    double numerator = 0.0;
    double denominator = 0.0;

    for (const auto& dp : *this) {
        if (std::isnan(dp.c) || dp.c == T(0)) continue; // avoid log(0) or log(NaN)
        numerator += std::log(std::fabs(dp.c)) * static_cast<double>(dp.t);
        denominator += std::pow(static_cast<double>(dp.t), 2);
    }

    if (denominator == 0.0) return 0.0;
    return -numerator / denominator;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::getcummulative() const {
    TimeSeries<T> result;
    result.reserve(this->size());

    if (this->empty()) return result;

    result.addPoint(this->front().t, T{ 0 });
    for (size_t i = 1; i < this->size(); ++i) {
        const T dt = (*this)[i].t - (*this)[i - 1].t;
        const T trapezoid = T(0.5) * dt * ((*this)[i].c + (*this)[i - 1].c);
        result.addPoint((*this)[i].t, result.back().c + trapezoid);
    }

    result.structured_ = this->structured_;
    result.dt_ = this->dt_;
    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::GetCummulativeDistribution() const {
    std::vector<T> values;
    values.reserve(this->size());

    for (const auto& dp : *this)
        values.push_back(dp.c);

    std::sort(values.begin(), values.end());

    TimeSeries<T> result;
    result.reserve(values.size());

    for (size_t i = 0; i < values.size(); ++i) {
        T y = static_cast<T>(i + 0.5) / static_cast<T>(values.size());
        result.addPoint(values[i], y);
    }

    result.structured_ = false;
    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::Exp() const {
    TimeSeries<T> result;
    result.reserve(this->size());

    for (const auto& dp : *this)
        result.addPoint(dp.t, std::exp(dp.c), dp.d);

    result.structured_ = this->structured_;
    result.dt_ = this->dt_;
    return result;
}

template<typename T>
TimeSeries<T> TimeSeries<T>::fabs() const {
    TimeSeries<T> result;
    result.reserve(this->size());

    for (const auto& dp : *this)
        result.addPoint(dp.t, std::fabs(dp.c), dp.d);

    result.structured_ = this->structured_;
    result.dt_ = this->dt_;
    return result;
}

template<typename T>
T R2_c(const TimeSeries<T>& series1, const TimeSeries<T>& series2) {
    const size_t count = std::min(series1.size(), series2.size());
    if (count == 0) return T{};

    T sum_cov = T{};
    T sum_var1 = T{};
    T sum_var2 = T{};
    T sum1 = T{};
    T sum2 = T{};

    for (size_t i = 0; i < count; ++i) {
        const T y1 = std::fabs(series1[i].c);
        const T y2 = std::fabs(series2[i].c);

        sum_cov += y1 * y2 / count;
        sum_var1 += y1 * y1 / count;
        sum_var2 += y2 * y2 / count;
        sum1 += y1 / count;
        sum2 += y2 / count;
    }

    const T numerator = std::pow(sum_cov - sum1 * sum2, 2);
    const T denom = (sum_var1 - sum1 * sum1) * (sum_var2 - sum2 * sum2);
    return denom > T(0) ? numerator / denom : T{};
}

template<typename T>
T norm2(const TimeSeries<T>& series) {
    T sum = T{};
    for (const auto& pt : series)
        sum += pt.c * pt.c;
    return sum;
}

template<typename T>
TimeSeries<T> max(const TimeSeries<T>& series, T scalar) {
    TimeSeries<T> result = series;

    for (size_t i = 0; i < result.size(); ++i)
        result[i].c = std::max(series[i].c, scalar);

    return result;
}

template<typename T>
TimeSeries<T> operator>(const TimeSeries<T>& series1, const TimeSeries<T>& series2) {
    size_t n = std::min(series1.size(), series2.size());
    TimeSeries<T> result;
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        result.emplace_back(DataPoint<T>{
            series1[i].t,  // assumes aligned timestamps
                series1[i].c - series2[i].c,
                series1[i].d
        });
    }

    result.detectStructure();
    return result;
}

template<typename T>
bool TimeSeries<T>::resize(unsigned int newSize) {
    if (this->size() > newSize)
        return false;

    this->Base::resize(newSize, DataPoint<T>{T{}, T{}, std::nullopt});
    return true;
}

template<typename T>
int TimeSeries<T>::Capacity() const {
    return static_cast<unsigned int>(this->capacity());
}

template<typename T>
std::string TimeSeries<T>::getFilename() const {
    return filename_;
}

template<typename T>
void TimeSeries<T>::setFilename(const std::string& name) {
    filename_ = name;
}

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
        T obs = std::max(pt.c, lowlim);
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

template<typename T>
T R2(const TimeSeries<T>& modeled, const TimeSeries<T>& observed) {
    T sum_prod = 0, sum_mod = 0, sum_obs = 0, sum_mod2 = 0, sum_obs2 = 0;
    int count = 0;

    for (const auto& pt : observed) {
        if (pt.t >= modeled.mint() && pt.t <= modeled.maxt()) {
            T m = modeled.interpol(pt.t);
            T o = pt.c;
            sum_prod += m * o;
            sum_mod += m;
            sum_obs += o;
            sum_mod2 += m * m;
            sum_obs2 += o * o;
            count++;
        }
    }

    T numerator = pow(count * sum_prod - sum_mod * sum_obs, 2);
    T denominator = (count * sum_mod2 - sum_mod * sum_mod) * (count * sum_obs2 - sum_obs * sum_obs);
    return (denominator != 0) ? numerator / denominator : T{};
}

template<typename T>
T R2(const TimeSeries<T>* modeled, const TimeSeries<T>* observed) {
    return R2(*modeled, *observed);
}

template<typename T>
T NSE(const TimeSeries<T>& modeled, const TimeSeries<T>& observed) {
    T avg = observed.mean();
    T numerator = 0, denominator = 0;

    for (const auto& pt : observed) {
        if (pt.t >= modeled.mint() && pt.t <= modeled.maxt()) {
            T err = pt.c - modeled.interpol(pt.t);
            numerator += err * err;
            denominator += (pt.c - avg) * (pt.c - avg);
        }
    }

    return (denominator != 0) ? (1 - numerator / denominator) : T{};
}

template<typename T>
T NSE(const TimeSeries<T>* modeled, const TimeSeries<T>* observed) {
    return NSE(*modeled, *observed);
}

template<typename T>
T R(const TimeSeries<T>& modeled, const TimeSeries<T>& observed, int skipFirstN) {
    if (observed.size() <= static_cast<size_t>(skipFirstN)) return T{};

    T sum_cov = 0, sum_mod = 0, sum_obs = 0, sum_mod2 = 0, sum_obs2 = 0;
    int n = observed.size() - skipFirstN;

    for (size_t i = skipFirstN; i < observed.size(); ++i) {
        T m = modeled.getValue(i);
        T o = observed.getValue(i);
        sum_cov += m * o;
        sum_mod += m;
        sum_obs += o;
        sum_mod2 += m * m;
        sum_obs2 += o * o;
    }

    T cov = (sum_cov / n) - (sum_mod / n) * (sum_obs / n);
    T var_m = (sum_mod2 / n) - pow(sum_mod / n, 2);
    T var_o = (sum_obs2 / n) - pow(sum_obs / n, 2);

    return (var_m > 0 && var_o > 0) ? cov / sqrt(var_m * var_o) : T{};
}

template<typename T>
T XYbar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
    T sum = 0;
    for (const auto& pt : observed)
        sum += pt.c * predicted.interpol(pt.t);
    return sum / static_cast<T>(observed.size());
}

template<typename T>
T X2bar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
    T sum = 0;
    for (const auto& pt : observed)
        sum += pt.c * pt.c;
    return sum / static_cast<T>(observed.size());
}

template<typename T>
T Y2bar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
    T sum = 0;
    for (const auto& pt : observed) {
        T yhat = predicted.interpol(pt.t);
        sum += yhat * yhat;
    }
    return sum / static_cast<T>(observed.size());
}

template<typename T>
T Xbar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
    T sum = 0;
    for (const auto& pt : observed)
        sum += pt.c;
    return sum / static_cast<T>(observed.size());
}

template<typename T>
T Ybar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed) {
    T sum = 0;
    for (const auto& pt : observed)
        sum += predicted.interpol(pt.t);
    return sum / static_cast<T>(observed.size());
}

namespace TimeSeriesMetrics {

    // Computes the x-percentile from a vector using QuickSort
    template<typename T>
    T percentile(const std::vector<T>& values, T x) {
        if (values.empty()) return T{};
        std::vector<T> sorted = QSort(values);
        size_t index = std::min(static_cast<size_t>(x * sorted.size()), sorted.size() - 1);
        return sorted[index];
    }

    // Computes multiple percentiles from a vector
    template<typename T>
    std::vector<T> percentile(const std::vector<T>& values, const std::vector<T>& fractions) {
        std::vector<T> sorted = QSort(values);
        std::vector<T> result;
        result.reserve(fractions.size());

        for (const auto& frac : fractions) {
            size_t index = std::min(static_cast<size_t>(frac * sorted.size()), sorted.size() - 1);
            result.push_back(sorted[index]);
        }

        return result;
    }

    
}

    /**
 * @brief Computes the Kolmogorov–Smirnov (KS) statistic between two time series.
 *
 * The KS statistic is the maximum absolute difference between two empirical cumulative distributions.
 *
 * @param ts1 First time series
 * @param ts2 Second time series
 * @return T The KS distance
 */
    template<class T>
    T KolmogorovSmirnov(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2) {
        TimeSeries<T> cdf1 = ts1.GetCummulativeDistribution();
        TimeSeries<T> cdf2 = ts2.GetCummulativeDistribution();

        TimeSeries<T> diff = cdf1 - cdf2;
        return std::max(std::fabs(diff.maxC()), std::fabs(diff.minC()));
    }

    /**
     * @brief Overload of KolmogorovSmirnov for pointer arguments.
     *
     * @param ts1 Pointer to the first time series
     * @param ts2 Pointer to the second time series
     * @return T The KS distance
     */
    template<class T>
    T KolmogorovSmirnov(const TimeSeries<T>* ts1, const TimeSeries<T>* ts2) {
        if (!ts1 || !ts2) return T{};

        TimeSeries<T> cdf1 = ts1->GetCummulativeDistribution();
        TimeSeries<T> cdf2 = ts2->GetCummulativeDistribution();

        TimeSeries<T> diff = cdf1 - cdf2;
        return std::max(std::fabs(diff.maxC()), std::fabs(diff.minC()));
    }


// namespace TimeSeriesMetrics





