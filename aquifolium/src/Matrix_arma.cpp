// Matrix.cpp: implementation of the CMatrix_arma class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix_arma.h"
#include "Vector_arma.h"
#include "math.h"
#include <iostream>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"
//#include "StringOP.h"
//#include "qstring.h"
//#include "qmap.h"
//#include "qvariant.h"
#include <vector>
#include "Vector.h"
#define ARMA_USE_SUPERLU 1

using namespace arma;


using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix_arma::CMatrix_arma(int m, int n)
{
	numrows = m;
	numcols = n;
	matr = mat(m, n);
	matr.fill(fill::zeros);

}

CMatrix_arma::CMatrix_arma()
{
	numrows = 0;
	numcols = 0;
}

CMatrix_arma::CMatrix_arma(int m)
{
	numrows = m;
	numcols = m;
	matr = mat(m, m);
	matr.fill(fill::zeros);
}

CMatrix_arma::CMatrix_arma(const CMatrix_arma &m)
{
	numrows = m.numrows;
	numcols = m.numcols;
	matr = m.matr;

}

CMatrix_arma::CMatrix_arma(const CVector_arma &v)
{
	numrows = v.num;
	numcols = 1;
	matr = mat(numrows,1);

	for (int i=0; i<numrows; ++i)  matr(i,0) = v.vect(i);
}


CMatrix_arma::~CMatrix_arma()
{
	matr.clear();
}

double& CMatrix_arma::get(int i, int j)
{
	return matr(i, j);
}

double & CMatrix_arma::operator()(int i, int j)
{
	return get(i, j);
}

vector<double*> CMatrix_arma::get(int i)
{
	vector<double*> v;
	for (int j = 0; j < numrows; j++)
		v[j] = &matr(i, j);

	return v;
}

int CMatrix_arma::getnumrows() const {return numrows;};
int CMatrix_arma::getnumcols() const {return numcols;};

CMatrix_arma& CMatrix_arma::operator=(const CMatrix_arma &m)
{

	numcols = m.numcols;
	numrows = m.numrows;
	matr = m.matr;

	return *this;
}

CMatrix_arma& CMatrix_arma::operator+=(const CMatrix_arma &m)
{

	for (int i=0; i<numrows; i++)
		matr[i] += m.matr(i);
	return *this;
}

CMatrix_arma& CMatrix_arma::operator-=(const CMatrix_arma &m)
{
	for (int i=0; i<numrows; i++)
		matr[i] -= m.matr(i);
	return *this;
}

void CMatrix_arma::Print(FILE *FIL)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(FIL, "%le ", matr(i,j));
		fprintf(FIL, "\n");
	}
	fclose(FIL);

}

CMatrix_arma operator+(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
	CMatrix_arma mt = m1;
	mt += m2;
	return mt;
}

CMatrix_arma operator-(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
	CMatrix_arma mt = m1;
	mt -= m2;
	return mt;
}

CMatrix_arma mult(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
	CMatrix_arma M;
	M.matr = m1.matr*m2.matr;
    M.setnumcolrows();
	return M;
}

CMatrix_arma operator*(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
	CMatrix_arma a= mult(m1,m2);
	return a;
}


CVector_arma mult(const CMatrix_arma &m1, const CVector_arma &v1)
{
	int nr = m1.getnumrows();
	CVector_arma vt(nr);
	vt.vect = m1.matr*v1.vect;
	vt.num = nr;
	return vt;
}

CMatrix_arma operator*(double d, CMatrix_arma m1)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	TrM.matr = d*m1.matr;
	return TrM;

}

CMatrix_arma operator+(double d, CMatrix_arma m1)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d+m1.get(i,j);
	return TrM;

}

CMatrix_arma operator-(double d, CMatrix_arma m1)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d-m1.get(i,j);
	return TrM;

}

CMatrix_arma operator+(CMatrix_arma m1, double d)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d+m1.get(i,j);
	return TrM;

}

CMatrix_arma operator-(CMatrix_arma m1,double d)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)-d;
	return TrM;

}

CMatrix_arma operator/(CMatrix_arma m1,double d)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)/d;
	return TrM;
}

CMatrix_arma operator/(double d, CMatrix_arma m1)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d/m1.get(i,j);
	return TrM;
}


CVector_arma operator*(const CMatrix_arma &m, const CVector_arma &v)
{
	return mult(m,v);
}

//CVector_arma_arma gauss(CMatrix_arma, CVector_arma_arma)
//{
//}


CVector_arma operator/(CVector_arma &V, CMatrix_arma &M)
{
	CVector_arma X(M.getnumcols());
	bool status = solve( X.vect, M.matr, V.vect);
	if (status == false) X.num = 0;
	return X;
}

CVector_arma operator/(const CVector_arma &V, const CMatrix_arma &M)
{
    CVector_arma X(M.getnumcols());
    bool status = solve( X.vect, M.matr, V.vect);
    if (status == false) X.num = 0;
    return X;
}

CMatrix_arma Log(CMatrix_arma &M1)
{
	CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = log(M1.get(i,j));
	return TrM;
}

CMatrix_arma Exp(CMatrix_arma &M1)
{
	CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = exp(M1.get(i,j));
	return TrM;
}

CMatrix_arma Sqrt(CMatrix_arma &M1)
{
	CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = sqrt(M1.get(i,j));
	return TrM;
}



CMatrix_arma Invert(CMatrix_arma M1)
{
	CMatrix_arma InvM(M1.getnumcols(), M1.getnumcols());
	InvM.matr = inv(M1.matr);
	return InvM;
}


/*double det(CMatrix_arma &A)
{
	CMatrix_arma D = LU_decomposition(A);
	double prod = 1;
	for (int i=0; i<A.getnumcols(); i++)
		prod *= A(i,i);

	return prod;

}
*/

double CMatrix_arma::det()
{
	return arma::det(matr);
}

double CMatrix_arma::rcond()
{
    //return arma::rcond(matr);
	return 0.7; 
}



CVector_arma diag(const CMatrix_arma &m)
{
	CVector_arma v(m.getnumcols());
	for (int i=0; i<m.getnumcols(); ++i)
        v[i] = m.matr(i,i);
    return v;
}

CVector_arma maxelements(const CMatrix_arma &m)
{
    CVector_arma v(m.getnumcols());
    for (int i=0; i<m.getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<m.getnumrows(); ++j)
            maxval = max(fabs(m.matr(i,j)),maxval);
        v[i] = maxval;
    }
    return v;

}

CVector CMatrix_arma::maxelements() const
{
    CVector_arma v(getnumcols());
    for (int i=0; i<getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<getnumrows(); ++j)
            maxval = max(fabs(matr(i,j)),maxval);
        v[i] = maxval;
    }
    return v;
}

CMatrix_arma operator*(const CVector_arma &v, const CMatrix_arma &m)
{
    auto tmpMat = CMatrix_arma(v);
    CMatrix_arma a = tmpMat*m;
	return a;
}


CMatrix_arma oneoneprod(CMatrix_arma &m1, CMatrix_arma &m2)
{
	CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)*m2.get(i,j);
	return TrM;
}

void CMatrix_arma::setval(double a)
{
	for (int i=0; i<numrows ; i++)
		for (int j=0; j<numcols ; j++)
			matr(i,j) = a;


}

void CMatrix_arma::setvaldiag(double a)
{
	for (int i=0; i<getnumrows(); i++)
		matr(i,i) = a;

}

void CMatrix_arma::writetofile(FILE *f)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr(i,j));
		fprintf(f, "\n");
	}
}

void CMatrix_arma::writetofile(string filename)
{
	FILE *f = fopen(filename.c_str(),"w");
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr(i,j));
		fprintf(f, "\n");
	}
	fclose(f);
}

//MM
//void CMatrix_arma::writetofile(string filename)
void CMatrix_arma::writetofile_app(string filename)
{
	FILE *f = fopen(filename.c_str(),"a");
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr(i,j));
		fprintf(f, "\n");
	}
	fclose(f);
}

CMatrix_arma Transpose(CMatrix_arma &M1)	//Works only when M1.getnumcols()=M1.getnumrows()
{
	CMatrix_arma TrM(M1.getnumcols(), M1.getnumrows());
	TrM.matr = M1.matr.t();
	return TrM;
}

void CMatrix_arma::print(string s)
{

	ofstream Afile;
	Afile.open(s+".txt");

	for (int i = 0; i<numrows; ++i)
	{
		for (int j = 0; j<numcols; ++j)
		{
			Afile << matr(i,j) << "\, ";
		}
		Afile << "\n";
	}
}

CVector_arma solve_ar(CMatrix_arma &M, CVector_arma &V)
{

	CVector_arma ansr;
	solve(ansr.vect, M.matr,V.vect);
	if (ansr.vect.n_rows > 0) ansr.num = ansr.vect.n_rows;
	return ansr;
}

CMatrix_arma inv(CMatrix_arma &M)
{

	CMatrix_arma A;
	bool X = inv(A.matr, M.matr);
	if (X) A.setnumcolrows();
	return A;
}

double det(CMatrix_arma &M)
{
	return det(M.matr);
}

double rcond(CMatrix_arma &M)
{
    //return rcond(M.matr);
	return 0.7; 
}


CMatrix_arma& CMatrix_arma::operator=(mat &A)
{
	numcols = A.n_cols;
	numrows = A.n_rows;
	matr = A;
	return *this;
}

void write_to_file(vector<CMatrix_arma> M, string filename)
{
	ofstream Afile;
	Afile.open(filename);
	M.push_back(Average(M));
    for (unsigned int k = 0; k<M.size(); k++)
	{	for (int i = 0; i<M[k].numrows; ++i)
		{
			for (int j = 0; j<M[k].numcols; ++j)
			{
				Afile << M[k].get(i,j) << "\, ";
				cout<< M[k].get(i,j) << "\, ";
			}
			Afile << "\n";
		}
	Afile << "\n";
	}

}

CMatrix_arma Average(vector<CMatrix_arma> M)
{
	CMatrix_arma AVG(M[0].numrows, M[0].numcols);
	int n = M.size();
    for (unsigned int k = 0; k<M.size(); k++)
        for (unsigned int i = 0; i<M[k].numrows; ++i)
			for (int j = 0; j<M[k].numcols; ++j)
				AVG.get(i,j) += M[k].get(i,j)/n;
	return AVG;
}

CVector_arma CMatrix_arma::diag_ratio()
{
	CVector_arma X(numcols);
	CVector_arma maxs(numcols);
	for (int i=0; i<numcols; i++)
	{	for (int j=0; j<numrows; j++)
			if (i!=j) maxs[i] += fabs(matr(i,j));
		X[i]=maxs[i]/matr(i,i);
	}
	return X;
}

vector<vector<bool>> CMatrix_arma::non_posdef_elems(double tol)
{
	vector<vector<bool>> M;
	M.resize(getnumcols());

	for (int i = 0; i < getnumcols(); i++)
	{
		M[i].resize(getnumcols());
		for (int j = 0; j < getnumrows(); j++)
			if (matr(i,j) / matr(i,i) > tol) M[i][j] = 1;
	}
	return M;


}

CMatrix_arma CMatrix_arma::non_posdef_elems_m(double tol)
{
	CMatrix_arma M(getnumcols(), getnumrows());

	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if (matr(i,j) / matr(i,i) > tol) M.get(i,j) = matr(i,j);

	return M;


}

CMatrix_arma Identity_ar(int rows)
{
	CMatrix_arma M(rows, rows);
	for (int i = 0; i < rows; i++)
		M.get(i,i) = 1;

	return M;
}

CMatrix_arma CMatrix_arma::Preconditioner(double tol)
{
	CMatrix_arma M = non_posdef_elems_m(tol)+Identity_ar(numcols);
	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if ((M.get(i,j) != 0) & (i != j))
				M.get(i,j) = -M.get(i,j);

	return M;
}
vector<string> CMatrix_arma::toString(string format, vector<string> columnHeaders, vector<string> rowHeaders) const
{
	vector<string> r;
	bool rowH = false, colH = false;
	int rowOffset = 0, colOffset = 0;
	if (columnHeaders.size() && columnHeaders.size() == numcols)
	{
		colH = true;
		rowOffset = 1;
	}
	if (rowHeaders.size() && rowHeaders.size() == numrows)
	{
		rowH = true;
		colOffset = 1;
	}
	r.resize(numrows + rowOffset);


	if (colH)
	{
		if (rowH) r[0] += "\, ";
		for (int j = 0; j<numcols; j++)
		{
			r[0] += columnHeaders[j];
			if (j < numcols - 1) r[0] += "\, ";
		}

	}
	for (int i = 0; i<numrows; i++)
	{
		if (rowH)
		{
			r[i + rowOffset] += rowHeaders[i];
			r[i + rowOffset] += "\, ";
		}
		for (int j = 0; j<numcols; j++)
		{
            std::ostringstream streamObj;
            streamObj << matr(i,j);
            std::string strObj = streamObj.str();
            r[i + rowOffset] += strObj;
            if (j < numcols - 1) r[i + rowOffset] += "\, ";
		}
	}
	return r;
}


void CMatrix_arma::setnumcolrows()
{
	numcols = matr.n_cols;
	numrows = matr.n_rows;
}

void CMatrix_arma::setrow(int i, CVector_arma V)
{
	for (int j = 0; j < getnumcols(); j++)
		matr(i, j) = V[j];

}
void CMatrix_arma::setrow(int i, CVector V)
{
	for (int j = 0; j < getnumcols(); j++)
		matr(i, j) = V[j];
}
void CMatrix_arma::setcol(int i,  CVector_arma V)
{
	for (int j = 0; j < getnumrows(); j++)
		matr(j, i) = V[j];
}
void CMatrix_arma::setcol(int i,  CVector V)
{
	for (int j = 0; j < getnumrows(); j++)
		matr(j, i) = V[j];
}

CMatrix_arma normalize_diag( const CMatrix_arma &M1, const CMatrix_arma &M2)
{
	CMatrix_arma M(M1);
	CVector_arma D = diag(M2);
	for (int i = 0; i<M1.getnumcols(); i++)
	{
		for (int j=0; j<M1.getnumrows(); j++)
			M.matr(i,j) = M1.matr(i,j) / D[i];
	}
	return M;

}

CVector_arma normalize_diag( const CVector_arma &V, const CMatrix_arma &M2)
{
	CVector_arma M(V);

	for (int i = 0; i<V.getsize(); i++)
	{
        M[i] = V.vect[i] / M2.matr(i,i);
	}
	return M;
}

CVector_arma normalize_diag( const CVector_arma &V, const CVector_arma &D)
{
    CVector_arma M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V.vect[i] / D.vect[i];
    }
    return M;
}

CMatrix_arma normalize_max( const CMatrix_arma &M1, const CMatrix_arma &M2)
{
    CMatrix_arma M(M1);
    CVector_arma D = maxelements(M2);
    for (int i = 0; i<M1.getnumcols(); i++)
    {
        for (int j=0; j<M1.getnumrows(); j++)
            M.matr(i,j) = M1.matr(i,j) / D[i];
    }
    return M;

}

CVector_arma normalize_max( const CVector_arma &V, const CMatrix_arma &M2)
{
    CVector_arma M(V);
    CVector_arma D = maxelements(M2);
    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V.vect[i] / D[i];
    }
    return M;
}

CVector_arma normalize_max( const CVector_arma &V, const CVector_arma &D)
{
    CVector_arma M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V.vect[i] / D.vect[i];
    }
    return M;
}

void CMatrix_arma::ScaleDiagonal(double x)
{
	for (int i = 0; i < getnumcols(); i++)
	{
		matr(i, i) *= x;
	}
}

CVector CMatrix_arma::diagvector()
{
    CVector X(numcols);
    CVector maxs(numcols);
    for (int i=0; i<numcols; i++)
    {	for (int j=0; j<numrows; j++)
            X[i]=matr(i,i);
    }
    return X;
}

