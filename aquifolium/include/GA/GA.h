#pragma once


#include "Individual.h"
#include "Distribution.h"
#include <stdio.h>
#include "BTC.h"
#include "DistributionNUnif.h"
#include "math.h"
#include <iostream>
#include "Matrix.h"
#include "omp.h"
#include <vector>
#include "Object.h"

//GUI
class RunTimeWindow;

struct GA_Twiking_parameters
{
    int totnumparams;
	int maxpop=2;
	int nParam;
	int numenhancements, num_enh;
	int nGen;
	//int writeinterval; //MCMC
	// int nchains, burnout; //MCMC
	// int nMCMCsamples; //MCMC
	// double acceptance_rate; //MCMC
	// int nrealizations; // MCMC
	// int continue_mcmc; // MCMC
	int cross_over_type;
	int no_bins;

	// bool sensbasedpurt; //MCMC
	// bool justreadmcmc; //MCMC
	// bool mcmc_realization; //MCMC
	// bool calculate_correlation; // MCMC
        bool sens_out = false;
	// bool global_sensitivity; //MCMC
	// bool calculate_percentile; // MCMC
	// bool noinipurt; //MCMC
        bool RCGA = false;
        bool readfromgafile;
	// bool calc_distributions; //MCMC
	// bool noise_realization_writeout; // MCMC
	// bool obs_realization_writeout; //MCMC
	// bool const_realization_writeout; //MCMC
	// bool influent_realization; // MCMC

	// double dp_sens; // MCMC
	// double purtscale; //MCMC
	// double purt_fac; //MCMC

	double N;
        double pcross=1;
        double pmute=0.02;

        double exponentcoeff=1;
        double shakescale=0.05;
        double shakescalered=0.75;
	// double pertscale, nonpurt; //MCMC
	// bool mcmc_run; // MCMC
         char fitnesstype;
};

struct _filenames
{
    string initialpopfilemame;
    string pathname;
    string getfromfilename;
    string outputfilename;
    string mcmcoutputfile;
};

using namespace std;

template<class T>
class CGA
{
public:

    double sumfitness;
	double MaxFitness;
    GA_Twiking_parameters GA_params;
    _filenames filenames;
    vector<double> calc_output_percentiles;
    vector<vector<double>> initial_pop;
    vector<double> final_params;
    vector<int> params;
    vector<int> loged;
    vector<int> to_ts;
    vector<double> fixedinputvale;
    vector<double> minval, maxval;
    vector<bool> apply_to_all;
    vector<vector<int>> outcompare;
    vector<CIndividual> Ind;
    vector<CIndividual> Ind_old;
    vector<string> paramname;
    vector<T> Models;
    T Model_out;
    T Model;

    CGA();
    virtual ~CGA();
    CGA(const CGA &C);
    CGA operator=(CGA &C);
    CGA(string filename, const T&);
    CGA(T*);
    void initialize();
    double getfromoutput(string filename);
    void getinifromoutput(string filename);
    void getinitialpop(string filename);
    int optimize();
    bool SetProperty(const string &varname, const string &value);
    string last_error;
    void SetParameters(Object *obj);
#ifdef Q_version
    void SetRunTimeWindow(RunTimeWindow *_rtw) {rtw=_rtw;}
#endif
private:
    void Setminmax(int a, double minrange, double maxrange, int prec);
    void fitnessdistini();
    void crossover();
    double avgfitness();
    void mutate(double mu);
    void assignfitnesses();
    int maxfitness();
    double variancefitness();
    double stdfitness();
    double avg_actual_fitness();
	void write_to_detailed_GA(string s);
    void setnumpop(int n);
    double avg_inv_actual_fitness();
    int optimize(int nGens, char DefOutPutFileName[]);
    void setnparams(int n);
    void assignfixedvalues();
    void assignrank();
    void assignfitness_rank(double N);
    void shake();
    void crossoverRC();
    void getfromfile(char filename[]);
    void fillfitdist();
    double assignfitnesses(vector<double> inp);
    int getparamno(int i, int ts);
    int get_act_paramno(int i);
    int get_time_series(int i);
    double evaluateforward();
    double evaluateforward_mixed(vector<double> v);

    CDistribution fitdist;

	// GUI
	int numberOfThreads;
        #ifdef Q_version
            RunTimeWindow *rtw=nullptr;
        #endif // QT_version


};

#include "GA.hpp"



