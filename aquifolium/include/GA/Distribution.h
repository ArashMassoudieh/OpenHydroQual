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

class CDistribution  
{
public:
	CDistribution();
	virtual ~CDistribution();
	int n;
	vector<double> s;
	vector<double> e;
    CDistribution(int nn);
	CDistribution(const CDistribution &C);
    CDistribution operator = (const CDistribution &C);
    int GetRand();

};

