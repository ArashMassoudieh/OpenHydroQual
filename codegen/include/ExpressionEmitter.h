/*
 * OpenHydroQual - Codegen: Expression -> C++ emitter
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Translates an aquifolium Expression tree into a C++ source string that
 * evaluates to the same value as Expression::calc, but with no interpreter:
 * constants become literals, variable references become caller-resolved
 * symbols, operators map to C++ (with '^' -> ohq::powr and correct precedence),
 * and the intrinsic functions map to the ohq:: runtime (ohq_intrinsics.h /
 * ohq_timeseries.h).
 *
 * The emitter is deliberately decoupled from the model walk: the caller supplies
 * resolver callbacks that turn a (name, location) into the concrete C++ symbol
 * for THIS object in the generated code (e.g. a local temporary, a folded
 * literal, a source/destination block field, or a time-series handle).
 */
#ifndef OHQ_EXPRESSION_EMITTER_H
#define OHQ_EXPRESSION_EMITTER_H

#include <string>
#include <functional>
#include "Expression.h"   // aquifolium expression tree

namespace ohqcg {

// Location of a referenced quantity, mirroring Expression::loc.
enum class Loc { self = 0, source = 1, destination = 2, average_of_links = 3 };

struct EmitContext {
    // Resolve a scalar quantity reference (name + location) to a C++ expression
    // string valid at the emission point (e.g. "self.area_", "src.head_",
    // "0.2" for a folded constant).
    std::function<std::string(const std::string& name, Loc loc)> resolveValue;

    // Resolve a time-series reference (the first argument of _ekr/_gkr) to a C++
    // handle on which .ekr(...)/.gkr(...) can be called (e.g. "ts_timeseries_").
    std::function<std::string(const std::string& name, Loc loc)> resolveSeries;

    // C++ symbol holding the current (forecast) time, passed to kernels.
    std::string timeVar = "t_new";
};

class ExpressionEmitter {
public:
    explicit ExpressionEmitter(EmitContext ctx) : ctx_(std::move(ctx)) {}

    // Emit a C++ expression string for the whole tree.
    // (Named 'translate' rather than 'emit' because Qt defines 'emit' as a macro.)
    std::string translate(const Expression& e) const;

private:
    std::string emitNode(const Expression& e) const;
    // Emit an arithmetic run of terms [begin,end) honoring C++ precedence.
    std::string emitArithmetic(const Expression& parent, int begin, int end) const;
    std::string emitFunction(const Expression& e) const;

    static std::string formatConstant(double v);
    static int  precedence(const std::string& op);
    static bool rightAssoc(const std::string& op);
    static std::string apply(const std::string& op, const std::string& l, const std::string& r);
    // Parse a parameter leaf's location from its raw text ("x", "x.s", "x.e", "x.v").
    static Loc locationOf(const Expression& leaf);

    EmitContext ctx_;
};

} // namespace ohqcg

#endif // OHQ_EXPRESSION_EMITTER_H
