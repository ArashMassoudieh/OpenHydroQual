#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"

using namespace std;

class Wizard_Argument
{
public:
    Wizard_Argument();
    Wizard_Argument(const string &exp);
    Wizard_Argument(const Wizard_Argument& WA);
    Wizard_Argument& operator=(const Wizard_Argument& WA);
    
private:
    vector<string> operators;
    vector<Wizard_Argument> terms;
    string text;
    static vector<string> funcs;
    static vector<string> opts;
    static bool func_operators_initialized;
    std::vector<std::string> _errors;
    string param_constant_expression;
    double constant;
    string function;

};
