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


// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"

class QVariant;
#ifdef QT_version
#include <QMap>
#endif

#include "Vector.h"
#include "Matrix_arma.h"
#include "Vector_arma.h"

using namespace arma;
class CVector;

/**
 * @brief A matrix class wrapping a 2D structure of CVectors.
 */
class CMatrix
{
    friend class D5Matrix;
private:
    int range(int);  ///< Range-check helper

public:
    int numrows() const { return static_cast<int>(matr.size()); }
    int numcols() const { return matr.empty() ? 0 : matr[0].size(); }
    std::vector<CVector> matr; ///< Matrix data stored row-wise

    /// Constructors
    CMatrix(int rows, int cols);
    CMatrix(int size);
    CMatrix();
    CMatrix(std::string filename);
    CMatrix(const CMatrix&);
    CMatrix(CMatrix_arma&);
    CMatrix(const CVector&);

    /// Accessors
    CVector& operator[](int row);
    const CVector& operator[](int row) const;
    double& operator()(int i, int j);
    int getnumrows() const;
    int getnumcols() const;

    /// Assignment and operators
    virtual ~CMatrix();
    CMatrix& operator=(const CMatrix&);
    CMatrix& operator+=(const CMatrix&);
    CMatrix& operator-=(const CMatrix&);
    CMatrix& operator=(mat&);

    /// Matrix multiplication (friend)
    friend CMatrix mult(const CMatrix&, const CMatrix&);
    friend CVector mult(const CMatrix&, const CVector&);
    friend CVector mult(const CVector&, const CMatrix&);

    /// Gaussian elimination routines
    friend void triangulate(CMatrix&, CVector&);
    friend void backsubst(CMatrix&, CVector&, CVector&);
    friend CVector gauss0(CMatrix, CVector);

    /// Utility functions
    friend CVector diag(const CMatrix&);
    CVector maxelements() const;
    friend CMatrix Cholesky_factor(const CMatrix&);
    friend CMatrix LU_decomposition(const CMatrix&);
    CMatrix LU_decomposition();
    CMatrix Cholesky_factor() const;
    double det();

    /// File and console output
    void Print(FILE* f) const;
    void print(const std::string &s) const;
    void writetofile(FILE* f) const;
    void writetofile(const std::string &filename) const;
    void writetofile_app(const std::string &filename) const;

    /// Matrix modification
    void setval(double a);
    void setvaldiag(double a);
    void ScaleDiagonal(double x);

    /// Diagnostic and formatting
    CVector diag_ratio() const;
    CVector diagvector() const;
    std::vector<std::vector<bool>> non_posdef_elems(double tol = 1) const;
    CMatrix non_posdef_elems_m(double tol = 1) const;
    CMatrix Preconditioner(double tol = 1) const;

    /// Export as string or HTML
    std::vector<std::string> toString(std::string format = "", std::vector<std::string> columnHeaders = {}, std::vector<std::string> rowHeaders = {}) const;
    std::vector<std::string> toHtml(std::string format = "", std::vector<std::string> columnHeaders = {}, std::vector<std::string> rowHeaders = {});




#ifdef QT_version
    QMap<QString, QVariant> compact() const;
    static CMatrix unCompact(QMap<QString, QVariant>);
#endif
};

/// Utility matrix functions
double det(const CMatrix&);
double rcond(const CMatrix&);
CMatrix Log(const CMatrix&);
CMatrix Exp(const CMatrix&);
CMatrix Sqrt(const CMatrix&);
CMatrix Transpose(const CMatrix&);
CMatrix Invert(const CMatrix&);
CMatrix Invert(const CMatrix*);
bool Invert(const CMatrix&, CMatrix&);
bool Invert(const CMatrix*, CMatrix*);

CMatrix operator+(const CMatrix&, const CMatrix&);
CMatrix operator+(double, const CMatrix&);
CMatrix operator+(const CMatrix&, double);
CMatrix operator-(double, const CMatrix&);
CMatrix operator+(const CMatrix&, double);
CMatrix operator-(const CMatrix&, double);
CMatrix operator/(const CMatrix&, double);
CMatrix operator/(double, const CMatrix&);
CMatrix operator-(const CMatrix&, const CMatrix&);
CMatrix operator*(const CMatrix&, const CMatrix&);
CVector operator*(const CMatrix&, const CVector&);
CMatrix operator*(const CVector&, const CMatrix&);
CMatrix operator*(double, const CMatrix&);
CVector operator/(CVector&, const CMatrix&);
CVector operator/(const CVector&, const CMatrix&);

CMatrix Identity(int rows);
CMatrix oneoneprod(const CMatrix&, const CMatrix&);
CVector solve_ar(const CMatrix&, const CVector&);
CMatrix inv(const CMatrix&);
CVector maxelements(const CMatrix&);
CMatrix normalize_diag(const CMatrix&, const CMatrix&);
CVector normalize_diag(const CVector&, const CMatrix&);
CVector normalize_diag(const CVector&, const CVector&);
CMatrix normalize_max(const CMatrix&, const CMatrix&);
CVector normalize_max(const CVector&, const CMatrix&);
CVector normalize_max(const CVector&, const CVector&);
CMatrix Average(const std::vector<CMatrix>&);
void write_to_file(const std::vector<CMatrix>&, const std::string&);
CMatrix Cholesky_factor(const CMatrix &M);

