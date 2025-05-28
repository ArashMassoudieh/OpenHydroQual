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

using namespace std;

class CDistributionNUnif  
{
public:
	CDistributionNUnif();
	CDistributionNUnif(int n);
	virtual ~CDistributionNUnif();
	int n;
	vector<double> x;
	vector<double> y;
    double GetRndNorm(double mean, double std);
    double GetRndGamma();
    void initializeNormal(double dx0, double dxmult,int nint);
    void initializeGamma(double dx0, double dxmult, int nint, double r, double lambda);
	bool set = false;
	bool symetrical = false;
	CDistributionNUnif(const CDistributionNUnif &D);
    CDistributionNUnif operator=(const CDistributionNUnif &D);
};

double Gammapdf(double x, double r, double lambda);
double NormalStdpdf(double x);
double calcGamma(double x);
double GetRndUniF(double xmin, double xmax);
