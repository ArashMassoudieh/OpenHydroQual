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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "wizardparameter.h"
#include "Expression.h"
/*struct _calculation_pattern
{
    std::vector<int> operands; // a vector of size two containing the indicators to the terms being oprated
    int output_cell_id;
    std::string operatr; //operator
    double value; // value of the term
    std::string sign = "+";
};

struct _calculation_struct
{
    std::vector<_calculation_pattern> CalcOrder;
    std::vector<int> sources;
    std::vector<int> targets;
};*/

using namespace std;

class Wizard_Entity;

class Wizard_Argument
{
public:
    Wizard_Argument();
    Wizard_Argument(const string &exp, const string &unit="");
    Wizard_Argument(const Wizard_Argument& WA);
    Wizard_Argument& operator=(const Wizard_Argument& WA);
    double calc(QMap<QString, WizardParameter>* params);
    QString Calc(QMap<QString, WizardParameter>* params);
    QString Unit() { return QString::fromStdString(unit); }
    QString UnitText();
    parameter_type ArgumentType() { return argument_type; }
    QString WorkingDirectory();
    void SetWizardEntity(Wizard_Entity *wiz_entity) {wizard_entity = wiz_entity;}
private:
    vector<string> operators;
    vector<Wizard_Argument> terms;
    string text;
    static vector<string> funcs;
    static vector<string> opts;
    static bool func_operators_initialized;
    std::vector<std::string> _errors;
    string param_constant_expression;
    double constant = 0;
    string constant_string; 
    string function;
    string parameter; 
    string sign; 
    void Setup_Calculation_Structure();
    _calculation_struct CalculationStructure;
    void AppendTermToStructure(int i);
    int get_target_item_of_term(int term_id);
    int count_operators(const string& s) const;
    double func(const string& f, const double& val);
    double func(const string& f, const double& val1, const double& val2);
    double func(const string& f, const double& cond, const double& val1, const double& val2);
    double oprt(unsigned int calculation_sequence, string& f, QMap<QString, WizardParameter>* params);
    double oprt(const string& f, const double& val1, const double& val2) const;
    vector<double> argument_values(unsigned int calculation_sequence, QMap<QString, WizardParameter>* params);
    string unit; 
    parameter_type argument_type = parameter_type::numeric;
    Wizard_Entity *wizard_entity;
};

double QDate2Xldate(const QDateTime& x);
double QString2Xldate(const QString& x);
