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


// Matrix.cpp: implementation of the CMatrix_arma class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix_arma.h"
#include "Vector_arma.h"
#include "Matrix.h"
#include "Vector.h"
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <iomanip>

using namespace std;

// Default constructor
CMatrix_arma::CMatrix_arma() : arma::mat() {}

// Constructor with dimensions
CMatrix_arma::CMatrix_arma(int r, int c) : arma::mat(r, c, fill::zeros) {}
CMatrix_arma::CMatrix_arma(int n) : arma::mat(n, n, arma::fill::zeros) {}
// Constructor from arma::mat
CMatrix_arma::CMatrix_arma(const mat& m) : arma::mat(m) {}

// Constructor from CMatrix
CMatrix_arma::CMatrix_arma(const CMatrix& m) {
    set_size(m.getnumrows(), m.getnumcols());
    for (int i = 0; i < m.getnumrows(); ++i)
        for (int j = 0; j < m.getnumcols(); ++j)
            (*this)(i, j) = m.matr[i][j];
}

// Constructor from CVector (treated as column vector)
CMatrix_arma::CMatrix_arma(const CVector& v) {
    set_size(static_cast<int>(v.size()), 1);
    for (size_t i = 0; i < v.size(); ++i)
        (*this)(i, 0) = v[i];
}

// Accessor functions
int CMatrix_arma::numrows() const { return static_cast<int>(n_rows); }
int CMatrix_arma::numcols() const { return static_cast<int>(n_cols); }
int CMatrix_arma::getnumrows() const { return numrows(); }
int CMatrix_arma::getnumcols() const { return numcols(); }

double* CMatrix_arma::get(int i, int j) {
    if (i >= 0 && i < static_cast<int>(n_rows) && j >= 0 && j < static_cast<int>(n_cols))
        return &(*this)(i, j);
    throw out_of_range("CMatrix_arma::get(i,j): index out of bounds");
}

double* CMatrix_arma::get(int i) {
    if (i >= 0 && i < static_cast<int>(n_rows))
        return &(*this)(i, 0);
    throw out_of_range("CMatrix_arma::get(i): row index out of bounds");
}

// Arithmetic, comparison, and basic linear algebra

// Assignment operators
CMatrix_arma& CMatrix_arma::operator=(const CMatrix_arma& other) {
    if (this != &other)
        arma::mat::operator=(other);
    return *this;
}

CMatrix_arma& CMatrix_arma::operator=(const mat& other) {
    arma::mat::operator=(other);
    return *this;
}

// Arithmetic compound assignment
CMatrix_arma& CMatrix_arma::operator+=(const CMatrix_arma& rhs) {
    *this += static_cast<const mat&>(rhs);
    return *this;
}

CMatrix_arma& CMatrix_arma::operator-=(const CMatrix_arma& rhs) {
    *this -= static_cast<const mat&>(rhs);
    return *this;
}

CMatrix_arma& CMatrix_arma::operator*=(double val) {
    *this *= val;
    return *this;
}

CMatrix_arma& CMatrix_arma::operator/=(double val) {
    *this /= val;
    return *this;
}

// Equality operator
bool CMatrix_arma::operator==(const CMatrix_arma& rhs) const {
    if (numrows() != rhs.numrows() || numcols() != rhs.numcols())
        return false;

    const double tol = 1e-10;
    for (size_t i = 0; i < numrows(); ++i)
        for (size_t j = 0; j < numcols(); ++j)
            if (std::abs((*this)(i, j) - rhs(i, j)) > tol)
                return false;

    return true;
}

// Matrix inverse
CMatrix_arma CMatrix_arma::inv() const {
    CMatrix_arma result;
    result = arma::inv(*this);
    return result;
}

// Matrix transpose
CMatrix_arma CMatrix_arma::t() const {
    return CMatrix_arma(static_cast<const arma::mat&>(*this).t());
}

// Extract diagonal as vector
CVector CMatrix_arma::diag() const {
    CVector out(n_rows);
    for (size_t i = 0; i < std::min(n_rows, n_cols); ++i)
        out[i] = (*this)(i, i);
    return out;
}

// Redundant diagvector() for compatibility
CVector CMatrix_arma::diagvector() const {
    return diag();
}

//Matrix editing functions

void CMatrix_arma::setval(double a) {
    this->fill(a);
}

void CMatrix_arma::setvaldiag(double a) {
    for (size_t i = 0; i < std::min(n_rows, n_cols); ++i)
        (*this)(i, i) = a;
}

void CMatrix_arma::ScaleDiagonal(double x) {
    for (size_t i = 0; i < std::min(n_rows, n_cols); ++i)
        (*this)(i, i) *= x;
}

void CMatrix_arma::setnumcolrows() {
    // No-op: n_rows and n_cols are always up to date in arma::mat
}

void CMatrix_arma::setrow(int i, const CVector_arma& v) {
    if (i < 0 || static_cast<size_t>(i) >= n_rows || v.n_elem != n_cols)
        throw std::invalid_argument("setrow: size mismatch or index out of bounds");
    for (size_t j = 0; j < n_cols; ++j)
        (*this)(i, j) = v(j);
}

void CMatrix_arma::setrow(int i, const CVector& v) {
    if (i < 0 || static_cast<size_t>(i) >= n_rows || v.size() != n_cols)
        throw std::invalid_argument("setrow: size mismatch or index out of bounds");
    for (size_t j = 0; j < n_cols; ++j)
        (*this)(i, j) = v[j];
}

void CMatrix_arma::setcol(int j, const CVector_arma& v) {
    if (j < 0 || static_cast<size_t>(j) >= n_cols || v.n_elem != n_rows)
        throw std::invalid_argument("setcol: size mismatch or index out of bounds");
    for (size_t i = 0; i < n_rows; ++i)
        (*this)(i, j) = v(i);
}

void CMatrix_arma::setcol(int j, const CVector& v) {
    if (j < 0 || static_cast<size_t>(j) >= n_cols || v.size() != n_rows)
        throw std::invalid_argument("setcol: size mismatch or index out of bounds");
    for (size_t i = 0; i < n_rows; ++i)
        (*this)(i, j) = v[i];
}

CVector CMatrix_arma::diag_ratio() const {
    CVector X(n_cols);
    for (size_t i = 0; i < n_cols; ++i) {
        double off_diag_sum = 0.0;
        for (size_t j = 0; j < n_rows; ++j) {
            if (i != j)
                off_diag_sum += std::abs((*this)(i, j));
        }
        X[i] = off_diag_sum / (*this)(i, i);
    }
    return X;
}

CMatrix_arma CMatrix_arma::LU_decomposition() const {
    mat L, U, P;
    arma::lu(L, U, P, *this);
    return CMatrix_arma(U);
}

CMatrix_arma CMatrix_arma::Cholesky_factor() const {
    mat L;
    if (!arma::chol(L, *this))
        throw std::runtime_error("Cholesky decomposition failed: matrix is not positive definite.");
    return CMatrix_arma(L);
}

std::vector<std::vector<bool>> CMatrix_arma::non_posdef_elems(double tol) const {
    std::vector<std::vector<bool>> mask(n_rows, std::vector<bool>(n_cols, false));
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            if (std::abs((*this)(i, j) / (*this)(i, i)) > tol && i != j)
                mask[i][j] = true;
        }
    }
    return mask;
}

CMatrix_arma CMatrix_arma::non_posdef_elems_m(double tol) const {
    CMatrix_arma out(n_rows, n_cols);
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            if (std::abs((*this)(i, j) / (*this)(i, i)) > tol && i != j)
                out(i, j) = (*this)(i, j);
        }
    }
    return out;
}

CMatrix_arma CMatrix_arma::Preconditioner(double tol) const {
    CMatrix_arma P = this->non_posdef_elems_m(tol);
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            if (P(i, j) != 0 && i != j)
                P(i, j) = -P(i, j);
        }
    }
    for (size_t i = 0; i < std::min(n_rows, n_cols); ++i)
        P(i, i) = 1.0;
    return P;
}

//File Output and Formatting Utilities

void CMatrix_arma::Print(FILE* f) const {
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j)
            fprintf(f, "%le ", (*this)(i, j));
        fprintf(f, "\n");
    }
    fclose(f);
}

void CMatrix_arma::writetofile(FILE* f) const {
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j)
            fprintf(f, "%le, ", (*this)(i, j));
        fprintf(f, "\n");
    }
}

void CMatrix_arma::writetofile(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "w");
    if (f) {
        writetofile(f);
        fclose(f);
    }
}

void CMatrix_arma::writetofile_app(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "a");
    if (f) {
        writetofile(f);
        fclose(f);
    }
}

void CMatrix_arma::writetofile(ofstream& f) const {
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            f << (*this)(i, j);
            if (j < n_cols - 1) f << ", ";
        }
        f << "\n";
    }
}

void CMatrix_arma::print(const string& title) const {
    if (!title.empty()) cout << title << " =\n";
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j)
            cout << std::setw(12) << (*this)(i, j) << " ";
        cout << "\n";
    }
}

std::vector<std::string> CMatrix_arma::toString(std::string format, std::vector<std::string> columnHeaders, std::vector<std::string> rowHeaders) const {
    std::vector<std::string> r;
    bool rowH = !rowHeaders.empty() && rowHeaders.size() == numrows();
    bool colH = !columnHeaders.empty() && columnHeaders.size() == numcols();

    int rowOffset = colH ? 1 : 0;
    int colOffset = rowH ? 1 : 0;
    r.resize(numrows() + rowOffset);

    // First row (column headers)
    if (colH) {
        if (rowH) r[0] += ", ";
        for (int j = 0; j < numcols(); ++j) {
            r[0] += columnHeaders[j];
            if (j < numcols() - 1) r[0] += ", ";
        }
    }

    // Data rows
    for (int i = 0; i < numrows(); ++i) {
        if (rowH) {
            r[i + rowOffset] += rowHeaders[i] + ", ";
        }
        for (int j = 0; j < numcols(); ++j) {
            std::ostringstream streamObj;

            if (!format.empty()) {
                int precision = std::stoi(format);  // Expecting format as precision, e.g., "6"
                streamObj << std::fixed << std::setprecision(precision);
            }

            streamObj << (*this)(i, j);
            r[i + rowOffset] += streamObj.str();
            if (j < numcols() - 1) r[i + rowOffset] += ", ";
        }
    }

    return r;
}

//Free utility functions for CMatrix_arma

CMatrix_arma Log(const CMatrix_arma& M) {
    CMatrix_arma out(M.n_rows, M.n_cols);
    for (size_t i = 0; i < M.n_rows; ++i)
        for (size_t j = 0; j < M.n_cols; ++j)
            out(i, j) = std::log(M(i, j));
    return out;
}

CMatrix_arma Exp(const CMatrix_arma& M) {
    CMatrix_arma out(M.n_rows, M.n_cols);
    for (size_t i = 0; i < M.n_rows; ++i)
        for (size_t j = 0; j < M.n_cols; ++j)
            out(i, j) = std::exp(M(i, j));
    return out;
}

CMatrix_arma Sqrt(const CMatrix_arma& M) {
    CMatrix_arma out(M.n_rows, M.n_cols);
    for (size_t i = 0; i < M.n_rows; ++i)
        for (size_t j = 0; j < M.n_cols; ++j)
            out(i, j) = std::sqrt(M(i, j));
    return out;
}

CMatrix_arma Transpose(const CMatrix_arma& M) {
    return M.t();
}

CMatrix_arma Invert(const CMatrix_arma& M) {
    return M.inv();
}

bool Invert(const CMatrix_arma& M, CMatrix_arma& InvOut) {
    mat result;
    bool ok = arma::inv(result, M);
    if (ok) InvOut = result;
    return ok;
}

CMatrix_arma oneoneprod(const CMatrix_arma& A, const CMatrix_arma& B) {
    if (A.n_rows != B.n_rows || A.n_cols != B.n_cols)
        throw std::invalid_argument("oneoneprod: size mismatch");
    CMatrix_arma C(A.n_rows, A.n_cols);
    for (size_t i = 0; i < A.n_rows; ++i)
        for (size_t j = 0; j < A.n_cols; ++j)
            C(i, j) = A(i, j) * B(i, j);
    return C;
}

CMatrix_arma Average(const std::vector<CMatrix_arma>& M) {
    if (M.empty()) return CMatrix_arma();
    CMatrix_arma avg(M[0]);
    for (size_t k = 1; k < M.size(); ++k) avg += M[k];
    avg /= static_cast<double>(M.size());
    return avg;
}

CVector solve_ar(const CMatrix_arma& A, const CVector& b) {
    mat B(b.size(), 1);
    for (size_t i = 0; i < b.size(); ++i) B(i, 0) = b[i];
    mat X;
    if (!arma::solve(X, A, B))
        throw std::runtime_error("solve_ar failed: matrix may be singular");
    CVector out(b.size());
    for (size_t i = 0; i < b.size(); ++i) out[i] = X(i, 0);
    return out;
}

double det(const CMatrix_arma& M) {
    return arma::det(M);
}

double rcond(const CMatrix_arma& M) {
    arma::vec s;
    arma::svd(s, M);
    return s.min() / s.max();  // reciprocal condition number
}

CVector normalize_diag(const CVector& V, const CMatrix_arma& M2) {
    CVector out(V);
    CVector D = M2.diag();
    for (size_t i = 0; i < V.size(); ++i)
        out[i] = V[i] / D[i];
    return out;
}

CVector normalize_diag(const CVector& V, const CVector& D) {
    CVector out(V);
    for (size_t i = 0; i < V.size(); ++i)
        out[i] = V[i] / D[i];
    return out;
}

CMatrix_arma normalize_diag(const CMatrix_arma& A, const CMatrix_arma& B) {
    CMatrix_arma out(A);
    CVector D = B.diag();
    for (size_t i = 0; i < A.n_cols; ++i)
        for (size_t j = 0; j < A.n_rows; ++j)
            out(j, i) = A(j, i) / D[i];
    return out;
}

CVector normalize_max(const CVector& V, const CVector& D) {
    CVector out(V);
    for (size_t i = 0; i < V.size(); ++i)
        out[i] = V[i] / D[i];
    return out;
}

CVector normalize_max(const CVector& V, const CMatrix_arma& M) {
    return normalize_max(V, M.maxelements());
}

CMatrix_arma normalize_max(const CMatrix_arma& A, const CMatrix_arma& B) {
    CMatrix_arma out(A);
    CVector D = B.maxelements();
    for (size_t i = 0; i < A.n_cols; ++i)
        for (size_t j = 0; j < A.n_rows; ++j)
            out(j, i) = A(j, i) / D[i];
    return out;
}

void write_to_file(const std::vector<CMatrix_arma>& M, const std::string& filename) {
    ofstream f(filename);
    CMatrix_arma avg = Average(M);
    for (const auto& mat : M) {
        for (size_t i = 0; i < mat.n_rows; ++i) {
            for (size_t j = 0; j < mat.n_cols; ++j)
                f << mat(i, j) << ", ";
            f << "\n";
        }
        f << "\n";
    }
    std::vector<std::string> lines = avg.toString();
    f << "Average:\n";
    for (const auto& line : lines) {
        f << line << "\n";
    }
    f.close();
}

// Arithmetic operators for matrix and scalar

CMatrix_arma& CMatrix_arma::operator+=(double val) {
    for (size_t i = 0; i < n_rows; ++i)
        for (size_t j = 0; j < n_cols; ++j)
            (*this)(i, j) += val;
    return *this;
}


CMatrix_arma& CMatrix_arma::operator-=(double val) {
    for (size_t i = 0; i < n_rows; ++i)
        for (size_t j = 0; j < n_cols; ++j)
            (*this)(i, j) -= val;
    return *this;
}


CMatrix_arma operator+(const CMatrix_arma& A, double d) {
    CMatrix_arma out(A);
    out += d;
    return out;
}

CMatrix_arma operator-(const CMatrix_arma& A, double d) {
    CMatrix_arma out(A);
    out -= d;
    return out;
}

CMatrix_arma operator+(double d, const CMatrix_arma& A) {
    return A + d;
}

CMatrix_arma operator-(double d, const CMatrix_arma& A) {
    CMatrix_arma out(A.n_rows, A.n_cols);
    for (size_t i = 0; i < A.n_rows; ++i)
        for (size_t j = 0; j < A.n_cols; ++j)
            out(i, j) = d - A(i, j);
    return out;
}

CMatrix_arma operator/(const CMatrix_arma& A, double d) {
    CMatrix_arma out(A);
    out /= d;
    return out;
}

CMatrix_arma operator/(double d, const CMatrix_arma& A) {
    CMatrix_arma out(A.n_rows, A.n_cols);
    for (size_t i = 0; i < A.n_rows; ++i)
        for (size_t j = 0; j < A.n_cols; ++j)
            out(i, j) = d / A(i, j);
    return out;
}

CMatrix_arma Identity_ar(int n) {
    return CMatrix_arma(arma::eye<mat>(n, n));
}

CVector_arma SpareSolve(const CMatrix_arma& A, const CVector_arma& b) {
    arma::sp_mat A_sparse(A);  // convert to sparse
    CVector_arma x;
    if (!arma::spsolve(x, A_sparse, b))
        throw std::runtime_error("SpareSolve failed: spsolve did not converge.");
    return x;

}

CVector CMatrix_arma::maxelements() const {
    CVector out(n_cols);
    for (size_t j = 0; j < n_cols; ++j) {
        double maxval = 0.0;
        for (size_t i = 0; i < n_rows; ++i)
            maxval = std::max(maxval, std::abs((*this)(i, j)));
        out[j] = maxval;
    }
    return out;
}


CMatrix_arma operator*(const CMatrix_arma& A, const CMatrix_arma& B) {
    if (A.n_cols != B.n_rows)
        throw std::invalid_argument("operator*: incompatible matrix sizes for multiplication");
    return CMatrix_arma(static_cast<arma::mat>(A) * static_cast<arma::mat>(B));
}

CMatrix_arma operator*(const CMatrix_arma& A, double x) {
    return CMatrix_arma(static_cast<arma::mat>(A) * x);
}

CMatrix_arma operator*(double x, const CMatrix_arma& A) {
    return CMatrix_arma(static_cast<arma::mat>(A) * x);
}


