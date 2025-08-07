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


 // TimeSeriesSet.hpp

#pragma once

#include "TimeSeriesSet.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "DistributionNUnif.h"
#include <iomanip>
#ifdef Q_JSON_SUPPORT
#include <QFile>
#include <QTextStream>
#endif // Q_JSON_SUPPORT


// --- Constructors ---

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet() {}

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet(const TimeSeriesSet<T>& other)
    : std::vector<TimeSeries<T>>(other), name(other.name), filename(other.filename), file_not_found(other.file_not_found), unif(other.unif) {}

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet(const TimeSeries<T>& ts) {
    this->push_back(ts);
}

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet(int num_series) {
    this->resize(num_series);
}

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet(const std::vector<std::string>& filenames) {
    for (const auto& file : filenames) {
        TimeSeries<T> ts(file);
        ts.setName(file);  // set name from filename
        this->push_back(ts);
    }
}

template<typename T>
TimeSeriesSet<T>::TimeSeriesSet(const std::string& filename, bool has_header) {
    read(filename, has_header);
}

template<typename T>
TimeSeriesSet<T>& TimeSeriesSet<T>::operator=(const TimeSeriesSet<T>& other) {
    if (this != &other) {
        std::vector<TimeSeries<T>>::operator=(other);  // Copy base
        this->filename = other.filename;
        this->name = other.name;
        this->file_not_found = other.file_not_found;
        this->unif = other.unif;
    }
    return *this;
}

template<typename T>
TimeSeriesSet<T>& TimeSeriesSet<T>::operator=(TimeSeriesSet<T>&& other) noexcept {
    if (this != &other) {
        std::vector<TimeSeries<T>>::operator=(std::move(other));  // Move base
        filename = std::move(other.filename);
        name = std::move(other.name);
        file_not_found = other.file_not_found;
        unif = other.unif;
    }
    return *this;
}


// --- I/O ---

template<typename T>
bool TimeSeriesSet<T>::read(const std::string& filename, bool has_header) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;

    // Header line with names
    if (has_header && std::getline(file, line)) {

        this->clear();  // Clear any existing series
        vector<string> header_time_and_label = aquiutils::split(line,',');
        for (int i=1; i<header_time_and_label.size(); i+=2) {
            TimeSeries<T> ts;
            ts.setName(header_time_and_label[i]);
            this->emplace_back(std::move(ts));
        }
    }

    // Read values row by row
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cell;

        vector<string> all_values_and_time = aquiutils::split(line,',');
        for (int i=0; i<all_values_and_time.size(); i+=2) {
            operator[](i/2).append(atof(all_values_and_time[i].c_str()), atof(all_values_and_time[i+1].c_str()));
        }

    }

    return true;
}

template<typename T>
void TimeSeriesSet<T>::write(const std::string& filename, const std::string& delimiter) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    // Write header using TimeSeries names
    if (!this->empty()) {
        for (size_t i = 0; i < this->size(); ++i) {
            file << "t, " << (*this)[i].name();
            if (i < this->size() - 1) file << delimiter;
        }
        file << "\n";
    }

    // Determine the maximum number of rows (point count) across series
    size_t rows = 0;
    for (const auto& ts : *this) {
        rows = std::max(rows, ts.size());
    }

    // Write row-wise values
    for (size_t j = 0; j < rows; ++j) {
        for (size_t i = 0; i < this->size(); ++i) {
            const auto& ts = (*this)[i];
            if (j < ts.size()) {
                // Format only the time with 3 decimal digits
                std::ostringstream time_str;
                time_str << std::fixed << std::setprecision(3) << ts.getTime(j);
                file << time_str.str() << "," << ts.getValue(j);
            }
            else {
                file << ",";
            }
            if (i < this->size() - 1) file << delimiter;
        }
        file << "\n";
    }

    file.close();
}

// --- Manipulation ---

template<typename T>
void TimeSeriesSet<T>::clear() {
    this->std::vector<TimeSeries<T>>::clear();
}

template<typename T>
void TimeSeriesSet<T>::setname(int index, const std::string& name) {
    if (index < 0 || static_cast<size_t>(index) >= this->size()) {
        throw std::out_of_range("setname: index out of range");
    }
    (*this)[index].setName(name);
}



template<typename T>
void TimeSeriesSet<T>::append(const std::vector<T>& values) {
    if (values.size() != this->size()) {
        throw std::runtime_error("append() failed: value count (" + std::to_string(values.size()) +
            ") does not match number of time series (" + std::to_string(this->size()) + ").");
    }

    for (size_t i = 0; i < values.size(); ++i) {
        (*this)[i].append(values[i]);
    }
}

template <typename T>
void TimeSeriesSet<T>::append(const std::string& name) {
    if (this->Contains(name)) {
        throw std::runtime_error("append: Time series with name '" + name + "' already exists.");
    }

    TimeSeries<T> new_ts;
    new_ts.setName(name);
    this->push_back(new_ts);
}


template<typename T>
void TimeSeriesSet<T>::append(double t, const std::vector<T>& values) {
    if (values.size() != this->size()) {
        throw std::runtime_error("append(t, values) failed: value count (" + std::to_string(values.size()) +
            ") does not match number of time series (" + std::to_string(this->size()) + ").");
    }

    for (size_t i = 0; i < values.size(); ++i) {
        (*this)[i].append(static_cast<T>(t), values[i]);
    }
}


// --- Set/Get Series Names ---

template<typename T>
void TimeSeriesSet<T>::setSeriesName(int index, const std::string& name) {
    if (index >= 0 && index < static_cast<int>(this->size())) {
        (*this)[index].setName(name);
    }
}


template<typename T>
std::string TimeSeriesSet<T>::getSeriesName(int index) const {
    if (index >= 0 && index < static_cast<int>(this->size())) {
        return (*this)[index].name();
    }
    return "";
}

// --- Series Access ---

template<typename T>
TimeSeries<T>& TimeSeriesSet<T>::operator[](int index) {
    if (index < 0 || static_cast<size_t>(index) >= this->size())
        throw std::out_of_range("TimeSeriesSet index out of range");
    return this->at(index);
}

template<typename T>
const TimeSeries<T>& TimeSeriesSet<T>::operator[](int index) const {
    if (index < 0 || static_cast<size_t>(index) >= this->size())
        throw std::out_of_range("TimeSeriesSet index out of range");
    return this->at(index);
}

template<typename T>
TimeSeries<T>& TimeSeriesSet<T>::operator[](const std::string& name) {
    for (auto& ts : *this) {
        if (ts.name() == name)
            return ts;
    }

    throw std::out_of_range("TimeSeriesSet: Series name not found: " + name);
}

template<typename T>
const TimeSeries<T>& TimeSeriesSet<T>::operator[](const std::string& name) const {
    for (const auto& ts : *this) {
        if (ts.name() == name)
            return ts;
    }
    throw std::out_of_range("TimeSeriesSet::operator[] const: Series name not found: " + name);
}


template<typename T>
bool TimeSeriesSet<T>::Contains(const std::string& name) const {
    for (const auto& ts : *this) {
        if (ts.name() == name)
            return true;
    }
    return false;
}

template<typename T>
int TimeSeriesSet<T>::indexOf(const std::string& name) const {
    for (size_t i = 0; i < this->size(); ++i) {
        if ((*this)[i].name() == name)
            return static_cast<int>(i);
    }
    return -1;
}

template<typename T>
int TimeSeriesSet<T>::lookup(const std::string& name) const {
    return indexOf(name);
}

// --- Append and Modify
template<typename T>
void TimeSeriesSet<T>::clearContents() {
    for (TimeSeries<T>& ts : *this) {
        ts.clear();
    }
}

template <typename T>
void TimeSeriesSet<T>::clearContentsExceptLastRow() {
    for (size_t i = 0; i < this->size(); ++i) {
        TimeSeries<T>& ts = (*this)[i];
        if (ts.size() >= 1) {
            T last_t = ts.getLastTime();
            T last_c = ts.getLastValue();
            ts.clear();
            ts.append(last_t, last_c);
        }
    }
}

template<typename T>
void TimeSeriesSet<T>::knockout(T t) {
    for (TimeSeries<T>& ts : *this) {
        ts.knock_out(t);
    }
}

/*
template <typename T>
void TimeSeriesSet<T>::ResizeIfNeeded(size_t new_size) {
    for (TimeSeries<T>& ts : *this)
        ts.adjust_size(new_size);
}
*/
template <typename T>
void TimeSeriesSet<T>::removeNaNs() {
    for (size_t i = 0; i < this->size(); ++i)
        (*this)[i].removeNaNs();
}

template <typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::removeNaNs() const {
    TimeSeriesSet<T> cleaned_set;

    for (const auto& ts : *this) {
        cleaned_set.append(ts.removeNaNs());
    }

    return cleaned_set;
}

// --- Metadata ---

template<typename T>
std::vector<std::string> TimeSeriesSet<T>::getSeriesNames() const {
    std::vector<std::string> names;
    for (const auto& ts : *this) {
        names.push_back(ts.name());
    }
    return names;
}

template<typename T>
void TimeSeriesSet<T>::resize(size_t num_series) {
    std::vector<TimeSeries<T>>::resize(num_series);
}

template<typename T>
size_t TimeSeriesSet<T>::seriesCount() const {
    return this->size();
}

template<typename T>
size_t TimeSeriesSet<T>::pointCount(int series_index) const {
    if (series_index < 0 || static_cast<size_t>(series_index) >= this->size())
        throw std::out_of_range("Invalid series index in pointCount()");
    return this->at(series_index).size();
}


template<typename T>
void TimeSeriesSet<T>::appendtofile(const std::string& filename, bool include_time) const {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) return;

    size_t max_rows = maxnumpoints();

    for (size_t i = 0; i < max_rows; ++i) {
        if (include_time && !this->empty()) {
            const auto& ts0 = this->at(0);
            if (i < ts0.size())
                file << ts0[i].t;
            else
                file << "";
        }

        for (size_t j = 0; j < this->size(); ++j) {
            const auto& ts = this->at(j);
            if (j > 0 || include_time) file << ",";
            if (i < ts.size())
                file << ts[i].c;
            else
                file << "";
        }

        file << '\n';
    }

    file.close();
}

template<typename T>
int TimeSeriesSet<T>::maxnumpoints() const {
    int max_points = 0;
    for (const auto& ts : *this) {
        max_points = std::max<int>(max_points, static_cast<int>(ts.size()));
    }
    return max_points;
}


template<typename T>
bool TimeSeriesSet<T>::append(const TimeSeries<T>& ts, const std::string& name) {
    std::string assigned_name = name.empty()
        ? (ts.name().empty()
            ? "series_" + std::to_string(this->size())
            : ts.name())
        : name;

    // Check for name conflict
    for (const auto& existing_ts : *this) {
        if (existing_ts.name() == assigned_name)
            return false;
    }

    TimeSeries<T> new_ts = ts;
    new_ts.setName(assigned_name);
    this->push_back(new_ts);
    return true;
}

// Interpolation & Extraction

template <typename T>
std::vector<T> TimeSeriesSet<T>::interpolate(T t) const {
    std::vector<T> result;
    result.reserve(this->size());
    for (size_t i = 0; i < this->size(); ++i) {
        result.push_back((*this)[i].interpol(t));
    }
    return result;
}

template <typename T>
std::vector<T> TimeSeriesSet<T>::interpolate(T t, int n) const {
    if (n > static_cast<int>(this->size()))
        throw std::out_of_range("interpolate: requested count exceeds number of time series.");

    std::vector<T> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        result.push_back((*this)[i].interpol(t));
    }
    return result;
}

template <typename T>
TimeSeries<T> TimeSeriesSet<T>::extract(int index, T t1, T t2) const {
    if (index < 0 || static_cast<size_t>(index) >= this->size())
        throw std::out_of_range("extract: index out of bounds.");
    return (*this)[index].extract(t1, t2);
}

// Series Statistics

template <typename T>
std::vector<T> TimeSeriesSet<T>::getrandom() const {
    if (this->empty() || (*this)[0].empty())
        throw std::runtime_error("getrandom(): TimeSeriesSet is empty or the first series has no data.");

    int a = static_cast<int>(GetRndUniF(0, (*this)[0].size()));
    std::vector<T> result;
    result.reserve(this->size());

    for (size_t i = 0; i < this->size(); ++i)
        result.push_back((*this)[i].getValue(a));

    return result;
}

template <typename T>
std::vector<T> TimeSeriesSet<T>::getrandom(int start_item) const {
    if (this->empty() || (*this)[0].size() <= static_cast<size_t>(start_item))
        throw std::runtime_error("getrandom(start_item): Invalid burn-in or empty data.");

    int available_range = static_cast<int>((*this)[0].size()) - start_item;
    int a = static_cast<int>(GetRndUniF(0, available_range));
    std::vector<T> result;
    result.reserve(this->size());

    for (size_t i = 0; i < this->size(); ++i)
        result.push_back((*this)[i].getValue(a + start_item));

    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::getrow(int index) const {
    std::vector<T> row;
    row.reserve(this->size());

    for (size_t i = 0; i < this->size(); ++i) {
        if (index < static_cast<int>((*this)[i].size()))
            row.push_back((*this)[i].getValue(index));
        else
            row.push_back(T{}); // default value if out of range
    }

    return row;
}
template<typename T>
std::vector<T> TimeSeriesSet<T>::percentile(T p) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.percentile(p));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::percentile(T p, int start_item) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.percentile(p, start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::percentile(T p, int start_item, const std::vector<int>& indices) const {
    std::vector<T> result;
    for (int idx : indices) {
        result.push_back((*this)[idx].percentile(p, start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::mean(int start_item) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.mean(start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::mean(int start_item, const std::vector<int>& indices) const {
    std::vector<T> result;
    for (int idx : indices) {
        result.push_back((*this)[idx].mean(start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::standardDeviation(int start_item) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.stddev(start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::standardDeviation(int start_item, const std::vector<int>& indices) const {
    std::vector<T> result;
    for (int idx : indices) {
        result.push_back((*this)[idx].stddev(start_item));
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::min(int start_item) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        T val = std::numeric_limits<T>::max();
        for (size_t i = start_item; i < ts.size(); ++i)
            val = std::min(val, ts[i].c);
        result.push_back(val);
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::max(int start_item) const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        T val = std::numeric_limits<T>::lowest();
        for (size_t i = start_item; i < ts.size(); ++i)
            val = std::max(val, ts[i].c);
        result.push_back(val);
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::integrate() const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.integrate());
    }
    return result;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::average() const {
    std::vector<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.average());
    }
    return result;
}


// Correlation & Linear Combinations
template<typename T>
CMatrix TimeSeriesSet<T>::correlation(int start_item, int end_item) const {
    int n_series = static_cast<int>(this->size());
    int n_points = end_item - start_item;

    CMatrix corr(n_series, n_series);

    for (int i = 0; i < n_series; ++i) {
        for (int j = i; j < n_series; ++j) {
            T mean_i = this->at(i).mean(start_item);
            T mean_j = this->at(j).mean(start_item);

            T num = 0, denom_i = 0, denom_j = 0;
            for (int k = start_item; k < end_item; ++k) {
                T xi = this->at(i).getValue(k) - mean_i;
                T xj = this->at(j).getValue(k) - mean_j;
                num += xi * xj;
                denom_i += xi * xi;
                denom_j += xj * xj;
            }

            T r = num / std::sqrt(denom_i * denom_j);
            corr(i, j) = r;
            corr(j, i) = r;  // symmetric
        }
    }

    return corr;
}

template<typename T>
TimeSeries<T> TimeSeriesSet<T>::add(const std::vector<int>& indices) const {
    if (indices.empty()) throw std::runtime_error("add: no indices provided.");

    size_t n = this->at(indices[0]).size();
    for (int idx : indices) {
        if (this->at(idx).size() != n)
            throw std::runtime_error("add: time series lengths do not match.");
    }

    TimeSeries<T> result(n);
    for (size_t i = 0; i < n; ++i) {
        T t = this->at(indices[0]).getTime(i);
        T sum = 0;
        for (int idx : indices)
            sum += this->at(idx).getValue(i);
        result.setTime(i, t);
        result.setValue(i, sum);
    }

    return result;
}

template<typename T>
TimeSeries<T> TimeSeriesSet<T>::add_mult(const std::vector<int>& indices, const std::vector<T>& weights) const {
    if (indices.size() != weights.size())
        throw std::runtime_error("add_mult: index and weight size mismatch.");

    size_t n = this->at(indices[0]).size();
    for (int idx : indices)
        if (this->at(idx).size() != n)
            throw std::runtime_error("add_mult: time series lengths do not match.");

    TimeSeries<T> result(n);
    for (size_t i = 0; i < n; ++i) {
        T t = this->at(indices[0]).getTime(i);
        T sum = 0;
        for (size_t j = 0; j < indices.size(); ++j)
            sum += weights[j] * this->at(indices[j]).getValue(i);
        result.setTime(i, t);
        result.setValue(i, sum);
    }

    return result;
}

template<typename T>
TimeSeries<T> TimeSeriesSet<T>::add_mult(const std::vector<int>& indices, const TimeSeriesSet<T>& other) const {
    if (indices.size() > other.size())
        throw std::runtime_error("add_mult: insufficient weights in other set.");

    size_t n = this->at(indices[0]).size();
    for (int idx : indices)
        if (this->at(idx).size() != n)
            throw std::runtime_error("add_mult: time series lengths do not match.");

    TimeSeries<T> result(n);
    for (size_t i = 0; i < n; ++i) {
        T t = this->at(indices[0]).getTime(i);
        T sum = 0;
        for (size_t j = 0; j < indices.size(); ++j)
            sum += this->at(indices[j]).getValue(i) * other[j].getValue(i);
        result.setTime(i, t);
        result.setValue(i, sum);
    }

    return result;
}


template<typename T>
TimeSeries<T> TimeSeriesSet<T>::divide(int numerator_index, int denominator_index) const {
    const auto& num = this->at(numerator_index);
    const auto& denom = this->at(denominator_index);

    if (num.size() != denom.size())
        throw std::runtime_error("divide: time series lengths do not match.");

    size_t n = num.size();
    TimeSeries<T> result(n);
    for (size_t i = 0; i < n; ++i) {
        T t = num.getTime(i);
        T y = denom.getValue(i);
        result.setTime(i, t);
        result.setValue(i, (y == 0) ? T(0) : num.getValue(i) / y);
    }

    return result;
}

// Transformations
template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::make_uniform(T increment, bool assign_d) const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.make_uniform(increment, assign_d));
    }
    return result;
}

template <typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::getpercentiles(const std::vector<T>& percents) const {
    TimeSeriesSet<T> result(1 + percents.size());
    if (this->empty())
        return result;

    // Set names: first is "Mean", then percentiles
    result[0].setName("Mean");
    for (size_t j = 0; j < percents.size(); ++j) {
        std::string label = std::to_string(percents[j] * 100) + " %";
        result[j + 1].setName(label);
    }

    const size_t max_points = this->maxnumpoints();

    for (size_t i = 0; i < max_points; ++i) {
        std::vector<T> values;
        int count = 0;

        for (size_t j = 0; j < this->size(); ++j) {
            if (i < (*this)[j].size()) {
                values.push_back((*this)[j].getValue(i));
                ++count;
            }
        }

        if (count == 0) continue;

        T mean_val = std::accumulate(values.begin(), values.end(), T(0)) / static_cast<T>(count);
        std::vector<T> percentile_vals = TimeSeriesMetrics::percentile(values, percents);

        std::vector<T> row(1 + percents.size());
        row[0] = mean_val;
        for (size_t j = 0; j < percents.size(); ++j)
            row[j + 1] = percentile_vals[j];

        // Use time from the first time series (if available)
        T time = (i < (*this)[0].size()) ? (*this)[0].getTime(i) : T(i);
        result.append(time, row);
    }

    return result;
}


template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::distribution(int n_bins, int start_index, int end_index) const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        TimeSeries<T> dist_ts(n_bins + 2);
        if (ts.size() > static_cast<size_t>(start_index) && ts.size() > static_cast<size_t>(end_index)) {
            std::vector<T> segment;
            for (int i = start_index; i <= end_index && i < static_cast<int>(ts.size()); ++i) {
                segment.push_back(ts.getValue(i));
            }
            T p_start = *std::min_element(segment.begin(), segment.end());
            T p_end = *std::max_element(segment.begin(), segment.end()) * 1.001;
            T dp = std::abs(p_end - p_start) / n_bins;
            if (dp == 0) {
                result.push_back(dist_ts);
                continue;
            }

            dist_ts[0].t = p_start - dp / 2;
            dist_ts[0].c = 0;
            for (int i = 0; i < n_bins + 1; ++i) {
                dist_ts[i + 1].t = dist_ts[i].t + dp;
                dist_ts[i + 1].c = 0;
            }

            for (T value : segment) {
                int bin = int((value - p_start) / dp) + 1;
                if (bin >= 0 && bin < static_cast<int>(dist_ts.size())) {
                    dist_ts[bin].c += 1.0 / segment.size() / dp;
                }
            }
        }
        result.push_back(dist_ts);
    }
    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::add_noise(const std::vector<T>& stddevs, bool log_noise) const {
    TimeSeriesSet<T> result;
    for (size_t i = 0; i < this->size(); ++i) {
        T stddev = (i < stddevs.size()) ? stddevs[i] : 0;
        result.push_back((*this)[i].add_noise(stddev, log_noise));
    }
    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::sort(int column_index) const {
    if (column_index < 0 || static_cast<size_t>(column_index) >= this->size())
        throw std::out_of_range("sort: column index out of range");

    const TimeSeries<T>& ref_series = (*this)[column_index];
    std::vector<std::pair<T, int>> indexed_times;
    for (size_t i = 0; i < ref_series.size(); ++i) {
        indexed_times.emplace_back(ref_series[i].c, static_cast<int>(i));
    }

    std::sort(indexed_times.begin(), indexed_times.end());

    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        TimeSeries<T> sorted_ts;
        for (const auto& pair : indexed_times) {
            int idx = pair.second;
            if (static_cast<size_t>(idx) < ts.size()) {
                sorted_ts.append(ts[idx].t, ts[idx].c);
            }
        }
        result.push_back(sorted_ts);
    }

    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::ConverttoNormalScore() const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.ConvertToNormalScore());
    }
    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::AutoCorrelation(const double& span, const double& increment) const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.AutoCorrelation(span, increment));
    }
    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::GetCummulativeDistribution() const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.GetCummulativeDistribution());
    }
    return result;
}

template<typename T>
TimeSeriesSet<T> TimeSeriesSet<T>::Log() const {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : *this) {
        result.push_back(ts.log());
    }
    return result;
}

// Wiggle Analysis
template<typename T>
std::vector<T> TimeSeriesSet<T>::max_wiggle() const {
    std::vector<T> results;
    for (size_t i = 0; i < this->size(); ++i) {
        results.push_back((*this)[i].wiggle());
    }
    return results;
}

template<typename T>
std::vector<T> TimeSeriesSet<T>::max_wiggle_corr(int back_steps) const {
    std::vector<T> results;
    for (size_t i = 0; i < this->size(); ++i) {
        results.push_back((*this)[i].wiggle_corr(back_steps));
    }
    return results;
}

template<typename T>
std::vector<int> TimeSeriesSet<T>::max_wiggle_sl(int back_steps, T tolerance) const {
    std::vector<int> results;
    for (size_t i = 0; i < this->size(); ++i) {
        results.push_back((*this)[i].wiggle_sl(tolerance) ? 1 : 0);
    }
    return results;
}

// File Serialization (Qt)
#ifdef Q_JSON_SUPPORT
template<typename T>
QJsonObject TimeSeriesSet<T>::toJson() const {
    QJsonObject obj;
    QJsonArray dataArray;
    for (const TimeSeries<T>& ts : *this) {
        QJsonObject tsObj = ts.toJson();
        tsObj["name"] = QString::fromStdString(ts.name());
        dataArray.append(tsObj);
    }
    obj["series"] = dataArray;
    return obj;
}

template<typename T>
void TimeSeriesSet<T>::fromJson(const QJsonObject& json) {
    this->clear();
    QJsonArray seriesArray = json["series"].toArray();
    for (const QJsonValue& val : seriesArray) {
        QJsonObject tsObj = val.toObject();
        TimeSeries<T> ts;
        ts.fromJson(tsObj);
        if (tsObj.contains("name"))
            ts.setName(tsObj["name"].toString().toStdString());
        this->push_back(ts);
    }
}
#endif // Q_JSON_SUPPORT

//Helper functions:

template<class T>
T diff(TimeSeriesSet<T> A, TimeSeriesSet<T> B) {
    if (A.size() != B.size()) {
        throw std::invalid_argument("diff: TimeSeriesSets must have the same number of series.");
    }

    T total_diff = T{};
    for (size_t i = 0; i < A.size(); ++i) {
        if (A[i].size() != B[i].size()) {
            throw std::invalid_argument("diff: Series lengths do not match at index " + std::to_string(i));
        }

        for (size_t j = 0; j < A[i].size(); ++j) {
            total_diff += std::abs(A[i][j].c - B[i][j].c);
        }
    }

    return total_diff;
}

// Helper function: Merge two TimeSeriesSets

template<class T>
TimeSeriesSet<T> merge(TimeSeriesSet<T> A, const TimeSeriesSet<T>& B) {
    if (A.size() != B.size()) {
        throw std::invalid_argument("merge: TimeSeriesSets must have the same number of series.");
    }

    for (size_t i = 0; i < B.size(); ++i) {
        A[i].insert(A[i].end(), B[i].begin(), B[i].end());
    }

    return A;
}

template<class T>
TimeSeriesSet<T> merge(std::vector<TimeSeriesSet<T>>& sets) {
    if (sets.empty())
        return TimeSeriesSet<T>();

    TimeSeriesSet<T> merged;
    size_t num_series = sets[0].size();

    // Ensure all sets have the same number of series
    for (size_t i = 1; i < sets.size(); ++i) {
        if (sets[i].size() != num_series)
            throw std::runtime_error("All TimeSeriesSets must have the same number of series to merge.");
    }

    // Copy the series names from the first set
    for (size_t i = 0; i < num_series; ++i) {
        TimeSeries<T> combined;
        std::string name = sets[0][i].name();

        for (size_t j = 0; j < sets.size(); ++j) {
            const TimeSeries<T>& ts = sets[j][i];
            for (const auto& point : ts) {
                combined.push_back(point);
            }
        }

        combined.setName(name);
        merged.push_back(combined);
    }

    return merged;
}

template<class T>
TimeSeriesSet<T> operator*(const TimeSeriesSet<T>& set, const T& scalar) {
    TimeSeriesSet<T> result;
    for (const TimeSeries<T>& ts : set) {
        result.push_back(ts * scalar);
    }
    return result;
}

// Compute norm2 difference between aligned sets

template<class T>
CVector norm2dif(TimeSeriesSet<T>& A, TimeSeriesSet<T>& B) {
    if (A.size() != B.size()) throw std::runtime_error("norm2dif: TimeSeriesSet sizes do not match");
    CVector diff(A.size());
    for (size_t i = 0; i < A.size(); ++i) {
        diff[i] = norm2(A[i] - B[i]);
    }
    return diff;
}

// Sum of interpolated values across aligned sets at given time

template<class T>
CVector sum_interpolate(std::vector<TimeSeriesSet<T>>& sets, double t) {
    if (sets.empty()) return CVector();
    size_t n_vars = sets[0].size();
    CVector result(n_vars, 0.0);

    for (size_t i = 0; i < n_vars; ++i) {
        for (size_t j = 0; j < sets.size(); ++j) {
            result[i] += sets[j][i].interpol(static_cast<T>(t));
        }
    }
    return result;
}

// Returns the maximum value across all time series
template <class T>
T TimeSeriesSet<T>::maxval() const {
    T max_val = -1e12;
    for (size_t i = 0; i < this->size(); ++i)
        max_val = std::max((*this)[i].maxC(), max_val);
    return max_val;
}

// Returns the minimum value across all time series
template <class T>
T TimeSeriesSet<T>::minval() const {
    T min_val = 1e12;
    for (size_t i = 0; i < this->size(); ++i)
        min_val = std::min((*this)[i].minC(), min_val);
    return min_val;
}

// Returns the minimum timestamp across all time series
template <class T>
T TimeSeriesSet<T>::mintime() const {
    T min_time = 1e12;
    for (size_t i = 0; i < this->size(); ++i)
        min_time = std::min((*this)[i].mint(), min_time);
    return min_time;
}

// Returns the maximum timestamp across all time series
template <class T>
T TimeSeriesSet<T>::maxtime() const {
    T max_time = -1e12;
    for (size_t i = 0; i < this->size(); ++i)
        max_time = std::max((*this)[i].maxt(), max_time);
    return max_time;
}
