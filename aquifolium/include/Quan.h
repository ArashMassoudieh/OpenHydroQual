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

#ifndef QUAN_H
#define QUAN_H

#include "Expression.h"
#include "Rule.h"
#include "TimeSeries.h"
#include "precalculatedfunction.h"
#include "Condition.h"
#include <json/json.h>
#include <string>

#ifdef Q_JSON_SUPPORT
#include <qjsonobject.h>
#endif

#define timeseriesprecision double

// Forward declarations
class Block;
class Link;
class System;
class Object;
class Source;
class QuanSet;
class CPrecipitation;

/**
 * @brief The Quan class represents a versatile quantity/parameter in environmental modeling.
 *
 * The Quan class is a fundamental component that can represent different types of quantities:
 * - Constants: Fixed values that don't change during simulation
 * - Values: Variable values that can be modified
 * - Expressions: Mathematical expressions evaluated at runtime
 * - Time Series: Values that vary over time based on input data
 * - Rules: Conditional logic for determining values
 * - Sources: Values derived from external source objects
 * - Balance: Mass/volume balance quantities
 * - Global: System-wide quantities
 * - String: Text-based parameters
 * - Boolean: True/false flags
 *
 * Each Quan object maintains both current (_val) and future (_val_star) values to support
 * temporal modeling schemes. The class provides extensive metadata support for model
 * configuration, validation, and user interfaces.
 */
class Quan
{
public:
    // ========================================================================
    // ENUMERATIONS
    // ========================================================================

    /**
     * @brief Defines the type of quantity this object represents
     */
    enum class _type {
        constant,       ///< Fixed constant value
        value,          ///< Variable value that can be modified
        balance,        ///< Mass/volume balance quantity
        expression,     ///< Mathematical expression evaluated at runtime
        timeseries,     ///< Time-varying values from file/data
        prec_timeseries,///< Precipitation time series data
        global_quan,    ///< System-wide global quantity
        rule,           ///< Conditional rule-based value
        source,         ///< Value derived from external source
        string,         ///< Text-based parameter
        boolean,        ///< Boolean flag (true/false)
        not_assigned    ///< Uninitialized/undefined type
    };

    /**
     * @brief Defines the role/scope of the quantity in the modeling system
     */
    enum class _role {
        none,           ///< No specific role assigned
        copytoblocks,   ///< Copy this quantity to all blocks
        copytolinks,    ///< Copy this quantity to all links
        copytosources,  ///< Copy this quantity to all sources
        copytoreactions ///< Copy this quantity to all reactions
    };

    // ========================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // ========================================================================

    /**
     * @brief Default constructor
     */
    Quan();

    /**
     * @brief Copy constructor
     * @param other The Quan object to copy from
     */
    Quan(const Quan& other);

    /**
     * @brief Constructor from JSON iterator
     * @param it JSON value iterator containing quantity definition
     */
    Quan(Json::ValueIterator &it);

#ifdef Q_JSON_SUPPORT
    /**
     * @brief Constructor from Qt JSON object
     * @param qjobject Qt JSON object containing quantity definition
     */
    Quan(QJsonObject& qjobject);
#endif

    /**
     * @brief Virtual destructor
     */
    virtual ~Quan();

    // ========================================================================
    // OPERATORS
    // ========================================================================

    /**
     * @brief Assignment operator
     * @param other The Quan object to assign from
     * @return Reference to this object
     */
    Quan& operator=(const Quan& other);

    /**
     * @brief Equality comparison operator
     * @param other The Quan object to compare with
     * @return True if objects are equal, false otherwise
     */
    bool operator==(const Quan& other);

    // ========================================================================
    // VALUE CALCULATION AND RETRIEVAL
    // ========================================================================

    /**
     * @brief Calculate the value of this quantity for a specific object
     * @param object Pointer to the object context for evaluation
     * @param tmg Timing specification (past/present)
     * @return Calculated value
     */
    double CalcVal(Object *object, const Expression::timing &tmg = Expression::timing::past);

    /**
     * @brief Calculate the value using the parent object context
     * @param tmg Timing specification (past/present)
     * @return Calculated value
     */
    double CalcVal(const Expression::timing &tmg = Expression::timing::past);

    /**
     * @brief Get the current value without recalculation
     * @param tmg Timing specification (past/present)
     * @return Current value
     */
    double GetVal(const Expression::timing &tmg = Expression::timing::past);

    /**
     * @brief Set the value of this quantity
     * @param v New value to set
     * @param tmg Timing specification for when to set the value
     * @param check_criteria Whether to validate against criteria
     * @return True if successful, false if validation failed
     */
    bool SetVal(const double &v, const Expression::timing &tmg = Expression::timing::past, bool check_criteria = false);

    // ========================================================================
    // TYPE AND METADATA MANAGEMENT
    // ========================================================================

    /**
     * @brief Set the type of this quantity
     * @param t New type to assign
     */
    void SetType(const _type &t) { type = t; }

    /**
     * @brief Get the current type of this quantity
     * @return Current type
     */
    _type GetType() const { return type; }

    /**
     * @brief Set the role of this quantity in the system
     * @param r New role to assign
     */
    void SetRole(const _role &r) { role = r; }

    /**
     * @brief Get the current role of this quantity
     * @return Current role
     */
    _role GetRole() const { return role; }

    /**
     * @brief Check if this quantity should be copied when objects are created
     * @return True if quantity should be copied based on its role
     */
    bool WhenCopied() {
        return (role == _role::copytoblocks || role == _role::copytolinks ||
                role == _role::copytosources || role == _role::copytoreactions);
    }

    // ========================================================================
    // NAME AND IDENTIFICATION
    // ========================================================================

    /**
     * @brief Get the name/identifier of this quantity
     * @return Quantity name
     */
    const string& GetName() const { return _var_name; }

    /**
     * @brief Set the name/identifier of this quantity
     * @param name New name to assign
     */
    void SetName(const string &name) { _var_name = name; }

    /**
     * @brief Get string value for string-type quantities
     * @return String value
     */
    const string& GetStringValue() const { return _string_value; }

    // ========================================================================
    // EXPRESSION AND RULE MANAGEMENT
    // ========================================================================

    /**
     * @brief Set mathematical expression for expression-type quantities
     * @param E Expression string to parse and set
     * @return True if successful
     */
    bool SetExpression(const string &E);

    /**
     * @brief Set mathematical expression from Expression object
     * @param E Expression object to copy
     * @return True if successful
     */
    bool SetExpression(const Expression &E);

    /**
     * @brief Get pointer to the expression object
     * @return Pointer to internal Expression object
     */
    Expression* GetExpression();

    /**
     * @brief Set rule string for rule-type quantities
     * @param R Rule string to parse and set
     * @return True if successful
     */
    bool SetRule(const string &R);

    /**
     * @brief Get pointer to the rule object
     * @return Pointer to internal Rule object
     */
    Rule* GetRule();

    /**
     * @brief Establish the structure of expressions (resolve references)
     * @return True if successful
     */
    bool EstablishExpressionStructure();

    /**
     * @brief Set pointers for quantities referenced in expressions
     * @param W Pointer to object containing the referenced quantities
     * @return True if successful
     */
    bool SetQuanPointers(Object *W) {
        if (type == _type::expression) {
            _expression.SetQuanPointers(W);
            return true;
        }
        return false;
    }

    // ========================================================================
    // TIME SERIES MANAGEMENT
    // ========================================================================

    /**
     * @brief Set time series data from file
     * @param filename Path to time series file
     * @param prec True if this is precipitation data
     * @return True if successful
     */
    bool SetTimeSeries(const string &filename, bool prec = false);

    /**
     * @brief Set time series data from TimeSeries object
     * @param timeseries TimeSeries object to copy
     * @return True if successful
     */
    bool SetTimeSeries(const TimeSeries<double> &timeseries);

    /**
     * @brief Set time series data from precipitation object
     * @param timeseries Precipitation object to extract time series from
     * @return True if successful
     */
    bool SetTimeSeries(const CPrecipitation &timeseries);

    /**
     * @brief Get pointer to the time series object
     * @return Pointer to TimeSeries object, nullptr if not applicable
     */
    TimeSeries<timeseriesprecision>* GetTimeSeries();

    /**
     * @brief Get reference to simulation time from parent system
     * @return Reference to simulation time
     */
    double &GetSimulationTime() const;

    // ========================================================================
    // SOURCE MANAGEMENT
    // ========================================================================

    /**
     * @brief Set external source for source-type quantities
     * @param sourcename Name of the source object
     * @return True if successful
     */
    bool SetSource(const string &sourcename);

    /**
     * @brief Get pointer to the source object
     * @return Pointer to Source object
     */
    Source* GetSource();

    /**
     * @brief Get the name of the associated source
     * @return Source name
     */
    const string& SourceName() const { return sourcename; }

    /**
     * @brief Set the source name
     * @param s Source name to set
     * @return True if successful
     */
    bool SetSourceName(const string& s) { sourcename = s; return true; }

    // ========================================================================
    // MASS BALANCE AND FLOW VARIABLES
    // ========================================================================

    /**
     * @brief Set the corresponding flow variable for balance quantities
     * @param s Name of the flow variable
     */
    void SetCorrespondingFlowVar(const string &s);

    /**
     * @brief Get the corresponding flow variable name
     * @return Flow variable name
     */
    const string& GetCorrespondingFlowVar() const { return corresponding_flow_quan; }

    /**
     * @brief Set corresponding inflow variables for balance quantities
     * @param s Comma-separated list of inflow variable names
     */
    void SetCorrespondingInflowVar(const string &s);

    /**
     * @brief Get list of corresponding inflow variables
     * @return Vector of inflow variable names
     */
    const vector<string>& GetCorrespondingInflowVar() const { return corresponding_inflow_quan; }

    /**
     * @brief Enable/disable mass balance calculation
     * @param on True to enable mass balance
     */
    void SetMassBalance(bool on);

    /**
     * @brief Pointer to corresponding flow variable object
     */
    Quan *Corresponding_flow_variable = nullptr;

    // ========================================================================
    // PARENT OBJECT MANAGEMENT
    // ========================================================================

    /**
     * @brief Set the parent object that owns this quantity
     * @param parent Pointer to parent object
     */
    void SetParent(Object *parent);

    /**
     * @brief Get the parent object
     * @return Pointer to parent object
     */
    Object* GetParent() const;

    // ========================================================================
    // VALUE STATE MANAGEMENT
    // ========================================================================

    /**
     * @brief Reset future value to current value
     */
    void Renew();

    /**
     * @brief Update current value from future value
     */
    void Update();

    /**
     * @brief Set whether the future value has been updated
     * @param x True if value has been updated
     */
    void Set_Value_Update(bool x) { value_star_updated = x; }

    /**
     * @brief Check if future value has been updated
     * @return True if future value is updated
     */
    bool Value_Updated() const { return value_star_updated; }

    // ========================================================================
    // OUTPUT AND ESTIMATION SETTINGS
    // ========================================================================

    /**
     * @brief Set whether this quantity should be included in output
     * @param x True to include in output
     */
    void SetIncludeInOutput(bool x) { includeinoutput = x; }

    /**
     * @brief Check if quantity is included in output
     * @return True if included in output
     */
    bool IncludeInOutput() const { return includeinoutput; }

    /**
     * @brief Set whether this quantity can be estimated/calibrated
     * @param x True if estimable
     */
    void SetEstimable(bool x) { estimable = x; }

    /**
     * @brief Check if quantity is estimable
     * @return True if estimable
     */
    bool Estimable() const { return estimable; }

    // ========================================================================
    // METADATA ACCESSORS
    // ========================================================================

    /**
     * @brief Get description text
     * @param graph True to get graph-specific description
     * @return Description string
     */
    string &Description(bool graph = false) {
        return graph ? description_graph : description;
    }

    /**
     * @brief Get help text
     * @return Help text string
     */
    string &HelpText() { return helptext; }

    /**
     * @brief Get primary unit
     * @return Unit string
     */
    string &Unit() { return unit; }

    /**
     * @brief Get all possible units
     * @return Units string (may contain multiple units separated by semicolons)
     */
    string &Units() { return units; }

    /**
     * @brief Get default unit
     * @return Default unit string
     */
    string &DefaultUnit() { return default_unit; }

    /**
     * @brief Get default value
     * @return Default value string
     */
    string &Default() { return default_val; }

    /**
     * @brief Get defaults specification
     * @return Defaults string
     */
    string &Defaults() { return defaults; }

    /**
     * @brief Get UI delegate type
     * @return Delegate string
     */
    string &Delegate() { return delegate; }

    /**
     * @brief Get category classification
     * @return Category string
     */
    string &Category() { return category; }

    /**
     * @brief Get input specification
     * @return Input string
     */
    string &Input() { return input; }

    /**
     * @brief Get input type specification
     * @return Input type string
     */
    string &InputType() { return input_type; }

    /**
     * @brief Check if quantity is experiment dependent
     * @return Reference to experiment dependent flag
     */
    bool &ExperimentDependent() { return experiment_dependent; }

    /**
     * @brief Get description code
     * @return Description code string
     */
    string &DescriptionCode() { return description_code; }

    /**
     * @brief Get abbreviation
     * @return Abbreviation string
     */
    string &Abbreviation() { return abbreviation; }

    /**
     * @brief Get warning/error specification
     * @return Warning error string
     */
    string &WarningError() { return warning_error; }

    /**
     * @brief Get warning message
     * @return Warning message string
     */
    string &WarningMessage() { return warning_message; }

    /**
     * @brief Check if quantity should prompt user for input
     * @return Reference to ask from user flag
     */
    bool &AskFromUser() { return ask_from_user; }

    // ========================================================================
    // VALIDATION AND CRITERIA
    // ========================================================================

    /**
     * @brief Get validation criteria
     * @return Reference to Condition object
     */
    Condition &Criteria() { return criteria; }

    /**
     * @brief Check if quantity has validation criteria
     * @return True if criteria are defined
     */
    bool HasCriteria() const { return criteria.Count() > 0; }

    /**
     * @brief Validate the quantity against its criteria
     * @return True if validation passes
     */
    bool Validate();

    // ========================================================================
    // PROPERTY MANAGEMENT
    // ========================================================================

    /**
     * @brief Set property value from string
     * @param val String representation of value
     * @param force_value Force interpretation as numeric value
     * @param check_criteria Whether to validate against criteria
     * @return True if successful
     */
    bool SetProperty(const string &val, bool force_value = false, bool check_criteria = true);

    /**
     * @brief Get property value as string
     * @param force_value Force return of numeric value
     * @return String representation of property
     */
    string GetProperty(bool force_value = false);

    // ========================================================================
    // OUTPUT AND SERIALIZATION
    // ========================================================================

    /**
     * @brief Convert quantity to string representation
     * @param _tabs Number of tabs for indentation
     * @return String representation
     */
    string ToString(int _tabs = 1) const;

    /**
     * @brief Convert to command string format
     * @return Command string
     */
    string toCommand();

    /**
     * @brief Set output item specification
     * @param s Output item string
     */
    void SetOutputItem(const string& s) { OutputItem = s; }

    /**
     * @brief Get output item specification
     * @return Output item string
     */
    const string& GetOutputItem() const { return OutputItem; }

    // ========================================================================
    // PARAMETER ASSIGNMENT
    // ========================================================================

    /**
     * @brief Set parameter assignment target
     * @param s Parameter assignment string
     */
    void SetParameterAssignedTo(const string &s) { _parameterassignedto = s; }

    /**
     * @brief Get parameter assignment target
     * @return Parameter assignment string
     */
    const string& GetParameterAssignedTo() const { return _parameterassignedto; }

    // ========================================================================
    // DEPENDENCY ANALYSIS
    // ========================================================================

    /**
     * @brief Get all required starting block properties
     * @return Vector of property names
     */
    vector<string> GetAllRequieredStartingBlockProperties();

    /**
     * @brief Get all required ending block properties
     * @return Vector of property names
     */
    vector<string> GetAllRequieredEndingBlockProperties();

    /**
     * @brief Get all constituents referenced by this quantity
     * @return Vector of constituent names
     */
    vector<string> AllConstituents() const;

    /**
     * @brief Get all reaction parameters referenced by this quantity
     * @return Vector of parameter names
     */
    vector<string> AllReactionParameters() const;

    // ========================================================================
    // QUANTITY RENAMING
    // ========================================================================

    /**
     * @brief Rename a referenced quantity throughout this object
     * @param oldname Current name to replace
     * @param newname New name to use
     * @return True if successful
     */
    bool RenameQuantity(const string &oldname, const string &newname);

    // ========================================================================
    // BEHAVIORAL FLAGS
    // ========================================================================

    /**
     * @brief Check if limits should be applied to this quantity
     * @return True if limits should be applied
     */
    bool ApplyLimit() const { return applylimit; }

    /**
     * @brief Check if quantity is rigid (cannot be modified)
     * @return True if rigid
     */
    bool isrigid() const { return rigid; }

    // ========================================================================
    // INITIAL VALUE EXPRESSIONS
    // ========================================================================

    /**
     * @brief Set initial value expression from string
     * @param expression Expression string for initial value calculation
     */
    void SetInitialValueExpression(const string &expression);

    /**
     * @brief Set initial value expression from Expression object
     * @param expression Expression object for initial value calculation
     */
    void SetInitialValueExpression(const Expression &expression);

    /**
     * @brief Get reference to initial value expression
     * @return Reference to Expression object
     */
    Expression &InitialValueExpression() { return initial_value_expression; }

    /**
     * @brief Check if initial value should be calculated from expression
     * @return True if initial value uses expression
     */
    bool calcinivalue() const { return calculate_initial_value_from_expression; }

    // ========================================================================
    // PRE-CALCULATED FUNCTIONS
    // ========================================================================

    /**
     * @brief Set independent variable for pre-calculated function
     * @param varname Name of independent variable
     * @return True if successful
     */
    bool SetPrecalcIndependentVariable(const string &varname) {
        return precalcfunction.SetIndependentVariable(varname);
    }

    /**
     * @brief Get pointer to pre-calculated function
     * @return Pointer to PreCalculatedFunction object
     */
    PreCalculatedFunction* PreCalcFunction() { return &precalcfunction; }

    /**
     * @brief Get const pointer to pre-calculated function
     * @return Const pointer to PreCalculatedFunction object
     */
    const PreCalculatedFunction* PreCalcFunction() const { return &precalcfunction; }

    /**
     * @brief Interpolate value based on pre-calculated function
     * @param val Independent variable value
     * @return Interpolated result
     */
    double InterpolateBasedonPrecalcFunction(const double &val) const;

    /**
     * @brief Initialize pre-calculated function with specified increments
     * @param n_inc Number of increments for function tabulation
     * @return True if successful
     */
    bool InitializePreCalcFunction(int n_inc = 100);

    // ========================================================================
    // ERROR HANDLING
    // ========================================================================

    /**
     * @brief Append error to parent system's error handler
     * @param objectname Name of object where error occurred
     * @param cls Class name where error occurred
     * @param funct Function name where error occurred
     * @param description Error description
     * @param code Error code
     * @return True if error was logged
     */
    bool AppendError(const string &objectname, const string &cls, const string &funct,
                     const string &description, const int &code) const;

    /**
     * @brief Last error message
     */
    string last_error;

private:
    // ========================================================================
    // CORE DATA MEMBERS
    // ========================================================================

    Expression _expression;                    ///< Mathematical expression for expression-type quantities
    Rule _rule;                               ///< Rule logic for rule-type quantities
    TimeSeries<timeseriesprecision> _timeseries; ///< Time series data
    Source *source = nullptr;                 ///< Pointer to external source object
    string sourcename = "";                   ///< Name of associated source
    string _var_name;                         ///< Variable name/identifier
    string _string_value = "";                ///< String value for string-type quantities

    // Value storage
    double _val = 0;                          ///< Current value (past/present)
    double _val_star = 0;                     ///< Future value for temporal schemes
    bool value_star_updated = false;          ///< Flag indicating if future value is updated

    // Type and behavior
    _type type = _type::not_assigned;         ///< Type of this quantity
    _role role = _role::none;                 ///< Role in the modeling system
    bool perform_mass_balance = false;        ///< Whether to perform mass balance calculations

    // Flow and balance relationships
    string corresponding_flow_quan;           ///< Name of corresponding flow variable
    SafeVector<string> corresponding_inflow_quan; ///< Names of corresponding inflow variables

    // Parent relationship
    Object *parent = nullptr;                 ///< Pointer to parent object

    // Output and estimation settings
    bool includeinoutput = false;             ///< Include in output flag
    bool estimable = false;                   ///< Can be estimated/calibrated flag

    // Metadata
    string description;                       ///< Description text
    string description_graph;                 ///< Graph-specific description
    string helptext;                          ///< Help text
    string unit;                              ///< Primary unit
    string units;                             ///< All possible units
    string default_unit;                      ///< Default unit
    string default_val = "";                  ///< Default value
    string input_type;                        ///< Input type specification
    string defaults;                          ///< Defaults specification
    string delegate;                          ///< UI delegate type
    string category;                          ///< Category classification
    string input;                             ///< Input specification
    bool experiment_dependent = false;        ///< Experiment dependent flag
    string description_code;                  ///< Description code
    string abbreviation;                      ///< Abbreviation

    // Validation and criteria
    Condition criteria;                       ///< Validation criteria
    string warning_error;                     ///< Warning/error specification
    string warning_message;                   ///< Warning message
    bool ask_from_user = false;               ///< Ask user for input flag

    // Output and parameter assignment
    string OutputItem;                        ///< Output item specification
    string _parameterassignedto = "";         ///< Parameter assignment target

    // Behavioral flags
    bool applylimit = false;                  ///< Apply limits flag
    bool rigid = false;                       ///< Rigid (non-modifiable) flag

    // Initial value expression
    bool calculate_initial_value_from_expression = false; ///< Use expression for initial value
    Expression initial_value_expression;      ///< Expression for initial value calculation

    // Pre-calculated function
    PreCalculatedFunction precalcfunction;    ///< Pre-calculated function for optimization

    // ========================================================================
    // JSON PARSING HELPER METHODS
    // ========================================================================

    /**
     * @brief Parse quantity type from JSON and configure type-specific settings
     * @param it JSON value iterator containing type information
     *
     * Handles parsing of the "type" field and sets up type-specific configuration:
     * - balance: Sets flow and inflow variables
     * - expression: Parses and sets the mathematical expression
     * - rule: Delegates to parseRuleDefinition() for rule parsing
     * - Other types: Sets appropriate type enum value
     */
    void parseQuantityType(Json::ValueIterator &it);

    /**
     * @brief Parse rule definition from JSON for rule-type quantities
     * @param it JSON value iterator containing rule information
     *
     * Iterates through the "rule" JSON object and appends each
     * condition-result pair to the internal Rule object.
     */
    void parseRuleDefinition(Json::ValueIterator &it);

    /**
     * @brief Parse output and estimation settings from JSON
     * @param it JSON value iterator containing output settings
     *
     * Handles parsing of:
     * - "includeinoutput": Whether quantity should be included in output
     * - "estimate": Whether quantity can be estimated/calibrated
     * Sets default values if fields are missing.
     */
    void parseOutputSettings(Json::ValueIterator &it);

    /**
     * @brief Parse behavioral flags from JSON
     * @param it JSON value iterator containing behavioral flags
     *
     * Handles parsing of:
     * - "applylimit": Whether limits should be applied to this quantity
     * - "rigid": Whether quantity is rigid (non-modifiable)
     * Sets default values (false) if fields are missing.
     */
    void parseBehavioralFlags(Json::ValueIterator &it);

    /**
     * @brief Parse pre-calculation function settings from JSON
     * @param it JSON value iterator containing pre-calculation settings
     *
     * Parses "precalcbasedon" field with format: "variable:log|linear:min:max"
     * Sets up the PreCalculatedFunction with:
     * - Independent variable name
     * - Logarithmic or linear scaling
     * - Minimum and maximum values for function domain
     * Validates format and ignores invalid specifications.
     */
    void parsePreCalculationSettings(Json::ValueIterator &it);

    /**
     * @brief Parse metadata fields from JSON
     * @param it JSON value iterator containing metadata
     *
     * Handles parsing of all metadata fields including:
     * - description, helptext, units, default values
     * - categories, input specifications, delegates
     * - experiment dependency flags
     * - initial value expressions
     * Delegates to specialized helper methods for complex fields.
     */
    void parseMetadata(Json::ValueIterator &it);

    /**
     * @brief Parse description field which may contain multiple parts
     * @param descriptionStr Description string from JSON
     *
     * Handles description format: "main_description;graph_description"
     * - Single part: Sets both description and graph description to same value
     * - Two parts: Sets description and graph description separately
     */
    void parseDescription(const string& descriptionStr);

    /**
     * @brief Parse optional string field from JSON if it exists
     * @param it JSON value iterator to check for field
     * @param fieldName Name of the JSON field to parse
     * @param target Reference to string variable to store result
     *
     * Helper method to reduce code duplication for optional string fields.
     * Only sets target if the field exists in the JSON object.
     */
    void parseOptionalStringField(Json::ValueIterator &it, const string& fieldName, string& target);

    /**
     * @brief Parse validation criteria and warning settings from JSON
     * @param it JSON value iterator containing validation settings
     *
     * Handles parsing of:
     * - "criteria": Validation condition expression
     * - "warningerror": Warning/error type specification
     * - "warningmessage": Warning message text
     */
    void parseValidationCriteria(Json::ValueIterator &it);

    /**
     * @brief Parse role and user interaction settings from JSON
     * @param it JSON value iterator containing role and user settings
     *
     * Handles parsing of:
     * - "ask_user": Whether to prompt user for input
     * - "role": Quantity role in the system (copy behavior)
     * Delegates role parsing to parseRole() for cleaner logic.
     */
    void parseRoleAndUserSettings(Json::ValueIterator &it);

    /**
     * @brief Parse role string and convert to appropriate enum value
     * @param roleStr Role string from JSON
     *
     * Converts string values to _role enum:
     * - "copytoblocks" → _role::copytoblocks
     * - "copytolinks" → _role::copytolinks
     * - "copytosources" → _role::copytosources
     * - "copytoreactions" → _role::copytoreactions
     * - Other values → _role::none
     * Performs case-insensitive comparison.
     */
    void parseRole(const string& roleStr);

    /**
     * @brief Parse and set initial value from JSON
     * @param it JSON value iterator containing initial value
     *
     * Handles parsing of "setvalue" field and applies it using
     * the SetProperty() method for proper type-specific handling.
     */
    void parseInitialValue(Json::ValueIterator &it);

#ifdef Q_JSON_SUPPORT
    // ========================================================================
    // QT JSON PARSING HELPER METHODS
    // ========================================================================

    /**
     * @brief Parse quantity type from Qt JSON object and configure type-specific settings
     * @param qjobject Qt JSON object containing type information
     *
     * Similar to parseQuantityType() but works with Qt JSON objects.
     * Handles parsing of the "type" field and sets up type-specific configuration.
     * Sets default type to global_quan if "type" field is missing.
     */
    void parseQuantityTypeQt(QJsonObject& qjobject);

    /**
     * @brief Parse rule definition from Qt JSON object for rule-type quantities
     * @param qjobject Qt JSON object containing rule information
     *
     * Iterates through the "rule" JSON object and appends each
     * condition-result pair to the internal Rule object.
     * Works with Qt JSON object iteration patterns.
     */
    void parseRuleDefinitionQt(QJsonObject& qjobject);

    /**
     * @brief Parse output and estimation settings from Qt JSON object
     * @param qjobject Qt JSON object containing output settings
     *
     * Handles parsing of:
     * - "includeinoutput": Whether quantity should be included in output
     * - "estimate": Whether quantity can be estimated/calibrated
     * Sets default values if fields are missing.
     */
    void parseOutputSettingsQt(QJsonObject& qjobject);

    /**
     * @brief Parse behavioral flags from Qt JSON object
     * @param qjobject Qt JSON object containing behavioral flags
     *
     * Handles parsing of:
     * - "rigid": Whether quantity is rigid (non-modifiable)
     * - "applylimit": Whether limits should be applied to this quantity
     * Uses case-insensitive comparison and sets default values (false) if fields are missing.
     */
    void parseBehavioralFlagsQt(QJsonObject& qjobject);

    /**
     * @brief Parse pre-calculation function settings from Qt JSON object
     * @param qjobject Qt JSON object containing pre-calculation settings
     *
     * Parses "precalcbasedon" field with format: "variable:log|linear:min:max"
     * Sets up the PreCalculatedFunction with independent variable, scaling, and domain.
     * Validates format and ignores invalid specifications.
     */
    void parsePreCalculationSettingsQt(QJsonObject& qjobject);

    /**
     * @brief Parse metadata fields from Qt JSON object
     * @param qjobject Qt JSON object containing metadata
     *
     * Handles parsing of all metadata fields including description, units,
     * categories, input specifications, and experiment dependency flags.
     * Uses Qt-specific helper methods for field extraction.
     */
    void parseMetadataQt(QJsonObject& qjobject);

    /**
     * @brief Parse optional string field from Qt JSON object if it exists
     * @param qjobject Qt JSON object to check for field
     * @param fieldName Name of the JSON field to parse
     * @param target Reference to string variable to store result
     *
     * Helper method to reduce code duplication for optional string fields.
     * Handles Qt string conversion and only sets target if field exists.
     */
    void parseOptionalStringFieldQt(QJsonObject& qjobject, const string& fieldName, string& target);

    /**
     * @brief Parse validation criteria and warning settings from Qt JSON object
     * @param qjobject Qt JSON object containing validation settings
     *
     * Handles parsing of:
     * - "criteria": Validation condition expression
     * - "warningerror": Warning/error type specification
     * - "warningmessage": Warning message text
     */
    void parseValidationCriteriaQt(QJsonObject& qjobject);

    /**
     * @brief Parse role and user interaction settings from Qt JSON object
     * @param qjobject Qt JSON object containing role and user settings
     *
     * Handles parsing of:
     * - "ask_user": Whether to prompt user for input
     * - "role": Quantity role in the system (copy behavior)
     * Uses case-insensitive comparison and shared parseRole() method.
     */
    void parseRoleAndUserSettingsQt(QJsonObject& qjobject);

    /**
     * @brief Parse and set initial value from Qt JSON object
     * @param qjobject Qt JSON object containing initial value
     *
     * Handles parsing of "setvalue" field and applies it using
     * the SetProperty() method for proper type-specific handling.
     */
    void parseInitialValueQt(QJsonObject& qjobject);

#endif // Q_JSON_SUPPORT

    // ========================================================================
    // CALCULATION HELPER METHODS
    // ========================================================================

    /**
     * @brief Get value based on timing specification
     * @param tmg Timing specification (past/present)
     * @return Value from appropriate time (past: _val, present: _val_star)
     *
     * Helper method to centralize the logic for choosing between
     * current and future values based on timing enum.
     */
    double getValueByTiming(const Expression::timing &tmg) const;

    /**
     * @brief Calculate time series value using provided object context
     * @param object Object context for getting simulation time
     * @return Interpolated value from time series, or 0.0 if no data
     *
     * Performs time series interpolation using the object's parent system time.
     * Returns 0.0 if time series is empty.
     */
    double calculateTimeSeriesValue(Object *object) const;

    /**
     * @brief Calculate time series value using parent object context
     * @return Interpolated value from time series, or 0.0 if no data
     *
     * Performs time series interpolation using the parent object's system time.
     * Returns 0.0 if time series is empty or parent is null.
     */
    double calculateTimeSeriesValueWithParent() const;

    /**
     * @brief Calculate source value with thread-safe caching
     * @param object Object context for source value calculation
     * @return Source value, cached value if already updated, or 0.0 if no source
     *
     * Handles source value calculation with thread-safe caching.
     * Returns cached _val_star if already updated, otherwise calculates
     * new value and updates cache.
     */
    double calculateSourceValue(Object *object);

    /**
     * @brief Calculate source value using parent context with thread-safe caching
     * @return Source value, cached value if already updated, or 0.0 if no source
     *
     * Similar to calculateSourceValue() but uses the parent object context.
     * Handles thread-safe caching of source values.
     */
    double calculateSourceValueWithParent();

    /**
     * @brief Calculate expression value with pre-calculation optimization
     * @param tmg Timing specification for expression evaluation
     * @return Calculated expression value
     *
     * Evaluates mathematical expressions with optimization support.
     * Uses pre-calculated function interpolation if available and configured,
     * otherwise falls back to standard expression evaluation.
     */
    double calculateExpressionValue(const Expression::timing &tmg);

    /**
     * @brief Update source value in a thread-safe manner
     * @param object Object context for source value calculation
     *
     * Handles thread-safe updating of _val_star from source object.
     * Uses OpenMP locks when multi-threading is enabled.
     * Sets value_star_updated flag after successful update.
     */
    void updateSourceValueThreadSafe(Object *object);

    /**
     * @brief Check if pre-calculated function should be used for optimization
     * @return True if pre-calculated function is available and initiated
     *
     * Determines whether expression evaluation should use the pre-calculated
     * function for performance optimization. Checks that independent variable
     * is set and function has been properly initiated.
     */
    bool shouldUsePrecalculatedFunction() const;

};

/**
 * @brief Convert quantity type enum to string representation
 * @param typ Quantity type to convert
 * @return String representation of the type
 */
string tostring(const Quan::_type &typ);

#endif // QUAN_H
