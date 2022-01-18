// MCMC.cpp : Defines the entry point for the console application.
//

#include <vector>
#include "NormalDist.h"
#include <string>
#include <omp.h>
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

/*
template<class T>
vector<CTimeSeriesSet<double>> CMCMC<T>::model_lumped(vector<double> par)
{
	double sum = 0;
    vector<CTimeSeriesSet<double>> res;

	for (int ts = 0; ts < 1; ts++)
	{
#ifdef GIFMOD
		CMediumSet G1 = G;
#endif
#ifdef GWA
		CGWASet G1 = G;
		G1.Medium[0].project = false;
#endif
        for (int i = 0; i < MCMC_Settings.number_of_parameters; i++)
		{
			if (apply_to_all[i] == true)
			{
				if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i, ts)] - MCMCParam[i].mean, 2) / (2.0*pow(MCMCParam[i].std, 2)) / 1;
				if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i, ts)]) - log(MCMCParam[i].mean), 2) / (2.0*pow(MCMCParam[i].std, 2)) / 1;
				if (MCMCParam[i].type == 2)
					if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;
#ifdef GIFMOD
				G1.set_param(i, par[getparamno(i, 0)]);
#endif
#ifdef GWA
				G1.Medium[0].set_param(i, par[getparamno(i, 0)]);
#endif
			}
			else
			{
				if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i, ts)] - MCMCParam[i].mean, 2) / (2.0*pow(MCMCParam[i].std, 2));
				if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i, ts)]) - log(MCMCParam[i].mean), 2) / (2.0*pow(MCMCParam[i].std, 2));
				if (MCMCParam[i].type == 2)
					if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;
#ifdef GIFMOD
				G1.set_param(i, par[getparamno(i, ts)]);
			}
		}
		G1.finalize_set_param();
		sum += G1.calc_log_likelihood();

		res.push_back(G1.ANS_obs);
#endif
#ifdef GWA
		G1.Medium[0].set_param(i, par[getparamno(i, ts)]);
			}
		}
G1.Medium[0].finalize_set_param();
sum += G1.Medium[0].calc_log_likelihood();

res.push_back(G1.Medium[0].ANS_obs);
#endif
	}
	return res;
}
#ifdef GIFMOD
    vector<CTimeSeriesSet<double>> CMCMC::model_lumped(vector<double> par, CMedium &G) const
#endif
#ifdef GWA
    vector<CTimeSeriesSet<double>> CMCMC::model_lumped(vector<double> par, CGWA &G) const
#endif
{
	double sum = 0;
    vector<CTimeSeriesSet<double>> res;

	for (int ts = 0; ts<1; ts++)
	{
#ifdef GIFMOD
		CMedium G1 = G;
#endif
#ifdef GWA
		CGWA G1 = G;
		G1.project = false;
#endif
        for (int i = 0; i<MCMC_Settings.number_of_parameters; i++)
		{
			if (apply_to_all[i] == true)
			{
				if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i, ts)] - MCMCParam[i].mean, 2) / (2.0*pow(MCMCParam[i].std, 2)) / 1;
				if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i, ts)]) - log(MCMCParam[i].mean), 2) / (2.0*pow(MCMCParam[i].std, 2)) / 1;
				if (MCMCParam[i].type == 2)
					if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;
				G1.set_param(i, par[getparamno(i, 0)]);
			}
			else
			{
				if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i, ts)] - MCMCParam[i].mean, 2) / (2.0*pow(MCMCParam[i].std, 2));
				if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i, ts)]) - log(MCMCParam[i].mean), 2) / (2.0*pow(MCMCParam[i].std, 2));
				if (MCMCParam[i].type == 2)
					if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;
				G1.set_param(i, par[getparamno(i, ts)]);
			}
		}
		G1.finalize_set_param();
		sum += G1.calc_log_likelihood();

        res.push_back(G1.Results.ANS_obs);
	}
	return res;
}
*/


template<class T>
double CMCMC<T>::posterior(vector<double> par, bool out)
{

    T Model1 = *Model;

	double sum = 0;
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
    {
        Model1.SetSilent(true);
        Model1.SetRecordResults(false);
        Model1.SetNumThreads(1);
        for (int i = 0; i < MCMC_Settings.number_of_parameters; i++)
            Model1.SetParameterValue(i, par[i]);
        Model1.ApplyParameters();

        if (parameter(i)->GetPriorDistribution() == "uniform")
            if (par[i]<parameter(i)->GetRange().low || par[i]>par[i]<parameter(i)->GetRange().high) sum -= 3000;
        if (parameter(i)->GetPriorDistribution() == "normal") sum -= pow(par[i]-parameter(i)->mean(),2)/(2.0*pow(parameter(i)->std(),2));
        if (parameter(i)->GetPriorDistribution() == "lognormal") sum -= pow(log(par[i])-log(parameter(i)->mean()),2)/(2.0*pow(parameter(i)->std(),2));
	}

    Model1.Solve();
    sum+= Model1.GetObjectiveFunctionValue();

    if (out) Model_out = Model1;
	return sum;
}

template<class T>
void CMCMC<T>::initialize(bool random)
{
    Params.resize(MCMC_Settings.total_number_of_samples);
    for (unsigned int i=0; i<MCMC_Settings.total_number_of_samples; i++)
        Params[i].resize(MCMC_Settings.number_of_parameters);
    double pp=0;
    if (random)
    {   for (int j=0; j<MCMC_Settings.number_of_chains; j++)
        {
            for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
            {	if (parameter(i)->GetPriorDistribution()=="lognormal")
                    Params[j][i] = exp(log(parameter(i)->GetRange().low)+(log(parameter(i)->GetRange().high)-log(parameter(i)->GetRange().low))*ND.unitrandom());
                else
                    Params[j][i] = parameter(i)->GetRange().low+(parameter(i)->GetRange().high-parameter(i)->GetRange().low)*ND.unitrandom();
                if (parameter(i)->GetPriorDistribution()=="lognormal")
                    pp += log(Params[j][i]);
            }
            logp[j] = posterior(Params[j]);
            logp1[j] = logp[j]+pp;
        }
    }
    else
    {
        for (int j=0; j<MCMC_Settings.number_of_chains; j++)
            for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
                Params[j][i] = parameter(i)->GetValue();
    }

}



template<class T>
void CMCMC<T>::initialize(vector<double> par)
{

    if (MCMC_Settings.sensbasedpurt == true)
	{
		CVector X = sensitivity(1e-4, par);

        for (int j = 0; j<MCMC_Settings.number_of_parameters; j++)
		{
            if (parameter(j).Get_Distribution()=="normal" || parameter(j).Get_Distribution()=="uniform")
			{
                pertcoeff[j] = MCMC_Settings.purturbation_factor / fabs(X[getparamno(j, 0)]);
			}
            if (parameter(j).Get_Distribution()=="lognormal")
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
            if (parameter(j).Get_Distribution()=="lognormal")
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
            else Params[j][i] = par[i] * exp(alpha*ND.getnormalrand(0, pertcoeff[i]));
            if (parameter(i).Get_Distribution()=="uniform")
                while ((Params[j][i]<parameter(i).GetRange().low) || (Params[j][i]>parameter(i).GetRange().high))
                    Params[j][i] = par[i] + alpha*ND.getnormalrand(0, pertcoeff[i]);

		}
		logp[j] = posterior(Params[j]);
		logp1[j] = logp[j] + pp;
	}
}

template<class T>
bool CMCMC<T>::step(int k)
{

    vector<double> X = purturb(k-MCMC_Settings.number_of_chains);

    double logp_0 = posterior(X, k%MCMC_Settings.number_of_chains);
	double logp_1 = logp_0;
	bool res;

    if (ND.unitrandom() <exp(logp_0-logp[k-MCMC_Settings.number_of_chains]))
	{
		res=true;
		Params[k] = X;
		logp[k] = logp_0;
		logp1[k] = logp_1;
		accepted_count += 1;
	}
	else
	{
		res = false;
        Params[k] = Params[k-MCMC_Settings.number_of_chains];
        logp[k] = logp[k-MCMC_Settings.number_of_chains];
		logp1[k] = logp_1;

	}
	total_count += 1;
	return res;
}

template<class T>
vector<double> CMCMC<T>::purturb(int k)
{
	vector<double> X;
    X.resize(MCMC_Settings.number_of_parameters);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
        if (parameter(i).GetDistribution() == "lognormal")
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
        for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
            fprintf(file, "%s, ", parameter(i)->GetName());
		fprintf(file,"%s, %s, %s,", "logp", "logp_1", "stuck_counter");
        for (int j=0; j<pertcoeff.size(); j++) fprintf(file,"%s,", string("purt_coeff_" + QString("%1").arg(j).toStdString()).c_str());
		fprintf(file, "\n");
		fclose(file);
	}

    CVector stuckcounter(MCMC_Settings.number_of_chains);
	int acceptance_count=0;
    MCMC_Settings.ini_purt_fact = pertcoeff[0];
	int k_0 = k;
    for (int kk=k; kk<k+nsamps+MCMC_Settings.number_of_chains; kk+=MCMC_Settings.number_of_chains)
	{
        QCoreApplication::processEvents(QEventLoop::AllEvents,10*1000);
        if (rtw->stoptriggered)
			break;

        omp_set_num_threads(MCMC_Settings.numberOfThreads);



#ifdef WIN64
#pragma omp parallel
		{
			srand(int(time(NULL)) ^ omp_get_thread_num() + kk);
		}
#endif

#pragma omp parallel for
        for (int jj = kk; jj < min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples); jj++)
		{
            QCoreApplication::processEvents(QEventLoop::AllEvents,10*1000);

			qDebug() << "Starting step: " + QString::number(jj);
			bool stepstuck = !step(jj);
			qDebug() << "Step: " + QString::number(jj) + "Done!";
            if (stepstuck == true)
			{
				stuckcounter[jj - kk]++;
				acceptance_count++;
			}
			else
				stuckcounter[jj - kk] = 0;


#pragma omp critical
            {   if (rtw->detailson)
                {   QString s;
                    s = s+"Sample no: "+QString::number(jj) + " Parameters: ";
                    for (int i = 0; i < MCMC_Settings.number_of_parameters; i++)
                        s+=QString::number(Params[jj][i],'e',2)+ ",";

                    s+= " Stuck counter: " + QString::number(stuckcounter[jj-kk]);
                    s+= " Log Likelihood: " + QString::number(logp[jj],'e',2);

                    rtw->AppendtoDetails(s);
                    rtw->AppendtoDetails(" ");
                    QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);
                }
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);

        if ((kk-k_0) % (50 * MCMC_Settings.number_of_chains) == 0 || kk == k + nsamps + MCMC_Settings.number_of_chains - 1)
		{

            file = fopen(filename.c_str(), "a");
            for (int jj = max(min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples) - 50 * MCMC_Settings.number_of_chains, k_0); jj < min(kk + MCMC_Settings.number_of_chains, MCMC_Settings.total_number_of_samples); jj++)
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
		//double error = double(accepted_count) / double(total_count) - acceptance_rate;
        //for (int i = 0; i < MCMC_Settings.number_of_parameters; i++) pertcoeff[i] /= pow(MCMC_Settings.purt_change_scale, error);


		if (rtw)
		{
			QMap<QString, QVariant> vars;
			//jj, pertcoeff[0]
			//jj, double(accepted_count)/double(total_count);
			//update kk/nsamps
			vars["mode"] = "MCMC";
			double progress = double(kk) / double(nsamps)*100.0;
			vars["progress"] = progress;
            vars["perterbation factor"] = pertcoeff[0] / MCMC_Settings.ini_purt_fact;
			vars["acceptance rate"] = double(accepted_count) / double(total_count);
			vars["x"] = kk;
            rtw->SetProgress(progress);
            rtw->AddDataPoint(kk,double(accepted_count) / double(total_count));
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
    filenames.pathname = Model->OutputPath();

    for (unsigned int i=0; i<Model->Parameters().size(); i++)
    {
        MCMC_Settings.number_of_parameters++;
        params.push_back(i);
    }
    parameters = &Model->Parameters();
}


template<class T>
int CMCMC<T>::getparamno(int i,int ts) const
{
		return i;
}

template<class T>
int CMCMC<T>::get_act_paramno(int i)
{
	return i;
}

template<class T>
int CMCMC<T>::get_time_series(int i)
{
	return 0;
}

template<class T>
CVector CMCMC<T>::sensitivity(double d, vector<double> par)
{

	double base = posterior(par);
    CVector X(MCMC_Settings.number_of_parameters);
    for (int i=0; i<MCMC_Settings.number_of_parameters; i++)
	{
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
		double base_1 = posterior(par1);

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
        if (parameter(i).GetDistribution() != "lognormal")
		{
            min_range = parameter(i)->mean() - 4*parameter(i)->std();
            max_range = parameter(i)->mean() + 4*parameter(i)->std();;
		}
        if (parameter(i).GetDistribution() == "lognormal")
		{
            min_range = parameter(i)->mean() * exp(-4*parameter(i)->std());
            max_range = parameter(i)->mean() * exp(4*parameter(i)->std());
		}


		double dp = abs(max_range - min_range) / n_bins;

        B.SetT(0, min_range + dp/2);
		for (int j=0; j<n_bins-1; j++)
            B.SetT(j+1, B.GetT(j) + dp);

        if (parameter(i).GetDistribution() != "lognormal")
			for (int j=0; j<n_bins; j++)
                B.SetC(j , exp(-pow(B.GetT(j)-parameter(i)->mean(),2)/(2.0*pow(parameter(i)->std(),2)))/(parameter(i)->std()*pow(6.28,0.5)));

        if (parameter(i).GetDistribution() == "lognormal")
			for (int j=0; j<n_bins; j++)
                B.SetC(j, exp(-pow(log(B.GetT(j))-log(parameter(i)->mean()),2)/(2.0*pow(parameter(i)->std(),2)))/(B.GetT(j)*parameter(i)->std()*pow(6.28,0.5)));

        prior_dist.BTC[i] = B;
	}

    return prior_dist;
}

template<class T>
void CMCMC<T>::getrealizations(CTimeSeriesSet<double> &MCMCout)
{
	BTCout_obs.resize(1);
	BTCout_obs_noise.resize(1);

    int n_BTCout_obs = Model->ObservationsCount();

	for (int i = 0; i < 1; i++)
	{
		BTCout_obs[i].resize(n_BTCout_obs);
		BTCout_obs_noise[i].resize(n_BTCout_obs);
	}
	for (int i = 0; i < n_BTCout_obs; i++)
	{
        BTCout_obs[0][i] = CTimeSeriesSet<double>(MCMC_Settings.number_of_post_estimate_realizations);
        BTCout_obs_noise[0][i] = CTimeSeriesSet<double>(MCMC_Settings.number_of_post_estimate_realizations);
		BTCout_obs[0][i].names.clear();
		BTCout_obs_noise[0][i].names.clear();
	}

    realized_paramsList = CTimeSeriesSet<double>(MCMCout.nvars);
    paramsList = CTimeSeriesSet<double>(MCMCout.nvars);
	//qDebug() << "paramsList.names.size()" << paramsList.names.size();
	paramsList.names = MCMCout.names;
	//qDebug() << "MCMCout.names.size()" << MCMCout.names.size();

    for (int j = 0; j < MCMC_Settings.number_of_post_estimate_realizations; j++)
	{
        vector<double> param = MCMCout.getrandom(MCMC_Settings.burnout_samples);
		realized_paramsList.append(j, param);
	}
	//qDebug() << 612;
	//progress->setValue(0);
	double pValue = 0;
    double inc = 100.0 / MCMC_Settings.number_of_post_estimate_realizations;
    for (int jj = 0; jj <=MCMC_Settings.number_of_post_estimate_realizations/MCMC_Settings.numberOfThreads; jj++)
	{


        vector<vector<T>> Sys1(MCMC_Settings.numberOfThreads);
        for (int i = 0; i < MCMC_Settings.numberOfThreads; i++) Sys1[i].resize(1);

        omp_set_num_threads(MCMC_Settings.numberOfThreads);
#pragma omp parallel for
        for (int j = 0; j < min(MCMC_Settings.numberOfThreads, MCMC_Settings.number_of_post_estimate_realizations - jj*MCMC_Settings.numberOfThreads); j++)
		{
            int realizationNumber = jj*MCMC_Settings.numberOfThreads + j;
			cout << "Realization Sample No. : " << realizationNumber << std::endl;
			//qDebug() << "Realization Sample No. : " << realizationNumber;
			vector<double> param = realized_paramsList.getrow(realizationNumber);
            Sys1[j][0] = *Model;
            Sys1[j][0].ID = aquiutils::numbertostring(j);

			int l = 0;
            for (int i = 0; i < MCMC_Settings.number_of_parameters; i++)

			Sys1[j][0].set_param(i, param[i]);
			Sys1[j][0].finalize_set_param();
			Sys1[j][0].calc_log_likelihood();

            if (MCMC_Settings.global_sensitivity == true)
                global_sens_lumped.push_back(sensitivity_mat_lumped(MCMC_Settings.dp_sens, param));

			for (int i = 0; i < n_BTCout_obs; i++)
			{
				for (int ts = 0; ts < 1; ts++)
				{

					BTCout_obs[ts][i].BTC[realizationNumber] = Sys1[j][ts].ANS_obs.BTC[i];
					BTCout_obs[ts][i].names.push_back(Sys1[j][ts].ANS_obs.names[i] + "_" + to_string(realizationNumber));

                    if (MCMC_Settings.noise_realization_writeout)
					{
						BTCout_obs_noise[ts][i].BTC[realizationNumber] = Sys1[j][ts].ANS_obs_noise.BTC[i];
						BTCout_obs_noise[ts][i].names.push_back(Sys1[j][ts].ANS_obs_noise.names[i] + "_" + to_string(realizationNumber));
					}

				}
			}
			//qDebug() << "Realization Completed : " << realizationNumber << endl;
			cout << "Realization Completed : " << realizationNumber;
			//qDebug() << "Progress: " << pValue << inc;
			pValue += inc;
			//qDebug() << "Progress: " << pValue;
		}
		//qDebug() << "Progress: " << pValue << inc;
        pValue += inc*MCMC_Settings.numberOfThreads;
		//qDebug() << "Progress: " << pValue;
        if (rtw)
		{
			QMap<QString, QVariant> vars;
			vars["mode"] = "MCMC";
            vars["progress"] = pValue/MCMC_Settings.number_of_post_estimate_realizations;
            rtw->SetProgress(pValue/MCMC_Settings.number_of_post_estimate_realizations);
            rtw->AddDataPoint(jj,double(accepted_count) / double(total_count));
            rtw->Replot();
		}
	}
	if (rtw)
	{
		QMap<QString, QVariant> vars;
		vars["mode"] = "MCMC";
        vars["progress"] = 1;
		vars["finished"] = 1;
        rtw->SetProgress(1);
        rtw->Replot();
	}
}

template<class T>
void CMCMC<T>::get_outputpercentiles(CTimeSeriesSet<double> &MCMCout)
{

	getrealizations(MCMCout);
    int n_BTCout_obs = Model->ObservationsCount();

	BTCout_obs_prcntle.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle[j].resize(n_BTCout_obs);
	BTCout_obs_prcntle_noise.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle_noise[j].resize(n_BTCout_obs);

	if (calc_output_percentiles.size()>0)
		for (int i = 0; i < n_BTCout_obs; i++)
		{
			for (int j = 0; j < 1; j++)
			{
				BTCout_obs_prcntle[j][i] = BTCout_obs[j][i].getpercentiles(calc_output_percentiles);

                BTCout_obs_prcntle[j][i].writetofile(filenames.pathname + "BTC_obs_prcntl_" + Model->Observation(i)->GetName() + ".txt");

                if (MCMC_Settings.noise_realization_writeout)
					BTCout_obs_prcntle_noise[j][i] = BTCout_obs_noise[j][i].getpercentiles(calc_output_percentiles);

                BTCout_obs_prcntle_noise[j][i].writetofile(filenames.pathname + "BTC_obs_prcntl_noise_" + Model->Observation(i)->GetName() + ".txt");

			}
		}

}

template<class T>
void CMCMC<T>::Perform()
{
    initialize(false);
}
