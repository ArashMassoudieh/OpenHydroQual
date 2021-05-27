// Matrix.cpp: implementation of the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix.h"
#include "math.h"
#include <iostream>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"
#include "Vector.h"
#include "Expression.h"
#ifdef QT_version
#include "qstring.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_version



using namespace arma;


using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix::CMatrix(int m, int n)
{
	numrows = m;
	numcols = n;
	matr.resize(numrows);

	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}

}

CMatrix::CMatrix()
{
	numrows = 0;
	numcols = 0;
}

CMatrix::CMatrix(int m)
{
	numrows = m;
	numcols = m;
	matr.resize(numrows);

	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}
}

CMatrix::CMatrix(string filename)
{

    bool file_not_found;

	ifstream file(filename);
	vector<string> s;
	if (file.good() == false)
	{
		file_not_found = true;
		return;
	}
	int num_cols = 0;
	int num_rows = 0;
	int i=0;
	while (!file.eof())
	{
		s = aquiutils::getline(file,'\t');
		if (i==0) num_cols = s.size();
		if (s.size()==num_cols) i++;
    }
    num_rows=i;
    numrows = num_rows;
	numcols = num_cols;
	file.close();
	matr.resize(numrows);

	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}
	file.open(filename);

	for (int i=0; i<num_rows; i++)
	{
		s = aquiutils::getline(file,'\t');
        for (int j=0; j<num_cols; j++)
            matr[i][j] = atof(s[j].c_str());
    }

	return;

}


CMatrix::CMatrix(const CMatrix &m)
{
	numrows = m.numrows;
	numcols = m.numcols;
	matr.resize(numrows);

	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}
	for (int i=0; i<numrows; ++i)  matr[i]=m.matr[i];
}

CMatrix::CMatrix(const CVector &v)
{
	numrows = v.num;
	numcols = 1;
	matr.resize(numrows);
	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}

	for (int i=0; i<numrows; ++i)  matr[i].vec[0] = v.vec[i];
}


CVector& CMatrix::operator[](int i)
{
	return matr[i];
}

CMatrix::~CMatrix()
{
	matr.clear();
}

int CMatrix::getnumrows() const {return numrows;};
int CMatrix::getnumcols() const {return numcols;};

CMatrix& CMatrix::operator=(const CMatrix &m)
{

	numcols = m.numcols;
	numrows = m.numrows;
	matr.resize(numrows);
	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}

	for (int i = 0; i<numrows; ++i)
		matr[i] = m.matr[i];
	return *this;
}

CMatrix& CMatrix::operator+=(const CMatrix &m)
{

	for (int i=0; i<numrows; i++)
        matr[i] += m.matr[i];
	return *this;
}

CMatrix& CMatrix::operator-=(const CMatrix &m)
{
	for (int i=0; i<numrows; i++)
		matr[i] -= m.matr[i];
	return *this;
}

void CMatrix::Print(FILE *FIL)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(FIL, "%le ", matr[i].vec[j]);
		fprintf(FIL, "\n");
	}
	fclose(FIL);

}

CMatrix operator+(const CMatrix &m1, const CMatrix &m2)
{
	CMatrix mt = m1;
	mt += m2;
	return mt;
}

CMatrix operator-(const CMatrix &m1, const CMatrix &m2)
{
	CMatrix mt = m1;
	mt -= m2;
	return mt;
}

CMatrix mult(CMatrix &m1, CMatrix &m2)
{
	int nr = m1.numrows;
	int nc = m2.numcols;
	CMatrix mt(nr,nc);
	for (int i=0; i<nr; i++)
		for (int j=0; j<nc; j++)
			for (int k=0; k<m1.numcols; k++)
				mt[i][j] += m1[i][k]*m2[k][j];
	return mt;
}

CMatrix operator*(CMatrix m1, CMatrix m2)
{
	CMatrix a= mult(m1,m2);
	return a;
}


CVector mult(CMatrix &m1, CVector &v1)
{
	int nr = m1.numrows;
	CVector vt(nr);
	for (int i=0; i<nr; i++)
		for (int k=0; k<v1.num ; k++)
				vt[i] += m1[i][k]*v1[k];
	return vt;
}

CMatrix operator*(double d, CMatrix m1)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = d*m1[i][j];
	return TrM;

}

CMatrix operator+(double d, CMatrix m1)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = d+m1[i][j];
	return TrM;

}

CMatrix operator-(double d, CMatrix m1)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = d-m1[i][j];
	return TrM;

}

CMatrix operator+(CMatrix m1, double d)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = d+m1[i][j];
	return TrM;

}

CMatrix operator-(CMatrix m1,double d)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = m1[i][j]-d;
	return TrM;

}

CMatrix operator/(CMatrix m1,double d)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = m1[i][j]/d;
	return TrM;
}

CMatrix operator/(double d, CMatrix m1)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = d/m1[i][j];
	return TrM;
}


CVector operator*(CMatrix m, CVector v)
{
	return mult(m,v);
}

//CVector gauss(CMatrix, CVector)
//{
//}

void triangulate(CMatrix &m, CVector &v)
{
	int n=m.numrows;
	for (int i=0; i<n-1; i++)
	{	double diag = m[i][i];
		for (int j=i+1; j<n; j++)
		{	double p = m[j][i]/diag;
			m[j] -= p*m[i];
			v[j] -= p*v[i];
		}
	}
}

void backsubst(CMatrix& a , CVector& b, CVector& x)
{
	int n = a.numrows;
	for (int i = n-1; i>=0; i--)
	{	double diag = a[i][i];
		x[i] = (b[i] - dotproduct(x,a[i]))/diag;
	}
}

CVector gauss0(CMatrix M, CVector V)
{
	int n = M.numrows;
	CVector b(n);

	triangulate(M, V);
	backsubst(M, V, b);
	return b;
}

CVector operator/(CVector &V, CMatrix &M)
{
	return solve_ar(M,V);
}

CVector operator/(const CVector &V, const CMatrix &M)
{
    return solve_ar(M,V);
}

CMatrix Log(CMatrix &M1)
{
	CMatrix TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM[i][j] = log(M1[i][j]);
	return TrM;
}

CMatrix Exp(CMatrix &M1)
{
	CMatrix TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM[i][j] = exp(M1[i][j]);
	return TrM;
}

CMatrix Sqrt(CMatrix &M1)
{
	CMatrix TrM(M1.getnumrows(), M1.getnumcols());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM[i][j] = sqrt(M1[i][j]);
	return TrM;
}



CMatrix Invert(CMatrix M1)
{
	CMatrix InvM(M1.getnumcols(), M1.getnumcols());
	for (int i=0; i<M1.getnumcols(); i++)
	{
		CVector V(M1.getnumcols());
		V[i] = 1;
		InvM[i] = V/M1;
	}
	return Transpose(InvM);
}

CMatrix Cholesky_factor(CMatrix &M)
{
	int i;
	int j;
	int k;
	double s;
	CMatrix b(M.getnumcols(), M.getnumcols());
	int n = M.getnumcols();
	for ( j = 0; j < n; j++ )
	{	for ( i = 0; i < n; i++ )
		{
			b[i][j] = M[i][j];
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

CMatrix CMatrix::Cholesky_factor()
{
	CMatrix M = *this;
	int i;
	int j;
	int k;
	double s;
	CMatrix b(M.getnumcols(), M.getnumcols());
	int n = M.getnumcols();
	for ( j = 0; j < n; j++ )
	{	for ( i = 0; i < n; i++ )
		{
			b[i][j] = M[i][j];
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

CMatrix LU_decomposition(CMatrix &M)
{
	double AMAX,DUM, SUM;
	int  I,IMAX,J,K;
	int d=0;
	int n = M.getnumrows();
	CVector VV(n);
	CMatrix A = M;


	for  (I=0; I<n; I++)  {
    AMAX=0.0;
    for (J=0; J<n; J++)
		if (fabs(A[I][J]) > AMAX)  AMAX=fabs(A[I][J]);

		if (AMAX<1E-200) return A;
		VV[I] = 1/AMAX;
	}
	for (J=0; J<n;J++)  {
		for (I=0; I<J; I++)  {
			SUM = A[I][J];
			for (K=1; K<I; K++)
				SUM = SUM - A[I][K]*A[K][J];
			A[I][J] = SUM;
			} // i loop
		AMAX = 0.0;

		for (I=J; I<n; I++)  {
			SUM = A[I][J];
			for  (K=0; K<J; K++)
				SUM = SUM - A[I][K]*A[K][J];
			A[I][J] = SUM;
			DUM = VV[I]*fabs(SUM);
			if (DUM >= AMAX) {
				IMAX = I;
				AMAX = DUM;
			}
		} // i loop

		if (J != IMAX)  {
			for (K=0; K<n; K++)  {
				DUM = A[IMAX][K];
				A[IMAX][K] = A[J][K];
				A[J][K] = DUM;
			} // k loop
		d = -d;
		VV[IMAX] = VV[J];
    }

    if (fabs(A[J][J]) < 1E-200)   A[J][J] = 1E-200;

    if (J != n)  {
      DUM = 1.0 / A[J][J];
      for (I=J+1; I<n; I++)
        A[I][J] *= DUM;
    }
  } // j loop

  return A;

}


/*double det(CMatrix &A)
{
	CMatrix D = LU_decomposition(A);
	double prod = 1;
	for (int i=0; i<A.getnumcols(); i++)
		prod *= A[i][i];

	return prod;

}
*/

double CMatrix::det()
{
	CMatrix A = *this;
	CMatrix D = A.LU_decomposition();
	double prod = 1;
	for (int i=0; i<A.getnumcols(); i++)
		prod *= D[i][i];

	return prod;
}

CMatrix CMatrix::LU_decomposition()
{
    CMatrix A = *this;
	double AMAX,DUM, SUM;
	int  I,IMAX,J,K;

	int n = A.getnumrows();
	CVector VV(n);

	for  (I=0; I<n; I++)  {
    AMAX=0.0;
    for (J=0; J<n; J++)
		if (fabs(A[I][J]) > AMAX)  AMAX=fabs(A[I][J]);

		if (AMAX<1E-200) return A;
		VV[I] = 1/AMAX;
	}
	for (J=0; J<n;J++)  {
		for (I=0; I<J; I++)  {
			SUM = A[I][J];
			for (K=1; K<I; K++)
				SUM = SUM - A[I][K]*A[K][J];
			A[I][J] = SUM;
			} // i loop
		AMAX = 0.0;

		for (I=J; I<n; I++)  {
			SUM = A[I][J];
			for  (K=0; K<J; K++)
				SUM = SUM - A[I][K]*A[K][J];
			A[I][J] = SUM;
			DUM = VV[I]*fabs(SUM);
			if (DUM >= AMAX) {
				IMAX = I;
				AMAX = DUM;
			}
		} // i loop

		if (J != IMAX)  {
			for (K=0; K<n; K++)  {
				DUM = A[IMAX][K];
				A[IMAX][K] = A[J][K];
				A[J][K] = DUM;
			} // k loop
		//d = -d;
		VV[IMAX] = VV[J];
    }

    if (fabs(A[J][J]) < 1E-200)   A[J][J] = 1E-200;

    if (J != n)  {
      DUM = 1.0 / A[J][J];
      for (I=J+1; I<n; I++)
        A[I][J] *= DUM;
    }
  } // j loop

  return A;

}

CVector diag(const CMatrix &m)
{
	CVector v(m.getnumcols());
	for (int i=0; i<m.getnumcols(); ++i)
        v[i] = m.matr[i].at(i);
	return v;
}

CMatrix operator*(CVector v, CMatrix m)
{
    auto tempMap = CMatrix(v);
    CMatrix a = tempMap*m;
	return a;
}


CMatrix oneoneprod(CMatrix &m1, CMatrix &m2)
{
	CMatrix TrM(m1.getnumrows(), m1.getnumcols());
	for (int i=0; i<m1.getnumrows(); i++)
		for (int j=0; j<m1.getnumcols(); j++)
			TrM[i][j] = m1[i][j]*m2[i][j];
	return TrM;
}

void CMatrix::setval(double a)
{
	for (int i=0; i<numrows ; i++)
		for (int j=0; j<numcols ; j++)
			matr[i].vec[j] = a;


}

void CMatrix::setvaldiag(double a)
{
	for (int i=0; i<getnumrows(); i++)
		matr[i].vec[i] = a;

}

void CMatrix::writetofile(FILE *f)
{
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr[i].vec[j]);
		fprintf(f, "\n");
	}
}

void CMatrix::writetofile(string filename)
{
	FILE *f = fopen(filename.c_str(),"w");
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr[i].vec[j]);
		fprintf(f, "\n");
	}
	fclose(f);
}

//MM
//void CMatrix::writetofile(string filename)
void CMatrix::writetofile_app(string filename)
{
	FILE *f = fopen(filename.c_str(),"a");
	for (int i=0; i<numrows; i++)
	{	for (int j=0; j<numcols; j++)
			fprintf(f, "%le, ", matr[i].vec[j]);
		fprintf(f, "\n");
	}
	fclose(f);
}

CMatrix Transpose(CMatrix &M1)	//Works only when M1.getnumcols()=M1.getnumrows()
{
	CMatrix TrM(M1.getnumcols(), M1.getnumrows());
	for (int i=0; i<M1.getnumrows(); i++)
		for (int j=0; j<M1.getnumcols(); j++)
			TrM.matr[j].vec[i] = M1.matr[i].vec[j];
	return TrM;
}

void CMatrix::print(string s)
{

	ofstream Afile;
	Afile.open(s+".txt");
	cout<<s+"\="<<endl;

	for (int i = 0; i<numrows; ++i)
	{
		for (int j = 0; j<numcols; ++j)
		{
			Afile << matr[i][j] << "\, ";
			cout<< matr[i][j] << "\, ";
		}
		Afile << "\n";
		cout<< "\n";
	}
}

CVector solve_ar(CMatrix &M, CVector &V)
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
	try {
		C = solve(A,B);
		throw 0;
	}

	catch(int rtt)
	{

	}
	for (int i = 0;i<V.getsize(); ++i)
		ansr[i] = C(i,0);

	return ansr;
}

CVector solve_ar(const CMatrix &M, const CVector &V)
{

    mat A(M.getnumrows(),M.getnumcols());
    mat B(V.getsize(),1);

    CVector ansr = V;

    for (int i = 0;i<M.getnumrows(); ++i)
    {
        B(i,0) = V.at(i);
        for (int j = 0;j<M.getnumcols(); ++j)
            A(i,j) = M.matr[i].at(j);
    };

    mat C;
    try {
        C = solve(A,B);
        throw 0;
    }

    catch(int rtt)
    {

    }
    for (int i = 0;i<V.getsize(); ++i)
        ansr[i] = C(i,0);

    return ansr;
}

CMatrix inv(CMatrix M)
{

	mat A(M.getnumrows(),M.getnumcols());

	CMatrix inv_M;

	for (int i = 0;i<M.getnumrows(); ++i)
	{
		for (int j = 0;j<M.getnumcols(); ++j)
		{	A(i,j) = M[i][j];
			if ((A(i,j)==A(i,j))==false) return inv_M;
		}
	};

	mat inv_A;

	bool X = inv(inv_A, A);

	if (X==true)
	{	inv_M = CMatrix(M.getnumrows(),M.getnumrows());
		for (int i = 0;i<M.getnumrows(); ++i)
		{
			for (int j = 0;j<M.getnumcols(); ++j)
				inv_M[i][j] = inv_A(i,j);
		};
	}
	return inv_M;
}

double det(CMatrix &M)
{

	mat A(M.getnumrows(),M.getnumcols());

	for (int i = 0;i<M.getnumrows(); ++i)
	{
		for (int j = 0;j<M.getnumcols(); ++j)
			A(i,j) = M[i][j];
	}

	return det(A);
}


CMatrix& CMatrix::operator=(mat &A)
{
	numcols = A.n_cols;
	numrows = A.n_rows;
	matr.resize(numrows);
	for (int i = 0;i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}

	for (int i = 0; i<numrows; ++i)
		for (int j = 0; j<numcols; ++j)
			matr[i][j]=A(i,j);

	return *this;
}

void write_to_file(vector<CMatrix> M, string filename)
{
	ofstream Afile;
	Afile.open(filename);
	M.push_back(Average(M));
	for (unsigned int k = 0; k<M.size(); k++)
	{	for (int i = 0; i<M[k].numrows; ++i)
		{
			for (int j = 0; j<M[k].numcols; ++j)
			{
				Afile << M[k][i][j] << "\, ";
				cout<< M[k][i][j] << "\, ";
			}
			Afile << "\n";
		}
	Afile << "\n";
	}

}

CMatrix Average(vector<CMatrix> M)
{
	CMatrix AVG(M[0].numrows, M[0].numcols);
	int n = M.size();
	for (unsigned int k = 0; k<M.size(); k++)
		for (int i = 0; i<M[k].numrows; ++i)
			for (int j = 0; j<M[k].numcols; ++j)
				AVG[i][j] += M[k][i][j]/n;
	return AVG;
}

CVector CMatrix::diag_ratio()
{
	CVector X(numcols);
	CVector maxs(numcols);
	for (int i=0; i<numcols; i++)
	{	for (int j=0; j<numrows; j++)
			if (i!=j) maxs[i] += fabs(matr[i][j]);
		X[i]=maxs[i]/matr[i][i];
	}
	return X;
}

CMatrix normalize_diag(CMatrix &M1, CMatrix&M2)
{
	CMatrix M(M1);
	CVector D = diag(M2);
	for (int i=0; i<M1.getnumcols(); i++)
	{
		M.matr[i] = M1.matr[i]/D[i];
	}
	return M;

}

CVector normalize_diag( CVector V, CVector D)
{
    CVector M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V[i] / D[i];
    }
    return M;
}


CVector normalize_diag(CVector &V, CMatrix&M2)
{
	CVector M(V);
	CVector D = diag(M2);

	for (int i=0; i<V.getsize(); i++)
	{
		M[i] = V[i]/D[i];
	}
	return M;
}

CVector normalize_diag(const CVector &V, const CMatrix&M2)
{
    CVector M(V);
    CVector D = diag(M2);

    for (int i=0; i<V.getsize(); i++)
    {
        M[i] = V.at(i)/D.at(i);
    }
    return M;
}

CVector maxelements(const CMatrix &m)
{
    CVector_arma v(m.getnumcols());
    for (int i=0; i<m.getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<m.getnumrows(); ++j)
            maxval = max(fabs(m.matr[i].at(j)),maxval);
        v[i] = maxval;
    }
    return v;
}

CVector CMatrix::maxelements() const
{
    CVector_arma v(getnumcols());
    for (int i=0; i<getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<getnumrows(); ++j)
            maxval = max(fabs(matr[i].at(j)),maxval);
        v[i] = maxval;
    }
    return v;
}


CVector normalize_diag(const CVector &V, const CVector&D)
{
    CVector M(V);

    for (int i=0; i<V.getsize(); i++)
    {
        M[i] = V.at(i)/D.at(i);
    }
    return M;
}

CMatrix normalize_max( const CMatrix &M1, const CMatrix &M2)
{
    CMatrix M(M1);
    CVector D = maxelements(M2);
    for (int i = 0; i<M1.getnumcols(); i++)
    {
        for (int j=0; j<M1.getnumrows(); j++)
            M.matr[i][j] = M1.matr[i].at(j) / D[i];
    }
    return M;

}

CVector normalize_max( const CVector &V, const CMatrix &M2)
{
    CVector M(V);
    CVector D = maxelements(M2);
    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V.at(i) / D[i];
    }
    return M;
}

CVector normalize_max( const CVector &V, const CVector &D)
{
    CVector M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V.at(i) / D.at(i);
    }
    return M;
}

vector<vector<bool>> CMatrix::non_posdef_elems(double tol)
{
	vector<vector<bool>> M;
	M.resize(getnumcols());

	for (int i = 0; i < getnumcols(); i++)
	{
		M[i].resize(getnumcols());
		for (int j = 0; j < getnumrows(); j++)
			if (matr[i][j] / matr[i][i] > tol) M[i][j] = 1;
	}
	return M;


}

CMatrix CMatrix::non_posdef_elems_m(double tol)
{
	CMatrix M(getnumcols(), getnumrows());

	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if (matr[i][j] / matr[i][i] > tol) M[i][j] = matr[i][j];

	return M;


}

CMatrix Identity(int rows)
{
	CMatrix M(rows, rows);
	for (int i = 0; i < rows; i++)
		M[i][i] = 1;

	return M;
}

CMatrix CMatrix::Preconditioner(double tol)
{
	CMatrix M = non_posdef_elems_m(tol)+Identity(numcols);
	for (int i = 0; i < getnumcols(); i++)
		for (int j = 0; j < getnumrows(); j++)
			if ((M[i][j] != 0) & (i != j))
				M[i][j] = -M[i][j];

	return M;
}
vector<string> CMatrix::toString(string format, vector<string> columnHeaders, vector<string> rowHeaders) const
{
	vector<string> r;
	bool rowH = false, colH = false;
	int rowOffset = 0, colOffset=0;
	if (columnHeaders.size() && int(columnHeaders.size()) == numcols)
	{
		colH = true;
		rowOffset = 1;
	}
	if (rowHeaders.size() && int(rowHeaders.size()) == numrows)
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
            streamObj << matr[i].at(j);
            std::string strObj = streamObj.str();
            r[i + rowOffset] += strObj;
			if (j < numcols - 1) r[i + rowOffset] += "\, ";
		}
	}
	return r;
}

#ifdef QT_version
vector<string> CMatrix::toHtml(string format, vector<string> columnHeaders, vector<string> rowHeaders)
{
	vector<string> html, csv = toString(format, columnHeaders, rowHeaders);
	QString line;
	html.push_back("<!DOCTYPE html>");
	html.push_back("<html>");
	html.push_back("<body>");
	html.push_back("<table border = '1'>");

	for (int i = 0; i < csv.size(); i++)
	{
		line = QString::fromStdString(csv[i]);
		html.push_back("<tr><td>" + line.replace(",", "</td> <td>").toStdString() + "</td></tr>");
	}
	html.push_back("</table></body></html>");
	return html;
}
#endif // QT_version

void CMatrix::setnumcolrows()
{
	if (matr.size())
	{
		numcols = matr[0].getsize();
		numrows = matr.size();
	}
	else
	{
		numcols = 0;
		numrows = 0;
	}
}

#ifdef QT_version
QMap<QString, QVariant> CMatrix::compact() const
{
	QMap<QString, QVariant> r;
	r["nrows"] = numrows;
	r["ncols"] = numcols;
	for (int i = 0; i<numrows; ++i)
	{
		QStringList rowList;
		for (int j = 0; j < numcols; j++)
		{
			rowList.append(QString::number(matr[i].vec[j]));
		}
		QString code = QString("row %1").arg(i);
		r[code] = rowList;
	}
	return r;
}
CMatrix CMatrix::unCompact(QMap<QString, QVariant> r)
{
	CMatrix m;
	m.numrows = r["nrows"].toInt();
	m.numcols = r["ncols"].toInt();
	m.matr.resize(m.numrows);

	for (int i = 0; i < m.numrows; ++i)
	{
		m.matr[i].vec.resize(m.numcols);
		m.matr[i].num = m.numcols;
	}
	for (int i = 0; i < m.numrows; ++i)
	{
		QString code = QString("row %1").arg(i);
		QStringList rowList = r[code].toStringList();
		for (int j = 0; j < rowList.count(); j++)
		{
			m.matr[i].vec[j] = rowList[j].toDouble();
		}
	}
	return m;
}
#endif // QT_version

CMatrix::CMatrix(CMatrix_arma &M)
{
	numrows = M.getnumrows();
	numcols = M.getnumcols();
	matr.resize(numrows);

	for (int i = 0; i<numrows; ++i)
	{
		matr[i].vec.resize(numcols);
		matr[i].num = numcols;
	}
	for (int i = 0; i < numrows; i++)
		for (int j = 0; j < numcols; j++)
			matr[i][j] = M(i, j);

}

double& CMatrix::operator()(int i, int j)
{
	return matr[i].vec[j];
}

void CMatrix::ScaleDiagonal(double x)
{
	for (int i = 0; i < getnumcols(); i++)
	{
		matr[i].vec[i] *= x;
	}
}
