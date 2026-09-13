/*
 * OpenHydroQual - Codegen CLI: load a model and emit a standalone C++ solver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Usage:
 *   ohq_generate <model.ohq> <resources_dir> <out_dir> <ClassName> [stateVar]
 *
 * Mirrors OHQLib/main.cpp's load path, then runs CodeGenerator instead of Solve.
 */
#include "System.h"
#include "Script.h"
#include "CodeGenerator.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 5) {
        std::cerr << "usage: ohq_generate <model.ohq> <resources_dir> <out_dir> "
                     "<ClassName> [stateVar]\n";
        return 1;
    }
    const std::string model = argv[1];
    const std::string res   = argv[2];
    const std::string out   = argv[3];
    const std::string cls   = argv[4];
    const std::string stateVar = (argc > 5) ? argv[5] : "Storage";

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
    opt.className   = cls;
    opt.outputDir   = out;
    opt.stateVariable = stateVar;

    try {
        ohqcg::CodeGenerator().generate(sys, opt);
        std::cout << "Generated: " << out << "/" << cls << ".h\n";
    } catch (const std::exception& e) {
        std::cerr << "Generation failed: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
