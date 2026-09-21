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
 * OpenHydroQual - headless runner support
 *
 * Output writing shared by the standalone MCMC and GA runners. Everything here
 * mirrors what MainWindow does after a run so that a headless run leaves the
 * same set of files in the working folder as the GUI would.
 */

#pragma once

#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

namespace ohq {

inline std::string EnsureTrailingSlash(std::string p)
{
    if (!p.empty() && p.back() != '/' && p.back() != '\\') p += "/";
    return p;
}

#ifndef OHQ_DEFAULT_RESOURCES
#define OHQ_DEFAULT_RESOURCES "./resources/"
#endif

// Template location. Compiled in from the OHQ path given to qmake; override at
// run time with the OHQ_RESOURCES environment variable. Note that a .ohq file
// carrying absolute `loadtemplate` paths overrides both -- and if that path does
// not resolve, every template silently fails to load and the model comes up empty.
inline std::string ResourcePath()
{
    const char *env = std::getenv("OHQ_RESOURCES");
    return EnsureTrailingSlash(env ? std::string(env) : std::string(OHQ_DEFAULT_RESOURCES));
}

// Load a .ohq script into a System. Returns false and prints why on failure.
inline bool LoadModel(System &system, const std::string &modelfile,
                      const std::string &workingfolder,
                      const std::string &templatepath)
{
    if (!QFileInfo::exists(QString::fromStdString(modelfile)))
    {
        std::cout << "Model file not found: " << modelfile << std::endl;
        return false;
    }
    system.SetDefaultTemplatePath(templatepath);
    system.SetWorkingFolder(workingfolder);

    Script scr(modelfile, &system);
    std::cout << "Executing script ..." << std::endl;
    system.CreateFromScript(scr, templatepath + "settings.json");
    system.SetSilent(false);
    return true;
}

// The GUI refuses to start a parameter-estimation run when either of these
// fails; a headless run should refuse for the same reasons rather than
// producing a meaningless chain.
inline bool VerifyReadyForEstimation(System &system)
{
    ErrorHandler errs = system.VerifyAllQuantities();
    if (system.ParametersCount() == 0)
    {
        std::cout << "No parameters have been defined - nothing to estimate." << std::endl;
        return false;
    }
    if (errs.Count() != 0)
    {
        std::cout << "There are errors in the values assigned to some variables:" << std::endl;
        for (int i = 0; i < (int)errs.Count(); i++)
        {
            const _error *e = errs[i];
            if (!e) continue;
            std::cout << "  [" << e->code << "] " << e->cls << " '" << e->objectname
                      << "' in " << e->funct << ": " << e->description << std::endl;
        }
        return false;
    }
    return true;
}

/*
 * Run the model forward once and write every per-run output the GUI writes:
 *   output.txt / observedoutput.txt   (names taken from the model settings)
 *   fit_measures.txt
 *   mapped_modeled_results.txt
 *   objective_function_values.txt
 *   errors.txt
 *   state.json
 * `sys` is expected to already carry the parameter values you want evaluated.
 */
inline void WriteForwardRunOutputs(System &sys, const std::string &folder,
                                   const std::string &jsonname = "state.json")
{
    std::cout << "Running the model forward at the selected parameter set ..." << std::endl;
    sys.SetSystemSettings();
    if (sys.GetSolverSettings().write_solution_details)
        sys.SetSolutionLogger(folder + "solution_details.txt");

    sys.SetParameterEstimationMode();     // forward mode: no misfit shortcut
    sys.WriteOutPuts();
    bool ok = sys.Solve(true);
    if (!ok || sys.GetSolutionFailed())
        std::cout << "WARNING: the forward run did not complete successfully. "
                     "The outputs below describe an incomplete solution." << std::endl;

    if (sys.OutputFileName() != "")
    {
        std::string f = sys.OutputFileName();
        if (f.find('/') == std::string::npos && f.find('\\') == std::string::npos) f = folder + f;
        if (sys.WriteIntermittently()) sys.GetOutputs().appendtofile(f, true);
        else                           sys.GetOutputs().write(f);
        std::cout << "  wrote " << f << std::endl;
    }
    if (sys.ObservedOutputFileName() != "")
    {
        std::string f = sys.ObservedOutputFileName();
        if (f.find('/') == std::string::npos && f.find('\\') == std::string::npos) f = folder + f;
        if (sys.WriteIntermittently()) sys.GetObservedOutputs().appendtofile(f, true);
        else                           sys.GetObservedOutputs().write(f);
        std::cout << "  wrote " << f << std::endl;
    }

    sys.errorhandler.Write(folder + "errors.txt");

    sys.SetVal("tstart", sys.GetTime());
    sys.SetProp("tstart", sys.GetTime());
    sys.SetSystemSettingsObjectProperties("simulation_start_time",
                                          aquiutils::numbertostring(sys.GetTime()));
    std::vector<std::string> notemplates;
    sys.SavetoJson(folder + jsonname, notemplates, false, false);
    std::cout << "  wrote " << folder + jsonname << std::endl;

    // Fit measures and observation mapping, exactly as MainWindow computes them.
    sys.ObjectiveFunctionSet()->Calculate();
    CVector ObjectiveFunctionValues = sys.ObjectiveFunctionSet()->Objective_Values();
    ObjectiveFunctionValues.writetofile(folder + "objective_function_values.txt");

    TimeSeriesSet<double> mapped_modeled_results;
    std::ofstream fitfile(folder + "fit_measures.txt");
    fitfile << "Observation\tMSE\tR2\tNSE\n";
    for (unsigned int i = 0; i < sys.ObservationsCount(); i++)
    {
        sys.observation(i)->CalcMisfit();
        fitfile << sys.observation(i)->GetName();
        if (sys.observation(i)->fit_measures.size() == 3)
            for (unsigned int j = 0; j < 3; j++)
                fitfile << "\t" << sys.observation(i)->fit_measures[j];
        else
            fitfile << "\t\t\t";
        fitfile << "\n";

        if (sys.observation(i)->GetModeledTimeSeries() != nullptr &&
            sys.observation(i)->Variable("observed_data")->GetTimeSeries() != nullptr)
            mapped_modeled_results.append(
                sys.observation(i)->MappedModeledSeries(),
                sys.observation(i)->GetName());
    }
    fitfile.close();
    mapped_modeled_results.write(folder + "mapped_modeled_results.txt");
    std::cout << "  wrote fit_measures.txt, mapped_modeled_results.txt, "
                 "objective_function_values.txt, errors.txt" << std::endl;
}

} // namespace ohq
