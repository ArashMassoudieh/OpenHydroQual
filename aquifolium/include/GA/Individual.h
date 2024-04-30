
#pragma once
#include <vector>

using namespace std;
class CIndividual  
{
public:
	CIndividual();
	CIndividual(int n);
	CIndividual(const CIndividual &C);
    CIndividual operator = (const CIndividual &C);
	virtual ~CIndividual();
	vector<double> x;
	vector<double> pert;
	vector<int> dir;
	vector<double> perteff;
	double fitness;
	double actual_fitness, actual_fitness2;
	int parent1, parent2, xsite;
	int nParams;
	vector<int> precision;
	vector<double> minrange, maxrange;
    void initialize();
    void mutate(double mu);
	int rank;
    void shake(double shakescale);
    vector<double> fit_measures;
    vector<int> parents;
    void SetParents(int i);
    void SetParents(int i, int j);

};


double GetRndUnif(double xmin, double xmax);
void cross(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2);
void cross2p(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2);
void cross_RC_L(const CIndividual &I1, const CIndividual &I2, CIndividual &IR1, CIndividual &IR2);

