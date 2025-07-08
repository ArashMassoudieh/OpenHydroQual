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


// Vector.cpp: implementation of the CVector_arma_arma class.
//
//////////////////////////////////////////////////////////////////////

// Refactored Vector_arma.cpp implementation based on new header design

#include "Vector_arma.h"
#include "Matrix_arma.h"
#include "Vector.h"
#include <cmath>
#include <cfloat>
#include <fstream>
#include <stdexcept>
#include "Expression.h"

// Constructors
CVector_arma::CVector_arma() : arma::vec() {}
CVector_arma::CVector_arma(int n) : arma::vec(n, fill::zeros) {}
CVector_arma::CVector_arma(const vector<double>& v) : arma::vec(v) {}
CVector_arma::CVector_arma(const vector<double>& v, int n) : arma::vec(vector<double>(v.begin(), v.begin() + n)) {}
CVector_arma::CVector_arma(const arma::vec& v) : arma::vec(v) {}
CVector_arma::CVector_arma(const CVector& v) : arma::vec(std::vector<double>(v.begin(), v.end())) {}
CVector_arma::CVector_arma(const vector<int>& v) {
    set_size(v.size());
    for (size_t i = 0; i < v.size(); ++i) (*this)(i) = static_cast<double>(v[i]);
}
CVector_arma::CVector_arma(double x, int n) : arma::vec(n) { (*this).fill(x); }
CVector_arma::CVector_arma(double x_min, double x_max, int n) : arma::vec(n + 1) {
    for (int i = 0; i <= n; ++i) (*this)(i) = x_min + (x_max - x_min) * i / double(n);
}
CVector_arma::CVector_arma(const CVector_arma& v) : arma::vec(v) {}

// Element access
double& CVector_arma::operator[](int i) {
    if (i >= 0 && i < static_cast<int>(n_elem)) return (*this)(i);
    throw std::out_of_range("CVector_arma::operator[] - index out of bounds");
}
const double& CVector_arma::operator[](int i) const {
    if (i >= 0 && i < static_cast<int>(n_elem)) return (*this)(i);
    throw std::out_of_range("CVector_arma::operator[] const - index out of bounds");
}

// Assignment operators
CVector_arma& CVector_arma::operator=(const CVector_arma& v) {
    arma::vec::operator=(v);
    return *this;
}
CVector_arma& CVector_arma::operator=(const CVector& v) {
    set_size(v.size());
    for (size_t i = 0; i < v.size(); ++i) (*this)(i) = v[i];
    return *this;
}
CVector_arma& CVector_arma::operator=(const vector<double>& v) {
    *this = arma::vec(v);
    return *this;
}
CVector_arma& CVector_arma::operator=(const double& val) {
    for (auto& x : *this) x = val;
    return *this;
}
CVector_arma CVector_arma::operator=(const mat& A) {
    set_size(A.n_rows);
    for (size_t i = 0; i < A.n_rows; ++i) (*this)(i) = A(i, 0);
    return *this;
}

// Arithmetic
CVector_arma& CVector_arma::operator+() { return *this; }
CVector_arma& CVector_arma::operator*=(double x) { (*this) *= x; return *this; }
CVector_arma& CVector_arma::operator/=(double x) { (*this) /= x; return *this; }
CVector_arma& CVector_arma::operator+=(const CVector_arma& v) {
    this->arma::vec::operator+=(v);
    return *this;
}

CVector_arma& CVector_arma::operator-=(const CVector_arma& v) {
    this->arma::vec::operator-=(v);
    return *this;
}
CVector_arma& CVector_arma::operator*=(const CVector_arma& v) {
    for (size_t i = 0; i < n_elem; ++i) (*this)(i) *= v(i);
    return *this;
}

// Comparison
bool CVector_arma::operator==(double v) const {
    for (auto& x : *this) if (x != v) return false;
    return true;
}

bool CVector_arma::operator==(const CVector_arma& v) const {
    if (n_elem != v.n_elem) return false;
    for (size_t i = 0; i < n_elem; ++i) if ((*this)(i) != v(i)) return false;
    return true;
}

// Utilities
void CVector_arma::swap(int i, int j) { std::swap((*this)(i), (*this)(j)); }
int CVector_arma::getsize() const { return static_cast<int>(n_elem); }
bool CVector_arma::haszeros() const {
    return arma::any(arma::abs(static_cast<const arma::vec&>(*this)) < 1e-12);
}
bool CVector_arma::is_finite() const {
    for (auto& x : *this) if (!std::isfinite(x)) return false;
    return true;
}
vector<int> CVector_arma::get_nan_elements() const {
    vector<int> out;
    for (size_t i = 0; i < n_elem; ++i)
        if (!std::isfinite((*this)(i))) out.push_back(i);
    return out;
}
vector<int> CVector_arma::negative_elements() const {
    vector<int> out;
    for (size_t i = 0; i < n_elem; ++i)
        if ((*this)(i) < 0) out.push_back(i);
    return out;
}

// File I/O
void CVector_arma::writetofile(FILE* f) const {
    for (auto& x : *this) fprintf(f, "%le, ", x);
    fprintf(f, "\n");
}
void CVector_arma::writetofile(ofstream& f) const {
    for (size_t i = 0; i < n_elem; ++i) {
        f << (*this)(i);
        if (i < n_elem - 1) f << ",";
    }
    f << endl;
}
void CVector_arma::writetofile(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "w");
    writetofile(f);
    fclose(f);
}
void CVector_arma::writetofile_app(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "a");
    writetofile(f);
    fclose(f);
}

// Text
void CVector_arma::print(const string& s) const {
    ofstream Afile(s);
    for (auto& x : *this) Afile << x << endl;
}
string CVector_arma::toString() const {
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < n_elem; ++i) {
        ss << (*this)(i);
        if (i < n_elem - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

// Continuation of full implementation for CVector_arma

CVector_arma CVector_arma::Log() const {
    CVector_arma out(n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        out(i) = std::log((*this)(i));
    return out;
}

CVector_arma Log(const CVector_arma& v) {
    return v.Log();
}

CVector_arma CVector_arma::Exp() const {
    CVector_arma out(n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        out(i) = std::exp((*this)(i));
    return out;
}

CVector_arma Exp(const CVector_arma& v) {
    return v.Exp();
}

CVector_arma CVector_arma::abs() const {
    CVector_arma out(n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        out(i) = std::abs((*this)(i));
    return out;
}

CVector_arma abs(const CVector_arma& v) {
    return v.abs();
}

CVector_arma CVector_arma::H() const {
    CVector_arma out(n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        out(i) = std::max(0.0, (*this)(i));
    return out;
}

CVector_arma H(const CVector_arma& v) {
    return v.H();
}

CMatrix_arma CVector_arma::T() const {
    CMatrix_arma M(1, n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        M(0, i) = (*this)(i);
    return M;
}

CVector_arma CVector_arma::sub(int i, int j) const {
    if (i < 0 || j > static_cast<int>(n_elem) || i >= j)
        throw std::out_of_range("CVector_arma::sub - invalid range");
    CVector_arma out(j - i);
    for (int k = i; k < j; ++k)
        out[k - i] = (*this)(k);
    return out;
}

CVector_arma CVector_arma::append(const CVector_arma& v1) {
    CVector_arma out(n_elem + v1.n_elem);
    for (size_t i = 0; i < n_elem; ++i) out(i) = (*this)(i);
    for (size_t i = 0; i < v1.n_elem; ++i) out(i + n_elem) = v1(i);
    return out;
}

CVector_arma CVector_arma::append(double d) {
    CVector_arma out(n_elem + 1);
    for (size_t i = 0; i < n_elem; ++i) out(i) = (*this)(i);
    out(n_elem) = d;
    return out;
}

vector<int> CVector_arma::Int() const {
    vector<int> out(n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        out[i] = static_cast<int>((*this)(i));
    return out;
}

vector<int> CVector_arma::lookup(double val) const {
    vector<int> res;
    for (size_t i = 0; i < n_elem; ++i)
        if ((*this)(i) == val) res.push_back(i);
    return res;
}

double CVector_arma::abs_max() const {
    double maxval = 0;
    for (auto& x : *this) maxval = std::max(maxval, std::abs(x));
    return maxval;
}

int CVector_arma::abs_max_elems() const {
    double maxval = 0;
    int idx = 0;
    for (size_t i = 0; i < n_elem; ++i) {
        double absval = std::abs((*this)(i));
        if (absval > maxval) {
            maxval = absval;
            idx = static_cast<int>(i);
        }
    }
    return idx;
}

double CVector_arma::norm2() const {
    return std::sqrt(dotproduct(*this, *this));
}

double CVector_arma::sum() const {
    return arma::sum(static_cast<const arma::vec&>(*this));
}

double max(const CVector_arma& v) { return v.max(); }
double min(const CVector_arma& v) { return v.min(); }
double abs_max(const CVector_arma& v) { return v.abs_max(); }
double avg(const CVector_arma& v) { return v.sum() / static_cast<double>(v.n_elem); }
double dotproduct(const CVector_arma& a, const CVector_arma& b)
{
    if (a.n_elem != b.n_elem)
        throw std::invalid_argument("dotproduct: vector size mismatch");
    return arma::dot(static_cast<const arma::vec&>(a), static_cast<const arma::vec&>(b));
}

CVector_arma zeros_ar(int i) { return CVector_arma(i); }

CMatrix_arma CVector_arma::diagmat() const {
    CMatrix_arma A(n_elem, n_elem);
    for (size_t i = 0; i < n_elem; ++i)
        A(i, i) = (*this)(i);
    return A;
}

// Element-wise and scalar arithmetic operator overloads for CVector_arma

#include "Vector_arma.h"
#include <stdexcept>

CVector_arma operator+(const CVector_arma& a, const CVector_arma& b) {
    if (a.n_elem != b.n_elem)
        throw std::invalid_argument("operator+: size mismatch");
    return CVector_arma(static_cast<arma::vec>(a) + static_cast<arma::vec>(b));
}

CVector_arma operator+(double x, const CVector_arma& v) {
    return CVector_arma(x + static_cast<arma::vec>(v));
}

CVector_arma operator+(const CVector_arma& v, double x) {
    return CVector_arma(static_cast<arma::vec>(v) + x);
}

CVector_arma operator-(const CVector_arma& a, const CVector_arma& b) {
    if (a.n_elem != b.n_elem)
        throw std::invalid_argument("operator-: size mismatch");
    return CVector_arma(static_cast<arma::vec>(a) - static_cast<arma::vec>(b));
}

CVector_arma operator-(double x, const CVector_arma& v) {
    return CVector_arma(x - static_cast<arma::vec>(v));
}

CVector_arma operator*(const CVector_arma& a, const CVector_arma& b) {
    if (a.n_elem != b.n_elem)
        throw std::invalid_argument("operator*: size mismatch");
    return CVector_arma(static_cast<arma::vec>(a) % static_cast<arma::vec>(b));
}

CVector_arma operator*(double x, const CVector_arma& v) {
    return CVector_arma(x * static_cast<arma::vec>(v));
}

CVector_arma operator*(const CVector_arma& v, double x) {
    return CVector_arma(static_cast<arma::vec>(v) * x);
}

CVector_arma operator/(const CVector_arma& v, double x) {
    if (x == 0.0)
        throw std::invalid_argument("operator/: division by zero");
    return CVector_arma(static_cast<arma::vec>(v) / x);
}

CVector_arma operator/(const CVector_arma& a, const CVector_arma& b) {
    if (a.n_elem != b.n_elem)
        throw std::invalid_argument("operator/: size mismatch");
    return CVector_arma(static_cast<arma::vec>(a) / static_cast<arma::vec>(b));
}

CVector_arma operator/(double x, const CVector_arma& v) {
    arma::vec result(v.n_elem);
    for (size_t i = 0; i < v.n_elem; ++i) {
        if (v(i) == 0.0)
            throw std::invalid_argument("operator/: division by zero in vector element");
        result(i) = x / v(i);
    }
    return CVector_arma(result);
}

double CVector_arma::max() const {
    return arma::max(static_cast<const arma::vec&>(*this));
}

double CVector_arma::min() const {
    return arma::min(static_cast<const arma::vec&>(*this));
}


