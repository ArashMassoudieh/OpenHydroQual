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
#ifdef Q_GUI_SUPPORT
    #include "XString.h"
#endif
#ifndef mac_version
#ifndef NO_OPENMP
#include "omp.h"
#endif
#endif


Quan::Quan()
{
    //ctor
}

Quan::Quan(Json::ValueIterator &it)
{

    SetName(it.key().asString());
    if ((*it)["type"].asString()=="balance")
    {
        SetType(Quan::_type::balance);
        SetCorrespondingFlowVar((*it)["flow"].asString());
        SetCorrespondingInflowVar((*it)["inflow"].asString());
    }
    if ((*it)["type"].asString()=="constant")
        SetType(Quan::_type::constant);
    if ((*it)["type"].asString()=="expression")
    {
        SetType(Quan::_type::expression);
        SetExpression((*it)["expression"].asString());
    }
    if ((*it)["type"].asString()=="rule")
    {
        SetType(Quan::_type::rule);
        for (Json::ValueIterator itrule=(*it)["rule"].begin(); itrule!=(*it)["rule"].end(); ++itrule)
        {
            _condplusresult Rle;
            GetRule()->Append(itrule.key().asString(),itrule->asString());
        }
    }

    if ((*it)["type"].asString()=="global")
        SetType(Quan::_type::global_quan);
    if ((*it)["type"].asString()=="timeseries")
        SetType(Quan::_type::timeseries);
	if ((*it)["type"].asString() == "timeseries_prec")
		SetType(Quan::_type::prec_timeseries);
    if ((*it)["type"].asString()=="source")
        SetType(Quan::_type::source);
    if ((*it)["type"].asString()=="value")
        SetType(Quan::_type::value);
	if ((*it)["type"].asString() == "string")
		SetType(Quan::_type::string);
    if ((*it)["type"].asString() == "boolean")
        SetType(Quan::_type::boolean);
    if (it->isMember("includeinoutput"))
    {
        if ((*it)["includeinoutput"].asString()=="true")
            SetIncludeInOutput(true);
        else
            SetIncludeInOutput(false);
    }
    else
        SetIncludeInOutput(false);
    if (it->isMember("estimate"))
    {
        if ((*it)["estimate"].asString()=="true")
            SetEstimable(true);
        else
            SetEstimable(false);
    }
    else
        SetEstimable(false);

    if (it->isMember("applylimit"))
    {
        if ((*it)["applylimit"].asString() == "true")
            applylimit = true;
        else
            applylimit = false; 
    }
    else
        applylimit = false;

    if (it->isMember("precalcbasedon"))
    {
        vector<string> s = aquiutils::split((*it)["precalcbasedon"].asString(),':');
        if (s.size()==4)
        {
            precalcfunction.SetIndependentVariable(s[0]);
            if (s[1]=="log")
                precalcfunction.SetLogarithmic(true);
            else
                precalcfunction.SetLogarithmic(false);
            precalcfunction.setminmax(aquiutils::atof(s[2]),aquiutils::atof(s[3]));
        }
    }

    if (it->isMember("rigid"))
    {
        if ((*it)["rigid"].asString() == "true")
            rigid = true;
        else
            rigid = false;
    }
    else
        rigid = false;

    if (it->isMember("description"))
    {
        if (aquiutils::split((*it)["description"].asString(),';').size()==1)
        {   Description() = (*it)["description"].asString();
            Description(true) = (*it)["description"].asString();
        }
        else if (aquiutils::split((*it)["description"].asString(),';').size()==2)
        {
            Description() = aquiutils::split((*it)["description"].asString(),';')[0];
            Description(true) = aquiutils::split((*it)["description"].asString(),';')[1];
        }
    }
    if (it->isMember("helptext"))
    {
        HelpText() = (*it)["helptext"].asString();
    }

    if (it->isMember("unit"))
    {
        if ((*it)["unit"].asString()!="")
        {
            Units() = (*it)["unit"].asString();
            Unit() = aquiutils::split(Units(),';')[0];
        }
    }

    if (it->isMember("default_unit"))
        DefaultUnit() = (*it)["default_unit"].asString();

    if (it->isMember("default"))
        Default() = (*it)["default"].asString();

    if (it->isMember("delegate"))
        Delegate() = (*it)["delegate"].asString();

    if (it->isMember("category"))
        Category() = (*it)["category"].asString();

    if (it->isMember("input"))
        Input() = (*it)["input"].asString();

    if (it->isMember("experiment_dependent"))
    {   if ((*it)["experiment_dependent"].asString()=="Yes")
            ExperimentDependent() = true;
        else
            ExperimentDependent() = false;

    }

    if (it->isMember("description_code"))
        DescriptionCode() = (*it)["description_code"].asString();

    if (it->isMember("initial_value_expression"))
       SetInitialValueExpression((*it)["initial_value_expression"].asString());

    if (it->isMember("criteria"))
        Criteria() = (*it)["criteria"].asString();

    if (it->isMember("warningerror"))
        WarningError() = (*it)["warningerror"].asString();

    if (it->isMember("warningmessage"))
        WarningMessage() = (*it)["warningmessage"].asString();

    if (it->isMember("inputtype"))
        InputType() = (*it)["inputtype"].asString();

    if (it->isMember("ask_user"))
    {   AskFromUser() = false;
        if (aquiutils::tolower((*it)["ask_user"].asString())=="true")
            AskFromUser() = true;
    }
    else
       AskFromUser() = false;

    if (it->isMember("role"))
    {   SetRole(_role::none);
        if (aquiutils::tolower((*it)["role"].asString())=="copytoblocks")
            SetRole(_role::copytoblocks);
        else if (aquiutils::tolower((*it)["role"].asString())=="copytolinks")
            SetRole(_role::copytolinks);
        else if (aquiutils::tolower((*it)["role"].asString())=="copytosources")
            SetRole(_role::copytosources);
        else if (aquiutils::tolower((*it)["role"].asString())=="copytoreactions")
            SetRole(_role::copytoreactions);

    }
    else
       SetRole(_role::none);
    if (it->isMember("setvalue"))
    {
        SetProperty((*it)["setvalue"].asString());
    }
}

System* Quan::GetSystem() const {
    return (parent && parent->Parent()) ? parent->Parent() : nullptr;
}

#ifdef  Q_JSON_SUPPORT
Quan::Quan(QJsonObject& it)
{
	//SetName(it.key().asString());
	if (it.keys().contains("type"))
	{
		if (it.value("type").toString() == "balance")
		{
			SetType(Quan::_type::balance);
				SetCorrespondingFlowVar(it.value("flow").toString().toStdString());
				SetCorrespondingInflowVar(it.value("inflow").toString().toStdString());
		}
		if (it.value("type").toString() == "constant")
			SetType(Quan::_type::constant);
        if (it.value("type").toString() == "boolean")
            SetType(Quan::_type::boolean);
        if (it.value("type").toString() == "expression")
        {
            SetType(Quan::_type::expression);
            SetExpression(it.value("expression").toString().toStdString());
        }
        if (it.value("type").toString() == "rule")
		{
			SetType(Quan::_type::rule);
			for (QJsonObject::Iterator itrule = it.value("rule").toObject().begin(); itrule != it.value("rule").toObject().begin(); ++itrule)
			{
				_condplusresult Rle;
				GetRule()->Append(itrule.key().toStdString(), itrule.value().toString().toStdString());
			}
		}

		if (it.value("type").toString() == "global")
			SetType(Quan::_type::global_quan);
		if (it.value("type").toString() == "timeseries")
			SetType(Quan::_type::timeseries);
		if (it.value("type").toString() == "timeseries_prec")
			SetType(Quan::_type::prec_timeseries);
		if (it.value("type").toString() == "source")
			SetType(Quan::_type::source);
		if (it.value("type").toString() == "value")
			SetType(Quan::_type::value);
		if (it.value("type").toString() == "string")
			SetType(Quan::_type::string);
	}
	else
		SetType(Quan::_type::global_quan);
	
	if (it.keys().contains("includeinoutput"))
	{
		if (it.value("includeinoutput").toString().toStdString() == "true")
			SetIncludeInOutput(true);
		else
			SetIncludeInOutput(false);
	}
	else
		SetIncludeInOutput(false);
	if (it.keys().contains("description"))
	{
        if (aquiutils::split(it.value("description").toString().toStdString(),';').size()==1)
        {   Description() = it.value("description").toString().toStdString();
            Description(true) = it.value("description").toString().toStdString();
        }
        else if (aquiutils::split(it.value("description").toString().toStdString(),';').size()==2)
        {
            Description() = aquiutils::split(it.value("description").toString().toStdString(),';')[0];
            Description(true) = aquiutils::split(it.value("description").toString().toStdString(),';')[1];
        }
	}

	if (it.keys().contains("unit"))
        Units() = it.value("unit").toString().toStdString();

    if (it.keys().contains("precalcbasedon"))
    {
        vector<string> s = aquiutils::split(it.value("precalcbasedon").toString().toStdString(),':');
        if (s.size()==4)
        {
            precalcfunction.SetIndependentVariable(s[0]);
            if (s[1]=="log")
                precalcfunction.SetLogarithmic(true);
            else
                precalcfunction.SetLogarithmic(false);
            precalcfunction.setminmax(aquiutils::atof(s[2]),aquiutils::atof(s[3]));
        }
    }

    if (it.keys().contains("default_unit"))
		DefaultUnit() = it.value("default_unit").toString().toStdString();

	if (it.keys().contains("default"))
		Default() = it.value("default").toString().toStdString();

	if (it.keys().contains("delegate"))
		Delegate() = it.value("delegate").toString().toStdString();

	if (it.keys().contains("category"))
		Category() = it.value("category").toString().toStdString();

	if (it.keys().contains("input"))
		Input() = it.value("input").toString().toStdString();

    if (it.keys().contains("initial_value_expression"))
       SetInitialValueExpression(it.value("initial_value_expression").toString().toStdString());

	if (it.keys().contains("experiment_dependent"))
	{
		if (it.value("experiment_dependent").toString().toStdString() == "Yes")
			ExperimentDependent() = true;
		else
			ExperimentDependent() = false;

	}

	if (it.keys().contains("description_code"))
		DescriptionCode() = it.value("description_code").toString().toStdString();

	if (it.keys().contains("criteria"))
		Criteria() = it.value("criteria").toString().toStdString();

	if (it.keys().contains("warningerror"))
		WarningError() = it.value("warningerror").toString().toStdString();

	if (it.keys().contains("warningmessage"))
		WarningMessage() = it.value("warningmessage").toString().toStdString();

	if (it.keys().contains("inputtype"))
		InputType() = it.value("inputtype").toString().toStdString();

	if (it.keys().contains("ask_user"))
	{
        AskFromUser() = false;
        if (aquiutils::tolower(it.value("ask_user").toString().toStdString()) == "true")
			AskFromUser() = true;
	}
	else
		AskFromUser() = false;

    if (it.keys().contains("role"))
    {   SetRole(_role::none);
        if (aquiutils::tolower(it.value("role").toString().toStdString())=="copytoblocks")
            SetRole(_role::copytoblocks);
        else if (aquiutils::tolower(it.value("role").toString().toStdString())=="copytolinks")
            SetRole(_role::copytolinks);
        else if (aquiutils::tolower(it.value("role").toString().toStdString())=="copytoreactions")
            SetRole(_role::copytoreactions);
        else if (aquiutils::tolower(it.value("role").toString().toStdString())=="copytosources")
            SetRole(_role::copytosources);
    }
    else
       SetRole(_role::none);

    if (it.keys().contains("rigid"))
    {
        if (aquiutils::tolower(it.value("rigid").toString().toStdString()) == "true")
            rigid = true;
        else
            rigid = false; 
    }
    else
        rigid = false;

    if (it.keys().contains("applylimit"))
    {
        if (aquiutils::tolower(it.value("applylimit").toString().toStdString()) == "true")
            applylimit = true;
        else
            applylimit = false; 
    }
    else
        applylimit = false;

    if (it.keys().contains("setvalue"))
    {
        SetProperty(it.value("setvalue").toString().toStdString());
    }

}
#endif //  Q_JSON_SUPPORT

Quan::~Quan()
{
    _timeseries.clear(); 
    precalcfunction.clear(); 
}

Quan::Quan(const Quan& other)
{
    _expression = other._expression;
    _timeseries = other._timeseries;
	_string_value = other._string_value; 
    _rule = other._rule;
	sourcename = other.sourcename;
    _var_name = other._var_name;
    _val = other._val;
    _val_star = other._val_star;
    //_parameters = other._parameters;
    value_star_updated = other.value_star_updated;
    _parameterassignedto = other._parameterassignedto;
    type = other.type;
	corresponding_flow_quan = other.corresponding_flow_quan;
	corresponding_inflow_quan = other.corresponding_inflow_quan;
	includeinoutput = other.includeinoutput;
	description = other.description;
    description_graph = other.description_graph;
    unit = other.unit;
    units = other.units;
    default_unit = other.default_unit;
    default_val = other.default_val;
    input_type = other.input_type;
    defaults = other.defaults;
    delegate = other.delegate;
    category = other.category;
    input = other.input;
    experiment_dependent = other.experiment_dependent;
    description_code = other.description_code;
    abbreviation = other.abbreviation;
    criteria = other.criteria;
    warning_error = other.warning_error;
    warning_message = other.warning_message;
    ask_from_user = other.ask_from_user;
    estimable = other.estimable;
    applylimit = other.applylimit; 
    rigid = other.rigid;
    role = other.role;
    initial_value_expression = other.initial_value_expression;
    calculate_initial_value_from_expression = other.calculate_initial_value_from_expression;
    value_star_updated = other.value_star_updated;
    precalcfunction = other.precalcfunction;
    OutputItem = other.OutputItem;
    helptext = other.helptext;
	//parent = other.parent;
}

Quan& Quan::operator=(const Quan& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    estimable = rhs.estimable;
    _expression = rhs._expression;
    _timeseries = rhs._timeseries;
    _rule = rhs._rule;
    _var_name = rhs._var_name;
	_string_value = rhs._string_value;
    _val = rhs._val;
    _val_star = rhs._val_star;
    value_star_updated = rhs.value_star_updated;
    //_parameters = rhs._parameters;
    _parameterassignedto = rhs._parameterassignedto;
    sourcename = rhs.sourcename;
    type = rhs.type;
	corresponding_flow_quan = rhs.corresponding_flow_quan;
	corresponding_inflow_quan = rhs.corresponding_inflow_quan;
	includeinoutput = rhs.includeinoutput;
	description = rhs.description;
    description_graph = rhs.description_graph;
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
    applylimit = rhs.applylimit;
    rigid = rhs.rigid;
    role = rhs.role;
    initial_value_expression = rhs.initial_value_expression;
    calculate_initial_value_from_expression = rhs.calculate_initial_value_from_expression;
    value_star_updated = rhs.value_star_updated;
    precalcfunction = rhs.precalcfunction;
    OutputItem = rhs.OutputItem;
    helptext = rhs.helptext;
    //parent = rhs.parent;
    return *this;
}

bool Quan::operator==(const Quan& other) const
{
    bool out = true;
    out = out && (_val != other._val);
    out = out && (_val_star != other._val_star);
    return out; 
}

double Quan::CalcVal(Object* block, const Timing& tmg)
{
    switch (type)
    {
    case _type::constant:
    case _type::boolean:
    case _type::value:
        return _val;

    case _type::expression:
        return _expression.calc(block, tmg);

    case _type::rule:
        return _rule.calc(block, tmg);

    case _type::timeseries:
        return (_timeseries.size() > 0)
            ? _timeseries.interpol(block->GetParent()->GetTime())
            : 0;

    case _type::source:
        if (source != nullptr)
        {
            std::scoped_lock lock(_mutex_val_star);
            if (!value_star_updated)
            {
                _val_star = source->GetValue(block);
                value_star_updated = true;
            }
            return _val_star;
        }
        return 0;

    default:
        last_error = "Quantity cannot be evaluated";
        return 0;
    }
}

double Quan::GetVal(const Timing& tmg)
{
    if (tmg == Timing::past)
        return _val;

    {
        std::scoped_lock lock(_mutex_val_star);  // Protect shared state
        if (!value_star_updated)
        {
            switch (type)
            {
            case _type::expression:
                _val_star = CalcVal(tmg);
                break;
            case _type::rule:
                _val_star = CalcVal(tmg);
                break;

            case _type::timeseries:
                if (auto* ts = GetTimeSeries())
                    _val_star = ts->interpol(GetSimulationTime());
                else
                    _val_star = 0;
                break;
            case _type::prec_timeseries:
                if (auto* ts = GetTimeSeries())
                    _val_star = ts->interpol(GetSimulationTime());
                else
                    _val_star = 0;
                break;

            default:
                _val_star = CalcVal(tmg);
                break;
            }
            value_star_updated = true;
        }
    }

    return _val_star;
}

double& Quan::GetSimulationTime() const {
    return GetSystem()->GetSimulationTime();
}

TimeSeries<timeseriesprecision>* Quan::GetTimeSeries()
{
    if (_timeseries.size() != 0)
        return &_timeseries;
    else
        return nullptr;

}

double Quan::CalcVal(const Timing& tmg)
{
    switch (type)
    {
    case _type::constant:
    case _type::boolean:
    case _type::value:
    case _type::balance:
        return (tmg == Timing::past) ? _val : _val_star;

    case _type::expression:
        if (precalcfunction.IndependentVariable().empty() || !precalcfunction.Initiated())
            return _expression.calc(parent, tmg);
        else
            return InterpolateBasedonPrecalcFunction(parent->GetVal(precalcfunction.IndependentVariable(), tmg));

    case _type::rule:
        return _rule.calc(parent, tmg);

    case _type::timeseries:
    case _type::prec_timeseries:
        return (_timeseries.size() > 0) ? _timeseries.interpol(parent->GetParent()->GetTime()) : 0;

    case _type::source:
        if (source != nullptr)
            return source->GetValue(parent);
        else
            return 0;

    default:
        last_error = "Quantity cannot be evaluated";
        return 0;
    }
}
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


bool Quan::SetVal(const double& v, const Timing& tmg, bool check_criteria)
{
    const double previous_val = _val;

    // Update _val if needed
    if (tmg == Timing::past || tmg == Timing::both)
        _val = v;

    // Update _val_star if needed
    if (tmg == Timing::present || tmg == Timing::both)
    {
        {
            std::scoped_lock lock(_mutex_val_star);
            _val_star = v;
            value_star_updated = true;
        }
    }

    // Criteria check only when setting both
    if (tmg == Timing::both && HasCriteria() && parent != nullptr)
    {
        const bool valid = !check_criteria || Criteria().calc(parent, tmg);
        if (!valid)
        {
            AppendError(parent->GetName(), "Quan", "SetVal", warning_message, 8012);
            _val = previous_val;
            std::scoped_lock lock(_mutex_val_star);
            _val_star = previous_val;
            return false;
        }
    }

    return true;
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


string Quan::ToString(int _tabs) const
{
    string out = aquiutils::tabs(_tabs) + _var_name + ":\n";
    out += aquiutils::tabs(_tabs) + "{\n";
    if (type==_type::constant)
        out += aquiutils::tabs(_tabs+1) + "type: constant\n";
    if (type==_type::balance)
        out += aquiutils::tabs(_tabs+1) + "type: balance\n";
    if (type==_type::expression)
        out += aquiutils::tabs(_tabs+1) + "type: expression\n";
    if (type==_type::constant)
        out += aquiutils::tabs(_tabs+1) + "type: constant\n";
    if (type==_type::global_quan)
        out += aquiutils::tabs(_tabs+1) + "type: global_quantity\n";
    if (type==_type::timeseries)
        out += aquiutils::tabs(_tabs+1) + "type: time_series\n";
    if (type==_type::value)
        out += aquiutils::tabs(_tabs+1) + "type: value\n";

    if (type==_type::expression)
        out += aquiutils::tabs(_tabs+1) + "expression: " + _expression.ToString() + "\n";

    if (type==_type::rule)
        out += aquiutils::tabs(_tabs+1) + "rule: " + _rule.ToString(_tabs) + "\n";

	if (type == _type::source)
	{
        if (source!=nullptr)
			out += aquiutils::tabs(_tabs + 1) + "source: " + source->GetName() + "\n";
		else
			out += aquiutils::tabs(_tabs + 1) + "source: " + sourcename + "\n";

	}

    if (calculate_initial_value_from_expression)
    {
        out += aquiutils::tabs(_tabs+1) + "initial_value_expression: " + initial_value_expression.ToString() + "\n";
    }
    out += aquiutils::tabs(_tabs+1) + "val: ";
    out += aquiutils::numbertostring(_val);
    out += string("\n");
    out += aquiutils::tabs(_tabs+1) + "val*:";
    out += aquiutils::numbertostring(_val_star);
    out += string("\n");
    out += aquiutils::tabs(_tabs) + "}";
    return out;
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

Object* Quan::GetParent()
{
    return parent;
}


void Quan::Renew()
{
    std::scoped_lock lock(_mutex_val_star);
    _val_star = _val;

}

void Quan::Update()
{
    std::scoped_lock lock(_mutex_val_star);
    _val = _val_star;

}

bool Quan::SetTimeSeries(const std::string& filename, bool prec)
{
    if (filename.empty())
    {
        _timeseries.clear();
        return true;
    }

    std::string resolved_path = resolveTimeSeriesFile(filename);

    return prec
        ? loadPrecipitationTimeSeries(resolved_path)
        : loadRegularTimeSeries(resolved_path);
}

std::string Quan::resolveTimeSeriesFile(const std::string& filename) const
{
    if (System* sys = GetSystem())
    {
        const std::string try_path = sys->InputPath() + filename;
        if (aquiutils::FileExists(try_path))
            return try_path;
    }
    return filename;
}

bool Quan::loadRegularTimeSeries(const std::string& path)
{
    _timeseries.readfile(path);
    if (_timeseries.fileNotFound)
    {
        AppendError(GetName(), "Quan", "SetTimeSeries", path + " was not found!", 3001);
        return false;
    }
    return true;
}

bool Quan::loadPrecipitationTimeSeries(const std::string& path)
{
    if (!CPrecipitation::isFileValid(path))
    {
        AppendError(GetName(), "Quan", "SetTimeSeries", path + " is not a valid precipitation file", 3023);
        return false;
    }

    CPrecipitation Prec;
    Prec.getfromfile(path);
    _timeseries = Prec.getflow(1)[0];
    _timeseries.setFilename(Prec.filename);
    return true;
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


string Quan::GetProperty(bool force_value)
{
    //qDebug()<<QString::fromStdString(this->GetName());
    if (type == _type::balance || type== _type::constant || type==_type::global_quan || type==_type::value || (type==_type::expression && force_value))
    {
        return aquiutils::numbertostring(GetVal(Timing::present));

    }
    if (type == _type::timeseries)
    {
        if (aquiutils::GetPath(_timeseries.getFilename()) == aquiutils::GetPath(GetSystem()->GetWorkingFolder()))
            return aquiutils::GetOnlyFileName(_timeseries.getFilename());
        else
            return _timeseries.getFilename();
    }
    if (type == _type::prec_timeseries)
    {
        if (aquiutils::GetPath(_timeseries.getFilename()) == aquiutils::GetPath(GetSystem()->GetWorkingFolder()))
            return aquiutils::GetOnlyFileName(_timeseries.getFilename());
        else
            return _timeseries.getFilename();
    }
    else if (type == _type::source)
    {
        return sourcename;
    }
	else if (type == _type::string)
	{
        return _string_value;
	}
    else if (type == _type::expression)
    {
        return this->GetExpression()->ToString();
    }
    else if (type == _type::boolean)
    {
        if (aquiutils::numbertostring(GetVal(Timing::present))=="1")
            return "Yes";
        else
            return "No";
    }
    return "";

}

bool Quan::SetProperty(const std::string& val, bool force_value, bool check_criteria)
{
    // Normalize whitespace or input
    if (val.empty()) return false;

    switch (type)
    {
    case _type::balance:
    case _type::constant:
    case _type::global_quan:
    case _type::value:
        return SetVal(aquiutils::atof(val), Timing::both, check_criteria);

    case _type::expression:
        if (force_value)
            return SetVal(aquiutils::atof(val), Timing::both, check_criteria);
        else
            return SetExpression(val);

    case _type::rule:
        AppendError(GetName(), "Quan", "SetProperty", "Rule cannot be set during runtime", 3011);
        return false;

    case _type::timeseries:
        return resolveAndSetTimeSeries(val, /*prec=*/false);

    case _type::prec_timeseries:
        return resolveAndSetTimeSeries(val, /*prec=*/true);

    case _type::source:
        sourcename = val;
        return SetSource(val);

    case _type::string:
        return setStringProperty(val);

    case _type::boolean:
        return SetVal((val == "1" || aquiutils::tolower(val) == "yes") ? 1.0 : 0.0, Timing::both, check_criteria);

    default:
        return false;
    }
}

bool Quan::resolveAndSetTimeSeries(const std::string& filename, bool prec)
{
    if (filename.empty()) {
        _timeseries.clear();
        return false;
    }

    std::string fullpath = filename;

    if (System* sys = GetSystem()) {
        std::string try_path = sys->InputPath() + filename;
        if (aquiutils::FileExists(try_path))
            fullpath = try_path;
    }

    return SetTimeSeries(fullpath, prec);
}

bool Quan::setStringProperty(const std::string& val)
{
    if (GetName() == "name" && parent) {
        if (!parent->SetName(val, false))
            return false;
    }

    _string_value = val;
    return true;
}


bool Quan::AppendError(const std::string& objectname, const std::string& cls, const std::string& funct, const std::string& description, const int& code) const
{
    if (System* sys = GetSystem()) {
#pragma omp critical (quan_append_error)
        sys->errorhandler.Append(objectname, cls, funct, description, code);
        return true;
    }
    return false;
}

string Quan::toCommand()
{
    string s;
    if (delegate=="UnitBox")
#ifdef Q_GUI_SUPPORT
        if (unit!=default_unit)
        {
            const double coefficient = XString::coefficient(QString::fromStdString(unit));
            const double _value = atof(GetProperty(true).c_str())/coefficient;
            s += GetName() + "=" + aquiutils::numbertostring(_value) + "[" + unit + "]";
        }
        else
#endif
            s += GetName() + "=" + GetProperty(true);
    else if (delegate=="ValueBox")
        s += GetName() + "=" + GetProperty(true);
    else
        s += GetName() + "=" + GetProperty(false);
    return s;
}

bool Quan::Validate()
{
    if ((type == _type::timeseries || type == _type::prec_timeseries) && !_timeseries.getFilename().empty())
    {
        const std::string path = _timeseries.getFilename();
        const std::string input_path = GetSystem() ? GetSystem()->InputPath() : "";

        bool path_mismatch = !input_path.empty() &&
            aquiutils::GetPath(input_path) != aquiutils::GetPath(path);

        if (path_mismatch)
        {
            std::string resolved = resolveTimeSeriesFile(path);
            return SetTimeSeries(resolved, type == _type::prec_timeseries);
        }
        else
        {
            return SetTimeSeries(path, type == _type::prec_timeseries);
        }
    }

    return Criteria().calc(parent, Timing::both);
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
    const double old_independent_variable_value = parent->GetVal(precalcfunction.IndependentVariable(),Timing::present);
    if (parent==nullptr) return false;
    if (precalcfunction.IndependentVariable().empty()) return false;
    precalcfunction.clear();
    if (!precalcfunction.Logarithmic())
        for (double x=precalcfunction.xmin(); x<=precalcfunction.xmax(); x+=(precalcfunction.xmax()-precalcfunction.xmin())/double(n_inc))
            {
                parent->UnUpdateAllValues();
                parent->SetVal(precalcfunction.IndependentVariable(),x,Timing::present);
                precalcfunction.append(x,CalcVal(Timing::present));
            }
    else
        for (double x=log(precalcfunction.xmin()); x<=log(precalcfunction.xmax()); x+=(log(precalcfunction.xmax())-log(precalcfunction.xmin()))/double(n_inc))
            {
                parent->UnUpdateAllValues();
                parent->SetVal(precalcfunction.IndependentVariable(),exp(x),Timing::present);
                precalcfunction.append(x,CalcVal(Timing::present));
            }
    parent->SetVal(precalcfunction.IndependentVariable(),old_independent_variable_value,Timing::present);
    precalcfunction.setStructured(true);
    precalcfunction.SetInitiated(true);
    return true;
}

