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


// Vector.cpp: implementation of the CVector class.
//
//////////////////////////////////////////////////////////////////////

// Refactored Vector.cpp with const& optimization

#include "Vector.h"
#include "math.h"
#include "Matrix.h"
#include "Vector_arma.h"
#include "QuickSort.h"
#include "Expression.h"
#include <cfloat>
#include <cmath>
#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <iomanip>

CVector::CVector() = default;

CVector::CVector(int n) : vector<double>(n) {}

CVector::CVector(const vector<double>& v, int n) : vector<double>(v.begin(), v.begin() + n) {}

CVector::CVector(const vector<double>& v) : vector<double>(v) {}

CVector::CVector(const vector<bool>& v) {
    reserve(v.size());
    for (bool b : v) push_back(static_cast<double>(b));
}

CVector::CVector(const vector<int>& v) {
    reserve(v.size());
    for (int i : v) push_back(static_cast<double>(i));
}

CVector::CVector(const CVector_arma& v) {
    reserve(v.getsize());
    for (size_t i = 0; i < v.getsize(); ++i) push_back(v[i]);
}

CVector::CVector(double x, int n) : vector<double>(n, x) {}

CVector::CVector(double x_min, double x_max, int n) {
    resize(n + 1);
    for (int i = 0; i <= n; ++i)
        (*this)[i] = x_min + (x_max - x_min) * i / static_cast<double>(n);
}

CVector::CVector(const CVector& v) : vector<double>(v) {}

// Accessors

double& CVector::operator[](int i) {
    return at(i); // Will throw std::out_of_range if invalid
}

const double& CVector::operator[](int i) const {
    return at(i); // Will throw std::out_of_range if invalid
}

double& CVector::at(int i) {
    if (i >= 0 && i < static_cast<int>(size()))
        return vector<double>::at(i);
    throw std::out_of_range("CVector::at (const): index out of range");
}

const double& CVector::at(int i) const {
    if (i >= 0 && i < static_cast<int>(size()))
        return vector<double>::at(i);
    throw std::out_of_range("CVector::at: index out of range");
}


// Assignment

CVector& CVector::operator=(const CVector& v) {
    vector<double>::operator=(v);
    return *this;
}

CVector& CVector::operator=(const vector<double>& v) {
    vector<double>::operator=(v);
    return *this;
}

CVector& CVector::operator=(const CVector_arma& v) {
    resize(v.getsize());
    for (size_t i = 0; i < v.getsize(); ++i) (*this)[i] = v[i];
    return *this;
}

CVector& CVector::operator=(const double& v) {
    for (auto& x : *this) x = v;
    return *this;
}

CVector CVector::operator=(const mat& A) {
    resize(A.n_rows);
    for (size_t i = 0; i < A.n_rows; ++i) (*this)[i] = A(i, 0);
    return *this;
}

// Operators

CVector& CVector::operator+() { return *this; }

void CVector::swap(int i, int j) {
    std::swap((*this)[i], (*this)[j]);
}

int CVector::getsize() const { return static_cast<int>(size()); }

bool CVector::haszeros() const {
    for (const auto& x : *this) if (x == 0.0) return true;
    return false;
}

CVector& CVector::operator*=(double x) {
    for (auto& v : *this) v *= x;
    return *this;
}

CVector& CVector::operator/=(double x) {
    for (auto& v : *this) v /= x;
    return *this;
}

CVector& CVector::operator+=(const CVector& v) {
    for (size_t i = 0; i < size(); ++i) (*this)[i] += v[i];
    return *this;
}

CVector& CVector::operator-=(const CVector& v) {
    for (size_t i = 0; i < size(); ++i) (*this)[i] -= v[i];
    return *this;
}

CVector& CVector::operator*=(const CVector& v) {
    for (size_t i = 0; i < size(); ++i) (*this)[i] *= v[i];
    return *this;
}

CVector operator*(double a, const CVector &v)
{
    CVector out = v;
    out*=a;
    return out;
}

CVector operator-(double a, const CVector& v)
{
    CVector out = v;
    for (int i=0; i<out.size(); i++)
        out[i] = a - v[i];
    return out;
}

CVector operator+(const CVector &v, double a)
{
    CVector v1(v.size());
    for (int i=0; i<v.size(); i++)
        v1[i] = a + v[i];
    return v1;
}

CVector operator+(double a, const CVector &v)
{
    CVector v1(v.size());
    for (int i=0; i<v.size(); i++)
        v1[i] = a + v[i];
    return v1;
}

CVector operator/(const CVector &v, double a)
{
    CVector out = v;
    for (int i=0; i<out.size(); i++)
        out[i] = v[i]/a;
    return out;
}

/**
 * @brief Element-wise division of two CVectors.
 * @throws std::invalid_argument if sizes do not match or division by zero occurs.
 */
CVector operator/(const CVector& v1, const CVector& v2)
{
    if (v1.size() != v2.size())
        throw std::invalid_argument("CVector operator/: vectors must be the same size.");

    CVector result(v1.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        if (v2[i] == 0.0)
            throw std::domain_error("CVector operator/: division by zero at index " + std::to_string(i));
        result[i] = v1[i] / v2[i];
    }

    return result;
}

/**
 * @brief Element-wise multiplication of two CVectors.
 * @throws std::invalid_argument if sizes do not match or division by zero occurs.
 */
CVector operator*(const CVector& v1, const CVector& v2)
{
    if (v1.size() != v2.size())
        throw std::invalid_argument("CVector operator/: vectors must be the same size.");

    CVector result(v1.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        result[i] = v1[i] * v2[i];
    }

    return result;
}

/**
 * @brief Element-wise summation of two CVectors.
 * @throws std::invalid_argument if sizes do not match or division by zero occurs.
 */
CVector operator+(const CVector& v1, const CVector& v2)
{
    if (v1.size() != v2.size())
        throw std::invalid_argument("CVector operator/: vectors must be the same size.");

    CVector result(v1.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        result[i] = v1[i] + v2[i];
    }

    return result;
}

/**
 * @brief Element-wise subtraction of two CVectors.
 * @throws std::invalid_argument if sizes do not match or division by zero occurs.
 */
CVector operator-(const CVector& v1, const CVector& v2)
{
    if (v1.size() != v2.size())
        throw std::invalid_argument("CVector operator/: vectors must be the same size.");

    CVector result(v1.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        result[i] = v1[i] - v2[i];
    }

    return result;
}


bool CVector::operator==(double v) const {
    for (const auto& x : *this) if (x != v) return false;
    return true;
}

bool CVector::operator==(const CVector& v) const {
    if (size() != v.size()) return false;
    for (size_t i = 0; i < size(); ++i) if ((*this)[i] != v[i]) return false;
    return true;
}

bool CVector::is_finite() const {
    for (const auto& x : *this) if (!std::isfinite(x)) return false;
    return true;
}

string CVector::toString() const {
    stringstream ss;
    for (const auto& x : *this) ss << x << ", ";
    return ss.str();
}

void CVector::print(const string& s) const {
    ofstream fout(s);
    for (const auto& x : *this) fout << x << endl;
}

// Math ops

double CVector::max() const {
    return *std::max_element(begin(), end());
}

double CVector::min() const {
    return *std::min_element(begin(), end());
}

double CVector::abs_max() const {
    double a = 0.0;
    for (const auto& x : *this) a = std::max(a, std::abs(x));
    return a;
}

int CVector::abs_max_elems() const {
    int idx = 0;
    double maxval = 0;
    for (size_t i = 0; i < size(); ++i)
        if (std::abs((*this)[i]) > maxval) maxval = std::abs((*this)[i]), idx = i;
    return idx;
}

double CVector::norm2() const {
    double s = 0;
    for (const auto& x : *this) s += x * x;
    return std::sqrt(s);
}

double CVector::sum() const {
    return std::accumulate(begin(), end(), 0.0);
}

double CVector::mean() const {
    return sum() / size();
}

double CVector::stdev() const {
    double m = mean(), ss = 0.0;
    for (const auto& x : *this) ss += (x - m) * (x - m);
    return sqrt(ss / size());
}

// Transformations

CVector CVector::Log() const {
    CVector out(size());
    for (size_t i = 0; i < size(); ++i) out[i] = log((*this)[i]);
    return out;
}

CVector CVector::Exp() const {
    CVector out(size());
    for (size_t i = 0; i < size(); ++i) out[i] = exp((*this)[i]);
    return out;
}

CVector Exp(const CVector &V)
{
    return V.Exp();
}

CVector Log(const CVector &V)
{
    return V.Log();

}

CVector CVector::abs() const {
    CVector out(size());
    for (size_t i = 0; i < size(); ++i) out[i] = fabs((*this)[i]);
    return out;
}

CVector CVector::H() const {
    CVector out(size());
    for (size_t i = 0; i < size(); ++i) out[i] = (*this)[i] > 0 ? (*this)[i] : 1e-25;
    return out;
}

CVector CVector::append(const CVector& V1) {
    insert(end(), V1.begin(), V1.end());
    return *this;
}

CVector CVector::append(double d) {
    push_back(d);
    return *this;
}

CVector CVector::sort() {
    std::sort(begin(), end());
    return *this;
}

vector<int> CVector::Int() const {
    vector<int> out(size());
    for (size_t i = 0; i < size(); ++i) out[i] = static_cast<int>((*this)[i]);
    return out;
}

vector<int> CVector::negative_elements() const {
    vector<int> out;
    for (size_t i = 0; i < size(); ++i)
        if ((*this)[i] < 0) out.push_back(i);
    return out;
}

vector<int> CVector::lookup(double val) const {
    vector<int> idx;
    for (size_t i = 0; i < size(); ++i)
        if ((*this)[i] == val) idx.push_back(i);
    return idx;
}

CVector CVector::sub(int i, int j) const {
    return CVector(vector<double>(begin() + i, begin() + j));
}

void CVector::writetofile(FILE* f) const {
    for (const auto& x : *this) fprintf(f, "%le, ", x);
    fprintf(f, "\n");
}

void CVector::writetofile(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "w");
    writetofile(f);
    fclose(f);
}

void CVector::writetofile_app(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "a");
    writetofile(f);
    fclose(f);
}

void CVector::writetofile(ofstream& f) const {
    for (size_t i = 0; i < size(); ++i) {
        f << (*this)[i];
        if (i != size() - 1) f << ",";
    }
    f << endl;
}

CMatrix CVector::T() const {
    CMatrix K(1, size());
    for (size_t i = 0; i < size(); i++)
        K[0][i] = (*this)[i];
    return K;
}

CMatrix CVector::diagmat() const {
    CMatrix A(size(), size());
    for (size_t i = 0; i < size(); i++)
        A[i][i] = (*this)[i];
    return A;
}

double dotproduct(const CVector& v1, const CVector& v2)
{
    if (v1.size() != v2.size())
        throw std::invalid_argument("dotproduct: vector sizes do not match.");

    double result = 0.0;
    for (size_t i = 0; i < v1.size(); ++i)
        result += v1[i] * v2[i];

    return result;
}

CVector combinesort(const CVector &V1, const CVector &V2)
{
    CVector V3 = V1;
    CVector V = V3.append(V2);
    return V.sort();
}

CVector combinesort_s(const CVector &V1, const CVector &V2)
{
    int n1=0;
    int n2=0;
    CVector V3;
    for (int i=0; i<V1.size()+V2.size(); i++)
    {
        if (n2==V2.size())
        {	V3.append(V1[n1]);
            n1++;
        }
        else if (n1==V1.size())
        {	V3.append(V2[n2]);
            n2++;
        }
        else
        {
            if (V1[n1]>V2[n2])
            {	V3.append(V2[n2]);
                n2++;
            }
            else
            {	V3.append(V1[n1]);
                n1++;
            }
        }
    }

    return V3;

}

void writetofile(const std::vector<CVector>& data,
                 const std::vector<std::string>& columnlabels,
                 const std::vector<std::string>& rowlabels,
                 const std::string& filename)
{
    if (data.empty()) {
        std::cerr << "Error: Data is empty.\n";
        return;
    }

    size_t nrows = data[0].size();
    size_t ncols = data.size();

    // Check consistency
    if (columnlabels.size() != ncols) {
        std::cerr << "Error: Column label count does not match number of columns.\n";
        return;
    }

    if (rowlabels.size() != nrows) {
        std::cerr << "Error: Row label count does not match number of rows.\n";
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    // Write column header
    file << "RowLabel";
    for (const auto& label : columnlabels) {
        file << "\t" << label;
    }
    file << "\n";

    // Write rows
    for (size_t i = 0; i < nrows; ++i) {
        file << rowlabels[i];
        for (size_t j = 0; j < ncols; ++j) {
            file << "\t" << std::setprecision(10) << data[j][i];
        }
        file << "\n";
    }

    file.close();
}
