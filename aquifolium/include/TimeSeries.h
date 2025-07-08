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

//#define CBTC CTimeSeries

using namespace std;

template<typename T>
class TimeSeries : public std::vector<typename TimeSeries<T>::DataPoint> {
public:
    struct DataPoint {
        T t;
        T c;
        std::optional<T> d;
    };

    using Base = std::vector<DataPoint>;

    // Constructors
    TimeSeries() = default;
    explicit TimeSeries(const Base& points) : Base(points) {}

    void addPoint(T t, T c, std::optional<T> d = std::nullopt) {
        this->emplace_back(DataPoint{t, c, d});
    }

    // Convenience accessors
    T getTime(size_t i) const { return (*this)[i].t; }
    T getValue(size_t i) const { return (*this)[i].c; }
    std::optional<T> getDistance(size_t i) const { return (*this)[i].d; }

    // Core ops
    T mean() const;
    T interpol(const T& x) const;
    T mint() const;
    T maxt() const;
};


#include "TimeSeries.hpp"
