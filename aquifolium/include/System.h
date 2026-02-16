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

#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Source.h"
#include "constituent.h"
#include "Vector_arma.h"
#include "Matrix_arma.h"
#include "MetaModel.h"
#include "TimeSeriesSet.h"
#include "Objective_Function_Set.h"
#include "observation.h"
#include "Parameter_Set.h"
#include "reaction.h"
#include "RxnParameter.h"
#include "solutionlogger.h"
#include "ErrorHandler.h"
#include "safevector.h"
#include <string>
#include <omp.h>
#define outputtimeseriesprecision double
#if Q_GUI_SUPPORT
#include <QStringList>
#include "ProgressWindow.h"
#include "QTime"
#include "logwindow.h"
#endif

 // std:: qualifiers used explicitly — no 'using namespace std' in headers

#ifdef SUPER_LU
#include "Matrix_arma_sp.h"
#endif


#ifdef DEBUG
#define CVector_arma CVector
#define CMatrix_arma CMatrix
#endif // _DEBUG

class Script;
class RestorePoint;

enum class parameter_estimation_options { none, optimize, inverse_model };

struct solversettings
{
    double C_N_weight; //Crank-Nicholson Weight
    double NRtolerance = 1e-6; //Newton Raphson Tolerance
    int n_threads = 16; //Number of threads
    double NR_coeff_reduction_factor = 0.8; //The coefficient to reduce the Newton-Raphson coefficient
    double NR_timestep_reduction_factor = 0.75;
    double NR_timestep_reduction_factor_fail = 0.3;
    double minimum_timestep = 1e-7;
    int NR_niteration_lower = 20;
    int NR_niteration_upper = 40;
    int NR_niteration_max = 100;
    bool makeresultsuniform = false;
    bool scalediagonal = false;
    double landtozero_factor = 0;
    bool optimize_lambda = true;
    bool direct_jacobian = false;
    bool write_solution_details = false;
    double maximum_simulation_time = 86400; //maximum simulation time allows in seconds
    int maximum_number_of_matrix_inversions = 200000; //maximum number of matrix inversions allowed
    bool RecordAllOutputs = true; //whether the results will be all recorded on not
    double timestepminfactor = 100000; // the maximum the timestep can decrease by
    double timestepmaxfactor = 50; // the maximum the timestep can increase by

};

struct function_operators
{
    std::vector<std::string> funcs;
    std::vector<std::string> opts;

};

struct outputs
{
    TimeSeriesSet<outputtimeseriesprecision> AllOutputs;
    TimeSeriesSet<timeseriesprecision> ObservedOutputs;
};

struct solvertemporaryvars
{
#ifdef SUPER_LU
    std::vector<CMatrix_arma_sp> Inverse_Jacobian;
#else
    std::vector<CMatrix_arma> Inverse_Jacobian;
#endif
    std::vector<double> NR_coefficient;
    std::vector<bool> updatejacobian;
    int MaxNumberOfIterations()
    {
        return aquiutils::Max(numiterations);
    }

    void SetUpdateJacobian(bool x)
    {
        for (unsigned int i = 0; i < updatejacobian.size(); i++)
            updatejacobian[i] = x;
    }
    std::vector<int> numiterations;
    int numiteration_tr;
    int epoch_count = 0;
    std::vector<std::string> fail_reason;
    double t;
    double dt;
    double dt_base;
    bool SolutionFailed = false;
    CVector_arma Jacobia_Diagonal;
    time_t time_start; //simulation start time
    time_t simulation_duration = 0; //simulation duration
    bool first_write = true; //Has the results been written or not

};

struct simulationparameters
{
    double tstart = 0; //start time of simulation
    double tend = 1; //end time of simulation
    double dt0 = 0.01; // initial time-step size
    double write_interval = 1000; // the interval at which the outputs are written
    bool write_outputs_intermittently = false; //whether to write the outputs intermittently or not
};

struct _directories
{
    std::string inputpath;
    std::string outputpath;
    std::string default_template_path;
    std::string outputfilename = "output.txt";
    std::string observedoutputfilename = "observedoutput.txt";
};

class System : public Object
{
public:

    // =====================================================================
    // Construction, destruction, and assignment
    // =====================================================================
    System();
    virtual ~System();
    void CopyFrom(const System& other);
    System(const System& other);
    System(Script& scr);
    System& operator=(const System& other);
    bool CreateFromScript(Script& scr, const std::string& settingsfilename = "");
    void Clear();
    void clear();

    // =====================================================================
    // Entity addition
    // =====================================================================
    bool AddBlock(Block& blk, bool SetQuantities = true);
    bool AddSource(Source& src, bool SetQuantities = true);
    bool AddLink(Link& lnk, const std::string& source, const std::string& destination, bool SetQuantities = true);
    bool AddConstituent(Constituent& cnst, bool SetQuantities = true);
    bool AddReaction(Reaction& rxn, bool SetQuantities = true);
    bool AddObservation(Observation& obs, bool SetQuantities = true);
    bool AddReactionParameter(RxnParameter& rxn, bool SetQuantities = true);
    bool Delete(const std::string& objectname);

    // =====================================================================
    // Entity access by name
    // =====================================================================
    Block* block(const std::string& s);
    const Block* block(const string& s) const;
    Link* link(const std::string& s);
    const Link* link(const string& s) const;
    Source* source(const std::string& s);
    const Source* source(const string& s) const;
    Constituent* constituent(const std::string& s);
    const Constituent* constituent(const string& s) const;
    Reaction* reaction(const std::string& s);
    const Reaction* reaction(const string& s) const;
    RxnParameter* reactionparameter(const std::string& s);
    const RxnParameter* reactionparameter(const string& s) const;
    Observation* observation(const std::string& s);
    const Observation* observation(const string& s) const;
    Parameter* parameter(const std::string& s);
    const Parameter* parameter(const string& s) const;
    Objective_Function* objectivefunction(const std::string& s);
    const Objective_Function* objectivefunction(const string& s) const;
    Object* object(const std::string& s);
    const Object* object(const string& s) const;
    Object* settings(const std::string& s);
    const Object* settings(const string& s) const;
    int blockid(const std::string& s);
    int linkid(const std::string& s);
    int blockid(const string& s) const;
    int linkid(const string& s) const;

    // =====================================================================
    // Entity access by index
    // =====================================================================
    Block* block(unsigned int i)
    {
        if (i < blocks.size()) return &blocks[i];
        return nullptr;
    }

    const Block* block(unsigned int i) const
    {
        if (i < blocks.size()) return &blocks[i];
        return nullptr;
    }

    Link* link(unsigned int i)
    {
        if (i < links.size()) return &links[i];
        return nullptr;
    }

    const Link* link(unsigned int i) const
    {
        if (i < links.size()) return &links[i];
        return nullptr;
    }

    Source* source(unsigned int i)
    {
        if (i < sources.size()) return &sources[i];
        return nullptr;
    }

    const Source* source(unsigned int i) const
    {
        if (i < sources.size()) return &sources[i];
        return nullptr;
    }


    Constituent* constituent(unsigned int i)
    {
        if (i < constituents.size()) return &constituents[i];
        return nullptr;
    }

    const Constituent* constituent(unsigned int i) const
    {
        if (i < constituents.size()) return &constituents[i];
        return nullptr;
    }

    Reaction* reaction(unsigned int i)
    {
        if (i < reactions.size()) return &reactions[i];
        return nullptr;
    }

    const Reaction* reaction(unsigned int i) const
    {
        if (i < reactions.size()) return &reactions[i];
        return nullptr;
    }

    RxnParameter* reactionparameter(unsigned int i)
    {
        if (i < reaction_parameters.size()) return &reaction_parameters[i];
        return nullptr;
    }

    const RxnParameter* reactionparameter(unsigned int i) const
    {
        if (i < reaction_parameters.size()) return &reaction_parameters[i];
        return nullptr;
    }

    Observation* observation(unsigned int i)
    {
        if (i < observations.size()) return &observations[i];
        return nullptr;
    }

    const Observation* observation(unsigned int i) const
    {
        if (i < observations.size()) return &observations[i];
        return nullptr;
    }

    Objective_Function* objectivefunction(unsigned int i)
    {
        if (i < objective_function_set.size()) return objective_function_set[i];
        return nullptr;
    }

    const Objective_Function* objectivefunction(unsigned int i) const
    {
        if (i < objective_function_set.size()) return objective_function_set[i];
        return nullptr;
    }

    Object* Setting(unsigned int i)
    {
        if (i < Settings.size()) return &Settings[i];
        return nullptr;
    }

    // =====================================================================
    // Entity counts
    //   [CHANGE] Add const to all count methods
    // =====================================================================
    unsigned int BlockCount() const { return blocks.size(); }
    unsigned int LinksCount() const { return links.size(); }
    unsigned int SourcesCount() const { return sources.size(); }
    unsigned int ConstituentsCount() const { return constituents.size(); }
    unsigned int ReactionsCount() const { return reactions.size(); }
    unsigned int ReactionParametersCount() const { return reaction_parameters.size(); }
    unsigned int ObservationsCount() const { return observations.size(); }
    unsigned int SettingsCount() const { return Settings.size(); }
    unsigned int ParametersCount() const { return Parameters().size(); }
    unsigned int ObjectiveFunctionsCount() { return ObjectiveFunctions().size(); }
    unsigned int ObjectiveFunctionsCount() const { return objective_function_set.size(); }

    // =====================================================================
    // Entity name listing
    //   [CHANGE] Add const — these only iterate and collect names
    // =====================================================================
    std::vector<std::string> GetAllBlockNames() const;
    std::vector<std::string> GetAllLinkNames() const;
    std::vector<std::string> GetAllSourceNames() const;
    std::vector<std::string> GetAllReactionNames() const;
    std::vector<std::string> GetAllObservationNames() const;

    // =====================================================================
    // Entity type listing
    //   [CHANGE] Add const — these only iterate and collect types
    // =====================================================================
    std::vector<std::string> GetAllBlockTypes() const;
    std::vector<std::string> GetAllLinkTypes() const;
    std::vector<std::string> GetAllSourceTypes() const;
    std::vector<std::string> GetAllTypesOf(const std::string& type) const;

    // =====================================================================
    // Graph topology
    // =====================================================================
    SafeVector<int> ConnectedBlocksTo(int blockid);
    SafeVector<int> ConnectedBlocksFrom(int blockid);
    SafeVector<int> SetLimitedOutFlow(int blockid, const std::string& variable, bool outflowlimited);
    void DisconnectLink(const std::string linkname);

    // =====================================================================
    // Solver
    // =====================================================================
    bool Solve(bool ApplyParams = false, bool uniformize_outputs = true);
    bool Update(const std::string& variable = "");
    bool Renew(const std::string& variable);
    bool OneStepSolve(unsigned int i, bool transport = false);
    double& GetTime() { return SolverTempVars.t; }
    double& dt() { return SolverTempVars.dt; }
    double& tend() { return SimulationParameters.tend; }
    double& tstart() { return SimulationParameters.tstart; }
    //   [CHANGE] Add const
    double dt0() const { return SimulationParameters.dt0; }
    int EpochCount() const { return SolverTempVars.epoch_count; }
    bool GetSolutionFailed() const { return SolverTempVars.SolutionFailed; }
    const solversettings& GetSolverSettings() const { return SolverSettings; }
    //   [CHANGE] Add const
    time_t GetSimulationDuration() const { return SolverTempVars.simulation_duration; }
    void SetSimulationDuration(const time_t& duration) { SolverTempVars.simulation_duration = duration; }
    void SetNumThreads(unsigned int i) { SolverSettings.n_threads = i; }
    //   [CHANGE] Add const
    int NumThreads() const { return SolverSettings.n_threads; }
    double& GetSimulationTime();
    void SetRecordResults(bool recresults) { SolverSettings.RecordAllOutputs = recresults; }
    //   [CHANGE] Add const
    bool RecordResults() const { return SolverSettings.RecordAllOutputs; }
    void SetParameterEstimationMode(parameter_estimation_options mode = parameter_estimation_options::none);
    bool stop_triggered = false;

    // =====================================================================
    // Solver settings (from script/properties)
    // =====================================================================
    bool SetProp(const std::string& s, const double& val);
    bool SetProperty(const std::string& s, const std::string& val);
    bool SetSystemSettingsObjectProperties(const std::string& s, const std::string& val, bool checkcritetia = false);

    // =====================================================================
    // Solution logging
    // =====================================================================
    void SetSolutionLogger(SolutionLogger& slnlogger);
    bool SetSolutionLogger(const std::string& filename);
    SolutionLogger* GetSolutionLogger();

    // =====================================================================
    // Outputs and results
    // =====================================================================
    TimeSeriesSet<outputtimeseriesprecision>& GetOutputs() { return Outputs.AllOutputs; }
    TimeSeriesSet<timeseriesprecision>& GetObservedOutputs() { return Outputs.ObservedOutputs; }
    bool TransferResultsFrom(System* other);
    bool WriteOutPuts();
    //   [CHANGE] Add const
    bool WriteIntermittently() const { return SimulationParameters.write_outputs_intermittently; }
    void SetOutputItems();
    bool SetLoadedOutputItems();

    // =====================================================================
    // Parameters
    // =====================================================================
    Parameter* GetParameter(const std::string& name) { return parameter_set[name]; }
    Parameter* GetParameter(int i) { return parameter_set[i]; }
    Parameter_Set& Parameters() { return parameter_set; }
    Parameter_Set Parameters() const { return parameter_set; }
    bool AppendParameter(const std::string& paramname, const double& lower_limit, const double& upper_limit, const std::string& prior_distribution = "normal");
    bool AppendParameter(const std::string& paramname, const Parameter& param);
    bool SetAsParameter(const std::string& location, const std::string& quantity, const std::string& parametername);
    bool SetAsParameter(const std::string& location, const std::string& quantity, const std::string& parametername, bool Full);
    bool RemoveAsParameter(const std::string& location, const std::string& quantity, const std::string& parametername);
    bool SetParameterValue(const std::string& paramname, const double& val);
    bool SetParameterValue(int i, const double& val);
    bool ApplyParameters();

    // =====================================================================
    // Objective functions
    // =====================================================================
    Objective_Function_Set& ObjectiveFunctions() { return objective_function_set; }
    const Objective_Function_Set& ObjectiveFunctions() const { return objective_function_set; }
    Objective_Function_Set* ObjectiveFunctionSet() { return &objective_function_set; }
    Objective_Function* ObjectiveFunction(const std::string& name);
    CVector ObjectiveFunctionValues() { return objective_function_set.Objective_Values(); }
    void AppendObjectiveFunction(const std::string& name, const Objective_Function&, double weight = 1);
    bool AppendObjectiveFunction(const std::string& name, const std::string& location, const Expression& exp, double weight = 1);
    double GetObjectiveFunctionValue();
    TimeSeries<timeseriesprecision>* GetObjectiveFunctionTimeSeries(const std::string& name) { return ObjectiveFunction(name)->GetTimeSeries(); }
    TimeSeriesSet<timeseriesprecision> GetModeledObjectiveFunctions();
    double CalcMisfit();
    std::vector<double> fit_measures;

    // =====================================================================
    // Observations
    // =====================================================================
    SafeVector<Observation>* Observations() { return &observations; }

    // =====================================================================
    // Constituent management
    //   [CHANGE] Move to private: AddConstituentRelatePropertiestoMetalModel,
    //            GetToBeCopiedQuantities (both overloads),
    //            AddAllConstituentRelateProperties (all overloads),
    //            AddConstituentRelateProperties
    //   These are internal mechanics called only during Add operations.
    //   Keep public: AllConstituents, AllReactionParameters, RenameConstituents,
    //                EraseConstituentRelatedProperties, AddPropertytoAllBlocks/Links,
    //                UpdateAddedPropertiestoAllBlockLinks
    // =====================================================================
    std::vector<std::string> AllConstituents();
    std::vector<std::string> AllReactionParameters();
    void RenameConstituents(const std::string& oldname, const std::string& newname);
    bool EraseConstituentRelatedProperties(const std::string& constituent_name);
    void AddPropertytoAllBlocks(const std::string& name, Quan& quan);
    void AddPropertytoAllLinks(const std::string& name, Quan& quan);
    void UpdateAddedPropertiestoAllBlockLinks();

    // =====================================================================
    // MetaModel and templates
    // =====================================================================
    MetaModel* GetMetaModel() { return &metamodel; }
    QuanSet* GetModel(const std::string& type) { if (metamodel.Count(type) == 1) return metamodel[type]; else return nullptr; }
    bool GetQuanTemplate(const std::string& filename);
    bool AppendQuanTemplate(const std::string& filename);
    void CopyQuansToMembers();
    bool ReadSystemSettingsTemplate(const std::string& filename);
    void SetSystemSettings();
    void SetSettingsParameter(const std::string& name, const double& value);
    void AddSolveVariableOrder(const std::string& variable) { solvevariableorder.push_back(variable); }

    // =====================================================================
    // Paths and directories
    // =====================================================================
    void SetWorkingFolder(const std::string& path) { paths.inputpath = path; }
    //   [CHANGE] Add const to all path getters
    std::string GetWorkingFolder() const { return paths.inputpath; }
    std::string InputPath() const { return paths.inputpath; }
    std::string OutputPath() const { return paths.outputpath; }
    std::string ObservedOutputFileName() const { return paths.observedoutputfilename; }
    std::string OutputFileName() const { return paths.outputfilename; }
    //   [CHANGE] DefaultTemplatePath has a side effect (cout). Remove the cout
    //   and add const, or split into getter/setter:
    std::string& DefaultTemplatePath() {
        std::cout << paths.default_template_path;  // TODO: Remove this side effect
        return paths.default_template_path;
    }
    void SetDefaultTemplatePath(const std::string& path) { paths.default_template_path = path; }

    // =====================================================================
    // Expression operators and functions
    //   [CHANGE] Add const
    // =====================================================================
    std::vector<std::string> exp_functions() const { return func_operators.funcs; }
    std::vector<std::string> exp_operators() const { return func_operators.opts; }
    static const vector<string> operators;
    static const vector<string> functions;

    // =====================================================================
    // Messaging, errors, and verification
    // =====================================================================
    void SetSilent(bool _s) { silent = _s; }
    bool IsSilent() const { return silent; }
    void ShowMessage(const std::string& msg) { if (!silent) std::cout << msg << std::endl; }
    ErrorHandler errorhandler;
    ErrorHandler* GetErrorHandler() { return &errorhandler; }
    ErrorHandler VerifyAllQuantities();
    bool Echo(const std::string& object, const std::string& quantity = "", const std::string& feature = "");

    void SetAllParents();                   
    void SetVariableParents();              
    void SetQuanPointers();                 
    void UnUpdateAllVariables();            
    bool CalcAllInitialValues();            
    bool InitiatePrecalculatedFunctions();  
    void InitOpenMP();      
    void MakeTimeSeriesUniform(const double& increment);  
    void ResetAllowLimitedFlows(bool allow);              
    bool VerifyAsSource(Block* blk, Link* lnk);           
    bool VerifyAsDestination(Block* blk, Link* lnk);      
    void WriteObjectsToLogger();            
    void WriteBlocksStates(const std::string& variable, const Expression::timing& tmg);   
    void WriteLinksStates(const std::string& variable, const Expression::timing& tmg);    
    bool CopyStateVariablesFrom(System* sys);
    bool ResetBasedOnRestorePoint(RestorePoint* rp);
    SafeVector<TimeSeries<timeseriesprecision>*> GetTimeSeries(bool onlyprecip);
    double GetMinimumNextTimeStepSize();
    Object* GetObjectBasedOnPrimaryKey(const std::string& s);
    std::vector<std::string> addedtemplates;
    void MakeObjectiveFunctionExpressionUniform();
    void MakeObservationsExpressionUniform();
    void UpdateObjectiveFunctions(double t);
    void UpdateObservations(double t);


    // =====================================================================
   // Constituent-related properties management
   // =====================================================================
    bool AddAllConstituentRelateProperties(Block* blk = nullptr);
    bool AddAllConstituentRelateProperties(Link* lnk = nullptr);
    bool AddAllConstituentRelateProperties(Reaction* rxn = nullptr);
    bool AddAllConstituentRelateProperties(Source* src = nullptr);
    void AddConstituentRelatePropertiestoMetalModel();
    bool AddConstituentRelateProperties(Object* constituent);


    // =====================================================================
    // Serialization — Script
    // =====================================================================
    bool SavetoScriptFile(const std::string& filename, const std::string& templatefilename = "", const std::vector<std::string>& addedtemplates = std::vector<std::string>());

    // =====================================================================
    // Serialization — JSON (Qt-dependent)
    // =====================================================================
#if defined(QT_GUI_SUPPORT) || defined (Q_JSON_SUPPORT)
    bool SavetoJson(const std::string& filename, const std::vector<std::string>& _addedtemplates, bool allvariable = false, bool calculatevalue = false);
    bool LoadfromJson(const QString& jsonfilename);
    bool LoadfromJson(const QJsonDocument& jsondoc);
    bool LoadfromJson(const QJsonObject& jsondoc);
    bool SaveStateVariableToJson(const std::string& variable, const std::string& filename);
    bool LoadStateVariableFromJson(const std::string& variable, const std::string& filename);
    QJsonObject toJsonObjectFull() const;
    bool SaveFullStateTo(const QString& filename) const;
#endif

    // =====================================================================
    // Qt GUI support
    // =====================================================================
#if defined(QT_GUI_SUPPORT) || defined (Q_JSON_SUPPORT)
    QStringList QGetAllCategoryTypes();
    QStringList QGetAllObjectsofTypes(QString _type);
    QStringList QGetAllObjectsofTypeCategory(QString _type);
    void Translate(double dx, double dy);

    /// Extracts all unique properties from blocks and links that have
    /// include_in_output set to true. Returns vector of (name, description) pairs.
    std::vector<std::pair<std::string, std::string>> GetOutputProperties();
#endif

#ifdef Q_GUI_SUPPORT
    ProgressWindow* RunTimewindow() { return rtw; }
    void SetProgressWindow(ProgressWindow* _rtw) { rtw = _rtw; }
    logwindow* LogWindow() { return _logWindow; }
    void SetLogWindow(logwindow* lgwnd) { _logWindow = lgwnd; }
#endif

protected:

private:

    // =====================================================================
    // Entity containers
    // =====================================================================
    SafeVector<Block> blocks;
    SafeVector<Link> links;
    SafeVector<Source> sources;
    SafeVector<Constituent> constituents;
    SafeVector<Reaction> reactions;
    SafeVector<RxnParameter> reaction_parameters;
    SafeVector<Object> Settings;
    SafeVector<Observation> observations;
    std::map<std::string, Quan> addedpropertiestoallblocks;
    std::map<std::string, Quan> addedpropertiestoalllinks;

    // =====================================================================
    // Constituent management (internal)
    // =====================================================================
    std::vector<Quan> GetToBeCopiedQuantities();
    std::vector<Quan> GetToBeCopiedQuantities(Object* consttnt, const object_type& object_typ = object_type::none);
    bool AddAllConstituentRelateProperties();
    

    // =====================================================================
    // MetaModel
    // =====================================================================
    MetaModel metamodel;
    void TransferQuantitiesFromMetaModel();
    void AppendQuantitiesFromMetaModel();
    void PopulateFunctionOperators();
    function_operators func_operators;

    // =====================================================================
    // Solver internals
    // =====================================================================
    solversettings SolverSettings;
    simulationparameters SimulationParameters;
    solvertemporaryvars SolverTempVars;
    std::vector<std::string> solvevariableorder;

    std::vector<bool> OneStepSolve();
    //   [CHANGE] Add const to variable parameter where applicable
    CVector_arma GetResiduals(const std::string& variable, CVector_arma& X, bool transport = false);
    CVector_arma GetResiduals_TR(const std::string& variable, CVector_arma& X);
    CMatrix_arma Jacobian(const std::string& variable, CVector_arma& X, bool transport = false);
    CMatrix_arma JacobianDirect(const std::string& variable, CVector_arma& X, bool transport);
    CVector_arma Jacobian(const std::string& variable, CVector_arma& V, CVector_arma& F0, int i, bool transport = false);
#ifdef SUPER_LU
    CMatrix_arma_sp Jacobian_SP(const std::string& variable, CVector_arma& X, bool transport = false);
    CMatrix_arma_sp JacobianDirect_SP(const std::string& variable, CVector_arma& X, bool transport);
#endif

    // Newton-Raphson solver steps
    bool ComputeNewtonStep(const std::string& variable, CVector_arma& X, CVector_arma& X1,
        CVector_arma& dx, const CVector_arma& F,
        unsigned int statevarno, bool transport,
        const std::vector<bool>& outflowlimitstatus_old);

    enum class NRAdjustResult { ok, failed };

    NRAdjustResult AdjustNRCoefficient(CVector_arma& X, const CVector_arma& X_past,
        const CVector_arma& X1, const CVector_arma& F,
        const CVector_arma& F1,
        double err, double& err_p,
        unsigned int statevarno, bool transport,
        int ini_max_error_block,
        double& error_increase_counter,
        const std::vector<bool>& outflowlimitstatus_old);

    void InitializeOneStep(const std::string& variable, unsigned int statevarno, bool transport,
        std::vector<bool>& outflowlimitstatus_old);

    // =====================================================================
    // State variable management
    // =====================================================================
    CVector_arma CalcStateVariables(const std::string& variable, const Expression::timing& tmg = Expression::timing::past);
    CVector_arma GetStateVariables(const std::string& variable, const Expression::timing& tmg = Expression::timing::past, bool transport = false);
    CVector_arma GetStateVariables_for_direct_Jacobian(const std::string& variable, const Expression::timing& tmg, bool transport);
    void SetStateVariables(const std::string& variable, CVector_arma& X, const Expression::timing& tmg = Expression::timing::present, bool transport = false);
    void SetStateVariables_for_direct_Jacobian(const std::string& variable, CVector_arma& X, const Expression::timing& tmg, bool transport);
    void SetStateVariables_TR(const std::string& variable, CVector_arma& X, const Expression::timing& tmg = Expression::timing::present);
    void SetNumberOfStateVariables(unsigned int n)
    {
        SolverTempVars.fail_reason.resize(n);
        SolverTempVars.Inverse_Jacobian.resize(n);
        SolverTempVars.NR_coefficient.resize(n);
        SolverTempVars.numiterations.resize(n);
        SolverTempVars.updatejacobian.resize(n);
    }
    
    std::pair<int, int> GetBlockConstituentValue(unsigned int i) const;
    std::string GetBlockConstituentSring(unsigned int i) const;
    std::string GetBlockConstituent(unsigned int i) const;

    // =====================================================================
    // Flow calculations
    // =====================================================================
    bool CalculateFlows(const std::string& var, const Expression::timing& tmg = Expression::timing::present);
    double Gradient(Object* obj, Object* wrt, const std::string& dependent_var, const std::string& independent_var);
    CVector_arma Gradient(Object* obj, const std::string& independent_var);
    void CorrectStoragesBasedonFluxes(const std::string& variable);
    void CalculateAllExpressions(Expression::timing tmg = Expression::timing::present);
    void CalcAllExpressions(const Expression::timing& tmg, bool force_all = true, bool dolinks = true);

    // =====================================================================
    // Outflow limiting
    // =====================================================================
    std::vector<bool> GetOutflowLimitedVector();
    std::vector<double> GetOutflowLimitFactorVector(const Expression::timing& tmg);
    void SetOutflowLimitedVector(const std::vector<bool>& x);
    CVector GetBlocksOutflowFactors(const Expression::timing& tmg);
    CVector GetLinkssOutflowFactors(const Expression::timing& tmg);
    bool OutFlowCanOccur(int blockno, const std::string& variable);

    // =====================================================================
    // Output management
    // =====================================================================
    outputs Outputs;
    void InitiateOutputs();
    void PopulateOutputs(bool links = true);

    // =====================================================================
    // Solver logging
    // =====================================================================
    SolutionLogger* solutionlogger = nullptr;
    void LogJacobianFailure(const CMatrix_arma& J, bool transport);
    void LogErrorIncrease(double err_p, double err, bool transport, int ini_max_error_block);
    void LogIterationLimitExceeded(const CVector_arma& F, const CVector_arma& X, const CMatrix_arma& InvJ,
        double err, double err_ini, double X_norm, bool transport,
        unsigned int statevarno, int ini_max_error_block);
    void LogErrorKeptIncreasing(const CVector_arma& F, bool transport, unsigned int statevarno);

    // =====================================================================
    // Other private state
    // =====================================================================
    Objective_Function_Set objective_function_set;
    Parameter_Set parameter_set;
    parameter_estimation_options ParameterEstimationMode = parameter_estimation_options::none;
    bool silent;
    _directories paths;
    std::vector<TimeSeries<timeseriesprecision>*> alltimeseries;
    unsigned int restore_interval = 200;

#ifndef NO_OPENMP
    omp_lock_t lock;
#endif
#ifdef Q_GUI_SUPPORT
    ProgressWindow* rtw = nullptr;
    logwindow* _logWindow = nullptr;
#endif


};