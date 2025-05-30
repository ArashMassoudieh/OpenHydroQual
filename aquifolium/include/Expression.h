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


#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <string>
#include <vector>
#include <algorithm>
#include "Utilities.h"
#include "safevector.h"
using namespace std;

class Object;
class Quan;

struct _calculation_pattern
{
    vector<int> operands; // a vector of size two containing the indicators to the terms being oprated
    int output_cell_id;
    string operatr; //operator
    double value; // value of the term
    string sign = "+";
};

struct _calculation_struct
{
    vector<_calculation_pattern> CalcOrder;
    vector<int> sources;
    vector<int> targets;
};

class Expression
{
    public:
        virtual ~Expression();
        Expression(void);
        Expression(string S);
        Expression(const Expression &S);
        Expression& operator=(const Expression&);
        vector<string> operators;
        vector<Expression> terms;
        Quan* quan = nullptr;
        string sign;
        string function;
        string parameter;
        double constant=0;
        string param_constant_expression;
        static vector<string> extract_operators(string s);
        vector<string> _errors;
        static vector<string> extract_terms(string s);
        enum class timing {past, present, both};
        double calc(Object * W, const timing &tmg, bool limit = false);
        static double func(const string & f, const double &val);
        static double func(const string & f, const double &val1, const double &val2);
        static double func(const string &f, const double &cond, const double &val1, const double &val2);
        double oprt(const string & f, const double &val1, const double &val2) const;
        double oprt(const string & f, unsigned int i1, unsigned int i2, Object * W, const timing &tmg, bool limit=false);
        double oprt(unsigned int calculation_sequence, string &f, Object *W, const Expression::timing &tmg, bool limit=false);
        vector<double> argument_values(unsigned int calculation_sequence, Object *W, const Expression::timing &tmg, bool limit=false);
        string unit;
        string text;
        static vector<string> funcs;
        static vector<string> opts;
        int lookup_operators(const string &s) const;
        int count_operators(const string &s) const;
        enum loc {self, source, destination, average_of_links};
        string ToString() const;
		vector<string> GetAllRequieredStartingBlockProperties();
		vector<string> GetAllRequieredEndingBlockProperties();
        Expression ReviseConstituent(const string &constituent_name, const string &quantity);
        bool RenameQuantity(const string &oldname, const string &newname);
        Expression RenameConstituent(const string &old_constituent_name, const string &new_constituent_name, const string &quantity);
        void ResetTermsSources();
        SafeVector<int> Order_Of_Calculation();
        void EstablishSourceStructure();
        void ClearTermSources()
        {
            term_sources_determined = false;
            term_sources.clear();
            term_vals.clear();
            terms_source_counter.clear();
        }
        bool SetQuanPointers(Object *W);
        static bool func_operators_initialized;
        void Setup_Calculation_Structure();

    protected:

    private:
        _calculation_struct CalculationStructure;
        SafeVector<double> term_vals;
        SafeVector<double> temporarily_stored_values;
        vector<bool> terms_calculated;
        bool order_of_calculation_initialized = false;
        SafeVector<SafeVector<int> > term_sources;
        bool term_sources_determined = false;
        bool sourceterms_resized = false;
        SafeVector<unsigned int> terms_source_counter;
        loc location = loc::self; //0: self, 1: start, 2: end
        int find_order_of_source_container(int j);
        int get_target_item_of_term(int term_id);
        void AppendTermToStructure(int i);

};


#endif // EXPRESSION_H
