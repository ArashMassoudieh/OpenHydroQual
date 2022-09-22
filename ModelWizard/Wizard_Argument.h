#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"
#include "wizardparameter.h"

using namespace std;

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

class Wizard_Argument
{
public:
    Wizard_Argument();
    Wizard_Argument(const string &exp, const string &unit="");
    Wizard_Argument(const Wizard_Argument& WA);
    Wizard_Argument& operator=(const Wizard_Argument& WA);
    double calc(QMap<QString, WizardParameter>* params);
    QString Unit() { return QString::fromStdString(unit); }
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
};
