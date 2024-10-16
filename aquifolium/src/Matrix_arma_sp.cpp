// Matrix.cpp: implementation of the CMatrix_arma_sp class.
//
//////////////////////////////////////////////////////////////////////

#define ARMA_USE_SUPERLU 1

#include "Matrix_arma_sp.h"
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


using namespace arma;


using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix_arma_sp::CMatrix_arma_sp(int m, int n)
{
	numrows = m;
	numcols = n;
	matr = sp_mat(m, n);
//	matr.fill(fill::zeros);
	
}

CMatrix_arma_sp::CMatrix_arma_sp()
{
	numrows = 0;
	numcols = 0;
}

CMatrix_arma_sp::CMatrix_arma_sp(int m)
{
	numrows = m;
	numcols = m;
	matr = sp_mat(m, m);
	//matr.fill(fill::zeros);
}

CMatrix_arma_sp::CMatrix_arma_sp(const CMatrix_arma_sp &m)
{
	numrows = m.numrows;
	numcols = m.numcols;
	matr = m.matr;
	
}

CMatrix_arma_sp::CMatrix_arma_sp(const CVector_arma &v)
{
    numrows = v.num;
	numcols = 1;
	matr = sp_mat(numrows,1);
	
	for (int i=0; i<numrows; ++i)  matr(i,0) = v.vect(i);
}


CMatrix_arma_sp::~CMatrix_arma_sp()
{
	matr.clear();
}

double& CMatrix_arma_sp::get(int i, int j)
{
	double x = matr.at(i, j);
	return x;
}

double & CMatrix_arma_sp::operator()(int i, int j)
{
	return get(i, j);
}

vector<double*> CMatrix_arma_sp::get(int i)
{
	vector<double*> v;
	for (int j = 0; j < numrows; j++)
	//	v[j] = &matr(i, j);

	return v;
}

int CMatrix_arma_sp::getnumrows() const {return numrows;};
int CMatrix_arma_sp::getnumcols() const {return numcols;};	

CMatrix_arma_sp& CMatrix_arma_sp::operator=(const CMatrix_arma_sp &m)
{
	
	numcols = m.numcols;
	numrows = m.numrows;
	matr = m.matr;
	
	return *this;
}

CMatrix_arma_sp& CMatrix_arma_sp::operator+=(CMatrix_arma_sp &m)
{
	matr = matr + m.matr;
	return *this;
}

CMatrix_arma_sp& CMatrix_arma_sp::operator-=(CMatrix_arma_sp &m)
{
	for (int i=0; i<numrows; i++)
		matr[i] -= m.matr(i);
	return *this;
}

void CMatrix_arma_sp::Print(FILE *FIL)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(FIL, "%le ", matr(i,j));
		fprintf(FIL, "\n");
	}
	fclose(FIL);

}

CMatrix_arma_sp operator+(const CMatrix_arma_sp &m1, const CMatrix_arma_sp &m2)
{
	CMatrix_arma_sp mt = m1;
	mt.matr = m1.matr + m2.matr;
	return mt;
}

CMatrix_arma_sp operator-(const CMatrix_arma_sp &m1, const CMatrix_arma_sp &m2)
{
	CMatrix_arma_sp mt = m1;
	mt.matr = m1.matr - m2.matr;
	return mt;
}

CMatrix_arma_sp mult(CMatrix_arma_sp &m1, CMatrix_arma_sp &m2)
{
	CMatrix_arma_sp M;
	M.matr = m1.matr*m2.matr;
	M.numcols = m2.numcols;
	M.numrows = m1.numrows;
	return M;
}

CMatrix_arma_sp operator*(CMatrix_arma_sp &m1, CMatrix_arma_sp &m2)
{
	CMatrix_arma_sp a= mult(m1,m2);
	return a;
}


CVector_arma mult(CMatrix_arma_sp &m1, CVector_arma &v1)
{	
	int nr = m1.getnumrows();
	CVector_arma vt(nr);
    vt = m1.matr*v1.vect;
	return vt;
}

CMatrix_arma_sp operator*(double d, CMatrix_arma_sp m1)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	TrM.matr = d*m1.matr;
	return TrM;

}

CMatrix_arma_sp operator+(double d, CMatrix_arma_sp m1)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d+m1.get(i,j);
	return TrM;

}

CMatrix_arma_sp operator-(double d, CMatrix_arma_sp m1)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d-m1.get(i,j);
	return TrM;

}

CMatrix_arma_sp operator+(CMatrix_arma_sp m1, double d)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d+m1.get(i,j);
	return TrM;

}

CMatrix_arma_sp operator-(CMatrix_arma_sp m1,double d)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)-d;
	return TrM;

}

CMatrix_arma_sp operator/(CMatrix_arma_sp m1,double d)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)/d;
	return TrM;
}

CMatrix_arma_sp operator/(double d, CMatrix_arma_sp m1)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = d/m1.get(i,j);
	return TrM;
}


CVector_arma operator*(CMatrix_arma_sp &m, CVector_arma &v)
{
	return mult(m,v);
}

//CVector_arma_arma gauss(CMatrix_arma_sp, CVector_arma_arma)
//{
//}


CVector_arma operator/(CVector_arma &V, CMatrix_arma_sp &M)
{
	CVector_arma X(M.getnumcols()); 
    bool status = spsolve( X.vect, M.matr, V.vect);
    if (status == false) X = arma::vec();
	return X; 
}

CMatrix_arma_sp Log(CMatrix_arma_sp &M1)
{
	CMatrix_arma_sp TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = log(M1.get(i,j));
	return TrM;
}

CMatrix_arma_sp Exp(CMatrix_arma_sp &M1)
{
	CMatrix_arma_sp TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = exp(M1.get(i,j));
	return TrM;
}

CMatrix_arma_sp Sqrt(CMatrix_arma_sp &M1)
{
	CMatrix_arma_sp TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.get(i,j) = sqrt(M1.get(i,j));
	return TrM;
}



CMatrix_arma_sp Invert(CMatrix_arma_sp M1)
{
	CMatrix_arma_sp InvM(M1.getnumcols(), M1.getnumcols());
	//InvM.matr = inv(M1.matr);
	return InvM;
}


/*double det(CMatrix_arma_sp &A)
{
	CMatrix_arma_sp D = LU_decomposition(A);
	double prod = 1;
	for (int i=0; i<A.getnumcols(); i++)
		prod *= A(i,i);

	return prod;

}
*/

double CMatrix_arma_sp::det()
{
	//return arma::det(matr);
	return 0;
}


CVector_arma diag(CMatrix_arma_sp m)
{
	CVector_arma v(m.getnumcols());
	for (int i=0; i<m.getnumcols(); ++i)
		v[i] = m.get(i,i);
	return v;
}

CMatrix_arma_sp operator*(CVector_arma v, CMatrix_arma_sp m)
{
    CMatrix_arma_sp V = CMatrix_arma_sp(v);
    CMatrix_arma_sp a = V*m;
    return a;
}


CMatrix_arma_sp oneoneprod(CMatrix_arma_sp &m1, CMatrix_arma_sp &m2)
{
	CMatrix_arma_sp TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM.get(i,j) = m1.get(i,j)*m2.get(i,j);
	return TrM;
}

void CMatrix_arma_sp::setval(double a)
{
	for (int i=0; i<numrows ; i++)
		for (int j=0; j<numcols ; j++)
			matr(i,j) = a;


}

void CMatrix_arma_sp::setvaldiag(double a)
{
	for (int i=0; i<getnumrows(); i++)
		matr(i,i) = a;

}

void CMatrix_arma_sp::writetofile(FILE *f)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr(i,j));
		fprintf(f, "\n");
	}
}

void CMatrix_arma_sp::writetofile(string filename)
{
	ofstream f(filename);
	for (int i=0; i<numrows; i++)
	{
		for (int j = 0; j < numcols; j++)
		{
			f<<matr.at(i, j)<<",";
			//double a = matr(i, j);
			//double b = 2 * a; 
		}

		f << endl;
	}
	f.close();
}

//MM
//void CMatrix_arma_sp::writetofile(string filename)
void CMatrix_arma_sp::writetofile_app(string filename)
{
	FILE *f = fopen(filename.c_str(),"a");
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr(i,j));
		fprintf(f, "\n");
	}
	fclose(f);
}

CMatrix_arma_sp Transpose(CMatrix_arma_sp &M1)	//Works only when M1.getnumcols()=M1.getnumrows()
{
	CMatrix_arma_sp TrM(M1.getnumcols(), M1.getnumrows());
	TrM.matr = M1.matr.t();
	return TrM;
}

void CMatrix_arma_sp::print(string s)
{
	
	ofstream Afile;
	Afile.open(s+".txt");

	for (int i = 0; i<numrows; ++i)
	{
		for (int j = 0; j<numcols; ++j)
		{
			Afile << matr(i,j) << ", ";
		}
		Afile << "\n";
	}	
}

CVector_arma solve_ar(CMatrix_arma_sp &M, CVector_arma &V)
{

	CVector_arma ansr = V/M;
	return ansr;
}

CMatrix_arma_sp inv(CMatrix_arma_sp &M)
{
	
	CMatrix_arma_sp A;	
	//bool X = inv(A.matr, M.matr);
	//if (X) A.setnumcolrows();
	return A;
}

double det(CMatrix_arma_sp &M)
{
	//return det(M.matr);
	return 0;
}


CMatrix_arma_sp& CMatrix_arma_sp::operator=(mat &A)
{
	numcols = A.n_cols;
	numrows = A.n_rows;
	matr = A;
	return *this;	
}

void write_to_file(vector<CMatrix_arma_sp> M, string filename)
{
	ofstream Afile;
	Afile.open(filename);
	M.push_back(Average(M));
	for (int k = 0; k<M.size(); k++)
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

CMatrix_arma_sp Average(vector<CMatrix_arma_sp> M)
{
	CMatrix_arma_sp AVG(M[0].numrows, M[0].numcols);
	int n = M.size();
	for (int k = 0; k<M.size(); k++)
		for (int i = 0; i<M[k].numrows; ++i)
			for (int j = 0; j<M[k].numcols; ++j)
				AVG.get(i,j) += M[k].get(i,j)/n;
	return AVG;		
}

CVector_arma CMatrix_arma_sp::diag_ratio()
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

vector<vector<bool>> CMatrix_arma_sp::non_posdef_elems(double tol)
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

CMatrix_arma_sp CMatrix_arma_sp::non_posdef_elems_m(double tol)
{
	CMatrix_arma_sp M(getnumcols(), getnumrows());
	
	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if (matr(i,j) / matr(i,i) > tol) M.get(i,j) = matr(i,j);
	
	return M;


}

CMatrix_arma_sp Identity_ar_sp(int rows)
{
	CMatrix_arma_sp M(rows, rows);
	for (int i = 0; i < rows; i++)
		M.get(i,i) = 1;

	return M;
}

CMatrix_arma_sp CMatrix_arma_sp::Preconditioner(double tol)
{
	CMatrix_arma_sp M = non_posdef_elems_m(tol)+Identity_ar_sp(numcols);
	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if ((M.get(i,j) != 0) & (i != j))
				M.get(i,j) = -M.get(i,j);

	return M;
}
vector<string> CMatrix_arma_sp::toString(string format, vector<string> columnHeaders, vector<string> rowHeaders)
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
		if (rowH) r[0] += ", ";
		for (int j = 0; j<numcols; j++)
		{
			r[0] += columnHeaders[j];
			if (j < numcols - 1) r[0] += ", ";
		}

	}
	for (int i = 0; i<numrows; i++)
	{
		if (rowH)
		{
			r[i + rowOffset] += rowHeaders[i];
			r[i + rowOffset] += ", ";
		}
		for (int j = 0; j<numcols; j++)
		{
			r[i + rowOffset] += to_string(matr(i,j));
			if (j < numcols - 1) r[i + rowOffset] += ", ";
		}
	}
	return r;
}


void CMatrix_arma_sp::setnumcolrows()
{
	numcols = matr.n_cols;
	numrows = matr.n_rows;
}

void CMatrix_arma_sp::setrow(int i, CVector_arma &V)
{
	for (int j = 0; j < getnumcols(); j++)
		matr(i, j) = V[j];

}
void CMatrix_arma_sp::setrow(int i, CVector &V)
{
	for (int j = 0; j < getnumcols(); j++)
		matr(i, j) = V[j];
}
void CMatrix_arma_sp::setcol(int i, CVector_arma &V)
{
	for (int j = 0; j < getnumrows(); j++)
		matr(j, i) = V[j];
}
void CMatrix_arma_sp::setcol(int i, CVector &V)
{
	for (int j = 0; j < getnumrows(); j++)
		matr(j, i) = V[j];
}

CMatrix_arma_sp normalize_diag(CMatrix_arma_sp &M1, CMatrix_arma_sp &M2)
{
	CMatrix_arma_sp M(M1);
	CVector_arma D = diag(M2);
	for (int i = 0; i<M1.getnumcols(); i++)
	{
		for (int j=0; j<M1.getnumrows(); j++)
			M.matr(i,j) = M1.matr(i,j) / D[i];
	}
	return M;

}

CVector_arma normalize_diag(CVector_arma &V, CMatrix_arma_sp &M2)
{
	CVector_arma M(V);
	CVector_arma D = diag(M2);

	for (int i = 0; i<V.getsize(); i++)
	{
		M[i] = V[i] / D[i];
	}
	return M;
}

void CMatrix_arma_sp::ScaleDiagonal(double x)
{
    for (int i = 0; i < getnumcols(); i++)
    {
        matr(i, i) *= x;
    }
}

CVector CMatrix_arma_sp::diagvector()
{
    CVector X(getnumcols());
    CVector maxs(getnumcols());
    for (int i=0; i<getnumcols(); i++)
    {	for (int j=0; j<getnumrows(); j++)
            X[i]=matr(i,i);
    }
    return X;
}
