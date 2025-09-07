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


// Matrix.h: interface for the CMatrix_arma class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// Fully restored and refactored Matrix_arma.h with all original methods and Doxygen comments
#pragma once

#include <armadillo>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace arma;
using namespace std;

class CMatrix;
class CVector;
class CVector_arma;

/**
 * @brief A matrix class inheriting from arma::mat with enhanced utilities
 * for numerical modeling and system emulation.
 */
class CMatrix_arma : public arma::mat {
public:
    /// Constructors
    CMatrix_arma();
    CMatrix_arma(int r, int c);
    CMatrix_arma(int n);
    CMatrix_arma(const mat&);
    CMatrix_arma(const CMatrix&);
    CMatrix_arma(const CVector&);
    template<typename T1, typename op_type>
    CMatrix_arma(const arma::Op<T1, op_type>& X) : arma::mat(X) {};

    /// Accessors
    int numrows() const;
    int numcols() const;
    int getnumrows() const;  ///< Alias
    int getnumcols() const;  ///< Alias

    /// Element access
    double* get(int i, int j);
    double* get(int i);

    /// Assignment
    CMatrix_arma& operator=(const CMatrix_arma&);
    CMatrix_arma& operator=(const mat&);
    template<typename T1, typename op_type>
    CMatrix_arma& operator=(const arma::Op<T1, op_type>& X) {
        arma::mat::operator=(X);  // call base assignment
        return *this;
    }


    /// Arithmetic operators
    CMatrix_arma& operator+=(const CMatrix_arma&);
    CMatrix_arma& operator-=(const CMatrix_arma&);
    CMatrix_arma& operator*=(double);
    CMatrix_arma& operator/=(double);
    CMatrix_arma& operator+=(double);
    CMatrix_arma& operator-=(double);

    /// Comparisons
    bool operator==(const CMatrix_arma&) const;

    /// Math operations
    CMatrix_arma inv() const;
    CMatrix_arma t() const;
    CVector diag() const;
    CVector diagvector() const;
    CVector diag_ratio() const;

    /// Decompositions
    CMatrix_arma LU_decomposition() const;
    CMatrix_arma Cholesky_factor() const;

    /// Matrix editing
    void setval(double a);
    void setvaldiag(double a);
    void ScaleDiagonal(double x);
    void setnumcolrows();
    void setrow(int, const CVector_arma&);
    void setrow(int, const CVector&);
    void setcol(int, const CVector_arma&);
    void setcol(int, const CVector&);

    /// Definite matrix diagnostics
    std::vector<std::vector<bool>> non_posdef_elems(double tol = 1.0) const;
    CMatrix_arma non_posdef_elems_m(double tol = 1.0) const;
    CMatrix_arma Preconditioner(double tol = 1.0) const;

    /// Utilities
    void Print(FILE* f) const;
    void writetofile(FILE* f) const;
    void writetofile(const string& filename) const;
    void writetofile(ofstream& f) const;
    void writetofile_app(const string& filename) const;
    void print(const string& title = "") const;
    vector<string> toString(string format = "", vector<string> columnHeaders = vector<string>(), vector<string> rowHeaders = vector<string>()) const;
    std::vector<std::string> toHtml(std::string format = "", std::vector<std::string> colHeaders = {}, std::vector<std::string> rowHeaders = {}) const;

    /// Stats
    CVector maxelements() const;
};

/// Free functions
CMatrix_arma operator+(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma operator-(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma operator*(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma operator*(const CMatrix_arma&, double);
CMatrix_arma operator*(double, const CMatrix_arma&);
CVector_arma operator*(const CMatrix_arma& A, const CVector_arma& x);
CMatrix_arma operator/(const CMatrix_arma&, double);

CMatrix_arma Log(const CMatrix_arma&);
CMatrix_arma Exp(const CMatrix_arma&);
CMatrix_arma Sqrt(const CMatrix_arma&);
CMatrix_arma Transpose(const CMatrix_arma&);
CMatrix_arma Invert(const CMatrix_arma&);
bool Invert(const CMatrix_arma&, CMatrix_arma&);

CVector normalize_diag(const CVector&, const CMatrix_arma&);
CVector normalize_diag(const CVector&, const CVector&);
CMatrix_arma normalize_diag(const CMatrix_arma&, const CMatrix_arma&);

CVector normalize_max(const CVector&, const CVector&);
CVector normalize_max(const CVector&, const CMatrix_arma&);
CMatrix_arma normalize_max(const CMatrix_arma&, const CMatrix_arma&);

CMatrix_arma oneoneprod(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma Average(const std::vector<CMatrix_arma>&);

CVector solve_ar(const CMatrix_arma&, const CVector&);
double det(const CMatrix_arma&);
double rcond(const CMatrix_arma&);

CVector gauss0(CMatrix_arma, CVector);
void triangulate(CMatrix_arma&, CVector&);
void backsubst(CMatrix_arma&, CVector&, CVector&);

void write_to_file(const std::vector<CMatrix_arma>&, const std::string&);
