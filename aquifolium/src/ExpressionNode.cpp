// ExpressionNode.cpp
#include "ExpressionNode.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include "Object.h"

using namespace std;

ExpressionNode::ExpressionNode(double value)
    : type(Type::Constant), constantValue(value) {
}

ExpressionNode::ExpressionNode(const std::string& varName)
    : type(Type::Variable), variableName(varName) {
}

ExpressionNode::ExpressionNode(Operator oper, Ptr left, Ptr right)
    : type(Type::Operator), op(oper) {
    children.push_back(left);
    children.push_back(right);
}

ExpressionNode::ExpressionNode(const std::string& func, std::vector<Ptr> args)
    : type(Type::Function), functionName(func), children(std::move(args)) {
}

double ExpressionNode::evaluate(Object* W, Timing timing, bool limit)
{
    double result = 0.0;

    switch (type) {
    case Type::Constant:
        result = constantValue;
        break;

    case Type::Variable:
        result = W ? W->GetVal(variableName, timing, limit) : 0.0;
        break;

    case Type::Operator: {
        double lhs = children[0]->evaluate(W, timing, limit);
        double rhs = children[1]->evaluate(W, timing, limit);

        switch (op) {
        case Operator::Add: result = lhs + rhs; break;
        case Operator::Subtract: result = lhs - rhs; break;
        case Operator::Multiply: result = lhs * rhs; break;
        case Operator::Divide: result = lhs / (rhs + 1e-23); break;
        case Operator::Power: result = pow(std::abs(lhs), rhs); break;
        default: throw runtime_error("Unknown operator");
        }
        break;
    }

    case Type::Function: {
        if (functionName == "exp" && children.size() == 1)
            result = exp(children[0]->evaluate(W, timing, limit));
        else if (functionName == "log" && children.size() == 1) {
            double val = children[0]->evaluate(W, timing, limit);
            result = val > 0 ? log(val) : -1e12;
        }
        else if (functionName == "abs" && children.size() == 1)
            result = fabs(children[0]->evaluate(W, timing, limit));
        else if (functionName == "min" && children.size() == 2)
            result = min(children[0]->evaluate(W, timing, limit), children[1]->evaluate(W, timing, limit));
        else if (functionName == "max" && children.size() == 2)
            result = max(children[0]->evaluate(W, timing, limit), children[1]->evaluate(W, timing, limit));
        else
            throw runtime_error("Unsupported function or wrong number of arguments: " + functionName);
        break;
    }
    }

    return (sign == "-") ? -result : result;
}



ExpressionNode::Operator ExpressionNode::parseOperator(const std::string& token) {
    if (token == "+") return Operator::Add;
    if (token == "-") return Operator::Subtract;
    if (token == "*") return Operator::Multiply;
    if (token == "/") return Operator::Divide;
    if (token == "^") return Operator::Power;
    if (token == ";") return Operator::Sequence;
    return Operator::Unknown;
}

std::string ExpressionNode::toStringOperator(Operator op) {
    switch (op) {
    case Operator::Add: return "+";
    case Operator::Subtract: return "-";
    case Operator::Multiply: return "*";
    case Operator::Divide: return "/";
    case Operator::Power: return "^";
    case Operator::Sequence: return ";";
    default: return "?";
    }
}
