/*
 * OpenHydroQual - Codegen: embedded header-only runtime (GENERATED FILE)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Produced by codegen/tools/embed_runtime.py from the codegen/runtime headers.
 * Do not edit by hand -- edit the runtime headers and re-run the script.
 */
#ifndef OHQ_EMBEDDED_RUNTIME_H
#define OHQ_EMBEDDED_RUNTIME_H

namespace ohqcg {

struct RuntimeFile {
    const char* name;   // file name, e.g. "ohq_massbalance.h"
    const char* text;   // full file contents
};

// The runtime headers a generated model needs. `count` receives the number
// of entries. Used by CodeGenerator to write a self-contained project.
const RuntimeFile* runtimeFiles(int& count);

} // namespace ohqcg

#endif // OHQ_EMBEDDED_RUNTIME_H
