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
 * OHQ-MCMC - standalone headless Bayesian parameter estimation for OpenHydroQual
 *
 *   OHQ-MCMC <model.ohq> [working_folder]
 *
 * Reproduces MainWindow::oninverserun() without a GUI and writes the same
 * outputs the GUI produces:
 *
 *   mcmc.txt                            the chain (name from the model's MCMC settings)
 *   MCMC_details.txt                    per-sample solver log
 *   Posterior_distributions.txt         marginal posterior densities
 *   posterior_percentiles.txt           2.5 / 50 / 97.5 percentiles and mean
 *   Realizations_<obs>.txt              posterior predictive realizations
 *   Predicted_95p_Bracket<obs>.txt      2.5 / 50 / 97.5 predictive bracket
 *   output.txt, observedoutput.txt      forward run at the posterior median
 *   fit_measures.txt, mapped_modeled_results.txt,
 *   objective_function_values.txt, errors.txt, state.json
 */

#include "ohq_common.h"
#include "ohq_kernel.h"
#include "MCMC.h"
#include <algorithm>
#include <vector>
#include <QtGlobal>

// The library calls qDebug() in ~40 places; during sampling those land on stderr
// once per forward solve and bury the progress report. Drop debug/info here and
// let anything more serious through.
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
        std::cout << "Usage: OHQ-MCMC <model.ohq> [working_folder] [options]\n"
                  << "  working_folder defaults to the folder containing the model file.\n"
                  << "  --continue  resume an interrupted run from the chain file already\n"
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

    // --continue may appear anywhere after the model file
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
    // when one is loaded and falls back to System::Solve otherwise.
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
        if (!ohq::LoadKernel(kernel))          return 1;
        if (!ohq::VerifyKernelMatches(system)) return 1;
    }
    else
        std::cout << "Forward model  : interpreter" << std::endl;

    CMCMC<ohq::KernelSystem> mcmc(&system);
    mcmc.FileInformation.outputpath = workingfolder;
    if (system.object("MCMC") == nullptr)
    {
        std::cout << "The model does not define an MCMC settings object." << std::endl;
        return 1;
    }
    mcmc.SetParameters(system.object("MCMC"));

    // Resume support. CMCMC::Perform() reads the existing chain and restarts
    // from the last recorded sample when continue_mcmc is set; the chain file is
    // then appended to rather than truncated.
    if (resume)
    {
        // CMCMC::SetParameters already resolves samples_filename against the
        // output path (MCMC.hpp:151-153), so outputfilename carries the folder
        // whenever the model gave one. Prepending workingfolder again produced
        // "<wf>/<wf>/mcmc.txt", which never exists -- so --continue silently
        // started a fresh run and overwrote the chain.
        const std::string ofn = mcmc.FileInformation.outputfilename;
        const bool has_path = ofn.find('/') != std::string::npos
                           || ofn.find('\\') != std::string::npos;
        const std::string chain = has_path ? ofn : workingfolder + ofn;
        if (QFileInfo::exists(QString::fromStdString(chain)))
        {
            mcmc.MCMC_Settings.continue_mcmc      = true;
            mcmc.MCMC_Settings.continue_filename  = chain;
            std::cout << "Resuming from " << chain << std::endl;
        }
        else
        {
            std::cout << "--continue given but no chain file at " << chain
                      << "; starting a fresh run." << std::endl;
        }
    }
    system.SetAllParents();
    system.SetParameterEstimationMode(parameter_estimation_options::inverse_model);

    std::cout << "\nSamples   : " << mcmc.MCMC_Settings.total_number_of_samples
              << "  (burn-in " << mcmc.MCMC_Settings.burnout_samples << ")" << std::endl;
    std::cout << "Chains    : " << mcmc.MCMC_Settings.number_of_chains
              << "   threads " << mcmc.MCMC_Settings.numberOfThreads << std::endl;
    std::cout << "Parameters: " << system.ParametersCount() << std::endl;
    // Sampling is silent: the per-block progress report below is the only output.
    // Errors are still recorded and land in errors.txt after the forward run.
    system.SetSilent(true);

    std::cout << "\nRunning MCMC (this writes the chain incrementally to "
              << mcmc.FileInformation.outputfilename << ") ..." << std::endl;

    // Perform() runs the chain, then builds the posterior marginals and
    // percentiles and calls ProduceRealizations() for the predictive bracket.
    mcmc.Perform();

    std::cout << "\nMCMC finished. Posterior summaries and realizations written." << std::endl;

    // Forward run at the posterior median so the folder also carries the usual
    // per-run outputs. Parameter::SetValue drives ApplyParameters() inside Solve.
    std::cout << "\nPosterior medians:" << std::endl;
    for (int i = 0; i < (int)system.ParametersCount(); i++)
    {
        Parameter *p = system.GetParameter(i);
        if (p == nullptr) continue;

        // GetMCMCSamples() holds one series per chain, already trimmed to
        // post-burn-in samples by CMCMC::Perform(). Pool them and take the median.
        TimeSeriesSet<double> &chains = p->GetMCMCSamples();
        std::vector<double> pooled;
        for (int c = 0; c < (int)chains.size(); c++)
            for (const auto &pt : chains[c])
                pooled.push_back(pt.c);

        if (pooled.empty()) continue;
        std::sort(pooled.begin(), pooled.end());
        double median = pooled[pooled.size() / 2];
        p->SetValue(median);
        std::cout << "  " << p->GetName() << " = " << median
                  << "   (" << pooled.size() << " samples)" << std::endl;
    }
    system.ApplyParameters();
    system.SetSilent(false);        // the closing forward run reports normally
    ohq::WriteForwardRunOutputs(system, workingfolder);

    std::cout << "\nAll done. Outputs are in " << workingfolder << std::endl;
    return 0;
}
