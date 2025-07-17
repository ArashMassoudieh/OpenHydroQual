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

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ExpressionNode.h"
#include "ExpressionParser.h"

class Object; // Forward declaration to avoid circular dependency
class Quan; // Forward declaration for Quan class

 /**
  * @class Expression
  * @brief Backward-compatible expression parser and evaluator for OpenHydroQual.
  *
  * Supports constant values, variables from various locations (self, source, destination, average),
  * and parsed mathematical expressions with functions and operators.
  */
class Expression {
public:

    // --- Constructors and lifecycle ---
    Expression();
    explicit Expression(const std::string& S);
    Expression(const Expression& other);
    Expression& operator=(const Expression& other);
	Expression& operator=(const std::string& S); ///< Assign from string expression
    ~Expression();

    // --- Core evaluation ---
    double calc(Object* W, const Timing &tmg, bool limit = false);
    bool SetQuanPointers(Object* W);
    std::string ToString() const;

    // --- Metadata (exposed for legacy compatibility) ---
    std::string param_constant_expression = ""; ///< "constant", "parameter", or "expression"
    std::string parameter = "";                 ///< Parameter name, e.g., "somequantity"
    std::string sign = "+";                    ///< Optional unary sign
    std::string function = "";                 ///< Optional wrapping function (e.g., "exp")
    double constant = 0.0;                      ///< Value if it's a constant expression
    std::string unit = "";                      ///< Optional unit label
    std::string text = "";                      ///< Original expression string
    Quan* quan = nullptr;                       ///< Pointer to resolved Quan object (if applicable)
    ExpressionNode::loc location = ExpressionNode::loc::self;                   ///< Source of the parameter value

    // --- Error and diagnostics ---
    std::vector<std::string> _errors; ///< Any parsing or evaluation errors

    // --- Utility accessors ---
    std::vector<std::string> GetAllRequieredStartingBlockProperties();
    std::vector<std::string> GetAllRequieredEndingBlockProperties();
    std::string ToExpressionStringFromTree(const ExpressionNode::Ptr& node) const;
    std::string ToExpressionStringFromTree() const;
    Expression ReviseConstituent(const std::string& constituent_name, const std::string& quantity);
    void ReviseName(std::string& name, ExpressionNode::loc& loc_ref, const std::string& constituent_name, const std::string& quantity);
    Expression RenameConstituent(const std::string& old_constituent_name, const std::string& new_constituent_name, const std::string& quantity);
    bool RenameQuantity(const std::string& oldname, const std::string& newname);

    // --- Static helpers ---
    static std::vector<std::string> extract_operators(const std::string& s);
    static std::vector<std::string> extract_terms(const std::string& s);
    static double func(const std::string& f, const double& val);
    static double func(const std::string& f, const double& val1, const double& val2);
    static double func(const std::string& f, const double& cond, const double& val1, const double& val2);
    static bool func_operators_initialized;

private:
    std::shared_ptr<ExpressionNode> rootNode; ///< Internal AST root for expression evaluation

    
};
