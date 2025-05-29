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


#pragma once
#include <string>
#include <vector>
#ifdef Q_version
#include "qdatetime.h"
#endif // Q_version
using namespace std;

bool isnumber(string S);
double dayOfYear(const double xldate);

#ifdef Q_version
qint64 xldate2julian(const qint64 xldate);
qint64 julian2xldate(const qint64 juliandate);
int dayOfYear(const qint64 xldate);
double xldate2julian(const double xldate);
double julian2xldate(const double juliandate);
double timetodayfraction(int hh = 0, int mm = 0, int ss = 0);
double QDate2Xldate(const QDateTime &x);
QList<int> dayfractiontotime(double dayFraction = 0);
QString float2date(const double d, QString format = "MMM dd yyyy", bool ignorefirst50years = true);
QString float2datetime(const double d, QString format = "MM/dd/yyyy hh:mm:ss", bool ignorefirst50years = true);
QStringList specialSplit(QString s);
QStringList extract_by_space_quote(QString s);
vector<int> find_indexes_of(const QString &s, QString &s1);
QString extract_in_between(const QString &s, QString s1, QString s2);
QString string2QString_qt(string s);
bool fileExists(QString path);
QStringList toQStringList(const vector<string> &s);
#else
string string2QString_nqt(string s);
#endif // Q_version


