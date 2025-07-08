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


template<typename T>
TimeSeries<T>::TimeSeries(const std::vector<DataPoint>& points)
    : data_(points) {}

template<typename T>
void TimeSeries<T>::addPoint(T t, T c, std::optional<T> d) {
    data_.push_back({t, c, d});
}

template<typename T>
void TimeSeries<T>::reserve(size_t n) {
    data_.reserve(n);
}

template<typename T>
size_t TimeSeries<T>::size() const {
    return data_.size();
}

template<typename T>
const std::vector<typename TimeSeries<T>::DataPoint>& TimeSeries<T>::points() const {
    return data_;
}

template<typename T>
const typename TimeSeries<T>::DataPoint& TimeSeries<T>::operator[](size_t idx) const {
    return data_.at(idx);
}

template<typename T>
T TimeSeries<T>::getTime(size_t i) const {
    return data_[i].t;
}

template<typename T>
T TimeSeries<T>::getValue(size_t i) const {
    return data_[i].c;
}

template<typename T>
std::optional<T> TimeSeries<T>::getDistance(size_t i) const {
    return data_[i].d;
}

