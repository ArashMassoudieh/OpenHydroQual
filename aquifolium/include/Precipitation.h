#pragma once

#include <vector>
#include <string>

//MM
#include "BTCSet.h"



using namespace std;

class CPrecipitation
{
public:
    CPrecipitation(void);
    CPrecipitation(int n);
    CPrecipitation(string filename);
    CPrecipitation(const CPrecipitation &Precip);
    CPrecipitation operator = (const CPrecipitation &Precip);
    int n;
    vector<double> s;
    vector<double> e;
    vector<double> i;
    bool structured = false;
    double dt;
    double getval(double time);
    void getfromfile(string filename);
    string filename;

	//MM
    //CBTCSet getflow_Evap(double A);
	
    CTimeSeriesSet<double> getflow (double A);
    CTimeSeriesSet<double> getflow(double A, double dt);
    void writefile(string Filename);

	static bool isFileValid(string filename);
public:
	~CPrecipitation(void);
};
