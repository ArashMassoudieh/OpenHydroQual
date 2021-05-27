// Distribution.cpp: implementation of the CDistribution class.
//
//////////////////////////////////////////////////////////////////////

#include "Distribution.h"
#include "DistributionNUnif.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDistribution::CDistribution()
{
		
}

CDistribution::CDistribution(int nn)
{
	n = nn;
	e.resize(n);
	s.resize(n);
}

CDistribution::~CDistribution()
{
	
}

CDistribution::CDistribution(const CDistribution &C)
{
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i=0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}


}

CDistribution CDistribution::operator = (const CDistribution &C)
{
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i=0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}

	return *this;

}

int CDistribution::GetRand()
{
	double x = GetRndUniF(0,1);
	int ii = 0;
	for (int i=0; i<n-1; i++)
	{	
		if (x<e[i] && x>s[i])
			ii = i;
	}
	return ii;

}
