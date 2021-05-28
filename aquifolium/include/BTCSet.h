#pragma once
#include "BTC.h"
#include <vector>
#include "Vector.h"

#define CBTCSet CTimeSeriesSet

class CTimeSeriesSet
{
public:
	CTimeSeriesSet(void); //default constructor
	CTimeSeriesSet(int n); //construction with number of variables (timeseries)
	CTimeSeriesSet(int numberofBTCs, int sizeofBTCvector);
	CTimeSeriesSet(const CTimeSeriesSet &BTC);
	CTimeSeriesSet(const CTimeSeries &BTC);
	CTimeSeriesSet(string filename, bool varytime);
	int nvars;
    string filename;
	vector<CTimeSeries> BTC;
	void writetofile(char outputfile[]);
	int maxnumpoints();
	CTimeSeriesSet& operator = (const CTimeSeriesSet &C);
	vector<string> names;
	bool unif;
	void writetofile(string outputfile, bool writeColumnHeaders = false);
	void writetofile(string outputfile, int writeinterval);
	vector<double> interpolate(double t);
	vector<double> interpolate(double t, int fnvars);
	void getfromfile(string filename, bool varytime);
	double maxtime();
	double mintime();
	vector<double> getrandom();
	vector<double> percentile(double x);
	vector<double> mean(int limit);
	vector<double> mean(int limit, vector<int> index);
	vector<double> std(int limit);
	vector<double> std(int limit, vector<int> index);
	CMatrix correlation(int limit, int n);
	vector<double> integrate();
	vector<double> average();
	vector<double> percentile(double x, int limit);
	vector<double> percentile(double x, int limit, vector<int> index);
	vector<double> getrandom(int burnin);
	void append(double t, vector<double> c);
	CTimeSeries add(vector<int> ii);
	CTimeSeries add_mult(vector<int> ii, vector<double> mult);
	CTimeSeries add_mult(vector<int> ii, CTimeSeriesSet &mult);
	CTimeSeries divide(int ii, int jj);
	CTimeSeriesSet make_uniform(double increment, bool assgn_d=true);
	CTimeSeriesSet getpercentiles(vector<double> percents);
	CVector out_of_limit(double limit);
	CTimeSeriesSet distribution(int n_bins, int n_columns, int limit);
	CTimeSeriesSet add_noise(vector<double> std, bool logd);
	void clear();
	vector<double> max_wiggle();
	vector<double> max_wiggle_corr(int _n = 10);
	vector<int> max_wiggle_sl(int ii, double tol);
	CTimeSeriesSet getflow (double A);
	void knockout(double t);
	int lookup(string S);
	vector<double> getrow(int a);
	void setname(int i, string name);
    void resize(unsigned int _size);
    void ResizeIfNeeded(unsigned int _increment);
    void adjust_size();
	bool file_not_found=false;
	CTimeSeries &operator[](int index);
	CTimeSeries &operator[](string BTCName);
    bool Contains(string BTCName);
#ifdef QT_version
	CTimeSeries &operator[](QString BTCName) {
		return operator[](BTCName.toStdString());}
#endif // QT_version


	CTimeSeriesSet(vector < vector<double>> &, int writeInterval = 1);
	int indexOf(const string& name) const;
	void pushBackName(string name);
	void append(const CTimeSeries &BTC, string name = "");
	CTimeSeriesSet sort(int burnOut = 0);
#ifdef QT_version
	void compact(QDataStream &data) const;
	static CTimeSeriesSet unCompact(QDataStream &data);
#endif // QT_version
	~CTimeSeriesSet(void);
};

double diff(CTimeSeriesSet B1, CTimeSeriesSet B2);
CTimeSeriesSet operator * (const CTimeSeriesSet &BTC, const double &C);
CVector norm2dif(CTimeSeriesSet &A, CTimeSeriesSet &B);
CTimeSeriesSet merge(CTimeSeriesSet A, const CTimeSeriesSet &B);
CTimeSeriesSet merge(vector<CTimeSeriesSet> &A);
CVector sum_interpolate(vector<CTimeSeriesSet> &BTC, double t);
double sum_interpolate(vector<CTimeSeriesSet> &BTC, double t, string name);
int max_n_vars(vector<CTimeSeriesSet> &BTC);
