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

 // TimeSeriesSet.h


#pragma once

#include <vector>
#include <string>
#include <optional>
#include "TimeSeries.h"

/**
 * @brief A class that represents a collection of TimeSeries objects.
 */
template<typename T>
class TimeSeriesSet : public std::vector<TimeSeries<T>> {
public:
    // Constructors
    TimeSeriesSet(); ///< Default constructor
    TimeSeriesSet(const TimeSeriesSet<T>& other); ///< Copy constructor
    TimeSeriesSet(const TimeSeries<T>& ts); ///< Construct with a single TimeSeries
    TimeSeriesSet(int num_series); ///< Construct with specified number of empty series
    TimeSeriesSet(const std::vector<std::string>& filenames); ///< Construct from multiple file inputs
    TimeSeriesSet(const std::string& filename, bool has_header = true); ///< Construct from CSV file

    TimeSeriesSet<T>& operator=(const TimeSeriesSet<T>& other); ///< Copy assignment
    TimeSeriesSet<T>& operator=(TimeSeriesSet<T>&& other) noexcept; ///< Move assignment

    // File I/O
    bool read(const std::string& filename, bool has_header = true); ///< Read from CSV
    void write(const std::string& filename, const std::string& delimiter = ",") const; ///< Write to CSV
    void appendtofile(const std::string& filename, bool include_time = true) const; ///< Append to existing CSV

    // Accessors
    TimeSeries<T>& operator[](int index); ///< Access by index
    TimeSeries<T>& operator[](const std::string& name); ///< Access by name (mutable)
    const TimeSeries<T>& operator[](int index) const; ///< Access by index (const)
    const TimeSeries<T>& operator[](const std::string& name) const; ///< Access by name (const)

    // Metadata
    void setSeriesName(int index, const std::string& name); ///< Set series name by index
    std::string getSeriesName(int index) const; ///< Get series name by index
    std::vector<std::string> getSeriesNames() const; ///< Get all series names
    void setname(int index, const std::string& name); ///< Set name on internal series object

    //Ranges
    /**
 * @brief Returns the maximum value across all time series.
 *
 * Iterates through each TimeSeries in the set and returns the largest
 * observed value (`c`) found across all of them.
 *
 * @return T Maximum value among all series.
 */
    T maxval() const;

    /**
     * @brief Returns the minimum value across all time series.
     *
     * Iterates through each TimeSeries in the set and returns the smallest
     * observed value (`c`) found across all of them.
     *
     * @return T Minimum value among all series.
     */
    T minval() const;

    /**
     * @brief Returns the minimum time value across all time series.
     *
     * @return T Minimum time (`t`) among all series.
     */
    T mintime() const;

    /**
     * @brief Returns the maximum time value across all time series.
     *
     * @return T Maximum time (`t`) among all series.
     */
    T maxtime() const;
    
    // Query
    bool Contains(const std::string& name) const; ///< Check if series exists by name
    int indexOf(const std::string& name) const; ///< Get index of named series
    int lookup(const std::string& name) const; ///< Alias for indexOf
    int maxnumpoints() const; ///< Get max number of points across all series
    size_t seriesCount() const; ///< Number of series in the set
    size_t pointCount(int series_index) const; ///< Number of points in a specific series

    // Append & Modify
    void append(const std::vector<T>& values); ///< Append a row of values
    void append(double t, const std::vector<T>& values); ///< Append a row with time
    bool append(const TimeSeries<T>& ts, const std::string& name = ""); ///< Append a named TimeSeries
    void append(const std::string& name); ///< Add empty TimeSeries with given name
    void clear(); ///< Clear all series
    void clearContents(); ///< Clear content of each series, retain structure
    void clearContentsExceptLastRow(); ///< Keep last row only
    void knockout(T t); ///< Knock out all points beyond time t
    void resize(size_t num_series); ///< Resize to given number of series
    //void ResizeIfNeeded(size_t new_size); ///< Expand size only if needed
	void removeNaNs(); ///< Remove NaN values from all series
	TimeSeriesSet<T> removeNaNs() const; ///< Return a new set with NaN values removed

    // Interpolation & Extraction
    std::vector<T> interpolate(T t) const; ///< Interpolate all series at given time
    std::vector<T> interpolate(T t, int n) const; ///< Interpolate for first n series
    TimeSeries<T> extract(int index, T t1, T t2) const; ///< Extract a subset of one series

    // Series Statistics
    std::vector<T> getrandom() const; ///< Random row of values
    std::vector<T> getrandom(int start_item) const; ///< Random row after index
    std::vector<T> getrow(int index) const; ///< Get values from specific row
    std::vector<T> percentile(T p) const; ///< Percentiles from each series
    std::vector<T> percentile(T p, int start_item) const;
    std::vector<T> percentile(T p, int start_item, const std::vector<int>& indices) const;
    std::vector<T> mean(int start_item = 0) const; ///< Means from each series
    std::vector<T> mean(int start_item, const std::vector<int>& indices) const;
    std::vector<T> standardDeviation(int start_item = 0) const; ///< Stddev from each series
    std::vector<T> standardDeviation(int start_item, const std::vector<int>& indices) const;
    std::vector<T> min(int start_item = 0) const; ///< Minimums from each series
    std::vector<T> max(int start_item = 0) const; ///< Maximums from each series
    std::vector<T> integrate() const; ///< Trapezoidal integration
    std::vector<T> average() const; ///< Time-averaged values

    // Correlation & Linear Combinations
    CMatrix correlation(int start_item, int end_item) const; ///< Correlation matrix
    TimeSeries<T> add(const std::vector<int>& indices) const; ///< Sum multiple series
    TimeSeries<T> add_mult(const std::vector<int>& indices, const std::vector<T>& weights) const; ///< Weighted sum
    TimeSeries<T> add_mult(const std::vector<int>& indices, const TimeSeriesSet<T>& other) const; ///< Elementwise multiplication with another set
    TimeSeries<T> divide(int numerator_index, int denominator_index) const; ///< Pointwise division

    // Transformations
    TimeSeriesSet<T> make_uniform(T increment, bool assign_d = true) const; ///< Make all series uniform
    TimeSeriesSet<T> getpercentiles(const std::vector<T>& fractions) const; ///< Get specified percentiles at all times
    TimeSeriesSet<T> distribution(int n_bins, int start_index, int end_index) const; ///< Density histograms
    TimeSeriesSet<T> add_noise(const std::vector<T>& stddevs, bool log_noise = false) const; ///< Add Gaussian noise
    TimeSeriesSet<T> sort(int column_index = 0) const; ///< Sort based on a column
    TimeSeriesSet<T> ConverttoNormalScore() const; ///< Convert each to normal score
    TimeSeriesSet<T> AutoCorrelation(const double& span, const double& increment) const; ///< Autocorrelation function
    TimeSeriesSet<T> cummulative() const; ///< convert to cummulative distribution
    TimeSeriesSet<T> GetCummulativeDistribution() const; ///< Extract CDF for each series
    TimeSeriesSet<T> Log() const; ///< Log-transform values

    // Wiggle Analysis
    std::vector<T> max_wiggle() const; ///< Max wiggle metric across series
    std::vector<T> max_wiggle_corr(int back_steps) const; ///< Max wiggle correlation
    std::vector<int> max_wiggle_sl(int back_steps, T tolerance) const; ///< Wiggle slope test

    // File Serialization (Qt)
#ifdef Q_JSON_SUPPORT
    QJsonObject toJson() const; ///< Convert to QJsonObject
    void fromJson(const QJsonObject& json); ///< Load from QJsonObject
#endif // Q_JSON_SUPPORT

    // Properties
    std::string filename; ///< File associated with the set
    std::string name; ///< Name of the set
    bool file_not_found = false; ///< Flag for file read failure
    bool unif = false; ///< Whether time steps are uniform

private:

    static size_t countRows(std::ifstream& file, bool has_header); // estimate the number of rows in a csv file

};

// Helper functions

/**
 * @brief Sum of absolute differences between two sets.
 */
template<class T>
T diff(TimeSeriesSet<T> A, TimeSeriesSet<T> B);

/**
 * @brief Concatenate two TimeSeriesSets (horizontally).
 */
template<class T>
TimeSeriesSet<T> merge(TimeSeriesSet<T> A, const TimeSeriesSet<T>& B);

/**
 * @brief Vertically merge multiple TimeSeriesSets.
 */
template<class T>
TimeSeriesSet<T> merge(std::vector<TimeSeriesSet<T>>& sets);

/**
 * @brief Scalar multiplication.
 */
template<class T>
TimeSeriesSet<T> operator*(const TimeSeriesSet<T>& set, const T& scalar);

/**
 * @brief Vector of L2 differences between pairs of series.
 */
template<class T>
CVector norm2dif(TimeSeriesSet<T>& A, TimeSeriesSet<T>& B);

/**
 * @brief Sum of interpolated values at a given time from multiple sets.
 */
template<class T>
CVector sum_interpolate(std::vector<TimeSeriesSet<T>>& sets, double t);

/**
 * @brief Sum interpolated values at a time for a given variable.
 */
template<class T>
T sum_interpolate(std::vector<TimeSeriesSet<T>>& sets, T t, std::string name);

/**
 * @brief Determine the maximum number of variables (series) across all sets.
 */
template<class T>
int max_n_vars(std::vector<TimeSeriesSet<T>>& sets);

#include "TimeSeriesSet.hpp"
