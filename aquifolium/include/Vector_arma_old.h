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

#include <iostream>
#include <vector>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"


using namespace arma;
using namespace std;

class CMatrix_arma;
class CVector;
class SizeDist;
class CVector_arma
{
private:


public:
	vec vect;
	CVector_arma();
	CVector_arma(int);
	CVector_arma(const vector<double>, int);
	CVector_arma(const vector<double> &v);
	CVector_arma(const vec &v);
	CVector_arma(CVector &v);
	CVector_arma(const vector<int> &v);
	CVector_arma(const double x, int n);
    CVector_arma(const double x_min, const double x_max, int n);  //CVector_arma:: is redundant. However, works fine here.
	CVector_arma(const CVector_arma&);
	double& operator[](int);
    const double& operator[](int) const;
	virtual ~CVector_arma();
	int num;
    int range(int);
	CVector_arma& operator=(const CVector_arma&);
	CVector_arma& operator=(const CVector&);
	CVector_arma& operator=(const vector<double>&);
	CVector_arma& operator=(const double &v);
	CVector_arma operator=(mat);
	CVector_arma& operator+();
	void swap(int , int );
    int getsize() const;
    bool haszeros() const;
	CVector_arma& operator*=(double);
	CVector_arma& operator/=(double);
	CVector_arma& operator+=(const CVector_arma&);
	CVector_arma& operator-=(const CVector_arma&);
	CVector_arma& operator*=(const CVector_arma&);
	friend double dotproduct(CVector_arma, CVector_arma);
	friend CVector_arma mult(CMatrix_arma&, CVector_arma&);
	friend double norm(CVector_arma);			//Friend can be deleted. we don't have any private or protected variable in this class  //
	friend double dotproduct(CVector_arma v1, CVector_arma v2);
	bool operator==(double v);
	bool operator==(CVector_arma &v);
	double max();
	double min();
	double norm2();
	double sum();
	double abs_max();
	int abs_max_elems();
	CMatrix_arma T();
	CVector_arma Log();
	CVector_arma abs();
	CVector_arma H();
	void writetofile(FILE *f);
	void writetofile(string filename);
	void writetofile(ofstream &f);
	void writetofile_app(string filename);
	CVector_arma Exp();
	vector<int> Int();
	CMatrix_arma diagmat();
	CVector_arma append(const CVector_arma& V1);
	CVector_arma append(double d);
	CVector_arma sort();
	vector<int> lookup(double val);
	void print(string s);
	CVector_arma sub(int i, int j);
	bool is_finite();
	vector<int> get_nan_elements();
    string toString() const;
    vector<int> negative_elements();
};

CVector_arma Log(CVector_arma &);
CVector_arma Exp(CVector_arma &);
CVector_arma abs(CVector_arma &);  //works w/o reference. if const included means read only
double abs_max(CVector_arma &);
double min(CVector_arma &);
double max(CVector_arma &);
CVector_arma H(CVector_arma &);
CVector_arma operator+(const CVector_arma&, const CVector_arma&);
CVector_arma operator+(double, CVector_arma);
CVector_arma operator+(CVector_arma, double);
CVector_arma operator-(const CVector_arma&, const CVector_arma&);
CVector_arma operator-(double, CVector_arma&);
CVector_arma operator*(CVector_arma, CVector_arma);
CVector_arma operator*(double, CVector_arma);
CVector_arma operator*(CVector_arma, double);
CVector_arma operator/(CVector_arma, double);
CVector_arma operator/(CVector_arma, CVector_arma);
CVector_arma operator/(double, CVector_arma);
CVector_arma zeros_ar(int i);
double avg(CVector_arma &);
