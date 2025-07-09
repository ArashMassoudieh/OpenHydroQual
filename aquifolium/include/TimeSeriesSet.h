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

#include <vector>
#include <string>
#include <map>
#include <QJsonObject>
#include "TimeSeries.h"
#include "Matrix.h"
#include "Vector.h"

#ifdef QT_version
#include <qstring.h>
#endif

 /**
  * @brief A container class representing a set of TimeSeries<T> with shared metadata.
  */
template<class T>
class TimeSeriesSet {
public:
    int nvars = 0;
    std::string name;
    std::string filename;
    std::vector<TimeSeries<T>> BTC;
    std::vector<std::string> names;
    bool unif = false;
    bool file_not_found = false;

    // Constructors
    TimeSeriesSet();
    TimeSeriesSet(int n);
    TimeSeriesSet(int nvars, int vectorLength);
    TimeSeriesSet(const TimeSeriesSet<T>& other);
    TimeSeriesSet(const TimeSeries<T>& single);
    TimeSeriesSet(std::string filename, bool varytime);
    TimeSeriesSet(std::vector<std::vector<T>>& matrix, int writeInterval = 1);

    // Assignment
    TimeSeriesSet& operator=(const TimeSeriesSet<T>& other);

    // Access
    TimeSeries<T>& operator[](int index);
    TimeSeries<T>& operator[](std::string BTCName);
    TimeSeries<T> operator[](int index) const;
    TimeSeries<T> operator[](std::string BTCName) const;
#ifdef QT_version
    TimeSeries<T>& operator[](QString BTCName);
#endif

    // File I/O
    void writetofile(std::string outputfile, bool writeColumnHeaders = false);
    void writetofile(std::string outputfile, int writeinterval);
    void writetofile(char outputfile[]);
    void appendtofile(std::string outputfile, bool skipfirstrow = false);
    bool ReadContentFromFile(std::string filename, bool varytime);
    void getfromfile(std::string filename, bool varytime);

    // Structure management
    void append(const TimeSeries<T>& series, std::string name = "");
    void append(T t, std::vector<T> c);
    void clear();
    void clearContents();
    void clearContentsExceptLastRow();
    void resize(unsigned int _size);
    void ResizeIfNeeded(unsigned int _increment);
    void adjust_size();
    void knockout(T t);

    // Statistical analysis
    std::vector<T> mean(int limit);
    std::vector<T> mean(int limit, std::vector<int> index);
    std::vector<T> std(int limit);
    std::vector<T> std(int limit, std::vector<int> index);
    std::vector<T> average();
    std::vector<T> integrate();
    std::vector<T> percentile(T x);
    std::vector<T> percentile(T x, int limit);
    std::vector<T> percentile(T x, int limit, std::vector<int> index);
    CMatrix correlation(int limit, int n);

    // Other operations
    T maxtime() const;
    T mintime() const;
    T maxval() const;
    T minval() const;
    int maxnumpoints() const;

    std::vector<T> getrandom();
    std::vector<T> getrandom(int burnin);
    std::vector<T> getrow(int a);

    int indexOf(const std::string& name) const;
    bool Contains(std::string BTCName) const;
    void pushBackName(std::string name);
    void setname(int i, std::string name);
    int lookup(std::string S);

    // Transformation and math
    TimeSeries<T> add(std::vector<int> ii);
    TimeSeries<T> add_mult(std::vector<int> ii, std::vector<T> mult);
    TimeSeries<T> add_mult(std::vector<int> ii, TimeSeriesSet<T>& mult);
    TimeSeries<T> divide(int ii, int jj);

    TimeSeriesSet<T> make_uniform(T increment, bool assign_d = true);
    TimeSeriesSet<T> getpercentiles(std::vector<T> percents);
    TimeSeriesSet<T> distribution(int n_bins, int n_columns, int limit);
    TimeSeriesSet<T> add_noise(std::vector<T> stddev, bool logd);
    CVector out_of_limit(T limit);
    TimeSeriesSet<T> ConverttoNormalScore();
    TimeSeriesSet<T> AutoCorrelation(const double& span, const double& increment);
    TimeSeriesSet<T> GetCummulativeDistribution();
    TimeSeriesSet<T> Log();
    TimeSeriesSet<T> sort(int burnOut = 0);
    std::vector<T> max_wiggle();
    std::vector<T> max_wiggle_corr(int _n = 10);
    std::vector<int> max_wiggle_sl(int ii, T tol);

    QJsonObject toJson() const;
};

// Free functions
template<class T>
T diff(TimeSeriesSet<T> B1, TimeSeriesSet<T> B2);

template<class T>
CVector norm2dif(TimeSeriesSet<T>& A, TimeSeriesSet<T>& B);

template<class T>
TimeSeriesSet<T> merge(TimeSeriesSet<T> A, TimeSeriesSet<T>& B);

template<class T>
TimeSeriesSet<T> merge(std::vector<TimeSeriesSet<T>>& A);

template<class T>
CVector sum_interpolate(std::vector<TimeSeriesSet<T>>& BTC, double t);

template<class T>
T sum_interpolate(std::vector<TimeSeriesSet<T>>& BTC, T t, std::string name);

template<class T>
int max_n_vars(std::vector<TimeSeriesSet<T>>& BTC);
