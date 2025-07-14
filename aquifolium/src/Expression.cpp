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


 // Expression.cpp — Refactored Legacy-Compatible Implementation
#include "Expression.h"
#include <stdexcept>
#include <iostream>
#include "Utilities.h"
#include "Object.h"
#include "Block.h"
#include "System.h"

bool Expression::func_operators_initialized = false;

// --- Default constructor ---
Expression::Expression() : rootNode(nullptr), quan(nullptr) {}

// --- Constructor from string expression ---
Expression::Expression(const std::string& S)
    : text(S), rootNode(nullptr), quan(nullptr) {

    if (aquiutils::contains(S, ":"))
    {
        std::cout << ": encountered";
    }
    
    if (!aquiutils::parantheses_balance(S)) {
        _errors.push_back("Unbalanced parentheses in expression: " + S);
        param_constant_expression = "error";
        return;
    }

    // Identify function if prefixed with _xxx(...)
    if (aquiutils::lookup({ "_min", "_max", "_exp", "_log", "_abs", "_sgn", "_sqr", "_sqt", "_lpw", "_pos", "_hsd", "_ups", "_bkw", "_mon", "_mbs", "_ekr", "_gkr" }, aquiutils::left(S, 4)) != -1 &&
        aquiutils::corresponding_parenthesis(S, 4) == int(S.size()) - 1) {
        function = aquiutils::right(aquiutils::left(S, 4), 3);
        text = aquiutils::right(S, S.size() - 4);
    }

    // Strip outermost parentheses
    std::string inner = text;
    if (aquiutils::left(inner, 1) == "(" &&
        aquiutils::corresponding_parenthesis(inner, 0) == int(inner.size()) - 1) {
        inner = aquiutils::right(inner, inner.size() - 1);
        inner = aquiutils::left(inner, inner.size() - 1);
    }

    if (aquiutils::isnumber(inner)) {
        constant = aquiutils::atof(inner);
        param_constant_expression = "constant";
        rootNode = std::make_shared<ExpressionNode>(constant);  // <--- Add this
        return;
    }

    if (aquiutils::left(inner, 1) == "(" &&
        aquiutils::corresponding_parenthesis(inner, 0) == int(inner.size()) - 1) {
        std::string stripped = inner.substr(1, inner.size() - 2);
        if (aquiutils::isnumber(stripped)) {
            constant = aquiutils::atof(stripped);
            param_constant_expression = "constant";
            rootNode = std::make_shared<ExpressionNode>(constant);  // <--- Add this
            return;
        }
    }
    
    // Check for variable with location suffix (e.g., "conc.s")
    if (!aquiutils::contains_any(inner, "+-*/^();")) {
        std::vector<std::string> parts = aquiutils::split(inner, '.');
        if (parts.size() == 1) {
            parameter = parts[0];
            location = ExpressionNode::loc::self;
        }
        else if (parts.size() == 2) {
            parameter = parts[0];
            std::string tag = aquiutils::tolower(parts[1]);
            if (tag == "s")
                location = ExpressionNode::loc::source;
            else if (tag == "e")
                location = ExpressionNode::loc::destination;
            else if (tag == "v")
                location = ExpressionNode::loc::average_of_links;
            else _errors.push_back("Unknown location tag: ." + parts[1]);
        }
        param_constant_expression = "parameter";
        return;
    }

    // Otherwise: it's an expression
    try {
        if (!function.empty())
            rootNode = ExpressionParser::parse("_" + function + "(" + inner + ")");
        else
            rootNode = ExpressionParser::parse(inner);
        param_constant_expression = "expression";
    }
    catch (const std::exception& e) {
        _errors.push_back("Parse error: " + std::string(e.what()));
        param_constant_expression = "error";
    }
}

// --- Copy constructor ---
Expression::Expression(const Expression& other)
    : text(other.text),
    param_constant_expression(other.param_constant_expression),
    parameter(other.parameter),
    sign(other.sign),
    function(other.function),
    constant(other.constant),
    unit(other.unit),
    _errors(other._errors),
    location(other.location),
    quan(nullptr)
    {

    rootNode = other.rootNode ? other.rootNode->clone() : nullptr;
}

// --- Assignment from string ---
Expression& Expression::operator=(const std::string& S) {
    *this = Expression(S);
    return *this;
}

// --- Assignment operator ---
Expression& Expression::operator=(const Expression& other) {
    if (this != &other) {
        text = other.text;
        param_constant_expression = other.param_constant_expression;
        parameter = other.parameter;
        sign = other.sign;
        function = other.function;
        constant = other.constant;
        unit = other.unit;
        _errors = other._errors;
        location = other.location;
        quan = nullptr;
        rootNode = other.rootNode ? other.rootNode->clone() : nullptr;
    }
    return *this;
}

// --- Destructor ---
Expression::~Expression() {
    rootNode.reset();
    quan = nullptr;
    _errors.clear();
}

// --- Set Quan Pointers ---
bool Expression::SetQuanPointers(Object* W) {
    if (param_constant_expression == "parameter") {
        if (location == ExpressionNode::loc::self)
            quan = W->Variable(parameter);
        else if (location != ExpressionNode::loc::average_of_links && W->ObjectType() == object_type::link)
            quan = W->GetConnectedBlock(location)->Variable(parameter);
        else
            quan = nullptr;
        return true;
    }
    return false;
}

// --- Convert expression to string ---
std::string Expression::ToString() const {
    if (param_constant_expression == "constant") {
        return aquiutils::numbertostring(constant);
    }

    if (param_constant_expression == "parameter") {
        std::string out = parameter;
        switch (location) {
        case ExpressionNode::loc::source: out += ".s"; break;
        case ExpressionNode::loc::destination: out += ".e"; break;
        case ExpressionNode::loc::average_of_links: out += ".v"; break;
        default: break;
        }
        return out;
    }

    if (param_constant_expression == "expression") {
        if (function.empty()) {
            return text;  // no wrapping
        }
        else {
            return "_" + function + "(" + text + ")";
        }
    }

    return text; // fallback or unknown type
}

// --- Rename a quantity if used as a bare parameter ---
bool Expression::RenameQuantity(const std::string& oldname, const std::string& newname) {
    bool changed = false;

    if (param_constant_expression == "parameter") {
        if (parameter == oldname) {
            parameter = newname;
            return true;
        }
    }
    else if (param_constant_expression == "expression" && rootNode) {
        std::function<bool(ExpressionNode::Ptr&)> rename;
        rename = [&](ExpressionNode::Ptr& node) -> bool {
            if (!node) return false;
            bool local_changed = false;

            if (node->type == ExpressionNode::Type::Variable) {
                std::string base = node->variableName;
                std::string suffix;
                if (aquiutils::ends_with(base, ".s")) {
                    suffix = ".s";
                    base = base.substr(0, base.size() - 2);
                }
                else if (aquiutils::ends_with(base, ".e")) {
                    suffix = ".e";
                    base = base.substr(0, base.size() - 2);
                }
                else if (aquiutils::ends_with(base, ".v")) {
                    suffix = ".v";
                    base = base.substr(0, base.size() - 2);
                }
                if (base == oldname) {
                    node->variableName = newname + suffix;
                    local_changed = true;
                }
            }

            for (ExpressionNode::Ptr& child : node->children) {
                local_changed = rename(child) || local_changed;
            }

            return local_changed;
            };
        changed = rename(rootNode);
    }

    if (changed) {
        text = ToExpressionStringFromTree(rootNode);
    }
    return changed;
}

// --- Rebuild expression text from the parsed tree ---
std::string Expression::ToExpressionStringFromTree(const ExpressionNode::Ptr& node) const {
    if (!node) return "";

    auto opPrecedence = [](ExpressionNode::Operator op) -> int {
        switch (op) {
        case ExpressionNode::Operator::Add:
        case ExpressionNode::Operator::Subtract:
            return 1;
        case ExpressionNode::Operator::Multiply:
        case ExpressionNode::Operator::Divide:
            return 2;
        case ExpressionNode::Operator::Power:
            return 3;
        case ExpressionNode::Operator::Sequence:
            return 0;
        default:
            return -1;
        }
        };

    switch (node->type) {
    case ExpressionNode::Type::Constant:
        return aquiutils::numbertostring(node->constantValue);

    case ExpressionNode::Type::Variable: {
        std::string out = node->variableName;
        switch (node->location) {
        case ExpressionNode::loc::source: out += ".s"; break;
        case ExpressionNode::loc::destination: out += ".e"; break;
        case ExpressionNode::loc::average_of_links: out += ".v"; break;
        default: break;
        }
        return out;
    }

    case ExpressionNode::Type::Operator: {
        if (node->children.size() == 2) {
            auto lhs = node->children[0];
            auto rhs = node->children[1];

            std::string lhsStr = ToExpressionStringFromTree(lhs);
            std::string rhsStr = ToExpressionStringFromTree(rhs);

            int parentPrec = opPrecedence(node->op);
            int lhsPrec = (lhs->type == ExpressionNode::Type::Operator) ? opPrecedence(lhs->op) : 10;
            int rhsPrec = (rhs->type == ExpressionNode::Type::Operator) ? opPrecedence(rhs->op) : 10;

            if (lhsPrec < parentPrec)
                lhsStr = "(" + lhsStr + ")";
            if (rhsPrec < parentPrec ||
                (rhsPrec == parentPrec && (node->op != ExpressionNode::Operator::Power)) ||
                (node->op == ExpressionNode::Operator::Subtract && rhs->type == ExpressionNode::Type::Operator)) {
                rhsStr = "(" + rhsStr + ")";
            }


            return lhsStr + " " + ExpressionNode::toStringOperator(node->op) + " " + rhsStr;
        }
        break;
    }

    case ExpressionNode::Type::Function: {
        std::string args;
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i > 0) args += ";";
            args += ToExpressionStringFromTree(node->children[i]);
        }
        return "_" + node->functionName + "(" + args + ")";
    }
    }

    return "?";
}

// --- Rebuild expression text from the parsed tree ---
std::string Expression::ToExpressionStringFromTree() const {
    if (!rootNode) return "";
	std::string result = ToExpressionStringFromTree(rootNode);
    return result;
}


// --- Helper function to revise a variable name with location ---
void Expression::ReviseName(std::string& name, ExpressionNode::loc& loc_ref, const std::string& constituent_name, const std::string& quantity) {
    if (aquiutils::ends_with(name, ".s")) {
        loc_ref = ExpressionNode::loc::source;
        name = name.substr(0, name.size() - 2);
    }
    else if (aquiutils::ends_with(name, ".e")) {
        loc_ref = ExpressionNode::loc::destination;
        name = name.substr(0, name.size() - 2);
    }
    else if (aquiutils::ends_with(name, ".v")) {
        loc_ref = ExpressionNode::loc::average_of_links;
        name = name.substr(0, name.size() - 2);
    }
    else {
        loc_ref = ExpressionNode::loc::self;
    }

    if (name == quantity) {
        name = constituent_name + ":" + quantity;
    }
}




// --- Revise constituent (e.g., add prefix to parameter) ---
Expression Expression::ReviseConstituent(const std::string& constituent_name, const std::string& quantity) {
    Expression out = *this;

    //qDebug() << "ReviseConstituent: " << constituent_name.c_str() << ":" << quantity.c_str() << ":" << ToExpressionStringFromTree();

    if (param_constant_expression == "parameter") {
        ReviseName(out.parameter, out.location, constituent_name, quantity);
    }
    else if (param_constant_expression == "expression" && out.rootNode) {
        out.rootNode->ReviseConstituentInTree(constituent_name, quantity);
    }

    out.text = out.ToExpressionStringFromTree();
    //qDebug() << "ReviseConstituent: " << constituent_name.c_str() << ":" << quantity.c_str() << ":" << out.text;
    return out;
}


// --- Rename constituent (e.g., change prefix) ---
Expression Expression::RenameConstituent(const std::string& old_constituent_name,
    const std::string& new_constituent_name,
    const std::string& quantity) {
    Expression out = *this;

    auto rename = [&](std::string& name) {
        std::string suffix;
        if (aquiutils::ends_with(name, ".s")) suffix = ".s";
        else if (aquiutils::ends_with(name, ".e")) suffix = ".e";
        else if (aquiutils::ends_with(name, ".v")) suffix = ".v";

        std::string base = suffix.empty() ? name : name.substr(0, name.size() - 2);
        const std::string old_full = old_constituent_name + ":" + quantity;
        const std::string new_full = new_constituent_name + ":" + quantity;

        if (base == old_full) {
            name = new_full + suffix;
        }
        };

    if (param_constant_expression == "parameter") {
        rename(out.parameter);
    }
    else if (param_constant_expression == "expression" && rootNode) {
        std::function<void(ExpressionNode::Ptr&)> apply;
        apply = [&](ExpressionNode::Ptr& node) {
            if (!node) return;
            if (node->type == ExpressionNode::Type::Variable) {
                rename(node->variableName);
            }
            for (ExpressionNode::Ptr& child : node->children) {
                apply(child);
            }
            };
        apply(out.rootNode);
    }

    out.text = out.ToExpressionStringFromTree();

    
    return out;
}



// --- Collect all required properties from destination-linked expressions ---
std::vector<std::string> Expression::GetAllRequieredEndingBlockProperties() {
    std::vector<std::string> props;

    if (param_constant_expression == "parameter" && location == ExpressionNode::loc::destination) {
        props.push_back(parameter);
    }
    else if (param_constant_expression == "expression" && rootNode) {
        std::function<void(const ExpressionNode::Ptr&)> collect;
        collect = [&](const ExpressionNode::Ptr& node) {
            if (!node) return;
            if (node->type == ExpressionNode::Type::Variable) {
                const std::string& name = node->variableName;
                if (aquiutils::ends_with(name, ".e")) {
                    props.push_back(name.substr(0, name.size() - 2));
                }
            }
            for (const auto& child : node->children) {
                collect(child);
            }
            };
        collect(rootNode);
    }

    return props;
}

// --- Collect all required properties from source-linked expressions ---
std::vector<std::string> Expression::GetAllRequieredStartingBlockProperties() {
    std::vector<std::string> props;

    if (param_constant_expression == "parameter" && location == ExpressionNode::loc::source) {
        props.push_back(parameter);
    }
    else if (param_constant_expression == "expression" && rootNode) {
        std::function<void(const ExpressionNode::Ptr&)> collect;
        collect = [&](const ExpressionNode::Ptr& node) {
            if (!node) return;
            if (node->type == ExpressionNode::Type::Variable) {
                const std::string& name = node->variableName;
                if (aquiutils::ends_with(name, ".s")) {
                    props.push_back(name.substr(0, name.size() - 2));
                }
            }
            for (const auto& child : node->children) {
                collect(child);
            }
            };
        collect(rootNode);
    }

    return props;
}


// --- Evaluate the expression ---
double Expression::calc(Object* W, const Timing& tmg, bool limit) {
    if (!W) return 0.0;

    if (param_constant_expression == "constant") return constant;

    if (param_constant_expression == "parameter") {
        if (!parameter.empty()) {
            Object* context = nullptr;
            switch (location) {
            case ExpressionNode::loc::self: context = W; break;
            case ExpressionNode::loc::source:
                if (W->ObjectType() == object_type::link)
                    context = W->GetConnectedBlock(location);
                else if (location == ExpressionNode::loc::average_of_links)
                    return dynamic_cast<Block*>(W)->GetAvgOverLinks(parameter, tmg);
                break;
            case ExpressionNode::loc::destination:
                if (W->ObjectType() == object_type::link)
                    context = W->GetConnectedBlock(location);
                else if (location == ExpressionNode::loc::average_of_links)
                    return dynamic_cast<Block*>(W)->GetAvgOverLinks(parameter, tmg);
                break;
            case ExpressionNode::loc::average_of_links:
                if (W->ObjectType() == object_type::link)
                    context = W->GetConnectedBlock(location);
                else if (location == ExpressionNode::loc::average_of_links)
                    return dynamic_cast<Block*>(W)->GetAvgOverLinks(parameter, tmg);
                break;
            }
            return context ? context->GetVal(parameter, tmg, limit) : 0.0;
        }
    }

    if (param_constant_expression == "expression" && rootNode)
    {
        //"Col1-1" : Expression evaluated : "porosity / rho_f" = 3.25e+22
        if (W->GetName() == "Col1-1" && ToExpressionStringFromTree() == "porosity / rho_f")
        {
            cout << "";
        }

        double out = rootNode->evaluate(W, tmg, limit);
		//qDebug() << W->GetName() << ":" << "Expression evaluated: " << ToExpressionStringFromTree() << " = " << out;
        return out;

    }

    return 0.0;
}