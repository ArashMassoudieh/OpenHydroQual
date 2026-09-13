/*
 * OpenHydroQual - Codegen: model -> standalone C++ solver library
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Top-level driver. Given a fully-loaded System, emits a self-contained C++
 * class (header + source) that reproduces the model's transient solve with no
 * expression interpreter. The generated class:
 *   - stores state (balance) variables in a flat array,
 *   - folds constants, hoists per-step values, keeps per-iteration values local,
 *   - links ONLY against the header-only ohq runtime (ohq_intrinsics.h,
 *     ohq_timeseries.h, ohq_solver.h, ohq_linalg.h) -- no Qt, no armadillo,
 *   - exposes the external API:  initialize(tstart, dt0), step(), runTo(t),
 *     time(), and named getters -- so it embeds into any program.
 *
 * The GENERATOR itself links the OHQ core (via libOHQLib + Qt6) to read a model;
 * the GENERATED library does not.
 *
 * Pipeline:  System  --DependencyAnalyzer-->  tiers
 *                     --ExpressionEmitter---->  C++ per quantity
 *                     --CodeGenerator-------->  <Class>.h / <Class>.cpp
 *
 * STATUS: interface + design. ExpressionEmitter (the core translation) is
 * implemented and tested; the runtime + a hand-written target model
 * (demo/two_pond_model.h) compile and solve. Remaining: the System walk that
 * assembles residual/Jacobian/initial-values and writes the files.
 */
#ifndef OHQ_CODE_GENERATOR_H
#define OHQ_CODE_GENERATOR_H

#include <string>

class System;  // aquifolium

namespace ohqcg {

struct GenOptions {
    std::string className   = "GeneratedModel";
    std::string outputDir   = ".";
    std::string stateVariable = "Storage";  // the solutionorder variable to integrate
    bool        emitJacobian  = true;        // analytical Jacobian (else numerical)
    bool        transport     = false;       // constituents (phase 2)
};

class CodeGenerator {
public:
    // Emit <className>.h and <className>.cpp into options.outputDir.
    // Returns true on success.
    bool generate(System& system, const GenOptions& options);  // TODO: phase 2
};

} // namespace ohqcg

#endif // OHQ_CODE_GENERATOR_H
