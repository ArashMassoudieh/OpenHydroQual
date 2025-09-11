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
#ifndef CVector_refactored
#define CVector_refactored

#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <cmath>
#include "armadillo"

using namespace arma;
using namespace std;

class CVector_arma;
class CMatrix;
class SizeDist;

/**
 * @brief A dynamic vector class inheriting from std::vector<double>
 * for numerical computations. Includes various mathematical and
 * utility operations, file I/O, and interoperability with Armadillo.
 */
class CVector : public vector<double>
{
public:
    /** Default constructor */
    CVector();

    /**
     * @brief Construct a vector of given size.
     * @param n Size of the vector.
     */
    CVector(int n);

    /**
     * @brief Construct a vector from another std::vector with a custom size.
     * @param v Source vector.
     * @param n Number of elements to take.
     */
    CVector(const vector<double>& v, int n);

    /**
     * @brief Construct from a full std::vector.
     * @param v Source vector.
     */
    CVector(const vector<double>& v);

    /**
     * @brief Construct from a std::vector of bools.
     * @param v Boolean vector to convert.
     */
    CVector(const vector<bool>& v);

    /**
     * @brief Construct from a std::vector of ints.
     * @param v Integer vector to convert.
     */
    CVector(const vector<int>& v);

    /**
     * @brief Construct from a CVector_arma.
     * @param v CVector_arma instance.
     */
    CVector(const CVector_arma& v);

    /**
     * @brief Fill constructor.
     * @param x Value to fill.
     * @param n Size of the vector.
     */

    CVector(double x, int n);

    /**
     * @brief Linearly spaced vector.
     * @param x_min Minimum value.
     * @param x_max Maximum value.
     * @param n Number of intervals (size will be n+1).
     */
    CVector(double x_min, double x_max, int n);

    /** Copy constructor */
    CVector(const CVector&);

    /**
     * @brief Writable index access.
     * @param i Index.
     * @return Reference to value at index.
     */
    double& operator[](int i);
    
    /**
     * @brief Read-only index access with bounds checking.
     * @param i Index.
     * @return Value at index.
     */
    const double& operator[](int i) const;

    /**
     * @brief Writable index access with bounds checking.
     * @param i Index.
     * @return Reference to value at index.
     */
    double& at(int i);
    const double& at(int i) const ;

    /** Assignment from CVector */
    CVector& operator=(const CVector&);

    /** Assignment from std::vector */
    CVector& operator=(const vector<double>&);

    /** Assignment from CVector_arma */
    CVector& operator=(const CVector_arma&);

    /** Fill assignment */
    CVector& operator=(const double& v);

    /** Assignment from Armadillo column vector */
    CVector operator=(const mat&);

    /** Unary plus operator */
    CVector& operator+();

    /**
     * @brief Swap elements at two indices.
     * @param i First index.
     * @param j Second index.
     */
    void swap(int i, int j);

    /** @return Size of the vector. */
    int getsize() const;

    /** @return True if any element is zero. */
    bool haszeros() const;

    /** Multiply by scalar */
    CVector& operator*=(double);

    /** Divide by scalar */
    CVector& operator/=(double);

    /** Element-wise addition */
    CVector& operator+=(const CVector&);

    /** Element-wise subtraction */
    CVector& operator-=(const CVector&);

    /** Element-wise multiplication */
    CVector& operator*=(const CVector&);

    /** Compare all elements to scalar */
    bool operator==(double v) const;

    /** Compare two vectors */
    bool operator==(const CVector& v) const;

    /** @return Maximum value */
    double max() const;

    /** @return Minimum value */
    double min() const;

    /** @return Euclidean norm */
    double norm2() const;

    /** @return Sum of elements */
    double sum() const;

    /** @return Maximum absolute value */
    double abs_max() const;

    /** @return Index of maximum absolute value */
    int abs_max_elems() const;

    /** @return Mean of the vector */
    double mean() const;

    /** @return Standard deviation */
    double stdev() const;

    /** @return Transposed version as CMatrix */
    CMatrix T() const;

    /** @return Diagonal matrix with vector as diagonal */
    CMatrix diagmat() const;

    /** @return Logarithm of all elements */
    CVector Log() const;

    /** @return Exponential of all elements */
    CVector Exp() const;

    /** @return Absolute value of all elements */
    CVector abs() const;

    /** @return H-transformed version of vector */
    CVector H() const;

    /** Append another vector */
    CVector append(const CVector& V1);

    /** Append a scalar value */
    CVector append(double d);

    /** Sort in ascending order */
    CVector sort();

    /** @return Integer-casted elements */
    vector<int> Int() const;

    /** @return Indices of negative elements */
    vector<int> negative_elements() const;

    /** @return Indices of elements matching val */
    vector<int> lookup(double val) const;

    /**
     * @brief Extract subvector.
     * @param i Start index (inclusive).
     * @param j End index (exclusive).
     * @return New subvector.
     */
    CVector sub(int i, int j) const;

    /** Check if all elements are finite */
    bool is_finite() const;

    /** Convert vector to comma-separated string */
    string toString() const;
    void print(const string& s) const;

    /** Write to file (raw FILE*) */
    void writetofile(FILE* f) const;

    /** Write to file (by filename, overwrite) */
    void writetofile(const string& filename) const;

    /** Write to file using existing ofstream */
    void writetofile(ofstream& f) const;

    /** Append to file by filename */
    void writetofile_app(const string& filename) const;

};

// Non-member operator overloads and utility functions
CVector operator+(const CVector&, const CVector&);
CVector operator+(double, const CVector&);
CVector operator+(const CVector&, double);
CVector operator-(const CVector&, const CVector&);
CVector operator-(double, const CVector&);
CVector operator*(const CVector&, const CVector&);
CVector operator*(double, const CVector&);
CVector operator/(const CVector&, double);
CVector operator/(const CVector&, const CVector&);
CVector operator/(double, const CVector&);

CVector Log(const CVector&);
CVector Exp(const CVector&);
CVector abs(const CVector&);
double abs_max(const CVector&);
double min(const CVector&);
double max(const CVector&);
CVector H(const CVector&);
double H(double x);

CVector zeros(int i);
CVector combinesort(const CVector& V1, const CVector& V2);
CVector combinesort_s(const CVector& V1, const CVector& V2);
double avg(const CVector&);
double stdev(const CVector&);
vector<double> create_vector(int i);
vector<vector<double>> create_vector(int i, int j);
double dotproduct(const CVector& v1, const CVector& v2);
void writetofile(const std::vector<CVector>& data,
                 const std::vector<std::string>& columnlabels,
                 const std::vector<std::string>& rowlabels,
                 const std::string& filename);

#endif CVector_refactored
