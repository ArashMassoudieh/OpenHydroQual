// ExpressionParser.cpp
#include "ExpressionParser.h"
#include <cctype>
#include <sstream>
#include <cmath>

using namespace std;

ExpressionNode::Ptr ExpressionParser::parse(const std::string& expression) {
    std::string trimmed = aquiutils::trim(expression);
    if (ExpressionParser::isNumber(trimmed)) {
        return std::make_shared<ExpressionNode>(std::stod(trimmed));
    }

    std::vector<std::string> tokens = tokenize(trimmed);
    if (tokens.empty()) throw std::runtime_error("Empty expression.");

    return parseTokens(tokens, 0, static_cast<int>(tokens.size()) - 1);
}

vector<string> ExpressionParser::tokenize(const std::string& s) {
    vector<string> tokens;
    string current;
    int paren_level = 0;

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];

        if (isspace(c)) continue;

        if (c == '(' || c == ')') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(string(1, c));
        }
        else if (string("+-*/^;").find(c) != string::npos) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(string(1, c));
        }
        else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

ExpressionNode::Ptr ExpressionParser::parseTokens(const std::vector<std::string>& tokens, int start, int end) {
    if (start > end) throw std::runtime_error("Invalid token range");

    // Remove outer parentheses
    if (tokens[start] == "(" && tokens[end] == ")") {
        int level = 0;
        for (int i = start; i <= end; ++i) {
            if (tokens[i] == "(") level++;
            if (tokens[i] == ")") level--;
            if (level == 0 && i < end) break;
            if (i == end && level == 0)
                return parseTokens(tokens, start + 1, end - 1);
        }
    }

    // Handle unary minus: "-X" => "0 - X"
    if (tokens[start] == "-" && start + 1 <= end) {
        auto zero = std::make_shared<ExpressionNode>(0.0);
        auto right = parseTokens(tokens, start + 1, end);
        return std::make_shared<ExpressionNode>(ExpressionNode::Operator::Subtract, zero, right);
    }

    int opIndex = ExpressionParser::findMainOperator(tokens, start, end);
    if (opIndex != -1) {
        auto op = ExpressionNode::parseOperator(tokens[opIndex]);
        auto left = parseTokens(tokens, start, opIndex - 1);
        auto right = parseTokens(tokens, opIndex + 1, end);
        return std::make_shared<ExpressionNode>(op, left, right);
    }

    // Function call or variable/constant
    if (start == end) {
        const std::string& token = tokens[start];
        if (ExpressionParser::isNumber(token)) return std::make_shared<ExpressionNode>(std::stod(token));

        std::string base = token;
        ExpressionNode::loc loc = ExpressionNode::loc::self;

        if (aquiutils::ends_with(token, ".s")) {
            loc = ExpressionNode::loc::source;
            base = token.substr(0, token.size() - 2);
        }
        else if (aquiutils::ends_with(token, ".e")) {
            loc = ExpressionNode::loc::destination;
            base = token.substr(0, token.size() - 2);
        }
        else if (aquiutils::ends_with(token, ".v")) {
            loc = ExpressionNode::loc::average_of_links;
            base = token.substr(0, token.size() - 2);
        }

        auto node = std::make_shared<ExpressionNode>(base);
        node->location = loc;
        return node;
    }

    if (ExpressionParser::isFunction(tokens[start])) {
        std::string func = tokens[start];
        static const std::vector<std::string> supportedFunctions = {
            "_min", "_max", "_exp", "_log", "_abs", "_sgn", "_sqr", "_sqt", "_lpw", "_pos", "_hsd",
            "_ups", "_bkw", "_mon", "_mbs", "_ekr", "_gkr"
        };
        if (std::find(supportedFunctions.begin(), supportedFunctions.end(), func) == supportedFunctions.end()) {
            throw std::runtime_error("Unsupported function: " + func);
        }

        if (tokens[start + 1] != "(") throw std::runtime_error("Expected '('");
        int level = 1, split = start + 2;
        std::vector<ExpressionNode::Ptr> args;
        int last = split;
        for (int i = split; i < end; ++i) {
            if (tokens[i] == "(") level++;
            else if (tokens[i] == ")") level--;
            else if ((tokens[i] == "," || tokens[i] == ";") && level == 1) {
                args.push_back(parseTokens(tokens, last, i - 1));
                last = i + 1;
            }
        }
        args.push_back(parseTokens(tokens, last, end - 1));
        return std::make_shared<ExpressionNode>(func.substr(1), args);
    }

    throw std::runtime_error("Unrecognized expression.");
}


int ExpressionParser::findMainOperator(const vector<string>& tokens, int start, int end) {
    int minPrecedence = 1000, opIndex = -1, level = 0;
    for (int i = start; i <= end; ++i) {
        const string& token = tokens[i];
        if (token == "(") level++;
        else if (token == ")") level--;
        else if (level == 0 && isOperator(token)) {
            int prec = (token == "+" || token == "-") ? 1 :
                (token == "*" || token == "/") ? 2 :
                (token == "^") ? 3 :
                (token == ";") ? 0 : 10;
            if (prec <= minPrecedence) {
                minPrecedence = prec;
                opIndex = i;
            }
        }
    }
    return opIndex;
}

bool ExpressionParser::isOperator(const string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || token == ";";
}

bool ExpressionParser::isFunction(const string& token) {
    return token.length() > 1 && token[0] == '_';
}

bool ExpressionParser::isNumber(const string& token) {
    char* end;
    strtod(token.c_str(), &end);
    return *end == '\0';
}
