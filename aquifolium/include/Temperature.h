#pragma once

#include <vector>
#include <string>

//MM
#include "BTCSet.h"



using namespace std;

class CTemperature
{
public:
    CTemperature(void);
    CTemperature(int n);
    CTemperature(string filename);
    CTemperature(const CTemperature &Temp);
    CTemperature operator = (const CTemperature &Temp);
    void append(const double &_t);
    int n;
    vector<double> t;
    bool structured = false;
    double dt;
    double getval(double time);
    void getfromfile(string filename);
    string filename;

    //MM
    //CBTCSet getflow_Evap(double A);

    CTimeSeriesSet<double> getflow (double A) const;
    CTimeSeriesSet<double> getflow(double A, double dt);
    void writefile(string Filename);

    static bool isFileValid(string filename);
public:
    ~CTemperature(void);
};
