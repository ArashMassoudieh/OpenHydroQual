#include "IntervalRainfall.h"
/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
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

    while (!fil.eof())
    {
        ss = aquiutils::getline(fil);

        if (ss.size() >= 3)
        {
            string s0 = aquiutils::trim(ss[0]);
            string s1 = aquiutils::trim(ss[1]);
            string s2 = aquiutils::trim(ss[2]);

            if (s0.substr(0, 2) != "//" && !s0.empty())
            {
                // Check if each value is numeric before conversion
                if (aquiutils::isnumber(s0) && aquiutils::isnumber(s1) && aquiutils::isnumber(s2))
                {
                    s.push_back(atof(s0.c_str()));
                    e.push_back(atof(s1.c_str()));
                    i.push_back(atof(s2.c_str()));
                    n++;
                }
            }
        }
    }
	dt = n > 0 ? e[0] - s[0] : 0.0;

}

bool CPrecipitation::isFileValid(string filename)
{
    std::ifstream file(filename);
    if(!file)return false;
    // Accept ordinary CSV whitespace, including CRLF and tab-delimited padding.
    // aquiutils::trim only strips spaces, so it must not define numeric-token
    // consumption here (a valid trailing CR is not numeric junk).
    auto clean=[](const std::string& value){
        const auto first=value.find_first_not_of(" \t\r\n\v\f");
        if(first==std::string::npos)return std::string();
        const auto last=value.find_last_not_of(" \t\r\n\v\f");
        return value.substr(first,last-first+1);
    };
    bool any=false;double lastEnd=0;
    while(!file.eof()){
        auto fields=aquiutils::getline(file);
        if(fields.empty())continue;
        if(clean(fields[0]).empty()){
            if(fields.size()==1)continue;
            return false;
        }
        if(clean(fields[0]).substr(0,2)=="//")continue;
        if(fields.size()!=3)return false;
        double values[3];
        try{
            for(int k=0;k<3;++k){
                std::string token=clean(fields[k]);size_t used=0;
                values[k]=std::stod(token,&used);
                if(used!=token.size() || !std::isfinite(values[k]))return false;
            }
        }catch(const std::exception&){return false;}
        if(values[1]<=values[0] || values[2]<0 || (any && values[0]<lastEnd))return false;
        lastEnd=values[1];any=true;
    }
    return any;
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
        {   s.push_back(atof(ss[0].c_str()));
			e.push_back(atof(ss[1].c_str()));
			i.push_back(atof(ss[2].c_str()));
			n++;
		}
	}
	// A missing/empty/single-row file (e.g. a stale filename reference from a
	// JSON config, reached via fromJsonObject() with no validity check ahead
	// of it) used to read e[1]/s[1] out of bounds here - undefined behavior.
	// Each row already has an interval, including a single-row file.
	dt = n > 0 ? e[0] - s[0] : 0.0;
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
    TimeSeriesSet<double> result(1);
    result.filename=filename;
    result.setSeriesName(0,"flow");
    if(n<0 || size_t(n)!=s.size())throw std::invalid_argument("Inconsistent rainfall bin count");
    for(const auto& point:intervalRainfallKnots(s,e,i,A))
        result[0].append(point.first,point.second);
    result[0].setFilename(filename);
    result[0].assign_D();
    return result;
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

#ifdef Q_JSON_SUPPORT
#include <QJsonObject>
#include <QJsonArray>

QJsonObject CPrecipitation::toJsonObject() const
{
    QJsonObject json;

    if (!filename.empty())
    {
        // Case A: backed by a file on disk. Just record the path; the
        // reader will pull the bins from that file when needed.
        json["filename"] = QString::fromStdString(filename);
        return json;
    }

    // Case B: injected at runtime, no backing file. Embed the bins inline
    // so the snapshot is self-contained.
    QJsonArray bins;
    for (int j = 0; j < n; ++j)
    {
        QJsonObject bin;
        bin["s"] = s[j];
        bin["e"] = e[j];
        bin["i"] = i[j];
        bins.append(bin);
    }
    json["bins"] = bins;
    return json;
}

bool CPrecipitation::fromJsonObject(const QJsonObject &obj)
{
    // Reset
    s.clear();
    e.clear();
    i.clear();
    n = 0;
    filename.clear();

    if (obj.contains("filename"))
    {
        const QString fn = obj.value("filename").toString();
        if (!fn.isEmpty())
        {
            // Mirror the guard Quan::SetTimeSeries already uses before calling
            // getfromfile(): a missing/invalid file should report failure to
            // the caller here, not silently succeed with an empty series.
            if (!isFileValid(fn.toStdString()))
            {
                return false;
            }
            getfromfile(fn.toStdString());   // existing method, populates s/e/i/n
            return true;
        }
    }

    if (obj.contains("bins"))
    {
        const QJsonArray bins = obj.value("bins").toArray();
        s.reserve(bins.size());
        e.reserve(bins.size());
        i.reserve(bins.size());
        for (const auto &v : bins)
        {
            const QJsonObject bin = v.toObject();
            s.push_back(bin.value("s").toDouble());
            e.push_back(bin.value("e").toDouble());
            i.push_back(bin.value("i").toDouble());
            ++n;
        }
        if (n >= 2)
            dt = e[1] - s[1];
        return true;
    }

    // Empty object — leave CPrecipitation empty, not an error.
    return true;
}
#endif
