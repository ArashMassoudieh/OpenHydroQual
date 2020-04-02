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
double QDate2Xldate(QDateTime &x);
QList<int> dayfractiontotime(double dayFraction = 0);
QString float2date(const double d, QString format = "MMM dd yyyy", bool ignorefirst50years = true);
QString float2datetime(const double d, QString format = "MM/dd/yyyy hh:mm:ss", bool ignorefirst50years = true);
QStringList specialSplit(QString s);
QStringList extract_by_space_quote(QString s);
vector<int> find_indexes_of(const QString &s, QString &s1);
QString extract_in_between(const QString &s, QString s1, QString s2);
QString string2QString_qt(string s);
QStringList toQSringList(const vector<string> &s);
#else
string string2QString_nqt(string s);
#endif // Q_version


