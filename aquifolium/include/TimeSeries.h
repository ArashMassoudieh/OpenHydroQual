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



#pragma once

#include <map>
#include <string>
#include <vector>
#include <optional>
#include "QuickSort.h"
#include "NormalDist.h"

//GUI
#ifdef QT_version
#include "qlist.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_version

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <QJsonObject>

template<typename T>
struct DataPoint {
    T t;
    T c;
    std::optional<T> d;
};

template<typename T>
class TimeSeries : public std::vector<DataPoint<T>> {
public:


    using Base = std::vector<DataPoint<T>>;

    // Constructors
    TimeSeries();                                      // Default
    TimeSeries(size_t n);                              // Zero elements
    TimeSeries(const Base& points);                    // From vector<DataPoint>
    TimeSeries(const std::vector<T>& t, const std::vector<T>& c); // From t and c vectors
    TimeSeries(const TimeSeries<T>& other);            // Copy constructor
    TimeSeries(TimeSeries<T>&& other) noexcept;        // Move constructor
    TimeSeries(const std::vector<T>& data, int interval); // Copy constructor with interval
    explicit TimeSeries(const std::string& filename);

#ifdef _arma
    TimeSeries(const arma::mat& x, const arma::mat& y);
#endif


    // Assignment operators
    TimeSeries<T>& operator=(const TimeSeries<T>& other);  // Copy assignment
    TimeSeries<T>& operator=(TimeSeries<T>&& other) noexcept; // Move assignment


    void addPoint(T t, T c, std::optional<T> d = std::nullopt);
    void detectStructure(); //Detects whether the time-intervals are equal or not

    // Convenience accessors
    T getTime(size_t i) const;
    T getValue(size_t i) const;
    std::optional<T> getDistance(size_t i) const;

    // Setters
    bool setTime(size_t i, const T& value);
    bool setValue(size_t i, const T& value);
    bool setDistance(size_t i, const T& value);

    void clear();

    // Core ops
    T mean() const;
    T interpol(const T& x) const;
    T interpol_D(const T& x) const;
    TimeSeries<T> interpol(const std::vector<T>& x) const;
    TimeSeries<T> interpol(const TimeSeries<T>& x) const;
    TimeSeries<T> interpol(const TimeSeries<T>* x) const;

    T mint() const;
    T maxt() const;
    T minC() const;
    T maxC() const;
    void setNumPoints(size_t n);


    // Math operations
    TimeSeries<T> log() const;
    TimeSeries<T> log(T minval) const;
    TimeSeries<T> movingAverageSmooth(int span) const;

    ~TimeSeries();

    bool fileNotFound = false;
    bool fileNotCorrect = false;

private:
    bool structured_ = false;
    T dt_ = 0; // valid only if structured_ is true
#ifdef GSL
    const gsl_rng_type* A_ = nullptr;
    gsl_rng* r_ = nullptr;
    void ensureGSLInitialized();
#endif

};

// Scalar multiplication
template<typename T>
TimeSeries<T> operator*(T alpha, const TimeSeries<T>& ts);

template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts, T alpha);

// Scalar division
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T alpha);

// Pointwise division by interpolated values
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise subtraction with interpolation
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise product with interpolation
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// TimeSeries minus scalar
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts, T scalar);

// TimeSeries divided by scalar (redundant, already declared but consistent)
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T scalar);

// Pointwise division without interpolation (assumes same time stamps)
template<typename T>
TimeSeries<T> operator%(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise addition without interpolation (assumes same time stamps)
template<typename T>
TimeSeries<T> operator&(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);


namespace TimeSeriesMetrics {
    // free functions
    template<typename T>
    T ADD(const TimeSeries<T>& modeled, const TimeSeries<T>& observed); // Sum of Absolute differences

    // Relative absolute difference (with threshold for small denominators)
    template<typename T>
    T diff_relative(const TimeSeries<T>& A, const TimeSeries<T>& B, T threshold);

    // Weighted RMSE-style difference with asymmetry (scaled)
    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, double scale);

    // Standard squared error sum (unscaled)
    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    // Sum of absolute differences
    template<typename T>
    T diff_abs(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    // Log-transformed squared difference (with min threshold)
    template<typename T>
    T diff_log(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, T lowlim);

    // RMSE-style difference: predicted as pointer, observed by value
    template<typename T>
    T diff2(const TimeSeries<T>* predicted, const TimeSeries<T>& observed);

    // RMSE-style difference: predicted by value, observed as pointer
    template<typename T>
    T diff2(const TimeSeries<T>& predicted, const TimeSeries<T>* observed);

    // RMSE-style difference: both passed by const reference
    template<typename T>
    T diff2(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    // Coefficient of Determination (R²)
    template<typename T>
    T R2(const TimeSeries<T>& modeled, const TimeSeries<T>& observed);

    template<typename T>
    T R2(const TimeSeries<T>* modeled, const TimeSeries<T>* observed);

    // Nash-Sutcliffe Efficiency
    template<typename T>
    T NSE(const TimeSeries<T>& modeled, const TimeSeries<T>& observed);

    template<typename T>
    T NSE(const TimeSeries<T>* modeled, const TimeSeries<T>* observed);

    // Correlation-like statistic with truncation
    template<typename T>
    T R(const TimeSeries<T>& modeled, const TimeSeries<T>& observed, int skipFirstN);

    // Component metrics for R² or regression analysis
    template<typename T>
    T XYbar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    template<typename T>
    T X2bar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    template<typename T>
    T Y2bar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    template<typename T>
    T Xbar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    template<typename T>
    T Ybar(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    // Normalized squared error between predicted and observed time series.
    // Denominator is the geometric mean of variance estimates.
    template<typename T>
    T diff_norm(const TimeSeries<T>& predicted, const TimeSeries<T>& observed);

    // Weighted squared error using a third time series Q as the weight function.
    template<typename T>
    T diff(const TimeSeries<T>& predicted, const TimeSeries<T>& observed, const TimeSeries<T>& weights);


} // namespace TimeSeriesMetrics


#include "TimeSeries.hpp"
