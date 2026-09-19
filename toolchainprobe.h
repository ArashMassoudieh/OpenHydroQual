/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 *
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */

#ifndef TOOLCHAINPROBE_H
#define TOOLCHAINPROBE_H

#include <QString>

// ---------------------------------------------------------------------------
// Detects whether this machine can build the standalone C++ projects emitted by
// "Model > Export to C++..." (see codegen/).  The generated project needs only
// CMake and a C++17 compiler -- no Qt, no Armadillo, no OpenHydroQual install --
// so the probe is deliberately narrow.
//
// Nothing here is required for *generating* a project.  The GUI uses the probe
// only to decide whether to offer the optional build/run/validate steps, so a
// machine with no toolchain degrades to export-only instead of failing late
// with a wall of CMake output.
// ---------------------------------------------------------------------------
class ToolchainProbe
{
public:
    // Absolute path to a usable cmake, or empty if none was found.  Resolved
    // from PATH first, then from the usual per-platform install locations
    // (CMake's own installer, Visual Studio's bundled copy, Qt Tools, Homebrew).
    // The result is cached for the lifetime of the process; pass true to
    // re-probe after the user has installed something.
    static QString cmakePath(bool forceRefresh = false);

    // "cmake version 3.28.3" reduced to "3.28.3", or empty if unknown.
    static QString cmakeVersion();

    static bool hasCMake() { return !cmakePath().isEmpty(); }

    // A one-line, user-facing reason the build steps are unavailable, or an
    // empty string when they are available.  Suitable for a tooltip.
    static QString unavailableReason();

    // Turns a failed CMake *configure* log into a plain-language explanation
    // when it matches a known "this machine is missing something" signature
    // (no compiler, no generator, Visual Studio not installed, CMake too old).
    // Returns an empty string for ordinary compile errors, which should be
    // reported as-is rather than second-guessed.
    static QString explainConfigureFailure(const QString& log);

    // Platform-specific advice on what to install, for the details pane.
    static QString installHint();

private:
    static QString probeCMake();
};

#endif // TOOLCHAINPROBE_H
