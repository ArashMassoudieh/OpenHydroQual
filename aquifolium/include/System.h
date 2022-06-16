#pragma once

#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Source.h"
#include "constituent.h"
#include "Vector_arma.h"
#include "Matrix_arma.h"
#include "MetaModel.h"
#include "BTCSet.h"
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
#if Q_version
#include <QStringList>
#include "runtimewindow.h"
#include "QTime"
#endif


using namespace std;


#ifdef DEBUG
#define CVector_arma CVector
#define CMatrix_arma CMatrix
#endif // _DEBUG

class Script;
class RestorePoint;

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
    CTimeSeriesSet<outputtimeseriesprecision> AllOutputs;
    CTimeSeriesSet<timeseriesprecision> ObservedOutputs;
};

struct solvertemporaryvars
{
    vector<CMatrix_arma> Inverse_Jacobian;
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

};

struct simulationparameters
{
    double tstart = 0; //start time of simulation
    double tend = 1; //end time of simulation
    double dt0 = 0.01; // initial time-step size
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
        bool AddBlock(Block &blk, bool SetQuantities=true);
        bool AddSource(Source &src);
        bool AddLink(Link &lnk, const string &source, const string &destination);
        bool AddConstituent(Constituent &cnst);
        bool AddReaction(Reaction &rxn);
        bool AddObservation(Observation &obs);
        bool AddReactionParameter(RxnParameter &rxn);
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
        SafeVector<int> ConnectedBlocksFrom(int blockid); // Get the list of connected to the block 'blockid'
        SafeVector<int> SetLimitedOutFlow(int blockid, const string &variable, bool outflowlimited); //Set outflow limitation status for block blockid and the connected rigid blocks to it
		bool Update(const string &variable="");
        void UnUpdateAllVariables();
		//bool Solve(const string &variable, bool ApplyParams = false);
		bool Solve(bool ApplyParams = false);
        void MakeTimeSeriesUniform(const double &increment);
		bool SetProp(const string &s, const double &val);
		bool SetProperty(const string &s, const string &val);
        CTimeSeriesSet<outputtimeseriesprecision>& GetOutputs() {return Outputs.AllOutputs;}
        CTimeSeriesSet<timeseriesprecision>& GetObservedOutputs() {return Outputs.ObservedOutputs;}
        vector<string> GetAllBlockTypes();
        vector<string> GetAllLinkTypes();
		vector<string> GetAllSourceTypes();
		vector<string> GetAllTypesOf(const string& type);
        void SetVariableParents();
        MetaModel *GetMetaModel() {return  &metamodel;}
        QuanSet* GetModel(const string &type) {if (metamodel.Count(type)==1) return metamodel[type]; else return nullptr;}
        void clear();
        int EpochCount() {return SolverTempVars.epoch_count;}
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
        bool RemoveAsParameter(const string &location, const string &quantity, const string &parametername);
        bool SetParameterValue(const string &paramname, const double &val);
        bool SetParameterValue(int i, const double &val);
        bool ApplyParameters();
        CTimeSeries<timeseriesprecision> *GetObjectiveFunctionTimeSeries(const string &name) {return ObjectiveFunction(name)->GetTimeSeries();}
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
		string InputPath() {return paths.inputpath;}
        string OutputPath() {return paths.outputpath;}
        string ObservedOutputFileName() {return paths.observedoutputfilename;}
        string OutputFileName() {return paths.outputfilename;}
        SafeVector<CTimeSeries<timeseriesprecision>*> TimeSeries();
        double GetMinimumNextTimeStepSize();
        Object *GetObjectBasedOnPrimaryKey(const string &s);
        bool SavetoScriptFile(const string &filename, const string &templatefilename="", const vector<string> &addedtemplates = vector<string>());
        bool ReadSystemSettingsTemplate(const string &filename);
        void SetSystemSettings();
        void DisconnectLink(const string linkname);
        bool SetSystemSettingsObjectProperties(const string &s, const string &val, bool checkcritetia = false);
        bool Delete(const string& objectname);
        void PopulateOperatorsFunctions();
        bool VerifyAsSource(Block* blk, Link* lnk);
        bool VerifyAsDestination(Block* blk, Link* lnk);
        ErrorHandler VerifyAllQuantities();
        bool CalcAllInitialValues();
        void WriteObjectsToLogger();
        void WriteBlocksStates(const string &variable, const Expression::timing &tmg);
        void WriteLinksStates(const string &variable, const Expression::timing &tmg);
        bool InitiatePrecalculatedFunctions();
        bool CopyStateVariablesFrom(System *sys);
        bool EraseConstituentRelatedProperties(const string &constituent_name);
        void SetNumThreads(unsigned int i) 
        {
            SolverSettings.n_threads = i;
        }
        int NumThreads() { return SolverSettings.n_threads; }
        void ResetAllowLimitedFlows(bool allow);

#if defined(QT_version)
        logWindow *LogWindow() {return logwindow;}
        void SetLogWindow(logWindow *lgwnd) {logwindow=lgwnd;}
#endif
        bool stop_triggered = false;
#if defined(QT_version) || defined (Q_version)
        QStringList QGetAllCategoryTypes();
		QStringList QGetAllObjectsofTypes(QString _type);
		QStringList QGetAllObjectsofTypeCategory(QString _type);
        RunTimeWindow *RunTimewindow() {return rtw;}
        void SetRunTimeWindow(RunTimeWindow* _rtw) {rtw = _rtw;}
#endif
        unique_ptr<vector<string>> operators;
        unique_ptr<vector<string>> functions;
        void SetOutputItems();
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
        CTimeSeriesSet<timeseriesprecision> GetModeledObjectiveFunctions();
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
        CVector_arma GetResiduals(const string &variable, CVector_arma &X, bool transport=false);
        CVector_arma GetResiduals_TR(const string &variable, CVector_arma &X);
        double Gradient(Object* obj, Object* wrt, const string &dependent_var, const string &independent_var);
        CVector_arma Gradient(Object* obj, const string &independent_var);
        CMatrix_arma JacobianDirect(const string &variable, CVector_arma &X, bool transport);
		void CorrectStoragesBasedonFluxes(const string& variable);
        CVector_arma CalcStateVariables(const string &variable, const Expression::timing &tmg = Expression::timing::past);
        CVector_arma GetStateVariables(const string &variable, const Expression::timing &tmg = Expression::timing::past, bool transport=false);
        CVector_arma GetStateVariables_for_direct_Jacobian(const string &variable, const Expression::timing &tmg, bool transport);
        solversettings SolverSettings;
        simulationparameters SimulationParameters;
        vector<bool> OneStepSolve();
        CMatrix_arma Jacobian(const string &variale, CVector_arma &X, bool transport=false);
        CVector_arma Jacobian(const string &variable, CVector_arma &V, CVector_arma &F0, int i, bool transport=false);
        bool CalculateFlows(const string &var, const Expression::timing &tmg = Expression::timing::present);
        void SetStateVariables(const string &variable, CVector_arma &X, const Expression::timing &tmg = Expression::timing::present, bool transport=false);
        void SetStateVariables_for_direct_Jacobian(const string &variable, CVector_arma &X, const Expression::timing &tmg, bool transport);
        void SetStateVariables_TR(const string &variable, CVector_arma &X, const Expression::timing &tmg = Expression::timing::present);
        vector<bool> GetOutflowLimitedVector();
        vector<double> GetOutflowLimitFactorVector(const Expression::timing &tmg);
        void SetOutflowLimitedVector(vector<bool>& x);
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
        vector<CTimeSeries<timeseriesprecision>*> alltimeseries;
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
        CVector GetBlocksOutflowFactors(const Expression::timing &tmg);
        bool OutFlowCanOccur(int blockno, const string &variable);
        CVector GetLinkssOutflowFactors(const Expression::timing &tmg);
        unsigned int restore_interval = 200;
        void PopulateFunctionOperators();
        function_operators func_operators;
        omp_lock_t lock;

#ifdef Q_version
    RunTimeWindow *rtw = nullptr;
#endif

#ifdef QT_version
        GraphWidget *diagramview;
        runtimeWindow *rtw = nullptr;
        void updateProgress(bool finished);
        logWindow *logwindow = nullptr;
#endif
};


