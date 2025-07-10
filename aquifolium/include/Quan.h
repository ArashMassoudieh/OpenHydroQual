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
#include <json/json.h>
#include "Condition.h"
#include <string>

#ifdef Q_version
#include <qjsonobject.h>
#endif

#define timeseriesprecision double
class Block;
class Link;
class System;
class Object;
class Source;
class QuanSet;
class CPrecipitation;


class Quan
{
    public:
        Quan();
        virtual ~Quan();
        Quan(const Quan& other);
        Quan(Json::ValueIterator &it);

#ifdef Q_version
		Quan(QJsonObject& qjobject);
#endif // QT_version

        string GetStringValue() {return _string_value;}
        Quan& operator=(const Quan& other);
        bool operator==(const Quan& other);
        enum class _type {constant, value, balance, expression, timeseries, prec_timeseries, global_quan, rule, source, string, not_assigned, boolean};
        enum class _role {none, copytoblocks, copytolinks, copytosources, copytoreactions};
        double CalcVal(Object *, const Expression::timing &tmg=Expression::timing::past);
        double CalcVal(const Expression::timing &tmg=Expression::timing::past);
        double GetVal(const Expression::timing &tmg=Expression::timing::past);
        bool EstablishExpressionStructure();
        double &GetSimulationTime() const;
        TimeSeries<timeseriesprecision>* GetTimeSeries();
        string last_error;
        void SetType(const _type &t) {type = t;}
        _type GetType() {return type;}
        Expression* GetExpression();
        Rule* GetRule();
        Source* GetSource();
        bool SetExpression(const string &E);
        bool SetExpression(const Expression &E);
        bool SetRule(const string &R);
        bool SetVal(const double &v, const Expression::timing &tmg=Expression::timing::past, bool check_criteria=false);
        bool SetSource(const string &sourcename);
        void SetCorrespondingFlowVar(const string &s);
        
        string GetCorrespondingFlowVar()
        {
            return corresponding_flow_quan;
        }
        void SetCorrespondingInflowVar(const string &s);
        vector<string>& GetCorrespondingInflowVar() {return corresponding_inflow_quan;}
        void SetMassBalance(bool on);
        void SetParent(Object *);
        Object* GetParent();
        Quan *Corresponding_flow_variable = nullptr;
		void Renew();
		void Update();
        void SetIncludeInOutput(bool x) {includeinoutput = x;}
        void SetEstimable(bool x) {estimable=x;}
        string GetName() {return _var_name;}
        bool IncludeInOutput() {return includeinoutput;}
		bool SetTimeSeries(const string &filename, bool prec=false);
        bool SetTimeSeries(const TimeSeries<double> &timeseries);
        bool SetTimeSeries(const CPrecipitation &timeseries);
        string &Description(bool graph=false)
        {
            if (!graph)
                return description;
            else
                return description_graph;
        }
        string &HelpText() {return helptext;}
        string &Unit() {return unit;}
        string &Units() {return units;}
        string &DefaultUnit() {return default_unit;}
        string &Defaults() {return defaults;}
        string &Delegate() {return delegate;}
        bool Estimable() {return estimable;}
        string &Category() {return category;}
        string &Input() {return input;}
        string &Default() {return default_val;}
        bool &ExperimentDependent() {return experiment_dependent;}
        string &DescriptionCode() {return description_code;}
        string &Abbreviation() {return abbreviation;}
        string &WarningError() {return warning_error;}
        string &WarningMessage() {return warning_message;}
        Condition &Criteria () {return criteria;}
        string &InputType() {return input_type;}
        string ToString(int _tabs=1) const;
        bool &AskFromUser() {return ask_from_user;}
        bool WhenCopied() {
            if (role == _role::copytoblocks || role==_role::copytolinks || role==_role::copytosources || role==_role::copytoreactions)
                return true;
            else {
                return false;
            }
        }
        void SetRole(const _role &r)
        {
            role = r;
        }
        _role GetRole() const
        {
            return role;
        }
		void SetName(const string &name) {_var_name=name;}
		bool AppendError(const string &objectname, const string &cls, const string &funct, const string &description, const int &code) const;
        bool SetProperty(const string &val, bool force_value = false, bool check_criteria=true);
        string GetProperty(bool force_value = false);
		string SourceName() { return sourcename;}
        bool SetSourceName(const string& s) { sourcename = s; return true;}
        string toCommand();
        void SetOutputItem(const string& s)
        {
            OutputItem = s;
        }
        string GetOutputItem() { return OutputItem; }
        void SetParameterAssignedTo(const string &s) {_parameterassignedto=s;}
        string GetParameterAssignedTo() {return _parameterassignedto;}
        bool Validate();
        bool HasCriteria() { if (criteria.Count() > 0) return true; else return false; }
        vector<string> GetAllRequieredStartingBlockProperties();
        vector<string> GetAllRequieredEndingBlockProperties();
        void Set_Value_Update(bool x) { value_star_updated = x; }
        bool Value_Updated() { return value_star_updated;}
        bool ApplyLimit() { return applylimit; }
        bool isrigid() { return rigid; }
        void SetInitialValueExpression(const string &expression);
        void SetInitialValueExpression(const Expression &expression);
        Expression &InitialValueExpression() {return initial_value_expression;}
        bool calcinivalue() const { return calculate_initial_value_from_expression; }
        vector<std::string> AllConstituents() const;
        vector<string> AllReactionParameters() const;
        bool RenameQuantity(const string &oldname, const string &newname);
        bool SetPrecalcIndependentVariable(const string &varname) {return precalcfunction.SetIndependentVariable(varname);}
        PreCalculatedFunction* PreCalcFunction() {return &precalcfunction;}
        double InterpolateBasedonPrecalcFunction(const double &val) const;
        bool InitializePreCalcFunction(int n_inc=100);
        bool SetQuanPointers(Object *W)
        {
            if (type==_type::expression)
            {   _expression.SetQuanPointers(W);
                return true;
            }

            return false;
        }

    protected:

    private:
        Expression _expression;
        Rule _rule;
        TimeSeries<timeseriesprecision> _timeseries;
        Source *source = nullptr;
		string sourcename = ""; 
		string _var_name;
		string _string_value = ""; 
        double _val=0;
        double _val_star=0;
        bool value_star_updated = false; 
        _type type = _type::not_assigned;
        bool perform_mass_balance = false;
        string corresponding_flow_quan;
        SafeVector<string> corresponding_inflow_quan;
        Object *parent = nullptr;
        bool includeinoutput = false;
        bool estimable = false;
        string description;
        string helptext;
        string description_graph;
        string unit;
        string units;
        string default_unit;
        string default_val="";
        string input_type;
        string defaults;
        string delegate;
        string category;
        string input;
        bool experiment_dependent = false;
        string description_code;
        string abbreviation;
        Condition criteria;
        string warning_error;
        string warning_message;
        bool ask_from_user=false;
        _role role = _role::none;
        string OutputItem;
        string _parameterassignedto = "";
        bool applylimit = false; 
        bool rigid = false; 
        bool calculate_initial_value_from_expression = false;
        Expression initial_value_expression;
        PreCalculatedFunction precalcfunction;
};

string tostring(const Quan::_type &typ);


#endif // QUAN_H
