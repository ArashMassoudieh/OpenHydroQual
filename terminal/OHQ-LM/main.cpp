/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 EnviroInformatics, LLC
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

/*
 * OHQ-LM - standalone headless Levenberg-Marquardt calibration
 *
 *   OHQ-LM <model.ohq> [working_folder] [options]
 *
 * Minimises the same negative log-likelihood OHQ-GA minimises, but by
 * Gauss-Newton steps on the residual vector instead of by selection over a
 * population. It is LOCAL: it starts from each parameter's current value in the
 * model file, so the usual sequence is OHQ-GA first and OHQ-LM second, on a
 * model whose parameter values have been updated with the GA's estimates.
 *
 * Writes:
 *   LM_output.txt        iteration log: objective, lambda, accept/reject,
 *                        parameter values in model space
 *   LM_covariance.txt    (J'J)^-1 at the solution: standard errors, 95%
 *                        intervals and the parameter correlation matrix
 *   outputs.txt, observedoutputs.txt, output.txt, observedoutput.txt,
 *   fit_measures.txt, errors.txt, state.json   as OHQ-GA writes them
 */

#include "ohq_common.h"
#include "ohq_kernel.h"
#include "LM/LM.h"
#include <vector>
#include <cmath>
#include <QtGlobal>

static void QuietMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Q_UNUSED(ctx);
    if (type == QtDebugMsg || type == QtInfoMsg) return;
    std::cerr << msg.toStdString() << std::endl;
}

// ---------------------------------------------------------------------------
// --verify-residuals
//
// The whole design rests on one identity: the residual vector LM differentiates
// reconstructs, exactly, the misfit the GA minimises.
//
//     0.5*||r||^2 + sum_k c_k*log(sigma_k)  ==  System::CalcMisfit()
//
// This checks it on the model as loaded, at its current parameter values, and
// reports the absolute and relative difference. Anything above rounding means
// the two have drifted and LM is optimising something other than the likelihood.
// ---------------------------------------------------------------------------
static int VerifyResiduals(ohq::KernelSystem &system)
{
    std::cout << "\nVerifying the residual decomposition against CalcMisfit() ..." << std::endl;
    system.SetSilent(true);
    system.ApplyParameters();
    system.Solve();
    if (system.GetSolutionFailed())
    {
        std::cout << "The model failed to solve, so there is nothing to compare." << std::endl;
        return 1;
    }

    const double misfit = system.CalcMisfit();

    std::vector<double> r;
    std::vector<ResidualBlock> blocks;
    std::string offender;
    const bool decomposable = system.ResidualVector(r, blocks, &offender);
    system.SetSilent(false);

    if (!decomposable)
    {
        std::cout << "Observation '" << offender << "' uses a comparison method with no "
                  << "sum-of-squares form, so LM cannot run on this model." << std::endl;
        return 1;
    }

    double reconstructed = 0;
    for (size_t k = 0; k < blocks.size(); k++)
        reconstructed += blocks[k].NegLogLikelihood();

    const double abs_err = std::fabs(reconstructed - misfit);
    const double rel_err = abs_err / std::max(std::fabs(misfit), 1e-300);

    std::cout << "  observations      : " << blocks.size() << std::endl;
    std::cout << "  residuals         : " << r.size() << std::endl;
    for (size_t k = 0; k < blocks.size(); k++)
        std::cout << "    " << system.observation((unsigned int)k)->GetName()
                  << ": n = " << blocks[k].r.size()
                  << ", sigma = " << blocks[k].sigma
                  << ", effective N = " << blocks[k].log_sigma_coeff
                  << ", -logL = " << blocks[k].NegLogLikelihood() << std::endl;
    std::cout.precision(17);
    std::cout << "  CalcMisfit()      : " << misfit << std::endl;
    std::cout << "  from residuals    : " << reconstructed << std::endl;
    std::cout.precision(6);
    std::cout << "  absolute error    : " << abs_err << std::endl;
    std::cout << "  relative error    : " << rel_err << std::endl;

    if (rel_err < 1e-12)
    {
        std::cout << "MATCH: the residual vector reproduces the misfit to rounding." << std::endl;
        return 0;
    }
    std::cout << "MISMATCH: the residual decomposition does not reproduce the misfit."
              << std::endl;
    return 1;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(QuietMessageHandler);

    if (argc < 2)
    {
        std::cout << "Usage: OHQ-LM <model.ohq> [working_folder] [options]\n"
                  << "  working_folder defaults to the folder containing the model file.\n"
                  << "  --verify-residuals  check that the residual vector reproduces\n"
                  << "              CalcMisfit() at the current parameters, then exit.\n"
                  << "              Run this once on a new model before trusting a\n"
                  << "              calibration; it is cheap (one forward solve).\n"
                  << "  --iterations <n>    maximum LM iterations (default 50).\n"
                  << "  --lambda0 <x>       initial Marquardt damping (default 1e-2).\n"
                  << "  --fd-step <x>       finite-difference step as a fraction of each\n"
                  << "              parameter's range (default 1e-3).\n"
                  << "  --central           central differences: twice the solves per\n"
                  << "              iteration, less finite-difference bias.\n"
                  << "  --no-profile-sigma  keep a calibrated error_standard_deviation in\n"
                  << "              the LM vector instead of setting it analytically.\n"
                  << "  --threads <n>       parallel model solves per Jacobian (default 8).\n"
                  << "  --kernel <lib.so>   run the forward model with a codegen-generated\n"
                  << "              shared library instead of the interpreter.\n";
        return 1;
    }

    // Tri-state for the boolean switches: a flag that was not given must leave
    // the model's own LM settings alone, not silently reset them to the
    // command-line default.
    bool verify_residuals = false;
    int  central = -1, profile_sigma = -1;   // -1 = not specified
    int iterations = -1, threads = -1;
    double lambda0 = -1, fd_step = -1;
    std::string kernel;
    std::vector<std::string> pos;
    for (int i = 1; i < argc; i++)
    {
        std::string s = argv[i];
        if      (s == "--verify-residuals") verify_residuals = true;
        else if (s == "--central")          central = 1;
        else if (s == "--no-profile-sigma") profile_sigma = 0;
        else if (s == "--iterations") { if (i+1>=argc) { std::cout << "--iterations needs a value" << std::endl; return 1; } iterations = atoi(argv[++i]); }
        else if (s == "--threads")    { if (i+1>=argc) { std::cout << "--threads needs a value" << std::endl; return 1; }    threads = atoi(argv[++i]); }
        else if (s == "--lambda0")    { if (i+1>=argc) { std::cout << "--lambda0 needs a value" << std::endl; return 1; }    lambda0 = atof(argv[++i]); }
        else if (s == "--fd-step")    { if (i+1>=argc) { std::cout << "--fd-step needs a value" << std::endl; return 1; }    fd_step = atof(argv[++i]); }
        else if (s == "--kernel")     { if (i+1>=argc) { std::cout << "--kernel needs a path" << std::endl; return 1; }      kernel = argv[++i]; }
        else pos.push_back(s);
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

    ohq::KernelSystem system;
    if (!ohq::LoadModel(system, modelfile, workingfolder, ohq::ResourcePath()))
        return 1;
    if (!ohq::VerifyReadyForEstimation(system))
        return 1;

    system.SetSystemSettings();
    system.SetAllParents();
    system.SetParameterEstimationMode(parameter_estimation_options::inverse_model);

    if (!kernel.empty())
    {
        std::cout << "Forward model  : generated kernel " << kernel << std::endl;
        if (!ohq::LoadKernel(kernel))          return 1;
        if (!ohq::VerifyKernelMatches(system)) return 1;
    }
    else
        std::cout << "Forward model  : interpreter" << std::endl;

    if (verify_residuals)
        return VerifyResiduals(system);

    CLM<ohq::KernelSystem> optimizer(&system);
    optimizer.filenames.pathname = workingfolder;
    // Settings from the model's LM object when it has one, then command-line
    // overrides on top, so a flag always wins over the file.
    if (system.object("LM") != nullptr)
        optimizer.SetParameters(system.object("LM"));
    if (iterations > 0) optimizer.LM_params.max_iterations = iterations;
    if (threads > 0)    optimizer.LM_params.numthreads = threads;
    if (lambda0 > 0)    optimizer.LM_params.lambda0 = lambda0;
    if (fd_step > 0)    optimizer.LM_params.fd_step = fd_step;
    if (central >= 0)       optimizer.LM_params.central_differences = (central == 1);
    if (profile_sigma >= 0) optimizer.LM_params.profile_sigma = (profile_sigma == 1);

    std::cout << "\nMax iterations : " << optimizer.LM_params.max_iterations << std::endl;
    std::cout << "Parameters     : " << system.ParametersCount() << std::endl;
    std::cout << "Differences    : "
              << (optimizer.LM_params.central_differences ? "central" : "forward")
              << ", step " << optimizer.LM_params.fd_step << " of range" << std::endl;
    std::cout << "Profiled sigma : "
              << (optimizer.LM_params.profile_sigma ? "yes" : "no") << std::endl;

    std::cout << "\nStarting parameter values (LM is local; these are where it starts):" << std::endl;
    for (int i = 0; i < (int)system.ParametersCount(); i++)
    {
        Parameter *p = system.GetParameter(i);
        if (p) std::cout << "  " << p->GetName() << " = " << p->GetValue() << std::endl;
    }

    system.SetSilent(true);
    std::cout << "\nRunning Levenberg-Marquardt ..." << std::endl;
    const int iters = optimizer.optimize();
    system.SetSilent(false);

    if (iters < 0)
    {
        std::cout << "\nLM did not run: " << optimizer.last_error << std::endl;
        return 1;
    }

    std::cout << "\nLM finished after " << iters << " iteration(s). Writing outputs ..." << std::endl;
    optimizer.Model_out.GetOutputs().write(workingfolder + "outputs.txt");
    optimizer.Model_out.GetObservedOutputs().write(workingfolder + "observedoutputs.txt");
    optimizer.Model_out.errorhandler.Write(workingfolder + "errors.txt");

    system.TransferResultsFrom(&optimizer.Model_out);
    system.Parameters() = optimizer.Model_out.Parameters();
    system.SetOutputItems();

    std::cout << "\nEstimates:" << std::endl;
    for (int i = 0; i < (int)system.ParametersCount(); i++)
    {
        Parameter *p = system.GetParameter(i);
        if (p) std::cout << "  " << p->GetName() << " = " << p->GetValue() << std::endl;
    }
    if (optimizer.covariance_valid)
        std::cout << "\nStandard errors and the parameter correlation matrix are in "
                  << workingfolder << optimizer.filenames.covariancefilename << std::endl;
    else
        std::cout << "\nNo covariance was produced: see the log above." << std::endl;

    system.ApplyParameters();
    ohq::WriteForwardRunOutputs(system, workingfolder);

    std::cout << "\nAll done. Outputs are in " << workingfolder << std::endl;
    return 0;
}
