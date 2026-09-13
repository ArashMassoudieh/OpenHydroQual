/*
 * OpenHydroQual - Codegen: dependency analysis & quantity tiering
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Classifies every (object, quantity) of a loaded model into the loop level at
 * which it must be recomputed, which drives constant folding and code hoisting
 * for an extreme-efficiency generated solver:
 *
 *   Constant       depends on neither state nor time  -> folded into a literal
 *   PerStep        depends on time but not on state    -> computed once per step
 *   PerIteration   depends on a state variable         -> computed each Newton it.
 *
 * The two boolean labels (dependsOnState / dependsOnTime) are propagated
 * transitively over the quantity dependency graph. Edges come from:
 *   - Expression variable references (self / source / destination / links),
 *   - Quan types: timeseries & prec_timeseries -> dependsOnTime;
 *                 balance (state variable)      -> dependsOnState (a root);
 *                 source                        -> dependsOnTime (forcing).
 * The same graph yields the Jacobian sparsity pattern (which residual rows a
 * given state variable can reach).
 *
 * STATUS: interface + design. Implementation lands with CodeGenerator (walks
 * System / Block / Link / QuanSet). The ExpressionEmitter it feeds is complete
 * and tested (codegen/tests/test_emitter.cpp).
 */
#ifndef OHQ_DEPENDENCY_ANALYZER_H
#define OHQ_DEPENDENCY_ANALYZER_H

#include <string>
#include <map>
#include <vector>

class System;  // aquifolium

namespace ohqcg {

enum class Tier { Constant, PerStep, PerIteration };

struct QuantityInfo {
    int         objectIndex = -1;      // index into blocks/links
    std::string objectName;
    std::string quantityName;
    Tier        tier = Tier::Constant;
    bool        dependsOnState = false;
    bool        dependsOnTime  = false;
    double      foldedValue = 0.0;     // valid when tier == Constant
    bool        isFolded = false;
    // Names of quantities this one references (resolved within its object /
    // across a link's endpoints), for ordering the straight-line emission.
    std::vector<std::string> dependencies;
};

// Key: "objectName::quantityName"
using AnalysisResult = std::map<std::string, QuantityInfo>;

class DependencyAnalyzer {
public:
    // Analyze a fully-loaded system and return per-quantity tiering.
    AnalysisResult analyze(System& system) const;   // TODO: implement in phase 2
};

} // namespace ohqcg

#endif // OHQ_DEPENDENCY_ANALYZER_H
