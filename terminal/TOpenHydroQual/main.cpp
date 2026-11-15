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

#include "System.h"
#include "Script.h"
#include "ohq_kernel.h"   // KernelSystem: run the forward model with a codegen-generated library
#include <QCoreApplication>
#include <QDir>
#include "qfileinfo.h"
#include <cstring>
#include <fstream>

// Exit codes. A command-line tool that always returns 0 cannot be used in a
// pipeline: the caller has no way to tell a converged run from a model that
// silently loaded no inflow. These are the states a script needs to branch on.
enum ExitCode {
    EXIT_OK          = 0,
    EXIT_USAGE       = 2,   // no input file, or --help
    EXIT_NO_INPUT    = 3,   // the .ohq does not exist or is unreadable
    EXIT_MODEL_ERROR = 4,   // the model could not be built as written (problems logged
                            // while executing the script), or a property violates its
                            // declared criteria. Messages logged DURING the solve are
                            // warnings: the run continues and exits 0.
    EXIT_SOLVE_FAIL  = 5,   // Solve() returned false
    EXIT_KERNEL      = 6    // --kernel: the library could not be loaded, or it was not
                            // generated from this model (parameters/observations differ)
};

static void usage(const char *prog)
{
    cout << "OpenHydroQual console\n\n"
         << "  " << prog << " <model.ohq> [options]\n\n"
         << "Options:\n"
         << "  -o, --outputfile <name>  override the model's alloutputfile\n"
         << "  -w, --workdir <dir>      working folder for relative paths and\n"
         << "                           outputs (default: the .ohq's folder)\n"
         << "  -r, --resources <dir>    template/resources folder\n"
         << "                           (default: <exe>/../../resources/,\n"
         << "                            or $OHQ_RESOURCES if set)\n"
         << "  -q, --quiet              suppress per-step solver output\n"
         << "      --skip-verify        do not check property criteria before\n"
         << "                           solving (not recommended)\n"
         << "  -k, --kernel <lib>       solve with a codegen-generated shared library\n"
         << "                           (ohq_generate ... --project shared) in place of\n"
         << "                           the interpreter. Observations, objective\n"
         << "                           functions and fit statistics are produced; the\n"
         << "                           bulk output file (alloutputfile) is not.\n"
         << "  -h, --help               this message\n\n"
         << "Exit codes: 0 ok, 2 usage, 3 input not found, 4 model errors,\n"
         << "            5 solve failed, 6 kernel not loadable or not this model's.\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    string input, outfile, workdir, resources, kernel;
    bool quiet = false, skipverify = false;
    for (int i = 1; i < argc; ++i) {
        const char *s = argv[i];
        if (!std::strcmp(s, "-h") || !std::strcmp(s, "--help")) {
            usage(argv[0]); return EXIT_USAGE;
        } else if ((!std::strcmp(s, "-o") || !std::strcmp(s, "--outputfile")) && i + 1 < argc) {
            outfile = argv[++i];
        } else if ((!std::strcmp(s, "-w") || !std::strcmp(s, "--workdir")) && i + 1 < argc) {
            workdir = argv[++i];
        } else if ((!std::strcmp(s, "-r") || !std::strcmp(s, "--resources")) && i + 1 < argc) {
            resources = argv[++i];
        } else if (!std::strcmp(s, "-q") || !std::strcmp(s, "--quiet")) {
            quiet = true;
        } else if (!std::strcmp(s, "--skip-verify")) {
            skipverify = true;
        } else if ((!std::strcmp(s, "-k") || !std::strcmp(s, "--kernel")) && i + 1 < argc) {
            kernel = argv[++i];
        } else if (s[0] != '-' && input.empty()) {
            input = s;                       // the original argv[1]=="" test was a
                                             // pointer comparison and never fired
        } else {
            cout << "Unrecognised argument: " << s << "\n\n";
            usage(argv[0]); return EXIT_USAGE;
        }
    }
    if (input.empty()) { usage(argv[0]); return EXIT_USAGE; }

    QFileInfo fi(QString::fromStdString(input));
    if (!fi.exists() || !fi.isReadable()) {
        cout << "Input file not found or not readable: " << input << endl;
        return EXIT_NO_INPUT;
    }

    // Resources. The hard-coded <exe>/../../resources/ only resolves inside the
    // build tree, so an installed or relocated binary silently fails to load its
    // templates. Precedence: --resources, then $OHQ_RESOURCES, then the old path.
    if (resources.empty()) {
        if (const char *env = std::getenv("OHQ_RESOURCES")) resources = env;
    }
    if (resources.empty())
        resources = qApp->applicationDirPath().toStdString() + "/../../resources/";
    if (resources.back() != '/') resources += '/';
    if (!QFileInfo::exists(QString::fromStdString(resources))) {
        cout << "Resources folder not found: " << resources << "\n"
             << "Set --resources or $OHQ_RESOURCES." << endl;
        return EXIT_NO_INPUT;
    }

    cout << "Input file: " << input << endl;
    cout << "Resources:  " << resources << endl;

    // With --kernel the model is still read and checked by the interpreter (it
    // owns the parameters, observations and their data); only Solve() is replaced.
    // KernelSystem overrides the virtual System::Solve, as in OHQ-LM/GA/MCMC.
    if (!kernel.empty() && !QFileInfo::exists(QString::fromStdString(kernel))) {
        cout << "Kernel library not found: " << kernel << endl;
        return EXIT_KERNEL;
    }
    System *system = kernel.empty() ? new System() : new ohq::KernelSystem();
    system->SetDefaultTemplatePath(resources);
    system->SetWorkingFolder(workdir.empty()
                             ? fi.canonicalPath().toStdString() + "/"
                             : (workdir.back() == '/' ? workdir : workdir + "/"));

    cout << "Reading script ..." << endl;
    Script scr(input, system);
    cout << "Executing script ..." << endl;
    system->CreateFromScript(scr, resources + "settings.json");

    // Errors raised while BUILDING the model are the dangerous ones: a missing
    // inflow file leaves the model runnable, and it then produces a complete and
    // entirely plausible output file with no inflow in it. Fail here, loudly,
    // rather than hand the caller a wrong answer with a zero exit code.
    int nerr = system->GetErrorHandler()->Count();
    if (nerr > 0) {
        cout << "\n*** " << nerr << " error(s) while building the model:" << endl;
        system->GetErrorHandler()->Flush();
        cout << "*** refusing to solve. Fix the above, or the results are not"
                " the model you wrote." << endl;
        delete system;
        return EXIT_MODEL_ERROR;
    }

    // Check every property against the "criteria" declared for it in the
    // template JSON -- length>0, side_slope>0, ManningCoeff>0 and the rest.
    // The GUI does this before every run (MainWindow::onrunmodel); the console
    // did not, and the two are not equivalent: `create block` assigns each
    // property with check_criteria=false (Command.cpp), deliberately, because
    // properties arrive one at a time and a criterion referring to a sibling
    // would fire on a half-built object. The whole-model sweep afterwards is
    // what makes that safe, and it was missing here. A Trapezoidal Channel
    // Segment with length = -4.75 m therefore loaded and solved: depth came out
    // negative for positive storage, the segment became a sink with no outlet,
    // and it absorbed 94% of the inflow over a 3.8-hour run that reported
    // success. Verify before the run, not after the results are believed.
    if (!skipverify) {
        ErrorHandler verrs = system->VerifyAllQuantities();
        if (verrs.Count() > 0) {
            cout << "\n*** " << verrs.Count()
                 << " property value(s) violate the criteria declared in the"
                    " templates:" << endl;
            for (int i = 0; i < verrs.Count(); ++i)
                cout << "  " << verrs[i]->description << endl;
            cout << "*** refusing to solve. Fix the above, or pass --skip-verify"
                    " to run anyway." << endl;
            delete system;
            return EXIT_MODEL_ERROR;
        }
    }

    // The solver settings parsed from the .ohq (initial_time_step, tolerances,
    // write_interval, jacobian method) only reach the solver through this call.
    // Without it they stay at their default-constructed values and Solve()
    // aborts in Armadillo while sizing its work vectors.
    system->SetSystemSettings();

    system->SetSilent(quiet);

    if (!kernel.empty()) {
        cout << "Forward model: generated kernel " << kernel << endl;
        if (!ohq::LoadKernel(kernel) || !ohq::VerifyKernelMatches(*system)) {
            cout << "*** the kernel cannot be used with this model." << endl;
            delete system;
            return EXIT_KERNEL;
        }
    } else if (!quiet) {
        cout << "Forward model: interpreter" << endl;
    }

    // Honour write_solution_details from the model file, as the GUI does.
    // Without this the setting is silently ignored in terminal runs and no
    // diagnostic log is produced.
    if (system->GetSolverSettings().write_solution_details)
    {
        const string logfile = system->GetWorkingFolder() + "solution_details.txt";
        system->SetSolutionLogger(logfile);
        if (!quiet) cout << "Solution details: " << logfile << endl;
    }
    // A model carrying calibrated parameters keeps the fitted values in the
    // Parameter objects, not in the blocks and constituents themselves; the
    // objects still hold whatever the builder wrote. Solve() only pushes them
    // across when asked, so a forward run of a calibrated model would otherwise
    // silently use the construction-time values (for the column models, a
    // dispersivity of 1e-4 instead of the fitted 5e-2). Apply them whenever the
    // model defines any.
    const size_t npar = system->Parameters().size();
    if (npar > 0)
        cout << "Applying " << npar << " calibrated parameter(s)" << endl;
    cout << "Solving ..." << endl;
    const bool ok = system->Solve(npar > 0);

    // Anything logged while solving (for example an inflow term a block type does
    // not define, which evaluates to 0) is reported as a warning. The solve itself
    // decides success: a failed solve exits EXIT_SOLVE_FAIL below.
    const int nwarn = system->GetErrorHandler()->Count();
    if (nwarn > 0) {
        cout << "\n*** " << nwarn << " warning(s) during the solve (the run continued):" << endl;
        system->GetErrorHandler()->Flush("Warning: ");
    }
    if (!ok) {
        cout << "*** Solve() failed." << endl;
        delete system;
        return EXIT_SOLVE_FAIL;
    }

    // Match System::WriteOutPuts(): an empty file name means "do not write this
    // file". Observations are written independently of the bulk output, so a model
    // can emit only the quantities it declared as Observations.
    if (!system->ObservedOutputFileName().empty()) {
        const string obs = system->GetWorkingFolder() + system->ObservedOutputFileName();
        cout << "Writing observed outputs in '" << obs << "'" << endl;
        if (kernel.empty()) {
            system->GetObservedOutputs().write(obs);
        } else {
            // The kernel hands its series to the observations themselves
            // (already resampled onto the dt0 grid); collect them in model order.
            TimeSeriesSet<timeseriesprecision> obsout;
            for (unsigned int i = 0; i < system->ObservationsCount(); i++) {
                TimeSeries<timeseriesprecision> ts = *system->observation(i)->GetModeledTimeSeries();
                ts.setName(system->observation(i)->GetName());
                obsout.push_back(ts);
            }
            obsout.write(obs);
        }
    }

    // Score every observation that has observed data, as the GUI does after a
    // run, so a forward run reports how well it fits. EMC observations also get
    // a per-event table: the only place their modeled EMCs are visible, since the
    // bulk output holds the observation's continuous series, not the EMCs.
    for (unsigned int i = 0; i < system->ObservationsCount(); i++) {
        Observation *ob = system->observation(i);
        TimeSeries<timeseriesprecision> *od = ob->Variable("observed_data")->GetTimeSeries();
        if (od == nullptr || od->size() == 0) continue;
        const double nll = ob->CalcMisfit();
        cout << "Observation '" << ob->GetName() << "' ("
             << ob->Variable("comparison_method")->GetProperty() << "): ";
        if (ob->fit_measures.size() == 3)
            cout << "MSE=" << ob->fit_measures[0] << " R2=" << ob->fit_measures[1]
                 << " NSE=" << ob->fit_measures[2] << " -logL=" << nll << endl;
        else
            cout << "no fit measures" << endl;
        if (!ob->GetLastError().empty())
            cout << "  warning: " << ob->GetLastError() << endl;
        if (ob->IsEMC()) {
            const string emcfile = system->GetWorkingFolder() + ob->GetName() + "_EMC.txt";
            std::ofstream f(emcfile);
            f << "t, observed_EMC, modeled_EMC" << endl;
            for (size_t k = 0; k < ob->GetObservedEMC()->size(); k++)
                f << ob->GetObservedEMC()->getTime(k) << ", " << ob->GetObservedEMC()->getValue(k)
                  << ", " << ob->GetModeledEMC()->getValue(k) << endl;
            cout << "  " << ob->GetObservedEMC()->size() << " event(s) written to '" << emcfile << "'" << endl;
        }
    }

    const string outname = outfile.empty() ? system->OutputFileName() : outfile;
    if (!kernel.empty()) {
        cout << "Kernel run: the bulk output file is not produced (only observations "
                "and objective functions are returned by the kernel)." << endl;
    } else if (outname.empty()) {
        cout << "No output file name given; bulk outputs not written." << endl;
    } else if (!system->RecordResults()) {
        cout << "record_results=No; bulk outputs not recorded." << endl;
    } else {
        const string out = system->GetWorkingFolder() + outname;
        cout << "Writing outputs in '" << out << "'" << endl;
        system->GetOutputs().write(out);
    }
    delete system;
    return EXIT_OK;
}
