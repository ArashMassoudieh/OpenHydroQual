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
