#pragma once
#include "BTC.h"
#include <vector>
#include "Vector.h"

template <class T>
class CTimeSeriesSet
{
public:
	CTimeSeriesSet(void); //default constructor
	CTimeSeriesSet(int n); //construction with number of variables (timeseries)
	CTimeSeriesSet(int numberofBTCs, int sizeofBTCvector);
    CTimeSeriesSet(const CTimeSeriesSet<T> &BTC);
    CTimeSeriesSet(const CTimeSeries<T> &BTC);
	CTimeSeriesSet(string filename, bool varytime);

	int nvars;
    string name;
    string filename;
    vector<CTimeSeries<T>> BTC;
	void writetofile(char outputfile[]);
	int maxnumpoints();
	CTimeSeriesSet& operator = (const CTimeSeriesSet &C);
	vector<string> names;
	bool unif = false;
	void writetofile(string outputfile, bool writeColumnHeaders = false);
    void appendtofile(string outputfile, bool skipfirstrow = false);
	void writetofile(string outputfile, int writeinterval);
    vector<T> interpolate(T t);
    vector<T> interpolate(T t, int fnvars);
	void getfromfile(string filename, bool varytime);
    T maxtime() const;
    T mintime() const;
    T maxval() const;
    T minval() const;
    vector<T> getrandom();
    vector<T> percentile(T x);
    vector<T> mean(int limit);
    vector<T> mean(int limit, vector<int> index);
    vector<T> std(int limit);
    vector<T> std(int limit, vector<int> index);
	CMatrix correlation(int limit, int n);
    vector<T> integrate();
    vector<T> average();
    vector<T> percentile(T x, int limit);
    vector<T> percentile(T x, int limit, vector<int> index);
    vector<T> getrandom(int burnin);
    void append(T t, vector<T> c);
    CTimeSeries<T> add(vector<int> ii);
    CTimeSeries<T> add_mult(vector<int> ii, vector<T> mult);
    CTimeSeries<T> add_mult(vector<int> ii, CTimeSeriesSet &mult);
    CTimeSeries<T> divide(int ii, int jj);
    CTimeSeriesSet make_uniform(T increment, bool assgn_d=true);
    CTimeSeriesSet getpercentiles(vector<T> percents);
    CVector out_of_limit(T limit);
	CTimeSeriesSet distribution(int n_bins, int n_columns, int limit);
    CTimeSeriesSet add_noise(vector<T> std, bool logd);
	void clear();
    void clearContents(); //clear the values stored in the time-series without clearing the structure
    void clearContentsExceptLastRow(); //clear the values stored in the time-series without clearing the structure, keep the last row
    vector<T> max_wiggle();
    vector<T> max_wiggle_corr(int _n = 10);
    vector<int> max_wiggle_sl(int ii, T tol);
    CTimeSeriesSet getflow (T A);
    void knockout(T t);
	int lookup(string S);
    vector<T> getrow(int a);
	void setname(int i, string name);
    void resize(unsigned int _size);
    void ResizeIfNeeded(unsigned int _increment);
    void adjust_size();
	bool file_not_found=false;
    CTimeSeries<T> &operator[](int index);
    CTimeSeries<T> &operator[](string BTCName);
    CTimeSeries<T> operator[](int index) const;
    CTimeSeries<T> operator[](string BTCName) const;
    bool Contains(string BTCName);
    bool ReadContentFromFile(string _filename, bool varytime);
#ifdef _ARMA
    arma::mat ToArmaMat(const vector<string> &columns = vector<string>());
    arma::mat ToArmaMat(const vector<int> &columns);
    CTimeSeriesSet(const mat &m, const double &dt);
    arma::mat ToArmaMatShifter(const vector<int> &columns, const vector<vector<int>> &lag);
#endif
#ifdef QT_version
	CTimeSeries &operator[](QString BTCName) {
		return operator[](BTCName.toStdString());}
#endif // QT_version


    CTimeSeriesSet(vector < vector<T>> &, int writeInterval = 1);
	int indexOf(const string& name) const;
	void pushBackName(string name);
    void append(const CTimeSeries<T> &BTC, string name = "");
	CTimeSeriesSet sort(int burnOut = 0);
#ifdef QT_version
	void compact(QDataStream &data) const;
	static CTimeSeriesSet unCompact(QDataStream &data);
#endif // QT_version
	~CTimeSeriesSet(void);
};

template <class T> T diff(CTimeSeriesSet<T> B1, CTimeSeriesSet<T> B2);
template <class T> CTimeSeriesSet<T> operator * (const CTimeSeriesSet<T> &BTC, const double &C);
template <class T> CVector norm2dif(CTimeSeriesSet<T> &A, CTimeSeriesSet<T> &B);
template <class T> CTimeSeriesSet<T> merge(CTimeSeriesSet<T> A, const CTimeSeriesSet<T> &B);
template <class T> CTimeSeriesSet<T> merge(vector<CTimeSeriesSet<T>> &A);
template <class T> CVector sum_interpolate(vector<CTimeSeriesSet<T>> &BTC, double t);
template <class T> T sum_interpolate(vector<CTimeSeriesSet<T>> &BTC, T t, string name);
template <class T> int max_n_vars(vector<CTimeSeriesSet<T>> &BTC);

#include "BTCSet.hpp"
