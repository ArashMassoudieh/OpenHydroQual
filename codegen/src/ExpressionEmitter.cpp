/*
 * OpenHydroQual - Codegen: Expression -> C++ emitter (implementation)
 * Copyright (C) 2025 EnviroInformatics, LLC
 */
#include "ExpressionEmitter.h"

#include <vector>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <charconv>
#include <array>

namespace ohqcg {

// Maps interpreter function names (3-char, no underscore, as stored in
// Expression::function) to the ohq:: runtime. _ekr/_gkr are handled separately
// because their first argument is a time series, not a value.
static std::string mapUnaryOrNary(const std::string& fn)
{
    if (fn == "exp") return "ohq::f_exp";
    if (fn == "log") return "ohq::f_log";
    if (fn == "abs") return "ohq::f_abs";
    if (fn == "sgn") return "ohq::sgn";
    if (fn == "sqr") return "ohq::f_sqr";
    if (fn == "sqt") return "ohq::f_sqt";
    if (fn == "pos") return "ohq::pos";
    if (fn == "hsd") return "ohq::hsd";
    if (fn == "min") return "ohq::f_min";
    if (fn == "max") return "ohq::f_max";
    if (fn == "mon") return "ohq::mon";
    if (fn == "mbs") return "ohq::mbs";
    if (fn == "lpw") return "ohq::lpw";
    if (fn == "ups") return "ohq::ups";
    if (fn == "bkw") return "ohq::bkw";
    return "";
}

std::string ExpressionEmitter::formatConstant(double v)
{
    // Shortest representation that round-trips to the exact same double, so the
    // generated code is both readable and bit-faithful to the model file.
    std::array<char, 64> buf{};
    std::string s;
    auto res = std::to_chars(buf.data(), buf.data() + buf.size(), v);
    if (res.ec == std::errc())
        s.assign(buf.data(), res.ptr);
    else {
        char tmp[64];
        std::snprintf(tmp, sizeof(tmp), "%.17g", v);
        s = tmp;
    }
    // ensure it reads as a double literal
    if (s.find('.') == std::string::npos &&
        s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos &&
        s.find("inf") == std::string::npos &&
        s.find("nan") == std::string::npos)
        s += ".0";
    return s;
}

int ExpressionEmitter::precedence(const std::string& op)
{
    if (op == "^") return 4;
    if (op == "*" || op == "/") return 3;
    if (op == "+" || op == "-") return 2;
    return 0;
}

bool ExpressionEmitter::rightAssoc(const std::string& op) { return op == "^"; }

std::string ExpressionEmitter::apply(const std::string& op, const std::string& l, const std::string& r)
{
    if (op == "^") return "ohq::powr(" + l + ", " + r + ")";
    return "(" + l + " " + op + " " + r + ")";
}

Loc ExpressionEmitter::locationOf(const Expression& leaf)
{
    // Parse suffix from the raw token (Expression::location is private).
    const std::string& t = leaf.text;
    auto dot = t.find_last_of('.');
    if (dot == std::string::npos || dot + 2 != t.size()) return Loc::self;
    char c = t[dot + 1];
    if (c == 's' || c == 'S') return Loc::source;
    if (c == 'e' || c == 'E') return Loc::destination;
    if (c == 'v' || c == 'V') return Loc::average_of_links;
    return Loc::self;
}

std::string ExpressionEmitter::translate(const Expression& e) const
{
    return emitNode(e);
}

std::string ExpressionEmitter::emitNode(const Expression& e) const
{
    if (e.param_constant_expression == "constant")
        return formatConstant(e.constant);

    if (e.param_constant_expression == "parameter") {
        const Loc loc = locationOf(e);
        if (!ctx_.resolveValue)
            throw std::runtime_error("ExpressionEmitter: no value resolver");
        return ctx_.resolveValue(e.parameter, loc);
    }

    // expression node
    if (!e.function.empty())
        return emitFunction(e);

    // plain arithmetic over all terms (single argument group, no ';')
    return emitArithmetic(e, 0, static_cast<int>(e.terms.size()));
}

std::string ExpressionEmitter::emitFunction(const Expression& e) const
{
    // Split the terms into argument groups at ';' boundaries. A term whose sign
    // is ";" starts a new argument.
    const int n = static_cast<int>(e.terms.size());
    std::vector<std::pair<int,int>> groups; // [begin,end)
    int start = 0;
    for (int i = 1; i < n; ++i) {
        if (e.terms[i].sign == ";") { groups.emplace_back(start, i); start = i; }
    }
    groups.emplace_back(start, n);

    const std::string& fn = e.function;

    // Kernel functions: first argument is a time series handle.
    if (fn == "ekr" || fn == "gkr") {
        if (groups.empty()) throw std::runtime_error("_" + fn + " with no arguments");
        const Expression& tsLeaf = e.terms[groups[0].first];
        if (!ctx_.resolveSeries)
            throw std::runtime_error("ExpressionEmitter: no series resolver");
        const std::string handle = ctx_.resolveSeries(tsLeaf.parameter, locationOf(tsLeaf));
        std::string out = handle + "." + fn + "(" + ctx_.timeVar;
        for (size_t g = 1; g < groups.size(); ++g)
            out += ", " + emitArithmetic(e, groups[g].first, groups[g].second);
        out += ")";
        return out;
    }

    const std::string mapped = mapUnaryOrNary(fn);
    if (mapped.empty())
        throw std::runtime_error("ExpressionEmitter: unsupported function _" + fn);

    std::string out = mapped + "(";
    for (size_t g = 0; g < groups.size(); ++g) {
        if (g) out += ", ";
        out += emitArithmetic(e, groups[g].first, groups[g].second);
    }
    out += ")";
    return out;
}

std::string ExpressionEmitter::emitArithmetic(const Expression& parent, int begin, int end) const
{
    if (begin >= end) return "0.0";

    // Build operand strings and binary operators for this run.
    // terms[begin].sign is the leading (unary) sign; terms[k].sign for k>begin
    // is the binary operator preceding operand k.
    std::vector<std::string> values;
    std::vector<std::string> ops;

    std::string first = emitNode(parent.terms[begin]);
    if (parent.terms[begin].sign == "-") first = "(-" + first + ")";
    values.push_back(first);

    for (int k = begin + 1; k < end; ++k) {
        ops.push_back(parent.terms[k].sign);        // "+","-","*","/","^"
        values.push_back(emitNode(parent.terms[k]));
    }

    if (ops.empty()) return values[0];

    // Shunting-yard over values/ops producing a fully-parenthesized C++ string.
    std::vector<std::string> outStack;
    std::vector<std::string> opStack;
    outStack.push_back(values[0]);

    auto popApply = [&]() {
        const std::string op = opStack.back(); opStack.pop_back();
        const std::string r = outStack.back(); outStack.pop_back();
        const std::string l = outStack.back(); outStack.pop_back();
        outStack.push_back(apply(op, l, r));
    };

    for (size_t i = 0; i < ops.size(); ++i) {
        const std::string& op = ops[i];
        while (!opStack.empty()) {
            const std::string& top = opStack.back();
            const int pt = precedence(top), po = precedence(op);
            if (pt > po || (pt == po && !rightAssoc(op))) popApply();
            else break;
        }
        opStack.push_back(op);
        outStack.push_back(values[i + 1]);
    }
    while (!opStack.empty()) popApply();
    return outStack.back();
}

} // namespace ohqcg
