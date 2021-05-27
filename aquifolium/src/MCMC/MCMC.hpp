// MCMC.cpp : Defines the entry point for the console application.
//

#include <vector>
#include "NormalDist.h"
#include <string>
#include <omp.h>

using namespace std;

template<class T>
CMCMC<T>::CMCMC(void)
{
}

template<class T>
CMCMC<T>::~CMCMC(void)
{
	Params.clear();
	MCMCParam.clear();
	logp1.clear();
	logp.clear();
}

vector<CBTCSet*> CMCMC::model(vector<double> par)
{
	double sum = 0;
	vector<CBTCSet> res;


	CMediumSet G1 = G;
	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)
		{	if (MCMCParam[i].type == 1) sum -= pow(par[i]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
		if (MCMCParam[i].type == 2) sum -= pow(log(par[i])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
		G1.set_param(params[i],par[getparamno(i,0)]);
		}
		else
		{
			if (MCMCParam[i].type == 1) sum -= pow(par[i]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[i])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			G1.set_param(params[i],par[i]);
		}
	}
	G1.finalize_set_param();
	sum+=G1.calc_log_likelihood();

	return G1.ANS_hyd;

}



template<class T>
vector<CBTCSet> CMCMC<T>::model_lumped(vector<double> par)
{
	double sum = 0;
	vector<CBTCSet> res;

	for (int ts = 0; ts < 1; ts++)
	{
#ifdef GIFMOD
		CMediumSet G1 = G;
#endif
#ifdef GWA
		CGWASet G1 = G;
		G1.Medium[0].project = false;
#endif
		for (int i = 0; i < nActParams; i++)
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
	vector<CBTCSet> CMCMC::model_lumped(vector<double> par, CMedium &G) const
#endif
#ifdef GWA
	vector<CBTCSet> CMCMC::model_lumped(vector<double> par, CGWA &G) const
#endif
{
	double sum = 0;
	vector<CBTCSet> res;

	for (int ts = 0; ts<1; ts++)
	{
#ifdef GIFMOD
		CMedium G1 = G;
#endif
#ifdef GWA
		CGWA G1 = G;
		G1.project = false;
#endif
		for (int i = 0; i<nActParams; i++)
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


template<class T>
double CMCMC<T>::posterior(vector<double> par, int ID)
{
	double sum = 0;
	for (int ts=0; ts<1; ts++)
	{
#ifdef GIFMOD
		CMediumSet G1 = G;
		if (ID == -1)
			G1.ID = "final";
		else
			G1.ID = numbertostring(ID);
		//G1.FI.write_details = false;
#endif
#ifdef GWA
		CGWASet G1 = G;
		G1.Medium[0].project = false;
#endif
		for (int i=0; i<nActParams; i++)
		{
				if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i,ts)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
				if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i,ts)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
				if (MCMCParam[i].type == 2)
					if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;
#ifdef GIFMOD
				G1.set_param(i,par[getparamno(i,ts)]);
		}
		G1.finalize_set_param();
		sum += G1.calc_log_likelihood();

#endif
#ifdef GWA
		G1.Medium[0].set_param(i, par[getparamno(i, ts)]);
		}
	G1.Medium[0].finalize_set_param();
	sum += G1.Medium[0].calc_log_likelihood();
#endif

		}
	return sum;
}

template<class T>
double CMCMC<T>::posterior(vector<double> par, bool out)
{
#ifdef GIFMOD
	CMediumSet G1 = G;
#endif
#ifdef GWA
	CGWASet G1 = G;
	G1.Medium[0].project = false;
#endif
	double sum = 0;
	for (int i=0; i<nActParams; i++)
	{	for (int ts=0; ts<1; ts++)
			G1.Medium[0].set_param(i,par[getparamno(i,ts)]);
		G1.Medium[0].finalize_set_param();
	}

	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)
		{
			if (MCMCParam[i].type == 2)
				if (par[getparamno(i, 0)]<MCMCParam[i].low || par[getparamno(i, 0)]>MCMCParam[i].high) sum -= 3000;
			if (MCMCParam[i].type == 0) sum -= pow(par[params[getparamno(i,0)]]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 1) sum -= pow(log(par[params[getparamno(i,0)]])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
#ifdef GIFMOD
			for (int ts=0; ts<1; ts++) G.set_param(i,par[getparamno(i,0)]);
#endif
#ifdef GWA
			for (int ts = 0; ts<1; ts++) G.Medium[0].set_param(i, par[getparamno(i, 0)]);
#endif
		}
			else
				for (int ts=0; ts<1; ts++)
				{
					if (MCMCParam[i].type == 2)
						if (par[getparamno(i, ts)]<MCMCParam[i].low || par[getparamno(i, ts)]>MCMCParam[i].high) sum -= 3000;

					if (MCMCParam[i].type == 0) sum -= pow(par[getparamno(i,ts)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
					if (MCMCParam[i].type == 1) sum -= pow(log(par[getparamno(i,ts)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
#ifdef GIFMOD
					G.set_param(i,par[getparamno(i,ts)]);
				}
	}

	for (int ts = 0; ts<1; ts++)
	{
		G1 = G;
		sum += G1.calc_log_likelihood();
#endif
#ifdef GWA
					G.Medium[0].set_param(i, par[getparamno(i, ts)]);
	}
}

for (int ts = 0; ts<1; ts++)
{
	G1 = G;
	sum += G1.Medium[0].calc_log_likelihood();
#endif
	}


	if (out) G_out = G1;
	return sum;
}

template<class T>
void CMCMC<T>::initialize()
{
	double pp=0;
	for (int j=0; j<n_chains; j++)
	{	for (int i=0; i<n; i++)
		{	if (MCMCParam[i].loged==true)
				Params[j][i] = exp(log(MCMCParam[i].low)+(log(MCMCParam[i].high-MCMCParam[i].low))*ND.unitrandom());
			else
				Params[j][i] = MCMCParam[i].low+(MCMCParam[i].high-MCMCParam[i].low)*ND.unitrandom();
			if (MCMCParam[get_act_paramno(i)].loged==true)
				pp += log(Params[j][i]);
		}
		logp[j] = posterior(Params[j]);
		logp1[j] = logp[j]+pp;
	}
}



template<class T>
void CMCMC<T>::initialize(vector<double> par)
{


	for (int j = 0; j<nActParams; j++)
	{
		if ((MCMCParam[j].type == 0) || (MCMCParam[j].type == 2))
		{
			MCMCParam[j].mean = 0.5*(MCMCParam[j].low + MCMCParam[j].high);
			MCMCParam[j].std = (MCMCParam[j].high - MCMCParam[j].mean) / 2.0;

		}
		if (MCMCParam[j].type == 1)
		{
			MCMCParam[j].mean = sqrt(MCMCParam[j].low*MCMCParam[j].high);
			MCMCParam[j].std = (log(MCMCParam[j].high) - log(MCMCParam[j].mean)) / 2.0;
		}

	}

	if (sensbasedpurt == true)
	{
		CVector X = sensitivity(1e-4, par);

		for (int j = 0; j<nActParams; j++)
		{
			if ((MCMCParam[j].type == 0) || (MCMCParam[j].type == 2))
			{
				pertcoeff[j] = purt_fac / fabs(X[getparamno(j, 0)]);
			}
			if (MCMCParam[j].type == 1)
			{
				pertcoeff[j] = purt_fac / fabs(sqrt(par[j])*X[getparamno(j, 0)]);
			}

		}
	}
	else
		for (int j = 0; j<nActParams; j++)
		{
			if ((MCMCParam[j].type == 0) || (MCMCParam[j].type == 2))
			{
				pertcoeff[j] = purt_fac*(-MCMCParam[j].low + MCMCParam[j].high);
			}
			if (MCMCParam[j].type == 1)
			{
				pertcoeff[j] = purt_fac*(-log(MCMCParam[j].low) + log(MCMCParam[j].high));
			}
		}
	double alpha;
	if (noinipurt == true) alpha = 0; else alpha = 1;


	for (int j = 0; j<n_chains; j++)
	{
		Params[j].resize(n);
		double pp = 0;
		for (int i = 0; i<n; i++)
		{
			if (MCMCParam[i].loged == 0) Params[j][i] = par[i] + alpha*ND.getnormalrand(0, pertcoeff[i]); else Params[j][i] = par[i] * exp(alpha*ND.getnormalrand(0, pertcoeff[i]));
			if (MCMCParam[i].loged == 2) while ((Params[j][i]<MCMCParam[i].low) || (Params[j][i]>MCMCParam[i].high))
				while ((Params[j][i]<MCMCParam[i].low) || (Params[j][i]>MCMCParam[i].high))
					if (MCMCParam[i].loged == 0) Params[j][i] = par[i] + alpha*ND.getnormalrand(0, pertcoeff[i]); else Params[j][i] = par[i] * exp(alpha*ND.getnormalrand(0, pertcoeff[i]));

		}
		logp[j] = posterior(Params[j]);
		logp1[j] = logp[j] + pp;
	}
}

template<class T>
bool CMCMC<T>::step(int k)
{

	vector<double> X = purturb(k-n_chains);

	double logp_0 = posterior(X, k%n_chains);
	double logp_1 = logp_0;
	bool res;

	if (ND.unitrandom() <exp(logp_0-logp[k-n_chains]))
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
		Params[k] = Params[k-n_chains];
		logp[k] = logp[k-n_chains];
		logp1[k] = logp_1;
		accepted_count = accepted_count;

	}
	total_count += 1;
	return res;
}

template<class T>
vector<double> CMCMC<T>::purturb(int k)
{
	vector<double> X;
	X.resize(n);
	for (int i=0; i<n; i++)
	{
		if (MCMCParam[get_act_paramno(i)].loged == true)
			X[i] = Params[k][i]*exp(pertcoeff[get_act_paramno(i)]*ND.getstdnormalrand());
		else
			X[i] = Params[k][i]+pertcoeff[get_act_paramno(i)]*ND.getstdnormalrand();

	}
	return X;
}

template<class T>
bool CMCMC<T>::step(int k, int nsamps, string filename, runtimeWindow *rtw)
{
	FILE *file;
	if (continue_mcmc == false)
	{	file = fopen(filename.c_str(),"w");
		fclose(file);
	}

    if (continue_mcmc == false)
	{
		file = fopen(filename.c_str(),"a");
        fprintf(file,"%s, ", "no.");
			for (int i=0; i<n; i++)
				fprintf(file, "%s, ", paramname[i].c_str());
		fprintf(file,"%s, %s, %s,", "logp", "logp_1", "stuck_counter");
        for (int j=0; j<pertcoeff.size(); j++) fprintf(file,"%s,", string("purt_coeff_" + QString("%1").arg(j).toStdString()).c_str());
		fprintf(file, "\n");
		fclose(file);
	}

	CVector stuckcounter(n_chains);
	int acceptance_count=0;
	ini_purt_fact = pertcoeff[0];
	int k_0 = k;
	for (int kk=k; kk<k+nsamps+n_chains; kk+=n_chains)
	{
        QCoreApplication::processEvents(QEventLoop::AllEvents,10*1000);
		if (rtw->stopTriggered)
			break;

        omp_set_num_threads(numberOfThreads);



#ifdef WIN64
#pragma omp parallel
		{
			srand(int(time(NULL)) ^ omp_get_thread_num() + kk);
		}
#endif

#pragma omp parallel for
		for (int jj = kk; jj < min(kk + n_chains, nsamples); jj++)
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
            {   if (rtw->sln_dtl_active)
                {   QString s;
                    s = s+"Sample no: "+QString::number(jj) + " Parameters: ";
                    for (int i = 0; i < n; i++)
                        s+=QString::number(Params[jj][i],'e',2)+ ",";

                    s+= " Stuck counter: " + QString::number(stuckcounter[jj-kk]);
                    s+= " Log Likelihood: " + QString::number(logp[jj],'e',2);

                    rtw->slndetails_append(s);
                    rtw->slndetails_append(" ");
                    QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);
                }
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);

		if ((kk-k_0) % (50 * n_chains) == 0 || kk == k + nsamps + n_chains - 1)
		{

            file = fopen(filename.c_str(), "a");
			for (int jj = max(min(kk + n_chains, nsamples) - 50 * n_chains, k_0); jj < min(kk + n_chains, nsamples); jj++)
			{
				if (jj%writeinterval == 0)
				{
                    //QCoreApplication::processEvents(QEventLoop::AllEvents,100*1000);

					fprintf(file, "%i, ", jj);
					for (int i = 0; i < n; i++)
						fprintf(file, "%le, ", Params[jj][i]);
                    fprintf(file, "%le, %le, %f,", logp[jj], logp1[jj], stuckcounter[jj%n_chains]);
					for (int j = 0; j < pertcoeff.size(); j++) fprintf(file, "%le,", pertcoeff[j]);
					fprintf(file, "\n");

				}

                //cout << jj << "," << pertcoeff[0] << "," << stuckcounter.max() << "," << stuckcounter.min() << endl;
                ////qDebug() << jj << "," << pertcoeff[0] << "," << stuckcounter.max() << "," << stuckcounter.min();
				//if (jj<n_burnout)
			}
			fclose(file);
		}
		if ((kk-k_0) % (50*n_chains) == 0)
		{
			if (double(accepted_count) / double(total_count)>acceptance_rate)
				for (int i = 0; i < nActParams; i++) pertcoeff[i] /= purtscale;
			else
				for (int i = 0; i < nActParams; i++) pertcoeff[i] *= purtscale;
			accepted_count = 0;
			total_count = 0;

		}
		//double error = double(accepted_count) / double(total_count) - acceptance_rate;
		//for (int i = 0; i < nActParams; i++) pertcoeff[i] /= pow(purtscale, error);


		if (rtw)
		{
			QMap<QString, QVariant> vars;
			//jj, pertcoeff[0]
			//jj, double(accepted_count)/double(total_count);
			//update kk/nsamps
			vars["mode"] = "MCMC";
			double progress = double(kk) / double(nsamps)*100.0;
			vars["progress"] = progress;
			vars["perterbation factor"] = pertcoeff[0] / ini_purt_fact;
			vars["acceptance rate"] = double(accepted_count) / double(total_count);
			vars["x"] = kk;
            rtw->update(vars);


		}
	}

	return 0;
}

template<class T>
void CMCMC<T>::writeoutput(string filename)
{
	FILE *file;
	file = fopen(filename.c_str(),"a");
	for (int i=0; i<n; i+writeinterval)
		fprintf(file, "%s, ", paramname[i].c_str());
	fprintf(file,"%s\n", "logp");
	for (int j=0; j<n; j++)
	{
		for (int i=0; i<n; i+writeinterval)
			fprintf(file, "%le, ", Params[j][i]);

		fprintf(file,"%le\n", logp[j]);
	}
	fclose(file);
}

template<class T>
void CMCMC<T>::getfromGA(const CGA<T> &GA)
{
	logtrans = GA.logtrans;
	G = GA.Sys;
	MCMCParam.resize(GA.params.size());
	params = GA.params;
	outputfilename = GA.mcmcoutputfile;
	n = GA.nParam;
	nsamples = GA.nMCMCsamples;
	Params.resize(nsamples);
	pertcoeff.resize(nActParams);
	acceptance_rate = GA.acceptance_rate;
	purtscale = GA.purtscale;
	apply_to_all = GA.apply_to_all;
	writeinterval = GA.writeinterval;
	purt_fac = GA.purt_fac;
	sensbasedpurt = GA.sensbasedpurt;
	continue_mcmc = GA.continue_mcmc;
	for (int i=0; i<n; i++)
	{	if (GA.loged[i] != 1)
		{
			MCMCParam[i].type = 0;
			MCMCParam[i].mean = 0.5*(GA.Ind[0].minrange[i]+GA.Ind[0].maxrange[i]);
			MCMCParam[i].std = 0.25*(GA.Ind[0].maxrange[i]-GA.Ind[0].minrange[i]);
			MCMCParam[i].loged = false;
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.Ind[0].minrange[i];
			MCMCParam[i].high = GA.Ind[0].maxrange[i];
		}
		else if (GA.loged[i] == 1)
		{
			MCMCParam[i].type = 1;
			MCMCParam[i].mean = pow(GA.Ind[0].minrange[i]*GA.Ind[0].maxrange[i],0.5);
			MCMCParam[i].std = 0.25*(log(GA.Ind[0].maxrange[i])-log(GA.Ind[0].minrange[i]));
			MCMCParam[i].loged = true;
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = pow(10,GA.Ind[0].minrange[i]);
			MCMCParam[i].high = pow(10,GA.Ind[0].maxrange[i]);
		}
	}


}

template<class T>
CMCMC::CMCMC<T>(const CGA<T> &GA)
{
	logtrans = GA.logtrans;
	fixedstd = GA.fixedstd;
	mixederror = GA.mixederror;
	G = GA.Sys;
	MCMCParam.resize(GA.totnParam);
	params = GA.params;
	n_chains = GA.nchains;
	n_burnout = GA.burnout/GA.writeinterval;
	nActParams = GA.nParam;
	outputfilename = GA.mcmcoutputfile;
	n = GA.totnParam;
	nsamples = GA.nMCMCsamples;
	Params.resize(nsamples);
	logp.resize(nsamples);
	logp1.resize(nsamples);
	pertcoeff.resize(nActParams);
	acceptance_rate = GA.acceptance_rate;
	purtscale = GA.purtscale;
	apply_to_all = GA.apply_to_all;
	writeinterval = GA.writeinterval;
	purt_fac = GA.purt_fac;
	noinipurt = GA.noinipurt;
	sensbasedpurt = GA.sensbasedpurt;
	global_sensitivity = GA.global_sensitivity;
	continue_mcmc = GA.continue_mcmc;
	n_realizations = GA.nrealizations;
	noise_realization_writeout = GA.noise_realization_writeout;
	dp_sens = GA.dp_sens;
	calc_output_percentiles = GA.calc_output_percentiles;

	for (int i = 0; i < nActParams; i++)
	{
		if (GA.loged[i] == 0)
		{
			MCMCParam[i].type = 0;
			MCMCParam[i].mean = 0.5*(GA.minval[i] + GA.maxval[i]);
			MCMCParam[i].std = 0.25*(-GA.minval[i] + GA.maxval[i]);
			MCMCParam[i].loged = false;
			//MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.minval[i];
			MCMCParam[i].high = GA.maxval[i];
		}
		else if (GA.loged[i] == 1)
		{
			MCMCParam[i].type = 1;
			MCMCParam[i].mean = pow(GA.minval[i] * GA.maxval[i], 0.5);
			MCMCParam[i].std = 0.25*(-log(GA.minval[i]) + log(GA.maxval[i]));
			MCMCParam[i].loged = true;
			//MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = pow(10, GA.minval[i]);
			MCMCParam[i].high = pow(10, GA.maxval[i]);
		}
		else  if (GA.loged[i] == 2)
		{
			MCMCParam[i].type = 2;
			MCMCParam[i].mean = 0.5*(GA.minval[i] + GA.maxval[i]);
			MCMCParam[i].std = 0.25*(-GA.minval[i] + GA.maxval[i]);
			MCMCParam[i].loged = false;
			//MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.minval[i];
			MCMCParam[i].high = GA.maxval[i];
		}
	}
	paramname = GA.paramname;
	numberOfThreads = GA.numberOfThreads;
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
	CVector X(n);
	for (int i=0; i<n; i++)
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

	vector<CBTCSet> base = model(par);
	CMatrix X(n,base[0].nvars);
	for (int i=0; i<n; i++)
	{
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
		vector<CBTCSet> base_1 = model(par1);

		for (int j=0; j<1;j++)
			X[i] = norm2dif(base[j],base_1[j])/d;
	}
 	return X;
}*/

template<class T>
CMatrix CMCMC<T>::sensitivity_mat_lumped(double d, vector<double> par)
{

	vector<CBTCSet> base = model_lumped(par);
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
		vector<CBTCSet> base_1 = model_lumped(par1);

		for (int j=0; j<1;j++)
			X[i] += norm2dif(base[j],base_1[j])/d;
	}
 	return X;
}

#ifdef GIFMOD
CMatrix CMCMC::sensitivity_mat_lumped(double d, vector<double> par, CMedium &G) const
#endif
#ifdef GWA
CMatrix CMCMC::sensitivity_mat_lumped(double d, vector<double> par, CGWA &G) const
#endif
{

	vector<CBTCSet> base = model_lumped(par, G);
#ifdef GIFMOD
	int ii = G.measured_quan().size();
#endif
#ifdef GWA
	int ii = G.measured_quan.size();
#endif

	CMatrix X(n, ii);
	for (int i = 0; i<n; i++)
	{
		vector<double> par1 = par;
		par1[i] = par[i] * (1 + d);
		vector<CBTCSet> base_1 = model_lumped(par1, G);

		for (int j = 0; j<1; j++)
			X[i] += norm2dif(base[j], base_1[j]) / d;
	}
	return X;
}

template<class T>
CVector CMCMC<T>::sensitivity_ln(double d, vector<double> par)
{

	CVector X = sensitivity(d, par);
	for (int i=0; i<n; i++)
	{
		X[i] = par[i]*X[i];
	}
	return X;
}

template<class T>
int CMCMC<T>::readfromfile(string filename)
{
	for (int j=0; j<nActParams; j++)
	{
		if (MCMCParam[j].type != 1)
		{
			MCMCParam[j].mean = 0.5*(MCMCParam[j].low+MCMCParam[j].high);
			MCMCParam[j].std = (MCMCParam[j].high - MCMCParam[j].mean)/2.0;

		}
		if (MCMCParam[j].type == 1)
		{
			MCMCParam[j].mean = sqrt(MCMCParam[j].low*MCMCParam[j].high);
			MCMCParam[j].std = (log(MCMCParam[j].high) - log(MCMCParam[j].mean))/2.0;
		}

	}

	ifstream file(filename);
	vector<string> s;
	s = getline(file);
	int jj=0;
	while (file.eof() == false)
	{
		s = getline(file);
		if (s.size() == 2*n+4)
		{	Params[jj].resize(n);
			for (int i=0; i<n; i++)
			{
				Params[jj][i] = atof(s[i+1].c_str());
				pertcoeff[i] = 	atof(s[n+i+4].c_str());
			}
			logp[jj] = atof(s[n+1].c_str());
			logp1[jj] = atof(s[n+2].c_str());
			jj++;
		}
	}
	file.close();
	return jj;
}

template<class T>
CBTCSet CMCMC<T>::prior_distribution(int n_bins)
{
	CBTCSet A(nActParams);
	CBTC B(n_bins);

	double min_range , max_range;

	for (int i=0; i<nActParams; i++)
	{
		if (MCMCParam[i].type != 1)
		{
			MCMCParam[i].mean = 0.5*(MCMCParam[i].low+MCMCParam[i].high);
			MCMCParam[i].std = (MCMCParam[i].high - MCMCParam[i].mean)/2.0;
			min_range = MCMCParam[i].mean - 4*MCMCParam[i].std;
			max_range = MCMCParam[i].mean + 4*MCMCParam[i].std;
		}
		if (MCMCParam[i].type == 1)
		{
			MCMCParam[i].mean = sqrt(MCMCParam[i].low*MCMCParam[i].high);
			MCMCParam[i].std = (log(MCMCParam[i].high) - log(MCMCParam[i].mean))/2.0;
			min_range = MCMCParam[i].mean * exp(-4*MCMCParam[i].std);
			max_range = MCMCParam[i].mean * exp(4*MCMCParam[i].std);
		}


		double dp = abs(max_range - min_range) / n_bins;

		B.t[0] = min_range + dp/2;
		for (int j=0; j<n_bins-1; j++)
			B.t[j+1] = B.t[j] + dp;

		if (MCMCParam[i].type != 1)
			for (int j=0; j<n_bins; j++)
				B.C[j] = exp(-pow(B.t[j]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2)))/(MCMCParam[i].std*pow(6.28,0.5));

		if (MCMCParam[i].type == 1)
			for (int j=0; j<n_bins; j++)
				B.C[j] = exp(-pow(log(B.t[j])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2)))/(B.t[j]*MCMCParam[i].std*pow(6.28,0.5));

		A.BTC[i] = B;
	}

	return A;
}

template<class T>
void CMCMC<T>::getrealizations(CBTCSet &MCMCout)
{
	BTCout_obs.resize(1);
	BTCout_obs_noise.resize(1);
#ifdef GIFMOD
	int n_BTCout_obs = G.measured_quan.size();
#endif
#ifdef GWA
	int n_BTCout_obs = G.Medium[0].measured_quan.size();
#endif
	for (int i = 0; i < 1; i++)
	{
		BTCout_obs[i].resize(n_BTCout_obs);
		BTCout_obs_noise[i].resize(n_BTCout_obs);
	}
	for (int i = 0; i < n_BTCout_obs; i++)
	{
		BTCout_obs[0][i] = CBTCSet(n_realizations);
		BTCout_obs_noise[0][i] = CBTCSet(n_realizations);
		BTCout_obs[0][i].names.clear();
		BTCout_obs_noise[0][i].names.clear();
	}

	realized_paramsList = CBTCSet(MCMCout.nvars);
	paramsList = CBTCSet(MCMCout.nvars);
	//qDebug() << "paramsList.names.size()" << paramsList.names.size();
	paramsList.names = MCMCout.names;
	//qDebug() << "MCMCout.names.size()" << MCMCout.names.size();

	for (int j = 0; j < n_realizations; j++)
	{
		vector<double> param = MCMCout.getrandom(n_burnout);
		realized_paramsList.append(j, param);
	}
	//qDebug() << 612;
	//progress->setValue(0);
	double pValue = 0;
	double inc = 100.0 / n_realizations;
	for (int jj = 0; jj <= n_realizations/numberOfThreads; jj++)
	{

#ifdef GIFMOD
		vector<vector<CMediumSet>> Sys1(numberOfThreads);
#endif
#ifdef GWA
		vector<vector<CGWASet>> Sys1(numberOfThreads);
#endif
		for (int i = 0; i < numberOfThreads; i++) Sys1[i].resize(1);

        omp_set_num_threads(numberOfThreads);
#pragma omp parallel for
		for (int j = 0; j < min(numberOfThreads, n_realizations - jj*numberOfThreads); j++)
		{
			int realizationNumber = jj*numberOfThreads + j;
			cout << "Realization Sample No. : " << realizationNumber << std::endl;
			//qDebug() << "Realization Sample No. : " << realizationNumber;
			vector<double> param = realized_paramsList.getrow(realizationNumber);
			Sys1[j][0] = G;
			Sys1[j][0].ID = numbertostring(j);
#ifdef GIFMOD
			//Sys1[j][0].FI.write_details = false;
#endif
#ifdef GWA
			Sys1[j][0].Medium[0].project = false;
#endif
			int l = 0;
			for (int i = 0; i < nActParams; i++)
#ifdef GIFMOD
			Sys1[j][0].set_param(i, param[i]);
			Sys1[j][0].finalize_set_param();
			Sys1[j][0].calc_log_likelihood();
#endif
#ifdef GWA
			Sys1[j][0].Medium[0].set_param(i, param[i]);
			Sys1[j][0].Medium[0].finalize_set_param();
			Sys1[j][0].Medium[0].calc_log_likelihood();
#endif
			if (global_sensitivity == true)
				global_sens_lumped.push_back(sensitivity_mat_lumped(dp_sens, param));

			for (int i = 0; i < n_BTCout_obs; i++)
			{
				for (int ts = 0; ts < 1; ts++)
				{
#ifdef GIFMOD
					BTCout_obs[ts][i].BTC[realizationNumber] = Sys1[j][ts].ANS_obs.BTC[i];
					BTCout_obs[ts][i].names.push_back(Sys1[j][ts].ANS_obs.names[i] + "_" + to_string(realizationNumber));

					if (noise_realization_writeout)
					{
						BTCout_obs_noise[ts][i].BTC[realizationNumber] = Sys1[j][ts].ANS_obs_noise.BTC[i];
						BTCout_obs_noise[ts][i].names.push_back(Sys1[j][ts].ANS_obs_noise.names[i] + "_" + to_string(realizationNumber));
					}
#endif
#ifdef GWA
					BTCout_obs[ts][i].BTC[realizationNumber] = Sys1[j][ts].Medium[0].ANS_obs.BTC[i];
					BTCout_obs[ts][i].names.push_back(Sys1[j][ts].Medium[0].ANS_obs.names[i] + "_" + to_string(realizationNumber));

					if (noise_realization_writeout)
					{
						BTCout_obs_noise[ts][i].BTC[realizationNumber] = Sys1[j][ts].Medium[0].ANS_obs_noise.BTC[i];
						BTCout_obs_noise[ts][i].names.push_back(Sys1[j][ts].Medium[0].ANS_obs_noise.names[i] + "_" + to_string(realizationNumber));
					}
#endif
				}
			}
			//qDebug() << "Realization Completed : " << realizationNumber << endl;
			cout << "Realization Completed : " << realizationNumber;
			//qDebug() << "Progress: " << pValue << inc;
			pValue += inc;
			//qDebug() << "Progress: " << pValue;
		}
		//qDebug() << "Progress: " << pValue << inc;
		pValue += inc*numberOfThreads;
		//qDebug() << "Progress: " << pValue;
		if (rtw)
		{
			QMap<QString, QVariant> vars;
			vars["mode"] = "MCMC";
			vars["progress"] = pValue;
			rtw->update(vars);
		}
	}
	if (rtw)
	{
		QMap<QString, QVariant> vars;
		vars["mode"] = "MCMC";
		vars["progress"] = 100;
		vars["finished"] = 1;
		rtw->update(vars);
	}
}

template<class T>
void CMCMC<T>::get_outputpercentiles(CBTCSet &MCMCout)
{
	//qDebug() << 501;
	getrealizations(MCMCout);
	//qDebug() << 502;
#ifdef GIFMOD
	int n_BTCout_obs = G.measured_quan.size();
#endif
#ifdef GWA
	int n_BTCout_obs = G.Medium[0].measured_quan.size();
#endif
//qDebug() << 503;
	BTCout_obs_prcntle.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle[j].resize(n_BTCout_obs);
	BTCout_obs_prcntle_noise.resize(1); for (int j = 0; j < 1; j++) BTCout_obs_prcntle_noise[j].resize(n_BTCout_obs);
	//qDebug() << 504;
	if (calc_output_percentiles.size()>0)
		for (int i = 0; i < n_BTCout_obs; i++)
		{
			for (int j = 0; j < 1; j++)
			{
				BTCout_obs_prcntle[j][i] = BTCout_obs[j][i].getpercentiles(calc_output_percentiles);
#ifdef GIFMOD
				BTCout_obs_prcntle[j][i].writetofile(G.FI.outputpathname + "BTC_obs_prcntl_" + G.measured_quan[i].name + ".txt", G.FI.write_interval);
#endif
#ifdef GWA
				BTCout_obs_prcntle[j][i].writetofile(G().pathname + "BTC_obs_prcntl" + to_string(i) + "_" + to_string(j) + ".txt", G.Medium[0].writeinterval);
#endif

				if (noise_realization_writeout)
					BTCout_obs_prcntle_noise[j][i] = BTCout_obs_noise[j][i].getpercentiles(calc_output_percentiles);
#ifdef GIFMOD
				BTCout_obs_prcntle_noise[j][i].writetofile(G.FI.outputpathname + "BTC_obs_prcntl_noise_" + G.measured_quan[i].name + ".txt", G.FI.write_interval);
#endif
#ifdef GWA
				BTCout_obs_prcntle_noise[j][i].writetofile(G().pathname + "BTC_obs_prcntl_noise" + to_string(i) + "_" + to_string(j) + ".txt", G.Medium[0].writeinterval);
#endif

			}
		}
	//qDebug() << 505;
}
