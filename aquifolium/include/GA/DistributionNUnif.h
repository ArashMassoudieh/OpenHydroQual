
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
	bool set;
	bool symetrical;
	CDistributionNUnif(const CDistributionNUnif &D);
    CDistributionNUnif operator=(const CDistributionNUnif &D);
};

double Gammapdf(double x, double r, double lambda);
double NormalStdpdf(double x);
double calcGamma(double x);
double GetRndUniF(double xmin, double xmax);
