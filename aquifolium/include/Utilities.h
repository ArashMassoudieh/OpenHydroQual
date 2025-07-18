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
#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <QJsonArray>

#define SMALLNUMBER 1e-23
#define PI 3.14159265359

enum class Timing { past, present, both };

using namespace std;

namespace aquiutils
{

    int lookup(const vector<string> &s, const string &s1);
    int lookup(const vector<int> &s, const int &s1, bool backward=false);
    int lookup(const vector<vector<int> > &s, const vector<int> &s1);
    int corresponding_parenthesis(string S, int i);
    int count(const string &s, const string &s1);
    bool parantheses_balance(string S);
    bool contains(const string &s, const string &s1);
    bool contains_any(const std::string& s, const std::string& chars);
    string left(const string &s, int i);
    string right(const string &s, int i);
    bool ends_with(const std::string& value, const std::string& ending);
    void remove(string &s,unsigned int i);
    void replace(string &s,unsigned int i,string p);
    void remove(string &s,unsigned int pos, unsigned int len);
    bool remove(string &s, string s1);
    void insert(string &s, unsigned int pos, string s1);
    bool replace(string &s,string s1, string s2);
    void replace(string &s,unsigned int i, unsigned int j, string p);
    bool isnumber(char S);
    bool isnumber(string S);
    bool isintegernumber(string S);
    double atof(const string &S);
    double atoi(const string &S);
    string trim(const string &s);
    void push_back(vector<string> &s, const vector<string> &s1);
    bool isnegative(const double &x);
    bool ispositive(const double &x);
    bool iszero(const double &x);
    vector<string> split(const string &s, char del);
    vector<string> getline(ifstream& file);
    vector<string> getline(ifstream& file, char del1);
    vector<vector<string>> getline_op(ifstream& file,char del1);
    vector<string> split(const string &s, const vector<char> &del);
    vector<vector<string>> getline_op(ifstream& file,vector<char> del1);
    vector<vector<string>> getline_op_eqplus(ifstream& file);
    vector<string> split_curly_semicolon(string s);
    vector<int> look_up(string s, char del);  //Returns a vector with indices of "del"
    vector<int> ATOI(vector<string> ii);
    vector<double> ATOF(vector<string> ii);
    string tolower(const string &S);
    vector<string> tolower(const vector<string> &S);
    void writeline(ofstream& f, vector<string> s, string del=",");
    void writeline(ofstream& f, vector<vector<string>> s, string del=",", string del2="&");
    void writestring(ofstream& f, string s);
    void writestring(string filename, string s);
    void writenumber(ofstream& f, double s);
    void writeendl(ofstream& f);
    double Heavyside(double x);
    double Pos(double x);
    string numbertostring(const double &x, bool scientific=false);
    string numbertostring(const vector<double> &x, bool scientific=false);
    string numbertostring(int x);
    string numbertostring(unsigned int x);
    string numbertostring(unsigned int x, int number_of_digits);
    string numbertostring(const vector<int> &x, bool scientific=false);
    string tail(std::string const& source, size_t const length);
    string tabs(int i);
    bool And(vector<bool> x);
    //double max(vector<double> x);
    //int max(vector<int> x);
    double Max(vector<double>);
    int Max(vector<int>);
    string remove_backslash_r(const string &ss);
    string GetOnlyFileName(const string &fullfilename);
    string GetPath(const string &fullfilename);
    template<class T>
    T randompick(const vector<T> &vec)
    {
        unsigned int i=rand()%vec.size();
        return vec[i];
    };
    template<typename T> bool isfinite(T arg);
    bool FileExists(const std::string& name);
    std::string extract_path(const std::string& filepath);
    double mod(double x, double y);


}

QJsonArray mergeArrays(const QJsonArray& a1, const QJsonArray& a2);

#endif // UTILITIES_H
