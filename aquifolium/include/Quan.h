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

#define timeseriesprecision double

#ifndef QUAN_H
#define QUAN_H

#include <string>
#include <json/json.h>
#include "Expression.h"
#include "Rule.h"
#include "TimeSeries.h"
#include "precalculatedfunction.h"
#include "Condition.h"
#include <mutex>

#ifdef Q_version
#include <QJsonObject>
#endif

 // Forward declarations
class Block;
class Link;
class System;
class Object;
class Source;
class QuanSet;
class CPrecipitation;

#ifndef NO_OPENMP
struct ScopedOmpLock {
    omp_lock_t lock;
    bool active = false;
    ScopedOmpLock() {
        if (omp_get_num_threads() > 1) {
            omp_init_lock(&lock);
            omp_set_lock(&lock);
            active = true;
        }
    }
    ~ScopedOmpLock() {
        if (active) {
            omp_unset_lock(&lock);
            omp_destroy_lock(&lock);
        }
    }
};
#endif

class Quan
{
public:
    Quan(); ///< Default constructor
    virtual ~Quan(); ///< Destructor cleans up internal structures
    Quan(const Quan& other); ///< Copy constructor
    Quan(Json::ValueIterator& it); ///< Constructor from JSON iterator

#ifdef Q_version
    Quan(QJsonObject& qjobject); ///< Constructor from QJsonObject (Qt integration)
#endif // QT_version

    std::string GetStringValue() { return _string_value; } ///< Returns string value for string-type quantities
    Quan& operator=(const Quan& other); ///< Assignment operator
    bool operator==(const Quan& other) const; ///< Equality operator

    enum class _type { constant, value, balance, expression, timeseries, prec_timeseries, global_quan, rule, source, string, not_assigned, boolean }; ///< Types of quantities handled by the model
    enum class _role { none, copytoblocks, copytolinks, copytosources, copytoreactions }; ///< Roles for automatic copying behavior

    double CalcVal(Object*, const Expression::timing& tmg = Expression::timing::past); ///< Calculates value using context object
    double CalcVal(const Expression::timing& tmg = Expression::timing::past); ///< Calculates value using parent object
    double GetVal(const Expression::timing& tmg = Expression::timing::past); ///< Gets value based on evaluation time
    bool EstablishExpressionStructure(); ///< Initializes expression term dependencies
    double& GetSimulationTime() const; ///< Retrieves simulation time from the parent system
    TimeSeries<timeseriesprecision>* GetTimeSeries(); ///< Returns pointer to associated time series

    std::string last_error; ///< Stores last error message if any

    void SetType(const _type& t) { type = t; }
    _type GetType() { return type; }
    Expression* GetExpression();
    Rule* GetRule();
    Source* GetSource();

    bool SetExpression(const std::string& E);
    bool SetExpression(const Expression& E);
    bool SetRule(const std::string& R);

    bool SetVal(const double& v, const Expression::timing& tmg = Expression::timing::past, bool check_criteria = false);
    bool SetSource(const std::string& sourcename);

    void SetCorrespondingFlowVar(const std::string& s);
    std::string GetCorrespondingFlowVar() { return corresponding_flow_quan; }

    void SetCorrespondingInflowVar(const std::string& s);
    std::vector<std::string>& GetCorrespondingInflowVar() { return corresponding_inflow_quan; }

    void SetMassBalance(bool on);
    void SetParent(Object*);
    Object* GetParent();

    Quan* Corresponding_flow_variable = nullptr;

    void Renew();
    void Update();

    void SetIncludeInOutput(bool x) { includeinoutput = x; }
    void SetEstimable(bool x) { estimable = x; }
    std::string GetName() { return _var_name; }
    bool IncludeInOutput() { return includeinoutput; }

    bool SetTimeSeries(const std::string& filename, bool prec = false);
    bool SetTimeSeries(const TimeSeries<double>& timeseries);
    bool SetTimeSeries(const CPrecipitation& timeseries);

    std::string& Description(bool graph = false)
    {
        if (!graph)
            return description;
        else
            return description_graph;
    }

    std::string& HelpText() { return helptext; }
    std::string& Unit() { return unit; }
    std::string& Units() { return units; }
    std::string& DefaultUnit() { return default_unit; }
    std::string& Defaults() { return defaults; }
    std::string& Delegate() { return delegate; }
    bool Estimable() { return estimable; }
    std::string& Category() { return category; }
    std::string& Input() { return input; }
    std::string& Default() { return default_val; }
    bool& ExperimentDependent() { return experiment_dependent; }
    std::string& DescriptionCode() { return description_code; }
    std::string& Abbreviation() { return abbreviation; }
    Condition& Criteria() { return criteria; }
    std::string& WarningError() { return warning_error; }
    std::string& WarningMessage() { return warning_message; }
    std::string& InputType() { return input_type; }

    std::string ToString(int _tabs = 1) const;

    bool& AskFromUser() { return ask_from_user; }

    bool WhenCopied() {
        if (role == _role::copytoblocks || role == _role::copytolinks || role == _role::copytosources || role == _role::copytoreactions)
            return true;
        else {
            return false;
        }
    }

    void SetRole(const _role& r) { role = r; }
    _role GetRole() const { return role; }

    void SetName(const std::string& name) { _var_name = name; }
    bool AppendError(const std::string& objectname, const std::string& cls, const std::string& funct, const std::string& description, const int& code) const;
    bool SetProperty(const std::string& val, bool force_value = false, bool check_criteria = true);
    std::string GetProperty(bool force_value = false);

    std::string SourceName() { return sourcename; }
    bool SetSourceName(const std::string& s) { sourcename = s; return true; }

    std::string toCommand();

    void SetOutputItem(const std::string& s) { OutputItem = s; }
    std::string GetOutputItem() { return OutputItem; }

    void SetParameterAssignedTo(const std::string& s) { _parameterassignedto = s; }
    std::string GetParameterAssignedTo() { return _parameterassignedto; }

    bool Validate();

    bool HasCriteria() { if (criteria.Count() > 0) return true; else return false; }
    std::vector<std::string> GetAllRequieredStartingBlockProperties();
    std::vector<std::string> GetAllRequieredEndingBlockProperties();

    void Set_Value_Update(bool x) { value_star_updated = x; }
    bool Value_Updated() { return value_star_updated; }
    bool ApplyLimit() { return applylimit; }
    bool isrigid() { return rigid; }

    void SetInitialValueExpression(const std::string& expression);
    void SetInitialValueExpression(const Expression& expression);
    Expression& InitialValueExpression() { return initial_value_expression; }
    bool calcinivalue() const { return calculate_initial_value_from_expression; }

    std::vector<std::string> AllConstituents() const;
    std::vector<std::string> AllReactionParameters() const;
    bool RenameQuantity(const std::string& oldname, const std::string& newname);
    bool SetPrecalcIndependentVariable(const std::string& varname) { return precalcfunction.SetIndependentVariable(varname); }
    PreCalculatedFunction* PreCalcFunction() { return &precalcfunction; }
    double InterpolateBasedonPrecalcFunction(const double& val) const;
    bool InitializePreCalcFunction(int n_inc = 100);
    bool SetQuanPointers(Object* W)
    {
        if (type == _type::expression)
        {
            _expression.SetQuanPointers(W);
            return true;
        }

        return false;
    }

protected:

private:
    mutable std::mutex val_star_mutex;
    Expression _expression; ///< Mathematical expression used if type is expression
    Rule _rule; ///< Rule-based logic used if type is rule
    TimeSeries<timeseriesprecision> _timeseries; ///< Time series data for timeseries types
    Source* source = nullptr; ///< Pointer to external source object if type is source
    std::string sourcename = ""; ///< Name of the source as string
    std::string _var_name; ///< Name of the quantity
    std::string _string_value = ""; ///< Value for string type quantities
    double _val = 0; ///< Value at previous timestep
    double _val_star = 0; ///< Value at current timestep
    bool value_star_updated = false; ///< Indicates whether current value has been updated
    _type type = _type::not_assigned; ///< Type of the quantity
    bool perform_mass_balance = false; ///< Flag for applying mass balance
    std::string corresponding_flow_quan; ///< Name of the corresponding flow quantity
    SafeVector<std::string> corresponding_inflow_quan; ///< List of inflow quantities for balance type
    Object* parent = nullptr; ///< Pointer to the parent object
    bool includeinoutput = false; ///< Flag indicating inclusion in output
    bool estimable = false; ///< Indicates if the value is subject to estimation
    std::string description; ///< User-visible description
    std::string helptext; ///< Help text for UI or documentation
    std::string description_graph; ///< Graphical description (for plotting/visuals)
    std::string unit; ///< Display unit of the value
    std::string units; ///< Units (may include multiple options)
    std::string default_unit; ///< Default unit of measure
    std::string default_val = ""; ///< Default value as string
    std::string input_type; ///< Type of input expected (e.g., number, text)
    std::string defaults; ///< Optional default expressions or values
    std::string delegate; ///< UI control used (e.g., ValueBox, UnitBox)
    std::string category; ///< Category for grouping in UI or logic
    std::string input; ///< Raw input value as string
    bool experiment_dependent = false; ///< Flag for experiment-specific values
    std::string description_code; ///< Code-based description identifier
    std::string abbreviation; ///< Abbreviation for use in UI or charts
    Condition criteria; ///< Conditions the value must satisfy
    std::string warning_error; ///< Warning or error to display on validation failure
    std::string warning_message; ///< Custom warning message text
    bool ask_from_user = false; ///< Whether the value is prompted from user
    _role role = _role::none; ///< Role used for model-wide auto-duplication
    std::string OutputItem; ///< Output label or identifier
    std::string _parameterassignedto = ""; ///< Name of parameter this value is assigned to
    bool applylimit = false; ///< Whether bounds or limits should be enforced
    bool rigid = false; ///< If true, value is fixed and not adjustable
    bool calculate_initial_value_from_expression = false; ///< If true, derive initial value from expression
    Expression initial_value_expression; ///< Expression to calculate initial value
    PreCalculatedFunction precalcfunction; ///< Precomputed interpolation of function values
};

std::string tostring(const Quan::_type& typ);

#endif // QUAN_H