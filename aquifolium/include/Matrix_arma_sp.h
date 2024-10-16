// Matrix.h: interface for the CMatrix_arma_sp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Vector_arma.h"
#include <iostream>
#include "math.h"
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"
class QVariant;
//class QString;
//class QList;
//#include "QMap"
using namespace arma;
class CVector_arma;
class CMatrix_arma_sp  
{

private:
	int numrows;
	int numcols;
	
public:
	sp_mat matr;
	CMatrix_arma_sp(int, int);
	CMatrix_arma_sp(int);
	CMatrix_arma_sp();
	CMatrix_arma_sp(const CMatrix_arma_sp&);
	CMatrix_arma_sp(const CVector_arma&);
	//CVector_arma operator[](int);
	double& get(int i, int j);
	double& operator()(int i, int j);
	std::vector<double*> get(int i);
	int getnumrows() const;
	int getnumcols() const;
	virtual ~CMatrix_arma_sp();
	CMatrix_arma_sp& operator=(const CMatrix_arma_sp&);
	CMatrix_arma_sp& operator+=(CMatrix_arma_sp&);
	CMatrix_arma_sp& operator-=(CMatrix_arma_sp &);	
	CMatrix_arma_sp& operator=(mat&);
	friend CMatrix_arma_sp mult(CMatrix_arma_sp&, CMatrix_arma_sp&);
	friend CVector_arma mult(CMatrix_arma_sp&, CVector_arma&);
	friend CVector_arma mult(CVector_arma&, CMatrix_arma_sp&);
	friend void triangulate(CMatrix_arma_sp&, CVector_arma&);
	friend void backsubst(CMatrix_arma_sp&, CVector_arma&, CVector_arma&);
	friend CVector_arma gauss0(CMatrix_arma_sp, CVector_arma);
	friend CVector_arma diag(CMatrix_arma_sp);
	friend CMatrix_arma_sp Cholesky_factor(CMatrix_arma_sp &M);
	friend CMatrix_arma_sp LU_decomposition(CMatrix_arma_sp &M);
	CMatrix_arma_sp LU_decomposition();
	CMatrix_arma_sp Cholesky_factor();	
	double det();
	void Print(FILE *FIL);
	void print(std::string s);
	void setval(double a);
	void setvaldiag(double a);
	void writetofile(FILE *f);
	void writetofile(std::string filename);
	void writetofile_app(std::string filename);
	friend void write_to_file(std::vector<CMatrix_arma_sp> M,std::string filename);
	friend CMatrix_arma_sp Average(std::vector<CMatrix_arma_sp> M);
	CVector_arma diag_ratio();
	std::vector<std::vector<bool> > non_posdef_elems(double tol = 1);
	CMatrix_arma_sp non_posdef_elems_m(double tol = 1);
	CMatrix_arma_sp Preconditioner(double tol = 1);
	std::vector<std::string> toString(std::string format = "", std::vector<std::string> columnHeaders = std::vector<std::string>(), std::vector<std::string> rowHeaders = std::vector<std::string>());
	std::vector<std::string> toHtml(std::string format = "", std::vector<std::string> columnHeaders = std::vector<std::string>(), std::vector<std::string> rowHeaders = std::vector<std::string>());
	void setnumcolrows();
	void setrow(int i, CVector_arma &V);
	void setrow(int i, CVector &V);
	void setcol(int i, CVector_arma &V);
	void setcol(int i, CVector &V);
    CVector diagvector();
    void ScaleDiagonal(double x);

	
};
	
double det(CMatrix_arma_sp &);
CMatrix_arma_sp Log(CMatrix_arma_sp &M1);
CMatrix_arma_sp Exp(CMatrix_arma_sp &M1);
CMatrix_arma_sp Sqrt(CMatrix_arma_sp &M1);
CMatrix_arma_sp operator+(const CMatrix_arma_sp&, const CMatrix_arma_sp&);
CMatrix_arma_sp operator+(double, CMatrix_arma_sp);
CMatrix_arma_sp operator+(CMatrix_arma_sp, double);
CMatrix_arma_sp operator-(double d, CMatrix_arma_sp m1);
CMatrix_arma_sp operator+(CMatrix_arma_sp m1, double d);
CMatrix_arma_sp operator-(CMatrix_arma_sp m1,double d);
CMatrix_arma_sp operator/(CMatrix_arma_sp m1,double d);
CMatrix_arma_sp operator/(double d, CMatrix_arma_sp m1);
CMatrix_arma_sp operator-(const CMatrix_arma_sp&, const CMatrix_arma_sp&);
CMatrix_arma_sp operator*(CMatrix_arma_sp&, CMatrix_arma_sp&);
CVector_arma operator*(CMatrix_arma_sp&, CVector_arma&);
CMatrix_arma_sp operator*(CVector_arma, CMatrix_arma_sp);
CMatrix_arma_sp operator*(double, CMatrix_arma_sp);
CVector_arma operator/(CVector_arma&, CMatrix_arma_sp&);
CMatrix_arma_sp Transpose(CMatrix_arma_sp &M1);
CMatrix_arma_sp Invert(CMatrix_arma_sp M1);
CVector_arma SpareSolve(CMatrix_arma_sp, CVector_arma);
CMatrix_arma_sp oneoneprod(CMatrix_arma_sp &m1, CMatrix_arma_sp &m2);
CVector_arma solve_ar(CMatrix_arma_sp&, CVector_arma&);
CMatrix_arma_sp inv(CMatrix_arma_sp&);
CMatrix_arma_sp normalize_diag(CMatrix_arma_sp&, CMatrix_arma_sp&);
CVector_arma normalize_diag(CVector_arma&, CMatrix_arma_sp&);
//CMatrix_arma_sp Identity_ar(int rows);


