// Binary.cpp: implementation of the CBinary class.
//
//////////////////////////////////////////////////////////////////////

#include "Binary.h"
#include "math.h"
#include <iostream>
#include "DistributionNUnif.h"
#ifdef QT_version
#include "qdebug.h"
#endif // QT_version

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBinary::CBinary()
{
	nDigits = 0;
	precision = 3;
	Digit.resize(nDigits);
	sign = true;

}

CBinary::CBinary(int n)
{
	nDigits = n;
	precision = 3;
	Digit.resize(nDigits);
	sign = true;

}

CBinary::CBinary(int n, int preci)
{
	nDigits = n;
	precision = preci;
	Digit.resize(nDigits);
	sign = true;

}

CBinary::~CBinary()
{

}

CBinary::CBinary(const CBinary &B)
{
	nDigits = B.nDigits;
	precision = B.precision;
	Digit.resize(nDigits);
	Digit = B.Digit;
	sign = B.sign;
}

CBinary CBinary::operator = (const CBinary &B)
{
	nDigits = B.nDigits;
	precision = B.precision;
	Digit.resize(nDigits);
	Digit = B.Digit;

	sign = B.sign;
	return *this;
}


double CBinary::decode(double minrange)
{

	double sum=0;
	for (int i=nDigits-1; i>=0; i--)
	{
		if (Digit[i] == true)
			sum += pow(2.0,nDigits-i-1);
	}

	return sum/pow(10.0,precision) + minrange;
}


CBinary code(double x, double minrange, double maxrange, int precision)
{
	int n = static_cast<int>(log((maxrange-minrange)*pow(10.0,precision))/log(2.0)+1);
	int xi = static_cast<int>((x-minrange)*pow(10.0,precision));
	CBinary B(n);
	B.precision = precision;

	for (int i=0; i<n; i++)
	{
		if (xi%2 == 1)
			B.Digit[B.nDigits - i-1] = true;
		else
			B.Digit[B.nDigits - i-1] = false;
		xi = static_cast<int>(xi/2);
	}

	return B;
}

CBinary CBinary::operator + (const CBinary &B1)
{
	int n = nDigits + B1.nDigits;
	CBinary B(n);
	for (int i=0; i<n; i++)
	{
		if (i<nDigits)
			B.Digit[i] = Digit[i];
		else
			B.Digit[i] = B1.Digit[i-nDigits];
	}

	return B;
}

CBinary CBinary::extract(int spoint, int epoint)
{
	int n = epoint - spoint + 1;
	CBinary B(n);
	for (int i=0; i<n; i++)
	{
			B.Digit[i] = Digit[i+spoint];
	}

	return B;

}

int& CBinary::operator[](unsigned int i)
{
    int *p = 0;
    if (i<Digit.size())
        return this->Digit[i];
    else
        return *p;
}

void cross(CBinary &B1, CBinary &B2, int p)
{
	CBinary BT1 = B1;
	CBinary BT2 = B2;
	for (int i=0; i<B1.nDigits; i++)
	{
		if (i < p)
		{
			B1[i] = BT1[i];
			B2[i] = BT2[i];
		}
		else
		{
			B1[i] = BT2[i];
			B2[i] = BT1[i];
		}
	}

}

void cross(CBinary &B1, CBinary &B2, vector<int> p)
{
	CBinary BT1 = B1;
	CBinary BT2 = B2;
	for (int i=0; i<B1.nDigits; i++)
	{
        for (unsigned int j=1; j<p.size(); j++)
        if (p[j-1]< i && i < p[j])
		{
			if (i%2==0)
			{
                ////qDebug()<<BT1[i];

                B1[i] = BT1[i];
				B2[i] = BT2[i];
			}
			else
			{
				B1[i] = BT2[i];
				B2[i] = BT1[i];
			}
		}
	}
}




void cross2p(CBinary &B1, CBinary &B2, int p1, int p2)
{
	CBinary BT1 = B1;
	CBinary BT2 = B2;
	for (int i=0; i<B1.nDigits; i++)
	{
		if (i < p1)
		{
			B1[i] = BT1[i];
			B2[i] = BT2[i];
		}
		if ((i >= p1) && (i < p2))
		{
			B1[i] = BT2[i];
			B2[i] = BT1[i];
		}
		if ( i >= p2)
		{
			B1[i] = BT1[i];
			B2[i] = BT2[i];
		}

	}

}

void CBinary::show()
{
	for (int i=0; i<nDigits; i++)
		cout<<Digit[i];
	cout<<endl;


}

void CBinary::mutate(double mu)
{
	for (int i=0; i<nDigits; i++)
		if (GetRndUniF(0,1)<mu)
			Digit[i] = !Digit[i];


}
