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


#include "Objective_Function_Set.h"
#include <regex>
#include <sstream>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <cctype>
#include <map>
#include <functional>


Objective_Function_Set::Objective_Function_Set()
{
    //ctor
}

Objective_Function_Set::~Objective_Function_Set()
{
    //dtor
}

Objective_Function_Set::Objective_Function_Set(const Objective_Function_Set& other)
{
    objectivefunctions = other.objectivefunctions;
    expression_map = other.expression_map;
}

Objective_Function_Set& Objective_Function_Set::operator=(const Objective_Function_Set& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    objectivefunctions = rhs.objectivefunctions;
    expression_map = rhs.expression_map;
    return *this;
}

void Objective_Function_Set::Append(const string &name, const Objective_Function &o_func, double weight)
{
    objectivefunctions.push_back(o_func);
    objectivefunctions[objectivefunctions.size()-1].SetVal("weight",weight);
    objectivefunctions[objectivefunctions.size()-1].SetName(name);
    return;
}
Objective_Function* Objective_Function_Set::operator[](string name)
{
    for (unsigned int i=0; i<objectivefunctions.size(); i++)
        if (objectivefunctions[i].GetName() == name)
            return &objectivefunctions[i];

     lasterror = "Objective Function " + name + " does not exist!";
     return nullptr;
}

double Objective_Function_Set::Calculate()
{
    double out = 0;
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        out += objectivefunctions[i].GetObjective()*objectivefunctions[i].Weight();

    return out;
}

void Objective_Function_Set::ClearStoredTimeSeries()
{
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].GetTimeSeries()->clear();
}

CTimeSeriesSet<double> Objective_Function_Set::TimeSeries()
{
    CTimeSeriesSet<double> output(objectivefunctions.size());
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
    {
        output.BTC[i] = *objectivefunctions[i].GetTimeSeries();
        output.setname(i,objectivefunctions[i].GetName());
    }
    return output;
}

CVector Objective_Function_Set::Objective_Values()
{
    CVector values(objectivefunctions.size());
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        values[i] = objectivefunctions[i].GetObjectiveValue();
    return values;
}

void Objective_Function_Set::Update(double t)
{

    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].append_value(t);
    return;
}

CTimeSeriesSet<timeseriesprecision> Objective_Function_Set::GetTimeSeriesSet()
{
    CTimeSeriesSet<timeseriesprecision> out;
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        out.append(*objectivefunctions[i].GetTimeSeries(),objectivefunctions[i].GetName());
    return out;
}

void Objective_Function_Set::SetSystem(System* s)
{
    for (unsigned int i=0; i < objectivefunctions.size(); i++)
        objectivefunctions[i].SetSystem(s);
}

void Objective_Function_Set::clear()
{
    objectivefunctions.clear();
}

bool Objective_Function_Set::erase(int i)
{
    if (i >= objectivefunctions.size()) return false;
    objectivefunctions.erase(objectivefunctions.begin() + i);

    return true;
}

double Objective_Function_Set::EvaluateExpression(const std::string& expression)
{
    std::string substituted_expr = expression;

    // Regex to match valid objective function names (variable names)
    std::regex name_regex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
    std::smatch match;

    std::string result;
    std::string::const_iterator searchStart(expression.cbegin());

    while (std::regex_search(searchStart, expression.cend(), match, name_regex))
    {
        std::string varname = match[0];
        Objective_Function* func = (*this)[varname];
        if (!func)
            throw std::runtime_error("Objective function \"" + varname + "\" not found.");

        double value = func->GetObjective();
        result += match.prefix().str(); // Text before the match
        result += std::to_string(value); // Replace with numeric value
        searchStart = match.suffix().first; // Continue after the match
    }

    result += std::string(searchStart, expression.cend()); // Add remaining string

    // Now evaluate the resulting numeric expression
    return EvaluateMathExpression(result);
}


void Objective_Function_Set::AppendExpression(const std::string& name, const std::string& expression)
{
    expression_map[name] = expression;
}

std::map<std::string, double> Objective_Function_Set::EvaluateAllExpressions()
{
    std::map<std::string, double> results;
    for (const auto& [name, expr] : expression_map)
    {
        try
        {
            double value = EvaluateExpression(expr); // Uses your existing expression parser
            results[name] = value;
        }
        catch (const std::exception& e)
        {
            lasterror = "Error in expression \"" + name + "\": " + e.what();
            results[name] = std::numeric_limits<double>::quiet_NaN(); // Mark as invalid
        }
    }
    return results;
}


// Precedence rules
int precedence(char op)
{
    switch (op)
    {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}

bool isRightAssociative(char op)
{
    return op == '^';
}

double applyOp(double a, double b, char op)
{
    switch(op)
    {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::runtime_error("Division by zero.");
            return a / b;
        case '^': return std::pow(a, b);
    }
    return 0;
}

bool isFunctionChar(char c) {
    return std::isalpha(c) || c == '_';
}

double EvaluateFunction(const std::string& name, const std::vector<double>& args)
{
    static const std::map<std::string, std::function<double(double)>> unary_functions = {
        {"sin", std::function<double(double)>(static_cast<double(*)(double)>(std::sin))},
        {"cos", std::function<double(double)>(static_cast<double(*)(double)>(std::cos))},
        {"tan", std::function<double(double)>(static_cast<double(*)(double)>(std::tan))},
        {"sqrt", std::function<double(double)>(static_cast<double(*)(double)>(std::sqrt))},
        {"log", std::function<double(double)>(static_cast<double(*)(double)>(std::log))},
        {"log10", std::function<double(double)>(static_cast<double(*)(double)>(std::log10))},
        {"exp", std::function<double(double)>(static_cast<double(*)(double)>(std::exp))},
        {"abs", std::function<double(double)>(static_cast<double(*)(double)>(std::fabs))}
    };

    static const std::map<std::string, std::function<double(double, double)>> binary_functions = {
        {"min", [](double a, double b) { return std::min(a, b); }},
        {"max", [](double a, double b) { return std::max(a, b); }}
    };

    if (unary_functions.count(name) && args.size() == 1)
        return unary_functions.at(name)(args[0]);
    else if (binary_functions.count(name) && args.size() == 2)
        return binary_functions.at(name)(args[0], args[1]);

    throw std::runtime_error("Unknown function or wrong number of arguments: " + name);
}

double parseNumber(std::istringstream& input)
{
    double val;
    input >> val;
    return val;
}

std::string parseIdentifier(std::istringstream& input)
{
    std::string name;
    while (input && isFunctionChar(input.peek()))
        name += input.get();
    return name;
}

std::vector<double> parseFunctionArgs(std::istringstream& input)
{
    std::vector<double> args;
    int parentheses = 1;
    std::string arg_expr;

    while (input && parentheses > 0)
    {
        char ch = input.get();
        if (ch == '(')
        {
            parentheses++;
            arg_expr += ch;
        }
        else if (ch == ')')
        {
            parentheses--;
            if (parentheses > 0) arg_expr += ch;
        }
        else if (ch == ',' && parentheses == 1)
        {
            args.push_back(EvaluateMathExpression(arg_expr));
            arg_expr.clear();
        }
        else
        {
            arg_expr += ch;
        }
    }

    if (!arg_expr.empty())
        args.push_back(EvaluateMathExpression(arg_expr));

    return args;
}

double EvaluateMathExpression(const std::string& expr)
{
    std::istringstream input(expr);
    std::stack<double> values;
    std::stack<char> ops;

    while (input >> std::ws && input.peek())
    {
        char ch = input.peek();

        if (isdigit(ch) || ch == '.')
        {
            values.push(parseNumber(input));
        }
        else if (isFunctionChar(ch))
        {
            std::string name = parseIdentifier(input);
            input >> std::ws;
            if (input.peek() == '(')
            {
                input.get(); // consume '('
                std::vector<double> args = parseFunctionArgs(input);
                values.push(EvaluateFunction(name, args));
            }
            else
            {
                throw std::runtime_error("Function call must be followed by '('");
            }
        }
        else if (ch == '(')
        {
            ops.push(input.get());
        }
        else if (ch == ')')
        {
            input.get();
            while (!ops.empty() && ops.top() != '(')
            {
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(a, b, op));
            }
            if (!ops.empty()) ops.pop();
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
        {
            char op = input.get();
            while (!ops.empty() && ops.top() != '(' &&
                   (precedence(ops.top()) > precedence(op) ||
                   (precedence(ops.top()) == precedence(op) && !isRightAssociative(op))))
            {
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                char prev_op = ops.top(); ops.pop();
                values.push(applyOp(a, b, prev_op));
            }
            ops.push(op);
        }
        else
        {
            throw std::runtime_error(std::string("Invalid character in expression: ") + ch);
        }
    }

    while (!ops.empty())
    {
        double b = values.top(); values.pop();
        double a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(a, b, op));
    }

    if (values.empty())
        throw std::runtime_error("Invalid expression.");

    return values.top();
}


