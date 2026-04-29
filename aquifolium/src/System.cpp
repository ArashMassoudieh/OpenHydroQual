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


#include "System.h"
#include <fstream>
#include <qstring.h>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#pragma warning(pop)
#pragma warning(disable : 4996)
#include <json/json.h>
#include <Script.h>
#ifndef NO_OPENMP
#include <omp.h>
#endif
#include "restorepoint.h"
#ifdef VALGRIND
#include <valgrind/callgrind.h>
#endif
//#define NormalizeByDiagonal

#include <QDebug>


#ifdef SUPER_LU
    #include "Matrix_arma_sp.h"
#endif




const vector<string> System::operators = { "+", "-", "*", "/", "^", "(", ")", ":" };
const vector<string> System::functions = { "abs", "sgn", "exp", "pos", "min", "max", "mon", "mbs", "lpw" };

System::System() : Object::Object()
{
    InitOpenMP();
    Object::AssignRandomPrimaryKey();
}

void System::InitOpenMP()
{
#ifndef NO_OPENMP
    if (!omp_in_parallel())
        omp_set_num_threads(SolverSettings.n_threads);
#endif
}

System::~System()
{
    clear();

}

void System::CopyFrom(const System& other)
{
    blocks = other.blocks;
    links = other.links;
    sources = other.sources;
    constituents = other.constituents;
    reactions = other.reactions;
    reaction_parameters = other.reaction_parameters;
    observations = other.observations;
    objective_function_set = other.objective_function_set;
    parameter_set = other.parameter_set;
    Settings = other.Settings;
    metamodel = other.metamodel;
    silent = other.silent;
    SimulationParameters = other.SimulationParameters;
    SolverSettings = other.SolverSettings;
    solvevariableorder = other.solvevariableorder;
    SolverTempVars = other.SolverTempVars;
    paths = other.paths;
    ParameterEstimationMode = other.ParameterEstimationMode;

    SolverTempVars.SolutionFailed = false;
    Outputs.AllOutputs.clear();
    Outputs.ObservedOutputs.clear();
    SetAllParents();
    InitOpenMP();
    Object::AssignRandomPrimaryKey();
}

System::System(const System& other) : Object::Object(other)
{
    CopyFrom(other);
}

System& System::operator=(const System& rhs)
{
    if (this == &rhs) return *this;
    Clear();
    Object::operator=(rhs);
    CopyFrom(rhs);
    return *this;
}

void System::Clear()
{
    blocks.clear();
    links.clear();
    sources.clear();
    constituents.clear();
    reactions.clear();
    reaction_parameters.clear();
    Settings.clear();                    
    observations.clear();               
    objective_function_set.clear();
    parameter_set.clear();
    Outputs.AllOutputs.clear();
    Outputs.ObservedOutputs.clear();
    metamodel.Clear();
    solvevariableorder.clear();         
    addedpropertiestoallblocks.clear(); 
    addedpropertiestoalllinks.clear();  
    addedtemplates.clear();             
    fit_measures.clear();               
    alltimeseries.clear();              
    errorhandler.clear();               
}


bool System::AddBlock(Block &blk, bool SetQuantities)
{
    blocks.push_back(blk);

    block(blk.GetName())->SetParent(this);
    if (SetQuantities)
        block(blk.GetName())->SetQuantities(metamodel, blk.GetType());
    block(blk.GetName())->SetParent(this);

    AddAllConstituentRelateProperties(block(blk.GetName()));

	return true;
}

bool System::AddSource(Source &src, bool SetQuantities)
{
    sources.push_back(src);
    source(src.GetName())->SetParent(this);
    if (SetQuantities)
        source(src.GetName())->SetQuantities(metamodel, src.GetType());
    source(src.GetName())->SetParent(this);
    AddAllConstituentRelateProperties(source(src.GetName()));
	return true;
}

bool System::AddConstituent(Constituent &cnst, bool SetQuantities)
{
    constituents.push_back(cnst);
    constituent(cnst.GetName())->SetParent(this);
    if (SetQuantities)
        constituent(cnst.GetName())->SetQuantities(metamodel, cnst.GetType());
    constituent(cnst.GetName())->SetParent(this);
    AddConstituentRelateProperties(constituent(cnst.GetName()));
    AddConstituentRelatePropertiestoMetalModel();
    return true;
}

bool System::AddReaction(Reaction &rxn, bool SetQuantities)
{
    reactions.push_back(rxn);

    reaction(rxn.GetName())->SetParent(this);
     if (SetQuantities)
        reaction(rxn.GetName())->SetQuantities(metamodel, rxn.GetType());
    reaction(rxn.GetName())->SetParent(this);
    AddAllConstituentRelateProperties(reaction(rxn.GetName()));
    AddConstituentRelateProperties(reaction(rxn.GetName()));
    return true;
}

bool System::AddReactionParameter(RxnParameter &rxnparam, bool SetQuantities)
{
    reaction_parameters.push_back(rxnparam);
    reactionparameter(rxnparam.GetName())->SetParent(this);
     if (SetQuantities)
        reactionparameter(rxnparam.GetName())->SetQuantities(metamodel, rxnparam.GetType());
    reactionparameter(rxnparam.GetName())->SetParent(this);
    return true;
}

bool System::AddObservation(Observation &obs, bool SetQuantities)
{
    observations.push_back(obs);
    observation(obs.GetName())->SetParent(this);
    if (SetQuantities)
        observation(obs.GetName())->SetQuantities(metamodel, obs.GetType());
    observation(obs.GetName())->SetParent(this);
    return true;

}

bool System::AddLink(Link &lnk, const string &source, const string &destination, bool SetQuantities)
{
	if (!block(source))
	{
		errorhandler.Append("System", "System", "AddLink", "Block '" + source + "' does not exist!", 8791);
		return false;
	}

	if (!block(destination))
	{
		errorhandler.Append("System", "System", "AddLink", "Block '" + destination + "' does not exist!", 8792);
		return false;
	}


	if (!VerifyAsDestination(block(destination), &lnk))
        return false;
    if (!VerifyAsSource(block(source), &lnk))
        return false;
    links.push_back(lnk);
    link(lnk.GetName())->SetParent(this);
    link(lnk.GetName())->SetConnectedBlock(Expression::loc::source, source);
    link(lnk.GetName())->SetConnectedBlock(Expression::loc::destination, destination);
	block(source)->AppendLink(links.size()-1,Expression::loc::source);
	block(destination)->AppendLink(links.size()-1,Expression::loc::destination);
    if (SetQuantities)
        link(lnk.GetName())->SetQuantities(metamodel, lnk.GetType());
	link(lnk.GetName())->SetParent(this);
    AddAllConstituentRelateProperties(link(lnk.GetName()));
	return true;
}

Block *System::block(const string &s)
{
    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetName() == s) return &blocks[i];

    return nullptr;
}

const Block* System::block(const string& s) const
{
    for (unsigned int i = 0; i < blocks.size(); i++)
        if (blocks[i].GetName() == s) return &blocks[i];

    return nullptr;
}

int System::blockid(const string &s)
{
    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetName() == s) return int(i);

    errorhandler.Append(GetName(),"System","blockid","Block '" + s + "' was not found",102);
    return -1;
}

int System::linkid(const string &s)
{
    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetName() == s) return int(i);

    errorhandler.Append(GetName(),"System","linkid","Link '" + s + "' was not found",103);

    return -1;
}

int System::blockid(const string& s) const
{
    for (unsigned int i = 0; i < blocks.size(); i++)
        if (blocks[i].GetName() == s) return int(i);

    return -1;
}

int System::linkid(const string& s) const
{
    for (unsigned int i = 0; i < links.size(); i++)
        if (links[i].GetName() == s) return int(i);

    return -1;
}

Link *System::link(const string &s)
{
    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetName() == s) return &links[i];

    return nullptr;
}

const Link* System::link(const string& s) const
{
    for (unsigned int i = 0; i < links.size(); i++)
        if (links[i].GetName() == s) return &links[i];

    return nullptr;
}


Source *System::source(const string &s)
{
    for (unsigned int i=0; i<sources.size(); i++)
        if (sources[i].GetName() == s) return &sources[i];

    return nullptr;
}

const Source* System::source(const string& s) const
{
    for (unsigned int i = 0; i < sources.size(); i++)
        if (sources[i].GetName() == s) return &sources[i];

    return nullptr;
}

Constituent *System::constituent(const string &s)
{
    for (unsigned int i=0; i<constituents.size(); i++)
        if (constituents[i].GetName() == s) return &constituents[i];

    return nullptr;
}

const Constituent* System::constituent(const string& s) const
{
    for (unsigned int i = 0; i < constituents.size(); i++)
        if (constituents[i].GetName() == s) return &constituents[i];

    return nullptr;
}

Reaction *System::reaction(const string &s)
{
    for (unsigned int i=0; i<reactions.size(); i++)
        if (reactions[i].GetName() == s) return &reactions[i];

    return nullptr;
}

const Reaction* System::reaction(const string& s) const
{
    for (unsigned int i = 0; i < reactions.size(); i++)
        if (reactions[i].GetName() == s) return &reactions[i];

    return nullptr;
}

RxnParameter *System::reactionparameter(const string &s)
{
    for (unsigned int i=0; i<reaction_parameters.size(); i++)
        if (reaction_parameters[i].GetName() == s) return &reaction_parameters[i];

    return nullptr;

}

const RxnParameter* System::reactionparameter(const string& s) const
{
    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        if (reaction_parameters[i].GetName() == s) return &reaction_parameters[i];

    return nullptr;

}

Observation *System::observation(const string &s)
{
    for (unsigned int i=0; i<observations.size(); i++)
        if (observations[i].GetName() == s) return &observations[i];

    return nullptr;

}

const Observation* System::observation(const string& s) const
{
    for (unsigned int i = 0; i < observations.size(); i++)
        if (observations[i].GetName() == s) return &observations[i];

    return nullptr;

}

Object *System::settings(const string &s)
{
    for (unsigned int i=0; i<Settings.size(); i++)
        if (Settings[i].GetName() == s) return &Settings[i];

    return nullptr;
}

const Object* System::settings(const string& s) const
{
    for (unsigned int i = 0; i < Settings.size(); i++)
        if (Settings[i].GetName() == s) return &Settings[i];

    return nullptr;
}


Parameter *System::parameter(const string &s)
{
    if (Parameters().count(s) == 0)
    {
        errorhandler.Append(GetName(),"System","parameter","parameter '" + s + "' was not found",110);
        return nullptr;
    }
    else
        return (Parameters()[s]);
}


const Parameter* System::parameter(const string& s) const
{
    if (Parameters().count(s) == 0)
    {
        return nullptr;
    }
    else
        return (Parameters()[s]);
}

Objective_Function *System::objectivefunction(const string &s)
{
    if (ObjectiveFunctions().count(s) == 0)
    {
        errorhandler.Append(GetName(),"System","objective function","objective function '" + s + "' was not found",111);
        return nullptr;
    }
    else
        return (ObjectiveFunctions()[s]);
}

const Objective_Function* System::objectivefunction(const string& s) const
{
    if (ObjectiveFunctions().count(s) == 0)
    {
        return nullptr;
    }
    else
        return (ObjectiveFunctions()[s]);
}

Object *System::object(const string &s)
{
    for (unsigned int i=0; i<Settings.size(); i++)
        if (Settings[i].GetName() == s) return &Settings[i];

    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetName() == s) return &links[i];

    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetName() == s) return &blocks[i];

    for (unsigned int i=0; i<sources.size(); i++)
        if (sources[i].GetName() == s) return &sources[i];

    for (unsigned int i=0; i<constituents.size(); i++)
        if (constituents[i].GetName() == s) return &constituents[i];

    for (unsigned int i=0; i<reaction_parameters.size(); i++)
        if (reaction_parameters[i].GetName() == s) return &reaction_parameters[i];

    for (unsigned int i=0; i<reactions.size(); i++)
        if (reactions[i].GetName() == s) return &reactions[i];

    for (unsigned int i=0; i<observations.size(); i++)
        if (observations[i].GetName() == s) return &observations[i];

    for (unsigned int i=0; i<ParametersCount(); i++)
        if (Parameters()[i]->GetName() == s) return Parameters()[i];

    for (unsigned int i=0; i<ObjectiveFunctionsCount(); i++)
        if (ObjectiveFunctions()[i]->GetName() == s)
            return ObjectiveFunctions()[i];

    //errorhandler.Append(GetName(),"System","object","Object '" + s + "' was not found",105);

    return nullptr;
}

const Object* System::object(const string& s) const
{
    for (unsigned int i = 0; i < Settings.size(); i++)
        if (Settings[i].GetName() == s) return &Settings[i];

    for (unsigned int i = 0; i < links.size(); i++)
        if (links[i].GetName() == s) return &links[i];

    for (unsigned int i = 0; i < blocks.size(); i++)
        if (blocks[i].GetName() == s) return &blocks[i];

    for (unsigned int i = 0; i < sources.size(); i++)
        if (sources[i].GetName() == s) return &sources[i];

    for (unsigned int i = 0; i < constituents.size(); i++)
        if (constituents[i].GetName() == s) return &constituents[i];

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        if (reaction_parameters[i].GetName() == s) return &reaction_parameters[i];

    for (unsigned int i = 0; i < reactions.size(); i++)
        if (reactions[i].GetName() == s) return &reactions[i];

    for (unsigned int i = 0; i < observations.size(); i++)
        if (observations[i].GetName() == s) return &observations[i];

    for (unsigned int i = 0; i < ParametersCount(); i++)
        if (Parameters()[i]->GetName() == s) return Parameters()[i];

    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        if (ObjectiveFunctions()[i]->GetName() == s)
            return ObjectiveFunctions()[i];

    return nullptr;
}

ErrorHandler System::VerifyAllQuantities()
{
    ErrorHandler errs;
    bool allfine = true; 
    for (unsigned int i=0; i<Settings.size(); i++)
        allfine &= Settings[i].VerifyQuans(&errs);

    for (unsigned int i=0; i<links.size(); i++)
        allfine &= links[i].VerifyQuans(&errs);

    for (unsigned int i=0; i<blocks.size(); i++)
        allfine &= blocks[i].VerifyQuans(&errs);

    for (unsigned int i=0; i<sources.size(); i++)
        allfine &= sources[i].VerifyQuans(&errs);

    for (unsigned int i=0; i<observations.size(); i++)
        allfine &= observations[i].VerifyQuans(&errs);

    for (unsigned int i=0; i<ParametersCount(); i++)
        allfine &= Parameters()[i]->VerifyQuans(&errs);

    for (unsigned int i=0; i<ObjectiveFunctionsCount(); i++)
        allfine &= ObjectiveFunctions()[i]->VerifyQuans(&errs);

    return errs;
}

void System::UnUpdateAllVariables()
{
    for (unsigned int i = 0; i < links.size(); i++)
        links[i].UnUpdateAllValues();

    for (unsigned int i = 0; i < blocks.size(); i++)
        blocks[i].UnUpdateAllValues();

    for (unsigned int i = 0; i < sources.size(); i++)
        sources[i].UnUpdateAllValues();

    for (unsigned int i = 0; i < observations.size(); i++)
        observations[i].UnUpdateAllValues();

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        reaction_parameters[i].UnUpdateAllValues();

    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        ObjectiveFunctions()[i]->UnUpdateAllValues();

    for (unsigned int i = 0; i < ReactionsCount(); i++)
        reactions[i].UnUpdateAllValues();
}

Object *System::GetObjectBasedOnPrimaryKey(const string &s)
{
    for (unsigned int i=0; i<links.size(); i++)
        if (links[i].GetPrimaryKey() == s) return &links[i];

    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].GetPrimaryKey() == s) return &blocks[i];

    for (unsigned int i=0; i<sources.size(); i++)
        if (sources[i].GetPrimaryKey() == s) return &sources[i];

    for (unsigned int i=0; i<ParametersCount(); i++)
        if (Parameters()[i]->GetPrimaryKey() == s) return Parameters()[i];

    //errorhandler.Append(GetName(),"System","object","Object '" + s + "' was not found",105);

    return nullptr;

}

bool System::GetQuanTemplate(const string &filename)
{
    //qDebug()<<QString::fromStdString(filename);
    if (aquiutils::lookup(addedtemplates,filename)==-1)
    {
        if (!metamodel.GetFromJsonFile(filename)) return false;
        addedtemplates.push_back(filename);
        TransferQuantitiesFromMetaModel();
    }
    return true;
}

bool System::AppendQuanTemplate(const string &filename)
{

    if (aquiutils::lookup(addedtemplates,filename)==-1)
    {
        if (!metamodel.AppendFromJsonFile(filename)) return false;
        addedtemplates.push_back(filename);
        TransferQuantitiesFromMetaModel();
    }

    return true;
}


void System::CopyQuansToMembers()
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].SetQuantities(metamodel,blocks[i].GetType());
    }
    for (unsigned int i=0; i<links.size(); i++)
        links[i].SetQuantities(metamodel,links[i].GetType());

    for (unsigned int i=0; i<sources.size(); i++)
        sources[i].SetQuantities(metamodel,sources[i].GetType());


}

vector<bool> System::OneStepSolve()
{
    int transport = 0;
    if (ConstituentsCount()>0) transport = 1;
    vector<bool> success(solvevariableorder.size()+transport);
    for (unsigned int i = 0; i < solvevariableorder.size(); i++)
        success[i] = OneStepSolve(i);

    if (ConstituentsCount()>0)
        success[solvevariableorder.size()] = OneStepSolve(solvevariableorder.size(),true);

    return success;
}

void System::MakeTimeSeriesUniform(const double &increment)
{

#ifdef Q_GUI_SUPPORT
    if (rtw!=nullptr)
        rtw->AppendInfo("Uniformizing of time-series...");
#endif
    for (unsigned int i=0; i<sources.size(); i++)
        sources[i].MakeTimeSeriesUniform(increment);

    for (unsigned int i=0; i<links.size(); i++)
        links[i].MakeTimeSeriesUniform(increment);

    for (unsigned int i=0; i<blocks.size(); i++)
        blocks[i].MakeTimeSeriesUniform(increment);
#ifdef Q_GUI_SUPPORT
    if (rtw!=nullptr)
        rtw->AppendInfo("Uniformizing of time-series (done!)");
#endif

}

bool System::Solve(bool applyparameters, bool uniformizeoutput)
{
#ifndef NO_OPENMP
    omp_init_lock(&lock);
#endif

    InitializeSolver(applyparameters);

    PopulateOutputs(false);
    RestorePoint restorepoint(this);

#ifdef Terminal_version
    cout << "Running from time " << SolverTempVars.t
        << " to " << SimulationParameters.tend << endl;
#endif

    int counter = 0;
    int fail_counter = 0;
    double progress = 0;
    double progress_p = 0;

#ifdef VALGRIND
    CALLGRIND_START_INSTRUMENTATION;
    CALLGRIND_TOGGLE_COLLECT;
#endif

    while (SolverTempVars.t < SimulationParameters.tend + SolverTempVars.dt
        && !stop_triggered)
    {
        progress_p = progress;
        progress = (SolverTempVars.t - SimulationParameters.tstart)
            / (SimulationParameters.tend - SimulationParameters.tstart);
        counter++;

        if (counter % 50 == 0)
            SolverTempVars.SetUpdateJacobian(true);

        SolverTempVars.dt = min(SolverTempVars.dt_base, GetMinimumNextTimeStepSize());
        SolverTempVars.dt = max(SolverTempVars.dt,
            SimulationParameters.dt0 / SolverSettings.timestepminfactor);

        vector<bool> _success = OneStepSolve();

        if (!aquiutils::And(_success))
            HandleSolveFailure(fail_counter, restorepoint);
        else
            HandleSolveSuccess(counter, fail_counter, restorepoint, progress, progress_p);

        if (CheckTerminationConditions())
            stop_triggered = true;

#ifdef Q_GUI_SUPPORT
        if (rtw)
        {
            errorhandler.Flush(rtw);
            QCoreApplication::processEvents();
        }
#else
        errorhandler.Flush();
#endif
    }

#ifdef VALGRIND
    CALLGRIND_TOGGLE_COLLECT;
    CALLGRIND_STOP_INSTRUMENTATION;
#endif

    FinalizeOutputs(uniformizeoutput);
    SetSimulationDuration(time(nullptr) - SolverTempVars.time_start);
    return true;
}

void System::LogMessage(const string& msg, bool isWarning)
{
#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        if (isWarning)
            rtw->AppendWarning(QString::fromStdString(msg));
        else
            rtw->AppendLog(QString::fromStdString(msg));
        QCoreApplication::processEvents();
    }
#endif
    ShowMessage(msg);
    if (GetSolutionLogger())
        GetSolutionLogger()->WriteString(msg);
}

void System::LogDetails(const string& msg)
{
#ifdef Q_GUI_SUPPORT
    if (rtw && rtw->DetailsOn())
    {
        rtw->AppendDetails(QString::fromStdString(msg));
        QCoreApplication::processEvents();
    }
#endif
    if (GetSolutionLogger())
        GetSolutionLogger()->WriteString(msg);
}

void System::InitializeSolver(bool applyparameters)
{
    fit_measures.resize(ObservationsCount() * 3);
    SolverTempVars.time_start = time(nullptr);
    SetAllParents();
    SetQuanPointers();

    if (ConstituentsCount() > 0)
        SetNumberOfStateVariables(solvevariableorder.size() + 1);
    else
        SetNumberOfStateVariables(solvevariableorder.size());

    SolverTempVars.SetUpdateJacobian(true);
    alltimeseries = GetTimeSeries(true);

#ifdef Q_GUI_SUPPORT
    errorhandler.SetProgressWindow(rtw);
#endif

    if (applyparameters)
        ApplyParameters();

    InitiateOutputs();
    WriteObjectsToLogger();

#ifdef Q_GUI_SUPPORT
    QCoreApplication::processEvents();
#endif

    MakeTimeSeriesUniform(SimulationParameters.dt0);

    SolverTempVars.dt_base = SimulationParameters.dt0;
    SolverTempVars.dt = SolverTempVars.dt_base;
    SolverTempVars.t = SimulationParameters.tstart;

    InitiatePrecalculatedFunctions();
    CalculateAllExpressions(Expression::timing::present);
    CalcAllInitialValues();
    UnUpdateAllVariables();

    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        ObjectiveFunctions()[i]->GetTimeSeries()->clear();

    for (unsigned int i = 0; i < ObservationsCount(); i++)
        observation(i)->GetTimeSeries()->clear();

#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        rtw->AppendLog("Simulation Started at "
            + QTime::currentTime().toString(Qt::RFC2822Date) + "!");
        rtw->SetPrimaryChartXRange(SimulationParameters.tstart, SimulationParameters.tend);
        rtw->SetPrimaryChartYRange(0, SimulationParameters.dt0 * SolverSettings.timestepmaxfactor);
        rtw->ReplotPrimaryChart();
        QCoreApplication::processEvents();
    }
#endif

    Update();
}

void System::HandleSolveFailure(int& fail_counter, RestorePoint& restorepoint)
{
    fail_counter++;

    LogDetails(SolverTempVars.fail_reason.back()
        + ", dt = " + aquiutils::numbertostring(SolverTempVars.dt));

#ifndef Q_GUI_SUPPORT
    cout << SolverTempVars.fail_reason.back()
        << ", dt = " << SolverTempVars.dt << endl;
#endif

    SolverTempVars.dt_base *= SolverSettings.NR_timestep_reduction_factor_fail;
    SolverTempVars.dt_base = max(SolverTempVars.dt_base,
        SimulationParameters.dt0 / (2 * SolverSettings.timestepminfactor));
    SolverTempVars.SetUpdateJacobian(true);

#ifdef Q_GUI_SUPPORT
    if (rtw && rtw->Cancelled())
        stop_triggered = true;
#endif

    if (fail_counter > 20)
    {
        if (!ResetBasedOnRestorePoint(&restorepoint))
        {
            LogMessage("The attempt to solve the problem failed!", true);
            SolverTempVars.SolutionFailed = true;
            stop_triggered = true;
        }
        else
        {
            LogDetails("Reseting to the restore point saved @ t = "
                + aquiutils::numbertostring(restorepoint.t));
        }
    }
}

void System::HandleSolveSuccess(int& counter, int& fail_counter,
    RestorePoint& restorepoint,
    double progress, double progress_p)
{
#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        rtw->SetProgress(progress);
        rtw->AddPrimaryChartPoint(SolverTempVars.t, SolverTempVars.dt);

        if (int(progress / 0.01) > int(progress_p / 0.01) || rtw->DetailsOn())
            rtw->ReplotPrimaryChart();

        if (rtw->DetailsOn())
            rtw->AppendDetails("Number of iterations:"
                + QString::number(SolverTempVars.MaxNumberOfIterations())
                + ",dt = " + QString::number(SolverTempVars.dt)
                + ",t = " + QString::number(SolverTempVars.t, 'f', 6)
                + ", NR_factor = "
                + QString::fromStdString(CVector(SolverTempVars.NR_coefficient).toString()));

        if (rtw->Cancelled())
            stop_triggered = true;

        QCoreApplication::processEvents();
    }
#endif

    // Restore point — independent of GUI
    if ((counter - 1) % restore_interval == 0)
    {
        restorepoint.GetSystem()->CopyStateVariablesFrom(this);
        restorepoint.t = SolverTempVars.t;
        restorepoint.dt = SolverTempVars.dt;
        restorepoint.used_counter = 0;
        LogDetails("@ t = " + aquiutils::numbertostring(SolverTempVars.t)
            + ": Restore point saved!");
    }

    // Intermittent output writing — independent of GUI
    if (SimulationParameters.write_outputs_intermittently)
    {
        double phase_now = aquiutils::mod(
            SolverTempVars.t - SimulationParameters.tstart,
            SimulationParameters.write_interval);
        double phase_prev = aquiutils::mod(
            SolverTempVars.t - SimulationParameters.tstart - SolverTempVars.dt,
            SimulationParameters.write_interval);
        if (phase_now < phase_prev)
            WriteOutPuts();
    }

    fail_counter = 0;
    Update();
    UpdateObjectiveFunctions(SolverTempVars.t);
    UpdateObservations(SolverTempVars.t);
    PopulateOutputs();
    SolverTempVars.t += SolverTempVars.dt;

    if (SolverTempVars.MaxNumberOfIterations() > SolverSettings.NR_niteration_upper)
    {
        SolverTempVars.dt_base = max(
            SolverTempVars.dt * SolverSettings.NR_timestep_reduction_factor,
            SolverSettings.minimum_timestep);
        SolverTempVars.SetUpdateJacobian(true);
        SolverTempVars.NR_coefficient = (CVector(SolverTempVars.NR_coefficient.size()) + 1);
    }
    else if (SolverTempVars.MaxNumberOfIterations() < SolverSettings.NR_niteration_lower)
    {
        SolverTempVars.dt_base = min(
            SolverTempVars.dt_base / SolverSettings.NR_timestep_reduction_factor,
            SimulationParameters.dt0 * SolverSettings.timestepmaxfactor);
        SolverTempVars.NR_coefficient = (CVector(SolverTempVars.NR_coefficient.size()) + 1);
    }
}
bool System::CheckTerminationConditions()
{
    if ((time(nullptr) - SolverTempVars.time_start) > SolverSettings.maximum_simulation_time)
    {
        string msg = "Simulation time exceeded the limit of "
            + aquiutils::numbertostring(SolverSettings.maximum_simulation_time)
            + " seconds, The attempt to solve the problem failed!";
        LogMessage(msg, true);
        SolverTempVars.SolutionFailed = true;
        return true;
    }

    if (SolverTempVars.epoch_count > SolverSettings.maximum_number_of_matrix_inversions)
    {
        string msg = "Maximum number of matrix inversions exceeded the limit of "
            + aquiutils::numbertostring(SolverSettings.maximum_number_of_matrix_inversions)
            + ". The attempt to solve the problem failed!";
        LogMessage(msg, true);
        SolverTempVars.SolutionFailed = true;
        return true;
    }

    return false;
}

void System::FinalizeOutputs(bool uniformizeoutput)
{
    LogMessage("Adjusting outputs ...");
    Outputs.AllOutputs.unif = false;

    if (uniformizeoutput)
    {
        LogMessage("Uniformizing outputs ...");
        Outputs.AllOutputs = Outputs.AllOutputs.make_uniform(SimulationParameters.dt0, false);

        LogMessage("Uniformizing observations ...");
        Outputs.ObservedOutputs = Outputs.ObservedOutputs.make_uniform(SimulationParameters.dt0, false);

        LogMessage("Uniformizing objective functions ...");
        MakeObjectiveFunctionExpressionUniform();
    }

    LogMessage("Uniformizing observation expressions ...");
    MakeObservationsExpressionUniform();

#ifdef Q_GUI_SUPPORT
    if (rtw)
    {
        if (!stop_triggered)
            rtw->SetProgress(1);
        rtw->AddPrimaryChartPoint(SolverTempVars.t, SolverTempVars.dt);
        rtw->AppendLog("Simulation finished! "
            + QTime::currentTime().toString(Qt::RFC2822Date) + "!");
        QCoreApplication::processEvents();
    }
#endif

    if (!stop_triggered)
        if (GetSolutionLogger())
            GetSolutionLogger()->WriteString("Simulation finished successfully!");

    ShowMessage("Simulation finished!");

#ifdef Q_GUI_SUPPORT
    if (GetSolutionLogger())
    {
        cout << "Flushing solution logger ..." << std::endl;
        GetSolutionLogger()->Flush();
    }
#endif
}

bool System::SetProp(const string &s, const double &val)
{
    return SetProperty(s, aquiutils::numbertostring(val));
}

bool System::SetSystemSettingsObjectProperties(const string &s, const string &val, bool check_criteria)
{
    bool out = SetProperty(s,val);
    for (unsigned int i=0; i<Settings.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator j=Settings[i].GetVars()->begin(); j!=Settings[i].GetVars()->end(); j++)
        {   if (j->first==s)
            {
                j->second.SetProperty(val,false, check_criteria);
                return true;
            }

        }
    }
    if (!out)
        errorhandler.Append("","System","SetSystemSettingsObjectProperties","Property '" + s + "' was not found!", 631);
    return false;

}
bool System::SetProperty(const string &s, const string &val)
{

    if (s=="name") return true;
    if (s == "c_n_weight" || s == "cn_weight")
    {
        SolverSettings.C_N_weight = aquiutils::atof(val); return true;
    }
    if (s=="nr_tolerance")
    {   SolverSettings.NRtolerance = aquiutils::atof(val); return true;}
    if (s=="n_threads")
    {
        SolverSettings.n_threads = aquiutils::atoi(val); return true;
    }
    if (s=="nr_coeff_reduction_factor")
    {   SolverSettings.NR_coeff_reduction_factor = aquiutils::atof(val); return true;}
    if (s=="nr_timestep_reduction_factor")
    {   SolverSettings.NR_timestep_reduction_factor = aquiutils::atof(val); return true;}
    if (s=="nr_timestep_reduction_factor_fail")
    {   SolverSettings.NR_timestep_reduction_factor_fail = aquiutils::atof(val); return true;}
    if (s=="minimum_timestep")
    {   SolverSettings.minimum_timestep = aquiutils::atof(val); return true;}
    if (s=="nr_niteration_lower")
    {   SolverSettings.NR_niteration_lower= aquiutils::atoi(val); return true;}
    if (s=="nr_niteration_upper")
    {   SolverSettings.NR_niteration_upper= aquiutils::atoi(val); return true;}
    if (s=="nr_niteration_max")
    {   SolverSettings.NR_niteration_max= aquiutils::atoi(val); return true;}
    if (s=="make_results_uniform")
    {   SolverSettings.makeresultsuniform = aquiutils::atoi(val); return true;}

    if (s=="tstart" || s== "simulation_start_time")
    {   SimulationParameters.tstart = aquiutils::atof(val); return true;}
    if (s=="tend" || s=="simulation_end_time")
    {   SimulationParameters.tend = aquiutils::atof(val); return true;}
    if (s=="dt" || s=="initial_time_step")
    {
        SimulationParameters.dt0 = aquiutils::atof(val); return true;
    }
    if (s=="maximum_time_allowed")
    {
        SolverSettings.maximum_simulation_time = aquiutils::atof(val); return true;
    }
    if (s=="maximum_number_of_matrix_inverstions")
    {
        SolverSettings.maximum_number_of_matrix_inversions = aquiutils::atof(val); return true;
    }
    if (s=="silent")
    {
        SetSilent(aquiutils::atoi(val)); return true;
    }
    if (s=="inputpath")
    {
        paths.inputpath = val; return true;
    }
    if (s=="jacobian_method")
    {
        if (val=="Inverse Jacobian")
            SolverSettings.direct_jacobian=false;
        if (val=="Direct")
            SolverSettings.direct_jacobian=true;
        return true;
    }
    if (s=="alloutputfile")
    {
        paths.outputfilename = val; return true;
    }
    if (s=="observed_outputfile")
    {
        paths.observedoutputfilename = val; return true;
    }
    if (s=="outputpath")
    {
        paths.outputpath = val; return true;
    }
    if (s=="write_solution_details")
    {
        if (aquiutils::trim(aquiutils::tolower(val))=="yes")
            SolverSettings.write_solution_details = true;
        else
            SolverSettings.write_solution_details = false;
        return true;
    }
    if (s=="write_intermittently")
    {
        if(aquiutils::trim(aquiutils::tolower(val))=="yes")
            SimulationParameters.write_outputs_intermittently = true;
        else
            SimulationParameters.write_outputs_intermittently = false;
        return true;
    }
    if (s=="write_interval")
    {

        SimulationParameters.write_interval = aquiutils::atof(val);
        return true;
    }
    if (s=="max_timestep_increase_factor" )
    {
        SolverSettings.timestepmaxfactor = aquiutils::atof(val);
        return true;
    }
    if (s=="max_timestep_decrease_factor" )
    {
        SolverSettings.timestepminfactor = aquiutils::atof(val);
        return true;
    }

    //errorhandler.Append("","System","SetProperty","Property '" + s + "' was not found!", 622);

    return false;
}


void System::InitiateOutputs()
{
    Outputs.AllOutputs.clear();
    Outputs.ObservedOutputs.clear();
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].EstablishExpressionStructure();
        for (unordered_map<string, Quan>::iterator it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), blocks[i].GetName() + "_" + it->first);
                it->second.SetOutputItem(blocks[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        links[i].EstablishExpressionStructure();
        for (unordered_map<string, Quan>::iterator it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), links[i].GetName() + "_" + it->first);
                it->second.SetOutputItem(links[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
    {
        reaction_parameters[i].EstablishExpressionStructure();
        for (unordered_map<string, Quan>::iterator it = reaction_parameters[i].GetVars()->begin(); it != reaction_parameters[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), reaction_parameters[i].GetName() + "_" + it->first);
                it->second.SetOutputItem(reaction_parameters[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i=0; i<objective_function_set.size(); i++)
    {
        objective_function_set[i]->EstablishExpressionStructure();
        Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), "Obj_" + objective_function_set[i]->GetName());
        objective_function_set[i]->SetOutputItem("Obj_" + objective_function_set[i]->GetName());
        for (unordered_map<string, Quan>::iterator it = objective_function_set[i]->GetVars()->begin(); it != objective_function_set[i]->GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), "Obj_" + objective_function_set[i]->GetName()+"_"+it->first);
                it->second.SetOutputItem("Obj_" + objective_function_set[i]->GetName()+"_"+it->first);
                //qDebug()<<QString::fromStdString(it->second.GetOutputItem());
            }
    }

    for (unsigned int i=0; i<observations.size(); i++)
    {
        observations[i].EstablishExpressionStructure();
        Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), "Obs_" + observations[i].GetName());
        observations[i].SetOutputItem("Obs_" + observations[i].GetName());
        for (unordered_map<string, Quan>::iterator it = observations[i].GetVars()->begin(); it != observations[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), "Obs_" + observations[i].GetName()+"_"+it->first);
                it->second.SetOutputItem("Obs_" + observations[i].GetName()+"_"+it->first);
                //qDebug()<<QString::fromStdString(it->second.GetOutputItem());
            }
         Outputs.ObservedOutputs.append(TimeSeries<timeseriesprecision>(), observations[i].GetName());
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
        sources[i].EstablishExpressionStructure();
        for (unordered_map<string, Quan>::iterator it = sources[i].GetVars()->begin(); it != sources[i].GetVars()->end(); it++)
        {
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<outputtimeseriesprecision>(), sources[i].GetName() + "_" + it->first);
                it->second.SetOutputItem(sources[i].GetName() + "_" + it->first);
            }
        }
    }

}

double & System::GetSimulationTime()
{
    return SolverTempVars.t;
}

bool System::SetLoadedOutputItems()
{
    int varcount = 0;
    vector<string> file_series = GetOutputs().getSeriesNames();

    for (unsigned int i = 0; i < blocks.size(); i++)
        for (auto it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string seriesname = blocks[i].GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, seriesname) != -1)
                    it->second.SetOutputItem(seriesname);
                else
                    qDebug() << "Series not found in file (block): " << QString::fromStdString(seriesname);
                varcount++;
            }

    qDebug() << "Blocks variables: " << varcount;

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        for (auto it = reaction_parameters[i].GetVars()->begin(); it != reaction_parameters[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string seriesname = reaction_parameters[i].GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, seriesname) != -1)
                    it->second.SetOutputItem(seriesname);
                else
                    qDebug() << "Series not found in file (reaction param): " << QString::fromStdString(seriesname);
                varcount++;
            }

    qDebug() << "Reaction parameters variables: " << varcount;

    for (unsigned int i = 0; i < links.size(); i++)
        for (auto it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string seriesname = links[i].GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, seriesname) != -1)
                    it->second.SetOutputItem(seriesname);
                else
                    qDebug() << "Series not found in file (link): " << QString::fromStdString(seriesname);
                varcount++;
            }

    qDebug() << "Link variables: " << varcount;

    for (unsigned int i = 0; i < sources.size(); i++)
        for (auto it = sources[i].GetVars()->begin(); it != sources[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string seriesname = sources[i].GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, seriesname) != -1)
                    it->second.SetOutputItem(seriesname);
                else
                    qDebug() << "Series not found in file (source): " << QString::fromStdString(seriesname);
                varcount++;
            }

    qDebug() << "Source variables: " << varcount;

    for (unsigned int i = 0; i < objective_function_set.size(); i++)
    {
        string seriesname = "Obj_" + objective_function_set[i]->GetName();
        if (aquiutils::lookup(file_series, seriesname) != -1)
            objective_function_set[i]->SetOutputItem(seriesname);
        else
            qDebug() << "Series not found in file (obj func): " << QString::fromStdString(seriesname);
        varcount++;

        for (auto it = objective_function_set[i]->GetVars()->begin(); it != objective_function_set[i]->GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string varseriesname = "Obj_" + objective_function_set[i]->GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, varseriesname) != -1)
                    it->second.SetOutputItem(varseriesname);
                else
                    qDebug() << "Series not found in file (obj func var): " << QString::fromStdString(varseriesname);
                varcount++;
            }
    }

    qDebug() << "Objective function variables: " << varcount;

    for (unsigned int i = 0; i < observations.size(); i++)
    {
        string seriesname = "Obs_" + observations[i].GetName();
        if (aquiutils::lookup(file_series, seriesname) != -1)
            observations[i].SetOutputItem(seriesname);
        else
            qDebug() << "Series not found in file (observation): " << QString::fromStdString(seriesname);
        varcount++;

        for (auto it = observations[i].GetVars()->begin(); it != observations[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                string varseriesname = "Obs_" + observations[i].GetName() + "_" + it->first;
                if (aquiutils::lookup(file_series, varseriesname) != -1)
                    it->second.SetOutputItem(varseriesname);
                else
                    qDebug() << "Series not found in file (obs var): " << QString::fromStdString(varseriesname);
                varcount++;
            }
    }

    qDebug() << "All variables: " << varcount;
    qDebug() << "Output size: " << GetOutputs().size();

    return true;
}

void System::SetOutputItems()
{

    for (unsigned int i=0; i<blocks.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                  it->second.SetOutputItem(blocks[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator it = reaction_parameters[i].GetVars()->begin(); it != reaction_parameters[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                  it->second.SetOutputItem(reaction_parameters[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                it->second.SetOutputItem(links[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator it = sources[i].GetVars()->begin(); it != sources[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                  it->second.SetOutputItem(sources[i].GetName() + "_" + it->first);
            }
    }

    for (unsigned int i=0; i<objective_function_set.size(); i++)
    {
        objective_function_set[i]->SetOutputItem("Obj_" + objective_function_set[i]->GetName());
        for (unordered_map<string, Quan>::iterator it = objective_function_set[i]->GetVars()->begin(); it != objective_function_set[i]->GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<timeseriesprecision>(), "Obj_" + objective_function_set[i]->GetName()+"_"+it->first);
                it->second.SetOutputItem("Obj_" + objective_function_set[i]->GetName()+"_"+it->first);

            }
    }

    for (unsigned int i=0; i<observations.size(); i++)
    {
        observations[i].SetOutputItem("Obs_" + observations[i].GetName());
        for (unordered_map<string, Quan>::iterator it = observations[i].GetVars()->begin(); it != observations[i].GetVars()->end(); it++)
            if (it->second.IncludeInOutput())
            {
                Outputs.AllOutputs.append(TimeSeries<timeseriesprecision>(), "Obs_" + observations[i].GetName()+"_"+it->first);
                it->second.SetOutputItem("Obs_" + observations[i].GetName()+"_"+it->first);

            }
    }

}

bool System::TransferResultsFrom(System *other)
{
    Outputs = other->Outputs;
    return true;
}

void System::PopulateOutputs(bool dolinks)
{

    CalcAllExpressions(Expression::timing::present, false, dolinks);
    if (RecordResults())
    {
        for (unsigned int i = 0; i < blocks.size(); i++)
        {
            for (unordered_map<string, Quan>::iterator it = blocks[i].GetVars()->begin(); it != blocks[i].GetVars()->end(); it++)
                if (it->second.IncludeInOutput())
                    Outputs.AllOutputs[blocks[i].GetName() + "_" + it->first].append(SolverTempVars.t, blocks[i].GetVal(it->first, Expression::timing::present));

        }

        if (dolinks)
            for (unsigned int i = 0; i < links.size(); i++)
            {
                for (unordered_map<string, Quan>::iterator it = links[i].GetVars()->begin(); it != links[i].GetVars()->end(); it++)
                    if (it->second.IncludeInOutput())
                    {
                        Outputs.AllOutputs[links[i].GetName() + "_" + it->first].append(SolverTempVars.t, links[i].GetVal(it->first, Expression::timing::present, true));
                    }
            }

        for (unsigned int i = 0; i < sources.size(); i++)
        {
            for (unordered_map<string, Quan>::iterator it = sources[i].GetVars()->begin(); it != sources[i].GetVars()->end(); it++)
                if (it->second.IncludeInOutput())
                {
                    //sources[i].CalcExpressions(Expression::timing::present);
                    Outputs.AllOutputs[sources[i].GetName() + "_" + it->first].append(SolverTempVars.t, sources[i].GetVal(it->first, Expression::timing::present, true));
                }
        }


        for (unsigned int i = 0; i < objective_function_set.size(); i++)
        {
            Outputs.AllOutputs["Obj_" + objective_function_set[i]->GetName()].append(SolverTempVars.t, objectivefunction(objective_function_set[i]->GetName())->Value());
            for (unordered_map<string, Quan>::iterator it = objective_function_set[i]->GetVars()->begin(); it != objective_function_set[i]->GetVars()->end(); it++)
                if (it->second.IncludeInOutput())
                {
                    //sources[i].CalcExpressions(Expression::timing::present);
                    Outputs.AllOutputs["Obj_" + objective_function_set[i]->GetName() + "_" + it->first].append(SolverTempVars.t, objective_function_set[i]->Variable(it->first)->CalcVal(object(objective_function_set[i]->GetLocation()), Expression::timing::present));
                }
        }

        for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        {
            for (unordered_map<string, Quan>::iterator it = reaction_parameters[i].GetVars()->begin(); it != reaction_parameters[i].GetVars()->end(); it++)
                if (it->second.IncludeInOutput())
                {
                    //sources[i].CalcExpressions(Expression::timing::present);
                    Outputs.AllOutputs[reaction_parameters[i].GetName() + "_" + it->first].append(SolverTempVars.t, reaction_parameters[i].GetVal(it->first, Expression::timing::present, true));
                }
        }


        for (unsigned int i = 0; i < observations.size(); i++)
        {
            Outputs.AllOutputs["Obs_" + observations[i].GetName()].append(SolverTempVars.t, observation(observations[i].GetName())->Value());
            Outputs.ObservedOutputs[observations[i].GetName()].append(SolverTempVars.t, observation(observations[i].GetName())->Value());
            for (unordered_map<string, Quan>::iterator it = observations[i].GetVars()->begin(); it != observations[i].GetVars()->end(); it++)
                if (it->second.IncludeInOutput())
                {
                    //sources[i].CalcExpressions(Expression::timing::present);
                    Object* location;
                    if (it->second.GetType() == Quan::_type::expression)
                    {   location = object(observations[i].GetLocation());
                        if (location)
                            if (location->ObjectType() != object_type::link || dolinks)
                                Outputs.AllOutputs["Obs_" + observations[i].GetName() + "_" + it->first].append(SolverTempVars.t, observations[i].Variable(it->first)->CalcVal(location, Expression::timing::present)*location->GetOutflowLimitFactor(Expression::timing::present));
                    }
                    else
                    {   location = observation(i);
                        if (location)
                            Outputs.AllOutputs["Obs_" + observations[i].GetName() + "_" + it->first].append(SolverTempVars.t, observations[i].Variable(it->first)->CalcVal(location, Expression::timing::present));
                    }
                }
        }
    }
    else
    {

    }
}


vector<bool> System::GetOutflowLimitedVector()
{
    vector<bool> out;
    for (unsigned int i = 0; i<blocks.size(); i++)
        out.push_back(blocks[i].GetLimitedOutflow());

    return out;
}

vector<double> System::GetOutflowLimitFactorVector(const Expression::timing &tmg)
{
    vector<double> out;
    for (unsigned int i = 0; i<blocks.size(); i++)
        out.push_back(blocks[i].GetOutflowLimitFactor(tmg));

    return out;
}


void System::SetOutflowLimitedVector(const vector<bool> &x)
{
    for (unsigned int i = 0; i < blocks.size(); i++)
        blocks[i].SetLimitedOutflow(x[i]);

}

bool System::OneStepSolve(unsigned int statevarno, bool transport)
{
    string variable;
    if (!transport)
        variable = solvevariableorder[statevarno];
    else
        variable = "mass";

    vector<bool> outflowlimitstatus_old;
    InitializeOneStep(variable, statevarno, transport, outflowlimitstatus_old);

    bool switchvartonegpos = true;
    unsigned int attempts = 0;

    while (attempts<BlockCount() && switchvartonegpos)
    {
        CVector_arma X = GetStateVariables(variable, Expression::timing::past,transport);
        double X_norm = X.norm2();
        double dx_norm = X_norm*10+1;
        if (!transport)
        {   for (unsigned int i = 0; i < blocks.size(); i++)
            {
                if (blocks[i].GetLimitedOutflow())
                    X[i] = blocks[i].GetOutflowLimitFactor(Expression::timing::present);
            }
        }

		CVector_arma X_past = X;
        //qDebug()<<"Getting residuals";
        CVector_arma F = GetResiduals(variable, X, transport);
        //qDebug()<<"Getting residuals done";
#ifdef DEBUG
        CVector_arma F_ini = F;
#endif // DEBUG
        int ini_max_error_block = F.abs_max_elems();
        if (!F.is_finite())
        {
            SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) + ": F is infinite");
            if (GetSolutionLogger())
            {   GetSolutionLogger()->WriteString("at " + aquiutils::numbertostring(SolverTempVars.t) + ": F is infinite, dt = " + aquiutils::numbertostring(SolverTempVars.dt));
                GetSolutionLogger()->WriteString("at " + aquiutils::numbertostring(SolverTempVars.t) + "X=" + X.toString());
                GetSolutionLogger()->WriteString("at " + aquiutils::numbertostring(SolverTempVars.t) + "F=" + F.toString());
                GetSolutionLogger()->Flush();
            }
            if (!transport)
                SetOutflowLimitedVector(outflowlimitstatus_old);
            return false;
        }

        double error_increase_counter = 0;
		double err_ini = F.norm2();
        double err;
        double err_p = err = err_ini;

		//if (SolverTempVars.NR_coefficient[statevarno]==0)
            SolverTempVars.NR_coefficient[statevarno] = 1;
        while ((err/(err_ini+1e-8*X_norm)>SolverSettings.NRtolerance && err>1e-12 && dx_norm/X_norm>1e-10))
        {
            SolverTempVars.numiterations[statevarno]++;

            if (SolverTempVars.updatejacobian[statevarno])
            {
                CMatrix_arma J;
                if (transport)
                    J = Jacobian(variable, X, transport);

                else
                    J = JacobianDirect(variable, X, transport);


                SolverTempVars.epoch_count ++;
                if (SolverSettings.scalediagonal)
                    J.ScaleDiagonal(1.0 / SolverTempVars.NR_coefficient[statevarno]);


                if (!SolverSettings.direct_jacobian)
                {   if (!Invert(J, SolverTempVars.Inverse_Jacobian[statevarno]))
                    {
                        LogJacobianFailure(J, transport);
                        SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) +
                                                             ": The Jacobian Matrix is not full-ranked");
                        if (!transport)
                            SetOutflowLimitedVector(outflowlimitstatus_old);
                        return false;
                    }
                }
                else
                    SolverTempVars.Inverse_Jacobian[statevarno] = J;
                SolverTempVars.updatejacobian[statevarno] = false;

            }
            CVector_arma X1;
            CVector_arma dx;
            if (!ComputeNewtonStep(variable, X, X1, dx, F, statevarno, transport, outflowlimitstatus_old))
                return false;

            dx_norm = dx.norm2();

            if (!X.is_finite())
			{
                SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) + ": X is infinite, X=" + X.toString() + ", F = " + F.toString());
                SetOutflowLimitedVector(outflowlimitstatus_old);
                return false;
			}

            CVector_arma F1;
            if (SolverSettings.optimize_lambda)
                F1 = GetResiduals(variable, X1, transport);

            F = GetResiduals(variable, X, transport);

			if (!F.is_finite())
			{
				SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) + ": F is infinite");
                if (GetSolutionLogger())
                {
                    GetSolutionLogger()->WriteString("at " + aquiutils::numbertostring(SolverTempVars.t) + ": F is infinite");
                    GetSolutionLogger()->Flush();
                }
                SetOutflowLimitedVector(outflowlimitstatus_old);
                return false;
			}
            err_p = err;
            err = F.norm2();

            if (AdjustNRCoefficient(X, X_past, X1, F, F1, err, err_p,
                                    statevarno, transport, ini_max_error_block,
                                    error_increase_counter, outflowlimitstatus_old) == NRAdjustResult::failed)
                return false;
        }
        switchvartonegpos = false;

        if (!transport)
        {
            for (unsigned int i=0; i<blocks.size(); i++)
            {
                if (X[i]<-1e-13 && !blocks[i].GetLimitedOutflow() && OutFlowCanOccur(i,variable))
                {
                    SafeVector<int> blocks_affected = SetLimitedOutFlow(i, variable, true);
                    switchvartonegpos = true;
                    SolverTempVars.updatejacobian[statevarno] = true;
                    if (attempts==BlockCount())
                    {
                        SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) + ": Storage is negative in block '" + blocks[i].GetName() + "' after " + aquiutils::numbertostring(int(BlockCount())) +" attempts");
                        if (GetSolutionLogger())
                        {   GetSolutionLogger()->WriteString("at " + aquiutils::numbertostring(SolverTempVars.t) + ": Storage is negative in block '" + blocks[i].GetName() + "' after " + aquiutils::numbertostring(int(BlockCount())) +" attempts , dt = "  + aquiutils::numbertostring(dt()));
                            GetSolutionLogger()->Flush();
                        }
                        if (!transport)
                            SetOutflowLimitedVector(outflowlimitstatus_old);
                        return false;
                    }
                    for (unsigned int j=0; j<blocks_affected.size(); j++)
                        X[blocks_affected[j]] = 0.9999;
                }
                else if (X[i]>=1 && blocks[i].GetLimitedOutflow())
                {
                    SetLimitedOutFlow(i,variable, false);
                    blocks[i].SetAllowLimitedFlow(false);
                    switchvartonegpos = true;
                    SolverTempVars.updatejacobian[statevarno] = true;
                }
                else if (X[i]<0)
                {
                    blocks[i].SetOutflowLimitFactor(0,Expression::timing::present);
                }
            }

        }
        if (switchvartonegpos) attempts++;
    }

	return true;
}

SafeVector<int> System::SetLimitedOutFlow(int blockid, const string &variable, bool outflowlimited)
{
    SafeVector<int> blocks_affected;
    blocks_affected.push_back(blockid);
    blocks[blockid].SetLimitedOutflow(outflowlimited);
    blocks[blockid].SetOutflowLimitFactor(1,Expression::timing::present);
    SafeVector<int> connected_blocks = ConnectedBlocksFrom(blockid);
    SafeVector<Link*> connected_links = blocks[blockid].GetLinksFrom();
    for (unsigned int i=0; i<connected_blocks.size(); i++)
    {
        if (blocks[connected_blocks[i]].Variable(variable)->isrigid() && aquiutils::ispositive(blocks[blockid].GetLinksFrom()[i]->GetVal(blocks[blockid].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present)))
        {
            if (blocks[i].AllowLimitedFlow())
                blocks_affected.append(SetLimitedOutFlow(connected_blocks[i],variable,outflowlimited));
        }
    }
    connected_blocks = ConnectedBlocksTo(blockid);
    connected_links = blocks[blockid].GetLinksTo();
    for (unsigned int i=0; i<connected_blocks.size(); i++)
    {
        if (blocks[connected_blocks[i]].Variable(variable)->isrigid() && aquiutils::isnegative(blocks[blockid].GetLinksTo()[i]->GetVal(blocks[blockid].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present)))
        {
            blocks_affected.append(SetLimitedOutFlow(connected_blocks[i],variable,outflowlimited));
        }
    }
    return blocks_affected;
}

SafeVector<int> System::ConnectedBlocksFrom(int blockid)
{
    SafeVector<int> out;
    for(unsigned int i=0; i<blocks[blockid].GetLinksFrom().size(); i++)
    {
        out.push_back(blocks[blockid].GetLinksFrom()[i]->e_Block_No());
    }
    return out;
}

SafeVector<int> System::ConnectedBlocksTo(int blockid)
{
    SafeVector<int> out;
    for(unsigned int i=0; i<blocks[blockid].GetLinksTo().size(); i++)
    {
        out.push_back(blocks[blockid].GetLinksTo()[i]->s_Block_No());
    }
    return out;
}

bool System::Renew(const string & variable)
{
    bool out = true;
    if (variable!="mass")
    {   for (unsigned int i = 0; i < blocks.size(); i++)
            out &= blocks[i].Renew(variable);

        for (unsigned int i = 0; i < links.size(); i++)
            out &= links[i].Renew(Variable(variable)->GetCorrespondingFlowVar());

        return out;
    }
    else
    {
        for (unsigned int j=0; j<ConstituentsCount(); j++)
        {
            for (unsigned int i = 0; i < blocks.size(); i++)
                out &= blocks[i].Renew(constituent(j)->GetName() + ":" + variable);

            for (unsigned int i = 0; i < links.size(); i++)
                out &= links[i].Renew(constituent(j)->GetName() + ":" + Variable(variable)->GetCorrespondingFlowVar());
        }
        return out;
    }
}

bool System::Update(const string & variable)
{
	bool out = true;
    if (variable == "")
    {
        for (unsigned int i = 0; i < blocks.size(); i++)
        {
            for (unsigned j=0; j<blocks[i].GetVars()->Quantity_Order().size(); j++)
                out &= blocks[i].Update(blocks[i].GetVars()->Quantity_Order()[j]);
            blocks[i].SetOutflowLimitFactor(blocks[i].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::past);
        }
        for (unsigned int i = 0; i < links.size(); i++)
            for (unsigned j = 0; j < links[i].GetVars()->Quantity_Order().size(); j++)
                out &= links[i].Update(links[i].GetVars()->Quantity_Order()[j]);
    }
    else
    {
        for (unsigned int i = 0; i < blocks.size(); i++)
        {
            out &= blocks[i].Update(variable);
            blocks[i].SetOutflowLimitFactor(blocks[i].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::past);
        }
    }

//	for (unsigned int i = 0; i < links.size(); i++)
//		out &= links[i].Update(variable);

	return out;
}


CVector_arma System::CalcStateVariables(const string &variable, const Expression::timing &tmg)
{
    CVector_arma X(blocks.size());
    for (unsigned int i=0; i<blocks.size(); i++)
        X[i] = blocks[i].CalcVal(variable,tmg);
    return X;
}

CVector_arma System::GetStateVariables_for_direct_Jacobian(const string &variable, const Expression::timing &tmg, bool transport)
{
    CVector_arma X = GetStateVariables(variable, Expression::timing::past,transport);

    if (!transport)
    {   for (unsigned int i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].GetLimitedOutflow())
                X[i] = blocks[i].GetOutflowLimitFactor(Expression::timing::past);
        }
    }

    return X;
}

CVector_arma System::GetStateVariables(const string &variable, const Expression::timing &tmg, bool transport)
{
    if (!transport)
    {
        CVector_arma X(blocks.size());
        for (unsigned int i=0; i<blocks.size(); i++)
            X[i] = blocks[i].GetVal(variable,tmg);
        return X;
    }
    else
    {
        CVector_arma X(blocks.size()*ConstituentsCount());
        for (unsigned int i=0; i<blocks.size(); i++)
            for (unsigned int j=0; j<ConstituentsCount(); j++)
                X[j+ConstituentsCount()*i] = blocks[i].GetVal(variable,constituent(j)->GetName(),tmg);

        return X;
    }
}

pair<int, int> System::GetBlockConstituentValue(unsigned int i) const
{
    pair<int, int>  out; 
    out.first = i / this->constituents.size();
    out.second = i % this->constituents.size();
    return out; 
}

std::string  System::GetBlockConstituentSring(unsigned int i) const // return block and constituent name for a state variable number i
{
    string out;
    out += block(i / this->constituents.size())->GetName();
    if (i % this->constituents.size() < constituents.size() > 0)
        out += ":" + constituent(i % this->constituents.size())->GetName();
    return out;
}

string System::GetBlockConstituent(unsigned int i) const
{
    int BlockNo = i / this->constituents.size();
    int ConstituentNo = i % this->constituents.size();
    string out = blocks[BlockNo].GetName() + ":" + constituents[ConstituentNo].GetName();
    return out;
}

void System::SetStateVariables(const string &variable, CVector_arma &X, const Expression::timing &tmg, bool transport)
{
    if (!transport)
    {   for (unsigned int i=0; i<blocks.size(); i++)
        {
            if (!blocks[i].GetLimitedOutflow())
            {
                blocks[i].SetVal(variable, X[i], tmg);
                blocks[i].SetOutflowLimitFactor(1, tmg);
            }
            else
            {
                blocks[i].SetOutflowLimitFactor(X[i],tmg);
                if (!blocks[i].isrigid(variable))
                    blocks[i].SetVal(variable, 0, tmg);
                else
                    blocks[i].SetVal(variable, blocks[i].GetVal(variable,Expression::timing::past), tmg);
            }
        }
    }
    else
    {
        for (unsigned int i=0; i<blocks.size(); i++)
        {
            vector<Quan*> masses = blocks[i].GetAllConstituentProperties(variable);
            for (unsigned int j=0; j<masses.size(); j++)
                masses[j]->SetVal(X[j+masses.size()*i],tmg);
        }
    }
}

void System::SetStateVariables_for_direct_Jacobian(const string &variable, CVector_arma &X, const Expression::timing &tmg, bool transport)
{
    if (!transport)
    {   for (unsigned int i=0; i<blocks.size(); i++)
        {
            if (!blocks[i].GetLimitedOutflow())
            {
                blocks[i].SetVal(variable, X[i], tmg);
                blocks[i].SetOutflowLimitFactor(1, tmg);
            }
            else
            {
                blocks[i].SetOutflowLimitFactor(X[i],tmg);
                if (!blocks[i].isrigid(variable))
                    blocks[i].SetVal(variable, 0, tmg);
                else
                    blocks[i].SetVal(variable, blocks[i].GetVal(variable,Expression::timing::past), tmg);
            }
        }
    }
    else
    {
        for (unsigned int i=0; i<blocks.size(); i++)
        {
            vector<Quan*> masses = blocks[i].GetAllConstituentProperties(variable);
            for (unsigned int j=0; j<masses.size(); j++)
                masses[j]->SetVal(X[j+masses.size()*i],tmg);
        }
    }
    UnUpdateAllVariables();

    for (unsigned int i=0; i<blocks.size(); i++)
    {
        if (blocks[i].GetLimitedOutflow())
        {
            blocks[i].SetOutflowLimitFactor(X[i],Expression::timing::present);
            blocks[i].SetVal(variable, blocks[i].GetVal(variable, Expression::timing::past) * SolverSettings.landtozero_factor,Expression::timing::present);
        }
    }
}

void System::SetStateVariables_TR(const string &variable, CVector_arma &X, const Expression::timing &tmg)
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        vector<Quan*> masses = blocks[i].GetAllConstituentProperties(variable);
        for (unsigned int j=0; j<masses.size(); j++)
            masses[j]->SetVal(X[j+masses.size()*i],tmg);
    }
}

void System::CalculateAllExpressions(Expression::timing tmg)
{
    for (int i=0; i<blocks.size(); i++)
    {
        for (unsigned int j = 0; j < blocks[i].QuantitOrder().size(); j++)
        {
            if (blocks[i].Variable(blocks[i].QuantitOrder()[j])->GetType() == Quan::_type::expression)
            {
                //blocks[i].Variable(blocks[i].QuantitOrder()[j])->GetExpression()->ClearTermSources();
                blocks[i].Variable(blocks[i].QuantitOrder()[j])->SetVal(blocks[i].Variable(blocks[i].QuantitOrder()[j])->CalcVal(tmg), tmg);
            }
        }
    }
    for (int i=0; i<links.size(); i++)
    {
        for (unsigned int j = 0; j < links[i].QuantitOrder().size(); j++)
        {
            if (links[i].Variable(links[i].QuantitOrder()[j])->GetType() == Quan::_type::expression)
            {   //links[i].Variable(links[i].QuantitOrder()[j])->GetExpression()->ClearTermSources();
                links[i].Variable(links[i].QuantitOrder()[j])->SetVal(links[i].Variable(links[i].QuantitOrder()[j])->CalcVal(tmg), tmg);

            }
        }
    }

    for (int i = 0; i < reaction_parameters.size(); i++)
    {
        for (unsigned int j = 0; j < reaction_parameters[i].QuantitOrder().size(); j++)
        {
            if (reaction_parameters[i].Variable(reaction_parameters[i].QuantitOrder()[j])->GetType() == Quan::_type::expression)
            {   //links[i].Variable(links[i].QuantitOrder()[j])->GetExpression()->ClearTermSources();
                reaction_parameters[i].Variable(reaction_parameters[i].QuantitOrder()[j])->SetVal(reaction_parameters[i].Variable(reaction_parameters[i].QuantitOrder()[j])->CalcVal(tmg), tmg);

            }
        }
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
        for (unsigned int j = 0; j < sources[i].QuantitOrder().size(); j++)
        {
            if (sources[i].Variable(sources[i].QuantitOrder()[j])->GetType() == Quan::_type::expression)
            {
                if (sources[i].Variable(sources[i].QuantitOrder()[j])->GetName()!="coefficient")
                    sources[i].Variable(sources[i].QuantitOrder()[j])->SetVal(sources[i].Variable(sources[i].QuantitOrder()[j])->CalcVal(tmg), tmg);
            }
        }
    }
}

CVector_arma System::GetResiduals(const string &variable, CVector_arma &X, bool transport)
{
    if (transport)
        return GetResiduals_TR(variable, X);

    CVector_arma F(blocks.size());
    SetStateVariables(variable,X,Expression::timing::present);
    UnUpdateAllVariables();
    //CalculateFlows(Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present);
    CVector LinkFlow(links.size());



//#ifndef NO_OPENMP
//#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
//#endif
    for (int i=0; i<blocks.size(); i++)
    {
        //qDebug()<<QString::fromStdString(blocks[i].GetName());
        if (blocks[i].isrigid(variable))
        {
            F[i] = - blocks[i].GetInflowValue(variable, Expression::timing::present);
        }
        else if (blocks[i].GetLimitedOutflow())
        {
            blocks[i].SetOutflowLimitFactor(X[i],Expression::timing::present); //max(X[i],0.0)
            blocks[i].SetVal(variable, blocks[i].GetVal(variable, Expression::timing::past) * SolverSettings.landtozero_factor,Expression::timing::present);
            double inflow = blocks[i].GetInflowValue(variable, Expression::timing::present);
            if (inflow<0) inflow*=blocks[i].GetOutflowLimitFactor(Expression::timing::present);
            F[i] = -blocks[i].GetVal(variable,Expression::timing::past)*(1.0-SolverSettings.landtozero_factor)/dt() - inflow;
        }
        else
            F[i] = (X[i]-blocks[i].GetVal(variable,Expression::timing::past))/dt() - blocks[i].GetInflowValue(variable,Expression::timing::present);
    }


    for (unsigned int i=0; i<links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) > 0)
            links[i].SetOutflowLimitFactor(blocks[links[i].s_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
        else if (blocks[links[i].e_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)<0)
            links[i].SetOutflowLimitFactor(blocks[links[i].e_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
        else
            links[i].SetOutflowLimitFactor(1, Expression::timing::present);

    }

{

#ifndef NO_OPENMP
//#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
#endif
    for (int i=0; i<links.size(); i++)
       LinkFlow[i] = links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)*links[i].GetOutflowLimitFactor(Expression::timing::present);



    for (unsigned int i=0; i<links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].isrigid(variable))
            F[links[i].s_Block_No()] += LinkFlow[i];
        else
            F[links[i].s_Block_No()] += LinkFlow[i];
        if (blocks[links[i].e_Block_No()].isrigid(variable))
            F[links[i].e_Block_No()] -= LinkFlow[i];
        else
            F[links[i].e_Block_No()] -= LinkFlow[i];
    }
}

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
        if (blocks[i].GetLimitedOutflow())
        {
            if (!OutFlowCanOccur(i,variable))
            {
                F[i] = X[i] - 1.1;
            }
            if (X[i]<0)
            {
                //qDebug()<<"flow factor is negative!";
                //F[i] = X[i];
            }

        }

    }

    return F;
}


bool System::OutFlowCanOccur(int blockno, const string &variable)
{
    bool alloutflowszero = true;
    double outflow;
    for (unsigned int i=0; i<blocks[blockno].Variable(variable)->GetCorrespondingInflowVar().size(); i++)
    {
        if (blocks[blockno].Variable(variable)->GetCorrespondingInflowVar()[i] != "")
        {
            if (blocks[blockno].Variable(blocks[blockno].Variable(variable)->GetCorrespondingInflowVar()[i]))
            {
                outflow = blocks[blockno].CalcVal(blocks[blockno].Variable(variable)->GetCorrespondingInflowVar()[i]);
                alloutflowszero &= !aquiutils::isnegative(outflow);
            }
        }
    }

    for (unsigned int j = 0; j < blocks[blockno].GetLinksFrom().size(); j++)
    {
        outflow = blocks[blockno].GetLinksFrom()[j]->GetVal(blocks[blockno].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present);
        alloutflowszero &= !aquiutils::ispositive(outflow);
    }
    for (unsigned int j = 0; j < blocks[blockno].GetLinksTo().size(); j++)
    {
        outflow = blocks[blockno].GetLinksTo()[j]->GetVal(blocks[blockno].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present);
        alloutflowszero &= !aquiutils::isnegative(outflow);
    }
    return !alloutflowszero;
}

CVector System::GetBlocksOutflowFactors(const Expression::timing &tmg)
{
    CVector out(blocks.size());
    for (unsigned int i=0; i<blocks.size(); i++)
        out[i] = blocks[i].GetOutflowLimitFactor(tmg);

    return out;
}

CVector System::GetLinkssOutflowFactors(const Expression::timing &tmg)
{
    CVector out(links.size());
    for (unsigned int i=0; i<links.size(); i++)
        out[i] = links[i].GetOutflowLimitFactor(tmg);

    return out;
}


CVector_arma System::GetResiduals_TR(const string &variable, CVector_arma &X)
{
    CVector_arma F(blocks.size()*ConstituentsCount());
    SetStateVariables_TR(variable,X,Expression::timing::present);
    UnUpdateAllVariables();
    //CalculateFlows(Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present);


   for (unsigned int i=0; i<blocks.size(); i++)
    {
       CVector V = blocks[i].GetAllReactionRates(Expression::timing::present);
       for (unsigned int j=0; j<ConstituentsCount(); j++)
            F[j+i*ConstituentsCount()] = (X[j+i*ConstituentsCount()]-blocks[i].GetVal(variable, constituent(j)->GetName(), Expression::timing::past))/dt() - blocks[i].GetInflowValue(variable,constituent(j)->GetName(), Expression::timing::present) - V[j];
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        for (unsigned int j=0; j<ConstituentsCount(); j++)
        {
            F[j+ConstituentsCount()*links[i].s_Block_No()] += links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable,constituent(j)->GetName())->GetCorrespondingFlowVar(),constituent(j)->GetName(),Expression::timing::present, true);
            F[j+ConstituentsCount()*links[i].e_Block_No()] -= links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable,constituent(j)->GetName())->GetCorrespondingFlowVar(),constituent(j)->GetName(),Expression::timing::present, true);
        }

    }


    return F;
}

void System::CorrectStoragesBasedonFluxes(const string& variable)
{
    CVector_arma UpdatedStorage(blocks.size());
    for (unsigned int i = 0; i < blocks.size(); i++)
        UpdatedStorage[i] = (blocks[i].GetVal(variable, Expression::timing::past)) + blocks[i].GetInflowValue(variable, Expression::timing::present)*dt();

    for (unsigned int i = 0; i < links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) > 0)
            links[i].SetOutflowLimitFactor(blocks[links[i].s_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
        else if (blocks[links[i].e_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) < 0)
            links[i].SetOutflowLimitFactor(blocks[links[i].e_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
        else
            links[i].SetOutflowLimitFactor(1, Expression::timing::present);

    }

    for (unsigned int i = 0; i < links.size(); i++)
    {
        UpdatedStorage[links[i].s_Block_No()] -= links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) * links[i].GetOutflowLimitFactor(Expression::timing::present)* dt();
        UpdatedStorage[links[i].e_Block_No()] += links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) * links[i].GetOutflowLimitFactor(Expression::timing::present)* dt();
    }

    for (unsigned int i = 0; i < blocks.size(); i++)
        blocks[i].SetVal(variable, UpdatedStorage[i], Expression::timing::present);

}

bool System::CalculateFlows(const string &var, const Expression::timing &tmg)
{
    for (unsigned int i=0; i<links.size(); i++)
    {
        links[i].SetVal(var,links[i].CalcVal(var,tmg));
    }
	return true;
}

CMatrix_arma System::Jacobian(const string &variable, CVector_arma &X, bool transport)
{
    CMatrix_arma M(X.size());
    CVector_arma F0;

    F0 = GetResiduals(variable, X, transport);


    for (int i=0; i < X.size(); i++)
    {
        CVector_arma V = Jacobian(variable, X, F0, i,transport);
        for (int j=0; j<X.size(); j++)
            M(i,j) = V[j];


    }

  return Transpose(M);
}

#ifdef SUPER_LU
CMatrix_arma_sp System::Jacobian_SP(const string &variable, CVector_arma &X, bool transport)
{
    CMatrix_arma_sp M(X.num);
    CVector_arma F0;

    F0 = GetResiduals(variable, X, transport);


    for (int i=0; i < X.num; i++)
    {
        CVector_arma V = Jacobian(variable, X, F0, i,transport);
        for (int j=0; j<X.num; j++)
            if (V[j]!=0)
                M(i,j) = V[j];


    }

  return Transpose(M);
}
#endif

CVector_arma System::Jacobian(const string &variable, CVector_arma &V, CVector_arma &F0, int i, bool transport)
{
      double epsilon;
      double u = 1;
      //if (unitrandom()>0.5) u=1; else u=-1;
      
      epsilon = -1e-6*u*(fabs(V[i])+1);
      CVector_arma V1(V);
      V1[i] += epsilon;
      CVector_arma F1 = GetResiduals(variable,V1,transport);

      CVector_arma grad = (F1 - F0) / epsilon;
      if (!grad.is_finite() || grad[i]==0)
      {
            epsilon = +1e-6*u*(fabs(V[i]) + 1);
            V1 = V;
            V1[i] += epsilon;
            F1 = GetResiduals(variable,V1,transport);
            grad = (F1 - F0) / epsilon;
      }
      if (grad[i]==0 && transport)
      {
          //qDebug()<<"Diagonal of jacobian is zero for block" << QString::fromStdString(blocks[i].GetName());
          //SavetoJson("state.json",addedtemplates,true,true);

          //cout<<"Diagonal Zero!"<<endl;
      }
      return grad;

}



void System::SetVariableParents()
    {
    for (unsigned int i = 0; i < blocks.size(); i++) blocks[i].ClearLinksToFrom();

    for (unsigned int i = 0; i < links.size(); i++)
	{
		links[i].SetVariableParents();
        links[i].Set_s_Block(&blocks[int(links[i].s_Block_No())]);
		links[i].Set_e_Block(&blocks[links[i].e_Block_No()]);
        blocks[links[i].e_Block_No()].AppendLink(i, Expression::loc::destination);
        blocks[links[i].s_Block_No()].AppendLink(i, Expression::loc::source);
    }


	for (unsigned int i = 0; i < blocks.size(); i++)
	{
		blocks[i].SetVariableParents();
	}

	for (unsigned int i = 0; i < sources.size(); i++)
	{
		sources[i].SetVariableParents();
	}

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
    {
        reaction_parameters[i].SetVariableParents();
    }

    for (unsigned int i = 0; i < reactions.size(); i++)
    {
        reactions[i].SetVariableParents();
    }

    for (unsigned int i = 0; i < parameter_set.size(); i++)
    {
        parameter_set[i]->SetVariableParents();
    }

    for (unsigned int i = 0; i < observations.size(); i++)
    {
        observations[i].SetVariableParents();
    }

    for (unsigned int i = 0; i < objective_function_set.size(); i++)
    {
        objective_function_set[i]->SetVariableParents();
    }
}

vector<string> System::GetAllBlockTypes() const 
{
    vector<string> out;
    for (map<string, QuanSet>::const_iterator it = metamodel.GetMetaModel()->cbegin(); it != metamodel.GetMetaModel()->cend(); it++)
        if (it->second.BlockLink == blocklink::block)
        {
            //ShowMessage(it->first);
            out.push_back(it->first);
        }

    return out;

}

vector<string> System::GetAllLinkTypes() const 
{
    vector<string> out;
    for (map<string, QuanSet>::const_iterator it = metamodel.GetMetaModel()->cbegin(); it != metamodel.GetMetaModel()->cend(); it++)
        if (it->second.BlockLink == blocklink::link)
        {
            //ShowMessage(it->first);
            out.push_back(it->first);
        }

    return out;
}

vector<string> System::GetAllSourceTypes() const
{
	vector<string> out;
	for (map<string, QuanSet>::const_iterator it = metamodel.GetMetaModel()->cbegin(); it != metamodel.GetMetaModel()->cend(); it++)
		if (it->second.BlockLink == blocklink::source)
		{
            //ShowMessage(it->first);
			out.push_back(it->first);
		}

	return out;
}

vector<string> System::GetAllSourceNames() const
{
    vector<string> out;
    for (unsigned int i=0; i<sources.size(); i++)
        out.push_back(sources[i].GetName());
    return out;
}

vector<string> System::GetAllBlockNames() const
{
    vector<string> out;
    for (unsigned int i=0; i<blocks.size(); i++)
        out.push_back(blocks[i].GetName());
    return out;
}
 
vector<string> System::GetAllLinkNames() const
{
    vector<string> out;
    for (unsigned int i=0; i<links.size(); i++)
        out.push_back(links[i].GetName());
    return out;
}

vector<string> System::GetAllReactionNames() const
{
    vector<string> out;
    for (unsigned int i=0; i<reactions.size(); i++)
        out.push_back(reactions[i].GetName());
    return out;

}

vector<string> System::GetAllObservationNames() const
{
    vector<string> out;
    for (unsigned int i=0; i<observations.size(); i++)
        out.push_back(observations[i].GetName());
    return out;

}


vector<string> System::GetAllTypesOf(const string& type) const
{
	vector<string> out;
	for (map<string, QuanSet>::const_iterator it = metamodel.GetMetaModel()->cbegin(); it != metamodel.GetMetaModel()->cend(); it++)
		if (it->second.CategoryType() == type)
		{
            //ShowMessage(it->first);
			out.push_back(it->first);
		}

	return out;
}

void System::clear()
{
    blocks.clear();
    links.clear();
    Outputs.AllOutputs.clear();
    Outputs.ObservedOutputs.clear();
    sources.clear();
    objective_function_set.clear();
    parameter_set.clear();
    reactions.clear();
    reaction_parameters.clear();
    observations.clear();
    constituents.clear();
    errorhandler.clear();
    addedpropertiestoallblocks.clear();
    addedpropertiestoalllinks.clear(); 
    addedtemplates.clear(); 
    alltimeseries.clear(); 
    fit_measures.clear(); 
    
}

void System::TransferQuantitiesFromMetaModel()
{
    solvevariableorder = metamodel.solvevariableorder;
	SetNumberOfStateVariables(solvevariableorder.size()); // The size of the SolutionTemporaryVariables are adjusted based on the number of state variables.
    vector<string> out;
    for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
        GetVars()->Append(it->second);
}

void System::AppendQuantitiesFromMetaModel()
{
    for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
    {
        if (Variable(it->first)==nullptr)
            GetVars()->Append(it->second);
    }
}


Objective_Function* System::ObjectiveFunction(const std::string& name)
{
	for (int i = 0; i < objective_function_set.size(); i++)
		if (objective_function_set[i]->GetName() == name)
			return objective_function_set[i];

    return nullptr;
}

void System::AppendObjectiveFunction(const string &name, const Objective_Function &obj, double weight)
{
    objective_function_set.Append(name,obj, weight);
    objective_function_set[name]->SetSystem(this);
    return;
}



bool System::AppendObjectiveFunction(const string &name, const string &location, const Expression &expr, double weight)
{
    Objective_Function obj(this,expr,location);
    obj.SetSystem(this);
    if (object(location)!=nullptr || true)
    {
        obj.SetQuantities(metamodel, "Objective_Function");
        objective_function_set.Append(name,obj, weight);
        objective_function_set[name]->SetSystem(this);
        return true;
    }

    errorhandler.Append(name,"System","AppendObjectiveFunction","Location '" + name + "' was not found", 501);
    return false;
}


void System::UpdateObjectiveFunctions(double t)
{
    objective_function_set.Update(t);
    return;
}

void System::UpdateObservations(double t)
{
    for (unsigned int i=0; i<ObservationsCount(); i++)
        observations[i].append_value(t);
    return;
}

double System::GetObjectiveFunctionValue()
{
    if (GetSolutionFailed())
        return  +1e18;
    if (ParameterEstimationMode == parameter_estimation_options::optimize)
        return objective_function_set.Calculate();
    else
        return CalcMisfit();
}

void System::MakeObjectiveFunctionExpressionUniform()
{

    for (unsigned int i=0; i < ObjectiveFunctionsCount(); i++)
        objective_function_set[i]->SetTimeSeries(objective_function_set[i]->GetTimeSeries()->make_uniform(SimulationParameters.dt0));

}

void System::MakeObservationsExpressionUniform()
{

    for (unsigned int i=0; i < ObservationsCount(); i++)
        observation(i)->SetModeledTimeSeries(observations[i].GetModeledTimeSeries()->make_uniform(SimulationParameters.dt0));

}

double System::CalcMisfit()
{
    double out=0;

    for (unsigned int i=0; i<ObservationsCount(); i++)
    {   out+=observation(i)->CalcMisfit();
        fit_measures[i*3] = observation(i)->fit_measures[0];
        fit_measures[i*3+1] = observation(i)->fit_measures[1];
        fit_measures[i*3+2] = observation(i)->fit_measures[2];
    }

    return out;
}

void System::SetParameterEstimationMode(parameter_estimation_options mode)
{
    ParameterEstimationMode = mode;
}

void System::SetQuanPointers()
{
    for (unsigned int i=0; i<BlockCount(); i++)
    {
        blocks[i].SetQuanPointers();
    }
    for (unsigned int i=0; i<LinksCount(); i++)
    {
        links[i].SetQuanPointers();
    }
}

bool System::RemoveAsParameter(const string &location, const string &quantity, const string &parametername)
{
    if (GetParameter(parametername)!=nullptr)
        return GetParameter(parametername)->RemoveLocationQuan(location,quantity);
    return false;
}
bool System::SetAsParameter(const string &location, const string &quantity, const string &parametername)
{
    if (object(location)==nullptr)
    {
        errorhandler.Append(GetName(),"System","SetAsParameter","Location '" + location + "' does not exist", 601);
        return false;
    }
    if (GetParameter(parametername) == nullptr)
    {
        errorhandler.Append(GetName(),"System","SetAsParameter","Parameter " + parametername + "does not exist!", 602);
        return false;
    }
    if (link(location)!=nullptr)
    {
        if (!link(location)->HasQuantity(quantity))
        {
            errorhandler.Append(GetName(),"System","SetAsParameter","In link" + location + ": variable " + quantity + " does not exist!", 603);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }

    if (block(location)!=nullptr)
    {
        if (!block(location)->HasQuantity(quantity))
        {
            lasterror() = "In block" + location + ": variable " + quantity + " does not exist";
            errorhandler.Append(GetName(),"System","SetAsParameter",lasterror(), 604);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }

    if (observation(location) != nullptr)
    {
        if (!observation(location)->HasQuantity(quantity))
        {
            lasterror() = "In observation" + location + ": variable " + quantity + " does not exist";
            errorhandler.Append(GetName(), "System", "SetAsParameter", lasterror(), 605);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }

    if (reactionparameter(location) != nullptr)
    {
        if (!reactionparameter(location)->HasQuantity(quantity))
        {
            lasterror() = "In reaction parameter" + location + ": variable " + quantity + " does not exist";
            errorhandler.Append(GetName(), "System", "SetAsParameter", lasterror(), 606);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }

    if (source(location) != nullptr)
    {
        if (!source(location)->HasQuantity(quantity))
        {
            lasterror() = "In source" + location + ": variable " + quantity + " does not exist";
            errorhandler.Append(GetName(), "System", "SetAsParameter", lasterror(), 607);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }

    if (constituent(location) != nullptr)
    {
        if (!constituent(location)->HasQuantity(quantity))
        {
            lasterror() = "In constituent" + location + ": variable " + quantity + " does not exist";
            errorhandler.Append(GetName(), "System", "SetAsParameter", lasterror(), 608);
            return false;
        }
        else
        {
            GetParameter(parametername)->AppendLocationQuan(location, quantity);
            return true;
        }
    }
    return false;
}

bool System::SetAsParameter(const string& location, const string& quantity, const string& parametername, bool Full)
{
    SetAsParameter(location, quantity, parametername);
    if (!object(location))
        return false; 
    object(location)->Variable(quantity)->SetParameterAssignedTo(parametername);
    return true; 
}

bool System::AppendParameter(const string &paramname, const double &lower_limit, const double &upper_limit, const string &prior_distribution)
{
    if (Parameters().Contains(paramname))
    {
        errorhandler.Append(GetName(),"System","AppendParameter","Parameter '" + paramname + "' has already been defined!", 605);
        return false;
    }
    else
    {
        Parameter param;
        param.SetType("Parameter");
        param.SetRange(lower_limit, upper_limit);
        param.SetPriorDistribution(prior_distribution);
        param.SetQuantities(metamodel, "Parameter");

        Parameters().Append(paramname,param);
        return true;
    }
}

bool System::AppendParameter(const string &paramname, const Parameter& param)
{
    if (Parameters().Contains(paramname))
    {
        errorhandler.Append(GetName(),"System","AppendParameter","Parameter " + paramname + " already exists!", 606);
        return false;
    }
    else
    {
        Parameters().Append(paramname,param);
        return true;
    }
}

bool System::SetParameterValue(const string &paramname, const double &val)
{
    if (!Parameters().Contains(paramname))
    {
        errorhandler.Append(GetName(),"System","SetParameterValue","Parameter " + paramname + " does not exist!", 608);
        return false;
    }
    else
    {
        GetParameter(paramname)->SetValue(val);
        return true;
    }
}

bool System::SetParameterValue(int i, const double &val)
{
    GetParameter(Parameters().getKeyAtIndex(i))->SetValue(val);
	return true;
}

bool System::ApplyParameters()
{
    for (unsigned int i = 0; i < Parameters().size(); i++)
        for (unsigned int j=0; j<GetParameter(i)->GetLocations().size();j++)
        {
            if (object(GetParameter(i)->GetLocations()[j])!=nullptr)
                object(GetParameter(i)->GetLocations()[j])->SetVal(GetParameter(i)->GetQuans()[j],GetParameter(i)->GetValue());
            else
            {
                errorhandler.Append(GetName(),"System","ApplyParameters" ,"Location '" + GetParameter(i)->GetLocations()[j] + "' does not exist!", 607);
                return false;
            }
        }
    return true;

}

void System::SetAllParents()
{
    SetVariableParents();
    for (unsigned int i = 0; i < links.size(); i++)
        links[i].SetParent(this);

    for (unsigned int i = 0; i < blocks.size(); i++)
        blocks[i].SetParent(this);

    for (unsigned int i = 0; i < sources.size(); i++)
        sources[i].SetParent(this);

    for (unsigned int i = 0; i < constituents.size(); i++)
        constituents[i].SetParent(this);
    
    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        reaction_parameters[i].SetParent(this);

    for (unsigned int i = 0; i < objective_function_set.size(); i++)
    {
        objective_function_set[i]->SetParent(this);
        objective_function_set[i]->SetSystem(this);
    }
    for (unsigned int i = 0; i < observations.size(); i++)
    {
        observations[i].SetParent(this);
        observations[i].SetSystem(this);
    }
    for (unsigned int i = 0; i < ParametersCount(); i++)
    {
        Parameters()[i]->SetParent(this);
        //Parameters()[i]->SetSystem(this);
    }

}

bool System::Echo(const string &obj, const string &quant, const string &feature)
{
    if (object(obj)==nullptr && parameter(obj)==nullptr)
    {
        errorhandler.Append(GetName(),"System","Echo" ,"Object or parameter '" + obj + "' does not exits!", 608);
        return false;
    }
    if (quant=="")
    {
        if (object(obj)!=nullptr)
        {
            cout<<object(obj)->toString()<<std::endl;
            return true;
        }
        else if (parameter(obj)!=nullptr)
        {
            cout<<parameter(obj)->toString()<<std::endl;
            return true;
        }
        else
        {
            errorhandler.Append(GetName(),"System","Echo" ,"Object or parameter '" + obj + "' does not exits!", 608);
            return false;
        }
    }
    else
    {
        if (object(obj)!=nullptr)
        {
            if (!object(obj)->HasQuantity(quant))
            {
                errorhandler.Append(GetName(),"System","Echo","Object '" + obj + "' has no property/quantity '" + quant + "'",613);
                return false;
            }
            if (feature == "")
                cout<<object(obj)->Variable(quant)->ToString()<<std::endl;
            else if (aquiutils::tolower(feature) == "value")
                cout<<object(obj)->Variable(quant)->GetVal();
            else if (aquiutils::tolower(feature) == "rule")
                cout<<object(obj)->Variable(quant)->GetRule()->ToString();
            else if (aquiutils::tolower(feature) == "type")
                cout<<tostring(object(obj)->Variable(quant)->GetType());
            else
            {
                errorhandler.Append(GetName(),"System","Echo","Feature '" + feature + "' is not valid!",612);
                return false;
            }
            return true;
        }
        else if (parameter(obj)!=nullptr)
        {
            if (!parameter(obj)->HasQuantity(quant))
            {
                errorhandler.Append(GetName(),"System","Echo","Parameter '" + obj + "' has no property/quantity '" + quant + "'",618);
                return false;
            }
            else
            {
                cout<<parameter(obj)->Variable(quant)<<std::endl;
                return true;
            }
        }
    }
    return false;

}

SafeVector<TimeSeries<timeseriesprecision>*> System::GetTimeSeries(bool onlyprecip)
{
    SafeVector<TimeSeries<timeseriesprecision>*> out;
    for (unsigned int i=0; i<links.size(); i++)
    {
        vector<TimeSeries<timeseriesprecision>*> linktimeseires = links[i].GetTimeSeries(onlyprecip);
        for (unsigned int j=0; j<linktimeseires.size(); j++)
        {
            links[i].GetTimeSeries()[j]->assign_D();
            out.push_back(linktimeseires[j]);
        }
    }

    for (unsigned int i=0; i<blocks.size(); i++)
    {
        vector<TimeSeries<timeseriesprecision>*> blocktimeseires = blocks[i].GetTimeSeries(onlyprecip);
        for (unsigned int j=0; j<blocktimeseires.size(); j++)
        {
            blocks[i].GetTimeSeries()[j]->assign_D();
            out.push_back(blocktimeseires[j]);
        }
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
        vector<TimeSeries<timeseriesprecision>*> sourcetimeseires = sources[i].GetTimeSeries(onlyprecip);
        for (unsigned int j=0; j<sourcetimeseires.size(); j++)
        {
            sources[i].GetTimeSeries()[j]->assign_D();
            out.push_back(sourcetimeseires[j]);
        }
    }

    return out;
}

double System::GetMinimumNextTimeStepSize()
{
    timeseriesprecision x=1e12;

    for (unsigned int i=0; i<alltimeseries.size(); i++)
    {
        x = min(x,alltimeseries[i]->interpol_D(this->SolverTempVars.t));
    }
    return max(x,timeseriesprecision(0.001));
}

#if defined(QT_version) || defined(Q_JSON_SUPPORT)
QStringList System::QGetAllCategoryTypes()
{
	QStringList out;
	for (map<string, QuanSet>::iterator it = metamodel.GetMetaModel()->begin(); it != metamodel.GetMetaModel()->end(); it++)
	{
		if (!out.contains(QString::fromStdString(it->second.CategoryType())))
			out.append(QString::fromStdString(it->second.CategoryType()));
	}

	return out;
}

QStringList System::QGetAllObjectsofTypeCategory(QString _type)
{
	QStringList out;
	if (_type == "Blocks")
		for (unsigned int i = 0; i < blocks.size(); i++)
			out.append(QString::fromStdString(blocks[i].GetName()));

	if (_type == "Connectors")
		for (unsigned int i = 0; i < links.size(); i++)
			out.append(QString::fromStdString(links[i].GetName()));

	if (_type == "Sources")
		for (unsigned int i = 0; i < sources.size(); i++)
			out.append(QString::fromStdString(sources[i].GetName()));


	if (_type == "Parameters")
		for (unsigned int i = 0; i < Parameters().size()  ; i++)
			out.append(QString::fromStdString(Parameters().getKeyAtIndex(i)));

	if (_type == "Objective_Functions")

	return out;
}
#endif // Qt_version


bool System::SavetoScriptFile(const string &filename, const string &templatefilename, const vector<string> &_addedtemplates)
{
    SetVariableParents();
    if (_addedtemplates.size()!=0)
        addedtemplates = _addedtemplates;
    fstream file(filename,ios_base::out);
    if (templatefilename!="")
    {
        file << "loadtemplate; filename = " << templatefilename << std::endl;
    }

    for (unsigned int i=0; i<addedtemplates.size();i++)
    {
        file << "addtemplate; filename = " << addedtemplates[i] << std::endl;
    }

    for (unsigned int i=0; i<Settings.size(); i++)
        for (unordered_map<string, Quan>::iterator j=Settings[i].GetVars()->begin(); j!=Settings[i].GetVars()->end(); j++)
            if (j->second.AskFromUser())
                file << "setvalue; object=system, quantity=" + j->first + ", value=" << j->second.GetProperty() << std::endl;

    for (unsigned int i=0; i<sources.size(); i++)
        file << "create source;" << sources[i].toCommand() << std::endl;

    for (unsigned int i=0; i<ParametersCount(); i++)
        file << "create parameter;" << Parameters()[i]->toCommand() << std::endl;

    for (unsigned int i=0; i<ConstituentsCount(); i++)
        file << "create constituent;" << constituents[i].toCommand() << std::endl;

    for (unsigned int i = 0; i < ReactionParametersCount(); i++)
        file << "create reaction_parameter;" << reaction_parameters[i].toCommand() << std::endl;

    for (unsigned int i = 0; i < ReactionsCount(); i++)
        file << "create reaction;" << reactions[i].toCommand() << std::endl;

    for (unsigned int i=0; i<blocks.size(); i++)
        file << "create block;" << blocks[i].toCommand() << std::endl;

    for (unsigned int i=0; i<links.size(); i++)
        file << "create link;" << links[i].toCommand() << std::endl;

    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        file << "create objectivefunction;" << ObjectiveFunctions()[i]->toCommand() << std::endl;

    for (unsigned int i = 0; i < ObservationsCount(); i++)
        file << "create observation;" << observation(i)->toCommand() << std::endl;



    for (unsigned int i=0; i<blocks.size(); i++)
        if (blocks[i].toCommandSetAsParam()!="")
			file << blocks[i].toCommandSetAsParam() << std::endl;

    for (unsigned int i=0; i<links.size(); i++)
		if (links[i].toCommandSetAsParam() != "")
			file << links[i].toCommandSetAsParam() << std::endl;

    for (unsigned int i = 0; i < observations.size(); i++)
        if (observations[i].toCommandSetAsParam() != "")
            file << observations[i].toCommandSetAsParam() << std::endl;

    for (unsigned int i = 0; i < sources.size(); i++)
        if (sources[i].toCommandSetAsParam() != "")
            file << sources[i].toCommandSetAsParam() << std::endl;

    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        if (reaction_parameters[i].toCommandSetAsParam() != "")
            file << reaction_parameters[i].toCommandSetAsParam() << std::endl;

    for (unsigned int i = 0; i < constituents.size(); i++)
        if (constituents[i].toCommandSetAsParam() != "")
            file << constituents[i].toCommandSetAsParam() << std::endl;
    file.close();

    return true;
}

System::System(Script& scr)
{
    for (int i=0; i<scr.CommandsCount(); i++)
    {
        if (!scr[i]->Execute(this))
        {
            errorhandler.Append("","Script","CreateSystem",scr[i]->LastError(),6001);
            scr.Errors().push_back(scr[i]->LastError());
        }
    }
    InitOpenMP();
    SetVariableParents();
    Object::AssignRandomPrimaryKey();

}

bool System::CreateFromScript(Script& scr, const string& settingsfilename)
{
    bool success = true;

    for (int i=0; i<scr.CommandsCount(); i++)
    {
        if (!scr[i]->Execute(this))
        {
            errorhandler.Append("","Script","CreateSystem",scr[i]->LastError(),6001);
            scr.Errors().push_back(scr[i]->LastError());
            success = false;
        }
        if (scr[i]->Keyword() == "loadtemplate")
        {
            if (settingsfilename != "")
                ReadSystemSettingsTemplate(settingsfilename);
        }
    }
    InitOpenMP();
    SetVariableParents();
    return success;


}

bool System::ReadSystemSettingsTemplate(const string &filename)
{
    Settings.clear();
    Json::Value root;
    Json::Reader reader;

    std::ifstream file(filename);
    if (!file.good())
    {
        cout << "File " + filename + " was not found!";
        return false;
    }

    file >> root;

    if (!reader.parse(file, root, true)) {
        //for some reason it always fails to parse
        std::cout << "Failed to parse configuration\n"
            << reader.getFormattedErrorMessages();
        lasterror() = "Failed to parse configuration\n";
    }

    Settings.clear();
    for (Json::ValueIterator object_types = root.begin(); object_types != root.end(); ++object_types)
    {
        //qDebug()<<QString::fromStdString(object_types.name());
        QuanSet quanset(object_types);
        //qDebug()<<QString::fromStdString(object_types.key().asString());
        //qDebug()<<1;
        Object settingsitem;
        //qDebug()<<2;
        quanset.SetParent(this);
        //qDebug()<<3;
        settingsitem.SetQuantities(quanset);
        //qDebug()<<4;
        settingsitem.SetDefaults();
        //qDebug()<<5;
        Settings.push_back(settingsitem);
        //qDebug()<<6;
        metamodel.Append(object_types.key().asString(), quanset);

    }
    return true;
}

void System::SetSystemSettings()
{
    for (unsigned int i=0; i<Settings.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator j=Settings[i].GetVars()->begin(); j!=Settings[i].GetVars()->end(); j++)
            SetProperty(j->first,j->second.GetProperty());

    }

}

void System::SetSettingsParameter(const string &name, const double &value)
{
    for (unsigned int i=0; i<Settings.size(); i++)
    {
        for (unordered_map<string, Quan>::iterator j=Settings[i].GetVars()->begin(); j!=Settings[i].GetVars()->end(); j++)
            if (j->first == name)
                j->second.SetVal(value,Expression::timing::both);

    }

}


void System::DisconnectLink(const string linkname)
{
    for (unsigned int i = 0; i < links.size(); i++)
        if (links[i].GetName() == linkname)
        {
            block(links[i].GetConnectedBlock(Expression::loc::source)->GetName())->deletelinkstofrom(links[i].GetName());
        }
}

bool System::Delete(const string& objectname)
{
    for (unsigned int i = 0; i < links.size(); i++)
        if (links[i].GetName() == objectname)
        {
            block(links[i].GetConnectedBlock(Expression::loc::source)->GetName())->deletelinkstofrom(links[i].GetName());
            for (unsigned int j = 0; j < blocks.size(); j++)
            {
                blocks[j].shiftlinkIds(i);
            }
            links.erase(links.begin()+i);

            return true;
        }

    for (unsigned int i = 0; i < blocks.size(); i++)
        if (blocks[i].GetName() == objectname)
        {
            vector<string> links_to_be_deleted;
            for (unsigned int j = 0; j < blocks[i].GetLinksFrom().size(); j++)
                links_to_be_deleted.push_back(blocks[i].GetLinksFrom()[j]->GetName());

            for (unsigned int j = 0; j < blocks[i].GetLinksTo().size(); j++)
                links_to_be_deleted.push_back(blocks[i].GetLinksTo()[j]->GetName());

            //for (unsigned int j = 0; j < links_to_be_deleted.size(); j++)
            //    DisconnectLink(links_to_be_deleted[j]);

            for (unsigned int j = 0; j < links_to_be_deleted.size(); j++)
                Delete(links_to_be_deleted[j]);

            blocks.erase(blocks.begin() + i);
            for (unsigned int j = 0; j < links.size(); j++)
            {
                if (links[j].s_Block_No() >= i)
                    links[j].ShiftLinkedBlock(-1, Expression::loc::source);
                if (links[j].e_Block_No() >= i)
                    links[j].ShiftLinkedBlock(-1, Expression::loc::destination);
            }

            return true;
        }

    for (unsigned int i = 0; i < sources.size(); i++)
        if (sources[i].GetName() == objectname)
        {
            sources.erase(sources.begin() + i);
            return true;
        }

    for (unsigned int i = 0; i < ParametersCount(); i++)
        if (Parameters()[i]->GetName() == objectname)
        {
            return Parameters().erase(i);
        }

    for (unsigned int i = 0; i < ConstituentsCount(); i++)
        if (constituent(i)->GetName() == objectname)
        {
            EraseConstituentRelatedProperties(objectname);
            constituents.erase(constituents.begin() + i);
            return true;
        }

    for (unsigned int i = 0; i < ObservationsCount(); i++)
        if (observation(i)->GetName() == objectname)
        {
            observations.erase(observations.begin() + i);
        }

    for (unsigned int i = 0; i < ReactionsCount(); i++)
        if (reaction(i)->GetName() == objectname)
        {
            reactions.erase(reactions.begin() + i);
        }

    for (unsigned int i = 0; i < ReactionParametersCount(); i++)
        if (reactionparameter(i)->GetName() == objectname)
        {
            EraseConstituentRelatedProperties(objectname);
            reaction_parameters.erase(reaction_parameters.begin() + i);
        }

    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        if (ObjectiveFunctions()[i]->GetName() == objectname)
        {
            return ObjectiveFunctions().erase(i);
        }

    errorhandler.Append(GetName(),"System","object","Object '" + objectname + "' was not found",105);

    return false;

}

bool System::EraseConstituentRelatedProperties(const string &constituent_name)
{
    for (unsigned int i = 0; i < links.size(); i++)
    {
        links[i].GetVars()->DeleteConstituentRelatedProperties(constituent_name);
    }
    for (unsigned int i = 0; i < blocks.size(); i++)
    {
        blocks[i].GetVars()->DeleteConstituentRelatedProperties(constituent_name);
    }
    for (unsigned int i = 0; i < sources.size(); i++)
    {
        sources[i].GetVars()->DeleteConstituentRelatedProperties(constituent_name);
    }
    for (unsigned int i = 0; i < reactions.size(); i++)
    {
        reactions[i].GetVars()->DeleteConstituentRelatedProperties(constituent_name);
    }
    return true;
}

bool System::VerifyAsSource(Block* blk, Link* lnk)
{
    for (unsigned int i = 0; i < lnk->GetAllRequieredStartingBlockProperties().size(); i++)
    {
        if (!blk->HasQuantity(lnk->GetAllRequieredStartingBlockProperties()[i]))
        {
            errorhandler.Append(lnk->GetName(), "System", "VerifyAsSource", "The source block must have a '" + lnk->GetAllRequieredStartingBlockProperties()[i] + "' property",106);
            lasterror() = "The source block must have a '" + lnk->GetAllRequieredStartingBlockProperties()[i] + "' property";
            return false;
        }
    }
    return true;
}


bool System::VerifyAsDestination(Block* blk, Link* lnk)
{
    for (unsigned int i = 0; i < lnk->GetAllRequieredDestinationBlockProperties().size(); i++)
    {
        if (!blk->HasQuantity(lnk->GetAllRequieredDestinationBlockProperties()[i]))
        {
            errorhandler.Append(lnk->GetName(), "System", "VerifyAsSource", "The destination block must have a '" + lnk->GetAllRequieredDestinationBlockProperties()[i] + "' property", 106);
            lasterror() = "The destination block must have a '" + lnk->GetAllRequieredDestinationBlockProperties()[i] + "' property";
            return false;
        }
    }
    return true;

}

void System::AddPropertytoAllBlocks(const string &name, Quan &quan)
{
    if (addedpropertiestoallblocks.count(name)==0)
        addedpropertiestoallblocks[name] = quan;
    else
        errorhandler.Append("System", "System", "AddPropertytoAllBlocks", "Property '" + name + "' has already been added to all blocks", 7649);
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        if (!blocks[i].HasQuantity(name))
        {
            blocks[i].AddQnantity(name, quan);
        }
    }
}
void System::AddPropertytoAllLinks(const string &name, Quan &quan)
{
    if (addedpropertiestoalllinks.count(name)==0)
        addedpropertiestoalllinks[name] = quan;
    else
        errorhandler.Append("System", "System", "AddPropertytoAllLinks", "Property '" + name + "' has already been added to all links", 7648);
    for (unsigned int i=0; i<links.size(); i++)
    {
        if (!links[i].HasQuantity(name))
        {
            links[i].AddQnantity(name, quan);
        }
    }
}
void System::UpdateAddedPropertiestoAllBlockLinks()
{
    for (map<string,Quan>::iterator it = addedpropertiestoallblocks.begin(); it!=addedpropertiestoallblocks.end(); it++  )
    {
        for (unsigned int i=0; i<blocks.size(); i++)
        {
            if (!blocks[i].HasQuantity(it->first))
            {
                blocks[i].AddQnantity(it->first, it->second);
            }
        }
    }

    for (map<string,Quan>::iterator it = addedpropertiestoalllinks.begin(); it!=addedpropertiestoalllinks.end(); it++  )
    {
        for (unsigned int i=0; i<links.size(); i++)
        {
            if (!links[i].HasQuantity(it->first))
            {
                links[i].AddQnantity(it->first, it->second);
            }
        }
    }
}

vector<Quan> System::GetToBeCopiedQuantities()
{
    vector<Quan> quantitiestobecopiedtoallobjects;
    for (unsigned int i = 0; i < constituents.size(); i++)
    {
        vector<Quan> quans = constituent(i)->GetCopyofAllQuans();
        for (unsigned int j = 0; j < quans.size(); j++)
        {
            if (quans[j].WhenCopied())
            {
                quans[j].SetName(constituent(i)->GetName() + ":" + quans[j].GetName());
                quans[j].SetRole(Quan::_role::none);
                quantitiestobecopiedtoallobjects.push_back(quans[j]);
            }
        }

    }
    return quantitiestobecopiedtoallobjects;

}

vector<Quan> System::GetToBeCopiedQuantities(Object *consttnt, const object_type &object_typ)
{
    Quan::_role role = Quan::_role::none;
    if (object_typ == object_type::block)
        role = Quan::_role::copytoblocks;
    if (object_typ == object_type::link)
        role = Quan::_role::copytolinks;
    if (object_typ == object_type::source)
        role = Quan::_role::copytosources;
    if (object_typ == object_type::reaction)
        role = Quan::_role::copytoreactions;

    vector<Quan> quantitiestobecopiedtoallobjects;
    vector<Quan> original_quans;
    vector<Quan> quans;
    if (consttnt)
    {
        original_quans = consttnt->GetCopyofAllQuans();
        quans = original_quans;
    }

    for (unsigned int j = 0; j < quans.size(); j++)
    {
        if (quans[j].WhenCopied() && (quans[j].GetRole() == role || role == Quan::_role::none))
        {
            quans[j].SetName(consttnt->GetName() + ":" + quans[j].GetName());
            quans[j].Description() = consttnt->GetName() + ":" + quans[j].Description();
            quans[j].Description(true) = consttnt->GetName() + ":" + quans[j].Description(true);
            if (quans[j].GetType() == Quan::_type::expression )
            {   for (unsigned int i=0; i<original_quans.size(); i++)
                   quans[j].SetExpression(quans[j].GetExpression()->ReviseConstituent(consttnt->GetName(),original_quans[i].GetName()));
            }
            if (quans[j].calcinivalue())
            {
                for (unsigned int i=0; i<original_quans.size(); i++)
                    quans[j].SetInitialValueExpression(quans[j].InitialValueExpression().ReviseConstituent(consttnt->GetName(),original_quans[i].GetName()));
            }
            quans[j].SetRole(Quan::_role::none);
            quantitiestobecopiedtoallobjects.push_back(quans[j]);
        }
    }

    return quantitiestobecopiedtoallobjects;
}

bool System::AddAllConstituentRelateProperties()
{

    return true;

}

vector<string> System::AllConstituents()
{
    vector<string> out;
    for (unsigned int i=0; i<constituents.size();i++)
        out.push_back(constituent(i)->GetName());

    return out;
}

vector<string> System::AllReactionParameters()
{
    vector<string> out;
    for (unsigned int i=0; i<reaction_parameters.size();i++)
        out.push_back(reactionparameter(i)->GetName());

    return out;

}


bool System::AddAllConstituentRelateProperties(Block *blk)
{
    vector<string> quantityordertobechanged;
    for (unsigned int i=0; i<constituents.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(constituent(i),object_type::block);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (blk)
            {   if (blk->GetVars()->Count(quanstobecopied[j].GetName())==0)
                {   blk->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                    blk->Variable(quanstobecopied[j].GetName())->SetParent(blk);
                    quantityordertobechanged.push_back(quanstobecopied[j].GetName());
                }
            }

        }
    }
    if (blk)
    {   for (unsigned int i=0; i<quantityordertobechanged.size(); i++)
            blk->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    }

    for (unsigned int i=0; i<reactions.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(reaction(i),object_type::block);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (blk)
            {   if (blk->GetVars()->Count(quanstobecopied[j].GetName())==0)
                {   blk->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                    blk->Variable(quanstobecopied[j].GetName())->SetParent(blk);
                    quantityordertobechanged.push_back(quanstobecopied[j].GetName());
                }
            }

        }
    }
    if (blk)
    {   for (unsigned int i=0; i<quantityordertobechanged.size(); i++)
            blk->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    }
    for (unsigned int i=0; i<reaction_parameters.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(reactionparameter(i),object_type::block);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (blk)
            {   if (blk->GetVars()->Count(quanstobecopied[j].GetName())==0)
                {   blk->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                    blk->Variable(quanstobecopied[j].GetName())->SetParent(blk);
                    quantityordertobechanged.push_back(quanstobecopied[j].GetName());
                }
            }
        }
    }
    if (blk)
    {   for (unsigned int i=0; i<quantityordertobechanged.size(); i++)
            blk->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    }
    return true;
}

bool System::AddAllConstituentRelateProperties(Reaction *rxn)
{
    vector<string> quantityordertobechanged;
    for (unsigned int i=0; i<constituents.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(constituent(i),object_type::reaction);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (rxn->GetVars()->Count(quanstobecopied[j].GetName())==0)
            {   rxn->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                rxn->Variable(quanstobecopied[j].GetName())->SetParent(rxn);
                quantityordertobechanged.push_back(quanstobecopied[j].GetName());
            }
        }
    }
    for (unsigned int i=0; i<quantityordertobechanged.size(); i++)
        rxn->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    return true;
}

bool System::AddAllConstituentRelateProperties(Link *lnk)
{
    vector<string> quantityordertobechanged;
    for (unsigned int i=0; i<constituents.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(constituent(i),object_type::link);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (lnk->GetVars()->Count(quanstobecopied[j].GetName())==0)
            {   lnk->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                lnk->Variable(quanstobecopied[j].GetName())->SetParent(lnk);
                quantityordertobechanged.push_back(quanstobecopied[j].GetName());
            }
        }
    }
    for (unsigned int i = 0; i < quantityordertobechanged.size(); i++)
        lnk->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    return true;

}

bool System::AddAllConstituentRelateProperties(Source *src)
{
    vector<string> quantityordertobechanged;
    for (unsigned int i=0; i<constituents.size();i++)
    {
        vector<Quan> quanstobecopied = GetToBeCopiedQuantities(constituent(i),object_type::source);
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (src->GetVars()->Count(quanstobecopied[j].GetName())==0)
            {   src->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                src->Variable(quanstobecopied[j].GetName())->SetParent(src);
                quantityordertobechanged.push_back(quanstobecopied[j].GetName());
            }
        }
    }
    for (unsigned int i = 0; i < quantityordertobechanged.size(); i++)
        src->GetVars()->Quantity_Order().push_back(quantityordertobechanged[i]);
    return true;

}


void System::AddConstituentRelatePropertiestoMetalModel()
{
    for (unsigned int j=0; j<ConstituentsCount(); j++)
        AddConstituentRelateProperties(constituent(j));

    for (unsigned int j=0; j<ReactionsCount(); j++)
        AddConstituentRelateProperties(reaction(j));
}

bool System::AddConstituentRelateProperties(Object *consttnt)
{
    vector<Quan> quanstobecopied = GetToBeCopiedQuantities(consttnt,object_type::block);
    for (map<string, QuanSet>::iterator it = metamodel.begin(); it!=metamodel.end(); it++)
    {
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {   if (it->second.Count(quanstobecopied[j].GetName())==0 && aquiutils::tolower(metamodel.GetItem(it->first)->ObjectType) == "block")
            {
                metamodel.GetItem(it->first)->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
            }
        }
    }
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        vector<string> quantityordertobechanged;
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (blocks[i].GetVars()->Count(quanstobecopied[j].GetName())==0)
            {   block(i)->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                block(i)->Variable(quanstobecopied[j].GetName())->SetParent(block(i));
            }
        }
    }
    quanstobecopied = GetToBeCopiedQuantities(consttnt,object_type::link);
    for (map<string, QuanSet>::iterator it = metamodel.begin(); it!=metamodel.end(); it++)
    {
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {   if (it->second.Count(quanstobecopied[j].GetName())==0 && (aquiutils::tolower(metamodel.GetItem(it->first)->ObjectType) == "link" || aquiutils::tolower(metamodel.GetItem(it->first)->ObjectType) == "connector"))
            {
                metamodel.GetItem(it->first)->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
            }
        }
    }
    for (unsigned int i=0; i<links.size(); i++)
    {
        vector<string> quantityordertobechanged;
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (links[i].GetVars()->Count(quanstobecopied[j].GetName())==0)
            {  link(i)->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
               link(i)->Variable(quanstobecopied[j].GetName())->SetParent(link(i));
            }
        }
    }
    quanstobecopied = GetToBeCopiedQuantities(consttnt,object_type::reaction);
    for (map<string, QuanSet>::iterator it = metamodel.begin(); it!=metamodel.end(); it++)
    {
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {   if (it->second.Count(quanstobecopied[j].GetName())==0 && aquiutils::tolower(metamodel.GetItem(it->first)->ObjectType) == "reaction")
            {
                metamodel.GetItem(it->first)->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
            }
        }
    }
    for (unsigned int i=0; i<reactions.size(); i++)
    {
        vector<string> quantityordertobechanged;
        for (unsigned int j=0; j<quanstobecopied.size(); j++)
        {
            if (reactions[i].GetVars()->Count(quanstobecopied[j].GetName())==0)
            {   reaction(i)->GetVars()->Append(quanstobecopied[j].GetName(),quanstobecopied[j]);
                reaction(i)->Variable(quanstobecopied[j].GetName())->SetParent(reaction(i));
            }
        }
    }
    return true;
}




void System::RenameConstituents(const string &oldname, const string &newname)
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].RenameConstituents(oldname, newname);
    }
    for (unsigned int i=0; i<links.size(); i++)
    {
        links[i].RenameConstituents(oldname, newname);
    }
    for (unsigned int i=0; i<sources.size(); i++)
    {
        sources[i].RenameConstituents(oldname, newname);
    }
    for (unsigned int i=0; i<reactions.size(); i++)
    {
        reactions[i].RenameConstituents(oldname, newname);
    }

    metamodel.RenameConstituent(oldname,newname);



}

bool System::CalcAllInitialValues()
{
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        blocks[i].CalculateInitialValues();
    }
    for (unsigned int i=0; i<links.size(); i++)
    {
        links[i].CalculateInitialValues();
    }
    for (unsigned int i=0; i<sources.size(); i++)
    {
        sources[i].CalculateInitialValues();
    }
    return true;
}


void System::SetSolutionLogger(SolutionLogger &slnlogger)
{
    solutionlogger = &slnlogger;

}

bool System::SetSolutionLogger(const string &filename)
{
    if (solutionlogger != nullptr)
        delete solutionlogger;
    solutionlogger = new SolutionLogger(filename);
    return true;

}

SolutionLogger *System::GetSolutionLogger()
{
    return solutionlogger;

}

void System::WriteObjectsToLogger()
{
    if (GetSolutionLogger()==nullptr) return;
    GetSolutionLogger()->WriteString("Blocks:");
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        GetSolutionLogger()->WriteString("    " + aquiutils::numbertostring(i) + ":   " + blocks[i].GetName());
    }
    GetSolutionLogger()->WriteString("Links:");
    for (unsigned int i=0; i<links.size(); i++)
    {
        GetSolutionLogger()->WriteString("    " + aquiutils::numbertostring(i) + ":   " +  links[i].GetName());
    }
    GetSolutionLogger()->WriteString("Sources:");
    for (unsigned int i=0; i<sources.size(); i++)
    {
        GetSolutionLogger()->WriteString("    " + aquiutils::numbertostring(i) + ":   " +  sources[i].GetName());
    }
    GetSolutionLogger()->WriteString("------------------------------------------------------------------------------------");
    GetSolutionLogger()->Flush();
}


void System::WriteBlocksStates(const string &variable, const Expression::timing &tmg)
{
    for (unsigned int i=0; i<blocks.size(); i++)
        GetSolutionLogger()->WriteString("    " + blocks[i].GetName() + ", Storage= " + aquiutils::numbertostring(blocks[i].GetVal(variable,tmg)) + ", correction factor= " + aquiutils::numbertostring(blocks[i].GetOutflowLimitFactor(tmg)) + ", Outflow limiting status:" + aquiutils::numbertostring(blocks[i].GetLimitedOutflow()));

}

void System::WriteLinksStates(const string &variable, const Expression::timing &tmg)
{
    for (unsigned int i=0; i<links.size(); i++)
        GetSolutionLogger()->WriteString("    " + links[i].GetName() + ", Flow = " + aquiutils::numbertostring(links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),tmg)) + ", correction factor= " + aquiutils::numbertostring(links[i].GetOutflowLimitFactor(tmg)));

}

bool System::InitiatePrecalculatedFunctions()
{
    bool out = true;
    for (unsigned int i=0; i<blocks.size(); i++)
    {
//      qDebug()<<QString::fromStdString(blocks[i].GetName());
        out &= blocks[i].InitializePrecalcFunctions();
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
//      qDebug()<<QString::fromStdString(links[i].GetName());
        out &= links[i].InitializePrecalcFunctions();
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
//      qDebug()<<QString::fromStdString(sources[i].GetName());
        out &= sources[i].InitializePrecalcFunctions();
    }
    return out;
    UnUpdateAllValues();
}

double System::Gradient(Object* obj, Object* wrt, const string &dependent_var, const string &independent_var)
{
    double basevalue = obj->GetVal(dependent_var,Expression::timing::present);
    double original_state = wrt->GetVal(independent_var,Expression::timing::present);
    double u;
    if (EpochCount()%2==0) u=1; else u=-1;
    double epsilon = -1e-6*u*(fabs(original_state)+1);
    wrt->SetVal(independent_var,original_state+epsilon,Expression::timing::present);
    wrt->UnUpdateAllValues();
    obj->UnUpdateAllValues();
    double permutedvalue = obj->GetVal(dependent_var,Expression::timing::present);
    double derivative = (permutedvalue - basevalue)/epsilon;
    wrt->SetVal(independent_var,original_state,Expression::timing::present);
    wrt->UnUpdateAllValues();
    obj->UnUpdateAllValues();
    return derivative;

}

bool System::CopyStateVariablesFrom(System *sys)
{
    bool out = true;
    for (unsigned int i=0; i<blocks.size(); i++)
    {
        if (sys->block(blocks[i].GetName())!=nullptr)
            blocks[i].CopyStateVariablesFrom(sys->block(blocks[i].GetName()));
    }

    for (unsigned int i=0; i<links.size(); i++)
    {
        if (sys->link(links[i].GetName())!=nullptr)
            links[i].CopyStateVariablesFrom(sys->link(links[i].GetName()));
    }

    for (unsigned int i=0; i<sources.size(); i++)
    {
        if (sys->source(sources[i].GetName())!=nullptr)
            sources[i].CopyStateVariablesFrom(sys->source(sources[i].GetName()));
    }
    return out;
    UnUpdateAllValues();
}

bool System::ResetBasedOnRestorePoint(RestorePoint *rp)
{
    if (rp->used_counter>1) return false;
    CopyStateVariablesFrom(rp->GetSystem());
    rp->used_counter++;
    SolverTempVars.t = rp->t;
    SolverTempVars.dt_base = rp->dt/5;
    SolverTempVars.dt = rp->dt/5;
    Outputs.AllOutputs.knockout(SolverTempVars.t);
    Outputs.ObservedOutputs.knockout(SolverTempVars.t);
    rp->dt = SolverTempVars.dt;
    return true;
}


CMatrix_arma System::JacobianDirect(const string &variable, CVector_arma &X, bool transport)
{
    for (unsigned int i=0; i<links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) > 0)
        {   links[i].SetOutflowLimitFactor(blocks[links[i].s_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
            links[i].SetLimitedOutflow(true);
        }
        else if (blocks[links[i].e_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)<0)
        {   links[i].SetOutflowLimitFactor(blocks[links[i].e_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
            links[i].SetLimitedOutflow(true);
        }
        else
        {   links[i].SetOutflowLimitFactor(1, Expression::timing::present);
            links[i].SetLimitedOutflow(false);
        }

    }
    CVector_arma current_state = GetStateVariables_for_direct_Jacobian(variable,Expression::timing::present,transport);
    SetStateVariables_for_direct_Jacobian(variable,X,Expression::timing::present,transport);
    CMatrix_arma jacobian(BlockCount());
#ifndef NO_OPENMP
//#pragma omp parallel for schedule(static)
#endif
    for (int i=0; i<LinksCount(); i++)
    {
        if (!link(i)->GetConnectedBlock(Expression::loc::source)->GetLimitedOutflow())
        {   jacobian(link(i)->s_Block_No(),link(i)->s_Block_No()) += Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::source),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
            jacobian(link(i)->e_Block_No(),link(i)->s_Block_No()) -= Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::source),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
        }
        else
        {
            jacobian(link(i)->s_Block_No(),link(i)->s_Block_No()) += aquiutils::Pos(link(i)->GetVal(blocks[link(i)->s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
            jacobian(link(i)->e_Block_No(),link(i)->s_Block_No()) -= aquiutils::Pos(link(i)->GetVal(blocks[link(i)->s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
        }
        if (!link(i)->GetConnectedBlock(Expression::loc::destination)->GetLimitedOutflow())
        {
            jacobian(link(i)->s_Block_No(),link(i)->e_Block_No()) += Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::destination),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
            jacobian(link(i)->e_Block_No(),link(i)->e_Block_No()) -= Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::destination),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
        }
        else
        {
            jacobian(link(i)->s_Block_No(),link(i)->e_Block_No()) -= aquiutils::Pos(-link(i)->GetVal(blocks[link(i)->e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
            jacobian(link(i)->e_Block_No(),link(i)->e_Block_No()) += aquiutils::Pos(-link(i)->GetVal(blocks[link(i)->e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
        }
    }
//#ifndef NO_OPENMP
//#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
//#endif
    for (int i=0; i<BlockCount(); i++)
    {
        if (!block(i)->GetLimitedOutflow() && !block(i)->isrigid(variable))
            jacobian(i,i) += 1/SolverTempVars.dt;
        for (unsigned int j=0; j<block(i)->Variable(variable)->GetCorrespondingInflowVar().size(); j++)
        {
            if (block(i)->Variable(variable)->GetCorrespondingInflowVar()[j] != "")
            {
                if (Variable(block(i)->Variable(variable)->GetCorrespondingInflowVar()[j]))
                {
                    if (!block(i)->GetLimitedOutflow())
                        jacobian(i,i) -= Gradient(block(i),block(i),block(i)->Variable(variable)->GetCorrespondingInflowVar()[j],variable);
                    else
                    {   double inflow = blocks[i].GetInflowValue(variable, Expression::timing::present);
                        if (inflow<0) jacobian(i,i) -= inflow;
                    }
                }
            }
        }
    }

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
        if (blocks[i].GetLimitedOutflow())
        {
            if (!OutFlowCanOccur(i,variable))
            {
                for (unsigned int j=0; j<blocks.size(); j++) jacobian(j,i)=0;
                jacobian(i,i)=1;
            }
        }
    }

    /*for (unsigned int i = 0; i < blocks.size(); i++)
    {
        if (blocks[i].GetOutflowLimitFactor(Expression::timing::present)<0 && blocks[i].GetLimitedOutflow() )
        {
            jacobian(i,i) = -2.0*blocks[i].GetOutflowLimitFactor(Expression::timing::present);
        }
    }*/

    SetStateVariables_for_direct_Jacobian(variable,current_state,Expression::timing::present,transport);
    return jacobian;
}

#ifdef SUPER_LU
CMatrix_arma_sp System::JacobianDirect_SP(const string &variable, CVector_arma &X, bool transport)
{
    for (unsigned int i=0; i<links.size(); i++)
    {
        if (blocks[links[i].s_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(), Expression::timing::present) > 0)
        {   links[i].SetOutflowLimitFactor(blocks[links[i].s_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
            links[i].SetLimitedOutflow(true);
        }
        else if (blocks[links[i].e_Block_No()].GetLimitedOutflow() && links[i].GetVal(blocks[links[i].e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present)<0)
        {   links[i].SetOutflowLimitFactor(blocks[links[i].e_Block_No()].GetOutflowLimitFactor(Expression::timing::present), Expression::timing::present);
            links[i].SetLimitedOutflow(true);
        }
        else
        {   links[i].SetOutflowLimitFactor(1, Expression::timing::present);
            links[i].SetLimitedOutflow(false);
        }

    }
    CVector_arma current_state = GetStateVariables_for_direct_Jacobian(variable,Expression::timing::present,transport);
    SetStateVariables_for_direct_Jacobian(variable,X,Expression::timing::present,transport);
    CMatrix_arma_sp jacobian_sp(BlockCount());
#ifndef NO_OPENMP
#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
#endif
    for (int i=0; i<LinksCount(); i++)
    {
        if (!link(i)->GetConnectedBlock(Expression::loc::source)->GetLimitedOutflow())
        {   jacobian_sp(link(i)->s_Block_No(),link(i)->s_Block_No()) += Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::source),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
            jacobian_sp(link(i)->e_Block_No(),link(i)->s_Block_No()) -= Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::source),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
        }
        else
        {
            jacobian_sp(link(i)->s_Block_No(),link(i)->s_Block_No()) += aquiutils::Pos(link(i)->GetVal(blocks[link(i)->s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
            jacobian_sp(link(i)->e_Block_No(),link(i)->s_Block_No()) -= aquiutils::Pos(link(i)->GetVal(blocks[link(i)->s_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
        }
        if (!link(i)->GetConnectedBlock(Expression::loc::destination)->GetLimitedOutflow())
        {
            jacobian_sp(link(i)->s_Block_No(),link(i)->e_Block_No()) += Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::destination),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
            jacobian_sp(link(i)->e_Block_No(),link(i)->e_Block_No()) -= Gradient(link(i),link(i)->GetConnectedBlock(Expression::loc::destination),Variable(variable)->GetCorrespondingFlowVar(),variable)*links[i].GetOutflowLimitFactor(Expression::timing::present);
        }
        else
        {
            jacobian_sp(link(i)->s_Block_No(),link(i)->e_Block_No()) -= aquiutils::Pos(-link(i)->GetVal(blocks[link(i)->e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
            jacobian_sp(link(i)->e_Block_No(),link(i)->e_Block_No()) += aquiutils::Pos(-link(i)->GetVal(blocks[link(i)->e_Block_No()].Variable(variable)->GetCorrespondingFlowVar(),Expression::timing::present));
        }
    }
#ifndef NO_OPENMP
#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
#endif
    for (int i=0; i<BlockCount(); i++)
    {
        if (!block(i)->GetLimitedOutflow() && !block(i)->isrigid(variable))
            jacobian_sp(i,i) += 1/SolverTempVars.dt;
        for (unsigned int j=0; j<block(i)->Variable(variable)->GetCorrespondingInflowVar().size(); j++)
        {
            if (block(i)->Variable(variable)->GetCorrespondingInflowVar()[j] != "")
            {
                if (Variable(block(i)->Variable(variable)->GetCorrespondingInflowVar()[j]))
                {
                    if (!block(i)->GetLimitedOutflow())
                        jacobian_sp(i,i) -= Gradient(block(i),block(i),block(i)->Variable(variable)->GetCorrespondingInflowVar()[j],variable);
                    else
                    {   double inflow = blocks[i].GetInflowValue(variable, Expression::timing::present);
                        if (inflow<0) jacobian_sp(i,i) -= inflow;
                    }
                }
            }
        }
    }

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
        if (blocks[i].GetLimitedOutflow())
        {
            if (!OutFlowCanOccur(i,variable))
            {
                for (unsigned int j=0; j<blocks.size(); j++) jacobian_sp(j,i)=0;
                jacobian_sp(i,i)=1;
            }
        }
    }

    /*for (unsigned int i = 0; i < blocks.size(); i++)
    {
        if (blocks[i].GetOutflowLimitFactor(Expression::timing::present)<0 && blocks[i].GetLimitedOutflow() )
        {
            jacobian(i,i) = -2.0*blocks[i].GetOutflowLimitFactor(Expression::timing::present);
        }
    }*/

    SetStateVariables_for_direct_Jacobian(variable,current_state,Expression::timing::present,transport);
    return jacobian_sp;
}
#endif
TimeSeriesSet<timeseriesprecision> System::GetModeledObjectiveFunctions()
{
    TimeSeriesSet<timeseriesprecision> out;
    for (unsigned int i=0; i<ObjectiveFunctionsCount(); i++)
    {
        out.append(*objectivefunction(objective_function_set[i]->GetName())->GetTimeSeries(),objective_function_set[i]->GetName());
    }
    return out;
}

void System::ResetAllowLimitedFlows(bool allow)
{
    for (unsigned int i=0; i<BlockCount(); i++)
        blocks[i].SetAllowLimitedFlow(allow);
}

void System::PopulateFunctionOperators()
{
    func_operators.funcs.push_back("_min");
    func_operators.funcs.push_back("_max");
    func_operators.funcs.push_back("_exp");
    func_operators.funcs.push_back("_log");
    func_operators.funcs.push_back("_abs");
    func_operators.funcs.push_back("_sgn");
    func_operators.funcs.push_back("_sqr");
    func_operators.funcs.push_back("_sqt");
    func_operators.funcs.push_back("_lpw");
    func_operators.funcs.push_back("_pos");
    func_operators.funcs.push_back("_hsd");
    func_operators.funcs.push_back("_ups");
    func_operators.funcs.push_back("_bkw");
    func_operators.funcs.push_back("_mon");
    func_operators.funcs.push_back("_mbs");
    func_operators.opts.push_back("+");
    func_operators.opts.push_back("-");
    func_operators.opts.push_back("*");
    func_operators.opts.push_back(";");
    func_operators.opts.push_back("/");
    func_operators.opts.push_back("^");
}

bool System::WriteOutPuts()
{
    Outputs.AllOutputs = Outputs.AllOutputs.make_uniform(SimulationParameters.dt0,false);
    Outputs.ObservedOutputs = Outputs.ObservedOutputs.make_uniform(SimulationParameters.dt0,false);
    if (OutputFileName() != "")
    {
        if (QString::fromStdString(OutputFileName()).contains("/") || QString::fromStdString(OutputFileName()).contains("\\"))
            if (SolverTempVars.first_write)
                GetOutputs().write(OutputFileName());
            else
                GetOutputs().appendtofile(OutputFileName());
        else
            if (SolverTempVars.first_write)
                GetOutputs().write(paths.inputpath + "/" + OutputFileName());
            else
                GetOutputs().appendtofile(paths.inputpath + "/" + OutputFileName());
    }
    if (ObservedOutputFileName() != "")
    {
        if (QString::fromStdString(ObservedOutputFileName()).contains("/") || QString::fromStdString(ObservedOutputFileName()).contains("\\"))
            if (SolverTempVars.first_write)
                GetObservedOutputs().write(ObservedOutputFileName());
            else
                GetObservedOutputs().appendtofile(ObservedOutputFileName(),true);
        else
            if (SolverTempVars.first_write)
                GetObservedOutputs().write(paths.inputpath + "/" + ObservedOutputFileName());
            else
                GetObservedOutputs().appendtofile(paths.inputpath + "/" + ObservedOutputFileName(),true);
    }

    GetOutputs().clearContentsExceptLastRow();
    GetObservedOutputs().clearContentsExceptLastRow();


    SolverTempVars.first_write = false;
    if (SolverTempVars.first_write)
        errorhandler.Write(paths.inputpath + "/errors.txt");
    return true;
}

#ifdef Q_JSON_SUPPORT
bool System::SavetoJson(const string &filename, const vector<string> &_addedtemplates, bool allvariables, bool calculatevalue)
{
    SetVariableParents();
    if (_addedtemplates.size()!=0)
        addedtemplates = _addedtemplates;

    QJsonObject out;
    QJsonArray templates;
    for (unsigned int i=0; i<addedtemplates.size();i++)
    {
        templates.append(QString::fromStdString(addedtemplates[i]));
    }
    out["Templates"] = templates;

    QJsonObject SettingsJsonObject;
    for (unsigned int i=0; i<Settings.size(); i++)
        for (unordered_map<string, Quan>::iterator j=Settings[i].GetVars()->begin(); j!=Settings[i].GetVars()->end(); j++)
            if (j->second.AskFromUser())
                SettingsJsonObject[QString::fromStdString(j->first)] = QString::fromStdString(j->second.GetProperty());
    out["Settings"] = SettingsJsonObject;

    QJsonObject SourcesJsonObject;
    for (unsigned int i=0; i<sources.size(); i++)
        SourcesJsonObject[QString::fromStdString(sources[i].GetName())] = sources[i].toJson(allvariables, calculatevalue);
    out["Sources"] = SourcesJsonObject;

    QJsonObject ParametersJsonObject;
    for (unsigned int i=0; i<ParametersCount(); i++)
        ParametersJsonObject[QString::fromStdString(Parameters()[i]->GetName())] = Parameters()[i]->toJson(allvariables);
    out["Parameters"] = ParametersJsonObject;


    QJsonObject ConstituentsJsonObject;
    for (unsigned int i=0; i<ConstituentsCount(); i++)
        ConstituentsJsonObject[QString::fromStdString(constituents[i].GetName())] = constituents[i].toJson(allvariables);
    out["Constituents"] = ConstituentsJsonObject;

    QJsonObject ReactionParametersJsonObject;
    for (unsigned int i = 0; i < ReactionParametersCount(); i++)
        ReactionParametersJsonObject[QString::fromStdString(reaction_parameters[i].GetName())] = reaction_parameters[i].toJson(allvariables);
    out["Reaction Parameters"] = ReactionParametersJsonObject;

    QJsonObject ReactionsJsonObject;
    for (unsigned int i = 0; i < ReactionsCount(); i++)
        ReactionsJsonObject[QString::fromStdString(reactions[i].GetName())] = reactions[i].toJson(allvariables);
    out["Reactions"] = ReactionsJsonObject;

    QJsonObject BlocksJsonObject;
    for (unsigned int i=0; i<blocks.size(); i++)
        BlocksJsonObject[QString::fromStdString(blocks[i].GetName())] = blocks[i].toJson(allvariables, calculatevalue);
    out["Blocks"] = BlocksJsonObject;

    QJsonObject LinksJsonObject;
    for (unsigned int i=0; i<links.size(); i++)
        LinksJsonObject[QString::fromStdString(links[i].GetName())] = links[i].toJson(allvariables, calculatevalue);
    out["Links"] = LinksJsonObject;

    QJsonObject ObjectiveFunctionsJsonObject;
    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        ObjectiveFunctionsJsonObject[QString::fromStdString(ObjectiveFunctions()[i]->GetName())] = ObjectiveFunctions()[i]->toJson(allvariables, calculatevalue);
    out["Objective Functions"] = ObjectiveFunctionsJsonObject;

    QJsonObject ObservationsJsonObject;
    for (unsigned int i = 0; i < ObservationsCount(); i++)
        ObservationsJsonObject[QString::fromStdString(observation(i)->GetName())] = observation(i)->toJson(allvariables, calculatevalue);
    out["Observations"] = ObservationsJsonObject;

    QJsonArray SetAsParametersJsonArray;
    for (unsigned int i=0; i<blocks.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, blocks[i].GetVars()->toJsonSetAsParameter());


    for (unsigned int i=0; i<links.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, links[i].GetVars()->toJsonSetAsParameter());


    for (unsigned int i = 0; i < observations.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, observations[i].GetVars()->toJsonSetAsParameter());


    for (unsigned int i = 0; i < sources.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, sources[i].GetVars()->toJsonSetAsParameter());


    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, reaction_parameters[i].GetVars()->toJsonSetAsParameter());


    for (unsigned int i = 0; i < constituents.size(); i++)
        SetAsParametersJsonArray = mergeArrays(SetAsParametersJsonArray, constituents[i].GetVars()->toJsonSetAsParameter());

    out["Set As Parameters"] = SetAsParametersJsonArray;

    QFile file(QString::fromStdString(filename));
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open file for writing:" << file.errorString();
            return false;
        }

    QJsonDocument jsonDoc(out);
    file.write(jsonDoc.toJson(QJsonDocument::Indented));  // Use Indented or Compact
    file.close();

    return true;

}

bool System::LoadfromJson(const QJsonDocument &jsondoc)
{
    QJsonObject root = jsondoc.object();
    return LoadfromJson(root);

}

bool System::LoadfromJson(const QString &jsonfilename)
{
    QFile file(jsonfilename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Couldn't open file:" << jsonfilename;
            return false;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error at offset" << parseError.offset
                       << ":" << parseError.errorString();
            return false;
        }

        LoadfromJson(doc);
        return true;

}

bool System::LoadfromJson(const QJsonObject &root)
{
    bool outcome = true;
    QJsonArray TemplatesJson = root["Templates"].toArray();
    qDebug()<<TemplatesJson;

    for (const QJsonValue& temp : TemplatesJson)
    {   if (AppendQuanTemplate(temp.toString().toStdString()))
        {
            outcome &= true;
        }
        else
        {
            if (!AppendQuanTemplate(DefaultTemplatePath() + aquiutils::GetOnlyFileName(temp.toString().toStdString())))
            {
                lasterror() = "File '" + temp.toString().toStdString() + "' was not found!";
                outcome &= false;
            }
            else
            {
                outcome &= true;

            }
        }
    }

    QJsonObject SettingsJson = root["Settings"].toObject();
    for (const QString& settingkey: SettingsJson.keys())
    {
        outcome &= SetSystemSettingsObjectProperties(settingkey.toStdString(), SettingsJson[settingkey].toString().toStdString(),false);
    }


    QJsonObject ConstituentsJson = root["Constituents"].toObject();
    for (const QString& constituentname: ConstituentsJson.keys())
    {
        QJsonObject ConstituentJson = ConstituentsJson[constituentname].toObject();
        Constituent current_constituent;
        current_constituent.SetName(ConstituentJson["name"].toString().toStdString());
        current_constituent.SetType(ConstituentJson["type"].toString().toStdString());
        AddConstituent(current_constituent);
        for (const QString &property: ConstituentJson.keys())
        {
            if (property != "type")
                if (!constituent(constituentname.toStdString())->SetProperty(property.toStdString(),ConstituentJson[property].toString().toStdString()))
                    errorhandler.Append("System", "Constituent","ReadFromJson","Constituent '" + constituentname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10015 );

        }
    }

    QJsonObject ParametersJson = root["Parameters"].toObject();
    for (const QString& parametername: ParametersJson.keys())
    {
        QJsonObject ParameterJson = ParametersJson[parametername].toObject();
        Parameter current_parameter;
        AppendParameter(parametername.toStdString(),ParameterJson["low"].toString().toDouble(),ParameterJson["high"].toString().toDouble());

        for (const QString &property: ParameterJson.keys())
        {
            if (property != "type")
                if (!parameter(parametername.toStdString())->SetProperty(property.toStdString(),ParameterJson[property].toString().toStdString(),false))
                    errorhandler.Append("System", "Parameter","ReadFromJson","Parameter '" + parametername.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10011 );
        }
    }

    QJsonObject RxnParametersJson = root["Reaction Parameters"].toObject();
    for (const QString& rxnparametername: RxnParametersJson.keys())
    {
        QJsonObject RxnParameterJson = RxnParametersJson[rxnparametername].toObject();
        RxnParameter current_rxnparameter;
        current_rxnparameter.SetName(RxnParameterJson["name"].toString().toStdString());
        current_rxnparameter.SetType(RxnParameterJson["type"].toString().toStdString());
        AddReactionParameter(current_rxnparameter);
        for (const QString &property: RxnParameterJson.keys())
        {
            if (!reactionparameter(rxnparametername.toStdString())->SetProperty(property.toStdString(),RxnParameterJson[property].toString().toStdString()))
                errorhandler.Append("System", "Reaction Parameter","ReadFromJson","Reaction parameter '" + rxnparametername.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10015 );

        }
    }

    QJsonObject RxnsJson = root["Reactions"].toObject();
    for (const QString& rxnname: RxnsJson.keys())
    {
        QJsonObject RxnJson = RxnsJson[rxnname].toObject();
        Reaction current_rxn;

        current_rxn.SetName(RxnJson["name"].toString().toStdString());
        current_rxn.SetType(RxnJson["type"].toString().toStdString());
        AddReaction(current_rxn);
        for (const QString &property: RxnJson.keys())
        {
            if (!reaction(rxnname.toStdString())->SetProperty(property.toStdString(),RxnJson[property].toString().toStdString()))
                errorhandler.Append("System", "Reaction ","ReadFromJson","Reaction  '" + rxnname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10015 );

        }
    }

    QJsonObject SourcesJson = root["Sources"].toObject();
    for (const QString& sourcename: SourcesJson.keys())
    {
        QJsonObject SourceJson = SourcesJson[sourcename].toObject();
        Source current_source;
        current_source.SetName(SourceJson["name"].toString().toStdString());
        current_source.SetType(SourceJson["type"].toString().toStdString());
        AddSource(current_source);

        for (const QString &property: SourceJson.keys())
        {
            if (property != "type")
                if (!source(sourcename.toStdString())->SetProperty(property.toStdString(),SourceJson[property].toString().toStdString(),true, false))
                        errorhandler.Append("System", "Source","ReadFromJson","Source '" + sourcename.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10012 );

        }
    }


    QJsonObject BlocksJson = root["Blocks"].toObject();
    for (const QString& blockname: BlocksJson.keys())
    {

        QJsonObject BlockJson = BlocksJson[blockname].toObject();
        Block current_block;
        current_block.SetName(BlockJson["name"].toString().toStdString());
        current_block.SetType(BlockJson["type"].toString().toStdString());
        AddBlock(current_block);
        for (const QString &property: BlockJson.keys())
        {
            if (property!="type" && property!="to" && property!="from")
            {
                if (!block(blockname.toStdString())->SetProperty(property.toStdString(),BlockJson[property].toString().toStdString(),true, false))
                {
                    qDebug() << blockname << ":" << property << ":"
                             << BlockJson[property].toString();
                    errorhandler.Append("System", "Block","ReadFromJson","Block '" + blockname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10012 );

                }
            }
        }
    }

    QJsonObject LinksJson = root["Links"].toObject();
    for (const QString& linkname: LinksJson.keys())
    {
        QJsonObject LinkJson = LinksJson[linkname].toObject();
        Link current_link;
        current_link.SetName(LinkJson["name"].toString().toStdString());
        current_link.SetType(LinkJson["type"].toString().toStdString());
        AddLink(current_link, LinkJson["from"].toString().toStdString(),LinkJson["to"].toString().toStdString());

        for (const QString &property: LinkJson.keys())
        {
            if (property!="type" && property!="to" && property!="from")
            {
                qDebug()<<linkname;
                if (!link(linkname.toStdString())->SetProperty(property.toStdString(),LinkJson[property].toString().toStdString(),true, false))
                    errorhandler.Append("System", "Link","ReadFromJson","Link '" + linkname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10013 );
            }
        }
    }

    QJsonObject ObjectiveFunctionsJson = root["Objective Functions"].toObject();
    for (const QString& objectivefunctionname: ObjectiveFunctionsJson.keys())
    {
        QJsonObject ObjectiveJson = ObjectiveFunctionsJson[objectivefunctionname].toObject();
        AppendObjectiveFunction( objectivefunctionname.toStdString(),ObjectiveJson["object"].toString().toStdString(),Expression(ObjectiveJson["expression"].toString().toStdString()), ObjectiveJson["weight"].toString().toDouble());
        for (const QString &property: ObjectiveJson.keys())
        {
            if (property != "type")
                if (!objectivefunction(objectivefunctionname.toStdString())->SetProperty(property.toStdString(),ObjectiveJson[property].toString().toStdString()))
                    errorhandler.Append("System", "Objective function","ReadFromJson","Objective Function '" + objectivefunctionname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10014 );

        }
    }

    QJsonObject ObservationsJson = root["Observations"].toObject();

    qDebug() << "Observations JSON:" << root["Observations"];
    qDebug() << "Root keys:" << root.keys();
    for (const QString& observationname: ObservationsJson.keys())
    {
        QJsonObject ObservationJson = ObservationsJson[observationname].toObject();
        Observation current_observation;
        current_observation.SetName(ObservationJson["name"].toString().toStdString());
        current_observation.SetType(ObservationJson["type"].toString().toStdString());
        AddObservation(current_observation);
        for (const QString &property: ObservationJson.keys())
        {
            if (property != "type")
                if (!observation(observationname.toStdString())->SetProperty(property.toStdString(),ObservationJson[property].toString().toStdString()))
                    errorhandler.Append("System", "Observation","ReadFromJson","Observation '" + observationname.toStdString() + "' does not have a propery '" + property.toStdString() + "'",10015 );

        }
    }



    QJsonArray SetAsParameterJsonArray = root["Set As Parameters"].toArray();
    for (const QJsonValue& SetAsParam : SetAsParameterJsonArray)
    {
        string s_object = SetAsParam.toObject()["Object"].toString().toStdString();
        string s_parameter = SetAsParam.toObject()["Parameter"].toString().toStdString();
        string s_quantity = SetAsParam.toObject()["Quantity"].toString().toStdString();
        SetAsParameter(s_object, s_quantity, s_parameter);
        if (object(s_object))
        {
            if (object(s_object)->HasQuantity(s_quantity))
            {
                object(s_object)->Variable(s_quantity)->SetParameterAssignedTo(s_parameter);
            }
        }
    }

    SetAllParents();

    return outcome;
}

bool System::SaveStateVariableToJson(const string &variable, const string &filename)
{
    QJsonObject out;

    for (unsigned int i=0; i<blocks.size(); i++)
    {
        if (blocks[i].HasQuantity(variable))
        {
            out[QString::fromStdString(blocks[i].GetName())] = blocks[i].Variable(variable)->GetVal();
        }
    }

    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << file.errorString();
        return false;
    }
    QJsonDocument jsonDoc(out);
    file.write(jsonDoc.toJson(QJsonDocument::Indented));  // Use Indented or Compact
    file.close();
    return true;
}

bool System::LoadStateVariableFromJson(const string &variable, const string &filename)
{
    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Couldn't open file:" << QString::fromStdString(filename);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error at offset" << parseError.offset
                   << ":" << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    for (const QString& key: root.keys())
    {
        if (block(key.toStdString()))
        {
            if (block(key.toStdString())->HasQuantity(variable))
                block(key.toStdString())->SetVal(variable,root[key].toDouble());
        }
    }
    return true;
}


#endif


void System::Translate(double dx, double dy)
{
    // Translate all blocks
    for (unsigned int i = 0; i < BlockCount(); i++)
    {
        Block* blk = block(i);
        if (blk)
        {
            // Check if the block has x and y properties
            if (blk->HasQuantity("x"))
            {
                double currentX = blk->GetVal("x");
                blk->Variable("x")->SetVal(currentX + dx);
            }

            if (blk->HasQuantity("y"))
            {
                double currentY = blk->GetVal("y");
                blk->Variable("y")->SetVal(currentY + dy);
            }
        }
    }
}

vector<pair<string, string>> System::GetOutputProperties()
{
    // Use a map to store unique properties with their descriptions
    // The key is the quan name, value is the description
    map<string, string> uniqueProperties;

    // Iterate through all blocks
    for (unsigned int i = 0; i < blocks.size(); i++)
    {
        // Get the variables for this block
        QuanSet* vars = blocks[i].GetVars();
        if (vars)
        {
            // Iterate through all quantities in this block
            for (unordered_map<string, Quan>::iterator it = vars->begin();
                 it != vars->end(); ++it)
            {
                // Check if this quantity should be included in output
                if (it->second.IncludeInOutput())
                {
                    // Add to map (automatically handles uniqueness)
                    uniqueProperties[it->first] = it->second.Description(true);
                }
            }
        }
    }

    // Iterate through all links
    for (unsigned int i = 0; i < links.size(); i++)
    {
        // Get the variables for this link
        QuanSet* vars = links[i].GetVars();
        if (vars)
        {
            // Iterate through all quantities in this link
            for (unordered_map<string, Quan>::iterator it = vars->begin();
                 it != vars->end(); ++it)
            {
                // Check if this quantity should be included in output
                if (it->second.IncludeInOutput())
                {
                    // Add to map (automatically handles uniqueness)
                    uniqueProperties[it->first] = it->second.Description();
                }
            }
        }
    }

    // Convert map to vector of pairs
    vector<pair<string, string>> result;
    for (map<string, string>::iterator it = uniqueProperties.begin();
         it != uniqueProperties.end(); ++it)
    {
        result.push_back(make_pair(it->first, it->second));
    }

    return result;
}

#ifdef Q_JSON_SUPPORT
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

QJsonObject System::toJsonObjectFull() const
{
    QJsonObject json;

    // Solver state
    QJsonObject solver;
    solver["t"] = SolverTempVars.t;
    solver["dt"] = SolverTempVars.dt;
    solver["dt_base"] = SolverTempVars.dt_base;
    solver["epoch_count"] = SolverTempVars.epoch_count;
    solver["SolutionFailed"] = SolverTempVars.SolutionFailed;
    //solver["simulation_duration"] = SolverTempVars.simulation_duration;
    QJsonArray nr_coeff;
    for (unsigned int i = 0; i < SolverTempVars.NR_coefficient.size(); i++)
        nr_coeff.append(SolverTempVars.NR_coefficient[i]);
    solver["NR_coefficient"] = nr_coeff;
    QJsonArray update_jac;
    for (unsigned int i = 0; i < SolverTempVars.updatejacobian.size(); i++)
        update_jac.append(static_cast<bool>(SolverTempVars.updatejacobian[i]));
    solver["updatejacobian"] = update_jac;
    QJsonArray num_iter;
    for (unsigned int i = 0; i < SolverTempVars.numiterations.size(); i++)
        num_iter.append(static_cast<int>(SolverTempVars.numiterations[i]));
    solver["numiterations"] = num_iter;
    QJsonArray fail_reasons;
    for (unsigned int i = 0; i < SolverTempVars.fail_reason.size(); i++)
        fail_reasons.append(QString::fromStdString(SolverTempVars.fail_reason[i]));
    solver["fail_reasons"] = fail_reasons;
    json["solver_state"] = solver;

    // Solver settings
    QJsonObject settings;
    settings["n_threads"] = SolverSettings.n_threads;
    settings["NRtolerance"] = SolverSettings.NRtolerance;
    settings["C_N_weight"] = SolverSettings.C_N_weight;
    settings["direct_jacobian"] = SolverSettings.direct_jacobian;
    settings["scalediagonal"] = SolverSettings.scalediagonal;
    settings["optimize_lambda"] = SolverSettings.optimize_lambda;
    settings["RecordAllOutputs"] = SolverSettings.RecordAllOutputs;
    //settings["maxiteration"] = SolverSettings.maxmaxiteration;
    settings["n_threads"] = SolverSettings.n_threads;
    settings["landtozero_factor"] = SolverSettings.landtozero_factor;
    settings["NR_coeff_reduction_factor"] = SolverSettings.NR_coeff_reduction_factor;
    json["solver_settings"] = settings;

    // Blocks
    QJsonObject blocksJson;
    for (unsigned int i = 0; i < blocks.size(); i++)
        blocksJson[QString::fromStdString(blocks.at(i).GetName())] = blocks.at(i).toJsonObjectFull();
    json["blocks"] = blocksJson;

    // Links
    QJsonObject linksJson;
    for (unsigned int i = 0; i < links.size(); i++)
        linksJson[QString::fromStdString(links.at(i).GetName())] = links.at(i).toJsonObjectFull();
    json["links"] = linksJson;

    // Sources
    QJsonObject sourcesJson;
    for (unsigned int i = 0; i < sources.size(); i++)
        sourcesJson[QString::fromStdString(sources.at(i).GetName())] = sources.at(i).toJsonObjectFull();
    json["sources"] = sourcesJson;

    // Constituents
    QJsonObject constituentsJson;
    for (unsigned int i = 0; i < constituents.size(); i++)
        constituentsJson[QString::fromStdString(constituents.at(i).GetName())] = constituents.at(i).toJsonObjectFull();
    json["constituents"] = constituentsJson;

    // Reactions
    QJsonObject reactionsJson;
    for (unsigned int i = 0; i < reactions.size(); i++)
        reactionsJson[QString::fromStdString(reactions.at(i).GetName())] = reactions.at(i).toJsonObjectFull();
    json["reactions"] = reactionsJson;

    // Reaction parameters
    QJsonObject rxnParamsJson;
    for (unsigned int i = 0; i < reaction_parameters.size(); i++)
        rxnParamsJson[QString::fromStdString(reaction_parameters.at(i).GetName())] = reaction_parameters.at(i).toJsonObjectFull();
    json["reaction_parameters"] = rxnParamsJson;

    // Observations
    QJsonObject obsJson;
    for (unsigned int i = 0; i < observations.size(); i++)
        obsJson[QString::fromStdString(observations.at(i).GetName())] = observations.at(i).toJsonObjectFull();
    json["observations"] = obsJson;

    // Parameters
    QJsonObject paramsJson;
    for (unsigned int i = 0; i < ParametersCount(); i++)
        paramsJson[QString::fromStdString(parameter_set[i]->GetName())] = parameter_set[i]->toJsonObjectFull();
    json["parameters"] = paramsJson;

    // Objective functions
    QJsonObject objFuncJson;
    for (unsigned int i = 0; i < ObjectiveFunctionsCount(); i++)
        objFuncJson[QString::fromStdString(objective_function_set[i]->GetName())] = objective_function_set[i]->toJsonObjectFull();
    json["objective_functions"] = objFuncJson;

    // Settings objects
    QJsonObject settingsObjJson;
    for (unsigned int i = 0; i < Settings.size(); i++)
        settingsObjJson[QString::fromStdString(Settings.at(i).GetName())] = Settings.at(i).toJsonObjectFull();
    json["settings_objects"] = settingsObjJson;

    // Added properties
    QJsonObject addedBlockProps;
    for (auto it = addedpropertiestoallblocks.begin(); it != addedpropertiestoallblocks.end(); ++it)
        addedBlockProps[QString::fromStdString(it->first)] = it->second.toJsonObject();
    json["addedpropertiestoallblocks"] = addedBlockProps;

    QJsonObject addedLinkProps;
    for (auto it = addedpropertiestoalllinks.begin(); it != addedpropertiestoalllinks.end(); ++it)
        addedLinkProps[QString::fromStdString(it->first)] = it->second.toJsonObject();
    json["addedpropertiestoalllinks"] = addedLinkProps;

    // Added templates
    QJsonArray templatesArr;
    for (const auto& t : addedtemplates)
        templatesArr.append(QString::fromStdString(t));
    json["addedtemplates"] = templatesArr;

    return json;
}

bool System::SaveFullStateTo(const QString &filename) const
{
    QJsonDocument doc(toJsonObjectFull());
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

void System::CalcAllExpressions(const Expression::timing &tmg, bool force_all, bool dolinks)
{
#ifndef NO_OPENMP
#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
#endif
    for (int i = 0; i < blocks.size(); i++)
        blocks[i].CalcExpressions(tmg, force_all);

    if (dolinks)
    {
#ifndef NO_OPENMP
#pragma omp parallel for schedule(static) if (SolverSettings.n_threads>1)
#endif
        for (int i = 0; i < links.size(); i++)
            links[i].CalcExpressions(tmg, force_all);
    }
}

void System::LogJacobianFailure(const CMatrix_arma &J, bool transport)
{
    auto logger = GetSolutionLogger();
    if (!logger) return;

    logger->WriteString("==============================================");
    logger->WriteString("Jacobian inversion failed");
    logger->WriteString("Time: " + aquiutils::numbertostring(SolverTempVars.t));
    if (transport) logger->WriteString("Context: Transport");
    logger->WriteString("----------------------------------------------");

    logger->WriteString("Jacobian diagonal vector:");
    logger->WriteVector(J.diagvector());

    auto zeros = J.diagvector().lookup(0);
    if (!zeros.empty())
    {
        logger->WriteString("Blocks/Constituents with ZERO diagonal:");
        for (unsigned int j = 0; j < zeros.size(); j++)
        {
            if (transport)
                logger->WriteString("  - " + GetBlockConstituentSring(zeros[j]));
            else
                logger->WriteString("  - " + blocks[zeros[j]].GetName());
        }
    }

    std::vector<unsigned int> negatives;
    for (unsigned int i = 0; i < J.diagvector().size(); i++)
        if (J.diagvector()[i] < 0)
            negatives.push_back(i);

    if (!negatives.empty())
    {
        logger->WriteString("Blocks/Constituents with NEGATIVE diagonal:");
        for (unsigned int j = 0; j < negatives.size(); j++)
        {
            if (transport)
                logger->WriteString("  - " + GetBlockConstituentSring(negatives[j]) +
                                    " (diag = " + aquiutils::numbertostring(J.diagvector()[negatives[j]]) + ")");
            else
                logger->WriteString("  - " + blocks[negatives[j]].GetName() +
                                    " (diag = " + aquiutils::numbertostring(J.diagvector()[negatives[j]]) + ")");
        }
    }

    if (J.diagvector().size() > 0)
    {
        unsigned int max_idx = J.diagvector().abs_max_elems();
        logger->WriteString("Block/Constituent with MAX ABSOLUTE diagonal:");
        if (transport)
            logger->WriteString("  - " + GetBlockConstituentSring(max_idx) +
                                " (diag = " + aquiutils::numbertostring(J.diagvector()[max_idx]) + ")");
        else
            logger->WriteString("  - " + blocks[max_idx].GetName() +
                                " (diag = " + aquiutils::numbertostring(J.diagvector()[max_idx]) + ")");
    }

    logger->WriteString("==============================================");
    logger->Flush();
}

void System::LogErrorIncrease(double err_p, double err, bool transport, int ini_max_error_block)
{
    auto logger = GetSolutionLogger();
    if (!logger) return;

    logger->WriteString("==============================================");
    logger->WriteString("Optimize Lambda: " + aquiutils::numbertostring(SolverSettings.optimize_lambda));
    logger->WriteString("Error increased at time: " + aquiutils::numbertostring(SolverTempVars.t));
    logger->WriteString("Previous error: " + aquiutils::numbertostring(err_p) +
                        ", Current error: " + aquiutils::numbertostring(err));
    logger->WriteString("----------------------------------------------");

    if (transport)
    {
        logger->WriteString("Block/Constituent with initial MAX error:");
        logger->WriteString("  - " + GetBlockConstituentSring(ini_max_error_block));
    }
    else
    {
        logger->WriteString("Block with initial MAX error:");
        logger->WriteString("  - " + blocks[ini_max_error_block].GetName());
    }

    CVector outflow_vec = GetOutflowLimitFactorVector(Expression::timing::present);
    logger->WriteString("Outflow limit factor vector: " + outflow_vec.toString());

    std::vector<unsigned int> invalid_idx;
    for (unsigned int i = 0; i < outflow_vec.size(); i++)
        if (outflow_vec[i] < 0.0 || outflow_vec[i] > 1.0)
            invalid_idx.push_back(i);

    if (!invalid_idx.empty())
    {
        logger->WriteString("Invalid outflow limit factors (outside [0,1]):");
        for (unsigned int idx : invalid_idx)
        {
            std::string name = transport ? GetBlockConstituentSring(idx) : blocks[idx].GetName();
            logger->WriteString("  - " + name + " (value = " + aquiutils::numbertostring(outflow_vec[idx]) + ")");
        }
    }

    logger->WriteString("==============================================");
    logger->Flush();
}

void System::LogErrorKeptIncreasing(const CVector_arma &F, bool transport, unsigned int statevarno)
{
    auto logger = GetSolutionLogger();
    if (!logger) return;

    logger->WriteString("==============================================");
    logger->WriteString("Solver failure: Error kept increasing");
    logger->WriteString("Time: " + aquiutils::numbertostring(SolverTempVars.t));
    logger->WriteString("State variable: " + aquiutils::numbertostring(statevarno));
    logger->WriteString("----------------------------------------------");

    logger->WriteString("Outflow limit vector: " +
                        CVector(GetOutflowLimitFactorVector(Expression::timing::present)).toString());

    logger->WriteString("Maximum error location:");
    if (transport)
        logger->WriteString("  - " + GetBlockConstituentSring(F.abs_max_elems()));
    else
        logger->WriteString("  - " + blocks[F.abs_max_elems()].GetName());

    logger->WriteString("Time step (dt): " + aquiutils::numbertostring(dt()));

    logger->WriteString("==============================================");
    logger->Flush();
}

void System::LogIterationLimitExceeded(const CVector_arma &F, const CVector_arma &X, const CMatrix_arma &InvJ,
                                       double err, double err_ini, double X_norm, bool transport,
                                       unsigned int statevarno, int ini_max_error_block)
{
    auto logger = GetSolutionLogger();
    if (!logger) return;

    logger->WriteString("==============================================");
    logger->WriteString("Solver failure: Iteration limit exceeded");
    logger->WriteString("Time: " + aquiutils::numbertostring(SolverTempVars.t));
    logger->WriteString("State variable: " + aquiutils::numbertostring(statevarno));
    logger->WriteString("Iteration count: " +
                        aquiutils::numbertostring(SolverTempVars.numiterations[statevarno]));
    logger->WriteString("----------------------------------------------");

    logger->WriteString("Maximum error location:");
    if (transport)
        logger->WriteString("  - " + GetBlockConstituentSring(F.abs_max_elems()));
    else
        logger->WriteString("  - " + blocks[F.abs_max_elems()].GetName());

    if (!transport)
    {
        logger->WriteString("Block with initial MAX error:");
        logger->WriteString("  - " + blocks[ini_max_error_block].GetName());
    }

    logger->WriteString("Time step (dt): " + aquiutils::numbertostring(dt()));
    logger->WriteString("Error criteria number: " +
                        aquiutils::numbertostring(err / (err_ini + 1e-10 * X_norm)));
    logger->WriteString("X_norm: " + aquiutils::numbertostring(X_norm));

    logger->WriteString("Block outflow factors: " +
                        GetBlocksOutflowFactors(Expression::timing::present).toString());
    logger->WriteString("Link outflow factors: " +
                        GetLinkssOutflowFactors(Expression::timing::present).toString());

    logger->WriteString("Error vector (F):");
    logger->WriteVector(F);
    logger->WriteString("Error norm = " + aquiutils::numbertostring(err) +
                        ", Initial error norm = " + aquiutils::numbertostring(err_ini));

    logger->WriteString("Inverse Jacobian:");
    logger->WriteMatrix(InvJ);

    logger->WriteString("==============================================");
    logger->Flush();
}

bool System::ComputeNewtonStep(const string &variable, CVector_arma &X, CVector_arma &X1,
                               CVector_arma &dx, const CVector_arma &F,
                               unsigned int statevarno, bool transport,
                               const vector<bool> &outflowlimitstatus_old)
{
    if (SolverTempVars.updatejacobian[statevarno])
    {
        CMatrix_arma J;
        if (transport)
            J = Jacobian(variable, X, transport);
        else
            J = JacobianDirect(variable, X, transport);

        SolverTempVars.epoch_count++;

        if (SolverSettings.scalediagonal)
            J.ScaleDiagonal(1.0 / SolverTempVars.NR_coefficient[statevarno]);


        if (!SolverSettings.direct_jacobian)
        {
            if (!Invert(J, SolverTempVars.Inverse_Jacobian[statevarno]))
            {
                LogJacobianFailure(J, transport);
                SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) +
                                                     ": The Jacobian Matrix is not full-ranked");
                if (!transport)
                    SetOutflowLimitedVector(outflowlimitstatus_old);
                return false;
            }
        }
        else
            SolverTempVars.Inverse_Jacobian[statevarno] = J;

        SolverTempVars.updatejacobian[statevarno] = false;
    }

    // Compute Newton step dx and update X

    if (SolverSettings.scalediagonal)
    {
        if (!SolverSettings.direct_jacobian)
        {
            dx = SolverTempVars.Inverse_Jacobian[statevarno] * F;
            X -= dx;
        }
        else
        {
            dx = F / SolverTempVars.Inverse_Jacobian[statevarno];
            if (dx.size() != X.size())
            {
                if (GetSolutionLogger())
                {
                    GetSolutionLogger()->WriteString("Jacobian matrix is singular");
                    GetSolutionLogger()->Flush();
                }
                SolverTempVars.fail_reason.push_back("at " + aquiutils::numbertostring(SolverTempVars.t) +
                                                     ": The Jacobian Matrix is not full-ranked");
                if (!transport) SetOutflowLimitedVector(outflowlimitstatus_old);
                return false;
            }
            X -= dx;
        }
    }
    else
    {
        if (!SolverSettings.direct_jacobian)
        {
            dx = SolverTempVars.NR_coefficient[statevarno] * SolverTempVars.Inverse_Jacobian[statevarno] * F;
            X -= dx;
        }
        else
        {
            dx = SolverTempVars.NR_coefficient[statevarno] * (F / SolverTempVars.Inverse_Jacobian[statevarno]);
            X -= dx;
        }
        if (SolverSettings.optimize_lambda)
        {
            X1 = X + 0.5 * dx;
        }
    }

    return true;
}

System::NRAdjustResult System::AdjustNRCoefficient(
    CVector_arma &X, const CVector_arma &X_past,
    const CVector_arma &X1, const CVector_arma &F,
    const CVector_arma &F1,
    double err, double &err_p,
    unsigned int statevarno, bool transport,
    int ini_max_error_block,
    double &error_increase_counter,
    const vector<bool> &outflowlimitstatus_old)
{
    if (SolverSettings.optimize_lambda)
    {
        double err2 = F1.norm2();
        if (err2 < err)
        {
            SolverTempVars.NR_coefficient[statevarno] = max(SolverTempVars.NR_coefficient[statevarno] / 2.0, 0.05);
            SolverTempVars.updatejacobian[statevarno] = true;
            X = X1;
        }
        else
        {
            SolverTempVars.NR_coefficient[statevarno] = max(min(SolverTempVars.NR_coefficient[statevarno] * 1.25, 1.0), 0.05);
        }
        if (min(err2, err) > err_p)
        {
            error_increase_counter++;
            LogErrorIncrease(err_p, err, transport, ini_max_error_block);
        }
    }
    else
    {
        if (err > err_p * 0.9)
        {
            SolverTempVars.NR_coefficient[statevarno] = max(SolverTempVars.NR_coefficient[statevarno] * SolverSettings.NR_coeff_reduction_factor, 0.05);
            SolverTempVars.updatejacobian[statevarno] = true;
            X = X_past;
        }
        if (err > err_p)
        {
            error_increase_counter++;
            LogErrorIncrease(err_p, err, transport, ini_max_error_block);
        }
        else if (err < err_p / 2.0)
        {
            if (SolverTempVars.NR_coefficient[statevarno] < 0.99)
                SolverTempVars.updatejacobian[statevarno] = true;
            SolverTempVars.NR_coefficient[statevarno] = max(min(SolverTempVars.NR_coefficient[statevarno] / SolverSettings.NR_coeff_reduction_factor, 1.0), 0.05);
        }
    }

    if (error_increase_counter > 10)
    {
        SolverTempVars.fail_reason.push_back(
            "at " + aquiutils::numbertostring(SolverTempVars.t) +
            ": Error kept increasing, state_variable: " +
            aquiutils::numbertostring(statevarno));
        LogErrorKeptIncreasing(F, transport, statevarno);
        if (!transport) SetOutflowLimitedVector(outflowlimitstatus_old);
        return NRAdjustResult::failed;
    }

    if (SolverTempVars.numiterations[statevarno] > SolverSettings.NR_niteration_max)
    {
        SolverTempVars.fail_reason.push_back(
            "at " + aquiutils::numbertostring(SolverTempVars.t) +
            ": Number of iterations exceeded the maximum threshold, state_variable: " +
            aquiutils::numbertostring(statevarno));
        LogIterationLimitExceeded(F, X, SolverTempVars.Inverse_Jacobian[statevarno],
                                  err, err_p, X.norm2(), transport, statevarno, ini_max_error_block);
        if (!transport) SetOutflowLimitedVector(outflowlimitstatus_old);
        return NRAdjustResult::failed;
    }

    return NRAdjustResult::ok;
}

void System::InitializeOneStep(const string &variable, unsigned int statevarno, bool transport,
                               vector<bool> &outflowlimitstatus_old)
{
    Renew(variable);
    if (!transport)
    {
        for (unsigned int i = 0; i < links.size(); i++)
            links[i].SetOutflowLimitFactor(links[i].GetOutflowLimitFactor(Expression::timing::past), Expression::timing::present);
        for (unsigned int i = 0; i < blocks.size(); i++)
            blocks[i].SetOutflowLimitFactor(blocks[i].GetOutflowLimitFactor(Expression::timing::past), Expression::timing::present);
    }
    SolverTempVars.numiterations[statevarno] = 0;
    if (!transport)
        outflowlimitstatus_old = GetOutflowLimitedVector();

    if (!transport)
    {
        for (unsigned int i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].GetLimitedOutflow())
                blocks[i].SetOutflowLimitFactor(blocks[i].GetOutflowLimitFactor(Expression::timing::past), Expression::timing::present);
        }
    }
    ResetAllowLimitedFlows(true);
}
#endif
