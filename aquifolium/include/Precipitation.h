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
#ifdef Q_JSON_SUPPORT
#include <QJsonObject>
#include <QJsonArray>
#endif

#include "TimeSeriesSet.h"



using namespace std;

class CPrecipitation
{
public:
    CPrecipitation(void);
    CPrecipitation(int n);
    CPrecipitation(string filename);
    CPrecipitation(const CPrecipitation &Precip);
    CPrecipitation operator = (const CPrecipitation &Precip);
    void append(const double &_s, const double &_e, const double &intensity);
    int n;
    vector<double> s;
    vector<double> e;
    vector<double> i;
    bool structured = false;
    double dt;
    double getval(double time);
    void getfromfile(string filename);
    string filename;

#ifdef Q_JSON_SUPPORT
    // Serialize to JSON.
    //   - If filename is non-empty: writes only {"filename": "..."} (terse,
    //     reload re-reads the file from disk).
    //   - If filename is empty:     embeds bins inline as
    //     {"bins": [{"s": ..., "e": ..., "i": ...}, ...]}
    //     Used when the precipitation was injected at runtime and has no
    //     backing file (Truth-Twin / forecast cycles).
    QJsonObject toJsonObject() const;

    // Inverse of toJsonObject. Recognizes either form. Returns false on
    // structural error; missing fields silently leave the object empty.
    bool fromJsonObject(const QJsonObject &obj);
#endif
    TimeSeriesSet<double> getflow (double A) const;
    TimeSeriesSet<double> getflow(double A, double dt);
    void writefile(string Filename);

	static bool isFileValid(string filename);
public:
	~CPrecipitation(void);
};
