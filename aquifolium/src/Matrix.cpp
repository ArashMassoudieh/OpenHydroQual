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


// Matrix.cpp: implementation of the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#define ARMA_DONT_PRINT_ERRORS
#include "Vector.h"
#include "Expression.h"
#ifdef Q_JSON_SUPPORT
#include "qstring.h"
#include "qmap.h"
#include "qvariant.h"
#endif

using namespace std;
using namespace arma;

// Accessor implementations
CVector& CMatrix::operator[](int i) { return matr[i]; }
const CVector& CMatrix::operator[](int i) const { return matr[i]; }
double& CMatrix::operator()(int i, int j) { return matr[i][j]; }
int CMatrix::getnumrows() const { return numrows(); }
int CMatrix::getnumcols() const { return numcols(); }

// Const-qualified member function
void CMatrix::Print(FILE* f) const {
    for (int i = 0; i < numrows(); i++) {
        for (int j = 0; j < numcols(); j++)
            fprintf(f, "%le ", matr[i][j]);
        fprintf(f, "\n");
    }
    fclose(f);
}

void CMatrix::writetofile(FILE* f) const {
    for (int i = 0; i < numrows(); i++) {
        for (int j = 0; j < numcols(); j++)
            fprintf(f, "%le, ", matr[i][j]);
        fprintf(f, "\n");
    }
}

void CMatrix::writetofile(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "w");
    writetofile(f);
    fclose(f);
}

void CMatrix::writetofile_app(const string& filename) const {
    FILE* f = fopen(filename.c_str(), "a");
    writetofile(f);
    fclose(f);
}

void CMatrix::print(const string& s) const {
    ofstream Afile(s + ".txt");
    cout << s << "=" << endl;
    for (int i = 0; i < numrows(); ++i) {
        for (int j = 0; j < numcols(); ++j) {
            Afile << matr[i][j] << ", ";
            cout << matr[i][j] << ", ";
        }
        Afile << "\n";
        cout << "\n";
    }
}

void CMatrix::setval(double a) {
    for (int i = 0; i < numrows(); i++)
        for (int j = 0; j < numcols(); j++)
            matr[i][j] = a;
}

void CMatrix::setvaldiag(double a) {
    for (int i = 0; i < getnumrows(); i++)
        matr[i][i] = a;
}

void CMatrix::ScaleDiagonal(double x) {
    for (int i = 0; i < getnumcols(); i++)
        matr[i][i] *= x;
}


CVector CMatrix::diag_ratio() const {
    CVector X(numcols());
    CVector maxs(numcols());
    for (int i = 0; i < numcols(); i++) {
        for (int j = 0; j < numrows(); j++)
            if (i != j) maxs[i] += fabs(matr[i][j]);
        X[i] = maxs[i] / matr[i][i];
    }
    return X;
}

CVector CMatrix::diagvector() const {
    CVector X(numcols());
    for (int i = 0; i < numcols(); i++)
        X[i] = matr[i][i];
    return X;
}

CMatrix CMatrix::non_posdef_elems_m(double tol) const {
    CMatrix M(getnumcols(), getnumrows());
    for (int i = 0; i < getnumcols(); i++)
        for (int j = 0; j < getnumrows(); j++)
            if (matr[i][j] / matr[i][i] > tol)
                M[i][j] = matr[i][j];
    return M;
}

vector<vector<bool>> CMatrix::non_posdef_elems(double tol) const {
    vector<vector<bool>> M(getnumcols(), vector<bool>(getnumrows()));
    for (int i = 0; i < getnumcols(); i++)
        for (int j = 0; j < getnumrows(); j++)
            if (matr[i][j] / matr[i][i] > tol)
                M[i][j] = true;
    return M;
}

CMatrix CMatrix::Preconditioner(double tol) const {
    CMatrix M = non_posdef_elems_m(tol) + Identity(numcols());
    for (int i = 0; i < getnumcols(); i++)
        for (int j = 0; j < getnumrows(); j++)
            if (M[i][j] != 0 && i != j)
                M[i][j] = -M[i][j];
    return M;
}

CVector CMatrix::maxelements() const {
    CVector v(getnumcols());
    for (int i = 0; i < getnumcols(); ++i) {
        double maxval = -1e36;
        for (int j = 0; j < getnumrows(); ++j)
            maxval = max(fabs(matr[i][j]), maxval);
        v[i] = maxval;
    }
    return v;
}

CMatrix::CMatrix(int m, int n) {
    matr.resize(m);
    for (auto& row : matr) row.resize(n);
}

CMatrix::CMatrix() : matr() {}

CMatrix::CMatrix(int m) : CMatrix(m, m) {}

CMatrix::CMatrix(const string filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    vector<vector<double>> values;
    while (getline(file, line)) {
        stringstream ss(line);
        vector<double> row;
        string val;
        while (getline(ss, val, '\t')) row.push_back(stod(val));
        if (!row.empty()) values.push_back(row);
    }

    matr.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i)
        matr[i] = CVector(values[i]);
}

CMatrix::CMatrix(const CMatrix& m) : matr(m.matr) {}

CMatrix::CMatrix(CMatrix_arma& M) {
    matr.resize(M.getnumrows());
    for (int i = 0; i < M.getnumrows(); ++i) {
        matr[i].resize(M.getnumcols());
        for (int j = 0; j < M.getnumcols(); ++j)
            matr[i][j] = M(i, j);
    }
}

CMatrix::CMatrix(const CVector& v) {
    matr.resize(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        matr[i].resize(1);
        matr[i][0] = v[i];
    }
}

CMatrix::~CMatrix() = default;

CMatrix& CMatrix::operator=(const CMatrix& m) {
    if (this != &m) matr = m.matr;
    return *this;
}

CMatrix& CMatrix::operator+=(const CMatrix& m) {
    for (size_t i = 0; i < matr.size(); ++i) matr[i] += m.matr[i];
    return *this;
}

CMatrix& CMatrix::operator-=(const CMatrix& m) {
    for (size_t i = 0; i < matr.size(); ++i) matr[i] -= m.matr[i];
    return *this;
}

CMatrix& CMatrix::operator=(mat& A) {
    matr.resize(A.n_rows);
    for (size_t i = 0; i < A.n_rows; ++i) {
        matr[i].resize(A.n_cols);
        for (size_t j = 0; j < A.n_cols; ++j)
            matr[i][j] = A(i, j);
    }
    return *this;
}

CVector diag(const CMatrix& m) {
    int n = std::min(m.getnumrows(), m.getnumcols());
    CVector v(n);
    for (int i = 0; i < n; ++i)
        v[i] = m[i][i];
    return v;
}

CVector gauss0(CMatrix M, CVector V) {
    CVector b(M.getnumrows());
    triangulate(M, V);
    backsubst(M, V, b);
    return b;
}

void triangulate(CMatrix& m, CVector& v) {
    int n = m.getnumrows();
    for (int i = 0; i < n - 1; i++) {
        double diag = m[i][i];
        for (int j = i + 1; j < n; j++) {
            double p = m[j][i] / diag;
            m[j] -= p * m[i];
            v[j] -= p * v[i];
        }
    }
}

void backsubst(CMatrix& a, CVector& b, CVector& x) {
    int n = a.getnumrows();
    for (int i = n - 1; i >= 0; i--) {
        double diag = a[i][i];
        x[i] = (b[i] - dotproduct(x, a[i])) / diag;
    }
}

CVector operator/(CVector& V, const CMatrix& M) {
    return solve_ar(M, V);
}

CVector operator/(const CVector& V, const CMatrix& M) {
    return solve_ar(M, V);
}

CMatrix operator+(const CMatrix& m1, const CMatrix& m2) {
    CMatrix result = m1;
    result += m2;
    return result;
}

CMatrix operator-(const CMatrix& m1, const CMatrix& m2) {
    CMatrix result = m1;
    result -= m2;
    return result;
}

CMatrix operator*(const CMatrix& m1, const CMatrix& m2) {
    int nr = m1.getnumrows();
    int nc = m2.getnumcols();
    int inner = m1.getnumcols();
    CMatrix result(nr, nc);
    for (int i = 0; i < nr; ++i)
        for (int j = 0; j < nc; ++j)
            for (int k = 0; k < inner; ++k)
                result[i][j] += m1[i][k] * m2[k][j];
    return result;
}

CVector operator*(const CMatrix& m, const CVector& v) {
    int nr = m.getnumrows();
    CVector result(nr);
    for (int i = 0; i < nr; ++i)
        for (int j = 0; j < v.size(); ++j)
            result[i] += m[i][j] * v[j];
    return result;
}

CMatrix operator*(const CVector& v, const CMatrix& m) {
    return CMatrix(v) * m;
}

CMatrix operator*(double d, const CMatrix& m) {
    CMatrix result(m);
    for (int i = 0; i < result.getnumrows(); ++i)
        for (int j = 0; j < result.getnumcols(); ++j)
            result[i][j] *= d;
    return result;
}

CMatrix operator+(double d, const CMatrix& m) {
    CMatrix result(m);
    for (int i = 0; i < result.getnumrows(); ++i)
        for (int j = 0; j < result.getnumcols(); ++j)
            result[i][j] += d;
    return result;
}

CMatrix operator-(double d, const CMatrix& m) {
    CMatrix result(m);
    for (int i = 0; i < result.getnumrows(); ++i)
        for (int j = 0; j < result.getnumcols(); ++j)
            result[i][j] = d - result[i][j];
    return result;
}

CMatrix operator-(const CMatrix& m, double d) {
    CMatrix result(m);
    for (int i = 0; i < result.getnumrows(); ++i)
        for (int j = 0; j < result.getnumcols(); ++j)
            result[i][j] -= d;
    return result;
}

CMatrix operator+(const CMatrix& m, double d) {
    return d + m;
}

CMatrix operator/(const CMatrix& m, double d) {
    return (1.0 / d) * m;
}

CMatrix operator/(double d, const CMatrix& m) {
    CMatrix result(m);
    for (int i = 0; i < result.getnumrows(); ++i)
        for (int j = 0; j < result.getnumcols(); ++j)
            result[i][j] = d / result[i][j];
    return result;
}

vector<string> CMatrix::toString(string format, vector<string> columnHeaders, vector<string> rowHeaders) const
{
	vector<string> r;
	bool rowH = false, colH = false;
	int rowOffset = 0, colOffset=0;
    if (columnHeaders.size() && int(columnHeaders.size()) == numcols())
	{
		colH = true;
		rowOffset = 1;
	}
    if (rowHeaders.size() && int(rowHeaders.size()) == numrows())
	{
		rowH = true;
		colOffset = 1;
	}
    r.resize(numrows() + rowOffset);


	if (colH)
	{
		if (rowH) r[0] += "\, ";
        for (int j = 0; j<numcols(); j++)
		{
			r[0] += columnHeaders[j];
            if (j < numcols() - 1) r[0] += "\, ";
		}

	}
    for (int i = 0; i<numrows(); i++)
	{
		if (rowH)
		{
			r[i + rowOffset] += rowHeaders[i];
			r[i + rowOffset] += "\, ";
		}
        for (int j = 0; j<numcols(); j++)
		{
            std::ostringstream streamObj;
            streamObj << matr[i].at(j);
            std::string strObj = streamObj.str();
            r[i + rowOffset] += strObj;
            if (j < numcols() - 1) r[i + rowOffset] += "\, ";
		}
	}
	return r;
}

CVector solve_ar(const CMatrix &M, const CVector &V)
{

    mat A(M.getnumrows(),M.getnumcols());
    mat B(V.getsize(),1);

    CVector ansr = V;

    for (int i = 0;i<M.getnumrows(); ++i)
    {
        B(i,0) = V[i];
        for (int j = 0;j<M.getnumcols(); ++j)
            A(i,j) = M[i][j];
    };

    mat C;
    bool out = solve( C, A, B );

    if (out)
    {   for (int i = 0;i<V.getsize(); ++i)
            ansr[i] = C(i,0);

        return ansr;
    }
    else return CVector();
}

CMatrix Identity(int rows)
{
    CMatrix M(rows, rows);
    for (int i = 0; i < rows; i++)
        M[i][i] = 1;

    return M;
}

CMatrix CMatrix::Cholesky_factor() const
{
    int i;
    int j;
    int k;
    double s;
    CMatrix b(getnumcols(), getnumcols());
    int n = getnumcols();
    for ( j = 0; j < n; j++ )
    {	for ( i = 0; i < n; i++ )
        {
            b[i][j] = this->operator[](i).operator[](j);
        }
    }

    for ( j = 0; j < n; j++ )
    {	for ( k = 0; k <= j-1; k++ )
        {
            for ( i = 0; i <= k-1; i++ )
            {	b[k][j] = b[k][j] - b[i][k] * b[i][j];
            }
            b[k][j] = b[k][j] / b[k][k];
        }

        s = b[j][j];
        for ( i = 0; i <= j-1; i++ )
        {
            s = s - b[i][j] * b[i][j];
        }

        b[j][j] = sqrt ( s );
    }
    //
    //  Since the Cholesky factor is in R8GE format, zero out the lower triangle.
    //
    for ( i = 0; i < n; i++ )
    {
        for ( j = 0; j < i; j++ )
        {
            b[i][j] = 0.0;
        }
    }

    return b;
}

CMatrix Cholesky_factor(const CMatrix &M)
{
    return M.Cholesky_factor();
}

CMatrix Invert(CMatrix *M1)
{
    CMatrix InvM(M1->getnumcols(), M1->getnumcols());
    for (int i=0; i<M1->getnumcols(); i++)
    {
        CVector V(M1->getnumcols());
        V[i] = 1;
        CVector invV = V/(*M1);
        if (invV.size()==0)
            return CMatrix();
        InvM[i] = invV;
    }
    return Transpose(InvM);
}


double CMatrix::det()
{
    mat A(numrows(),numcols());

    for (int i = 0;i<numrows(); ++i)
    {
        for (int j = 0;j<numcols(); ++j)
            A(i,j) = this->operator[](i).operator[](j);
    }

    return arma::det(A);
}

bool Invert(const CMatrix &M1,CMatrix &out)
{
    out=Invert(M1);
    if (out.getnumrows()==0)
        return false;
    return true;
}

bool Invert(CMatrix *M1,CMatrix *out)
{
    *out=Invert(M1);
    if (out->getnumrows()==0)
        return false;

    return true;
}

CMatrix Invert(const CMatrix &M1)
{
    CMatrix InvM(M1.getnumcols(), M1.getnumcols());
    for (int i=0; i<M1.getnumcols(); i++)
    {
        CVector V(M1.getnumcols());
        V[i] = 1;
        CVector invV = V/M1;
        if (invV.size()==0)
            return CMatrix();
        InvM[i] = invV;
    }
    return Transpose(InvM);
}

CMatrix Transpose(const CMatrix &M1)
{
    CMatrix TrM(M1.getnumcols(), M1.getnumrows());
    for (int i=0; i<M1.getnumrows(); i++)
        for (int j=0; j<M1.getnumcols(); j++)
            TrM.matr[j][i] = M1.matr[i][j];
    return TrM;
}
