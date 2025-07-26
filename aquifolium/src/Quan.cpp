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


#include "Quan.h"
#include "Block.h"
#include "Link.h"
#include "System.h"
#include "Precipitation.h"
#include "Expression.h"
#ifdef Q_JSON_SUPPORT
#include "XString.h"
#endif
#ifndef mac_version
#ifndef NO_OPENMP
#include "omp.h"
#endif
#endif


// ========================================================================
// CONSTRUCTORS AND DESTRUCTOR
// ========================================================================

Quan::Quan()
: source(nullptr)
, _val(0.0)
, _val_star(0.0)
, value_star_updated(false)
, type(_type::not_assigned)
, perform_mass_balance(false)
, parent(nullptr)
, includeinoutput(false)
, estimable(false)
, experiment_dependent(false)
, ask_from_user(false)
, role(_role::none)
, applylimit(false)
, rigid(false)
, calculate_initial_value_from_expression(false)
{
    // Default constructor - all initialization done in member initializer list
}

Quan::Quan(const Quan& other)
    : _expression(other._expression)
    , _timeseries(other._timeseries)
    , _rule(other._rule)
    , source(nullptr) // Note: Don't copy the pointer, it needs to be re-established
    , sourcename(other.sourcename)
    , _var_name(other._var_name)
    , _string_value(other._string_value)
    , _val(other._val)
    , _val_star(other._val_star)
    , value_star_updated(other.value_star_updated)
    , _parameterassignedto(other._parameterassignedto)
    , type(other.type)
    , perform_mass_balance(other.perform_mass_balance)
    , corresponding_flow_quan(other.corresponding_flow_quan)
    , corresponding_inflow_quan(other.corresponding_inflow_quan)
    , parent(nullptr) // Note: Parent relationship needs to be re-established
    , includeinoutput(other.includeinoutput)
    , estimable(other.estimable)
    , description(other.description)
    , description_graph(other.description_graph)
    , helptext(other.helptext)
    , unit(other.unit)
    , units(other.units)
    , default_unit(other.default_unit)
    , default_val(other.default_val)
    , input_type(other.input_type)
    , defaults(other.defaults)
    , delegate(other.delegate)
    , category(other.category)
    , input(other.input)
    , experiment_dependent(other.experiment_dependent)
    , description_code(other.description_code)
    , abbreviation(other.abbreviation)
    , criteria(other.criteria)
    , warning_error(other.warning_error)
    , warning_message(other.warning_message)
    , ask_from_user(other.ask_from_user)
    , role(other.role)
    , OutputItem(other.OutputItem)
    , applylimit(other.applylimit)
    , rigid(other.rigid)
    , calculate_initial_value_from_expression(other.calculate_initial_value_from_expression)
    , initial_value_expression(other.initial_value_expression)
    , precalcfunction(other.precalcfunction)
    , Corresponding_flow_variable(nullptr) // Note: This pointer needs to be re-established
{
    // Copy constructor - all initialization done in member initializer list
    // Note: Pointers (source, parent, Corresponding_flow_variable) are not copied
    // and need to be re-established in the new context
}


Quan::~Quan()
{
    // Clear resources
    _timeseries.clear();
    precalcfunction.clear();

    // Note: We don't delete source or parent as they are not owned by this object
}

// ========================================================================
// JSON CONSTRUCTOR
// ========================================================================

Quan::Quan(Json::ValueIterator &it)
    : Quan() // Delegate to default constructor for basic initialization
{
    // Set the quantity name from JSON key
    SetName(it.key().asString());

    // Parse and set the quantity type
    parseQuantityType(it);

    // Parse output and estimation settings
    parseOutputSettings(it);

    // Parse behavioral flags
    parseBehavioralFlags(it);

    // Parse pre-calculation settings
    parsePreCalculationSettings(it);

    // Parse metadata (description, units, etc.)
    parseMetadata(it);

    // Parse validation criteria
    parseValidationCriteria(it);

    // Parse role and user interaction settings
    parseRoleAndUserSettings(it);

    // Set initial value if provided
    parseInitialValue(it);
}

// ========================================================================
// JSON PARSING HELPER METHODS
// ========================================================================

void Quan::parseQuantityType(Json::ValueIterator &it)
{
    const string typeStr = (*it)["type"].asString();

    if (typeStr == "balance") {
        SetType(_type::balance);
        SetCorrespondingFlowVar((*it)["flow"].asString());
        SetCorrespondingInflowVar((*it)["inflow"].asString());
    }
    else if (typeStr == "constant") {
        SetType(_type::constant);
    }
    else if (typeStr == "expression") {
        SetType(_type::expression);
        SetExpression((*it)["expression"].asString());
    }
    else if (typeStr == "rule") {
        SetType(_type::rule);
        parseRuleDefinition(it);
    }
    else if (typeStr == "global") {
        SetType(_type::global_quan);
    }
    else if (typeStr == "timeseries") {
        SetType(_type::timeseries);
    }
    else if (typeStr == "timeseries_prec") {
        SetType(_type::prec_timeseries);
    }
    else if (typeStr == "source") {
        SetType(_type::source);
    }
    else if (typeStr == "value") {
        SetType(_type::value);
    }
    else if (typeStr == "string") {
        SetType(_type::string);
    }
    else if (typeStr == "boolean") {
        SetType(_type::boolean);
    }
    else {
        // Default or unknown type handling
        SetType(_type::not_assigned);
    }
}

void Quan::parseRuleDefinition(Json::ValueIterator &it)
{
    for (Json::ValueIterator itrule = (*it)["rule"].begin();
         itrule != (*it)["rule"].end(); ++itrule) {
        GetRule()->Append(itrule.key().asString(), itrule->asString());
    }
}

void Quan::parseOutputSettings(Json::ValueIterator &it)
{
    // Parse includeinoutput flag
    if (it->isMember("includeinoutput")) {
        SetIncludeInOutput((*it)["includeinoutput"].asString() == "true");
    } else {
        SetIncludeInOutput(false);
    }

    // Parse estimate flag
    if (it->isMember("estimate")) {
        SetEstimable((*it)["estimate"].asString() == "true");
    } else {
        SetEstimable(false);
    }
}

void Quan::parseBehavioralFlags(Json::ValueIterator &it)
{
    // Parse apply limit flag
    if (it->isMember("applylimit")) {
        applylimit = ((*it)["applylimit"].asString() == "true");
    } else {
        applylimit = false;
    }

    // Parse rigid flag
    if (it->isMember("rigid")) {
        rigid = ((*it)["rigid"].asString() == "true");
    } else {
        rigid = false;
    }
}

void Quan::parsePreCalculationSettings(Json::ValueIterator &it)
{
    if (!it->isMember("precalcbasedon")) {
        return;
    }

    const vector<string> precalcParts = aquiutils::split((*it)["precalcbasedon"].asString(), ':');
    if (precalcParts.size() != 4) {
        return; // Invalid format
    }

    precalcfunction.SetIndependentVariable(precalcParts[0]);
    precalcfunction.SetLogarithmic(precalcParts[1] == "log");
    precalcfunction.setminmax(aquiutils::atof(precalcParts[2]),
                              aquiutils::atof(precalcParts[3]));
}

void Quan::parseMetadata(Json::ValueIterator &it)
{
    // Parse description
    if (it->isMember("description")) {
        parseDescription((*it)["description"].asString());
    }

    // Parse help text
    if (it->isMember("helptext")) {
        HelpText() = (*it)["helptext"].asString();
    }

    // Parse units
    if (it->isMember("unit") && !(*it)["unit"].asString().empty()) {
        Units() = (*it)["unit"].asString();
        Unit() = aquiutils::split(Units(), ';')[0];
    }

    // Parse other metadata fields
    parseOptionalStringField(it, "default_unit", DefaultUnit());
    parseOptionalStringField(it, "default", Default());
    parseOptionalStringField(it, "delegate", Delegate());
    parseOptionalStringField(it, "category", Category());
    parseOptionalStringField(it, "input", Input());
    parseOptionalStringField(it, "description_code", DescriptionCode());
    parseOptionalStringField(it, "inputtype", InputType());

    // Parse experiment dependent flag
    if (it->isMember("experiment_dependent")) {
        ExperimentDependent() = ((*it)["experiment_dependent"].asString() == "Yes");
    }

    // Parse initial value expression
    if (it->isMember("initial_value_expression")) {
        SetInitialValueExpression((*it)["initial_value_expression"].asString());
    }
}

void Quan::parseDescription(const string& descriptionStr)
{
    const vector<string> descParts = aquiutils::split(descriptionStr, ';');
    if (descParts.size() == 1) {
        Description() = descriptionStr;
        Description(true) = descriptionStr;
    } else if (descParts.size() == 2) {
        Description() = descParts[0];
        Description(true) = descParts[1];
    }
}

void Quan::parseOptionalStringField(Json::ValueIterator &it, const string& fieldName, string& target)
{
    if (it->isMember(fieldName)) {
        target = (*it)[fieldName].asString();
    }
}

void Quan::parseValidationCriteria(Json::ValueIterator &it)
{
    if (it->isMember("criteria")) {
        Criteria() = (*it)["criteria"].asString();
    }

    if (it->isMember("warningerror")) {
        WarningError() = (*it)["warningerror"].asString();
    }

    if (it->isMember("warningmessage")) {
        WarningMessage() = (*it)["warningmessage"].asString();
    }
}

void Quan::parseRoleAndUserSettings(Json::ValueIterator &it)
{
    // Parse ask user flag
    if (it->isMember("ask_user")) {
        AskFromUser() = (aquiutils::tolower((*it)["ask_user"].asString()) == "true");
    } else {
        AskFromUser() = false;
    }

    // Parse role
    if (it->isMember("role")) {
        parseRole((*it)["role"].asString());
    } else {
        SetRole(_role::none);
    }
}

void Quan::parseRole(const string& roleStr)
{
    const string lowerRole = aquiutils::tolower(roleStr);

    if (lowerRole == "copytoblocks") {
        SetRole(_role::copytoblocks);
    } else if (lowerRole == "copytolinks") {
        SetRole(_role::copytolinks);
    } else if (lowerRole == "copytosources") {
        SetRole(_role::copytosources);
    } else if (lowerRole == "copytoreactions") {
        SetRole(_role::copytoreactions);
    } else {
        SetRole(_role::none);
    }
}

void Quan::parseInitialValue(Json::ValueIterator &it)
{
    if (it->isMember("setvalue")) {
        SetProperty((*it)["setvalue"].asString());
    }
}

#ifdef Q_JSON_SUPPORT

// ========================================================================
// QT JSON CONSTRUCTOR
// ========================================================================

Quan::Quan(QJsonObject& qjobject)
    : Quan() // Delegate to default constructor for basic initialization
{
    // Parse and set the quantity type
    parseQuantityTypeQt(qjobject);

    // Parse output and estimation settings
    parseOutputSettingsQt(qjobject);

    // Parse behavioral flags
    parseBehavioralFlagsQt(qjobject);

    // Parse pre-calculation settings
    parsePreCalculationSettingsQt(qjobject);

    // Parse metadata (description, units, etc.)
    parseMetadataQt(qjobject);

    // Parse validation criteria
    parseValidationCriteriaQt(qjobject);

    // Parse role and user interaction settings
    parseRoleAndUserSettingsQt(qjobject);

    // Set initial value if provided
    parseInitialValueQt(qjobject);
}

// ========================================================================
// QT JSON PARSING HELPER METHODS
// ========================================================================

void Quan::parseQuantityTypeQt(QJsonObject& qjobject)
{
    if (!qjobject.keys().contains("type")) {
        SetType(_type::global_quan); // Default type
        return;
    }

    const string typeStr = qjobject.value("type").toString().toStdString();

    if (typeStr == "balance") {
        SetType(_type::balance);
        SetCorrespondingFlowVar(qjobject.value("flow").toString().toStdString());
        SetCorrespondingInflowVar(qjobject.value("inflow").toString().toStdString());
    }
    else if (typeStr == "constant") {
        SetType(_type::constant);
    }
    else if (typeStr == "boolean") {
        SetType(_type::boolean);
    }
    else if (typeStr == "expression") {
        SetType(_type::expression);
        SetExpression(qjobject.value("expression").toString().toStdString());
    }
    else if (typeStr == "rule") {
        SetType(_type::rule);
        parseRuleDefinitionQt(qjobject);
    }
    else if (typeStr == "global") {
        SetType(_type::global_quan);
    }
    else if (typeStr == "timeseries") {
        SetType(_type::timeseries);
    }
    else if (typeStr == "timeseries_prec") {
        SetType(_type::prec_timeseries);
    }
    else if (typeStr == "source") {
        SetType(_type::source);
    }
    else if (typeStr == "value") {
        SetType(_type::value);
    }
    else if (typeStr == "string") {
        SetType(_type::string);
    }
    else {
        SetType(_type::global_quan); // Default fallback
    }
}

void Quan::parseRuleDefinitionQt(QJsonObject& qjobject)
{
    QJsonObject ruleObject = qjobject.value("rule").toObject();
    for (auto itrule = ruleObject.begin(); itrule != ruleObject.end(); ++itrule) {
        GetRule()->Append(itrule.key().toStdString(),
                          itrule.value().toString().toStdString());
    }
}

void Quan::parseOutputSettingsQt(QJsonObject& qjobject)
{
    // Parse includeinoutput flag
    if (qjobject.keys().contains("includeinoutput")) {
        SetIncludeInOutput(qjobject.value("includeinoutput").toString().toStdString() == "true");
    } else {
        SetIncludeInOutput(false);
    }

    // Parse estimate flag
    if (qjobject.keys().contains("estimate")) {
        SetEstimable(qjobject.value("estimate").toString().toStdString() == "true");
    } else {
        SetEstimable(false);
    }
}

void Quan::parseBehavioralFlagsQt(QJsonObject& qjobject)
{
    // Parse rigid flag
    if (qjobject.keys().contains("rigid")) {
        rigid = (aquiutils::tolower(qjobject.value("rigid").toString().toStdString()) == "true");
    } else {
        rigid = false;
    }

    // Parse apply limit flag
    if (qjobject.keys().contains("applylimit")) {
        applylimit = (aquiutils::tolower(qjobject.value("applylimit").toString().toStdString()) == "true");
    } else {
        applylimit = false;
    }
}

void Quan::parsePreCalculationSettingsQt(QJsonObject& qjobject)
{
    if (!qjobject.keys().contains("precalcbasedon")) {
        return;
    }

    const vector<string> precalcParts = aquiutils::split(
        qjobject.value("precalcbasedon").toString().toStdString(), ':');

    if (precalcParts.size() != 4) {
        return; // Invalid format
    }

    precalcfunction.SetIndependentVariable(precalcParts[0]);
    precalcfunction.SetLogarithmic(precalcParts[1] == "log");
    precalcfunction.setminmax(aquiutils::atof(precalcParts[2]),
                              aquiutils::atof(precalcParts[3]));
}

void Quan::parseMetadataQt(QJsonObject& qjobject)
{
    // Parse description
    if (qjobject.keys().contains("description")) {
        parseDescription(qjobject.value("description").toString().toStdString());
    }

    // Parse help text
    if (qjobject.keys().contains("helptext")) {
        HelpText() = qjobject.value("helptext").toString().toStdString();
    }

    // Parse units
    if (qjobject.keys().contains("unit")) {
        const string unitStr = qjobject.value("unit").toString().toStdString();
        if (!unitStr.empty()) {
            Units() = unitStr;
            Unit() = aquiutils::split(Units(), ';')[0];
        }
    }

    // Parse other metadata fields using helper
    parseOptionalStringFieldQt(qjobject, "default_unit", DefaultUnit());
    parseOptionalStringFieldQt(qjobject, "default", Default());
    parseOptionalStringFieldQt(qjobject, "delegate", Delegate());
    parseOptionalStringFieldQt(qjobject, "category", Category());
    parseOptionalStringFieldQt(qjobject, "input", Input());
    parseOptionalStringFieldQt(qjobject, "description_code", DescriptionCode());
    parseOptionalStringFieldQt(qjobject, "inputtype", InputType());

    // Parse experiment dependent flag
    if (qjobject.keys().contains("experiment_dependent")) {
        ExperimentDependent() = (qjobject.value("experiment_dependent").toString().toStdString() == "Yes");
    }

    // Parse initial value expression
    if (qjobject.keys().contains("initial_value_expression")) {
        SetInitialValueExpression(qjobject.value("initial_value_expression").toString().toStdString());
    }
}

void Quan::parseOptionalStringFieldQt(QJsonObject& qjobject, const string& fieldName, string& target)
{
    if (qjobject.keys().contains(QString::fromStdString(fieldName))) {
        target = qjobject.value(QString::fromStdString(fieldName)).toString().toStdString();
    }
}

void Quan::parseValidationCriteriaQt(QJsonObject& qjobject)
{
    if (qjobject.keys().contains("criteria")) {
        Criteria() = qjobject.value("criteria").toString().toStdString();
    }

    parseOptionalStringFieldQt(qjobject, "warningerror", warning_error);
    parseOptionalStringFieldQt(qjobject, "warningmessage", warning_message);
}

void Quan::parseRoleAndUserSettingsQt(QJsonObject& qjobject)
{
    // Parse ask user flag
    if (qjobject.keys().contains("ask_user")) {
        AskFromUser() = (aquiutils::tolower(qjobject.value("ask_user").toString().toStdString()) == "true");
    } else {
        AskFromUser() = false;
    }

    // Parse role
    if (qjobject.keys().contains("role")) {
        parseRole(qjobject.value("role").toString().toStdString());
    } else {
        SetRole(_role::none);
    }
}

void Quan::parseInitialValueQt(QJsonObject& qjobject)
{
    if (qjobject.keys().contains("setvalue")) {
        SetProperty(qjobject.value("setvalue").toString().toStdString());
    }
}

#endif // Q_JSON_SUPPORT

Quan& Quan::operator=(const Quan& rhs)
{
    // Self-assignment check
    if (this == &rhs) {
        return *this;
    }

    // Copy all data members
    _expression = rhs._expression;
    _timeseries = rhs._timeseries;
    _rule = rhs._rule;
    sourcename = rhs.sourcename;
    _var_name = rhs._var_name;
    _string_value = rhs._string_value;
    _val = rhs._val;
    _val_star = rhs._val_star;
    value_star_updated = rhs.value_star_updated;
    _parameterassignedto = rhs._parameterassignedto;
    type = rhs.type;
    perform_mass_balance = rhs.perform_mass_balance;
    corresponding_flow_quan = rhs.corresponding_flow_quan;
    corresponding_inflow_quan = rhs.corresponding_inflow_quan;
    includeinoutput = rhs.includeinoutput;
    estimable = rhs.estimable;
    description = rhs.description;
    description_graph = rhs.description_graph;
    helptext = rhs.helptext;
    unit = rhs.unit;
    units = rhs.units;
    default_unit = rhs.default_unit;
    default_val = rhs.default_val;
    input_type = rhs.input_type;
    defaults = rhs.defaults;
    delegate = rhs.delegate;
    category = rhs.category;
    input = rhs.input;
    experiment_dependent = rhs.experiment_dependent;
    description_code = rhs.description_code;
    abbreviation = rhs.abbreviation;
    criteria = rhs.criteria;
    warning_error = rhs.warning_error;
    warning_message = rhs.warning_message;
    ask_from_user = rhs.ask_from_user;
    role = rhs.role;
    OutputItem = rhs.OutputItem;
    applylimit = rhs.applylimit;
    rigid = rhs.rigid;
    calculate_initial_value_from_expression = rhs.calculate_initial_value_from_expression;
    initial_value_expression = rhs.initial_value_expression;
    precalcfunction = rhs.precalcfunction;

    // Note: Don't copy pointers - they need to be re-established
    source = nullptr;
    parent = nullptr;
    Corresponding_flow_variable = nullptr;

    return *this;
}


// ========================================================================
// OPERATORS
// ========================================================================

bool Quan::operator==(const Quan& other)
{
    return (_val == other._val) &&
           (_val_star == other._val_star) &&
           (_var_name == other._var_name) &&
           (type == other.type);

}

// ========================================================================
// VALUE CALCULATION METHODS - REFACTORED
// ========================================================================

double Quan::CalcVal(Object *object, const Expression::timing &tmg)
{
    if (!object) {
        last_error = "Null object provided for calculation";
        return 0.0;
    }

    switch (type) {
    case _type::constant:
    case _type::boolean:
        return _val;

    case _type::expression:
        return _expression.calc(object, tmg);

    case _type::rule:
        return _rule.calc(object, tmg);

    case _type::timeseries:
    case _type::prec_timeseries:
        return calculateTimeSeriesValue(object);

    case _type::value:
        return _val;

    case _type::source:
        return calculateSourceValue(object);

    case _type::balance:
        return _val; // Balance quantities return stored value

    case _type::global_quan:
        return _val; // Global quantities return stored value

    case _type::string:
        return 0.0; // String types don't have numeric values

    case _type::not_assigned:
    default:
        last_error = "Quantity type not assigned or unknown";
        return 0.0;
    }
}

double Quan::CalcVal(const Expression::timing &tmg)
{
    if (!parent) {
        last_error = "No parent object set for calculation";
        return 0.0;
    }

    switch (type) {
    case _type::constant:
    case _type::boolean:
        return getValueByTiming(tmg);

    case _type::expression:
        return calculateExpressionValue(tmg);

    case _type::rule:
        return _rule.calc(parent, tmg);

    case _type::timeseries:
    case _type::prec_timeseries:
        return calculateTimeSeriesValueWithParent();

    case _type::value:
    case _type::balance:
        return getValueByTiming(tmg);

    case _type::source:
        return calculateSourceValueWithParent();

    case _type::global_quan:
        return getValueByTiming(tmg);

    case _type::string:
        return 0.0; // String types don't have numeric values

    case _type::not_assigned:
    default:
        last_error = "Quantity type not assigned or unknown";
        return 0.0;
    }
}

// ========================================================================
// CALCULATION HELPER METHODS
// ========================================================================

double Quan::getValueByTiming(const Expression::timing &tmg) const
{
    return (tmg == Expression::timing::past) ? _val : _val_star;
}

double Quan::calculateTimeSeriesValue(Object *object) const
{
    if (_timeseries.size() > 0) {
        return _timeseries.interpol(object->GetParent()->GetTime());
    }
    return 0.0;
}

double Quan::calculateTimeSeriesValueWithParent() const
{
    if (_timeseries.size() > 0) {
        return _timeseries.interpol(parent->GetParent()->GetTime());
    }
    return 0.0;
}

double Quan::calculateSourceValue(Object *object)
{
    if (!source) {
        return 0.0;
    }

    if (value_star_updated) {
        return _val_star;
    }

    // Thread-safe value update
    updateSourceValueThreadSafe(object);
    return _val_star;
}

double Quan::calculateSourceValueWithParent()
{
    if (!source) {
        return 0.0;
    }

    if (value_star_updated) {
        return _val_star;
    }

    // Thread-safe value update
    updateSourceValueThreadSafe(parent);
    return _val_star;
}

double Quan::calculateExpressionValue(const Expression::timing &tmg)
{
    // Check if we should use pre-calculated function for optimization
    if (shouldUsePrecalculatedFunction()) {
        const double independentValue = parent->GetVal(precalcfunction.IndependentVariable(), tmg);
        return InterpolateBasedonPrecalcFunction(independentValue);
    }

    // Standard expression calculation
    return _expression.calc(parent, tmg);
}

void Quan::updateSourceValueThreadSafe(Object *object)
{
#ifndef NO_OPENMP
    omp_lock_t writelock;
    if (omp_get_num_threads() > 1) {
        omp_init_lock(&writelock);
        omp_set_lock(&writelock);
    }
#endif

    _val_star = source->GetValue(object);
    value_star_updated = true;

#ifndef NO_OPENMP
    if (omp_get_num_threads() > 1) {
        omp_unset_lock(&writelock);
        omp_destroy_lock(&writelock);
    }
#endif
}

bool Quan::shouldUsePrecalculatedFunction() const
{
    return !precalcfunction.IndependentVariable().empty() &&
           precalcfunction.Initiated();
}

// ========================================================================
// GET VALUE METHOD - REFACTORED
// ========================================================================

double Quan::GetVal(const Expression::timing &tmg)
{
    // For past timing, always return stored value
    if (tmg == Expression::timing::past) {
        return _val;
    }

    // For present/future timing, check if already updated
    if (value_star_updated) {
        return _val_star;
    }

    // Calculate and cache the future value with thread safety
#ifndef NO_OPENMP
    omp_lock_t writelock;
    if (omp_get_num_threads() > 1) {
        omp_init_lock(&writelock);
        omp_set_lock(&writelock);
    }
#endif

    // Calculate value based on type
    switch (type) {
    case _type::constant:
    case _type::boolean:
    case _type::value:
    case _type::balance:
    case _type::global_quan:
        // These types don't need recalculation for future timing
        _val_star = _val;
        break;

    case _type::expression:
    case _type::rule:
        _val_star = CalcVal(tmg);
        break;

    case _type::timeseries:
    case _type::prec_timeseries:
        if (GetTimeSeries()) {
            _val_star = GetTimeSeries()->interpol(GetSimulationTime());
        } else {
            _val_star = 0.0;
        }
        break;

    case _type::source:
        if (source && parent) {
            _val_star = source->GetValue(parent);
        } else {
            _val_star = 0.0;
        }
        break;

    case _type::string:
    case _type::not_assigned:
    default:
        _val_star = 0.0;
        break;
    }

    value_star_updated = true;

#ifndef NO_OPENMP
    if (omp_get_num_threads() > 1) {
        omp_unset_lock(&writelock);
        omp_destroy_lock(&writelock);
    }
#endif

    return _val_star;
}


// ========================================================================
// VALUE SETTING WITH VALIDATION
// ========================================================================

bool Quan::SetVal(const double &v, const Expression::timing &tmg, bool check_criteria)
{
    const double past_val = _val;

    // Set values based on timing specification
    if (tmg == Expression::timing::past || tmg == Expression::timing::both) {
        _val = v;
    }
    if (tmg == Expression::timing::present || tmg == Expression::timing::both) {
#ifndef NO_OPENMP
        omp_lock_t writelock;
        if (omp_get_num_threads() > 1) {
            omp_init_lock(&writelock);
            omp_set_lock(&writelock);
        }
#endif
        _val_star = v;
        value_star_updated = true;
#ifndef NO_OPENMP
        if (omp_get_num_threads() > 1) {
            omp_unset_lock(&writelock);
            omp_destroy_lock(&writelock);
        }
#endif
    }

    // Validate against criteria if both timing is set and validation is requested
    if (tmg == Expression::timing::both && HasCriteria() && parent && check_criteria) {
        if (!Criteria().calc(parent, tmg)) {
            AppendError(parent->GetName(), "Quan", "SetVal", warning_message, 8012);
            _val = past_val;
            _val_star = past_val;
            return false;
        }
    }

    return true;
}

// ========================================================================
// PROPERTY MANAGEMENT
// ========================================================================

string Quan::GetProperty(bool force_value)
{
    switch (type) {
    case _type::balance:
    case _type::constant:
    case _type::global_quan:
    case _type::value:
        return aquiutils::numbertostring(GetVal(Expression::timing::present));

    case _type::expression:
        if (force_value) {
            return aquiutils::numbertostring(GetVal(Expression::timing::present));
        }
        return GetExpression()->ToString();

    case _type::timeseries:
    case _type::prec_timeseries: {
        const string& filename = _timeseries.getFilename();
        if (parent && parent->Parent() &&
            aquiutils::GetPath(filename) == aquiutils::GetPath(parent->Parent()->GetWorkingFolder())) {
            return aquiutils::GetOnlyFileName(filename);
        }
        return filename;
    }

    case _type::source:
        return sourcename;

    case _type::string:
        return _string_value;

    case _type::boolean:
        return (aquiutils::numbertostring(GetVal(Expression::timing::present)) == "1") ? "Yes" : "No";

    case _type::rule:
    case _type::not_assigned:
    default:
        return "";
    }
}

bool Quan::SetProperty(const string &val, bool force_value, bool check_criteria)
{
    switch (type) {
    case _type::balance:
    case _type::constant:
    case _type::global_quan:
    case _type::value:
        return SetVal(aquiutils::atof(val), Expression::timing::both, check_criteria);

    case _type::expression:
        if (force_value) {
            return SetVal(aquiutils::atof(val), Expression::timing::both, check_criteria);
        }
        _expression = val;
        return SetExpression(val);

    case _type::timeseries:
        if (val.empty()) {
            SetTimeSeries("");
            return false;
        }
        if (parent && parent->Parent() && !parent->Parent()->InputPath().empty() &&
            aquiutils::FileExists(parent->Parent()->InputPath() + val)) {
            return SetTimeSeries(parent->Parent()->InputPath() + val);
        }
        return SetTimeSeries(val);

    case _type::prec_timeseries:
        if (val.empty()) {
            SetTimeSeries("");
            return false;
        }
        if (parent && parent->Parent() && !parent->Parent()->InputPath().empty() &&
            aquiutils::FileExists(parent->Parent()->InputPath() + val)) {
            return SetTimeSeries(parent->Parent()->InputPath() + val, true);
        }
        return SetTimeSeries(val, true);

    case _type::source:
        sourcename = val;
        return SetSource(val);

    case _type::string: {
        bool outcome = true;
        if (GetName() == "name" && parent) {
            outcome = parent->SetName(val, false);
        }
        if (outcome) {
            _string_value = val;
            return true;
        }
        return false;
    }

    case _type::boolean:
        return SetVal((val == "1") ? 1.0 : 0.0, Expression::timing::both, check_criteria);

    case _type::rule:
        AppendError(GetName(), "Quan", "SetProperty", "Rule cannot be set during runtime", 3011);
        return false;

    case _type::not_assigned:
    default:
        _string_value = val;
        return SetVal(aquiutils::atof(val), Expression::timing::both, check_criteria);
    }
}

// ========================================================================
// TIME SERIES MANAGEMENT
// ========================================================================

bool Quan::SetTimeSeries(const string &filename, bool prec)
{
    if (filename.empty()) {
        _timeseries = TimeSeries<double>();
        return true;
    }

    if (!prec) {
        _timeseries.readfile(filename);
        if (_timeseries.fileNotFound) {
            AppendError(GetName(), "Quan", "SetTimeSeries", filename + " was not found!", 3001);
            return false;
        }
        return true;
    } else {
        // Handle precipitation time series
        CPrecipitation Prec;
        if (!CPrecipitation::isFileValid(filename)) {
            AppendError(GetName(), "Quan", "SetTimeSeries", filename + " is not a valid precipitation file", 3023);
            return false;
        }

        Prec.getfromfile(filename);
        _timeseries = Prec.getflow(1)[0];
        _timeseries.setFilename(Prec.filename);
        return true;
    }
}

bool Quan::SetTimeSeries(const TimeSeries<double> &timeseries)
{
    _timeseries = timeseries;
    return true;
}

bool Quan::SetTimeSeries(const CPrecipitation &timeseries)
{
    _timeseries = timeseries.getflow(1)[0];
    return true;
}

// ========================================================================
// EXPRESSION AND RULE SETTING
// ========================================================================

bool Quan::SetExpression(const string &E)
{
    _expression = E;
    return true;
}

bool Quan::SetExpression(const Expression &E)
{
    _expression = E;
    return true;
}

bool Quan::SetRule(const string &R)
{
    _expression = R;
    return true;
}

// ========================================================================
// VALUE STATE MANAGEMENT
// ========================================================================

void Quan::Renew()
{
    _val_star = _val;
}

void Quan::Update()
{
    _val = _val_star;
}

// ========================================================================
// VALIDATION
// ========================================================================

bool Quan::Validate()
{
    // Handle time series file validation
    if ((type == _type::timeseries || type == _type::prec_timeseries) &&
        !_timeseries.getFilename().empty()) {

        const string& filename = _timeseries.getFilename();
        bool needsInputPath = parent && parent->Parent() &&
                              !parent->Parent()->InputPath().empty() &&
                              aquiutils::GetPath(parent->Parent()->InputPath()) != aquiutils::GetPath(filename);

        if (needsInputPath && aquiutils::FileExists(parent->Parent()->InputPath() + filename)) {
            if (type == _type::timeseries) {
                return SetTimeSeries(parent->Parent()->InputPath() + filename);
            } else {
                return SetTimeSeries(parent->Parent()->InputPath() + filename, true);
            }
        } else {
            if (type == _type::timeseries) {
                return SetTimeSeries(filename);
            } else {
                return SetTimeSeries(filename, true);
            }
        }
    }

    // Validate against criteria
    return !parent || Criteria().calc(parent, Expression::timing::both);
}

// ========================================================================
// STRING REPRESENTATION
// ========================================================================

string Quan::ToString(int _tabs) const
{
    string out = aquiutils::tabs(_tabs) + _var_name + ":\n";
    out += aquiutils::tabs(_tabs) + "{\n";

    // Add type information
    switch (type) {
    case _type::constant:
        out += aquiutils::tabs(_tabs + 1) + "type: constant\n";
        break;
    case _type::balance:
        out += aquiutils::tabs(_tabs + 1) + "type: balance\n";
        break;
    case _type::expression:
        out += aquiutils::tabs(_tabs + 1) + "type: expression\n";
        out += aquiutils::tabs(_tabs + 1) + "expression: " + _expression.ToString() + "\n";
        break;
    case _type::global_quan:
        out += aquiutils::tabs(_tabs + 1) + "type: global_quantity\n";
        break;
    case _type::timeseries:
        out += aquiutils::tabs(_tabs + 1) + "type: time_series\n";
        break;
    case _type::prec_timeseries:
        out += aquiutils::tabs(_tabs + 1) + "type: precipitation_time_series\n";
        break;
    case _type::value:
        out += aquiutils::tabs(_tabs + 1) + "type: value\n";
        break;
    case _type::rule:
        out += aquiutils::tabs(_tabs + 1) + "type: rule\n";
        out += aquiutils::tabs(_tabs + 1) + "rule: " + _rule.ToString(_tabs) + "\n";
        break;
    case _type::source:
        out += aquiutils::tabs(_tabs + 1) + "type: source\n";
        if (source) {
            out += aquiutils::tabs(_tabs + 1) + "source: " + source->GetName() + "\n";
        } else {
            out += aquiutils::tabs(_tabs + 1) + "source: " + sourcename + "\n";
        }
        break;
    case _type::string:
        out += aquiutils::tabs(_tabs + 1) + "type: string\n";
        break;
    case _type::boolean:
        out += aquiutils::tabs(_tabs + 1) + "type: boolean\n";
        break;
    case _type::not_assigned:
    default:
        out += aquiutils::tabs(_tabs + 1) + "type: not_assigned\n";
        break;
    }

    // Add initial value expression if present
    if (calculate_initial_value_from_expression) {
        out += aquiutils::tabs(_tabs + 1) + "initial_value_expression: " + initial_value_expression.ToString() + "\n";
    }

    // Add current values
    out += aquiutils::tabs(_tabs + 1) + "val: " + aquiutils::numbertostring(_val) + "\n";
    out += aquiutils::tabs(_tabs + 1) + "val*: " + aquiutils::numbertostring(_val_star) + "\n";
    out += aquiutils::tabs(_tabs) + "}";

    return out;
}

double &Quan::GetSimulationTime() const
{
    return parent->GetParent()->GetSimulationTime();
}

TimeSeries<timeseriesprecision>* Quan::GetTimeSeries()
{
    if (_timeseries.size() != 0)
        return &_timeseries;
    else
        return nullptr;

}

bool Quan::EstablishExpressionStructure()
{
    if (type == _type::expression)
        _expression.ResetTermsSources();
    return true;
}


string tostring(const Quan::_type &typ)
{
    if (typ==Quan::_type::balance) return "Balance";
    if (typ==Quan::_type::constant) return "Constant";
    if (typ==Quan::_type::expression) return "Expression";
    if (typ==Quan::_type::global_quan) return "Global";
    if (typ==Quan::_type::rule) return "Rule";
    if (typ==Quan::_type::timeseries) return "TimeSeries";
    if (typ==Quan::_type::value) return "Value";
    if (typ==Quan::_type::source) return "Source";
    return "";
}



Expression* Quan::GetExpression()
{
    return &_expression;
}


Rule* Quan::GetRule()
{
    return &_rule;
}

Source* Quan::GetSource()
{
    return source;
}



void Quan::SetCorrespondingFlowVar(const string &s)
{
    corresponding_flow_quan = s;
}

void Quan::SetCorrespondingInflowVar(const string &s)
{
    corresponding_inflow_quan = SafeVector<string>::fromStdVector(aquiutils::split(s,','));
}



void Quan::SetMassBalance(bool on)
{
    perform_mass_balance = on;
}

void Quan::SetParent(Object *o)
{
    parent = o;
}

Object* Quan::GetParent() const
{
    return parent;
}



bool Quan::SetSource(const string &sourcename)
{
    if (sourcename.empty())
    {
        source = nullptr;
        return true;
    }
    if (parent->GetParent()->source(sourcename))
	{
        if (parent->GetParent())
            source = parent->GetParent()->source(sourcename);
        return true;
	}
	else
	{
        source = nullptr;
        AppendError(GetName(),"Quan", "Source", sourcename + " was not found!", 3062);
		return false;
	}

}



bool Quan::AppendError(const string &objectname, const string &cls, const string &funct, const string &description, const int &code) const
{
    if (!parent)
        return false;
    if (!parent->Parent())
        return false;
#pragma omp critical (quan_append_error)
    parent->Parent()->errorhandler.Append(objectname, cls, funct, description, code);
    return true;
}

string Quan::toCommand()
{
    const string name = GetName();

    if (delegate == "UnitBox") {
#ifdef Q_GUI_SUPPORT
        if (unit != default_unit) {
            const double coefficient = XString::coefficient(QString::fromStdString(unit));
            const double adjustedValue = atof(GetProperty(true).c_str()) / coefficient;
            return name + "=" + aquiutils::numbertostring(adjustedValue) + "[" + unit + "]";
        } else {
            return name + "=" + GetProperty(true);
        }
#else
        // Without Qt support, just return the property value
        return name + "=" + GetProperty(true);
#endif
    }

    // Handle value-based delegates
    if (delegate == "ValueBox") {
        return name + "=" + GetProperty(true);
    }

    // Default case - use property without forcing value
    return name + "=" + GetProperty(false);
}

vector<string> Quan::GetAllRequieredStartingBlockProperties()
{
    return _expression.GetAllRequieredStartingBlockProperties();

}

vector<string> Quan::GetAllRequieredEndingBlockProperties()
{
    return _expression.GetAllRequieredEndingBlockProperties();
}

void Quan::SetInitialValueExpression(const string &expression)
{
    calculate_initial_value_from_expression = true;
    initial_value_expression = expression;
}

void Quan::SetInitialValueExpression(const Expression &expression)
{
    calculate_initial_value_from_expression = true;
    initial_value_expression = expression;
}

vector<std::string> Quan::AllConstituents() const
{
    if (parent)
        return parent->AllConstituents();
    else
        return vector<string>();
}

vector<string> Quan::AllReactionParameters() const
{
    if (parent)
        return parent->AllReactionParameters();
    else
        return vector<string>();
}

bool Quan::RenameQuantity(const string &oldname, const string &newname)
{
    _expression.RenameQuantity(oldname,newname);
    initial_value_expression.RenameQuantity(oldname,newname);
    return false;
}

double Quan::InterpolateBasedonPrecalcFunction(const double &val) const
{
    if (precalcfunction.Logarithmic())
    {
        if (val<=0) return precalcfunction.getValue(0);
        return precalcfunction.interpol(log(val));
    }
    else
        return precalcfunction.interpol(val);
}
bool Quan::InitializePreCalcFunction(int n_inc)
{
    const double old_independent_variable_value = parent->GetVal(precalcfunction.IndependentVariable(),Expression::timing::present);
    if (parent==nullptr) return false;
    if (precalcfunction.IndependentVariable().empty()) return false;
    precalcfunction.clear();
    if (!precalcfunction.Logarithmic())
        for (double x=precalcfunction.xmin(); x<=precalcfunction.xmax(); x+=(precalcfunction.xmax()-precalcfunction.xmin())/double(n_inc))
            {
                parent->UnUpdateAllValues();
                parent->SetVal(precalcfunction.IndependentVariable(),x,Expression::timing::present);
                precalcfunction.append(x,CalcVal(Expression::timing::present));
            }
    else
        for (double x=log(precalcfunction.xmin()); x<=log(precalcfunction.xmax()); x+=(log(precalcfunction.xmax())-log(precalcfunction.xmin()))/double(n_inc))
            {
                parent->UnUpdateAllValues();
                parent->SetVal(precalcfunction.IndependentVariable(),exp(x),Expression::timing::present);
                precalcfunction.append(x,CalcVal(Expression::timing::present));
            }
    parent->SetVal(precalcfunction.IndependentVariable(),old_independent_variable_value,Expression::timing::present);
    precalcfunction.setStructured(true);
    precalcfunction.SetInitiated(true);
    return true;
}

