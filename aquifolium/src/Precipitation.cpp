/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */



#include "Precipitation.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

CPrecipitation::CPrecipitation(void)
{
    n=0;
}

CPrecipitation::CPrecipitation(int nn)
{
	n = nn;
	s.resize(n);
	e.resize(n);
	i.resize(n);
}

CPrecipitation::CPrecipitation(const CPrecipitation &Precip)
{
	n = Precip.n;
	s = Precip.s;
	e = Precip.e;
	i = Precip.i;
    filename = Precip.filename;


}

CPrecipitation CPrecipitation::operator = (const CPrecipitation &Precip)
{
	n = Precip.n;
	s = Precip.s;
	e = Precip.e;
	i = Precip.i;
    filename = Precip.filename;
	return *this;

}

CPrecipitation::~CPrecipitation(void)
{

}

double CPrecipitation::getval(double time)
{

	structured = false;
	double res = 0;
	if (!structured)
	{
		for (int ii=0; ii<n; ii++)
			if ((time<e[ii]) && (time>=s[ii]))
				res = i[ii]/(e[ii]-s[ii]);
	}
	else
	{
		int ii = static_cast<int>((time-s[0])/dt);
		if (ii<n)
			res = i[ii]/(e[ii]-s[ii]);
		else
			res = 0;

	}
	return res;
}

CPrecipitation::CPrecipitation(string _filename)
{
    filename = _filename;
    ifstream fil(filename);
	n=0;
	if (!fil.good()) return;
	vector<string> ss;
	s.clear(); e.clear(); i.clear();

	while (fil.eof()==false)
	{	ss = aquiutils::getline(fil);

		if (ss.size()>=3)
            if (ss[0].substr(0, 2) != "//" && aquiutils::trim(ss[0])!="")
			{
				s.push_back(atof(ss[0].c_str()));
				e.push_back(atof(ss[1].c_str()));
				i.push_back(atof(ss[2].c_str()));
				n++;
			}
	}
	dt = e[1] - s[1];

}

bool CPrecipitation::isFileValid(string filename)
{
	CPrecipitation temp;
	ifstream fil(filename);
	temp.n = 0;
	if (!fil.good()) return false;
	vector<string> ss;
	temp.s.clear(); temp.e.clear(); temp.i.clear();

	while (fil.eof() == false)
	{
		ss = aquiutils::getline(fil);

		if (ss.size() >= 3)
			if (ss[0].substr(0, 2) != "//")
			{
				temp.s.push_back(atof(ss[0].c_str()));
				temp.e.push_back(atof(ss[1].c_str()));
				temp.i.push_back(atof(ss[2].c_str()));
				temp.n++;
			}
	}
	if (!temp.n)
		return false;
	if (temp.e.size() < 1 || temp.s.size() < 1)
		return false;
	for (unsigned int i = 0; i < temp.e.size(); i++)
		if (temp.e[i] - temp.s[i] <= 0)
			return false;
//	temp.dt = temp.e[1] - temp.s[1];
//	if (!temp.dt)
//		return false;
	return true;
}

void CPrecipitation::getfromfile(string _filename)
{
    filename = _filename;
    ifstream fil(filename);
	vector<string> ss;
	s.clear(); e.clear(); i.clear();
	n=0;
	while (fil.eof()==false)
	{	ss = aquiutils::getline(fil);
		if (ss.size()>=3)
		{	s.push_back(atof(ss[0].c_str()));
			e.push_back(atof(ss[1].c_str()));
			i.push_back(atof(ss[2].c_str()));
			n++;
		}
	}
	dt = e[1] - s[1];
}


TimeSeriesSet<double> CPrecipitation::getflow (double A, double dt)
{
    TimeSeriesSet<double> Rainflowout(1);
    Rainflowout.filename = filename;
	if (n == 0) return Rainflowout;
    Rainflowout.setSeriesName(0,"flow");
	for (double t = s[0]; t<e[n-1]; t+=dt)
        Rainflowout[0].append(t,getval(t)*A);  //i [m]

    Rainflowout[0].assign_D();
	return Rainflowout;
}

TimeSeriesSet<double> CPrecipitation::getflow(double A) const
{
    TimeSeriesSet<double> Rainflowout(1);
    Rainflowout.filename = filename;
    Rainflowout.setSeriesName(0,"flow");
	for (int ii = 0; ii < n; ii++)
	{
		if (ii>0)
			if (s[ii] > e[ii-1]) 
                Rainflowout[0].append(-0.5*e[ii] + 1.5*s[ii] , 0);
		
        Rainflowout[0].append((e[ii] + s[ii]) * 0.5, i[ii] / (e[ii] - s[ii]) * A);  //i [m]
		if (ii < n - 1)
			if (e[ii] < s[ii + 1])
                Rainflowout[0].append(1.5 * e[ii] - 0.5 * s[ii], 0);
	}

    Rainflowout[0].assign_D();
	return Rainflowout;
}

void CPrecipitation::writefile(string Filename)
{
    ofstream file(Filename);
    if (file.good())
    {
        for (int j=0; j<n; j++)
        {   file << std::setprecision(4);
            file << std::fixed;
            file << s[j] << ", " << e[j] <<", " << i[j] << std::endl;

        }
    }
    file.close();

}

void CPrecipitation::append(const double &_s, const double &_e, const double &intensity)
{
    if (intensity!=9999.9)
    {   s.push_back(_s);
        e.push_back(_e);
        i.push_back(intensity);
        n++;
    }

}


