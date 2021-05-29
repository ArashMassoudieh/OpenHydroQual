
#pragma once

#include <map>
#include <string>
#include <vector>
#include "QuickSort.h"
#include "NormalDist.h"

//GUI
#ifdef QT_version
#include "qlist.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_version

#define CBTC CTimeSeries

using namespace std;


class CTimeSeries
{
public:
    bool structured=true;
	CTimeSeries();
	CTimeSeries(int n);
	virtual ~CTimeSeries();
	int n;
	vector<double> t;
	vector<double> C;
    string filename;
	string name = "";
	string unit = "";
	string defaultUnit = "";
	vector<string> unitsList;

    vector<double> D;

    double interpol(const double &x) const; //interpolate at location x
	CTimeSeries MA_smooth(int span); //Moving average smoothing with span of 1+2*span
	double interpol_D(double x); //interpolate the distance to the next non-zero data point
	CTimeSeries interpol(vector<double> x); //interpolate at each value in vector x
    CTimeSeries interpol(CTimeSeries &x) const; //interpolate at times in the time axis of x
	CTimeSeries(const CTimeSeries &C);
	CTimeSeries(string Filename); //create BTC based on the filename
	CTimeSeries& operator = (const CTimeSeries &C);
	void readfile(string); //read the values from a text file
	void writefile(string Filename); //writes the BTC contets into a fild
	double maxC(); //returns the maximum value
	double minC(); //returns the minimum value
	void setnumpoints(int); //resize the timeseries
	CTimeSeries Log(); //take the log of all the data points
	CTimeSeries Log(double min); //log(min(min,C))
	double std(); //standard deviation of C
	double mean(); //mean of C
	double percentile(double x); //x^th percentile of C
	double percentile(double x, int limit); //x^th percentile with the exception of the first "limit" data points
	double mean(int limit); // mean of the data after excluding "limit" data points
	double std(int nlimit); // standard deviation of the data after excluding "limit" data points
	double mean_log(int limit); //mean of log transformed data after excluding "limit" data points
	double integrate(); // integral of the time series
	double variance(); //calculates the variance of the value of the timeseries
	double integrate(double t); //integral from the begining to time t
	double integrate(double t1, double t2); //integral between time t1 and t2
	int lookupt(double t); // finds the index of the datapoint with time t
	double average(); //integral of time-series devided by the domail length
	double average(double t); // integral to time t devided by domain length
    double slope(); //slope of time-series at its end
	CTimeSeries distribution(int n_bins, int limit); //extract the histogram of values
    bool append(double x); //appends a data point with value x
    bool append(double tt, double xx); //appends a datapoint with value xx at time tt
	void append(CTimeSeries &CC);// appends a time-series to the time-series
    void ResizeIfNeeded(int _increment); //increases the size of the vectors more capacity is needed
	CTimeSeries& operator+=(CTimeSeries &v); //adds another time-series to the existing one
	CTimeSeries& operator%=(CTimeSeries &v); //adds another time-series by corresponding indexes
	CTimeSeries make_uniform(double increment); //create a new time-series with uniformly distributed time-axis
	CTimeSeries extract(double t1, double t2); //extracts a sub time-series from t1 to t2.
	vector<double> trend(); //calculate the slope based on regression
	double mean_t(); //mean of t values of data point
	CTimeSeries add_noise(double std, bool); //adds Gaussian noise to values
	void assign_D(); //Assign distances to the next non-zero values
	void clear(); // clears the time-series
	double wiggle(); //calculate oscillation
	double wiggle_corr(int _n=10);
	bool wiggle_sl(double tol);
	double maxfabs();
	double max_fabs;
	void knock_out(double t);
	double AutoCor1(int i=0);
	bool file_not_found = false;
	CTimeSeries getcummulative();
	CTimeSeries Exp();
	CTimeSeries fabs();
    void adjust_size();
	//GUI
	//QList <QMap <QVariant, QVariant>> compact() const;
    bool resize(unsigned int _size);
    unsigned int Capacity();
	CTimeSeries(double a, double b, const vector<double>&x);
	CTimeSeries(double a, double b, const CTimeSeries &btc);
	CTimeSeries(const vector<double> &t, const vector<double> &C);
	CTimeSeries(vector<double>&, int writeInterval = 1);
	bool error = false;
    double GetLastItemValue();
    double GetLastItemTime();


#ifdef QT_version
	CTimeSeries(QList <QMap <QVariant, QVariant>> data);
	void compact(QDataStream &data) const;
	static CTimeSeries unCompact(QDataStream &data);
#endif // QT_version

};

double diff(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
double diff_abs(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
double diff_log(CTimeSeries &BTC_p, CTimeSeries &BTC_d, double lowlim);
double diff_norm(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
double diff(CTimeSeries BTC_p, CTimeSeries BTC_d, int scale);
double diff(CTimeSeries BTC_p, CTimeSeries BTC_d, CTimeSeries Q);
double diff2(CTimeSeries *BTC_p, CTimeSeries BTC_d);
double diff2(CTimeSeries BTC_p, CTimeSeries *BTC_d);
double diff2(const CTimeSeries &BTC_p, const CTimeSeries &BTC_d);
double diff_mixed(CTimeSeries &BTC_p, CTimeSeries &BTC_d, double lowlim, double std_n, double std_ln);
double ADD(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
double diff_relative(CTimeSeries &BTC_p, CTimeSeries &BTC_d, double m);
double R2(CTimeSeries BTC_p, CTimeSeries BTC_d);
double R(CTimeSeries BTC_p, CTimeSeries BTC_d, int nlimit);
CTimeSeries operator*(double, CTimeSeries&);
CTimeSeries operator*(CTimeSeries&, double);
CTimeSeries operator*(CTimeSeries&, CTimeSeries&);
CTimeSeries operator/(CTimeSeries&, CTimeSeries&);
CTimeSeries operator+(CTimeSeries&, CTimeSeries&);
CTimeSeries operator-(CTimeSeries&, CTimeSeries&);
CTimeSeries operator%(CTimeSeries&, CTimeSeries&);
CTimeSeries operator&(CTimeSeries&, CTimeSeries&);
CTimeSeries operator/(CTimeSeries &CTimeSeries_T, double alpha);
CTimeSeries operator>(CTimeSeries& BTC1, CTimeSeries& BTC2);
double XYbar(CTimeSeries& BTC_p, CTimeSeries &BTC_d);
double X2bar(CTimeSeries& BTC_p, CTimeSeries &BTC_d);
double Y2bar(CTimeSeries& BTC_p, CTimeSeries &BTC_d);
double Ybar(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
double Xbar(CTimeSeries &BTC_p, CTimeSeries &BTC_d);
CTimeSeries operator+(CTimeSeries &v1, CTimeSeries &v2);
double prcntl(vector<double> C, double x);
vector<double> prcntl(vector<double> C, vector<double> x);
double sgn(double val);
int sgn(int val);
double sum_interpolate(vector<CTimeSeries>, double t);
double R2_c(CTimeSeries BTC_p, CTimeSeries BTC_d);
double norm2(CTimeSeries BTC1);
CTimeSeries max(CTimeSeries A, double b);
//GUI
map<string, double> regression(vector<double> x, vector<double> y);
