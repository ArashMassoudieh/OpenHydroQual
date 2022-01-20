#pragma once
#include <vector>
#include "math.h"
#include <iostream>
#include "NormalDist.h"
#include "GA.h"
#include "Vector.h"
#include "BTCSet.h"

using namespace std;

/*struct Param
{
	int param_ID;
	int type; // 0: normal, 1: lognormal, 2: uniform
	double low, high;
	bool loged;
	double mean, std;
};*/

struct _MCMC_file_names
{
    string outputpath;
    string outputfilename;
};


struct _MCMC_settings
{
    int total_number_of_samples;
    int number_of_chains;
    int burnout_samples;
    double ini_purt_fact = 1;
    double purturbation_factor = 0.05;
    int number_of_parameters;
    //int nActParams;
    //int numBTCs;
    //int nsamples;
    //int n_ts;
    int save_interval=1;
    string continue_filename;
    //bool mixederror;
    bool noinipurt;
    bool sensbasedpurt;
    bool global_sensitivity;
    bool continue_mcmc;
    int number_of_post_estimate_realizations;
    double dp_sens;
    bool noise_realization_writeout;
    int numberOfThreads = 8;
    double acceptance_rate;
    double purt_change_scale = 0.75;

};

class RunTimeWindow;
class Parameter_Set;
class Parameter;

template<class T>
class CMCMC
{
public:

    T* Model;
    T Model_out;
	CMCMC(void);
	CMCMC(int nn, int nn_chains);
    CMCMC(T *system);
    void SetParameters(Object *obj);
    bool SetProperty(const string &varname, const string &value);
	~CMCMC(void);
    _MCMC_settings MCMC_Settings;
    //vector<Param> MCMCParam;
	vector<vector<double>> Params;
	vector<double> pertcoeff;
	vector<double> logp;
	vector<double> logp1;
	vector<double> u;
    //double posterior(vector<double> par, int ID = -1);
    void initialize(bool random=false);
    void initialize(vector<double> par);
    bool step(int k);
    bool step(int k, int nsamps, string filename, RunTimeWindow* _rtw = 0);
	vector<double> purturb(int k);
	CNormalDist ND;
    void writeoutput(string filename);
	vector<int> params;
    CTimeSeriesSet<double> MData;
    _MCMC_file_names FileInformation;
    int getparamno(int j);
    double posterior(vector<double> par, bool out=false);
    void getfromGA(const CGA<T> &GA);
	string outputfilename;
    RunTimeWindow *rtw;
    int getparamno(int i,int ts)const;
    int get_act_paramno(int i);
    int get_time_series(int i);
	vector<bool> apply_to_all;
    Parameter_Set *parameters = nullptr;
    Parameter* parameter(int i);
    CVector sensitivity(double d, vector<double> par);
    CVector sensitivity_ln(double d, vector<double> par);
	//runtimeWindow * rtw = 0;
    CMatrix sensitivity_mat_lumped(double d, vector<double> par);
    CTimeSeriesSet<double> prior_distribution(int n_bins);

    int readfromfile(string filename);
    CTimeSeriesSet<double> model(vector<double> par);
    vector<vector<CTimeSeriesSet<double>>> BTCout_obs;
    vector<vector<CTimeSeriesSet<double>>> BTCout_obs_noise;
    vector<vector<CTimeSeriesSet<double>>> BTCout_obs_prcntle;
    vector<vector<CTimeSeriesSet<double>>> BTCout_obs_prcntle_noise;
	vector<CMatrix> global_sens_lumped;
    CTimeSeriesSet<double> paramsList;
    CTimeSeriesSet<double> realized_paramsList;
    void getrealizations(CTimeSeriesSet<double> &MCMCout);
    void get_outputpercentiles(CTimeSeriesSet<double> &MCMCout);

	vector<double> calc_output_percentiles;
    void SetRunTimeWindow(RunTimeWindow *_rtw);
	double accepted_count=0, total_count=0;
    string last_error;
    void Perform();
};

#include "MCMC.hpp"
