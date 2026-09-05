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

#include "solutionlogger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <ctime>

namespace {

string fmt(double v, int prec = 6)
{
    ostringstream ss;
    ss << setprecision(prec) << v;
    return ss.str();
}

string pad(const string &s, size_t w)
{
    if (s.size() >= w) return s;
    return s + string(w - s.size(), ' ');
}

string rule(char c = '-', size_t n = 78) { return string(n, c); }

} // namespace

SolutionLogger::SolutionLogger() {}

SolutionLogger::SolutionLogger(const string &filename) { AssignFile(filename); }

SolutionLogger::~SolutionLogger() { Flush(); Close(); }

bool SolutionLogger::AssignFile(const string &filename)
{
    if (file.is_open()) file.close();
    file.open(filename, std::ofstream::out);
    return file.good();
}

bool SolutionLogger::Close()
{
    if (file.is_open()) file.close();
    return true;
}

void SolutionLogger::Push(const string &s)
{
    if (!enabled) return;
    contents.push_back(s);
    if (++since_flush >= autoflush_every) Flush();
}

// ---- original API ---------------------------------------------------------

bool SolutionLogger::WriteString(const string &s) { Push(s); return true; }

bool SolutionLogger::WriteVector(const CVector &v)      { Push(v.toString()); return true; }
bool SolutionLogger::WriteVector(const CVector_arma &v) { Push(v.toString()); return true; }

bool SolutionLogger::WriteMatrix(const CMatrix &m)
{
    for (const auto &l : m.toString()) Push(l);
    return true;
}
bool SolutionLogger::WriteMatrix(const CMatrix_arma &m)
{
    for (const auto &l : m.toString()) Push(l);
    return true;
}

// ---- structured output ----------------------------------------------------

void SolutionLogger::WriteHeader(const string &modelname,
                                 const vector<pair<string,string>> &settings)
{
    time_t now = time(nullptr);
    char stamp[64];
    strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    Push(rule('='));
    Push("OpenHydroQual solution log");
    Push(rule('='));
    Push("  model   : " + modelname);
    Push("  started : " + string(stamp));
    if (!settings.empty())
    {
        Push("");
        Push("  solver settings");
        size_t w = 0;
        for (const auto &kv : settings) w = max(w, kv.first.size());
        for (const auto &kv : settings)
            Push("    " + pad(kv.first, w) + " : " + kv.second);
    }
    Push("");
    Push(rule('='));
    Push("EVENT STREAM");
    Push("  columns: time | dt | category | location | detail");
    Push(rule('='));
    Flush();
}

void SolutionLogger::Section(const string &title)
{
    Push("");
    Push(rule('-'));
    Push(title);
    Push(rule('-'));
}

void SolutionLogger::Event(double t, double dt, const string &category,
                           const string &location, const string &detail)
{
    Tally(category, location);
    ostringstream ss;
    ss << "  t=" << pad(fmt(t, 8), 12)
       << "dt=" << pad(fmt(dt, 4), 12)
       << "| " << pad(category, 26)
       << "| " << pad(location, 20);
    if (!detail.empty()) ss << "| " << detail;
    Push(ss.str());
    event_lines++;
}

void SolutionLogger::KeyValue(const string &key, const string &value, int indent)
{
    Push(string(indent, ' ') + pad(key, 34) + " : " + value);
}

void SolutionLogger::TopEntries(const string &title, const CVector_arma &v,
                                const vector<string> &labels, unsigned int topn)
{
    vector<pair<double,size_t>> idx;
    for (size_t i = 0; i < static_cast<size_t>(v.getsize()); ++i)
    {
        double val = v[i];
        if (std::isnan(val)) val = 1e300;      // surface NaNs at the top
        idx.push_back({fabs(val), i});
    }
    sort(idx.begin(), idx.end(),
         [](const pair<double,size_t>&a, const pair<double,size_t>&b){ return a.first > b.first; });

    Push("    " + title + "  (" + to_string(min<size_t>(topn, idx.size()))
         + " largest of " + to_string(idx.size()) + ")");
    for (unsigned int k = 0; k < topn && k < idx.size(); ++k)
    {
        size_t i = idx[k].second;
        string lbl = (i < labels.size() ? labels[i] : ("index " + to_string(i)));
        Push("      " + pad(lbl, 34) + " = " + fmt(v[i], 6));
    }
}

void SolutionLogger::TimeStepChange(double t, double dt_old, double dt_new,
                                    const string &cause, const string &location)
{
    if (dt_new < dt_old) timestep_reductions++; else timestep_increases++;

    if (smallest_dt == 0.0 || dt_new < smallest_dt)
    {
        smallest_dt       = dt_new;
        smallest_dt_time  = t;
        smallest_dt_cause = cause + (location.empty() ? "" : " @ " + location);
    }
    Event(t, dt_new,
          dt_new < dt_old ? "timestep reduced" : "timestep increased",
          location,
          "from " + fmt(dt_old, 4) + "  cause: " + cause);
}

void SolutionLogger::Tally(const string &category, const string &location)
{
    if (!enabled) return;
    category_counts[category]++;
    if (!location.empty()) category_location_counts[category][location]++;
}

void SolutionLogger::WriteSummary(double t_reached, double t_end, bool solution_failed)
{
    Push("");
    Push(rule('='));
    Push("SUMMARY");
    Push(rule('='));

    KeyValue("simulation reached t", fmt(t_reached, 8));
    KeyValue("requested end time",   fmt(t_end, 8));
    KeyValue("completed",            (t_reached >= t_end ? "yes" : "NO - stopped early"));
    KeyValue("solution failed flag", (solution_failed ? "TRUE" : "false"));
    KeyValue("diagnostic events logged", to_string(event_lines));
    KeyValue("time step reductions", to_string(timestep_reductions));
    KeyValue("time step increases",  to_string(timestep_increases));
    if (smallest_dt > 0.0)
    {
        KeyValue("smallest time step", fmt(smallest_dt, 4)
                 + "  at t = " + fmt(smallest_dt_time, 8));
        KeyValue("  triggered by", smallest_dt_cause);
    }

    if (!category_counts.empty())
    {
        Push("");
        Push("  events by category (most frequent first)");
        vector<pair<string,long>> cats(category_counts.begin(), category_counts.end());
        sort(cats.begin(), cats.end(),
             [](const pair<string,long>&a, const pair<string,long>&b){ return a.second > b.second; });
        long total = 0; for (auto &c : cats) total += c.second;
        for (const auto &c : cats)
        {
            double pct = total ? 100.0 * double(c.second) / double(total) : 0.0;
            ostringstream ss;
            ss << "    " << pad(c.first, 34) << pad(to_string(c.second), 10)
               << "(" << fixed << setprecision(1) << pct << "%)";
            Push(ss.str());
        }

        Push("");
        Push("  where those events happened  <- the block to look at first");
        for (const auto &c : cats)
        {
            auto it = category_location_counts.find(c.first);
            if (it == category_location_counts.end() || it->second.empty()) continue;
            vector<pair<string,long>> locs(it->second.begin(), it->second.end());
            sort(locs.begin(), locs.end(),
                 [](const pair<string,long>&a, const pair<string,long>&b){ return a.second > b.second; });
            Push("    " + c.first + ":");
            for (size_t k = 0; k < locs.size() && k < 5; ++k)
                Push("      " + pad(locs[k].first, 34) + to_string(locs[k].second));
            if (locs.size() > 5)
                Push("      ... and " + to_string(locs.size() - 5) + " more location(s)");
        }
    }
    else
    {
        Push("");
        Push("  No solver difficulties were recorded.");
    }

    Push(rule('='));
    Flush();
}

void SolutionLogger::Flush()
{
    if (!file.is_open()) { contents.clear(); since_flush = 0; return; }
    for (const auto &line : contents) file << line << std::endl;
    file.flush();
    contents.clear();
    since_flush = 0;
}
