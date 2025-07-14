// ExpressionNode.cpp
#include "ExpressionNode.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include "Object.h"
#include "System.h"
#include "Utilities.h"

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
    
    //qDebug() << toStringFromTree(); 
    //qDebug() << "Childs: " << children.size();
    double result = 0.0;

    switch (type) {
    case Type::Constant:
        result = constantValue;
        break;

    case Type::Variable: {
        if (!W) return 0.0;
        Object* context = nullptr;
        switch (location) {
        case loc::self: context = W; break;
        case loc::source:
            if (W->ObjectType() == object_type::link)
                context = W->GetConnectedBlock(location);
            else if (location == loc::average_of_links)
                return dynamic_cast<Block*>(W)->GetAvgOverLinks(variableName, timing);
            break;
        case loc::destination:
            if (W->ObjectType() == object_type::link)
                context = W->GetConnectedBlock(location);
            else if (location == loc::average_of_links)
                return dynamic_cast<Block*>(W)->GetAvgOverLinks(variableName, timing);
            break;
        case loc::average_of_links:
            if (W->ObjectType() == object_type::link)
                context = W->GetConnectedBlock(location);
            else if (location == loc::average_of_links)
                return dynamic_cast<Block*>(W)->GetAvgOverLinks(variableName, timing);
            break;
        }
        return context ? context->GetVal(variableName, timing, limit) : 0.0;
    }

    case Type::Operator: {
        
		//qDebug() << "Child 1: " << children[0]->toStringFromTree();
        //qDebug() << "Child 2: " << children[1]->toStringFromTree();
        //qDebug() << children.size();
        double lhs = children[0]->evaluate(W, timing, limit);
        double rhs = children[1]->evaluate(W, timing, limit);
        switch (op) {
        case Operator::Add: result = lhs + rhs; break;
        case Operator::Subtract: result = lhs - rhs; break;
        case Operator::Multiply: result = lhs * rhs; break;
        case Operator::Divide: result = lhs / (rhs + 1e-23); break;
        case Operator::Power: result = pow(std::abs(lhs), rhs); break;
        case Operator::Sequence: result = rhs; break; // for functions like min(a;b), etc.
        default: throw std::runtime_error("Unknown operator");
        }
        break;
    }

    case Type::Function: {
        if (children.empty()) throw std::runtime_error("Function with no arguments: " + functionName);
                
        if (functionName == "exp" || functionName == "_exp") result = std::exp(children[0]->evaluate(W, timing, limit));
        else if (functionName == "log" || functionName == "_log") {
            double val = children[0]->evaluate(W, timing, limit);
            result = val > 0 ? std::log(val) : -1e12;
        }
        else if (functionName == "abs" || functionName == "_abs") 
            result = std::fabs(children[0]->evaluate(W, timing, limit));
        else if (functionName == "sgn" || functionName == "_sgn") {
            double val = children[0]->evaluate(W, timing, limit);
            result = (val > 0) - (val < 0);
        }
        else if (functionName == "sqr" || functionName == "_sqr") {
            double val = children[0]->evaluate(W, timing, limit);
            result = sqrt(aquiutils::Pos(val));
        }
        else if (functionName == "sqt" || functionName == "_sqt") {
            double val = children[0]->evaluate(W, timing, limit);
            if (val > 0)
                result = sqrt(val * val / (fabs(val) + 1e-4));
            else
                result = -sqrt(val * val / (fabs(val) + 1e-4));
        }
        else if (functionName == "pos" || functionName == "_pos") {
            double val = children[0]->evaluate(W, timing, limit);
            result = (val > 0) ? val : 0.0;
        }
        else if (functionName == "hsd" || functionName == "_hsd") {
            double val = children[0]->evaluate(W, timing, limit);
            result = (val > 0) ? 1.0 : 0.0;
        }
        else if ((functionName == "min" || functionName == "_min") && children.size() == 2)
            result = std::min(children[0]->evaluate(W, timing, limit), children[1]->evaluate(W, timing, limit));
        else if ((functionName == "max" || functionName == "_max") && children.size() == 2)
            result = std::max(children[0]->evaluate(W, timing, limit), children[1]->evaluate(W, timing, limit));
        else if ((functionName == "lpw" || functionName == "_lpw") && children.size() == 2) {
            double base = children[0]->evaluate(W, timing, limit);
            double expn = children[1]->evaluate(W, timing, limit);
            result = (base >= 0) ? std::pow(base, expn) : -std::pow(-base, expn);
        }
        else if ((functionName == "mon" || functionName == "_mon") && children.size() == 2) {
            double a = children[0]->evaluate(W, timing, limit);
            double b = children[1]->evaluate(W, timing, limit);
            result = (a + b != 0) ? a / (a + b) : 0.0;
        }
        else if ((functionName == "mbs" || functionName == "_mbs") && children.size() == 2) {
            double a = std::abs(children[0]->evaluate(W, timing, limit));
            double b = children[1]->evaluate(W, timing, limit);
            result = (a + b != 0) ? a / (a + b) : 0.0;
        }
        else if ((functionName == "ekr" || functionName == "_ekr") && children.size() == 2 && W) {
            std::string var = children[0]->variableName;
            if (!W->HasQuantity(var)) return 0.0;
            auto* ts = W->Variable(var)->GetTimeSeries();
            if (!ts) return 0.0;
            double decay = children[1]->evaluate(W, timing, limit);
            result = ts->Exponential_Kernel(W->Parent()->GetTime(), decay);
        }
        else if ((functionName == "gkr" || functionName == "_gkr") && children.size() == 3 && W) {
            std::string var = children[0]->variableName;
            if (!W->HasQuantity(var)) return 0.0;
            auto* ts = W->Variable(var)->GetTimeSeries();
            if (!ts) return 0.0;
            double mu = children[1]->evaluate(W, timing, limit);
            double sigma = children[2]->evaluate(W, timing, limit);
            result = ts->Gaussian_Kernel(W->Parent()->GetTime(), mu, sigma);
        }
        else if ((functionName == "ups" || functionName == "_ups") && children.size() == 2 && W && W->ObjectType() == object_type::link) {
            if (!W->GetConnectedBlock(loc::source) || !W->GetConnectedBlock(loc::destination)) return 0.0;
            double flow = children[0]->evaluate(W, timing, limit);
            double val = (flow >= 0)
                ? children[1]->evaluate(W->GetConnectedBlock(loc::source), timing, limit)
                : children[1]->evaluate(W->GetConnectedBlock(loc::destination), timing, limit);
            result = flow * val;
        }
        else if ((functionName == "bkw" || functionName == "_bkw") && children.size() == 2 && W && W->ObjectType() == object_type::link) {
            if (!W->GetConnectedBlock(loc::source) || !W->GetConnectedBlock(loc::destination)) return 0.0;
            double srcVal = children[0]->evaluate(W->GetConnectedBlock(loc::source), timing, limit);
            double dstVal = children[0]->evaluate(W->GetConnectedBlock(loc::destination), timing, limit);
            double slope = srcVal - dstVal;
            result = slope >= 0
                ? children[1]->evaluate(W->GetConnectedBlock(loc::source), timing, limit)
                : children[1]->evaluate(W->GetConnectedBlock(loc::destination), timing, limit);
        }
        else {
            throw std::runtime_error("Unsupported function or incorrect arguments: " + functionName);
        }
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

std::string ExpressionNode::toStringFromTree() const {
    switch (type) {
    case Type::Constant:
        return std::to_string(constantValue);

    case Type::Variable:
        return variableName;

    case Type::Operator: {
        std::string opStr = ExpressionNode::toStringOperator(op);
        std::string lhs = children.size() > 0 ? children[0]->toStringFromTree() : "";
        std::string rhs = children.size() > 1 ? children[1]->toStringFromTree() : "";
        return lhs + opStr + rhs;
    }

    case Type::Function: {
        std::string argString;
        for (size_t i = 0; i < children.size(); ++i) {
            if (i > 0) argString += ";";
            argString += children[i]->toStringFromTree();
        }
        return "_" + functionName + "(" + argString + ")";
    }
    }

    return "?";
}


