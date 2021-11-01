// BTC.cpp: implementation of the CTimeSeries class.
//
//////////////////////////////////////////////////////////////////////

#include "BTC.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>
//#include "StringOP.h"
#include "Expression.h"
#include "NormalDist.h"
#ifdef Q_version
#include "qfile.h"
#include "qdatastream.h"
#include <qdebug.h>
#endif


using namespace std;


CTimeSeries::CTimeSeries()
{
	n = 0;
	structured = true;
	max_fabs = 0;
}

CTimeSeries::CTimeSeries(int n1)
{
	n=n1;
	t.resize(n);
	C.resize(n);
	structured = true;
	max_fabs = 0;
}

CTimeSeries::CTimeSeries(vector<double> &data, int writeInterval)
{
	n = 0;
	structured = 0;
	for (unsigned int i = 0; i < data.size(); i++)
		if (i%writeInterval == 0)
		{
			n++;
			t.push_back(i);
			C.push_back(data[i]);
		}
}
void CTimeSeries::setnumpoints(int n1)
{

	n = n1;
	t.resize(n);
	C.resize(n);
}

CTimeSeries::~CTimeSeries()
{

}

CTimeSeries::CTimeSeries(const CTimeSeries &CC)
{
	n=CC.n;
	if (n > 0)
	{
		t = CC.t;
		D = CC.D;
		C = CC.C;
	}
    filename = CC.filename;
    structured = CC.structured;
	name = CC.name;
	unit = CC.unit;
	defaultUnit = CC.defaultUnit;
	unitsList = CC.unitsList;
	error = CC.error;
	file_not_found = CC.file_not_found;

}

CTimeSeries::CTimeSeries(string Filename)
{
	n = 0;
	t.clear();
	C.clear();
    filename = Filename;
	ifstream file(Filename);
	if (file.good() == false)
	{
		file_not_found = true;
		error = true;
		return;
	}

	vector<string> s;
	structured = true;
	if (file.good())
	while (file.eof()== false)
	{
		s = aquiutils::getline(file);
		if (s.size() == 1)
		{
			error = true;
//			file.close();
//			return;
		}
		if (s.size()>=2)
		if ((s[0].substr(0,2)!="//") && (aquiutils::tolower(s[0])!="names"))
		{
			t.push_back(atof(s[0].c_str()));
			C.push_back(atof(s[1].c_str()));
			n++;
			if (t.size()>2)
				if ((t[t.size()-1]-t[t.size()-2])!=(t[t.size()-2]-t[t.size()-3]))
					structured = false;

		}
	}
	error = (n == 0) ? true : false;
	file.close();
}

/*CTimeSeries CTimeSeries::operator = (const CTimeSeries &CC)
{
	n=CC.n;
	t = new double[n];
	C = new double[n];
	for (int i=0; i<n; i++)
	{
		t[i] = CC.t[i];
		C[i] = CC.C[i];
	}

	return *this;
}*/

CTimeSeries& CTimeSeries::operator = (const CTimeSeries &CC)
{
    t.clear();
    C.clear();
    D.clear();
    n=CC.n;
	if (n > 0)
	{
		t = CC.t;
		D = CC.D;
		C = CC.C;
	}
    filename = CC.filename;
	structured = CC.structured;
	name = CC.name;
	unit = CC.unit;
	defaultUnit = CC.defaultUnit;
	unitsList = CC.unitsList;
	error = CC.error;
	file_not_found = CC.file_not_found;
	return *this;
}

CTimeSeries CTimeSeries::Log()
{
	CTimeSeries BTC = CTimeSeries(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = log(C[i]);
	}
	return BTC;
}

CTimeSeries CTimeSeries::Log(double m)
{
	CTimeSeries BTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = log(max(C[i],m));
	}
	return BTC;
}


double CTimeSeries::interpol(const double &x) const
{
	if (n==0)
        return 0;
	if (n>1)
	{

		if (structured == false)
		{	for (int i=0; i<n-1; i++)
			{
				if (t[i] <= x && t[i+1] >= x)
					return (C[i+1]-C[i])/(t[i+1]-t[i])*(x-t[i]) + C[i];
			}
			if (x>t[n-1]) return C[n-1];
			if (x<t[0]) return C[0];
		}
		else
		{
			if (x < t[0]) return C[0];
			if (x > t[n - 1]) return C[n - 1];
			int i = int((x-t[0])/(t[1]-t[0]));
			if (i>=n-1) return C[n-1];
			else if (i<0) return C[0];
			else return (C[i+1]-C[i])/(t[i+1]-t[i])*(x-t[i]) + C[i];
		}
	}
	else
		return C[0];


}

CTimeSeries CTimeSeries::MA_smooth(int span)
{
	CTimeSeries out;
	for (int i = 0; i < n; i++)
	{
		double sum = 0;
		int span_1 = min(span, i);
		span_1 = min(span_1, n - i - 1);
		for (int j = i - span_1; j <= i + span_1; j++)
		{
			sum += C[j] / double(1 + 2 * span_1);
		}
		out.append(t[i], sum);
	}
	return out;
}

double CTimeSeries::interpol_D(double x)
{
	double r=0;
    if (x<t[0])
        return t[0]-x;
    if (n>1)
	{

		if (structured == false)
		{	for (int i=0; i<n-1; i++)
			{
				if (t[i] <= x && t[i+1] >= x)
                    r=max((D[i+1]-D[i])/(t[i+1]-t[i])*(x-t[i]) + D[i],t[i+1]-t[i]);
			}
			if (x>t[n-1]) r=D[n-1];
			if (x<t[0]) r=D[0];
		}
		else
		{
			double dt = t[1]-t[0];
			int i = int((x-t[0])/dt);
			if (x>=t[n-1]) r=D[n-1];
			else if (x<=t[0]) r=D[0];
            else r=max((D[i+1]-D[i])/(t[i+1]-t[i])*(x-t[i]) + D[i],t[i+1]-t[i]);
		}
	}
	else
		r = D[0];
    return r;

}

CTimeSeries CTimeSeries::interpol(vector<double> x)
{
	CTimeSeries BTCout;
	for (unsigned int i=0; i<x.size(); i++)
		BTCout.append(x[i],interpol(x[i]));
	return BTCout;

}

CTimeSeries CTimeSeries::interpol(CTimeSeries &x) const
{
	CTimeSeries BTCout;
	for (int i=0; i<x.n; i++)
		BTCout.append(x.t[i],interpol(x.t[i]));
	return BTCout;

}

double ADD(CTimeSeries &BTC_p, CTimeSeries &BTC_d)
{
	double sum = 0;
	for (int i=0; i<BTC_d.n; i++)
		if (abs(BTC_d.C[i]) < 1e-3)
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));
		else
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i])) /BTC_d.C[i];

	return sum/BTC_d.n;
}

double diff_relative(CTimeSeries &BTC_A, CTimeSeries &BTC_B, double m)
{
	double sum = 0;
	for (int i=0; i<min(BTC_A.n,BTC_B.n); i++)
		if (abs(BTC_A.C[i]) < m)
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i]));
		else
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i])) /BTC_A.C[i];

	return sum;
}


double diff(CTimeSeries BTC_p, CTimeSeries BTC_d, int scale)
{
	double sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		if (BTC_d.C[i] > BTC_p.interpol(BTC_d.t[i]))
			sum += scale*pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
		else
			sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
	}
	return sum;
}

double diff(CTimeSeries &BTC_p, CTimeSeries &BTC_d)
{
	double sum = 0;
	double a;
    if ((BTC_p.n==0) || (BTC_d.n==0)) return sum;
    for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(BTC_d.C[i] - a,2);
	}

	return sum;
}

double diff_abs(CTimeSeries &BTC_p, CTimeSeries &BTC_d)
{
	double sum = 0;

	for (int i=0; i<BTC_d.n; i++)
	{
		sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));
	}

	return sum;
}

double diff_log(CTimeSeries &BTC_p, CTimeSeries &BTC_d, double lowlim)
{
	double sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(log(max(BTC_d.C[i],lowlim)) - log(max(a,lowlim)),2);

	}

	return sum;
}

double diff_mixed(CTimeSeries &BTC_p, CTimeSeries &BTC_d, double lowlim, double std_n, double std_ln)
{

	CNormalDist ND;
	double sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += ND.likelihood_mixed(a,BTC_d.C[i],std_n,std_ln);
	}

	return sum;
}


double diff2(CTimeSeries *BTC_p, CTimeSeries BTC_d)
{
    double sum = 0;
    for (int i=0; i<BTC_d.n; i++)
        sum += pow(BTC_d.C[i] - BTC_p->interpol(BTC_d.t[i]),2);

    return sum/double(BTC_d.n);
}

double diff2(CTimeSeries BTC_p, CTimeSeries *BTC_d)
{
    double sum = 0;
    for (int i=0; i<BTC_d->n; i++)
    {
        sum += pow(BTC_d->C[i] - BTC_p.interpol(BTC_d->t[i]),2);
    }

    return sum/double(BTC_d->n);
}

double diff2(const CTimeSeries &BTC_p, const CTimeSeries &BTC_d)
{
    double sum = 0;
    for (int i=0; i<BTC_d.n; i++)
    {
        sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2);
    }

    return sum/double(BTC_d.n);
}


double R2(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return pow(sumcov-sum1*sum2,2)/(sumvar1-sum1*sum1)/(sumvar2-sum2*sum2);
}

double R(CTimeSeries BTC_p, CTimeSeries BTC_d, int nlimit)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	int N = BTC_d.n - nlimit;

	for (int i=nlimit; i<BTC_d.n; i++)
	{
		double x1 = BTC_d.C[i];
		double x2 = BTC_p.C[i];
		sumcov += x1*x2/N;
		sumvar1 += x1*x1/N;
		sumvar2 += x2*x2/N;
		sum1 += x1/N;
		sum2 += x2/N;
	}

	double R_x1x2 = (sumcov-sum1*sum2)/pow(sumvar1-sum1*sum1,0.5)/pow(sumvar2-sum2*sum2,0.5);

	return R_x1x2;
}

double XYbar(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumcov;
}

double X2bar(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar1;
}

double Y2bar(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar2;
}

double Ybar(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum2;
}

double Xbar(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum1;
}

double diff_norm(CTimeSeries &BTC_p, CTimeSeries &BTC_d)
{
	double sum = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(BTC_d.C[i] - a,2)/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += pow(a,2)/BTC_d.n;
	}
	//cout<<sum<<endl;
	return sum/sqrt(sumvar1*sumvar2);

}


double diff(CTimeSeries BTC_p, CTimeSeries BTC_d, CTimeSeries Q)
{
	double sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)*pow(Q.interpol(BTC_d.t[i]),2);
	}
	return sum;
}

void CTimeSeries::readfile(string Filename)
{
    filename = Filename;
    ifstream file(Filename);
	vector<string> s;
	if (file.good() == false)
	{
		file_not_found = true;
		return;
	}

	if (file.good())
	while (file.eof()== false)
	{
		s = aquiutils::getline(file);
		if (s.size()>0)
		if (s[0].substr(0,2)!="//")
		{

            t.push_back(aquiutils::atof(s[0]));
			C.push_back(atof(s[1].c_str()));
			n++;
			if (t.size()>2)
				if (t[t.size()-1]-t[t.size()-2]!=t[t.size()-2]-t[t.size()-3])
					structured = false;

		}
	}
    file_not_found = false;
    file.close();

}

/*void CTimeSeries::readfile(CString Filename)
{
	FILE *FILEBTC;
	FILEBTC = fopen(Filename, "r");
	if (FILEBTC == NULL)
		double e=1;
	int numpoints = 0;
	double tt, CC;
	while (feof(FILEBTC)==false)
	{
		fscanf(FILEBTC, "%lf, %lf\n", &tt, &CC);
		numpoints++;
	}
	//numpoints--;
	fclose(FILEBTC);

	n=numpoints;
	t = new double[numpoints];
	C = new double[numpoints];

	FILEBTC = fopen(Filename, "r");
	for (int i=0; i<numpoints; i++)
	{
		fscanf(FILEBTC, "%lf, %lf", &t[i], &C[i]);
	}
	fclose(FILEBTC);



}*/

void CTimeSeries::writefile(string Filename)
{
	ofstream file(Filename);
	file<< "n " << n <<", BTC size " << C.size() << std::endl;

	for (int i=0; i<n; i++)
		file << t[i] << ", " << C[i] << std::endl;

	file.close();

}

/*void CTimeSeries::writefile(CString Filename)
{
	FILE *FILEBTC;
	FILEBTC = fopen(Filename, "w");
	for (int i=0; i<n; i++)
		fprintf(FILEBTC, "%lf, %le\n", t[i], C[i]);

	fclose(FILEBTC);

}*/

/*double CTimeSeries::GetS0(CTimeSeries &M)
{
	double sumprod = 0;
	double sumsqr = 0;
	for (int i = 0; i<M.n; i++)
	{
		sumprod += M.C[i]*interpol(M.t[i]);
		sumsqr += interpol(M.t[i])*interpol(M.t[i]);
	}
	double S0 = sumprod/sumsqr;
	return S0;
}

double CTimeSeries::GetS0(CTimeSeries &M, CTimeSeries &Q)
{
	double sumprod = 0;
	double sumsqr = 0;
	for (int i = 0; i<M.n; i++)
	{
		sumprod += M.C[i]*interpol(M.t[i])*pow(Q.interpol(M.t[i]),2);
		sumsqr += interpol(M.t[i])*interpol(M.t[i])*pow(Q.interpol(M.t[i]),2);
	}
	double S0 = sumprod/sumsqr;
	return S0;
}*/

CTimeSeries operator*(double alpha, CTimeSeries &CTimeSeries_T)
{
	CTimeSeries S(CTimeSeries_T.n);
	for (int i=0; i<CTimeSeries_T.n; i++)
	{
		S.t[i] = CTimeSeries_T.t[i];
		S.C[i] = alpha*CTimeSeries_T.C[i];
	}

	return S;
}

CTimeSeries operator*(CTimeSeries &CTimeSeries_T, double alpha)
{
	CTimeSeries S = CTimeSeries_T;
	for (int i=0; i<CTimeSeries_T.n; i++)
	{
		//S.t[i] = CTimeSeries_T.t[i];
		S.C[i] = alpha*CTimeSeries_T.C[i];
	}


	return S;
}

CTimeSeries operator/(CTimeSeries &CTimeSeries_T, double alpha)
{
    CTimeSeries S = CTimeSeries_T;
    for (int i=0; i<CTimeSeries_T.n; i++)
    {
        //S.t[i] = CTimeSeries_T.t[i];
        S.C[i] = 1/alpha*CTimeSeries_T.C[i];
    }


    return S;
}

CTimeSeries operator/(CTimeSeries &BTC1, CTimeSeries &BTC2)
{
	CTimeSeries S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.interpol(BTC1.t[i]);

	return S;

}

CTimeSeries operator-(CTimeSeries &BTC1, CTimeSeries &BTC2)
{
	CTimeSeries S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]-BTC2.interpol(BTC1.t[i]);

	return S;
}


CTimeSeries operator*(CTimeSeries &BTC1, CTimeSeries &BTC2)
{
	CTimeSeries S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]*BTC2.interpol(BTC1.t[i]);

	return S;
}

CTimeSeries operator%(CTimeSeries &BTC1, CTimeSeries &BTC2)
{
	CTimeSeries S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.C[i];

	return S;
}
CTimeSeries operator&(CTimeSeries &BTC1, CTimeSeries &BTC2)
{
	CTimeSeries S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]+BTC2.C[i];

	return S;


}

/*double CTimeSeries::EMC(CTimeSeries &M)
{
	double sum = 0;
	double sumflow = 0;
	for (int i=0; i<n; i++)
	{
		sum += C[i]*M.interpol(t[i]);
		sumflow += M.interpol(t[i]);
	}
	if (sumflow == 0.0)
		return 0;
	else
		return sum/sumflow;
}

double CTimeSeries::Calculate_load(CTimeSeries &M)
{
	double sum = 0;
	double sumflow = 0;
	for (int i=0; i<n; i++)
	{
		sum += C[i]*M.interpol(t[i])*(t[2]-t[1]);

	}

	return sum;
}*/

double CTimeSeries::maxC()
{
	double max = -1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]>max)
			max = C[i];
	}
	return max;
}

double CTimeSeries::maxfabs()
{
	if (max_fabs>0)
		return max_fabs;
	else
	{
		double max = -1e32;
		for (int i=0; i<n; i++)
		{	if (std::fabs(C[i])>max)
				max = std::fabs(C[i]);
		}
		return max;
	}

}

double CTimeSeries::minC()
{
	double min = 1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]<min)
			min = C[i];
	}
	return min;
}

double CTimeSeries::std()
{
	double sum = 0;
	double m = mean();
	for (int i=0; i<n; i++)
	{
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

double CTimeSeries::std(int nlimit)
{
	double sum = 0;
	double m = mean(nlimit);
	for (int i=nlimit; i<n; i++)
	{
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

double CTimeSeries::mean()
{
	double sum = 0;
	for (int i=0; i<n; i++)
	{
		sum+= C[i];
	}
	if (n>0)
		return sum/n;
	else
		return 0;
}

double CTimeSeries::integrate()
{
	double sum = 0;
	for (int i=1; i<n; i++)
	{
		sum+= (C[i]+C[i-1])/2.0*(t[i]-t[i-1]);
	}
	return sum;
}

double CTimeSeries::variance()
{
	double sum = 0;
	double mean = average(); 
	for (int i = 1; i < n; i++)
	{
		sum += pow((C[i] + C[i - 1]) / 2.0 - mean, 2) * (t[i] - t[i - 1]);
	}
	return sum/(t[n-1]-t[0]);
}

double CTimeSeries::integrate(double tt)
{
	double sum = 0;
	for (int i = 1; i<n; i++)
	{
		if (t[i]<=tt) sum += (C[i] + C[i - 1]) / 2.0*(t[i] - t[i - 1]);
	}
	return sum;
}

double CTimeSeries::integrate(double t1, double t2)
{
	double sum=0;
	if (structured)
	{
		int i1 = int(t1 - t[0]) / (t[1] - t[0]);
		int i2 = int(t1 - t[0]) / (t[1] - t[0]);

		for (int i = i1; i <= i2; i++)
			sum += C[i] / (i2+1 - i1)*(t2-t1);

	}
	else
	{
        int i1 = int(t1 - t[0]) / (t[1] - t[0]);
		int i2 = int(t1 - t[0]) / (t[1] - t[0]);

		for (int i = i1; i < i2; i++)
			sum += (C[i] + C[i+1]) * 0.5 * (t[i+1]-t[i]);


	}
	return sum;
}

int CTimeSeries::lookupt(double _t)
{
	for (int i = 0; i < n - 1; i++)
		if ((t[i]<_t) && (t[i + 1]>_t))
			return i;
	return -1;
}

double CTimeSeries::average()
{
	if (n>0)
		return integrate()/(t[n-1]-t[0]);
	else
		return 0;
}

double CTimeSeries::average(double tt)
{
	if (n>0)
		return integrate(tt) / (max(tt,t[n - 1]) - t[0]);
	else
		return 0;
}

double CTimeSeries::slope()
{
	return (C[n - 1] - C[n - 2]) / (t[n - 1] - t[n - 2]);
}



double CTimeSeries::percentile(double x)
{
	vector<double> X = QSort(C);
	int i = int(x*X.size());
	return X[i];

}

double CTimeSeries::percentile(double x, int limit)
{
	vector<double> C1(C.size()-limit);
	for (unsigned int i=0; i<C1.size(); i++)
		C1[i] = C[i+limit];
	vector<double> X = bubbleSort(C1);
	//vector<double> X = bubbleSort(C1);
//	vector<double> X = C1;
	int ii = int(x*double(X.size()));
	return X[ii];

}

double CTimeSeries::mean(int limit)
{
	double sum = 0;
	for (int i=limit; i<n; i++)
		sum += C[i];
	return sum/double(n-limit);
}

double CTimeSeries::mean_log(int limit)
{
	double sum = 0;
	for (int i=limit; i<n; i++)
		sum += log(C[i]);
	return sum/double(n-limit);
}

bool CTimeSeries::append(double x)
{

    bool increase = false;
    if (C.size()==n-1) increase = true;
    if (C.size()<n+1)
    {   t.push_back(0);
        C.push_back(x);
    }
    else
    {   C[n]=x;
        t[n]=0;
    }
	max_fabs = max(max_fabs,std::fabs(x));
    n++;
    return increase;
}

bool CTimeSeries::append(double tt, double xx)
{


    bool increase = false;
    if (C.size()==n-1) increase = true;
    if (C.size()<n+1)
    {
        t.push_back(tt);
        C.push_back(xx);
        D.push_back(0);
    }
    else
    {
        C[n]=xx;
        t[n]=tt;
    }
    n++;
    if (n>2)
		if (t[n-1]-t[n-2]!=t[n-2]-t[n-3])
			structured = false;
	max_fabs = max(max_fabs,std::fabs(xx));

    return increase;
}

void CTimeSeries::ResizeIfNeeded(int _increment)
{
    if (C.size()==n)
    {
        C.resize(C.size()+_increment);
        t.resize(t.size()+_increment);
        D.resize(D.size()+_increment);
    }
}

void CTimeSeries::append(CTimeSeries &CC)
{
	for (int i = 0; i<CC.n; i++) append(CC.t[i], CC.C[i]);
}

void CTimeSeries::adjust_size()
{
    C.resize(n);
    t.resize(n);
    D.resize(n);
}

CTimeSeries& CTimeSeries::operator+=(CTimeSeries &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.interpol(t[i]);
	return *this;
}

CTimeSeries& CTimeSeries::operator%=(CTimeSeries &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.C[i];
	return *this;

}

CTimeSeries operator+(CTimeSeries &v1, CTimeSeries &v2)
{
	return v1 += v2;
}

CTimeSeries CTimeSeries::make_uniform(double increment)
{
	CTimeSeries out;
	assign_D();
	if (t.size() >1 && C.size() > 1)
	{
		out.append(t[0], C[0]);
		out.D.push_back(0);
		for (int i = 0; i < n - 1; i++)
		{
			int i1 = int((t[i] - t[0]) / increment);
			int i2 = int((t[i + 1] - t[0]) / increment);
			for (int j = i1 + 1; j <= i2; j++)
			{
				double x = j*increment + t[0];
				double CC = (x - t[i]) / (t[i + 1] - t[i])*(C[i + 1] - C[i]) + C[i];
				double DD = (x - t[i]) / (t[i + 1] - t[i])*(D[i + 1] - D[i]) + D[i];
                if (x>out.GetLastItemTime())
                {
                    out.append(x, CC);
                    out.lastD() = DD;
                }

			}
		}
	}
	out.structured = true;

	return out;
}

double CTimeSeries::GetLastItemValue()
{
    return C[n-1];
}

double CTimeSeries::GetLastItemTime()
{
    return t[n-1];
}


double prcntl(vector<double> C, double x)
{
	vector<double> X = QSort(C);
	int ii = int(x*double(X.size()));
	return X[ii];

}

vector<double> prcntl(vector<double> C, vector<double> x)
{
	vector<double> X = QSort(C);
	vector<double> Xout = x;
	for(unsigned int j =0; j< x.size(); j++)
	{
		int ii = int(x[j]*double(X.size()));
		Xout[j] = X[ii];
	}

	return Xout;
}

CTimeSeries CTimeSeries::extract(double t1, double t2)
{
	CTimeSeries out;
	for (int i=0; i<n; i++)
		if ((t[i]>=t1) && (t[i]<=t2))
			out.append(t[i], C[i]);

	return out;
}


CTimeSeries CTimeSeries::distribution(int n_bins, int limit)
{
	CTimeSeries out(n_bins+2);

	CVector C1(C.size()-limit);
	for (int i=0; i<C1.num; i++)
		C1[i] = C[i+limit];

    double p_start = C1.min();
    double p_end = C1.max()*1.001;
	double dp = abs(p_end - p_start)/n_bins;
	if (dp == 0) return out;
	out.t[0] = p_start - dp/2;
	out.C[0] = 0;
	for (int i=0; i<n_bins+1; i++)
	{
		out.t[i+1] = out.t[i] + dp;
		out.C[i+1] = out.C[i];
	}

	for (int i=0; i<C1.num; i++)
		out.C[int((C1[i]-p_start)/dp)+1] += 1.0/C1.num/dp;

	return out;
}

vector<double> CTimeSeries::trend()
{
	double x_bar = mean_t();
	double y_bar = mean();
	double sum_num = 0;
	double sum_denom = 0;
	for (int i=0; i<n; i++)
	{
		sum_num+=(t[i]-x_bar)*(C[i]-y_bar);
		sum_denom+=(t[i]-x_bar)*(t[i]-x_bar);
	}
	vector<double> out(2);
	out[1] = sum_num/sum_denom;
	out[0] = y_bar-out[1]*x_bar;
	return out;

}

double CTimeSeries::mean_t()
{
	double sum = 0;
	for (int i=0; i<n; i++)
		sum += t[i];
	return sum/double(n);

}

int sgn(int val) {
    return (int(0) < val) - (val < int(0));
}

double sgn(double val) {
    return double(double(0) < val) - (val < double(0));
}

CTimeSeries CTimeSeries::add_noise(double std, bool logd)
{
	CTimeSeries X(n);
	for (int i=0; i<n; i++)
	{
		X.t[i] = t[i];
		if (logd==false)
			X.C[i] = C[i]+getnormalrand(0,std);
		else
			X.C[i] = C[i]*exp(getnormalrand(0,std));
	}
	return X;

}

double sum_interpolate(vector<CTimeSeries> BTC, double t)
{
	double sum=0;
	for (unsigned int i=0; i<BTC.size(); i++)
	{
		sum+=BTC[i].interpol(t);
	}
	return sum;
}


void CTimeSeries::assign_D()
{
	D.clear();
	for (int i = 0; i<n; i++)
	{
		double counter = 0;
		for (int j = i + 1; j<n; j++)
		{
			if (C[j] == C[i]) counter += (t[j] - t[j - 1]);
			if (C[j] != C[i])
            {
                counter += (t[j] - t[j - 1]);
                break;
            }
		}
		if (i + 1 == n)
			counter = t[n - 1] - t[n - 2];
		if (counter == 0)
			counter = t[i] - ((i > 0)? t[i - 1]:0);
		D.push_back(fabs(counter));
	}
}

void CTimeSeries::clear()
{
	C.clear();
	t.clear();
	D.clear();
	n = 0;
}

double CTimeSeries::wiggle()
{
	if (n>2)
		return 3*(std::fabs(C[n-1])*(t[n-2]-t[n-3])-std::fabs(C[n-2])*(t[n-1]-t[n-3])+std::fabs(C[n-3])*(t[n-1]-t[n-2]))/(t[n-1]-t[n-3])/max(maxfabs(),1e-7);
	else
		return 0;

}

double CTimeSeries::wiggle_corr(int _n)
{
	if (n < _n) return 0;
	double sum=0;
	double var = 0;
	double C_m=0;
	for (int i = 0; i < _n; i++)
	{
		C_m += C[n - i-1] / double(_n);
	}
	for (int i = 0; i < _n-1; i++)
	{
		sum += (C[n - i-1] - C_m)*(C[n - i - 2] - C_m);
	}
	for (int i = 0; i < _n ; i++)
	{
		var += pow(C[n - i-1] - C_m,2);
	}
	if (var == 0)
		return 0;
	else
		return sum / var;
}

bool CTimeSeries::wiggle_sl(double tol)
{
	if (n < 4) return false;
	double mean = std::fabs(C[n - 1] + C[n - 2] + C[n - 3] + C[n - 4]) / 4.0+tol/100;
	double slope1 = (C[n - 1] - C[n - 2]) / (t[n - 1] - t[n - 2])/mean;
	double slope2 = (C[n - 2] - C[n - 3]) / (t[n - 2] - t[n - 3])/mean;
	double slope3 = (C[n - 3] - C[n - 4]) / (t[n - 3] - t[n - 4])/mean;
	if (std::fabs(slope1) < tol && std::fabs(slope2) < tol && std::fabs(slope3) < tol) return false;
	if ((slope1*slope2 < 0) && (slope2*slope3 < 0))
		return true;
	else
		return false;
}

void CTimeSeries::knock_out(double tt)
{
    int eliminate_from_here=0;
    while (t[eliminate_from_here]<=tt) eliminate_from_here++;
    t.resize(eliminate_from_here);
    C.resize(eliminate_from_here);
    D.resize(eliminate_from_here);
    n = eliminate_from_here;

}


double CTimeSeries::AutoCor1(int k)
{
	if (k == 0) k = n;
	double sum_product = 0;
	double sum_sq = 0;
	double mean1 = mean();
	for (int i = n - k; i < n - 1; i++)
	{
		sum_product += (C[i] - mean1)*(C[i + 1] - mean1);
		sum_sq += (C[i] - mean1);
	}
	return sum_product / sum_sq;

}
/*
vector<double> CTimeSeries::trend()
{
	double x_bar = mean_t();
	double y_bar = mean();
	double sum_num = 0;
	double sum_denom = 0;
	for (int i = 0; i<n; i++)
	{
		sum_num += (t[i] - x_bar)*(C[i] - y_bar);
		sum_denom += (t[i] - x_bar)*(t[i] - x_bar);
	}
	vector<double> out(2);
	out[1] = sum_num / sum_denom;
	out[0] = y_bar - out[1] * x_bar;
	return out;

}
*/

CTimeSeries CTimeSeries::getcummulative()
{
	CTimeSeries X(n);
	X.t = t;
	X.C[0] = 0;
	for (int i = 1; i<n; i++)
		X.C[i] = X.C[i - 1] + (X.t[i] - X.t[i - 1])*0.5*(C[i] + C[i - 1]);

	return X;
}
CTimeSeries CTimeSeries::Exp()
{
	CTimeSeries BTC(n);
	for (int i = 0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = exp(C[i]);
	}
	return BTC;
}

CTimeSeries CTimeSeries::fabs()
{
	CTimeSeries BTC = CTimeSeries(n);
	for (int i = 0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = std::fabs(C[i]);
	}
	return BTC;
}

double R2_c(CTimeSeries BTC_p, CTimeSeries BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	double totcount = min(BTC_d.n, BTC_p.n);
	for (int i = 0; i<totcount; i++)
	{
		sumcov += fabs(BTC_d.C[i])*fabs(BTC_p.C[i]) / totcount;
		sumvar1 += BTC_d.C[i] * BTC_d.C[i] / totcount;
		sumvar2 += BTC_p.C[i] * BTC_p.C[i] / totcount;
		sum1 += fabs(BTC_d.C[i]) / totcount;
		sum2 += fabs(BTC_p.C[i]) / totcount;
	}

	return pow(sumcov - sum1*sum2, 2) / (sumvar1 - sum1*sum1) / (sumvar2 - sum2*sum2);
}
double norm2(CTimeSeries BTC1)
{
	double sum = 0;
	for (int i = 0; i<BTC1.n; i++)
		sum += pow(BTC1.C[i], 2);

	return sum;
}
CTimeSeries max(CTimeSeries A, double b)
{
	CTimeSeries S = A;
	for (int i = 0; i<A.n; i++)
		S.C[i] = max(A.C[i], b);
	return S;
}
CTimeSeries operator>(CTimeSeries BTC1, CTimeSeries BTC2)
{
	CTimeSeries S = BTC1;
	for (int i = 0; i<min(BTC1.n, BTC2.n); i++)
		S.C[i] = BTC1.C[i] - BTC2.C[i];

	return S;
}
#ifdef QT_version
void CTimeSeries::compact(QDataStream &data) const
{
	QMap<QString, QVariant> r;
	r.insert("n", n);
	//r.insert("t", t.size());
	//r.insert("C", C.size());
	//r.insert("D", D.size());
	r.insert("structured", structured);
	r.insert("name", QString::fromStdString(name));
	r.insert("unit", QString::fromStdString(unit));
	r.insert("defaultUnit", QString::fromStdString(defaultUnit));
	QStringList uList;
    for (int i = 0; i < (int)unitsList.size(); i++)
		uList.push_back(QString::fromStdString(unitsList[i]));
	r.insert("UnitsList", uList);
	r.insert("error", error);
	data << r;

	QList<QVariant> tList;
    for (int i = 0; i < (int)t.size(); i++)
		tList.append(t[i]);
	data << tList;

	QList<QVariant> CList;
    for (int i = 0; i < (int)C.size(); i++)
		tList.append(C[i]);
	data << CList;

	QList<QVariant> DList;
    for (unsigned int i = 0; i < D.size(); i++)
		tList.append(D[i]);
	data << DList;

	return;
}

CTimeSeries CTimeSeries::unCompact(QDataStream &data)
{
	QMap<QString, QVariant> r;
	data >> r;

	CTimeSeries b;
	b.n = r["n"].toInt();
	b.structured = r["structured"].toBool();
	b.name = r["name"].toString().toStdString();
	b.unit = r["unit"].toString().toStdString();
	b.defaultUnit = r["defaultUnit"].toString().toStdString();
	QStringList uList = r["UnitsList"].toStringList();
	for (int i = 0; i < uList.size(); i++)
		b.unitsList.push_back(uList[i].toStdString());
	b.error = r["error"].toBool();

	//int tSize, CSize, DSize;

	//tSize = r["t"];
	//CSize = r["C"];
	//DSize = r["D"];

	QList<QVariant> tList, CList, DList;

	data >> tList;
	data >> CList;
	data >> DList;


	for (int i = 0; i < tList.size(); i++)
		b.t.push_back(tList[i].toDouble());


	for (int i = 0; i < DList.size(); i++)
		b.D.push_back(DList[i].toDouble());

	for (int i = 0; i < CList.size(); i++)
		b.C.push_back(CList[i].toDouble());

	return b;
}
#endif // QT_version


bool CTimeSeries::resize(unsigned int _size)
{
    if (C.size()>_size) return false;
    C.resize(_size);
    t.resize(_size);
    D.resize(_size);
    return true;
}

unsigned int CTimeSeries::Capacity()
{
    return C.size();
}

CTimeSeries::CTimeSeries(double a, double b, const vector<double> &x)
{
    int n = x.size();
    vector<double> y(n);
    for (int i = 0; i < n; i++)
        y[i] = a + b*x[i];
    *this = CTimeSeries(x,y);
}
CTimeSeries::CTimeSeries(double a, double b, const CTimeSeries &btc)
{
    CTimeSeries(a, b, btc.t);
}

CTimeSeries::CTimeSeries(const vector<double> &t, const vector<double> &C)
{
    if (t.size() != C.size()) return;
    n = t.size();
    structured = true;
    this->t = t;
    this->C = C;
    if (n > 2) for (int i = 2; i < n; i++)
        if ((t[i] - t[i - 1]) != (t[i - 1] - t[i - 2]))structured = false;
}

double &CTimeSeries::lastD()
{
    return D[n-1];
}

double &CTimeSeries::lastC()
{
    return C[n-1];
}

double &CTimeSeries::lastt()
{
    return t[n-1];
}

