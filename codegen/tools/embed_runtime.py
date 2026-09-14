#!/usr/bin/env python3
"""
OpenHydroQual - Codegen: embed the header-only runtime into the generator.

Reads codegen/runtime/*.h and writes
    codegen/include/EmbeddedRuntime.h
    codegen/src/EmbeddedRuntime.cpp
so CodeGenerator can emit a SELF-CONTAINED project folder (runtime copied
alongside the generated model) with no dependency on the source-tree layout.
This matters for the GUI "Export to C++" action on an installed/relocated
binary (Windows/macOS bundles), where codegen/runtime/ is not on disk.

Re-run after editing anything in codegen/runtime/:
    python3 codegen/tools/embed_runtime.py

Each file is emitted as adjacent raw string literals chunked at CHUNK bytes:
MSVC rejects a single literal over 16380 bytes (C2026) but concatenates
adjacent pieces up to 65535, so chunking keeps the generated source
compilable with Visual Studio as the runtime grows.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RT_DIR = os.path.join(ROOT, "runtime")
OUT_H = os.path.join(ROOT, "include", "EmbeddedRuntime.h")
OUT_CPP = os.path.join(ROOT, "src", "EmbeddedRuntime.cpp")

CHUNK = 15000
DELIM = "OHQRT"          # raw-string delimiter; content must not contain )OHQRT"
LIMIT_TOTAL = 65535      # MSVC concatenated-literal limit


def main():
    files = sorted(f for f in os.listdir(RT_DIR) if f.endswith(".h"))
    if not files:
        sys.exit("no runtime headers found in " + RT_DIR)

    entries = []
    for name in files:
        with open(os.path.join(RT_DIR, name), "r", encoding="utf-8") as fh:
            text = fh.read()
        closer = ")" + DELIM + '"'
        if closer in text:
            sys.exit(f"{name}: contains raw-string terminator {closer}; change DELIM")
        if len(text.encode("utf-8")) > LIMIT_TOTAL:
            sys.exit(f"{name}: {len(text)} bytes exceeds MSVC concatenation limit")
        chunks = [text[i:i + CHUNK] for i in range(0, len(text), CHUNK)] or [""]
        lits = "\n".join(f'R"{DELIM}({c}){DELIM}"' for c in chunks)
        entries.append(f'    {{ "{name}",\n{lits} }}')

    header = f"""/*
 * OpenHydroQual - Codegen: embedded header-only runtime (GENERATED FILE)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Produced by codegen/tools/embed_runtime.py from the codegen/runtime headers.
 * Do not edit by hand -- edit the runtime headers and re-run the script.
 */
#ifndef OHQ_EMBEDDED_RUNTIME_H
#define OHQ_EMBEDDED_RUNTIME_H

namespace ohqcg {{

struct RuntimeFile {{
    const char* name;   // file name, e.g. "ohq_massbalance.h"
    const char* text;   // full file contents
}};

// The runtime headers a generated model needs. `count` receives the number
// of entries. Used by CodeGenerator to write a self-contained project.
const RuntimeFile* runtimeFiles(int& count);

}} // namespace ohqcg

#endif // OHQ_EMBEDDED_RUNTIME_H
"""
    body = ",\n".join(entries)
    cpp = f"""/*
 * OpenHydroQual - Codegen: embedded header-only runtime (GENERATED FILE)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Produced by codegen/tools/embed_runtime.py from the codegen/runtime headers.
 * Do not edit by hand -- edit the runtime headers and re-run the script.
 */
#include "EmbeddedRuntime.h"

namespace ohqcg {{

static const RuntimeFile kRuntimeFiles[] = {{
{body}
}};

const RuntimeFile* runtimeFiles(int& count)
{{
    count = static_cast<int>(sizeof(kRuntimeFiles) / sizeof(kRuntimeFiles[0]));
    return kRuntimeFiles;
}}

}} // namespace ohqcg
"""
    with open(OUT_H, "w", encoding="utf-8") as fh:
        fh.write(header)
    with open(OUT_CPP, "w", encoding="utf-8") as fh:
        fh.write(cpp)
    print(f"embedded {len(files)} runtime headers -> {OUT_CPP}")
    for name in files:
        print("  ", name)


if __name__ == "__main__":
    main()
