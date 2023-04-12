

// GA.cpp: implementation of the CGA class.
////////////////////////////////////////////////////////////////////////
#include "GA.h"
#include <stdlib.h>
#ifndef mac_version
#include <omp.h>
#endif
#include "Expression.h"
#include "Object.h"

#ifdef Q_version
    #include "runtimewindow.h"
#endif

template<class T>
CGA<T>::CGA()
{
	GA_params.maxpop = 100;
	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);
	fitdist = CDistribution(GA_params.maxpop);
	GA_params.N = 1;       //MM N=1 by default
	GA_params.pcross = 1;
	GA_params.cross_over_type = 1;
	MaxFitness = 0;
    numberOfThreads = 16;
}

template<class T>
CGA<T>::CGA(string filename, const T &model)
{
	Model = model;
	ifstream file(filename);
	GA_params.nParam = 0;
	GA_params.pcross = 1;
	GA_params.N = 1;
	GA_params.RCGA = false;
    numberOfThreads = 20;
    filenames.pathname = Model->OutputPath();
	vector<string> s;
	while (file.eof() == false)
	{
		s = aquiutils::getline(file);
		if (s.size()>0)
		{	if (s[0] == "maxpop") GA_params.maxpop = atoi(s[1].c_str());
			if (s[0] == "ngen") GA_params.nGen = atoi(s[1].c_str());
			if (s[0] == "pcross") GA_params.pcross = atof(s[1].c_str());
			if (s[0] == "pmute") GA_params.pmute = atof(s[1].c_str());
			if (s[0] == "shakescale") GA_params.shakescale = atof(s[1].c_str());
			if (s[0] == "shakescalered") GA_params.shakescalered = atof(s[1].c_str());
			if (s[0] == "outputfile") filenames.outputfilename = s[1];
			if (s[0] == "getfromfilename") filenames.getfromfilename = s[1].c_str();
			if (s[0] == "initial_population") filenames.initialpopfilemame = s[1];
			if (s[0] == "numthreads") numberOfThreads = atoi(s[1].c_str());
		}
	}

	file.close();
    for (int i=0; i<Model->Parameters().size(); i++)
	{
        GA_params.nParam++;
        params.push_back(i);
        if (Model->Parameters()[i]->GetPriorDistribution() == "lognormal")
        {	minval.push_back(log10(Model->Parameters()[i]->GetVal("low")));
            maxval.push_back(log10(Model->Parameters()[i]->GetVal("high")));

        }
        else
        {
            minval.push_back(Model->Parameters()[i]->GetVal("low"));
            maxval.push_back(Model->Parameters()[i]->GetVal("high"));
        }
        apply_to_all.push_back(false);
        if (Model->Parameters()[i]->GetPriorDistribution() == "lognormal")
            loged.push_back(1);
        else
            loged.push_back(0);

        paramname.push_back(Model->Parameters().getKeyAtIndex(i));

	}


	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);

	fitdist = CDistribution(GA_params.maxpop);
	GA_params.cross_over_type = 1;

	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i] = CIndividual(GA_params.nParam);
        Ind[i].fit_measures.resize(model->ObservationsCount()*3);
		Ind_old[i] = CIndividual(GA_params.nParam);
        Ind_old[i].fit_measures.resize(model->ObservationsCount()*3);
	}

	for (int i = 0; i<GA_params.nParam; i++)
		Setminmax(i, minval[i], maxval[i],4);

	MaxFitness = 0;
}

template<class T>
CGA<T>::CGA(T *model)
{
    Model = model;
	GA_params.nParam = 0;
	GA_params.pcross = 1;
	GA_params.N = 1;
	GA_params.RCGA = false;
    numberOfThreads = 20;
    filenames.pathname = Model->OutputPath();
    GA_params.maxpop = max(1,GA_params.maxpop);
    for (unsigned int i=0; i<Model->Parameters().size(); i++)
	{
        GA_params.nParam++;
        params.push_back(i);
        if (Model->Parameters()[i]->GetPriorDistribution() == "lognormal")
        {	minval.push_back(log10(Model->Parameters()[i]->GetVal("low")));
            maxval.push_back(log10(Model->Parameters()[i]->GetVal("high")));

        }
        else
        {
            minval.push_back(Model->Parameters()[i]->GetVal("low"));
            maxval.push_back(Model->Parameters()[i]->GetVal("high"));
        }
        apply_to_all.push_back(false);
        if (Model->Parameters()[i]->GetPriorDistribution() == "lognormal")
            loged.push_back(1);
        else
            loged.push_back(0);

        paramname.push_back(Model->Parameters().getKeyAtIndex(i));

	}


	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);

	fitdist = CDistribution(GA_params.nParam);
	GA_params.cross_over_type = 1;

	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i] = CIndividual(GA_params.nParam);
        Ind[i].fit_measures.resize(model->ObservationsCount()*3);
		Ind_old[i] = CIndividual(GA_params.nParam);
        Ind_old[i].fit_measures.resize(model->ObservationsCount()*3);
	}

	for (int j = 0; j < GA_params.nParam; j++)
		Setminmax(j, minval[j], maxval[j], 4);

	MaxFitness = 0;
}


template<class T>
void CGA<T>::setnparams(int n_params)
{
	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);
	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i] = CIndividual(n_params);
        Ind[i].fit_measures.resize(Model->ObservationsCount()*3);
		Ind_old[i] = CIndividual(n_params);
        Ind_old[i].fit_measures.resize(Model->ObservationsCount()*3);

	}


}

template<class T>
void CGA<T>::setnumpop(int n)
{
	GA_params.maxpop = n;
	CIndividual TempInd = Ind[0];

	int nParam = Ind[0].nParams;
	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);
	for (int i=0; i<n; i++)
	{
		Ind[i] = CIndividual(GA_params.nParam);
		Ind_old[i] = CIndividual(GA_params.nParam);
		for (int j = 0; j<nParam; j++)
		{
			Ind[i].minrange[j] = TempInd.minrange[j];
			Ind[i].maxrange[j] = TempInd.maxrange[j];
			Ind[i].precision[j] = TempInd.precision[j];
            Ind[i].fit_measures.resize(Model->ObservationsCount()*3);
			Ind_old[i].minrange[j] = TempInd.minrange[j];
			Ind_old[i].maxrange[j] = TempInd.maxrange[j];
			Ind_old[i].precision[j] = TempInd.precision[j];
            Ind_old[i].fit_measures.resize(Model->ObservationsCount()*3);
		}

	}
	fitdist = CDistribution(GA_params.maxpop);
}

template<class T>
CGA<T>::CGA(const CGA<T> &C)
{
    GA_params.maxpop = C.maxpop;
	Ind.resize(GA_params.maxpop);
	Ind_old.resize(GA_params.maxpop);
	Ind = C.Ind;
    GA_params = C.GA_params;
    filenames = C.filenames;
    params = C.params;
    loged = C.loged;
	fitdist = C.fitdist;
	MaxFitness = C.MaxFitness;
	paramname = C.paramname;

}

template<class T>
CGA<T> CGA<T>::operator=(CGA<T> &C)
{
    GA_params = C.GA_params;
	filenames = C.filenames;
	Ind = C.Ind;
    Ind_old = C.Ind_old;
	params = C.params;
	loged = C.loged;
	fitdist = C.fitdist;
	MaxFitness = C.MaxFitness;
	paramname = C.paramname;

	return *this;

}

template<class T>
CGA<T>::~CGA()
{

}

template<class T>
void CGA<T>::initialize()
{
	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i].initialize();
	}

	if (filenames.initialpopfilemame!="")
	{
		getinifromoutput(filenames.pathname+filenames.initialpopfilemame);
		for (int i=0; i<initial_pop.size(); i++)
			for (int j=0; j<max(int(initial_pop[i].size()),GA_params.nParam); j++)
				if (loged[j]==1)
					Ind[i].x[j] = log10(initial_pop[i][j]);
				else
					Ind[i].x[j] = initial_pop[i][j];
	}
}

template<class T>
void CGA<T>::Setminmax(int a, double minrange, double maxrange, int prec)
{
	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i].maxrange[a] = maxrange;
		Ind[i].minrange[a] = minrange;
		Ind[i].precision[a] = prec;
	}

}

template<class T>
void CGA<T>::assignfitnesses()
{
	sumfitness = 0;

	vector<vector<double>> inp;

	inp.resize(GA_params.maxpop);


	for (int k=0; k<GA_params.maxpop; k++)
		inp[k].resize(GA_params.nParam);

	vector<double> time_(GA_params.maxpop);
	vector<int> epochs(GA_params.maxpop);
	clock_t t0,t1;
    Models.clear();
    Models.resize(GA_params.maxpop);
	for (int k = 0; k < GA_params.maxpop; k++)
	{
		for (int i = 0; i < GA_params.nParam; i++)
		{
			if (loged[i] != 1)
			{
				inp[k][i] = Ind[k].x[i];    //Ind
			}
			else
			{
				inp[k][i] = pow(10, Ind[k].x[i]);
			}
		}

        Ind[k].actual_fitness = 0;

        Models[k] = *Model;
        Models[k].SetSilent(true);
		Models[k].SetRecordResults(false);
        Models[k].SetNumThreads(1);
		for (int i = 0; i < GA_params.nParam; i++)
			Models[k].SetParameterValue(i, inp[k][i]);
        Models[k].ApplyParameters();

	}


#ifndef NO_OPENMP
	omp_set_num_threads(numberOfThreads);
#endif
int counter=0;
#ifndef NO_OPENMP
#pragma omp parallel for //private(ts,l)
#endif
		for (int k=0; k<GA_params.maxpop; k++)
		{

			FILE *FileOut;
#ifndef NO_OPENMP
#pragma omp critical
#endif
            {
                FileOut = fopen((filenames.pathname+"detail_GA.txt").c_str(),"a");
                fprintf(FileOut, "%i, ", k);
                for (int l=0; l<Ind[0].nParams; l++)
                    if (loged[l]==1)
                        fprintf(FileOut, "%le, ", pow(10,Ind[k].x[l]));
                    else
                        fprintf(FileOut, "%le, ", Ind[k].x[l]);

                //fprintf(FileOut, "%le, %le, %i, %e, %i, %i", Ind[k].actual_fitness, Ind[k].fitness, Ind[k].rank, time_[k], threads_num[k],num_threads[k]);
                //fprintf(FileOut, "%le, %le, %i, %e", Ind[k].actual_fitness, Ind[k].fitness, Ind[k].rank, time_[k]);
                fprintf(FileOut, "\n");
                fclose(FileOut);
            }
            time_t t0 = time(nullptr);

#ifdef Debug_GA
            Models[k].SavetoScriptFile(filenames.pathname+"/temp/model_" + aquiutils::numbertostring(k) +"_" +aquiutils::numbertostring(current_generation)+".ohq",string(""), vector<string>());
#endif
            Models[k].Solve();
            Ind[k].actual_fitness = Models[k].GetObjectiveFunctionValue();
            for (unsigned int i=0; i<Models[k].fit_measures.size(); i++)
                Ind[k].fit_measures[i] = Models[k].fit_measures[i];
#ifdef Debug_GA
            Models[k].GetModeledObjectiveFunctions().writetofile(filenames.pathname+"/temp//observedoutputs_"+aquiutils::numbertostring(k)+"_"+aquiutils::numbertostring(current_generation)+".txt");
            Models[k].GetOutputs().writetofile(filenames.pathname+"/temp//outputs_"+aquiutils::numbertostring(k)+"_"+aquiutils::numbertostring(current_generation)+".txt");
#endif
			epochs[k] += Models[k].EpochCount();
            time_[k] = time(nullptr)-t0;
            counter++;
#ifndef NO_OPENMP
#pragma omp critical
#endif
        {
#ifdef Q_version
			if (rtw != nullptr)
			{
#ifndef NO_OPENMP
                if (omp_get_thread_num() == 0)
#endif
                {
                    rtw->SetProgress2(double(counter + 1) / GA_params.maxpop);
                    QCoreApplication::processEvents();
                }
			}
#endif

            {
                FileOut = fopen((filenames.pathname+"detail_GA.txt").c_str(),"a");
                fprintf(FileOut, "%i, fitness=%e, time=%e, internal_time=%e, failed=%i\n", k, Ind[k].actual_fitness, time_[k], double(Models[k].GetSimulationDuration()), Models[k].GetSolutionFailed());
                fclose(FileOut);
            }

		}
    }
	Model_out = Models[maxfitness()];
#ifdef Q_version
    if (rtw != nullptr)
    {
        rtw->SetProgress2(1);
        QCoreApplication::processEvents();
    }
#endif
	inp.clear();
	assignfitness_rank(GA_params.N);

}

template<class T>
void CGA<T>::crossover()
{

    Ind_old = Ind;
	int a = maxfitness();
	Ind[0] = Ind_old[a];
	Ind[1] = Ind_old[a];
	for (int i=2; i<GA_params.maxpop; i+=2)
	{
        ////qDebug()<<"i = "<< i;
		int j1 = fitdist.GetRand();
		int j2 = fitdist.GetRand();
		double x = GetRndUniF(0,1);
		if (x<GA_params.pcross)
			if (GA_params.cross_over_type == 1)
				cross(Ind_old[j1], Ind_old[j2], Ind[i], Ind[min(i+1,GA_params.maxpop-1)]);   //1 Breaking point
			else
				cross2p(Ind_old[j1], Ind_old[j2], Ind[i], Ind[min(i + 1, GA_params.maxpop - 1)]);  //2 Breaking point
		else
		{
			Ind[i] = Ind_old[j1];
			Ind[i+1] = Ind_old[j2];
		}

	}

}

template<class T>
void CGA<T>::crossoverRC()
{

	for (int i=0; i<GA_params.maxpop; i++)
		Ind_old[i] = Ind[i];
	int a = maxfitness();
	Ind[0] = Ind_old[a];
	Ind[1] = Ind_old[a];
	for (int i=2; i<GA_params.maxpop; i+=2)
	{
		int j1 = fitdist.GetRand();
		int j2 = fitdist.GetRand();
		double x = GetRndUnif(0,1);
		if (x<GA_params.pcross)
			cross_RC_L(Ind_old[j1], Ind_old[j2], Ind[i], Ind[i+1]);
		else
		{
			Ind[i] = Ind_old[j1];
			Ind[i+1] = Ind_old[j2];
		}
	}
}

template<class T>
bool CGA<T>::SetProperty(const string &varname, const string &value)
{
    if (aquiutils::tolower(varname) == "maxpop") {GA_params.maxpop = aquiutils::atoi(value); setnumpop(GA_params.maxpop); return true;}
    if (aquiutils::tolower(varname) == "ngen") {GA_params.nGen = aquiutils::atoi(value); return true;}
	if (aquiutils::tolower(varname) == "pcross") {GA_params.pcross = aquiutils::atof(value); return true;}
	if (aquiutils::tolower(varname) == "pmute") {GA_params.pmute = aquiutils::atof(value); return true;}
	if (aquiutils::tolower(varname) == "shakescale") {GA_params.shakescale = aquiutils::atof(value); return true;}
	if (aquiutils::tolower(varname) == "shakescalered") {GA_params.shakescalered = aquiutils::atof(value); return true;}
	if (aquiutils::tolower(varname) == "outputfile") {filenames.outputfilename = value; return true;}
	if (aquiutils::tolower(varname) == "getfromfilename") {filenames.getfromfilename = value.c_str(); return true;}
	if (aquiutils::tolower(varname) == "initial_population") {filenames.initialpopfilemame = value; return true;}
	if (aquiutils::tolower(varname) == "numthreads") {numberOfThreads = aquiutils::atoi(value.c_str()); return true;}
    last_error = "Property '" + varname + "' was not found!";
    return false;
}


template<class T>
double CGA<T>::avgfitness()
{
	double sum=0;
	for (int i=0; i<GA_params.maxpop; i++)
		sum += Ind[i].fitness;
	return sum/GA_params.maxpop;
}

template<class T>
void CGA<T>::write_to_detailed_GA(string s)
{
	FILE *FileOut;

    FileOut = fopen((filenames.pathname + "detail_GA.txt").c_str(), "a");
    fprintf(FileOut, "%s\n", s.c_str());
	fclose(FileOut);

}

template<class T>
void CGA<T>::SetParameters(Object *obj)
{
    for (unordered_map<string,Quan>::iterator it=obj->GetVars()->begin(); it!=obj->GetVars()->end(); it++)
    {
        SetProperty(it->first,it->second.GetProperty());
    }
}

template<class T>
int CGA<T>::optimize()
{
	#ifdef Q_version
	QCoreApplication::processEvents();
	#endif // Q_version
	string RunFileName = filenames.pathname + filenames.outputfilename;

	FILE *FileOut;
	FILE *FileOut1;

	FileOut = fopen(RunFileName.c_str(),"w");
	fclose(FileOut);
    FileOut1 = fopen((filenames.pathname + "detail_GA.txt").c_str(), "w");
	fclose(FileOut1);

	double shakescaleini = GA_params.shakescale;

	vector<double> X(Ind[0].nParams);

	initialize();
	double ininumenhancements = GA_params.numenhancements;
	GA_params.numenhancements = 0;

	CMatrix Fitness(GA_params.nGen, 3);

    for (current_generation=0; current_generation<GA_params.nGen; current_generation++)
	{

		write_to_detailed_GA("Assigning fitnesses ...");
        Models.clear();
        Models.resize(GA_params.maxpop);
        assignfitnesses();

		write_to_detailed_GA("Assigning fitnesses done!");
		FileOut = fopen(RunFileName.c_str(),"a");
        printf("Generation: %i\n", current_generation);
        fprintf(FileOut, "Generation: %i\n", current_generation);
		fprintf(FileOut, "ID, ");
		for (int k=0; k<Ind[0].nParams; k++)
			fprintf(FileOut, "%s, ", paramname[k].c_str());
        fprintf(FileOut, "%s, %s, %s, ", "likelihood", "Fitness", "Rank");
        for (unsigned int i=0; i<Model->ObservationsCount();i++)
        {
            fprintf(FileOut, "%s, %s, %s,", (Model->observation(i)->GetName()+"_MSE").c_str(), (Model->observation(i)->GetName()+"_R2").c_str(), (Model->observation(i)->GetName()+"_NSE").c_str());
        }
        fprintf(FileOut, "\n");
        write_to_detailed_GA("Generation: " + aquiutils::numbertostring(current_generation));
		for (int j1=0; j1<GA_params.maxpop; j1++)
		{

			fprintf(FileOut, "%i, ", j1);

			for (int k=0; k<Ind[0].nParams; k++)
				if (loged[k] == 1)
					fprintf(FileOut, "%le, ", pow(10, Ind[j1].x[k]));
				else
					fprintf(FileOut, "%le, ", Ind[j1].x[k]);

            fprintf(FileOut, "%le, %le, %i, ", Ind[j1].actual_fitness, Ind[j1].fitness, Ind[j1].rank);
            for (unsigned int i=0; i<Model->ObservationsCount();i++)
            {
                fprintf(FileOut, ",%le, %le, %le", Ind[j1].fit_measures[i*3], Ind[j1].fit_measures[i*3+1], Ind[j1].fit_measures[i*3+2]);
            }
			fprintf(FileOut, "\n");
		}
		fclose(FileOut);

		int j = maxfitness();

        Fitness[current_generation][0] = Ind[j].actual_fitness;

#ifdef Q_version
    if (rtw)
    {   if (current_generation==0) rtw->SetYRange(0,Ind[j].actual_fitness*1.1);
        rtw->SetProgress(double(current_generation)/double(GA_params.nGen));
        rtw->AddDataPoint(current_generation+1,Ind[j].actual_fitness);
        rtw->Replot();
        QCoreApplication::processEvents();
    }
#endif
        if (current_generation>10)
		{
            if ((Fitness[current_generation][0] == Fitness[current_generation - 3][0]) && GA_params.shakescale>pow(10.0, -Ind[0].precision[0]))
				GA_params.shakescale *= GA_params.shakescalered;


            if ((Fitness[current_generation][0]>Fitness[current_generation - 1][0]) && (GA_params.shakescale<shakescaleini))
				GA_params.shakescale /= GA_params.shakescalered;
			GA_params.numenhancements = 0;
		}

        if (current_generation>50)
		{
            if (Fitness[current_generation][0] == Fitness[current_generation - 20][0])
			{
				GA_params.numenhancements *= 1.05;
				if (GA_params.numenhancements == 0) GA_params.numenhancements = ininumenhancements;
			}

            if (Fitness[current_generation][0] == Fitness[current_generation - 50][0])
				GA_params.numenhancements = ininumenhancements * 10;
		}

        Fitness[current_generation][1] = GA_params.shakescale;
        Fitness[current_generation][2] = GA_params.pmute;

        if (current_generation>20)
		{
            if (GA_params.shakescale == Fitness[current_generation - 20][1])
				GA_params.shakescale = shakescaleini;
		}


		j = maxfitness();
		MaxFitness = Ind[j].actual_fitness;

        Fitness[current_generation][0] = Ind[j].actual_fitness;


		fillfitdist();

		write_to_detailed_GA("Cross-over ...");

		if (GA_params.RCGA == true)
			crossoverRC();
		else
			crossover();

		write_to_detailed_GA("Cross-over done! ");

		write_to_detailed_GA("Mutation ...");

		mutate(GA_params.pmute);
		write_to_detailed_GA("Mutation done!");
		write_to_detailed_GA("Shake...!");
		shake();
		write_to_detailed_GA("Shake done!");


	}
    Models.clear();
    Models.resize(GA_params.maxpop);
    assignfitnesses();
	FileOut = fopen(RunFileName.c_str(), "a");
	fprintf(FileOut, "Final Enhancements\n");

	int j = maxfitness();

	MaxFitness = Ind[j].actual_fitness;
	final_params.resize(GA_params.nParam);


	for (int k = 0; k<Ind[0].nParams; k++)
	{
		if (loged[k] == 1) final_params[k] = pow(10, Ind[j].x[k]); else final_params[k] = Ind[j].x[k];
		fprintf(FileOut, "%s, ", paramname[k].c_str());
		fprintf(FileOut, "%le, ", final_params[k]);
		fprintf(FileOut, "%le, %le\n", Ind[j].actual_fitness, Ind[j].fitness);
	}
    for (unsigned int i=0; i<Model->ObservationsCount();i++)
    {
        fprintf(FileOut, "%le, %le, %le\n", Ind[j].fit_measures[i*3], Ind[j].fit_measures[i*3+1], Ind[j].fit_measures[i*3+2]);
    }
    fclose(FileOut);

	assignfitnesses(final_params);

#ifdef Q_version
	if (rtw)
	{
		rtw->SetProgress(1.0);
		QCoreApplication::processEvents();
	}
#endif

	Models.clear();

	return maxfitness();
}

template<class T>
double CGA<T>::assignfitnesses(vector<double> inp)
{

	double likelihood = 0;
    T Model1;
    Model1 = *Model;

	for (int i = 0; i < GA_params.nParam; i++)
        Model1.SetParameterValue(i, inp[i]);

    Model1.ApplyParameters();
    Model1.Solve();
    likelihood -= Model1.GetObjectiveFunctionValue();

    Model_out = Model1;
    Model_out.TransferResultsFrom(&Model1);
	return likelihood;

}
/*
template<class T>
vector<T>& CGA<T>::assignfitnesses_p(vector<double> inp) //Generates an instance of the model with the provided parameters
{
	double likelihood = 0;
	vector<T> Models(1);
	for (int ts = 0; ts<1; ts++)
	{
		Models[ts] = Model;

		int l = 0;
		for (int i = 0; i<GA_params.nParam; i++)
			Models[ts].SetParam(params[i], inp[getparamno(i, ts)]);
		Models[ts].FinalizeSetParams();
		likelihood -= Models[ts].EvaluateObjectiveFunction();
	}
	return Models;
}

template<class T>
int CGA<T>::get_act_paramno(int i)
{
	int l = -1;
	for (int j = 0; j<GA_params.nParam; j++)
	{
		if (apply_to_all[j]) l++; else l += 1;
		if (l >= i)
		{
			if (apply_to_all[j]) l -= 1; else l--;
			return j;
		}
	}
}
*/
template<class T>
double CGA<T>::getfromoutput(string filename)
{
	ifstream file(filename);
	vector<string> s;
	final_params.resize(GA_params.nParam);
	while (file.eof() == false)
	{
		s = aquiutils::getline(file);
		if (s.size()>0)
		{
			if (s[0] == "Final Enhancements")
				for (int i = 0; i<GA_params.nParam; i++)
				{
					s = aquiutils::getline(file);
					if (s.size() == 0)
						write_to_detailed_GA("The number of parameters in GA output file does not match the number of unknown parameters");
					else
						final_params[i] = atof(s[1].c_str());
				}
		}
	}
	double ret = assignfitnesses(final_params);
	return ret;
}


template<class T>
int CGA<T>::getparamno(int i, int ts)
{
	int l = 0;
	for (int j = 0; j<i; j++)
		if (apply_to_all[j]) l++; else l += 1;

	if (apply_to_all[i])
		return l;
	else
		return l + ts;

}
template<class T>
int CGA<T>::get_time_series(int i)
{
	int l = 0;
	for (int j = 0; j<GA_params.nParam; j++)
	{
		if (apply_to_all[j]) l += 1; else l += 1;
		if (l >= i)
		{
			if (apply_to_all[j]) l -= 1; else l--;
			return i - l;
		}
	}
}


template<class T>
void CGA<T>::shake()
{
	for (int i=1; i<GA_params.maxpop; i++)
		Ind[i].shake(GA_params.shakescale);

}

template<class T>
void CGA<T>::mutate(double mu)
{
	for (int i=2; i<GA_params.maxpop; i++)
		Ind[i].mutate(mu);

}

template<class T>
int CGA<T>::maxfitness()
{
	double max_fitness = 1E+308 ;
	int i_max = 0;
	for (int i=0; i<GA_params.maxpop; i++)
		if (max_fitness>Ind[i].actual_fitness)
		{
			max_fitness = Ind[i].actual_fitness;
			i_max = i;
		}
	return i_max;

}

template<class T>
double CGA<T>::variancefitness()
{
	double sum = 0;
	double a = avgfitness();
	for (int i=0; i<GA_params.maxpop; i++)
		sum += (a - Ind[i].fitness)*(a - Ind[i].fitness);
	return sum;

}

template<class T>
double CGA<T>::stdfitness()
{
	double sum = 0;
	double a = avg_inv_actual_fitness();
	for (int i=0; i<GA_params.maxpop; i++)
		sum += (a - 1/Ind[i].actual_fitness)*(a - 1/Ind[i].actual_fitness);
	return sqrt(sum)/GA_params.maxpop/a;

}

template<class T>
double CGA<T>::avg_actual_fitness()
{
	double sum=0;
	for (int i=0; i<GA_params.maxpop; i++)
		sum += Ind[i].actual_fitness;
	return sum/GA_params.maxpop;

}

template<class T>
double CGA<T>::avg_inv_actual_fitness()
{
	double sum=0;
	for (int i=0; i<GA_params.maxpop; i++)
		sum += 1/Ind[i].actual_fitness;
	return sum/GA_params.maxpop;

}


template<class T>
void CGA<T>::assignrank()
{
	for (int i=0; i<GA_params.maxpop; i++)
	{
		int r = 1;
		for (int j=0; j<GA_params.maxpop; j++)
		{
			if (Ind[i].actual_fitness > Ind[j].actual_fitness) r++;
		}
		Ind[i].rank = r;
	}

}

template<class T>
void CGA<T>::assignfitness_rank(double N)
{
	assignrank();
	for (int i=0; i<GA_params.maxpop; i++)
	{
		Ind[i].fitness = pow(1.0/static_cast<double>(Ind[i].rank),GA_params.N);
	}
}


template<class T>
void CGA<T>::fillfitdist()
{
       double sum=0;
       for (int i=0; i<GA_params.maxpop; i++)
       {
              sum+=Ind[i].fitness;
       }

       fitdist.s[0] = 0;
       fitdist.e[0] = Ind[0].fitness/sum;
       for (int i=1; i<GA_params.maxpop-1; i++)
       {
              fitdist.e[i] = fitdist.e[i-1] + Ind[i].fitness/sum;
              fitdist.s[i] = fitdist.e[i-1];
       }
       fitdist.s[GA_params.maxpop-1] = fitdist.e[GA_params.maxpop-2];
       fitdist.e[GA_params.maxpop-1] = 1;

}

template<class T>
double CGA<T>::evaluateforward()
{
	vector<double> v(1);
	v[0] = 1;
	CVector out(1);
	int x_nParam = GA_params.nParam;
	vector<int> x_params = params;
	GA_params.nParam = 1;
	params.resize(1);
	params[0] = 100;
	out[0] = assignfitnesses(v);

	out.writetofile(filenames.pathname + "likelihood.txt");
	params = x_params;
	GA_params.nParam = x_nParam;
	return out[0];
}

template<class T>
void CGA<T>::getinifromoutput(string filename)
{
	ifstream file(filename);
	vector<string> s;
	initial_pop.resize(1);
	initial_pop[0].resize(GA_params.nParam);
	while (file.eof() == false)
	{
		s = aquiutils::getline(file);
		if (s.size()>0)
		{	if (s[0] == "Final Enhancements")
			for (int i=0; i<GA_params.nParam; i++)
			{
				s = aquiutils::getline(file);
				if (loged[i]==1)
					initial_pop[0][i] = atof(s[1].c_str());
				else
					initial_pop[0][i] = atof(s[1].c_str());

			}
		}
	}

}

template<class T>
void CGA<T>::getinitialpop(string filename)
{
	ifstream file(filename);
	vector<string> s;

	while (file.eof() == false)
	{
		s = aquiutils::getline(file);

		if (s.size()>0)
		{
			vector<double> x;
			for (int j=0; j<s.size(); j++)
				initial_pop.push_back(aquiutils::ATOF(s));

		}
	}
	file.close();
}


