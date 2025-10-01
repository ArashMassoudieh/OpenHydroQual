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



// Refactored Vector_arma.h
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

/**
 * @brief An Armadillo-based vector class with enhanced functionality.
 * Inherits from arma::vec for efficient linear algebra operations.
 */
class CVector_arma : public arma::vec {
public:
    /// Constructors
    CVector_arma();
    explicit CVector_arma(int n);
    CVector_arma(const vector<double>& v);
    CVector_arma(const vector<double>& v, int n);
    CVector_arma(const arma::vec& v);
    CVector_arma(const CVector& v);
    CVector_arma(const vector<int>& v);
    CVector_arma(double x, int n);
    CVector_arma(double x_min, double x_max, int n);
    CVector_arma(const CVector_arma&);

    template<typename T1, typename op_type>
    CVector_arma(const arma::Op<T1, op_type>& X) : arma::vec(X) {}

    template<typename T1>
    CVector_arma(const arma::Base<double, T1>& X) : arma::vec(X) {}
    /// Element access
    double& operator[](int i);
    const double& operator[](int i) const;

    /// Assignment operators
    CVector_arma& operator=(const CVector_arma&);
    CVector_arma& operator=(const CVector&);
    CVector_arma& operator=(const vector<double>&);
    CVector_arma& operator=(const double&);
    CVector_arma operator=(const mat&);

    /// Unary operations
    CVector_arma& operator+();

    /// Modifiers
    void swap(int i, int j);

    /// Utilities
    int getsize() const;
    bool haszeros() const;
    bool is_finite() const;

    /// Arithmetic
    CVector_arma& operator*=(double);
    CVector_arma& operator/=(double);
    CVector_arma& operator+=(const CVector_arma&);
    CVector_arma& operator-=(const CVector_arma&);
    CVector_arma& operator*=(const CVector_arma&);

    /// Comparison
    bool operator==(double v) const;
    bool operator==(const CVector_arma& v) const;

    /// Math operations
    double max() const;
    double min() const;
    double abs_max() const;
    int abs_max_elems() const;
    double norm2() const;
    double sum() const;

    /// Transformations
    CMatrix_arma T() const;
    CVector_arma Log() const;
    CVector_arma Exp() const;
    CVector_arma abs() const;
    CVector_arma H() const;
    CVector_arma sub(int i, int j) const;
    CVector_arma append(const CVector_arma&);
    CVector_arma append(double d);
    vector<int> Int() const;
    vector<int> lookup(double val) const;
    vector<int> get_nan_elements() const;
    vector<int> negative_elements() const;
    CMatrix_arma diagmat() const;

    /// File output
    void writetofile(FILE* f) const;
    void writetofile(const string& filename) const;
    void writetofile(ofstream& f) const;
    void writetofile_app(const string& filename) const;
    void print(const string& s) const;

    /// Output formatting
    string toString() const;
};

/// Non-member utility functions
CVector_arma Log(const CVector_arma&);
CVector_arma Exp(const CVector_arma&);
CVector_arma abs(const CVector_arma&);
double abs_max(const CVector_arma&);
double min(const CVector_arma&);
double max(const CVector_arma&);
CVector_arma H(const CVector_arma&);

CVector_arma operator+(const CVector_arma&, const CVector_arma&);
CVector_arma operator+(double, const CVector_arma&);
CVector_arma operator+(const CVector_arma&, double);
CVector_arma operator-(const CVector_arma&, const CVector_arma&);
CVector_arma operator-(double, const CVector_arma&);
CVector_arma operator*(const CVector_arma&, const CVector_arma&);
CVector_arma operator*(double, const CVector_arma&);
CVector_arma operator*(const CVector_arma&, double);
CVector_arma operator/(const CVector_arma&, double);
CVector_arma operator/(const CVector_arma&, const CVector_arma&);
CVector_arma operator/(double, const CVector_arma&);

CVector_arma zeros_ar(int i);
double avg(const CVector_arma&);
double dotproduct(const CVector_arma&, const CVector_arma&);


