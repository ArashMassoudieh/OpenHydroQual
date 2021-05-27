// Distribution.cpp: implementation of the CDistributionNUnif class.
//
//////////////////////////////////////////////////////////////////////

#include "DistributionNUnif.h"
#include "math.h"
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDistributionNUnif::CDistributionNUnif()
{

	set = false;

}

CDistributionNUnif::CDistributionNUnif(int n_n)
{
	n = n_n;
	x.resize(n);
	y.resize(n);

	set = false;
}

CDistributionNUnif::CDistributionNUnif(const CDistributionNUnif &D)
{
	n = D.n;
	x = D.x;
	y = D.y;

	set = D.set;
}

CDistributionNUnif CDistributionNUnif::operator = (const CDistributionNUnif &D)
{
	n = D.n;
	x = D.x;
	y = D.y;
	set = D.set;

	return *this;
}

CDistributionNUnif::~CDistributionNUnif()
{

}

void CDistributionNUnif::initializeGamma(double dx0, double dxmult, int nint, double r, double lambda)
{

	x[0] = 0;
	y[0] = 0;
	double x0 = 0;
	double x1 = dx0;
	for (int i=1; i<=n; i++)
	{	y[i] = y[i-1];
		x[i] = x1;
		double dx = (x1-x0)/static_cast<int>(nint);
		for (double xi = x0+dx/2; xi<=x1; xi+=dx)
			y[i] += Gammapdf(xi,r,lambda)*dx;
		if (i==1)
		{
			x0 = dx0;
			x1 = dx0*dxmult;
		}
		else
		{
			x0 = x0*dxmult;
			x1 = x0*dxmult;
		}

	}
	set = true;
	symetrical = false;

}


void CDistributionNUnif::initializeNormal(double dx0, double dxmult,int nint)
{

	x[0] = 0;
	y[0] = 0;
	double x0 = 0;
	double x1 = dx0;
	for (int i=1; i<=n; i++)
	{	y[i] = y[i-1];
		x[i] = x1;
		double dx = (x1-x0)/static_cast<int>(nint);
		for (double xi = x0+dx/2; xi<=x1; xi+=dx)
			y[i] += NormalStdpdf(xi)*dx;
		if (i==1)
		{
			x0 = dx0;
			x1 = dx0*dxmult;
		}
		else
		{
			x0 = x0*dxmult;
			x1 = x0*dxmult;
		}

	}
	set = true;
	symetrical = true;

}

double Gammapdf(double x, double r, double lambda)
{
    return pow(lambda, r)*pow(x, r-1)*exp(-lambda*x)/calcGamma(r);
}

double NormalStdpdf(double x)
{
	double pi = atan(1.0)*4;
	return 1/sqrt(2*pi)*exp(-0.5*x*x);
}


double calcGamma(double x)
{
    int i,k,m;
    double ga,gr,r,z;
    double m_PI = atan(1.0)*4;
    static double g[] = {
        1.0,
        0.5772156649015329,
       -0.6558780715202538,
       -0.420026350340952e-1,
        0.1665386113822915,
       -0.421977345555443e-1,
       -0.9621971527877e-2,
        0.7218943246663e-2,
       -0.11651675918591e-2,
       -0.2152416741149e-3,
        0.1280502823882e-3,
       -0.201348547807e-4,
       -0.12504934821e-5,
        0.1133027232e-5,
       -0.2056338417e-6,
        0.6116095e-8,
        0.50020075e-8,
       -0.11812746e-8,
        0.1043427e-9,
        0.77823e-11,
       -0.36968e-11,
        0.51e-12,
       -0.206e-13,
       -0.54e-14,
        0.14e-14};

    if (x > 171.0) return 1e308;    // This value is an overflow flag.
    if (x == (int)x) {
        if (x > 0.0) {
            ga = 1.0;               // use factorial
            for (i=2;i<x;i++) {
               ga *= i;
            }
         }
         else
            ga = 1e308;
     }
     else {
        if (fabs(x) > 1.0) {
            z = fabs(x);
            m = (int)z;
            r = 1.0;
            for (k=1;k<=m;k++) {
                r *= (z-k);
            }
            z -= m;
        }
        else
            z = x;
        gr = g[24];
        for (k=23;k>=0;k--) {
            gr = gr*z+g[k];
        }
        ga = 1.0/(gr*z);
        if (fabs(x) > 1.0) {
            ga *= r;
            if (x < 0.0) {
                ga = -m_PI/(x*ga*sin(m_PI*x));
            }
        }
    }
    return ga;
}

double CDistributionNUnif::GetRndNorm(double mean, double std)
{
	if (set == true)
	{
		double xi = GetRndUniF(0, 1)-0.5;
		if (xi>0)
		{	for (int i=0; i<n; i++)
				if ((xi>y[i]) && (xi<y[i+1]))
					return (x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(xi-y[i]))*std + mean;
		}
		else
		{
			for (int i=0; i<n; i++)
				if ((-xi>y[i]) && (-xi<y[i+1]))
					return (-(x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(-xi-y[i])))*std + mean;
		}
	}
	else
		return -1;
    return -1;
}

double CDistributionNUnif::GetRndGamma()
{
	if (set == true)
	{
		double xi = GetRndUniF(0, 1);
		{	for (int i=0; i<n; i++)
				if ((xi>y[i]) && (xi<y[i+1]))
					return x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(xi-y[i]);
		}
	}
	else
		return -1;
    return -1;
}

double GetRndUniF(double xmin, double xmax)
{
	double a = double(rand());
	double k = double(RAND_MAX);
	return a/k*(xmax-xmin) + xmin;
}
