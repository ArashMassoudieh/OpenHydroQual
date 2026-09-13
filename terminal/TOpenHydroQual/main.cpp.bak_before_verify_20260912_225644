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
#include <QCoreApplication>
#include <QDir>
#include "qfileinfo.h"
#include <cstring>

// Exit codes. A command-line tool that always returns 0 cannot be used in a
// pipeline: the caller has no way to tell a converged run from a model that
// silently loaded no inflow. These are the states a script needs to branch on.
enum ExitCode {
    EXIT_OK          = 0,
    EXIT_USAGE       = 2,   // no input file, or --help
    EXIT_NO_INPUT    = 3,   // the .ohq does not exist or is unreadable
    EXIT_MODEL_ERROR = 4,   // model built, but the error handler logged problems
    EXIT_SOLVE_FAIL  = 5    // Solve() returned false
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
         << "  -h, --help               this message\n\n"
         << "Exit codes: 0 ok, 2 usage, 3 input not found, 4 model errors,\n"
         << "            5 solve failed.\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    string input, outfile, workdir, resources;
    bool quiet = false;
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

    System *system = new System();
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

    // The solver settings parsed from the .ohq (initial_time_step, tolerances,
    // write_interval, jacobian method) only reach the solver through this call.
    // Without it they stay at their default-constructed values and Solve()
    // aborts in Armadillo while sizing its work vectors.
    system->SetSystemSettings();

    system->SetSilent(quiet);

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

    const int nerr_after = system->GetErrorHandler()->Count();
    if (nerr_after > 0) {
        cout << "\n*** " << nerr_after << " error(s) during the solve:" << endl;
        system->GetErrorHandler()->Flush();
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
        system->GetObservedOutputs().write(obs);
    }

    const string outname = outfile.empty() ? system->OutputFileName() : outfile;
    if (outname.empty()) {
        cout << "No output file name given; bulk outputs not written." << endl;
    } else if (!system->RecordResults()) {
        cout << "record_results=No; bulk outputs not recorded." << endl;
    } else {
        const string out = system->GetWorkingFolder() + outname;
        cout << "Writing outputs in '" << out << "'" << endl;
        system->GetOutputs().write(out);
    }
    delete system;
    return nerr_after > 0 ? EXIT_MODEL_ERROR : EXIT_OK;
}
