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

//MM
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

	//MM
    //CBTCSet getflow_Evap(double A);
	
    TimeSeriesSet<double> getflow (double A) const;
    TimeSeriesSet<double> getflow(double A, double dt);
    void writefile(string Filename);

	static bool isFileValid(string filename);
public:
	~CPrecipitation(void);
};
