// BTC.cpp: implementation of the CTimeSeries class.
//
//////////////////////////////////////////////////////////////////////



#include "BTC.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>
//#include "StringOP.h"
#include "Utilities.h"
#include "NormalDist.h"
#ifdef Q_version
#include "qfile.h"
#include "qdatastream.h"
#include <qdebug.h>
#endif


using namespace std;


template<class T>
CTimeSeries<T>::CTimeSeries()
{
	n = 0;
	structured = true;
	max_fabs = 0;
}

template<class T>
CTimeSeries<T>::CTimeSeries(int n1)
{
	n=n1;
	t.resize(n);
	C.resize(n);
	structured = true;
	max_fabs = 0;
}

template<class T>
CTimeSeries<T>::CTimeSeries(vector<T> &data, int writeInterval)
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
template<class T>
void CTimeSeries<T>::setnumpoints(int n1)
{

	n = n1;
	t.resize(n);
	C.resize(n);
}


template<class T>
bool CTimeSeries<T>::SetT(int i, const T &value)
{
    if (i<t.size())
        t[i] = value;
    else
        return false;
}

template<class T>
bool CTimeSeries<T>::SetC(int i, const T &value)
{
    if (i<C.size())
        C[i] = value;
    else
        return false;
}

template<class T>
bool CTimeSeries<T>::SetD(int i, const T& value)
{
	if (i < D.size())
		D[i] = value;
	else
		return false;
}

template<class T>
T CTimeSeries<T>::GetT(int i) const
{
    if (i<t.size())
        return t[i];
    else
        return 0;
}

template<class T>
T CTimeSeries<T>::GetD(int i) const
{
	if (i < D.size())
		return D[i];
	else
		return 0;
}

template<class T>
T CTimeSeries<T>::GetC(int i) const
{
    if (i<C.size())
        return C[i];
    else
        return 0;
}

template<class T>
CTimeSeries<T>::~CTimeSeries()
{

}

template<class T>
CTimeSeries<T>::CTimeSeries(const CTimeSeries<T> &CC)
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

#ifdef _arma
template<class T>
CTimeSeries<T>::CTimeSeries(arma::mat &x, arma::mat &y)
{
    if (x.size()!=y.size())
        return;

    for (int i=0; i<x.size(); i++)
    {
        append(x[i],y[i]);
    }

}
#endif


template<class T>
CTimeSeries<T>::CTimeSeries(string Filename)
{
	n = 0;
	t.clear();
	C.clear();
	D.clear();
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
        if ((s[0].substr(0,2)!="//") && (aquiutils::tolower(s[0])!="names") && aquiutils::trim(s[0])!="")
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

template<class T>
CTimeSeries<T>& CTimeSeries<T>::operator = (const CTimeSeries<T> &CC)
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

template<class T>
CTimeSeries<T> CTimeSeries<T>::Log()
{
	CTimeSeries BTC = CTimeSeries(n);
	for (int i=0; i<n; i++)
	{
		BTC.SetT(i,t[i]);
		BTC.C[i] = log(C[i]);
	}
	return BTC;
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::Log(T m)
{
	CTimeSeries BTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
        BTC.C[i] = log(max(C[i],m));
	}
	return BTC;
}


template<class T>
T CTimeSeries<T>::interpol(const T &x) const
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

template<class T>
CTimeSeries<T> CTimeSeries<T>::MA_smooth(int span)
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

template<class T>
T CTimeSeries<T>::interpol_D(const T &x)
{
    T r=0;
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
            T dt = t[1]-t[0];
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

template<class T>
CTimeSeries<T> CTimeSeries<T>::interpol(vector<T> x)
{
    CTimeSeries<T> BTCout;
	for (unsigned int i=0; i<x.size(); i++)
		BTCout.append(x[i],interpol(x[i]));
	return BTCout;

}

template<class T>
CTimeSeries<T> CTimeSeries<T>::interpol(CTimeSeries<T> &x) const
{
    CTimeSeries<T> BTCout;
	for (int i=0; i<x.n; i++)
		BTCout.append(x.t[i],interpol(x.t[i]));
	return BTCout;

}

template<class T>
T ADD(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d)
{
    T sum = 0;
	for (int i=0; i<BTC_d.n; i++)
		if (abs(BTC_d.C[i]) < 1e-3)
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));
		else
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i])) /BTC_d.C[i];

	return sum/BTC_d.n;
}

template<class T>
T diff_relative(CTimeSeries<T> &BTC_A, CTimeSeries<T> &BTC_B, T m)
{
	double sum = 0;
	for (int i=0; i<min(BTC_A.n,BTC_B.n); i++)
		if (abs(BTC_A.C[i]) < m)
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i]));
		else
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i])) /BTC_A.C[i];

	return sum;
}

template<class T>
T diff(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, int scale)
{
    T sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		if (BTC_d.C[i] > BTC_p.interpol(BTC_d.t[i]))
			sum += scale*pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
		else
			sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
	}
	return sum;
}

template<class T>
T diff(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d)
{
    T sum = 0;
	double a;
    if ((BTC_p.n==0) || (BTC_d.n==0)) return sum;
    for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(BTC_d.C[i] - a,2);
	}

	return sum;
}

template<class T>
T diff_abs(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d)
{
    T sum = 0;

	for (int i=0; i<BTC_d.n; i++)
	{
		sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));
	}

	return sum;
}

template<class T>
T diff_log(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d, T lowlim)
{
    T sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(log(max(BTC_d.C[i],lowlim)) - log(max(a,lowlim)),2);

	}

	return sum;
}

template<class T>
T diff2(CTimeSeries<T> *BTC_p, CTimeSeries<T> BTC_d)
{
    T sum = 0;
    for (int i=0; i<BTC_d.n; i++)
        sum += pow(BTC_d.GetC(i) - BTC_p->interpol(BTC_d.GetT(i)),2);

    return sum/double(BTC_d.n);
}


template<class T>
T diff2(const CTimeSeries<T> &BTC_p, CTimeSeries<T> *BTC_d)
{
    T sum = 0;
    int count = 0;
    for (int i=0; i<BTC_d->n; i++)
    {
        if (BTC_d->GetT(i)>BTC_p->mint() && BTC_d->GetT(i)<BTC_p->maxt())
        {   sum += pow(BTC_d->GetC(i) - BTC_p.interpol(BTC_d->GetT(i)),2);
            count++;
        }
    }

    return sum/double(count);
}


template<class T>
T diff2(const CTimeSeries<T> &BTC_p, const CTimeSeries<T> &BTC_d)
{
    T sum = 0;
    int count = 0;
    for (int i=0; i<BTC_d.n; i++)
    {
        if (BTC_d.GetT(i)>BTC_p->mint() && BTC_d.GetT(i)<BTC_p->maxt())
        {   sum += pow(BTC_d.GetC(i) - BTC_p.interpol(BTC_d.GetT(i)),2);
            count++;
        }
    }

    return sum/double(count);
}

template<class T>
T R2(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return pow(sumcov-sum1*sum2,2)/(sumvar1-sum1*sum1)/(sumvar2-sum2*sum2);
}

template<class T>
T R(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, int nlimit)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	int N = BTC_d.n - nlimit;

	for (int i=nlimit; i<BTC_d.n; i++)
	{
        T x1 = BTC_d.C[i];
        T x2 = BTC_p.C[i];
		sumcov += x1*x2/N;
		sumvar1 += x1*x1/N;
		sumvar2 += x2*x2/N;
		sum1 += x1/N;
		sum2 += x2/N;
	}

    T R_x1x2 = (sumcov-sum1*sum2)/pow(sumvar1-sum1*sum1,0.5)/pow(sumvar2-sum2*sum2,0.5);

	return R_x1x2;
}

template<class T>
T XYbar(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumcov;
}

template<class T>
T X2bar(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar1;
}

template<class T>
T Y2bar(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar2;
}

template<class T>
T Ybar(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum2;
}

template<class T>
T Xbar(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
        T x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum1;
}

template<class T>
T diff_norm(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d)
{
    T sum = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T a;
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

template<class T>
T diff(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, CTimeSeries<T> Q)
{
    T sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)*pow(Q.interpol(BTC_d.t[i]),2);
	}
	return sum;
}

template<class T>
bool CTimeSeries<T>::readfile(string Filename)
{
    clear();
    filename = Filename;
    ifstream file(Filename);
	vector<string> s;
	if (file.good() == false)
	{
		file_not_found = true;
        return false;
	}

	if (file.good())
	while (file.eof()== false)
	{
		s = aquiutils::getline(file);
		if (s.size() > 0)
		{
			while (!aquiutils::isnumber(s[0][0]) && s[0].size()>1)
			{
				s[0] = s[0].substr(1, s[0].length() - 1);
			}
            if (s.size()>1 && s[0].substr(0, 2) != "//" && aquiutils::trim(s[0]) != "" && aquiutils::isnumber(s[0][0]))
			{
                if (aquiutils::isnumber(s[1][0]))
                {   if (n>1)
                    {   if (aquiutils::atof(s[0])<t[n-1])
                        {
                            file_not_correct = true;
                            return false;
                        }
                    }
                    t.push_back(aquiutils::atof(s[0]));
                    C.push_back(atof(s[1].c_str()));
                    n++;
                    if (t.size() > 2)
                        if (t[t.size() - 1] - t[t.size() - 2] != t[t.size() - 2] - t[t.size() - 3])
                            structured = false;

                }
            }
		}
	}
    file_not_found = false;
    return true;
    file.close();

}

template<class T>
void CTimeSeries<T>::writefile(string Filename)
{
	ofstream file(Filename);
    if (file.good())
    {
        file<< "n " << n <<", BTC size " << C.size() << std::endl;
        for (int i=0; i<n; i++)
            file << t[i] << ", " << C[i] << std::endl;
    }
	file.close();

}

template<class T>
CTimeSeries<T> operator*(T alpha, CTimeSeries<T> &CTimeSeries_T)
{
    CTimeSeries<T> S(CTimeSeries_T.n);
	for (int i=0; i<CTimeSeries_T.n; i++)
	{
		S.t[i] = CTimeSeries_T.t[i];
		S.C[i] = alpha*CTimeSeries_T.C[i];
	}

	return S;
}

template<class T>
CTimeSeries<T> operator*(CTimeSeries<T> &CTimeSeries_T, T alpha)
{
    CTimeSeries<T> S = CTimeSeries_T;
	for (int i=0; i<CTimeSeries_T.n; i++)
	{
		//S.t[i] = CTimeSeries_T.t[i];
		S.C[i] = alpha*CTimeSeries_T.C[i];
	}


	return S;
}

template<class T>
CTimeSeries<T> operator/(CTimeSeries<T> &CTimeSeries_T, T alpha)
{
    CTimeSeries<T> S = CTimeSeries_T;
    for (int i=0; i<CTimeSeries_T.n; i++)
    {
        //S.t[i] = CTimeSeries_T.t[i];
        S.C[i] = 1/alpha*CTimeSeries_T.C[i];
    }


    return S;
}

template<class T>
CTimeSeries<T> operator/(CTimeSeries<T> &BTC1, CTimeSeries<T> &BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.interpol(BTC1.t[i]);

	return S;

}

template<class T>
CTimeSeries<T> operator-(CTimeSeries<T> &BTC1, CTimeSeries<T> &BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i=0; i<BTC1.n; i++)
        S.SetC(i, BTC1.GetC(i)-BTC2.interpol(BTC1.GetT(i)));

	return S;
}

template<class T>
CTimeSeries<T> operator*(CTimeSeries<T> &BTC1, CTimeSeries<T> &BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i=0; i<BTC1.n; i++)
        S.SetC(i, BTC1.GetC(i)*BTC2.interpol(BTC1.GetT(i)));

	return S;
}

template<class T>
CTimeSeries<T> operator%(CTimeSeries<T> &BTC1, CTimeSeries<T> &BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.C[i];

	return S;
}

template<class T>
CTimeSeries<T> operator&(CTimeSeries<T> &BTC1, CTimeSeries<T> &BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]+BTC2.C[i];

	return S;


}

template<class T>
T CTimeSeries<T>::maxC() const
{
	double max = -1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]>max)
			max = C[i];
	}
	return max;
}

template<class T>
T CTimeSeries<T>::maxt() const
{
    double max = -1e32;
    for (int i=0; i<n; i++)
    {	if (t[i]>max)
            max = t[i];
    }
    return max;
}

template<class T>
T CTimeSeries<T>::mint() const
{
    double min = 1e32;
    for (int i=0; i<n; i++)
    {	if (t[i]<min)
            min = t[i];
    }
    return min;
}

template<class T>
T CTimeSeries<T>::maxfabs()
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

template<class T>
T CTimeSeries<T>::minC() const
{
	double min = 1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]<min)
			min = C[i];
	}
	return min;
}

template<class T>
T CTimeSeries<T>::std()
{
    T sum = 0;
    T m = mean();
	for (int i=0; i<n; i++)
	{
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

template<class T>
T CTimeSeries<T>::std(int nlimit)
{
    T sum = 0;
    T m = mean(nlimit);
	for (int i=nlimit; i<n; i++)
	{
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

template<class T>
T CTimeSeries<T>::mean()
{
    T sum = 0;
	for (int i=0; i<n; i++)
	{
		sum+= C[i];
	}
	if (n>0)
		return sum/n;
	else
		return 0;
}

template<class T>
T CTimeSeries<T>::integrate()
{
    T sum = 0;
	for (int i=1; i<n; i++)
	{
		sum+= (C[i]+C[i-1])/2.0*(t[i]-t[i-1]);
	}
	return sum;
}

template<class T>
T CTimeSeries<T>::variance()
{
    T sum = 0;
    T mean = average();
	for (int i = 1; i < n; i++)
	{
		sum += pow((C[i] + C[i - 1]) / 2.0 - mean, 2) * (t[i] - t[i - 1]);
	}
	return sum/(t[n-1]-t[0]);
}

template<class T>
T CTimeSeries<T>::integrate(T tt)
{
    T sum = 0;
	for (int i = 1; i<n; i++)
	{
		if (t[i]<=tt) sum += (C[i] + C[i - 1]) / 2.0*(t[i] - t[i - 1]);
	}
	return sum;
}

template<class T>
T CTimeSeries<T>::integrate(T t1, T t2)
{
    T sum=0;
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

template<class T>
int CTimeSeries<T>::lookupt(T _t)
{
	for (int i = 0; i < n - 1; i++)
		if ((t[i]<_t) && (t[i + 1]>_t))
			return i;
	return -1;
}

template<class T>
T CTimeSeries<T>::average()
{
	if (n>0)
		return integrate()/(t[n-1]-t[0]);
	else
		return 0;
}

template<class T>
T CTimeSeries<T>::average(T tt)
{
	if (n>0)
        return integrate(tt) / (std::max(tt,t[n - 1]) - t[0]);
	else
		return 0;
}

template<class T>
T CTimeSeries<T>::slope()
{
	return (C[n - 1] - C[n - 2]) / (t[n - 1] - t[n - 2]);
}


template<class T>
T CTimeSeries<T>::percentile(T x)
{
    vector<T> X = QSort(C);
	int i = int(x*X.size());
	return X[i];

}

template<class T>
T CTimeSeries<T>::percentile(T x, int limit)
{
    vector<T> C1(C.size()-limit);
	for (unsigned int i=0; i<C1.size(); i++)
		C1[i] = C[i+limit];
    vector<T> X = bubbleSort(C1);
	//vector<double> X = bubbleSort(C1);
//	vector<double> X = C1;
	int ii = int(x*double(X.size()));
	return X[ii];

}

template<class T>
T CTimeSeries<T>::mean(int limit)
{
    T sum = 0;
	for (int i=limit; i<n; i++)
		sum += C[i];
	return sum/double(n-limit);
}

template<class T>
T CTimeSeries<T>::mean_log(int limit)
{
    T sum = 0;
	for (int i=limit; i<n; i++)
		sum += log(C[i]);
	return sum/double(n-limit);
}

template<class T>
bool CTimeSeries<T>::append(T x)
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

template<class T>
bool CTimeSeries<T>::append(T tt, T xx)
{


    bool increase = false;
    if (C.size()==n-1) increase = true;
	if (D.size() != t.size())
		cout << "D size not equal to t size" << std::endl;

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

template<class T>
void CTimeSeries<T>::ResizeIfNeeded(int _increment)
{
    if (C.size()==n)
    {
        C.resize(C.size()+_increment);
        t.resize(t.size()+_increment);
        D.resize(D.size()+_increment);
    }
}

template<class T>
void CTimeSeries<T>::append(CTimeSeries<T> &CC)
{
	for (int i = 0; i<CC.n; i++) append(CC.t[i], CC.C[i]);
}

template<class T>
void CTimeSeries<T>::adjust_size()
{
    C.resize(n);
    t.resize(n);
    D.resize(n);
}

template<class T>
CTimeSeries<T>& CTimeSeries<T>::operator+=(CTimeSeries<T> &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.interpol(t[i]);
	return *this;
}

template<class T>
CTimeSeries<T>& CTimeSeries<T>::operator%=(CTimeSeries<T> &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.C[i];
	return *this;

}

template<class T>
CTimeSeries<T> operator+(CTimeSeries<T> &v1, CTimeSeries<T> &v2)
{
	return v1 += v2;
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::make_uniform(T increment)
{
    CTimeSeries<T> out;
	assign_D();
	if (t.size() >1 && C.size() > 1)
	{
		out.append(t[0], C[0]);
		for (int i = 0; i < n - 1; i++)
		{
			int i1 = int((t[i] - t[0]) / increment);
			int i2 = int((t[i + 1] - t[0]) / increment);
			for (int j = i1 + 1; j <= i2; j++)
			{
                T x = j*increment + t[0];
                T CC = (x - t[i]) / (t[i + 1] - t[i])*(C[i + 1] - C[i]) + C[i];
                T DD = (x - t[i]) / (t[i + 1] - t[i])*(D[i + 1] - D[i]) + D[i];
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

template<class T>
T CTimeSeries<T>::GetLastItemValue()
{
    return C[n-1];
}

template<class T>
T CTimeSeries<T>::GetLastItemTime()
{
    return t[n-1];
}

template<class T>
T prcntl(vector<T> C, T x)
{
    vector<T> X = QSort(C);
	int ii = int(x*double(X.size()));
	return X[ii];

}

template<class T>
vector<T> prcntl(vector<T> &C, vector<T> &x)
{
    vector<T> X = QSort(C);
    vector<T> Xout = x;
	for(unsigned int j =0; j< x.size(); j++)
	{
		int ii = int(x[j]*double(X.size()));
		Xout[j] = X[ii];
	}

	return Xout;
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::extract(T t1, T t2)
{
    CTimeSeries<T> out;
	for (int i=0; i<n; i++)
		if ((t[i]>=t1) && (t[i]<=t2))
			out.append(t[i], C[i]);

	return out;
}


template<class T>
CTimeSeries<T> CTimeSeries<T>::distribution(int n_bins, int limit)
{
    CTimeSeries<T> out(n_bins+2);

	CVector C1(C.size()-limit);
	for (int i=0; i<C1.num; i++)
		C1[i] = C[i+limit];

    T p_start = C1.min();
    T p_end = C1.max()*1.001;
    T dp = abs(p_end - p_start)/n_bins;
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

template<class T>
vector<T> CTimeSeries<T>::trend()
{
    T x_bar = mean_t();
    T y_bar = mean();
    T sum_num = 0;
    T sum_denom = 0;
	for (int i=0; i<n; i++)
	{
		sum_num+=(t[i]-x_bar)*(C[i]-y_bar);
		sum_denom+=(t[i]-x_bar)*(t[i]-x_bar);
	}
    vector<T> out(2);
	out[1] = sum_num/sum_denom;
	out[0] = y_bar-out[1]*x_bar;
	return out;

}

template<class T>
T CTimeSeries<T>::mean_t()
{
    T sum = 0;
	for (int i=0; i<n; i++)
		sum += t[i];
	return sum/double(n);

}

template<class T>
T sgn(T val) {
    return double(double(0) < val) - (val < double(0));
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::add_noise(T std, bool logd)
{
    CTimeSeries<T> X(n);
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

template<class T>
T sum_interpolate(vector<T> BTC, T t)
{
    T sum=0;
	for (unsigned int i=0; i<BTC.size(); i++)
	{
		sum+=BTC[i].interpol(t);
	}
	return sum;
}

template<class T>
void CTimeSeries<T>::assign_D()
{
	D.clear();
	for (int i = 0; i<n; i++)
	{
        T counter = 0;
		for (int j = i + 1; j<n; j++)
		{
			if (C[j] == C[i]) counter += (t[j] - t[j - 1]);
			if (C[j] != C[i])
            {
                counter += (t[j] - t[j - 1]);
                break;
            }
		}
		if (i + 1 == n && n > 1)
			counter = t[n - 1] - t[n - 2];
		else if (n == 1)
			counter = 100; 
		if (counter == 0)
			counter = t[i] - ((i > 0)? t[i - 1]:0);
		D.push_back(std::fabs(counter));
	}
}

template<class T>
void CTimeSeries<T>::clear()
{
	C.clear();
	t.clear();
	D.clear();
	n = 0;
}

template<class T>
T CTimeSeries<T>::wiggle()
{
	if (n>2)
		return 3*(std::fabs(C[n-1])*(t[n-2]-t[n-3])-std::fabs(C[n-2])*(t[n-1]-t[n-3])+std::fabs(C[n-3])*(t[n-1]-t[n-2]))/(t[n-1]-t[n-3])/max(maxfabs(),1e-7);
	else
		return 0;

}

template<class T>
T CTimeSeries<T>::wiggle_corr(int _n)
{
	if (n < _n) return 0;
    T sum=0;
    T var = 0;
    T C_m=0;
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

template<class T>
bool CTimeSeries<T>::wiggle_sl(T tol)
{
	if (n < 4) return false;
    T mean = std::fabs(C[n - 1] + C[n - 2] + C[n - 3] + C[n - 4]) / 4.0+tol/100;
    T slope1 = (C[n - 1] - C[n - 2]) / (t[n - 1] - t[n - 2])/mean;
    T slope2 = (C[n - 2] - C[n - 3]) / (t[n - 2] - t[n - 3])/mean;
    T slope3 = (C[n - 3] - C[n - 4]) / (t[n - 3] - t[n - 4])/mean;
	if (std::fabs(slope1) < tol && std::fabs(slope2) < tol && std::fabs(slope3) < tol) return false;
	if ((slope1*slope2 < 0) && (slope2*slope3 < 0))
		return true;
	else
		return false;
}

template<class T>
void CTimeSeries<T>::knock_out(T tt)
{
    int eliminate_from_here=0;
    if (t.size()>0)
        while (t[eliminate_from_here]<=tt && eliminate_from_here<n) eliminate_from_here++;
    else
        eliminate_from_here=0;
    t.resize(eliminate_from_here);
    C.resize(eliminate_from_here);
    D.resize(eliminate_from_here);
    n = eliminate_from_here;

}

template<class T>
T CTimeSeries<T>::AutoCor1(int k)
{
	if (k == 0) k = n;
    T sum_product = 0;
    T sum_sq = 0;
    T mean1 = mean();
	for (int i = n - k; i < n - 1; i++)
	{
		sum_product += (C[i] - mean1)*(C[i + 1] - mean1);
		sum_sq += (C[i] - mean1);
	}
	return sum_product / sum_sq;

}

template<class T>
CTimeSeries<T> CTimeSeries<T>::getcummulative()
{
	CTimeSeries X(n);
	X.t = t;
	X.C[0] = 0;
	for (int i = 1; i<n; i++)
		X.C[i] = X.C[i - 1] + (X.t[i] - X.t[i - 1])*0.5*(C[i] + C[i - 1]);

	return X;
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::Exp()
{
    CTimeSeries<T> BTC(n);
	for (int i = 0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = exp(C[i]);
	}
	return BTC;
}

template<class T>
CTimeSeries<T> CTimeSeries<T>::fabs()
{
    CTimeSeries<T> BTC = CTimeSeries<T>(n);
	for (int i = 0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = std::fabs(C[i]);
	}
	return BTC;
}

template<class T>
T R2_c(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d)
{
    T sumcov = 0;
    T sumvar1 = 0;
    T sumvar2 = 0;
    T sum1 = 0;
    T sum2 = 0;
    T totcount = min(BTC_d.n, BTC_p.n);
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

template<class T>
T norm2(CTimeSeries<T> BTC1)
{
    T sum = 0;
	for (int i = 0; i<BTC1.n; i++)
        sum += pow(BTC1.GetC(i), 2);

	return sum;
}

template<class T>
CTimeSeries<T> max(CTimeSeries<T> A, T b)
{
    CTimeSeries<T> S = A;
	for (int i = 0; i<A.n; i++)
        S.C[i] = max(A.GetC(i), b);
	return S;
}

template<class T>
CTimeSeries<T> operator>(CTimeSeries<T> BTC1, CTimeSeries<T> BTC2)
{
    CTimeSeries<T> S = BTC1;
	for (int i = 0; i<min(BTC1.n, BTC2.n); i++)
		S.C[i] = BTC1.C[i] - BTC2.C[i];

	return S;
}
#ifdef QT_version
template<class T>
void CTimeSeries<T>::compact(QDataStream &data) const
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

template<class T>
bool CTimeSeries<T>::resize(unsigned int _size)
{
    if (C.size()>_size) return false;
    C.resize(_size);
    t.resize(_size);
    D.resize(_size);
    return true;
}

template<class T>
unsigned int CTimeSeries<T>::Capacity()
{
    return C.size();
}

template<class T>
CTimeSeries<T>::CTimeSeries(T a, T b, const vector<T> &x)
{
    int n = x.size();
    vector<T> y(n);
    for (int i = 0; i < n; i++)
        y[i] = a + b*x[i];
    *this = CTimeSeries<T>(x,y);
}
template<class T>
CTimeSeries<T>::CTimeSeries(T a, T b, const CTimeSeries<T> &btc)
{
    CTimeSeries<T>(a, b, btc.t);
}

template<class T>
CTimeSeries<T>::CTimeSeries(const vector<T> &t, const vector<T> &C)
{
    if (t.size() != C.size()) return;
    n = t.size();
    structured = true;
    this->t = t;
    this->C = C;
    if (n > 2) for (int i = 2; i < n; i++)
        if ((t[i] - t[i - 1]) != (t[i - 1] - t[i - 2]))structured = false;
}

template<class T>
T &CTimeSeries<T>::lastD()
{
    return D[n-1];
}

template<class T>
T &CTimeSeries<T>::lastC()
{
    return C[n-1];
}

template<class T>
T &CTimeSeries<T>::lastt()
{
    return t[n-1];
}

