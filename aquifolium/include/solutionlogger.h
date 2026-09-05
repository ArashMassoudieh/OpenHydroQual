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

#ifndef SOLUTIONLOGGER_H
#define SOLUTIONLOGGER_H
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "Vector.h"
#include "Matrix.h"
#include "Matrix_arma.h"
#include "Vector_arma.h"

using namespace std;

/*
 * Diagnostic log for a solver run.
 *
 * Two things are recorded:
 *
 *   1. A chronological, column-aligned event stream - one line per solver
 *      event, with the simulation time, the time step in force, the event
 *      category, and where in the model it happened.
 *
 *   2. Running tallies of those events, by category and by location, which are
 *      printed as a ranked summary at the end of the run (WriteSummary()).
 *
 * The summary is the part that answers "why is my time step collapsing?" - it
 * names the block responsible and how often, instead of leaving the user to
 * read several thousand lines of event stream and count by hand.
 */
class SolutionLogger
{
public:
    SolutionLogger();
    SolutionLogger(const string &filename);
    ~SolutionLogger();

    bool AssignFile(const string &filename);
    bool Close();
    void Flush();

    // ---- original API, retained so existing call sites keep working --------
    bool WriteString(const string &s);
    bool WriteVector(const CVector &v);
    bool WriteVector(const CVector_arma &v);
    bool WriteMatrix(const CMatrix &v);
    bool WriteMatrix(const CMatrix_arma &v);

    // ---- structured output -------------------------------------------------

    /// Run banner: model name, output file, start time, solver settings.
    void WriteHeader(const string &modelname, const vector<pair<string,string>> &settings);

    /// A titled section rule in the event stream.
    void Section(const string &title);

    /// One event row: "  t=<time>  dt=<step>  | CATEGORY      | location | detail"
    /// Also tallies (category) and (category, location) for the summary.
    void Event(double t, double dt, const string &category,
               const string &location = "", const string &detail = "");

    /// Aligned "key : value" line.
    void KeyValue(const string &key, const string &value, int indent = 2);

    /// The `topn` largest-magnitude entries of a residual/state vector, named.
    /// Far more useful than dumping the whole vector, which for a 72-block,
    /// 6-constituent model is 432 unlabelled numbers.
    void TopEntries(const string &title, const CVector_arma &v,
                    const vector<string> &labels, unsigned int topn = 5);

    /// Record a time-step change and what triggered it.
    void TimeStepChange(double t, double dt_old, double dt_new,
                        const string &cause, const string &location = "");

    /// Count an event without emitting a line (for very high-frequency events).
    void Tally(const string &category, const string &location = "");

    /// Ranked summary of everything tallied. Call once at the end of a run.
    void WriteSummary(double t_reached, double t_end, bool solution_failed);

    void SetAutoFlushInterval(size_t n) { autoflush_every = n; }
    void SetEnabled(bool e) { enabled = e; }
    bool IsEnabled() const { return enabled; }

private:
    void Push(const string &s);

    ofstream file;
    vector<string> contents;
    bool enabled = true;
    size_t autoflush_every = 200;   // keep the log useful if the run is killed
    size_t since_flush = 0;

    // tallies
    map<string, long>                  category_counts;
    map<string, map<string, long>>     category_location_counts;
    long   timestep_reductions = 0;
    long   timestep_increases  = 0;
    double smallest_dt = 0.0;
    double smallest_dt_time = 0.0;
    string smallest_dt_cause;
    long   event_lines = 0;
};

#endif // SOLUTIONLOGGER_H
