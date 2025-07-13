// ExpressionParser.h
#pragma once

#include "ExpressionNode.h"
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class ExpressionParser {
public:
    /**
     * @brief Parses a mathematical expression string into an expression tree.
     * @param expression The input expression string.
     * @return A shared pointer to the root node of the parsed expression tree.
     */
    static ExpressionNode::Ptr parse(const std::string& expression);

private:
    // Tokenization and recursive parsing helpers
    static std::vector<std::string> tokenize(const std::string& s);
    static ExpressionNode::Ptr parseTokens(const std::vector<std::string>& tokens, int start, int end);

    // Utility
    static int findMainOperator(const std::vector<std::string>& tokens, int start, int end);
    static bool isOperator(const std::string& token);
    static bool isFunction(const std::string& token);
    static bool isNumber(const std::string& token);
};
