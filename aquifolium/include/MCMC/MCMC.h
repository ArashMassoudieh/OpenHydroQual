/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
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
    // --- covariance-adapted proposal (ON by default) ------------------------
    // A diagonal proposal perturbs each parameter independently by
    // pertcoeff[i].  Where the posterior has a correlated ridge -- for the
    // two-site sorption model, corr(ln alpha, ln Kd) = -0.81 within a medium,
    // because a faster grain holding less reproduces the same breakthrough
    // curve as a slower grain holding more -- it cannot step along that ridge.
    // The step must suit the narrowest direction while the chain needs
    // ~cond(Sigma) steps to cross the widest, and the chains drift apart
    // instead of mixing.
    // With covariance_proposal = true the sampler accumulates the empirical
    // covariance of the post-burn-in samples in log space and proposes from it
    // (Haario et al. 2001 adaptive Metropolis), which removes that factor.
    //
    // Default changed false -> true on 2026-09-19.  Measured on the 8-column
    // copper study, 16 chains, at equal cost (19,216 samples): worst split
    // R-hat 1.16 against 3.50, smallest ESS 53 against 9, 7 of 10 parameters
    // below R-hat 1.1 against 1 of 10, median ESS gain 6.0x.  The same 20,000
    // correlated samples also beat 60,000 diagonal ones on every diagnostic,
    // i.e. at a third of the cost.  The one parameter that does worse is the
    // error standard deviation (0.39x), which is nearly independent of the
    // rest; a proposal tuned to the joint covariance is less efficient in that
    // single direction.  Set it to No to recover the old behaviour.
    bool covariance_proposal = true;
    unsigned int covariance_update_interval = 1000;   // samples between refreshes
    unsigned int covariance_min_samples = 500;        // before the first refresh
    double covariance_scale = 2.38;                   // /sqrt(d), Roberts & Rosenthal
    double covariance_ridge = 1e-8;                   // added to the diagonal
    // covariance_centering = "chain": the proposal covariance is built from each sample's deviation from the
    // mean of its own chain (pooled within-chain covariance) instead of from the mean of all samples. While
    // the chains sit in different places (before they have mixed) the pooled covariance is dominated by the
    // gaps between chains: proposals along them are too long, are rejected, and the acceptance adaptation then
    // shrinks every step until the chains stall. Default "pooled" (the previous behaviour).
    bool covariance_within_chain = false;
    // steps (blocks of number_of_chains samples) between adjustments of the step size to the target acceptance
    // rate (x or / purturbation_change_scale); 50 was the fixed value before this setting
    unsigned int adaptation_interval = 50;
    // --- differential-evolution proposal (proposal_method = de) -------------
    // DE-MCz (ter Braak & Vrugt 2008) with DREAM-style crossover (Vrugt et al. 2009): a chain proposes
    // x* = x + gamma (z1 - z2) + e, with z1 != z2 drawn from the archive of past chain states (from
    // de_archive_start to the end of the previous round), in the same space as the other proposals (log for
    // log-normal priors). The differences between states that are already spread over the posterior give the
    // proposal its size and orientation, so it follows ridges without an estimated covariance; a covariance
    // proposal learns a long ridge only once each chain has explored it. gamma = de_gamma_scale * 2.38 /
    // sqrt(2 d'), d' = number of parameters updated; gamma = 1 every de_jump_interval rounds (jumps between
    // distant parts of the posterior). Each parameter is updated with probability de_crossover (at least one).
    // e ~ N(0, de_noise) (absolute in log space; relative to |x| for other priors) keeps the chain ergodic.
    // The proposal is symmetric, so the Metropolis acceptance in step() is unchanged.
    bool de_proposal = false;
    int de_archive_start = -1;          // -1: burnout_samples
    double de_gamma_scale = 1.0;
    unsigned int de_jump_interval = 10; // 0: never
    double de_crossover = 1.0;
    double de_noise = 1e-4;
    // de_adapt: the (non-jump) gamma is multiplied by an adaptive factor that is tuned, like the random-walk
    // step size, every adaptation_interval rounds toward acceptance_rate (x or / purturbation_change_scale).
    // On the Rock Creek ridge the theoretical gamma proposed moves ~2-14 log units downhill (none accepted
    // in 48), i.e. too long for a 16-d ridge that is not straight.
    bool de_adapt = true;
    double de_adapt_factor = 1.0;       // state; not a setting

};

// Forward-declared unconditionally: the `rtw` member and the `step()` default
// argument below are not inside Q_GUI_SUPPORT, so a headless build needs the
// name to exist even though no ProgressWindow is ever constructed. (GA.h guards
// its own member instead; either approach works, this one keeps the many
// `if (rtw)` sites in MCMC.hpp unguarded and readable.)
class ProgressWindow;
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
    // Cholesky factor of the adapted proposal covariance (log space), empty
    // until the first refresh; see _MCMC_settings::covariance_proposal.
    vector<vector<double>> proposal_chol;
    unsigned int last_covariance_update = 0;
    void UpdateProposalCovariance(int upto);
	vector<double> logp;
	vector<double> logp1;
    // filled_copy[k]: slot k was filled by readfromfile() with its chain's previous state because the
    // chain file has no row for it (e.g. a file holding only each chain's last state). Such slots are
    // not samples and are left out of the proposal covariance.
    vector<char> filled_copy;
    // pertcoeff[0] as initialize() set it: the reference of the acceptance-rate adaptation. A continued
    // run measures its adaptation against it, so its proposal resumes at the size the previous run reached.
    double initial_pertcoeff0 = 0;
	vector<double> u;
    //double posterior(vector<double> par, int ID = -1);
    void initialize(bool random=false);
    void initialize(vector<double> par);
    bool step(int k);
    bool step(int k, int nsamps, string filename, ProgressWindow* _rtw = 0);
	vector<double> purturb(int k);
	CNormalDist ND;
    void writeoutput(string filename);
	vector<int> params;
    TimeSeriesSet<double> MData;
    _MCMC_file_names FileInformation;
    double posterior(vector<double> par, int sample_number, bool out=false);
    void model(T *Model1 , vector<double> par);
    ProgressWindow *rtw = nullptr;   // headless runs leave this null; every use is `if (rtw)`-guarded
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
#ifdef Q_GUI_SUPPORT
    void SetProgressWindow(ProgressWindow *_rtw);
#endif
	double accepted_count=0, total_count=0;
    string last_error;
    void Perform();
private:

};

#include "MCMC.hpp"
