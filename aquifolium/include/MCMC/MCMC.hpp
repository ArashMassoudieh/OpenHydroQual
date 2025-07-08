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


// MCMC.cpp : Defines the entry point for the console application.
//

#include <vector>
#include "NormalDist.h"
#include <string>
#ifndef mac_version
#include <omp.h>
#endif
#include "MCMC.h"
#include "runtimewindow.h"
#include "Utilities.h"
#include "Parameter_Set.h"


using namespace std;

template<class T>
CMCMC<T>::CMCMC(void)
{
}

template<class T>
CMCMC<T>::~CMCMC(void)
{
	Params.clear();
    logp1.clear();
	logp.clear();
}

template<class T>
Parameter* CMCMC<T>::parameter(int i)
{
    return (*parameters)[i];
}

template <class T>
Observation* CMCMC<T>::observation(int i)
{
    return (&observations->at(i));
}

template<class T>
CTimeSeriesSet<double> CMCMC<T>::model(vector<double> par)
{
	double sum = 0;
    vector<CTimeSeriesSet<double>> res;


    T G1 = *Model;
    G1.SetSilent(true);
    G1.SetRecordResults(false);
    G1.SetNumThreads(1);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
        G1.SetParameterValue(i, par[i]);

    G1.ApplyParameters();
    G1.Solve();
    sum +=G1.GetObjectiveFunctionValue();

    return G1.Outputs.ObservedOutputs;

}

template<class T>
void CMCMC<T>::SetParameters(Object *obj)
{
    for (unordered_map<string,Quan>::iterator it=obj->GetVars()->begin(); it!=obj->GetVars()->end(); it++)
    {
        SetProperty(it->first,it->second.GetProperty());
    }
}

template<class T>
bool CMCMC<T>::SetProperty(const string &varname, const string &value)
{

    if (aquiutils::tolower(varname) == "number_of_samples") {MCMC_Settings.total_number_of_samples = aquiutils::atoi(value); return true;}
    if (aquiutils::tolower(varname) == "number_of_chains") {MCMC_Settings.number_of_chains = aquiutils::atoi(value); return true;}
    if (aquiutils::tolower(varname) == "number_of_burnout_samples") {MCMC_Settings.burnout_samples = aquiutils::atoi(value); return true;}
    if (aquiutils::tolower(varname) == "initial_purturbation_factor") {MCMC_Settings.purturbation_factor = aquiutils::atof(value); return true;}
    if (aquiutils::tolower(varname) == "record_interval") {MCMC_Settings.save_interval = aquiutils::atoi(value); return true;}
    if (aquiutils::tolower(varname) == "initial_purturbation")
    {
        if (aquiutils::tolower(value)=="yes")
            MCMC_Settings.noinipurt = false;
        else
            MCMC_Settings.noinipurt = true;
        return true;
    }
    if (aquiutils::tolower(varname) == "perform_global_sensitivity")
    {
        if (aquiutils::tolower(value)=="yes")
            MCMC_Settings.global_sensitivity = true;
        else
            MCMC_Settings.global_sensitivity = false;
        return true;
    }
    if (aquiutils::tolower(varname) == "continue_based_on_file_name")
    {
        if (value!="")
        {   MCMC_Settings.continue_filename = value;
            MCMC_Settings.continue_mcmc = true;
        }
        else
            MCMC_Settings.continue_mcmc = false;
        return true;
    }
    if (aquiutils::tolower(varname) == "samples_filename")
    {
        if (value!="")
        {
            if (value.find_first_of('/')!=string::npos || value.find_first_of('\\')!=string::npos)
                FileInformation.outputfilename = value;
            else
                FileInformation.outputfilename = FileInformation.outputpath + value;
        }
        FileInformation.detailfilename = aquiutils::extract_path(FileInformation.outputfilename) + "/" + "MCMC_details.txt";
        return true;
    }
    if (aquiutils::tolower(varname) == "number_of_post_estimate_realizations")
    {
        MCMC_Settings.number_of_post_estimate_realizations = aquiutils::atoi(value);
        return true;
    }
    if (aquiutils::tolower(varname) == "increment_for_sensitivity_analysis")
    {
        MCMC_Settings.dp_sens = aquiutils::atof(value);
        return true;
    }
    if (aquiutils::tolower(varname) == "add_noise_to_realizations")
    {
        if (aquiutils::tolower(value)=="yes")
            MCMC_Settings.noise_realization_writeout = true;
        else
            MCMC_Settings.noise_realization_writeout = false;
        return true;

    }
    if (aquiutils::tolower(varname) == "number_of_threads")
    {
        MCMC_Settings.numberOfThreads = aquiutils::atoi(value);
        return true;
    }
    if (aquiutils::tolower(varname) == "acceptance_rate")
    {
        MCMC_Settings.acceptance_rate = aquiutils::atof(value);
        return true;
    }
    if (aquiutils::tolower(varname) == "purturbation_change_scale")
    {
        MCMC_Settings.purt_change_scale = aquiutils::atof(value);
        return true;
    }

    last_error = "Property '" + varname + "' was not found!";
    return false;
}


template<class T>
double CMCMC<T>::posterior(vector<double> par, int sample_number, bool out)
{

    T Model1 = *Model;
    Model1.SetSilent(true);
    Model1.SetRecordResults(false);
    Model1.SetNumThreads(1);
	double sum = 0;
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
    {

        Model1.SetParameterValue(i, par[i]);
        sum+=parameter(i)->CalcLogPriorProbability(par[i]);
	}

    Model1.ApplyParameters();

#pragma omp critical
    {
        std::ofstream file_details(FileInformation.detailfilename, std::ios::app);
        file_details<<"Sample #" << sample_number << " started.\n";
        file_details.close();
    }
    Model1.Solve();
    double ObjectiveFunctionVal = Model1.GetObjectiveFunctionValue();

#pragma omp critical
    {
        std::ofstream file_details(FileInformation.detailfilename, std::ios::app);
        file_details<<"Sample #" << sample_number << " simulation_duration: " << double(Model1.GetSimulationDuration()) << ", simulation status: " << Model1.GetSolutionFailed() << ", objective function value: " << ObjectiveFunctionVal << "\n";
        file_details.close();
    }
    sum+= -ObjectiveFunctionVal;

    if (out) Model_out = Model1;
	return sum;
}

template<class T>
void CMCMC<T>::model(T *Model1, vector<double> par)
{

    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
    {
        Model1->SetSilent(true);
        Model1->SetRecordResults(false);
        Model1->SetNumThreads(1);
        for (unsigned int i = 0; i < MCMC_Settings.number_of_parameters; i++)
            Model1->SetParameterValue(i, par[i]);
        Model1->ApplyParameters();
    }

    Model1->Solve();

}

template<class T>
void CMCMC<T>::initialize(bool random)
{
    std::ofstream file(FileInformation.detailfilename); // Default mode truncates
    if (!file) {
        std::cerr << "Could not open file for clearing: " << FileInformation.detailfilename << std::endl;
    }
    file.close();
    Params.resize(MCMC_Settings.total_number_of_samples);
    logp.resize(MCMC_Settings.total_number_of_samples);
    logp1.resize(MCMC_Settings.total_number_of_samples);
    for (unsigned int i=0; i<MCMC_Settings.total_number_of_samples; i++)
        Params[i].resize(MCMC_Settings.number_of_parameters);
    
    for (int j = 0; j<MCMC_Settings.number_of_parameters; j++)
    {
        if (parameter(j)->GetPriorDistribution()=="normal" || parameter(j)->GetPriorDistribution()=="uniform")
        {
            pertcoeff[j] = MCMC_Settings.purturbation_factor*(-parameter(j)->GetRange().low + parameter(j)->GetRange().high);
        }
        if (parameter(j)->GetPriorDistribution()=="log-normal")
        {
            pertcoeff[j] = MCMC_Settings.purturbation_factor*(-log(parameter(j)->GetRange().low) + log(parameter(j)->GetRange().high));
        }
    }

    if (random)
    {
#pragma omp parallel for
        for (int j=0; j<MCMC_Settings.number_of_chains; j++)
        {
            double pp = 0;
            for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
            {	if (parameter(i)->GetPriorDistribution()=="log-normal")
                    Params[j][i] = exp(log(parameter(i)->GetRange().low)+(log(parameter(i)->GetRange().high)-log(parameter(i)->GetRange().low))*ND.unitrandom());
                else
                    Params[j][i] = parameter(i)->GetRange().low+(parameter(i)->GetRange().high-parameter(i)->GetRange().low)*ND.unitrandom();
                if (parameter(i)->GetPriorDistribution()=="log-normal")
                    pp += log(Params[j][i]);
            }
            logp[j] = posterior(Params[j], j)+pp;
            logp1[j] = logp[j];
        }
    }
    else
    {
#pragma omp parallel for
        for (int j=0; j<MCMC_Settings.number_of_chains; j++)
        {
            double pp = 0;
            for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
            {   Params[j][i] = parameter(i)->GetValue();
                if (parameter(i)->GetPriorDistribution()=="log-normal")
                    pp += log(Params[j][i]);
            }

            logp[j] = posterior(Params[j], j)+pp;
            logp1[j] = logp[j];
        }
    }

}



template<class T>
void CMCMC<T>::initialize(vector<double> par)
{

    std::ofstream file(FileInformation.detailfilename); // Default mode truncates
    if (!file) {
        std::cerr << "Could not open file for clearing: " << FileInformation.detailfilename << std::endl;
    }
    file.close();
    if (MCMC_Settings.sensbasedpurt == true)
	{
		CVector X = sensitivity(1e-4, par);

        for (int j = 0; j<MCMC_Settings.number_of_parameters; j++)
		{
            if (parameter(j).Get_Distribution()=="normal" || parameter(j).Get_Distribution()=="uniform")
			{
                pertcoeff[j] = MCMC_Settings.purturbation_factor / fabs(X[getparamno(j, 0)]);
			}
            if (parameter(j).Get_Distribution()=="log-normal")
			{
                pertcoeff[j] = MCMC_Settings.purturbation_factor / fabs(sqrt(par[j])*X[getparamno(j, 0)]);
			}

		}
	}
	else
        for (int j = 0; j<MCMC_Settings.number_of_parameters; j++)
		{
            if (parameter(j).Get_Distribution()=="normal" || parameter(j).Get_Distribution()=="uniform")
			{
                pertcoeff[j] = MCMC_Settings.purturbation_factor*(-parameter(j)->GetRange().low + -parameter(j)->GetRange().high);
			}
            if (parameter(j).Get_Distribution()=="log-normal")
			{
                pertcoeff[j] = MCMC_Settings.purturbation_factor*(-log(parameter(j)->GetRange().low) + log(parameter(j)->GetRange().high));
			}
		}
	double alpha;
    if (MCMC_Settings.noinipurt == true) alpha = 0; else alpha = 1;


    for (int j = 0; j<MCMC_Settings.number_of_chains; j++)
	{
        Params[j].resize(MCMC_Settings.number_of_parameters);
		double pp = 0;
        for (int i = 0; i<MCMC_Settings.number_of_parameters; i++)
		{
            if (parameter(i).Get_Distribution()=="normal" || parameter(i).Get_Distribution()=="uniform")
                Params[j][i] = par[i] + alpha*ND.getnormalrand(0, pertcoeff[i]);
            else
            {   Params[j][i] = par[i] * exp(alpha*ND.getnormalrand(0, pertcoeff[i]));
                pp+=log(par[i]);
            }
            if (parameter(i).Get_Distribution()=="uniform")
                while ((Params[j][i]<parameter(i).GetRange().low) || (Params[j][i]>parameter(i).GetRange().high))
                    Params[j][i] = par[i] + alpha*ND.getnormalrand(0, pertcoeff[i]);

		}
        logp[j] = posterior(Params[j], j);
		logp1[j] = logp[j] + pp;
	}
}

template<class T>
bool CMCMC<T>::step(int k)
{

    qDebug()<<"MCMC step " << k;
    vector<double> X = purturb(k-MCMC_Settings.number_of_chains);
    double pp =0;
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
    {
        if (parameter(i)->GetPriorDistribution()=="log-normal")
            pp += log(X[i]);
    }
    double logp_0 = posterior(X, k) + pp;


    double logp_1 = logp_0;
	bool res;

#pragma omp critical
    {
        std::ofstream file_details(FileInformation.detailfilename, std::ios::app);
        file_details<<"Sample #" << k << " proposal likelihood: " << logp_0 << ", previous likelihood: " << logp[k-MCMC_Settings.number_of_chains] << "\n";
        file_details.close();
    }


    if (ND.unitrandom() <exp(logp_0-logp[k-MCMC_Settings.number_of_chains]) && !isnan(logp_0))
	{
		res=true;
		Params[k] = X;
		logp[k] = logp_0;
		logp1[k] = logp_1;
#pragma omp critical
        {
            std::ofstream file_details(FileInformation.detailfilename, std::ios::app);
            file_details<<"Sample #" << k << " proposal likelihood: " << logp_0 << ", previous likelihood: " << logp[k-MCMC_Settings.number_of_chains] << ", accepted! \n";
            file_details.close();
        }
	}
	else
	{
		res = false;
        Params[k] = Params[k-MCMC_Settings.number_of_chains];
        logp[k] = logp[k-MCMC_Settings.number_of_chains];
		logp1[k] = logp_1;
#pragma omp critical
        {
            std::ofstream file_details(FileInformation.detailfilename, std::ios::app);
            file_details<<"Sample #" << k << " proposal likelihood: " << logp_0 << ", previous likelihood: " << logp[k-MCMC_Settings.number_of_chains] << ", rejected! \n";
            file_details.close();
        }

	}
    //total_count += 1;
	return res;
}

template<class T>
vector<double> CMCMC<T>::purturb(int k)
{
	vector<double> X;
    X.resize(MCMC_Settings.number_of_parameters);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
        if (parameter(i)->GetPriorDistribution() == "log-normal")
            X[i] = Params[k][i]*exp(pertcoeff[i]*ND.getstdnormalrand());
		else
            X[i] = Params[k][i]+pertcoeff[i]*ND.getstdnormalrand();

	}
	return X;
}

template<class T>
bool CMCMC<T>::step(int k, int nsamps, string filename, RunTimeWindow *rtw)
{
	FILE *file;
    if (MCMC_Settings.continue_mcmc == false)
	{	file = fopen(filename.c_str(),"w");
		fclose(file);
	}

    if (MCMC_Settings.continue_mcmc == false)
	{
		file = fopen(filename.c_str(),"a");
        fprintf(file,"%s, ", "no.");
        for (unsigned int i=0; i<MCMC_Settings.number_of_parameters; i++)
            fprintf(file, "%s, ", parameter(i)->GetName().c_str());
		fprintf(file,"%s, %s, %s,", "logp", "logp_1", "stuck_counter");
        for (unsigned int j=0; j<pertcoeff.size(); j++) fprintf(file,"%s,", string("purt_coeff_" + QString("%1").arg(j).toStdString()).c_str());
		fprintf(file, "\n");
		fclose(file);
	}

    CVector stuckcounter(MCMC_Settings.number_of_chains);
    CVector accepted(MCMC_Settings.number_of_chains);

    MCMC_Settings.ini_purt_fact = pertcoeff[0];
	int k_0 = k;
    for (unsigned int kk=k; kk<k+nsamps+MCMC_Settings.number_of_chains; kk+=MCMC_Settings.number_of_chains)
	{
        QCoreApplication::processEvents(QEventLoop::AllEvents,10*1000);
        if (rtw->stoptriggered)
			break;

#ifndef NO_OPENMP
        omp_set_num_threads(MCMC_Settings.numberOfThreads);
#endif


#ifdef WIN64
#pragma omp parallel
		{
			srand(int(time(NULL)) ^ omp_get_thread_num() + kk);
		}
#endif

#pragma omp parallel for
        for (int jj = kk; jj < min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples); jj++)
		{

            qDebug() << "Starting step: " + QString::number(jj);
            bool stepstuck = !step(jj);
            //qDebug() << "Step: " + QString::number(jj) + "Done!";
            if (stepstuck)
            {   stuckcounter[jj - kk]++;
                accepted[jj-kk]=0;
            }
            else
            {	stuckcounter[jj - kk] = 0;
                accepted[jj-kk]=1;
            }


#pragma omp critical
            {   if (rtw->detailson)
                {   QString s;
                    s = s+"Sample no: "+QString::number(jj) + " Parameters: ";
                    for (unsigned int i = 0; i < MCMC_Settings.number_of_parameters; i++)
                        s+=QString::number(Params[jj][i],'e',2)+ ",";

                    s+= " Stuck counter: " + QString::number(stuckcounter[jj-kk]);
                    s+= " Log Likelihood: " + QString::number(logp[jj],'e',2);

                    rtw->AppendtoDetails(s);
                    rtw->AppendtoDetails(" ");
                }
            }
        }
        accepted_count += accepted.sum();
        total_count += accepted.size();
        QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);

        if ((kk-k_0) % (50 * MCMC_Settings.number_of_chains) == 0 || kk == k + nsamps + MCMC_Settings.number_of_chains - 1)
		{

            file = fopen(filename.c_str(), "a");
            for (int jj = max(int(min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples) - 50 * MCMC_Settings.number_of_chains), k_0); jj < min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples); jj++)
			{
                if (jj%MCMC_Settings.save_interval == 0)
				{
                    //QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);

					fprintf(file, "%i, ", jj);
                    for (int i = 0; i < MCMC_Settings.number_of_parameters; i++)
						fprintf(file, "%le, ", Params[jj][i]);
                    fprintf(file, "%le, %le, %f,", logp[jj], logp1[jj], stuckcounter[jj%MCMC_Settings.number_of_chains]);
					for (int j = 0; j < pertcoeff.size(); j++) fprintf(file, "%le,", pertcoeff[j]);
					fprintf(file, "\n");

				}

                //cout << jj << "," << pertcoeff[0] << "," << stuckcounter.max() << "," << stuckcounter.min() << endl;
                ////qDebug() << jj << "," << pertcoeff[0] << "," << stuckcounter.max() << "," << stuckcounter.min();
				//if (jj<n_burnout)
			}
			fclose(file);
		}

        if ((kk-k_0) % (50*MCMC_Settings.number_of_chains) == 0)
		{

            if (double(accepted_count) / double(total_count)>MCMC_Settings.acceptance_rate)
                for (int i = 0; i < MCMC_Settings.number_of_parameters; i++) pertcoeff[i] /= MCMC_Settings.purt_change_scale;
			else
                for (int i = 0; i < MCMC_Settings.number_of_parameters; i++) pertcoeff[i] *= MCMC_Settings.purt_change_scale;
			accepted_count = 0;
			total_count = 0;

		}


		if (rtw)
		{
            double progress = double(kk) / double(nsamps);
            rtw->SetProgress(progress);
            rtw->AddDataPoint(kk,double(accepted_count) / double(total_count));
            if (rtw->plot2)
            {
                rtw->AddDataPoint(kk,double(pertcoeff[0] / MCMC_Settings.ini_purt_fact),1);
            }
            rtw->Replot();
		}
	}

	return 0;
}

template<class T>
void CMCMC<T>::SetRunTimeWindow(RunTimeWindow *_rtw)
{
    rtw = _rtw;
}

template<class T>
CMCMC<T>::CMCMC(T *_system)
{
    Model = _system;
    MCMC_Settings.number_of_parameters = 0;
    MCMC_Settings.numberOfThreads = 20;
    FileInformation.outputpath = Model->OutputPath();

    for (unsigned int i=0; i<Model->Parameters().size(); i++)
    {
        MCMC_Settings.number_of_parameters++;
        params.push_back(i);
    }
    parameters = &Model->Parameters();
    observations = Model->Observations();
    pertcoeff.resize(parameters->size());
}

template<class T>
CVector CMCMC<T>::sensitivity(double d, vector<double> par)
{

    double base = posterior(par, 0);
    CVector X(MCMC_Settings.number_of_parameters);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
        double base_1 = posterior(par1, 0);

		X[i] = (sqrt(fabs(base))-sqrt(fabs(base_1)))/(d*par[i]);
	}
 	return X;
}

/*CMatrix CMCMC::sensitivity_mat(double d, vector<double> par)
{

    vector<CTimeSeriesSet<double>> base = model(par);
	CMatrix X(n,base[0].nvars);
	for (int i=0; i<n; i++)
	{
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
        vector<CTimeSeriesSet<double>> base_1 = model(par1);

		for (int j=0; j<1;j++)
			X[i] = norm2dif(base[j],base_1[j])/d;
	}
 	return X;
}*/
/*
template<class T>
CMatrix CMCMC<T>::sensitivity_mat_lumped(double d, vector<double> par)
{

    vector<CTimeSeriesSet<double>> base = model_lumped(par);
#ifdef GIFMOD
	int ii = G.measured_quan.size();
#endif
#ifdef GWA
	int ii = G.Medium[0].measured_quan.size();
#endif

	CMatrix X(n,ii);
	for (int i=0; i<n; i++)
	{
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
        vector<CTimeSeriesSet<double>> base_1 = model_lumped(par1);

		for (int j=0; j<1;j++)
			X[i] += norm2dif(base[j],base_1[j])/d;
	}
 	return X;
}
*/

/*
template<class T>
CMatrix CMCMC<T>::sensitivity_mat_lumped(double d, vector<double> par, T &G) const

{

    vector<CTimeSeriesSet<double>> base = model_lumped(par, G);

	int ii = G.measured_quan().size();

	CMatrix X(n, ii);
	for (int i = 0; i<n; i++)
	{
		vector<double> par1 = par;
		par1[i] = par[i] * (1 + d);
        vector<CTimeSeriesSet<double>> base_1 = model_lumped(par1, G);

		for (int j = 0; j<1; j++)
			X[i] += norm2dif(base[j], base_1[j]) / d;
	}
	return X;
}
*/

template<class T>
CVector CMCMC<T>::sensitivity_ln(double d, vector<double> par)
{

	CVector X = sensitivity(d, par);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
		X[i] = par[i]*X[i];
	}
	return X;
}

template<class T>
int CMCMC<T>::readfromfile(string filename)
{
    ifstream file(filename);
	vector<string> s;
    s = aquiutils::getline(file);
	int jj=0;
	while (file.eof() == false)
	{
        s = aquiutils::getline(file);
        if (s.size() == 2*MCMC_Settings.number_of_parameters+4)
        {	Params[jj].resize(MCMC_Settings.number_of_parameters);
            for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
			{
				Params[jj][i] = atof(s[i+1].c_str());
                pertcoeff[i] = 	atof(s[MCMC_Settings.number_of_parameters+i+4].c_str());
			}
            logp[jj] = atof(s[MCMC_Settings.number_of_parameters+1].c_str());
            logp1[jj] = atof(s[MCMC_Settings.number_of_parameters+2].c_str());
			jj++;
		}
	}
	file.close();
	return jj;
}

template<class T>
CTimeSeriesSet<double> CMCMC<T>::prior_distribution(int n_bins)
{
    CTimeSeriesSet<double> prior_dist(MCMC_Settings.number_of_parameters);
    CTimeSeries<double> B(n_bins);

	double min_range , max_range;

    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
        if (parameter(i).GetDistribution() != "log-normal")
		{
            min_range = parameter(i)->mean() - 4*parameter(i)->std();
            max_range = parameter(i)->mean() + 4*parameter(i)->std();;
		}
        if (parameter(i).GetDistribution() == "log-normal")
		{
            min_range = parameter(i)->mean() * exp(-4*parameter(i)->std());
            max_range = parameter(i)->mean() * exp(4*parameter(i)->std());
		}


		double dp = abs(max_range - min_range) / n_bins;

        B.SetT(0, min_range + dp/2);
		for (int j=0; j<n_bins-1; j++)
            B.SetT(j+1, B.GetT(j) + dp);

        if (parameter(i).GetDistribution() != "log-normal")
			for (int j=0; j<n_bins; j++)
                B.SetC(j , exp(-pow(B.GetT(j)-parameter(i)->mean(),2)/(2.0*pow(parameter(i)->std(),2)))/(parameter(i)->std()*pow(6.28,0.5)));

        if (parameter(i).GetDistribution() == "log-normal")
			for (int j=0; j<n_bins; j++)
                B.SetC(j, exp(-pow(log(B.GetT(j))-log(parameter(i)->mean()),2)/(2.0*pow(parameter(i)->std(),2)))/(B.GetT(j)*parameter(i)->std()*pow(6.28,0.5)));

        prior_dist.BTC[i] = B;
	}

    return prior_dist;
}

template<class T>
void CMCMC<T>::ProduceRealizations(CTimeSeriesSet<double> &MCMCout)
{

    vector<CTimeSeriesSet<double>> realized_timeseries(observations->size());
    vector<CTimeSeriesSet<double>> predicted_percentiles(observations->size());

    for (unsigned int jj = 0; jj <=MCMC_Settings.number_of_post_estimate_realizations/MCMC_Settings.numberOfThreads; jj++)
	{
        vector<T> Sys1(MCMC_Settings.numberOfThreads);
#ifndef NO_OPENMP
        omp_set_num_threads(MCMC_Settings.numberOfThreads);
#endif
#pragma omp parallel for
        for (int j = 0; j < min(MCMC_Settings.numberOfThreads, MCMC_Settings.number_of_post_estimate_realizations - jj*MCMC_Settings.numberOfThreads); j++)
		{
            Sys1[j] = *Model;
            vector<double> sampled_parameters = MCMCout.getrandom(MCMC_Settings.burnout_samples);
            model(&Sys1[j],sampled_parameters);
        }

        for (unsigned int j = 0; j < min(MCMC_Settings.numberOfThreads, MCMC_Settings.number_of_post_estimate_realizations - jj*MCMC_Settings.numberOfThreads); j++)
            for (unsigned int i=0; i<observations->size(); i++)
                realized_timeseries[i].append(*(Sys1[j].observation(i)->GetModeledTimeSeries()));

        if (rtw)
        {
            double progress = double(jj*MCMC_Settings.numberOfThreads) / double(MCMC_Settings.number_of_post_estimate_realizations);
            rtw->SetProgress(progress);
        }
    }
    vector<double> percents; percents.push_back(0.025); percents.push_back(0.5); percents.push_back(0.975);
    for (unsigned int i=0; i<observations->size(); i++)
    {
        realized_timeseries[i].writetofile(FileInformation.outputpath + "Realizations_" + observation(i)->GetName() + ".txt");
        predicted_percentiles[i] = realized_timeseries[i].getpercentiles(percents);
        observation(i)->SetPercentile95(predicted_percentiles[i]);
        predicted_percentiles[i].writetofile(FileInformation.outputpath + "Predicted_95p_Bracket" + observation(i)->GetName() + ".txt");
        observation(i)->SetRealizations(realized_timeseries[i]);

    }
    rtw->SetProgress(1);
}

template<class T>
void CMCMC<T>::get_outputpercentiles(CTimeSeriesSet<double> &MCMCout)
{

    ProduceRealizations(MCMCout);
    int n_BTCout_obs = Model->ObservationsCount();

	BTCout_obs_prcntle.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle[j].resize(n_BTCout_obs);
	BTCout_obs_prcntle_noise.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle_noise[j].resize(n_BTCout_obs);

	if (calc_output_percentiles.size()>0)
		for (int i = 0; i < n_BTCout_obs; i++)
		{
			for (int j = 0; j < 1; j++)
			{
				BTCout_obs_prcntle[j][i] = BTCout_obs[j][i].getpercentiles(calc_output_percentiles);

                BTCout_obs_prcntle[j][i].writetofile(FileInformation.outputpath + "BTC_obs_prcntl_" + Model->Observation(i)->GetName() + ".txt");

                if (MCMC_Settings.noise_realization_writeout)
					BTCout_obs_prcntle_noise[j][i] = BTCout_obs_noise[j][i].getpercentiles(calc_output_percentiles);

                BTCout_obs_prcntle_noise[j][i].writetofile(FileInformation.outputpath + "BTC_obs_prcntl_noise_" + Model->Observation(i)->GetName() + ".txt");

			}
		}

}

template<class T>
void CMCMC<T>::Perform()
{
    initialize(false);
    int mcmcstart = MCMC_Settings.number_of_chains;
    if (MCMC_Settings.continue_mcmc)
    {
        if (rtw) rtw->AppendText("Reading samples from ... " + MCMC_Settings.continue_filename);
        mcmcstart = readfromfile(MCMC_Settings.continue_filename);
    }
    if (rtw) rtw->AppendText(string("Generating samples ... "));
    step(mcmcstart, int((MCMC_Settings.total_number_of_samples - mcmcstart) / MCMC_Settings.number_of_chains)*MCMC_Settings.number_of_chains, FileInformation.outputfilename , rtw);
    if (rtw) rtw->AppendText(string("Creating posterior distribution ..."));
    CTimeSeriesSet<double> all_posterior_distributions;
    CTimeSeriesSet<double> parameter_samples;
    vector<CVector> posterior_percentiles;
    vector<string> col_labels;
    vector<string> row_labels = {"0.025", "0.5", "0.975", "mean"};
    for (unsigned int i=0; i<parameters->size(); i++)
    {
        CTimeSeriesSet<double> chain_values(MCMC_Settings.number_of_chains);
        CTimeSeries<double> all_samples;
        for (unsigned int i=0; i<MCMC_Settings.number_of_chains; i++)
        {
            chain_values.setname(i,"Chain_" + aquiutils::numbertostring(i));
        }
        for (unsigned int j=MCMC_Settings.burnout_samples; j<MCMC_Settings.total_number_of_samples; j++)
            chain_values[j%MCMC_Settings.number_of_chains].append(j,Params[j][i]);

        for (unsigned int i=0; i<MCMC_Settings.number_of_chains; i++)
        {
            all_samples.append(chain_values[i]);
        }

        chain_values.name = parameter(i)->GetName();
        parameter(i)->SetMCMCSamples(chain_values);

        CTimeSeries<double> posterior_distribution = all_samples.distribution(all_samples.n/100,0);
        all_posterior_distributions.append(posterior_distribution,parameter(i)->GetName());
        posterior_distribution.name = "Posterior density";
        parameter(i)->SetPosteriorDistribution(posterior_distribution);
        parameter_samples.append(all_samples);
        CVector posterior_percentiles_for_this_param;
        col_labels.push_back(chain_values.name);
        posterior_percentiles_for_this_param.append(all_samples.percentile(0.025,MCMC_Settings.burnout_samples));
        posterior_percentiles_for_this_param.append(all_samples.percentile(0.5, MCMC_Settings.burnout_samples));
        posterior_percentiles_for_this_param.append(all_samples.percentile(0.975, MCMC_Settings.burnout_samples));
        posterior_percentiles_for_this_param.append(all_samples.mean());
        posterior_percentiles.push_back(posterior_percentiles_for_this_param);

    }
    writetofile(posterior_percentiles,col_labels, row_labels, FileInformation.outputpath + "posterior_percentiles.txt");

    all_posterior_distributions.writetofile(FileInformation.outputpath + "Posterior_distributions.txt");

    if (rtw) rtw->AppendText(string("Generating Realizations ..."));
    ProduceRealizations(parameter_samples);
}
