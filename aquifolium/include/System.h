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
#ifdef QT_version
    #include "runtimeWindow.h"
    class GWidget;
    class logWindow;
#endif
#include "ErrorHandler.h"
#include "safevector.h"
#include <string>
#define outputtimeseriesprecision double
#if Q_GUI_SUPPORT
#include <QStringList>
#include "QTime"
#endif

#define unordered_map map

using namespace std;

#ifdef SUPER_LU
#include "Matrix_arma_sp.h"
#endif


#ifdef DEBUG
#define CVector_arma CVector
#define CMatrix_arma CMatrix
#endif // _DEBUG

class Script;
class RestorePoint;
class QuanSet; 

enum class parameter_estimation_options {none, optimize, inverse_model};

struct solversettings
{
    double C_N_weight; //Crank-Nicholson Weight
    double NRtolerance = 1e-6; //Newton Raphson Tolerance
    int n_threads = 16; //Number of threads
    double NR_coeff_reduction_factor = 0.8; //The coefficient to reduce the Newton-Raphson coefficient
    double NR_timestep_reduction_factor = 0.75;
    double NR_timestep_reduction_factor_fail = 0.3;
    double minimum_timestep = 1e-7;
    int NR_niteration_lower=20;
    int NR_niteration_upper=40;
    int NR_niteration_max=100;
    bool makeresultsuniform = false;
    bool scalediagonal = false;
    double landtozero_factor = 0;
    bool optimize_lambda = true;
    bool direct_jacobian = false;
    bool write_solution_details = false;
    double maximum_simulation_time = 86400; //maximum simulation time allows in seconds
    int maximum_number_of_matrix_inversions = 200000; //maximum number of matrix inversions allowed
    bool RecordAllOutputs = true; //whether the results will be all recorded on not

};

struct function_operators
{
    vector<string> funcs;
    vector<string> opts;

};

struct outputs
{
    TimeSeriesSet<outputtimeseriesprecision> AllOutputs;
    TimeSeriesSet<timeseriesprecision> ObservedOutputs;
};

struct solvertemporaryvars
{
#ifdef SUPER_LU
    vector<CMatrix_arma_sp> Inverse_Jacobian;
#else
    vector<CMatrix_arma> Inverse_Jacobian;
#endif
    vector<double> NR_coefficient;
    vector<bool> updatejacobian;
	int MaxNumberOfIterations()
	{
		return aquiutils::Max(numiterations);
	}

	void SetUpdateJacobian(bool x)
	{
        for (unsigned int i = 0; i < updatejacobian.size(); i++)
			updatejacobian[i] = x;
    }
    vector<int> numiterations;
    int numiteration_tr;
    int epoch_count = 0;
    vector<string> fail_reason;
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
    string inputpath;
    string outputpath;
	string default_template_path;
    string outputfilename="output.txt";
    string observedoutputfilename="observedoutput.txt";
};

class System: public Object
{
    public:
        System();
        virtual ~System();
        System(const System& other);
        System(Script& scr);
        bool TransferResultsFrom(System *other);
        System& operator=(const System& other);
        bool CreateFromScript(Script& scr, const string &settingsfilename="");
        double &GetTime()
        {
            return SolverTempVars.t;
        }
        bool AddBlock(const Block &blk, bool SetQuantities=true);
        bool AddSource(const Source &src, bool SetQuantities=true);
        bool AddLink(const Link &lnk, const string &source, const string &destination, bool SetQuantities=true);
        bool AddConstituent(const Constituent &cnst, bool SetQuantities=true);
        bool AddReaction(const Reaction &rxn, bool SetQuantities=true);
        bool AddObservation(const Observation &obs, bool SetQuantities=true);
        bool AddReactionParameter(const RxnParameter &rxn, bool SetQuantities=true);
        Block *block(const string &s);
        Block *block(unsigned int i)
        {
            if (i<blocks.size())
                return &blocks[i];
            else
                return nullptr;
        }
        Object *Setting(unsigned int i)
        {
            if (i<Settings.size())
                return &Settings[i];
            else
                return nullptr;
        }
        unsigned int BlockCount() {return blocks.size();}
        unsigned int SettingsCount() {return Settings.size();}
        unsigned int LinksCount() {return links.size();}
        unsigned int SourcesCount() {return sources.size();}
        unsigned int ReactionsCount() {return reactions.size();}
        unsigned int ObservationsCount() {return observations.size();}
        unsigned int ParametersCount() {return Parameters().size();}
        unsigned int ObjectiveFunctionsCount() {return ObjectiveFunctions().size();}
        unsigned int ConstituentsCount() {return constituents.size();}
        unsigned int ReactionParametersCount() {return reaction_parameters.size();}
        vector<string> GetAllSourceNames();
        vector<string> GetAllBlockNames();
        vector<string> GetAllLinkNames();
        vector<string> GetAllReactionNames();
        vector<string> GetAllObservationNames();

        Link *link(const string &s);
        Link *link(unsigned int i)
        {
            if (i<links.size())
                return &links[i];
            else
                return nullptr;
        }
        Source *source(const string &s);
        Constituent *constituent(const string &s);
        Reaction *reaction(const string &s);
        RxnParameter *reactionparameter(const string &s);
        Observation *observation(const string &s);
        Source *source(unsigned int i)
        {   if (i<sources.size())
                return &sources[i];
            else
                return nullptr;
        }
        Constituent *constituent(unsigned int i)
        {   if (i<constituents.size())
                return &constituents[i];
            else
                return nullptr;
        }
        RxnParameter *reactionparameter(unsigned int i)
        {   if (i<reaction_parameters.size())
                return &reaction_parameters[i];
            else
                return nullptr;
        }
        Reaction *reaction(unsigned int i)
        {   if (i<reactions.size())
                return &reactions[i];
            else
                return nullptr;
        }
        Observation *observation(unsigned int i)
        {   if (i<observations.size())
                return &observations[i];
            else
                return nullptr;
        }
        Parameter *parameter(const string &s);
        Objective_Function *objectivefunction(const string &s);
        Objective_Function *objectivefunction(unsigned int i)
        {
            if (i<objective_function_set.size())
                return objective_function_set[i];
            else
                return nullptr;
        }
        CVector ObjectiveFunctionValues()
        {
            return objective_function_set.Objective_Values();
        }
        Object *object(const string &s);
        Object *settings(const string &s);

        int blockid(const string &s);
        int linkid(const string &s);
        bool GetQuanTemplate(const string &filename);
        bool AppendQuanTemplate(const string &filename);
        void CopyQuansToMembers();
        double &dt() {return SolverTempVars.dt;}
        double &tend() {return SimulationParameters.tend;}
        double &tstart() {return SimulationParameters.tstart;}
        bool OneStepSolve(unsigned int i, bool transport=false);
        bool Renew(const string &variable);
        SafeVector<int> ConnectedBlocksTo(int blockid); // Get the list of blocks the block 'blockid' is connected to
        SafeVector<int> ConnectedBlocksFrom(int blockid);
        
        bool OneStepSolve_(unsigned int statevarno, bool transport);

        // Get the list of connected to the block 'blockid'
        SafeVector<int> SetLimitedOutFlow(int blockid, const string &variable, bool outflowlimited); //Set outflow limitation status for block blockid and the connected rigid blocks to it
		bool Update(const string &variable="");
        void UnUpdateAllVariables();
		//bool Solve(const string &variable, bool ApplyParams = false);
		bool Solve(bool ApplyParams = false);
        void MakeTimeSeriesUniform(const double &increment);
        /**
     * @brief Prepares the system for numerical solving by initializing solver parameters and internal state.
     *
     * This includes:
     * - Initializing OpenMP locks (if enabled)
     * - Resizing observation fit metrics
     * - Recording simulation start time
     * - Setting parent-child relationships across all model components
     * - Preparing state variable storage based on the number of state variables
     * - Enabling Jacobian updates for the Newton-Raphson solver
     * - Extracting all relevant time series inputs (e.g., precipitation)
     */
        void InitializeSolverEnvironment();
        /**
     * @brief Initializes the simulation state, expressions, initial conditions and output structures.
     *
     * This function performs the following tasks:
     * - Applies model parameters to objects (if requested)
     * - Initializes simulation outputs and registers logging
     * - Uniformizes all time series to the base time step
     * - Initializes symbolic and precalculated expressions
     * - Computes initial values for state variables
     * - Clears update flags on all model objects
     *
     * @param applyparameters If true, parameters will be applied to the model before initialization.
     */
        void InitializeModelState(bool applyparameters);
        /**
 * @brief Clears existing time series for all objective functions and observations.
 *
 * This ensures that previous simulation results do not interfere with the current run.
 */
        void ClearObservationTimeSeries();
        
        /**
 * @brief Initializes the runtime window GUI with simulation metadata.
 *
 * If GUI support is enabled and a runtime window exists:
 * - Displays a simulation start message with timestamp
 * - Sets the X axis to time range [tstart, tend]
 * - Sets the Y axis to timestep range
 * - Triggers event processing to refresh the UI
 */
        void InitializeSimulationUI();
        /**
 * @brief Executes the main adaptive time-stepping loop for solving the system.
 *
 * This function runs the simulation forward in time using the Newton-Raphson solver,
 * updating system states, logging outputs, handling convergence and failure recovery,
 * and interacting with GUI elements if enabled.
 *
 * It adjusts time step size based on solver convergence, writes outputs periodically,
 * and terminates early if global limits (e.g. simulation time or number of matrix inversions) are exceeded.
 *
 * @param success Reference to a boolean that will be set to true if the loop completes successfully, or false on failure.
 */
        void RunTimeLoop(bool& success);
        /**
 * @brief Finalizes simulation by uniformizing outputs, updating logs, and flushing GUI.
 *
 * This function:
 * - Uniformizes all output and observation time series
 * - Applies final expression processing for objective functions and observations
 * - Updates GUI progress bars and completion messages
 * - Writes a final status message to the solution logger
 */
        void FinalizeOutputs();
        bool SetProp(const string &s, const double &val);
		bool SetProperty(const string &s, const string &val);
        TimeSeriesSet<outputtimeseriesprecision>& GetOutputs() {return Outputs.AllOutputs;}
        TimeSeriesSet<timeseriesprecision>& GetObservedOutputs() {return Outputs.ObservedOutputs;}
        vector<string> GetAllBlockTypes();
        vector<string> GetAllLinkTypes();
		vector<string> GetAllSourceTypes();
		vector<string> GetAllTypesOf(const string& type);
        void SetVariableParents();
        MetaModel *GetMetaModel() {return  &metamodel;}
        QuanSet* GetModel(const string& type);
        void clear();
        int EpochCount() {return SolverTempVars.epoch_count;}
        bool WriteIntermittently() {return SimulationParameters.write_outputs_intermittently;}
        ErrorHandler *GetErrorHandler() {return &errorhandler;}
        void UpdateObservations(double t);
        double CalcMisfit(); //calculates the difference between modeled and measured data
 //Objective Functions
        void AppendObjectiveFunction(const string &name, const Objective_Function&, double weight=1);
        bool AppendObjectiveFunction(const string &name, const string &location, const Expression &exp, double weight=1);
        void UpdateObjectiveFunctions(double t);
        double GetObjectiveFunctionValue();
        void MakeObjectiveFunctionExpressionUniform();
        void MakeObservationsExpressionUniform();
        Objective_Function *ObjectiveFunction(const string &name); // returns a pointer to an objective function
// Parameters
        Parameter *GetParameter(const string &name) {return parameter_set[name];}
        Parameter *GetParameter(int i) {return parameter_set[i];}
        Parameter_Set &Parameters() {return parameter_set;}
        Objective_Function_Set &ObjectiveFunctions() {return objective_function_set;}
        bool AppendParameter(const string &paramname, const double &lower_limit, const double &upper_limit, const string &prior_distribution = "normal");
        bool AppendParameter(const string &paramname, const Parameter& param);
        bool SetAsParameter(const string &location, const string &quantity, const string &parametername);
        bool SetAsParameter(const string& location, const string& quantity, const string& parametername, bool Full);
        bool RemoveAsParameter(const string &location, const string &quantity, const string &parametername);
        bool SetParameterValue(const string &paramname, const double &val);
        bool SetParameterValue(int i, const double &val);
        bool ApplyParameters();
        TimeSeries<timeseriesprecision> *GetObjectiveFunctionTimeSeries(const string &name) {return ObjectiveFunction(name)->GetTimeSeries();}
        void SetSilent(bool _s) {silent = _s;}
        bool IsSilent() {return silent;}
        void ShowMessage(const string &msg) {if (!silent) cout<<msg<<std::endl; }
        void SetAllParents();
        ErrorHandler errorhandler;
        bool Echo(const string &object, const string &quantity = "", const string &feature="");
        string& DefaultTemplatePath() {
            cout<<paths.default_template_path;
            return paths.default_template_path;
        }
        void SetDefaultTemplatePath(const string &path) {
            paths.default_template_path = path;
        }
		string InputPath() {return paths.inputpath;}
        string OutputPath() {return paths.outputpath;}
        string ObservedOutputFileName() {return paths.observedoutputfilename;}
        string OutputFileName() {return paths.outputfilename;}
        SafeVector<TimeSeries<timeseriesprecision>*> GetTimeSeries(bool onlyprecip=false);
        double GetMinimumNextTimeStepSize();
        Object *GetObjectBasedOnPrimaryKey(const string &s);
        bool SavetoScriptFile(const string &filename, const string &templatefilename="", const vector<string> &addedtemplates = vector<string>());
        bool ReadSystemSettingsTemplate(const string &filename);
        void SetSystemSettings();
        void SetSettingsParameter(const string &name, const double &value);
        void DisconnectLink(const string linkname);
        bool SetSystemSettingsObjectProperties(const string &s, const string &val, bool checkcritetia = false);
        bool Delete(const string& objectname);
        void PopulateOperatorsFunctions();
        bool VerifyAsSource(const Block* blk, const Link* lnk);
        bool VerifyAsDestination(const Block* blk, const Link* lnk);
        ErrorHandler VerifyAllQuantities();
        bool CalcAllInitialValues();
        void WriteObjectsToLogger();
        void WriteBlocksStates(const string &variable, const Timing &tmg);
        void WriteLinksStates(const string &variable, const Timing &tmg);
        bool InitiatePrecalculatedFunctions();
        bool CopyStateVariablesFrom(System *sys);
        bool EraseConstituentRelatedProperties(const string &constituent_name);
        void SetNumThreads(unsigned int i) 
        {
            SolverSettings.n_threads = i;
        }
        int NumThreads() { return SolverSettings.n_threads; }
        void ResetAllowLimitedFlows(bool allow);

#if defined(Q_GUI_SUPPORT)
        logWindow *LogWindow() {return logwindow;}
        void SetLogWindow(logWindow *lgwnd) {logwindow=lgwnd;}
        RunTimeWindow *RunTimewindow() {return rtw;}
        void SetRunTimeWindow(RunTimeWindow* _rtw) {rtw = _rtw;}
#endif
        bool stop_triggered = false;
#if defined(QT_GUI_SUPPORT) || defined (Q_JSON_SUPPORT)
        QStringList QGetAllCategoryTypes();
		QStringList QGetAllObjectsofTypes(QString _type);
		QStringList QGetAllObjectsofTypeCategory(QString _type);
#endif

#ifdef QT_GUI_SUPPORT
        RunTimeWindow *RunTimewindow() {return rtw;}
        void SetRunTimeWindow(RunTimeWindow* _rtw) {rtw = _rtw;}
#endif

        unique_ptr<vector<string>> operators;
        unique_ptr<vector<string>> functions;
        void SetOutputItems();
        bool SetLoadedOutputItems();
        double & GetSimulationTime();
        vector<string> addedtemplates;
        //constituents
        void AddPropertytoAllBlocks(const string &name, Quan &quan);
        void AddPropertytoAllLinks(const string &name, Quan &quan);
        void UpdateAddedPropertiestoAllBlockLinks();
        vector<Quan> GetToBeCopiedQuantities();
        vector<Quan> GetToBeCopiedQuantities(Object *consttnt, const object_type &object_typ = object_type::none);
        bool AddAllConstituentRelateProperties();
        bool AddConstituentRelateProperties(Object *constituent);
        bool AddAllConstituentRelateProperties(Block *blk = nullptr);
        bool AddAllConstituentRelateProperties(Link *lnk = nullptr);
        bool AddAllConstituentRelateProperties(Reaction *rxn = nullptr);
        bool AddAllConstituentRelateProperties(Source *src = nullptr);
        void AddConstituentRelatePropertiestoMetalModel();
        void RenameConstituents(const string &oldname, const string &newname);
        vector<string> AllConstituents();
        vector<string> AllReactionParameters();
        void SetSolutionLogger(SolutionLogger &slnlogger);
        bool SetSolutionLogger(const string &filename);
        const solversettings& GetSolverSettings() const {return SolverSettings;}
        SolutionLogger *GetSolutionLogger();
        bool GetSolutionFailed() {return SolverTempVars.SolutionFailed;}
        void SetParameterEstimationMode(parameter_estimation_options mode = parameter_estimation_options::none);
        void SetQuanPointers();
        bool ResetBasedOnRestorePoint(RestorePoint *rp);
        TimeSeriesSet<timeseriesprecision> GetModeledObjectiveFunctions();
        time_t GetSimulationDuration() 
        {
            return SolverTempVars.simulation_duration;
        }
        void SetSimulationDuration(const time_t &duration) 
        {
            SolverTempVars.simulation_duration = duration;
        }
        void SetRecordResults(bool recresults)
        {
            SolverSettings.RecordAllOutputs = recresults;
        }
        bool RecordResults()
        {
            return SolverSettings.RecordAllOutputs;
        }
        void Clear();
        SafeVector<Observation>* Observations() {return &observations;}
        vector<string> exp_functions() {return func_operators.funcs;}
        vector<string> exp_operators() {return func_operators.opts;}
        vector<double> fit_measures;
        void SetWorkingFolder(const string &path)
        {
            paths.inputpath = path;
        }
        string GetWorkingFolder() {return paths.inputpath;}
        double dt0() {return SimulationParameters.dt0;}
        Objective_Function_Set *ObjectiveFunctionSet() {return &objective_function_set;}
        bool WriteOutPuts();
        bool SavetoJson(const string &filename, const vector<string> &_addedtemplates, bool allvariable = false, bool calculatevalue = false);
        bool SaveEquationstoJson(const string& filename);
        bool LoadfromJson(const QJsonDocument &jsondoc);
        bool LoadfromJson(const QJsonObject &jsondoc);
    protected:

    private:
        vector<string> solvevariableorder;
        SafeVector<Block> blocks;
        SafeVector<Link> links;
        SafeVector<Source> sources;
        SafeVector<Constituent> constituents;
        SafeVector<Reaction> reactions;
        SafeVector<RxnParameter> reaction_parameters;
        SafeVector<Object> Settings;
        SafeVector<Observation> observations;
        map<string, Quan> addedpropertiestoallblocks;
        map<string, Quan> addedpropertiestoalllinks;
        MetaModel metamodel;
        CVector_arma GetResiduals(const string &variable, const CVector_arma &X, bool transport=false);
        CVector_arma GetResiduals_TR(const string &variable, const CVector_arma &X);
        double Gradient(Object* obj, Object* wrt, const string &dependent_var, const string &independent_var);
        CVector_arma Gradient(Object* obj, const string &independent_var);

		void CorrectStoragesBasedonFluxes(const string& variable);
        CVector_arma CalcStateVariables(const string &variable, const Timing &tmg = Timing::past);
        CVector_arma GetStateVariables(const string &variable, const Timing &tmg = Timing::past, bool transport=false);
        CVector_arma GetStateVariables_for_direct_Jacobian(const string &variable, const Timing &tmg, bool transport);
        solversettings SolverSettings;
        simulationparameters SimulationParameters;
        vector<bool> OneStepSolve();
        CMatrix_arma Jacobian(const string &variale, CVector_arma &X, bool transport=false);
        CMatrix_arma JacobianDirect(const string &variable, CVector_arma &X, bool transport);
        CVector_arma Jacobian(const string &variable, CVector_arma &V, CVector_arma &F0, int i, bool transport=false);
#ifdef SUPER_LU
        CMatrix_arma_sp Jacobian_SP(const string &variable, CVector_arma &X, bool transport = false);
        CMatrix_arma_sp JacobianDirect_SP(const string &variable, CVector_arma &X, bool transport);
#endif

        bool CalculateFlows(const string &var, const Timing &tmg = Timing::present);
        void SetStateVariables(const string &variable, const CVector_arma &X, const Timing &tmg = Timing::present, bool transport=false);
        string GetBlockConstituent(unsigned int i);
        void SetStateVariables_for_direct_Jacobian(const string &variable, CVector_arma &X, const Timing &tmg, bool transport);
        void SetStateVariables_TR(const string &variable, const CVector_arma &X, const Timing &tmg = Timing::present);
        vector<bool> GetOutflowLimitedVector();
        vector<double> GetOutflowLimitFactorVector(const Timing &tmg);
        void SetOutflowLimitedVector(const vector<bool>& x);
        
        /**
         * @brief Copies outflow limit factors from one time step to another for all links and blocks.
         *
         * This function transfers the outflow limitation values (which may represent flow caps or physical constraints)
         * from a source time step (e.g., Timing::past) to a destination time step (e.g., Timing::present).
         *
         * @param from The time context to read the current outflow limit factors (typically Timing::past)
         * @param to The time context to apply the propagated outflow limit factors (typically Timing::present)
         */
        void PropagateOutflowLimitFactors(Timing from, Timing to);


        /**
         * @brief Prepares the initial guess and residual vector for Newton-Raphson iteration.
         *
         * This function retrieves the state variable values from the previous time step,
         * applies any relevant outflow limits (for non-transport mode),
         * computes the residual vector, and stores a backup of the initial guess.
         *
         * @param variable The name of the state variable being solved
         * @param transport Indicates whether the solve is for transport (mass flow)
         * @param[out] X The initial guess vector (possibly limited)
         * @param[out] F The residual vector at the initial guess
         * @param[out] X_past Backup copy of the initial guess (used to restore state on divergence)
         * @param outflowlimitstatus_old Original outflow limitation flags (restored on failure)
         * @return true if residuals are finite and initialization succeeded; false otherwise
         */
        bool PrepareSolveStateForInitialGuess(const std::string& variable, bool transport, CVector_arma& X, CVector_arma& F, CVector_arma& X_past, const std::vector<bool>& outflowlimitstatus_old);

        /**
         * @brief Logs detailed information when the residual vector becomes non-finite.
         *
         * This function appends a diagnostic message to the fail_reason vector and writes
         * detailed solver diagnostics (time, residual, state vector, and timestep) to the
         * solution logger, if one is available.
         *
         * @param variable The name of the state variable being solved
         * @param X The current state vector at failure
         * @param F The residual vector at failure
         */
        void LogResidualFailure(const std::string& variable, const CVector_arma& X, const CVector_arma& F);

        /**
         * @brief Handles the case when the residual vector F is invalid (NaN or Inf).
         *
         * Logs diagnostic messages, flushes the solution logger, and restores outflow limit vector if necessary.
         *
         * @param variable Name of the variable being solved
         * @param X Current state vector
         * @param F Residual vector (invalid)
         * @param transport Whether the simulation is in transport mode
         * @param outflowlimitstatus_old Saved outflow limitation status to restore
         * @return Always returns false to allow inline usage as `return HandleInvalidResidual(...)`
         */
        bool HandleInvalidResidual(const std::string& variable, const CVector_arma& X, const CVector_arma& F, bool transport, const std::vector<bool>& outflowlimitstatus_old);

        /**
         * @brief Logs detailed information when the Jacobian matrix is found to be not full-ranked.
         *
         * This includes:
         * - A message about Jacobian singularity
         * - The diagonal vector
         * - Indices of blocks with zero diagonal values
         * - Appends failure reason to SolverTempVars
         *
         * @param J The Jacobian matrix
         */
        void LogJacobianSingularity(const CMatrix_arma& J);

        /**
         * @brief Handles failure due to exceeding maximum allowed Newton-Raphson iterations.
         *
         * Logs the failure reason, outputs detailed diagnostic information,
         * and restores outflow-limited status if needed.
         *
         * @param variable The state variable being solved
         * @param X The current state vector
         * @param F The current residual vector
         * @param statevarno Index of the state variable in solve order
         * @param ini_max_error_block Index of the block with the initial max error
         * @param err Current residual norm
         * @param err_ini Initial residual norm
         * @param X_norm Norm of the state vector X
         * @param transport Whether this is a transport solve
         * @param outflowlimitstatus_old Outflow limitation status to restore if needed
         * @return Always returns false to allow inline usage like `return HandleIterationLimitFailure(...)`
         */
        bool HandleIterationLimitFailure(const std::string& variable, const CVector_arma& X, const CVector_arma& F, unsigned int statevarno, int ini_max_error_block, double err, double err_ini, double X_norm, bool transport, const std::vector<bool>& outflowlimitstatus_old);

        /**
         * @brief Handles failure when storage becomes negative in a block after maximum allowed attempts.
         *
         * Logs the failure, outputs the block name and time step, flushes the logger,
         * and restores the outflow limit status if applicable.
         *
         * @param block_index Index of the block with negative storage
         * @param transport Whether this is a transport solve
         * @param outflowlimitstatus_old Previously saved outflow limit vector
         * @return Always returns false for inline usage
         */
        bool HandleNegativeStorageFailure(unsigned int block_index, bool transport, const std::vector<bool>& outflowlimitstatus_old);

         

        solvertemporaryvars SolverTempVars;
        outputs Outputs;
        void InitiateOutputs();

        void PopulateOutputs(bool links=true);
        void TransferQuantitiesFromMetaModel();
        void AppendQuantitiesFromMetaModel();
        Objective_Function_Set objective_function_set;
        Parameter_Set parameter_set;
        bool silent;
        _directories paths;
        vector<TimeSeries<timeseriesprecision>*> alltimeseries;
        void CalculateAllExpressions(Expression::timing tmg = Expression::timing::present);
        void SetNumberOfStateVariables(unsigned int n)
		{
			SolverTempVars.fail_reason.resize(n);
			SolverTempVars.Inverse_Jacobian.resize(n);
			SolverTempVars.NR_coefficient.resize(n);
			SolverTempVars.numiterations.resize(n);
			SolverTempVars.updatejacobian.resize(n);
		}
        SolutionLogger *solutionlogger = nullptr;
        parameter_estimation_options ParameterEstimationMode = parameter_estimation_options::none;
        CVector GetBlocksOutflowFactors(const Timing &tmg);
        bool OutFlowCanOccur(int blockno, const string &variable);
        CVector GetLinkssOutflowFactors(const Timing &tmg);
        unsigned int restore_interval = 200;
        void PopulateFunctionOperators();

        function_operators func_operators;

#ifndef NO_OPENMP
        omp_lock_t lock;
#endif
#ifdef Q_GUI_SUPPORT
    RunTimeWindow *rtw = nullptr;
    void updateProgress(bool finished);
    logWindow *logwindow = nullptr;
#endif


};


