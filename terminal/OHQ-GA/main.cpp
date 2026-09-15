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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */

/*
 * OHQ-GA - standalone headless genetic-algorithm calibration for OpenHydroQual
 *
 *   OHQ-GA <model.ohq> [working_folder]
 *
 * Reproduces MainWindow::onoptimize() without a GUI and writes the same
 * outputs the GUI produces:
 *
 *   GA_output.txt / detail_GA.txt       generation-by-generation GA log
 *                                       (names from the model's Optimizer settings)
 *   outputs.txt, observedoutputs.txt    best individual's model output
 *   output.txt, observedoutput.txt      forward run at the best parameter set
 *   fit_measures.txt, mapped_modeled_results.txt,
 *   objective_function_values.txt, errors.txt, state.json
 */

#include "ohq_common.h"
#include "ohq_kernel.h"
#include "GA.h"
#include <vector>
#include <QtGlobal>

// The library calls qDebug() in ~40 places; during a GA those land on stderr once
// per forward solve and bury the generation reports. Drop debug/info, keep the rest.
static void QuietMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Q_UNUSED(ctx);
    if (type == QtDebugMsg || type == QtInfoMsg) return;
    std::cerr << msg.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(QuietMessageHandler);

    if (argc < 2)
    {
        std::cout << "Usage: OHQ-GA <model.ohq> [working_folder] [options]\n"
                  << "  working_folder defaults to the folder containing the model file.\n"
                  << "  --continue  seed the initial population from the GA output already\n"
                  << "              in the working folder, instead of starting over.\n"
                  << "  --dependency-jacobian  assemble the transport Jacobian by re-evaluating\n"
                  << "              only the residual rows each perturbed variable can reach.\n"
                  << "              Exact, and much faster for models with many blocks.\n"
                  << "  --sparse    solve J*dx=F with a sparse LU instead of a dense inverse.\n"
                  << "  --verify-jacobian  assemble both ways and log max|dJ| (diagnostic).\n"
                  << "  --kernel <lib.so>  run the forward model with a codegen-generated\n"
                  << "              shared library instead of the interpreter. The library must\n"
                  << "              be generated from THIS model; parameter and observation\n"
                  << "              names are checked before the run starts.\n";
        return 1;
    }

    bool resume = false, dep_jac = false, sparse = false, verify = false;
    std::string kernel;
    std::vector<std::string> pos;
    for (int i = 1; i < argc; i++)
    {
        std::string a = argv[i];
        if (a == "--continue" || a == "-c")   resume = true;
        else if (a == "--dependency-jacobian") dep_jac = true;
        else if (a == "--sparse")              sparse  = true;
        else if (a == "--verify-jacobian")     verify  = true;
        else if (a == "--kernel")
        {
            if (i + 1 >= argc) { std::cout << "--kernel needs a path to the generated library" << std::endl; return 1; }
            kernel = argv[++i];
        }
        else pos.push_back(a);
    }
    const std::string modelfile = pos[0];
    std::string workingfolder = (pos.size() > 1)
        ? ohq::EnsureTrailingSlash(pos[1])
        : ohq::EnsureTrailingSlash(QFileInfo(QString::fromStdString(modelfile))
                                     .absolutePath().toStdString());
    QDir().mkpath(QString::fromStdString(workingfolder));

    std::cout << "Model file     : " << modelfile << std::endl;
    std::cout << "Working folder : " << workingfolder << std::endl;
    std::cout << "Templates      : " << ohq::ResourcePath() << std::endl;

    // KernelSystem is a System whose Solve() delegates to a generated library
    // when one is loaded and falls back to System::Solve otherwise, so the two
    // paths differ only in the forward model.
    ohq::KernelSystem system;
    if (!ohq::LoadModel(system, modelfile, workingfolder, ohq::ResourcePath()))
        return 1;
    if (!ohq::VerifyReadyForEstimation(system))
        return 1;

    // Solver overrides from the command line, applied after the model is read so
    // they win over the .ohq. Defaults leave the model's own settings alone.
    if (dep_jac) { system.SetProperty("jacobian_assembly", "Dependency");
                   std::cout << "Jacobian assembly: Dependency" << std::endl; }
    if (sparse)  { system.SetProperty("jacobian_method", "Sparse");
                   std::cout << "Linear solver: Sparse (SuperLU)" << std::endl; }
    if (verify)  { system.SetProperty("verify_jacobian", "1");
                   std::cout << "Jacobian verification ON (slow; logs max|dJ|)" << std::endl; }

    system.SetSystemSettings();

    if (!kernel.empty())
    {
        std::cout << "Forward model  : generated kernel " << kernel << std::endl;
        if (!ohq::LoadKernel(kernel))        return 1;
        if (!ohq::VerifyKernelMatches(system)) return 1;
    }
    else
        std::cout << "Forward model  : interpreter" << std::endl;

    CGA<ohq::KernelSystem> optimizer(&system);
    if (system.object("Optimizer") == nullptr)
    {
        std::cout << "The model does not define an Optimizer settings object." << std::endl;
        return 1;
    }
    optimizer.SetParameters(system.object("Optimizer"));
    optimizer.filenames.pathname = workingfolder;

    // Resume support. The GA has no notion of a partial generation, so a resumed
    // run re-seeds its initial population from the best individuals of the
    // previous output rather than continuing mid-generation.
    if (resume)
    {
        const std::string prev = workingfolder + "GA_output.txt";
        if (QFileInfo::exists(QString::fromStdString(prev)))
        {
            optimizer.filenames.getfromfilename = prev;
            std::cout << "Seeding initial population from " << prev << std::endl;
        }
        else
        {
            std::cout << "--continue given but no GA_output.txt at " << prev
                      << "; starting a fresh run." << std::endl;
        }
    }
    system.SetAllParents();
    system.SetParameterEstimationMode(parameter_estimation_options::optimize);

    std::cout << "\nGenerations : " << optimizer.GA_params.nGen << std::endl;
    std::cout << "Population  : " << optimizer.GA_params.maxpop << std::endl;
    std::cout << "Parameters  : " << system.ParametersCount() << std::endl;
    // Silent during the search: the model's per-solve warnings would otherwise
    // repeat 2000 times. Errors are still recorded and written to errors.txt.
    system.SetSilent(true);

    std::cout << "\nRunning GA ..." << std::endl;

    optimizer.optimize();

    system.SetSilent(false);        // the closing forward run reports normally

    std::cout << "\nGA finished. Writing best-individual outputs ..." << std::endl;
    optimizer.Model_out.GetOutputs().write(workingfolder + "outputs.txt");
    optimizer.Model_out.GetObservedOutputs().write(workingfolder + "observedoutputs.txt");
    optimizer.Model_out.errorhandler.Write(workingfolder + "errors.txt");

    // Carry the optimized parameter values back into the main system, exactly as
    // MainWindow::onoptimize() does, then run forward once at that parameter set.
    system.TransferResultsFrom(&optimizer.Model_out);
    system.Parameters() = optimizer.Model_out.Parameters();
    system.SetOutputItems();

    std::cout << "\nBest parameter set:" << std::endl;
    for (int i = 0; i < (int)system.ParametersCount(); i++)
    {
        Parameter *p = system.GetParameter(i);
        if (p) std::cout << "  " << p->GetName() << " = " << p->GetValue() << std::endl;
    }

    system.ApplyParameters();
    ohq::WriteForwardRunOutputs(system, workingfolder);

    std::cout << "\nAll done. Outputs are in " << workingfolder << std::endl;
    return 0;
}
