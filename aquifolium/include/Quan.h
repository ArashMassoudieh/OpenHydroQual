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
#include "safevector.h"

#ifdef Q_JSON_SUPPORT
#include <qjsonobject.h>
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
        CTimeSeries<timeseriesprecision>* GetTimeSeries();
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
        CTimeSeries<timeseriesprecision>* TimeSeries();
        bool IncludeInOutput() {return includeinoutput;}
		bool SetTimeSeries(const string &filename, bool prec=false);
        bool SetTimeSeries(const CTimeSeries<double> &timeseries);
        bool SetTimeSeries(const CPrecipitation &timeseries);
        string &Description(bool graph=false)
        {
            _expression.SetQuanPointers(W);
            return true;
        }

        return false;
    }

protected:

    private:
        Expression _expression;
        Rule _rule;
        CTimeSeries<timeseriesprecision> _timeseries;
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

std::string tostring(const Quan::_type& typ);

#endif // QUAN_H
