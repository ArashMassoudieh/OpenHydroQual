// Individual.cpp: implementation of the CIndividual class.
//
//////////////////////////////////////////////////////////////////////

#include "Individual.h"
#include "Binary.h"
#include "DistributionNUnif.h"
#include <cstdlib>
#include "QuickSort.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CIndividual::CIndividual()
{
	nParams = 1;
	x.resize(nParams);
	pert.resize(nParams);
	dir.resize(nParams);
	perteff.resize(nParams);
	precision.resize(nParams);
	minrange.resize(nParams);
	maxrange.resize(nParams);
    fit_measures.resize(3);

}

CIndividual::~CIndividual()
{
	
}

CIndividual::CIndividual(int n)
{
	nParams = n;
	x.resize(nParams);
	pert.resize(nParams);
	dir.resize(nParams);
	perteff.resize(nParams);
	precision.resize(nParams);
	minrange.resize(nParams);
	maxrange.resize(nParams);
	for (int i=0; i<nParams; i++)
	{
		x[i] = 0;
		pert[i] = 0;
		dir[i] = 0;
		minrange[i] = 0;
		maxrange[i] = 1;
		perteff[i] = 0;
	}
    fit_measures.resize(3);
}


CIndividual::CIndividual(const CIndividual &C)
{
	nParams = C.nParams;
	x.resize(nParams);
	pert.resize(nParams);
	dir.resize(nParams);
	perteff.resize(nParams);
	precision.resize(nParams);
	minrange.resize(nParams);
	maxrange.resize(nParams);
    parents = C.parents;
	for (int i=0; i<nParams; i++)
	{	
		pert[i] = C.pert[i];
		dir[i] = C.dir[i];
		x[i] = C.x[i];
		precision[i] = C.precision[i];
		minrange[i] = C.minrange[i];
		maxrange[i] = C.maxrange[i];
		perteff[i] = C.perteff[i];
	}
	
	fitness = C.fitness;
    fit_measures.resize(3);
	actual_fitness = C.actual_fitness;
	actual_fitness2 = C.actual_fitness2;

}

CIndividual CIndividual::operator = (const CIndividual &C)
{
	nParams = C.nParams;
	x.resize(nParams);
	pert.resize(nParams);
	dir.resize(nParams);
	perteff.resize(nParams);
	precision.resize(nParams);
	minrange.resize(nParams);
	maxrange.resize(nParams);
    parents = C.parents;
	for (int i=0; i<nParams; i++)
	{	
		pert[i] = C.pert[i];
		dir[i] = C.dir[i];
		x[i] = C.x[i];
		precision[i] = C.precision[i];
		minrange[i] = C.minrange[i];
		maxrange[i] = C.maxrange[i];
		perteff[i] = C.perteff[i];
	}
	
	fitness = C.fitness;
    fit_measures = C.fit_measures;
	actual_fitness = C.actual_fitness;
	actual_fitness2 = C.actual_fitness2;
	return *this;
}

void CIndividual::initialize()
{
	for (int i=0; i<nParams; i++)
	{	
		x[i] = GetRndUnif(0,1)*(maxrange[i] - minrange[i]) + minrange[i];
		//MM
		//x[i] = (i/4.0)*(maxrange[i] - minrange[i]) + minrange[i];
	}

}

double GetRndUnif(double xmin, double xmax)
{
	return double(rand())/double(RAND_MAX)*(xmax-xmin) + xmin;
}

void CIndividual::SetParents(int i)
{
    parents.clear();
    parents.push_back(i);
    parents.push_back(i);
}
void CIndividual::SetParents(int i, int j)
{
    parents.clear();
    parents.push_back(i);
    parents.push_back(j);
}

void cross(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2)
{
	IR1 = I1;
	IR2 = I2;
	
	CBinary B1 = code(I1.x[0], I1.minrange[0], I1.maxrange[0], I1.precision[0]);
	CBinary B2 = code(I2.x[0], I2.minrange[0], I2.maxrange[0], I2.precision[0]);

	vector<double> endpos(I1.nParams);
	
	endpos[0] = B1.nDigits;

	for (int i=1; i<I1.nParams; i++)
	{
		B1 = B1 + code(I1.x[i], I1.minrange[i], I1.maxrange[i], I1.precision[i]);
		B2 = B2 + code(I2.x[i], I2.minrange[i], I2.maxrange[i], I2.precision[i]);
		endpos[i] = B1.nDigits;
	}

	int nn = max(1,static_cast<int>(GetRndUnif(0,1)*(I1.nParams/4)));
	vector<int> rndint(nn);
	for (int i=0; i<rndint.size(); i++) rndint[i] = static_cast<int>(GetRndUnif(0,1)*B1.nDigits);
	rndint.push_back(0);
	rndint.push_back(B1.nDigits-1);
	rndint = QSort(rndint);

	cross(B1, B2, rndint);

	CBinary BT1 = B1.extract(0, endpos[0]-1);
	CBinary BT2 = B2.extract(0, endpos[0]-1);
	BT1.precision = I1.precision[0];
	BT2.precision = I2.precision[0];

	IR1.x[0] = BT1.decode(I1.minrange[0]);
	IR2.x[0] = BT2.decode(I2.minrange[0]);
	if (IR1.x[0]>IR1.maxrange[0]) 
		IR1.x[0]=IR1.maxrange[0];
	if (IR2.x[0]>IR2.maxrange[0]) 
		IR2.x[0]=IR2.maxrange[0];

	for (int i=1; i<I1.nParams; i++)
	{
		CBinary BT1 = B1.extract(endpos[i-1], endpos[i]-1);
		CBinary BT2 = B2.extract(endpos[i-1], endpos[i]-1);
		BT1.precision = I1.precision[i];
		BT2.precision = I2.precision[i];
		IR1.x[i] = BT1.decode(I1.minrange[i]);
		if (IR1.x[i]>IR1.maxrange[i]) 
			IR1.x[i]=IR1.maxrange[i];
		IR2.x[i] = BT2.decode(I2.minrange[i]);
		if (IR2.x[i]>IR2.maxrange[i]) 
			IR2.x[i]=IR2.maxrange[i];
		
	}

}

void cross_RC_L(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2)
{
	IR1 = I1;
	IR2 = I2;

	for (int i=0; i<IR1.nParams; i++)
	{	double rnd = GetRndUnif(0,1);
		IR1.x[i] = I1.x[i]*rnd + I2.x[i]*(1-rnd);
		IR2.x[i] = I2.x[i]*rnd + I1.x[i]*(1-rnd);
	}
	
	
}


void CIndividual::mutate(double mu)
{
	for (int i=0; i<nParams; i++)
	{
		CBinary B = code(x[i],minrange[i],maxrange[i], precision[i]);
		B.mutate(mu);
		x[i] = B.decode(minrange[i]);
		if (x[i]>maxrange[i]) x[i] = maxrange[i];
	}
}

void cross2p(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2)
{
	IR1 = I1;
	IR2 = I2;
	
	CBinary B1 = code(I1.x[0], I1.minrange[0], I1.maxrange[0], I1.precision[0]);
	CBinary B2 = code(I2.x[0], I2.minrange[0], I2.maxrange[0], I2.precision[0]);

	int endpos[10];
	
	endpos[0] = B1.nDigits;

	for (int i=1; i<I1.nParams; i++)
	{
		B1 = B1 + code(I1.x[i], I1.minrange[i], I1.maxrange[i], I1.precision[i]);
		B2 = B2 + code(I2.x[i], I2.minrange[i], I2.maxrange[i], I2.precision[i]);
		endpos[i] = B1.nDigits;
	}

	int rndint1 = static_cast<int>(GetRndUnif(0,1)*B1.nDigits);
	int rndint2 = static_cast<int>(GetRndUnif(0,1)*B1.nDigits);
	
	if (rndint1>rndint2)
	{	
		int tmp = rndint2;
		rndint2 = rndint1;
		rndint1 = tmp;
	}

	cross2p(B1, B2, rndint1, rndint2);

	CBinary BT1 = B1.extract(0, endpos[0]-1);
	CBinary BT2 = B2.extract(0, endpos[0]-1);
	BT1.precision = I1.precision[0];
	BT2.precision = I2.precision[0];

	IR1.x[0] = BT1.decode(I1.minrange[0]);
	IR2.x[0] = BT2.decode(I2.minrange[0]);
	if (IR1.x[0]>IR1.maxrange[0]) 
		IR1.x[0]=IR1.maxrange[0];
	if (IR2.x[0]>IR2.maxrange[0]) 
		IR2.x[0]=IR2.maxrange[0];

	
	

	for (int i=1; i<I1.nParams; i++)
	{
		CBinary BT1 = B1.extract(endpos[i-1], endpos[i]-1);
		CBinary BT2 = B2.extract(endpos[i-1], endpos[i]-1);
		BT1.precision = I1.precision[i];
		BT2.precision = I2.precision[i];
		IR1.x[i] = BT1.decode(I1.minrange[i]);
		if (IR1.x[i]>IR1.maxrange[i]) 
			IR1.x[i]=IR1.maxrange[i];
		IR2.x[i] = BT2.decode(I2.minrange[i]);
		if (IR2.x[i]>IR2.maxrange[i]) 
			IR2.x[i]=IR2.maxrange[i];
		
	}

}

void CIndividual::shake(double shakescale)
{
	for (int i=0; i<nParams; i++)
	{
		double dev = GetRndUnif(-shakescale,+shakescale);
		x[i] *= (1+dev);
		if (x[i]>maxrange[i]) x[i] = maxrange[i];
		if (x[i]<minrange[i]) x[i] = minrange[i];
	}
}
