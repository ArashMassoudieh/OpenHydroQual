// ExpressionNode.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "Utilities.h"

class Object; // Forward declaration of Object class
class Expression; // Forward declaration of timing enum

/**
 * @class ExpressionNode
 * @brief Represents a node in an abstract syntax tree (AST) for evaluating mathematical and logical expressions.
 *
 * This class is designed to support constants, variables, binary operations, and functions. It allows recursive
 * evaluation of expressions given a simulation context (i.e., an Object pointer).
 */
class ExpressionNode {
public:
    /**
     * @enum Type
     * @brief Describes the kind of node.
     */
    enum class Type {
        Constant,   ///< Represents a numeric constant
        Variable,   ///< Represents a variable name to be resolved from an Object
        Operator,   ///< Represents a binary operator node with two children
        Function    ///< Represents a function node with one or more children
    };

    /**
     * @enum Operator
     * @brief Enumeration of supported binary operators.
     */
    enum class Operator {
        Add,        ///< Addition (+)
        Subtract,   ///< Subtraction (-)
        Multiply,   ///< Multiplication (*)
        Divide,     ///< Division (/)
        Power,      ///< Exponentiation (^)
        Sequence,   ///< Sequence (;) used in functions with multiple arguments
        Unknown     ///< Unknown or unsupported operator
    };

    using Ptr = std::shared_ptr<ExpressionNode>; ///< Shared pointer alias for ExpressionNode

    /**
     * @brief Constructs a constant expression node.
     * @param value The constant value.
     */
    ExpressionNode(double value);

    /**
     * @brief Constructs a variable expression node.
     * @param varName The variable name.
     */
    ExpressionNode(const std::string& varName);

    /**
     * @brief Constructs an operator node with left and right children.
     * @param op The operator.
     * @param left Pointer to the left-hand child node.
     * @param right Pointer to the right-hand child node.
     */
    ExpressionNode(Operator op, Ptr left, Ptr right);

    /**
     * @brief Constructs a function node with a name and list of argument nodes.
     * @param func Function name.
     * @param args Argument nodes.
     */
    ExpressionNode(const std::string& func, std::vector<Ptr> args);

    /**
     * @brief Evaluates the expression recursively within a given context.
     * @param W Pointer to the Object providing variable values.
     * @param timing String representation of time context (e.g., "present").
     * @param limit Optional flag to limit evaluation in certain contexts.
     * @return Resulting value of the evaluated expression.
     */
    double evaluate(Object* W, Timing timing, bool limit = false);


    /**
     * @brief Converts a string representation of an operator to the corresponding enum.
     * @param token The string operator.
     * @return Corresponding Operator enum.
     */
    static Operator parseOperator(const std::string& token);

    /**
     * @brief Converts an Operator enum to its string representation.
     * @param op The operator.
     * @return String equivalent of the operator.
     */
    static std::string toStringOperator(Operator op);

    // --- Data members ---
    Type type;                          ///< Node type
    Operator op = Operator::Unknown;   ///< Operator (if applicable)
    std::string variableName;          ///< Variable name (for variable nodes)
    std::string functionName;          ///< Function name (for function nodes)
    double constantValue = 0.0;        ///< Constant value (for constant nodes)
    std::vector<Ptr> children;         ///< Child nodes (arguments or operands)
    std::string sign = "+";            ///< Optional unary sign ('+' or '-')
};