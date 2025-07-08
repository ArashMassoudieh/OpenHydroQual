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


//#include "StdAfx.h"
#include "Vector.h"
#include "Matrix.h"
#include "NormalDist.h"
#include <cstdlib>
#include "math.h"

CNormalDist::CNormalDist(void)
{
	
}

CNormalDist::~CNormalDist(void)
{

}

double CNormalDist::unitrandom()
{
	int randmax1 = 1435;
	int randmax2 = 1533;
	double x1 = rand()%randmax1;
	double x2 = rand()%randmax2;
	int i = rand() % 2;
	if (i==1)
		return (x1+x2*(randmax1-1))/(double(randmax2)*double(randmax1-1));
	else
		return double(1) - (x1 + x2*(randmax1 - 1)) / (double(randmax2)*double(randmax1 - 1));
}

double CNormalDist::getstdnormalrand()
{
	double x3 = unitrandom();
	double signn;
	if (x3 > 0.5) signn = 1; else signn = -1;
	double x1 = unitrandom();
	double x2 = unitrandom();
	double pi = atan(1.0)*4;
	double y1 = signn*sqrt(-2*log(x1))*cos(2*pi*x2);
	return y1;
}

double CNormalDist::getnormalrand(double mu, double std)
{
	return getstdnormalrand()*std+mu;
}

double unitrandom()
{
	double x1 = rand();
	double x2 = rand();
	return (x1+x2*(double(RAND_MAX)-1))/(double(RAND_MAX)*double(RAND_MAX));
}

double getstdnormalrand()
{
	double x1 = unitrandom();
	double x2 = unitrandom();
	double pi = atan(1.0)*4;
	double y1 = sqrt(-2*log(x1))*cos(2*pi*x2);
	return y1;
}

double getnormalrand(double mu, double std)
{
	return getstdnormalrand()*std+mu;
}
CVector CNormalDist::getnormal(CVector &mu, CMatrix &sigma)
{
	CMatrix L = sigma.Cholesky_factor();
	CVector V = getnormal(L.getnumrows(),0,1);
	CVector X =  mu + (L*V);
	return X;
}

CMatrix CNormalDist::getnormal(int m, int n, double mu, double std)
{
	CMatrix M(m,n);
	for (int i=0; i<m; i++)
		for (int j=0; j<n; j++)
			M[i][j] = getstdnormalrand()*std+mu;

	return M;
}

CVector CNormalDist::getnormal(int m, double mu, double std)
{
	CVector M(m);
	for (int i=0; i<m; i++)
		M[i] = getstdnormalrand()*std+mu;

	return M;
}

double CNormalDist::getlognormalrand(double mu, double std)
{
	return exp(getstdnormalrand()*std+mu);
}

CVector CNormalDist::getlognormal(CVector &mu, CMatrix &sigma)
{
    CVector V =getnormal(mu,sigma);
    return Exp(V);
}

CMatrix CNormalDist::getlognormal(int m, int n, double mu, double std)
{
	CMatrix M(m,n);
	for (int i=0; i<m; i++)
		for (int j=0; j<n; j++)
			M[i][j] = exp(getstdnormalrand()*std+mu);

	return M;
}

CVector CNormalDist::getlognormal(int m, double mu, double std)
{
	CVector M(m);
	for (int i=0; i<m; i++)
		M[i] = exp(getstdnormalrand()*std+mu);

	return M;
}

double getpdfnormal(CVector &X, CVector &mu, CMatrix &std)
{
    int k = X.size();
	double pi = atan(1.0)*4;
	CMatrix exparg = 0.5*(X.T()-mu.T())*Invert(std)*(X-mu);

	double pdf = 1/pow(2*pi,k)/sqrt(std.det())*exp(-exparg[0][0]);
	return pdf;

}

double getpdflognormal(CVector X, CVector mu, CMatrix std)
{
    int k = X.size();
	double pi = atan(1.0)*4;
    // ARASH: LOOK HERE
    CMatrix exparg = 0.5*(Log(X)-mu.T())*Invert(std)*(Log(X)-mu);

	double pdf = 1/pow(2*pi,k)/sqrt(fabs(std.det()))*exp(-exparg[0][0]);
	return pdf;

}

double CNormalDist::likelihood_mixed(double x_mod, double x_obs, double std_ln, double std_n)
{
	int nx;
	if (fabs(x_obs-x_mod)<5*std_n)
		nx = 100;
	else
		nx = 1000;
	CVector X(-1+double(0.1)/double(nx),1-double(0.1)/double(nx),nx); 
	CVector X3 = x_obs/std_n+Log((X+1)/(1-X));
	CVector X4 = (x_obs+(-x_mod)*(1+Log((1-X)/(X+1))*(1+Log((1-X)/(X+1)))*(1+Log((1-X)/(X+1)))))/std_n;
	CVector X2 = combinesort_s(X3,X4); 
	CVector Y = 1/sqrt(atan(1.0)*4)*Exp(-0.5*(X2*X2));
	double sum = 0;
	double a1,a2;
    for (int i=0; i<X.size()-1; i++)
	{
		if (x_obs-std_n*X2[i]>0)
			a1 = 1/std_ln*exp(-0.5*pow(log((x_obs-std_n*X2[i])/x_mod),2)/(2*pow(std_ln,2)))/(x_obs-std_n*X2[i]);
		else
			a1 = 0;
		if (x_obs-std_n*X2[i+1]>0)
			a2 = 1/std_ln*exp(-0.5*pow(log((x_obs-std_n*X2[i+1])/x_mod),2)/(2*pow(std_ln,2)))/(x_obs-std_n*X2[i+1]);
		else
			a2 = 0;
		sum+=0.5*(X2[i+1]-X2[i])*(Y[i+1]*a2+Y[i]*a1);
	}
	if (sum>0)
		return log(sum);
	else
		return -1e3;
}
