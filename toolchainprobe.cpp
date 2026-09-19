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

#include "toolchainprobe.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>

namespace {

// Cached across calls: probing spawns a process, and the export dialog asks
// several times while the user toggles checkboxes.
bool    g_probed = false;
QString g_cmakePath;
QString g_cmakeVersion;

// Install locations to try when cmake is not on PATH. These are the paths the
// mainstream installers actually use; a machine that keeps CMake somewhere else
// entirely is expected to have it on PATH.
QStringList candidateCMakePaths()
{
    QStringList out;
#ifdef _WIN32
    const QStringList roots = {
        QProcessEnvironment::systemEnvironment().value("ProgramFiles", "C:/Program Files"),
        QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "C:/Program Files (x86)")
    };
    const char* editions[] = { "2022", "2019" };
    const char* skus[]     = { "Enterprise", "Professional", "Community", "BuildTools" };
    for (const QString& root : roots) {
        if (root.isEmpty()) continue;
        out << root + "/CMake/bin/cmake.exe";
        // Visual Studio ships a private CMake with the C++ CMake workload.
        for (const char* edition : editions)
            for (const char* sku : skus)
                out << root + "/Microsoft Visual Studio/" + QLatin1String(edition) + "/"
                        + QLatin1String(sku)
                        + "/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe";
    }
    // Qt's maintenance tool installs a CMake alongside the toolchains.
    out << "C:/Qt/Tools/CMake_64/bin/cmake.exe"
        << "D:/Qt/Tools/CMake_64/bin/cmake.exe";
#elif defined(__APPLE__)
    out << "/opt/homebrew/bin/cmake"
        << "/usr/local/bin/cmake"
        << "/Applications/CMake.app/Contents/bin/cmake";
#else
    out << "/usr/bin/cmake"
        << "/usr/local/bin/cmake"
        << "/snap/bin/cmake";
#endif
    return out;
}

// Runs "<path> --version" and returns the parsed version, or empty if the
// binary is missing, not executable, or does not answer like CMake.
QString queryVersion(const QString& path)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(path, QStringList() << "--version");
    if (!p.waitForStarted(5000)) return QString();
    if (!p.waitForFinished(10000)) { p.kill(); p.waitForFinished(2000); return QString(); }
    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) return QString();

    const QString text = QString::fromLocal8Bit(p.readAll());
    const QRegularExpressionMatch m =
        QRegularExpression("cmake version ([0-9]+(?:\\.[0-9]+)*)").match(text);
    // A CMake that answers --version with an unexpected banner is still usable;
    // report an unknown version rather than rejecting it.
    return m.hasMatch() ? m.captured(1) : QString("unknown");
}

} // namespace

QString ToolchainProbe::probeCMake()
{
    // PATH first: it is what the user's own shell would pick.
    const QString onPath = QStandardPaths::findExecutable("cmake");
    if (!onPath.isEmpty()) {
        const QString v = queryVersion(onPath);
        if (!v.isEmpty()) { g_cmakeVersion = v; return onPath; }
    }
    for (const QString& candidate : candidateCMakePaths()) {
        if (!QFileInfo(candidate).isExecutable()) continue;
        const QString v = queryVersion(candidate);
        if (!v.isEmpty()) { g_cmakeVersion = v; return QDir::toNativeSeparators(candidate); }
    }
    g_cmakeVersion.clear();
    return QString();
}

QString ToolchainProbe::cmakePath(bool forceRefresh)
{
    if (forceRefresh) g_probed = false;
    if (!g_probed) { g_cmakePath = probeCMake(); g_probed = true; }
    return g_cmakePath;
}

QString ToolchainProbe::cmakeVersion()
{
    cmakePath();
    return g_cmakeVersion;
}

QString ToolchainProbe::unavailableReason()
{
    if (cmakePath().isEmpty())
        return QObject::tr("CMake was not found on this machine, so the generated project "
                           "cannot be built here. It will still be exported.");
    return QString();
}

QString ToolchainProbe::installHint()
{
#ifdef _WIN32
    return QObject::tr(
        "To build exported projects on this machine, install:\n"
        "  - Visual Studio 2022 Build Tools with the \"Desktop development with C++\" "
        "workload (free), and\n"
        "  - CMake 3.16 or newer (https://cmake.org/download), added to PATH.\n\n"
        "The exported folder is self-contained, so it can also be copied to another "
        "machine and built there with:\n"
        "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release\n"
        "  cmake --build build --config Release");
#elif defined(__APPLE__)
    return QObject::tr(
        "To build exported projects on this machine, install:\n"
        "  - the Xcode command line tools (xcode-select --install), and\n"
        "  - CMake 3.16 or newer (brew install cmake).\n\n"
        "The exported folder is self-contained, so it can also be copied to another "
        "machine and built there with:\n"
        "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release\n"
        "  cmake --build build");
#else
    return QObject::tr(
        "To build exported projects on this machine, install a C++17 compiler and CMake, "
        "for example:\n"
        "  sudo apt install build-essential cmake\n\n"
        "The exported folder is self-contained, so it can also be copied to another "
        "machine and built there with:\n"
        "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release\n"
        "  cmake --build build");
#endif
}

QString ToolchainProbe::explainConfigureFailure(const QString& log)
{
    // Only the signatures that mean "this machine is missing a tool". Anything
    // else is a genuine problem with the generated code or the model, and must
    // be shown verbatim rather than paraphrased into something reassuring.
    struct Signature { const char* needle; const char* explanation; };
    static const Signature signatures[] = {
        { "No CMAKE_CXX_COMPILER could be found",
          QT_TR_NOOP("No C++ compiler was found on this machine. The project was generated "
                     "successfully, but it cannot be compiled here.") },
        { "CMAKE_CXX_COMPILER not set",
          QT_TR_NOOP("No C++ compiler was found on this machine. The project was generated "
                     "successfully, but it cannot be compiled here.") },
        { "No CMAKE_C_COMPILER could be found",
          QT_TR_NOOP("No C/C++ compiler was found on this machine. The project was generated "
                     "successfully, but it cannot be compiled here.") },
        { "Could not find any instance of Visual Studio",
          QT_TR_NOOP("CMake is installed, but no Visual Studio C++ toolchain was found. The "
                     "project was generated successfully, but it cannot be compiled here.") },
        { "is not a full path to an existing compiler tool",
          QT_TR_NOOP("The configured C++ compiler could not be run. The project was generated "
                     "successfully, but it cannot be compiled here.") },
        { "The C++ compiler\n",
          QT_TR_NOOP("The C++ compiler on this machine failed CMake's basic test compile. The "
                     "project was generated successfully, but it cannot be compiled here.") },
        { "CMake 3.16 or higher is required",
          QT_TR_NOOP("The CMake on this machine is too old; the generated project requires "
                     "CMake 3.16 or newer.") },
        { "Could not create named generator",
          QT_TR_NOOP("The build-system generator CMake selected is not available on this "
                     "machine. The project was generated successfully, but it cannot be "
                     "compiled here.") }
    };
    for (const Signature& s : signatures)
        if (log.contains(QLatin1String(s.needle), Qt::CaseInsensitive))
            return QObject::tr(s.explanation);
    return QString();
}
