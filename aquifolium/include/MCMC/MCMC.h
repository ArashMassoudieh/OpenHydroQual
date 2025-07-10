/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#pragma once
#include <vector>
#include "math.h"
#include <iostream>
#include "NormalDist.h"
#include "GA.h"
#include "Vector.h"
#include "TimeSeriesSet.h"
#include "observation.h"

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
    string detailfilename;
};


struct _MCMC_settings
{
    unsigned int total_number_of_samples;
    unsigned int number_of_chains;
    unsigned int burnout_samples;
    double ini_purt_fact = 1;
    double purturbation_factor = 0.05;
    unsigned int number_of_parameters;
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
    unsigned int number_of_post_estimate_realizations;
    double dp_sens;
    bool noise_realization_writeout;
    unsigned int numberOfThreads = 8;
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
    TimeSeriesSet<double> MData;
    _MCMC_file_names FileInformation;
    double posterior(vector<double> par, int sample_number, bool out=false);
    void model(T *Model1 , vector<double> par);
    RunTimeWindow *rtw;
    int getparamno(int i,int ts)const;
    int get_act_paramno(int i);
    int get_time_series(int i);
	vector<bool> apply_to_all;
    Parameter_Set *parameters = nullptr;
    vector<Observation> *observations = nullptr;
    Parameter* parameter(int i);
    Observation *observation(int i);
    CVector sensitivity(double d, vector<double> par);
    CVector sensitivity_ln(double d, vector<double> par);
	//runtimeWindow * rtw = 0;
    CMatrix sensitivity_mat_lumped(double d, vector<double> par);
    TimeSeriesSet<double> prior_distribution(int n_bins);

    int readfromfile(string filename);
    TimeSeriesSet<double> model(vector<double> par);
    vector<vector<TimeSeriesSet<double>>> BTCout_obs;
    vector<vector<TimeSeriesSet<double>>> BTCout_obs_noise;
    vector<vector<TimeSeriesSet<double>>> BTCout_obs_prcntle;
    vector<vector<TimeSeriesSet<double>>> BTCout_obs_prcntle_noise;
	vector<CMatrix> global_sens_lumped;
    TimeSeriesSet<double> paramsList;
    TimeSeriesSet<double> realized_paramsList;
    void ProduceRealizations(TimeSeriesSet<double> &MCMCout);
    void get_outputpercentiles(TimeSeriesSet<double> &MCMCout);

	vector<double> calc_output_percentiles;
    void SetRunTimeWindow(RunTimeWindow *_rtw);
	double accepted_count=0, total_count=0;
    string last_error;
    void Perform();
private:

};

#include "MCMC.hpp"
