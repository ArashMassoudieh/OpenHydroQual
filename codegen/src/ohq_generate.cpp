/*
 * OpenHydroQual - Codegen CLI: load a model and emit a standalone C++ solver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Usage:
 *   ohq_generate <model.ohq> <resources_dir> <out_dir> <ClassName> [stateVar]
 *                [--project exe|lib|shared]
 *
 *   --project exe     also write main.cpp + CMakeLists.txt + runtime/ (a
 *                     self-contained folder that builds a forward-model exe)
 *   --project lib     same, but a static library with a C API
 *   --project shared  same, as a shared library (.so/.dll)
 *
 * This is the same code path as the GUI's "Model > Export to C++", so a
 * generated folder can be checked from the command line.
 * Mirrors OHQLib/main.cpp's load path, then runs CodeGenerator instead of Solve.
 */
#include "System.h"
#include "Script.h"
#include "CodeGenerator.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::vector<std::string> pos;
    std::string project;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--project") {
            if (i + 1 >= argc) { std::cerr << "--project needs exe|lib|shared\n"; return 1; }
            project = argv[++i];
        } else {
            pos.push_back(a);
        }
    }
    if (pos.size() < 4) {
        std::cerr << "usage: ohq_generate <model.ohq> <resources_dir> <out_dir> "
                     "<ClassName> [stateVar] [--project exe|lib|shared]\n";
        return 1;
    }
    if (!project.empty() && project != "exe" && project != "lib" && project != "shared") {
        std::cerr << "--project must be exe, lib or shared (got '" << project << "')\n";
        return 1;
    }
    const std::string model = pos[0];
    const std::string res   = pos[1];
    const std::string out   = pos[2];
    const std::string cls   = pos[3];
    const std::string stateVar = (pos.size() > 4) ? pos[4] : "Storage";

    System sys;
    sys.SetDefaultTemplatePath(res + "/");
    sys.SetWorkingFolder(
        QFileInfo(QString::fromStdString(model)).canonicalPath().toStdString() + "/");
    sys.SetSilent(true);

    Script scr(model, &sys);
    std::cout << "Loading model: " << model << "\n";
    sys.CreateFromScript(scr, res + "/settings.json");

    std::cout << "Blocks: " << sys.BlockCount() << "  Links: " << sys.LinksCount() << "\n";

    ohqcg::GenOptions opt;
    opt.className     = cls;
    opt.outputDir     = out;
    opt.stateVariable = stateVar;
    opt.emitProject   = !project.empty();
    opt.asLibrary     = (project == "lib" || project == "shared");
    opt.sharedLibrary = (project == "shared");

    try {
        ohqcg::CodeGenerator().generate(sys, opt);
        std::cout << "Generated: " << out << "/" << cls << ".h\n";
        if (opt.emitProject)
            std::cout << "Project:   " << out << "  (" << project << ")  -> cmake -S " << out
                      << " -B " << out << "/build && cmake --build " << out << "/build\n";
    } catch (const std::exception& e) {
        std::cerr << "Generation failed: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
