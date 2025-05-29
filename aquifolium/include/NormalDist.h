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
#include "Vector.h"
#include "Matrix.h"
#include "NormalDist.h"


class CNormalDist
{
public:
	CNormalDist(void);
	~CNormalDist(void);
    double unitrandom();
    double getstdnormalrand();
    double getnormalrand(double mu, double std);
    CVector getnormal(CVector &mu, CMatrix &sigma);
    CMatrix getnormal(int m, int n, double mu, double std);
    CVector getnormal(int m, double mu, double std);
    double getlognormalrand(double mu, double std);
    CVector getlognormal(CVector &mu, CMatrix &sigma);
    CMatrix getlognormal(int m, int n, double mu, double std);
    CVector getlognormal(int m, double mu, double std);
    double likelihood_mixed(double x_mod, double x_obs, double std_ln, double std_n);

};

double getnormalrand(double mu, double std);
double getstdnormalrand();
double unitrandom();
