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
 * OpenHydroQual - headless runner support: generated-kernel back end (--kernel)
 *
 * Runs the forward model with a shared library produced by the codegen
 * (`ohq_generate <model.ohq> <resources> <out> <Class> Storage --project shared`)
 * instead of the interpreter, while everything else -- parameter bookkeeping,
 * the objective function, the GA/MCMC drivers and all output writing -- stays
 * with System. Only the solve is replaced.
 *
 * How it hooks in
 *   CGA<T> / CMCMC<T> are templates that call, per individual,
 *       SetParameterValue(i, v); ApplyParameters(); Solve(); GetObjectiveFunctionValue();
 *   KernelSystem derives from System and SHADOWS Solve(). Template dispatch is
 *   static, so instantiating CGA<KernelSystem> picks up this Solve() while every
 *   other member is System's. Nothing in the library changes.
 *
 * What Solve() does
 *   push the parameter values into the kernel (same index order -- verified by
 *   name at load), run it, uniformize its observation series exactly as
 *   System::FinalizeOutputs does, and copy them into the System's observations.
 *   GetObjectiveFunctionValue() then scores the identical quantity it would have
 *   scored after an interpreter solve.
 *
 * Parameters the kernel does not own (an observation's error_standard_deviation,
 * for instance) are applied by System::ApplyParameters as usual, which the GA
 * calls before Solve(); pushing them to the kernel as well is harmless because
 * the kernel simply stores values it never reads.
 */

#pragma once

#include "System.h"
#include "codegen/tools/ohq_kernel.h"   // the ABI: declarations + loader + self-test
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace ohq {

// ---- the ABI ---------------------------------------------------------------
// Declared once, in codegen/tools/ohq_kernel.h, next to the generator that
// emits it (that header also carries the dlopen loader and the parameter
// self-test, and needs no OHQ core -- so a host that does not link System, such
// as the twin's DTRunner, can use it on its own). This file adds only what
// needs System: KernelSystem, which swaps the solve into GA/MCMC.
using KernelABI = Kernel;

// One process-wide kernel library; every KernelSystem copy makes its own handle
// so parallel GA individuals never share solver state.
inline Kernel& TheKernel()
{
    static Kernel k;
    return k;
}

inline bool LoadKernel(const std::string& path)
{
    if (!TheKernel().load(path))
    {
        std::cout << TheKernel().error() << std::endl;
        std::cout << "Generate the library with\n"
                  << "  ohq_generate <model.ohq> <resources> <out> <Class> Storage --project shared"
                  << std::endl;
        return false;
    }
    return true;
}

// The kernel is compiled from one .ohq; the runner is given the same file. If
// the two ever drift apart the objective would be computed on mismatched
// quantities, silently. Refuse rather than produce a plausible wrong answer.
inline bool VerifyKernelMatches(System& system)
{
    KernelABI& k = TheKernel();
    std::cout << "Kernel class    : " << (k.class_name ? k.class_name() : "?") << std::endl;

    const int np = k.n_parameters();
    if (np != (int)system.ParametersCount())
    {
        std::cout << "Kernel has " << np << " parameters, the model has "
                  << system.ParametersCount() << ". Regenerate the kernel from this model."
                  << std::endl;
        return false;
    }
    for (int i = 0; i < np; i++)
    {
        Parameter* p = system.GetParameter(i);
        const std::string kn = k.parameter_name(i), mn = p ? p->GetName() : std::string();
        if (kn != mn)
        {
            std::cout << "Parameter " << i << " is '" << kn << "' in the kernel but '"
                      << mn << "' in the model." << std::endl;
            return false;
        }
    }
    const int no = k.n_observations();
    if (no != (int)system.ObservationsCount())
    {
        std::cout << "Kernel has " << no << " observations, the model has "
                  << system.ObservationsCount() << "." << std::endl;
        return false;
    }
    for (int i = 0; i < no; i++)
    {
        const std::string kn = k.observation_name(i);
        const std::string mn = system.observation(i)->GetName();
        if (kn != mn)
        {
            std::cout << "Observation " << i << " is '" << kn << "' in the kernel but '"
                      << mn << "' in the model." << std::endl;
            return false;
        }
    }
    // Names and counts matching is NOT enough (issues.md ISSUE 17): a kernel can
    // advertise a parameter it silently ignores, and a GA then "converges" on an
    // optimum that does not reproduce. Perturb each one and confirm the model
    // actually moves. Parameters the kernel legitimately does not own (an
    // observation's error_standard_deviation -- applied by System::ApplyParameters
    // instead, see G9) are skipped by name.
    std::vector<std::string> host_owned;
    for (int i = 0; i < np; i++)
    {
        Parameter* p = system.GetParameter(i);
        if (!p) continue;
        for (unsigned j = 0; j < p->GetLocations().size(); j++)
        {
            Object* o = system.object(p->GetLocations()[j]);
            if (o && (o->ObjectType() == object_type::observation
                   || o->ObjectType() == object_type::objective_function))
            { host_owned.push_back(p->GetName()); break; }
        }
    }
    // Probe each parameter across ITS OWN prior range rather than by a blanket
    // relative step. self_test's default +5% is the wrong probe for a parameter
    // whose prior spans orders of magnitude and whose current value sits at the
    // bottom of it: a control threshold at its 1e-4 lower bound with a 1e-4..0.03
    // log-normal prior moves nothing at 1.05e-4 -- the forecast indicator is
    // either 0 or already far above both -- yet it plainly moves the model at
    // 1e-3. Perturbing to the far side of the range asks the question the GA
    // will actually ask. (Still a BITWISE check, per ISSUE 17.)
    {
        const double t0 = k.simulation_start();
        const double tend = t0 + 0.05 * (k.simulation_end() - t0);
        const int ns = k.n_states(), nm = k.n_mass();
        auto snap = [&](void* h, std::vector<double>& o) {
            o.clear();
            o.reserve(ns + nm + no * 2);
            for (int i = 0; i < ns; ++i) o.push_back(k.state(h, i));
            for (int i = 0; i < nm; ++i) o.push_back(k.mass(h, i));
            // Some estimated parameters affect only a reported observation
            // (for example an EC(theta) mapping) and correctly leave every
            // hydraulic/transport state unchanged. Include the observation
            // series so those live bindings are not rejected as inert.
            for (int i = 0; i < no; ++i)
            {
                const int n = k.observation_count(h, i);
                o.push_back((double)n);
                for (int j = 0; j < n; ++j)
                {
                    double t = 0, v = 0;
                    if (k.observation_at(h, i, j, &t, &v)) o.push_back(v);
                }
            }
        };
        std::vector<double> base, cur;
        { void* h = k.create(); k.initialize(h); k.run_to(h, tend); snap(h, base); k.destroy(h); }
        for (int i = 0; i < np; i++)
        {
            const std::string pname = k.parameter_name(i);
            bool skip = false;
            for (size_t j = 0; j < host_owned.size(); j++)
                if (host_owned[j] == pname) { skip = true; break; }
            if (skip) continue;

            void* h = k.create(); k.initialize(h);
            const double v0 = k.parameter(h, i);
            double probe = (v0 == 0.0) ? 0.05 : v0 * 1.05;     // self_test's default
            Parameter* mp = system.GetParameter(i);
            if (mp)
            {
                const double lo = mp->GetVal("low"), hi = mp->GetVal("high");
                if (hi > lo)                                    // far side of the prior
                    probe = (v0 - lo) < 0.5 * (hi - lo) ? lo + 0.75 * (hi - lo)
                                                        : lo + 0.25 * (hi - lo);
            }
            k.set_parameter(h, i, probe); k.apply_parameters(h);
            k.run_to(h, tend); snap(h, cur); k.destroy(h);

            bool moved = cur.size() != base.size();
            for (size_t q = 0; q < cur.size() && !moved; ++q) moved = (cur[q] != base[q]);
            if (!moved)
            {
                std::cout << "Parameter " << i << " '" << pname
                          << "' is advertised by the kernel but does NOT change its output"
                             " (probed " << v0 << " -> " << probe << ")."
                             " Refusing to calibrate against it (issues.md ISSUE 17)." << std::endl;
                return false;
            }
        }
    }
    std::cout << "Kernel verified : " << np << " parameters, " << no
              << " observations, names match the model; all "
              << (np - (int)host_owned.size()) << " kernel-owned parameters move it"
              << (host_owned.empty() ? "" : " (host-owned skipped)") << "." << std::endl;
    return true;
}

// ---------------------------------------------------------------------------
class KernelSystem : public System
{
public:
    KernelSystem() = default;
    KernelSystem(const System& s) : System(s) {}
    KernelSystem(const KernelSystem& o) : System(o) {}          // never copy the handle
    KernelSystem& operator=(const KernelSystem& o)
    {
        if (this != &o) { release(); System::operator=(o); }
        return *this;
    }
    KernelSystem& operator=(const System& o) { release(); System::operator=(o); return *this; }
    ~KernelSystem() { release(); }

    // Overrides System::Solve (virtual since the kernel path was introduced), so
    // helpers holding a System& reach the kernel too rather than silently falling
    // back to the interpreter.
    bool Solve(bool ApplyParams = false, bool uniformize_outputs = true) override
    {
        (void)uniformize_outputs;
        KernelABI& k = TheKernel();
        if (!k.valid()) return System::Solve(ApplyParams, uniformize_outputs);
        // The kernel records only each observation's expression; an EMC
        // observation also needs its weighting (flow) series. It is taken from
        // another observation that records the same weighting expression on the
        // same weighting object (e.g. a flow observation on the outlet link).
        // Without one, fall back to the interpreter, which records it per step.
        const std::vector<int> emc_weight = EMCWeightObservations();
        for (unsigned int i = 0; i < ObservationsCount(); i++)
            if (observation(i)->IsEMC() && emc_weight[i] < 0) return System::Solve(ApplyParams, uniformize_outputs);
        if (ApplyParams) ApplyParameters();
        if (!h_) h_ = k.create();
        if (!h_) return false;

        // System::InitializeSolver sizes this, and System::CalcMisfit writes
        // fit_measures[3i..3i+2] for every observation. The kernel path never
        // calls InitializeSolver, so without this the first misfit evaluation
        // writes past the end of an empty vector.
        if (fit_measures.size() < ObservationsCount() * 3)
            fit_measures.resize(ObservationsCount() * 3);

        for (int i = 0; i < (int)ParametersCount(); i++)
            if (Parameter* p = GetParameter(i)) k.set_parameter(h_, i, p->GetValue());
        k.apply_parameters(h_);

        k.clear_observations(h_);
        k.initialize(h_);

        // Wall-clock budget. System::Solve gives up at maximum_time_allowed and
        // reports a failed solve, so a pathological parameter set costs one
        // rejected proposal. run_to() has no cancellation, so a set that
        // collapses dt to the floor (minimum_timestep against tend can be
        // millions of steps) runs effectively forever -- which is exactly how an
        // MCMC with 16 chains stalled on its very first sample. Step instead, and
        // check the clock.
        const double t_end   = k.simulation_end();
        const double budget  = GetSolverSettings().maximum_simulation_time;
        const auto   started = std::chrono::steady_clock::now();
        bool ok = true, timed_out = false;
        if (budget > 0 && k.step_to) {
            int guard = 0;
            while (k.time(h_) < t_end) {
                if (!k.step_to(h_, t_end)) { ok = false; break; }
                // checking the clock every step would cost more than the step
                if ((++guard & 0x3F) == 0 &&
                    std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count() > budget)
                { ok = false; timed_out = true; break; }
            }
        } else {
            ok = k.run_to(h_, t_end) != 0;
        }
        if (timed_out) ++kernel_timeouts_;
        k.uniformize_observations(h_);        // System::FinalizeOutputs does this

        // Hand the kernel's modeled series to the System's observations; the
        // objective function then scores exactly what it would have after an
        // interpreter solve.
        for (int i = 0; i < (int)ObservationsCount(); i++)
        {
            const int n = k.observation_count(h_, i);
            TimeSeries<timeseriesprecision> ts;
            double t = 0, v = 0;
            for (int s = 0; s < n; s++)
                if (k.observation_at(h_, i, s, &t, &v)) ts.addPoint(t, v);
            observation(i)->SetModeledTimeSeries(ts);
        }
        // EMC observations: weight = the matched observation's series, flux =
        // expression x weight, both on this observation's time grid. The
        // interpreter forms the same two series from per-step values.
        for (int i = 0; i < (int)ObservationsCount(); i++)
        {
            if (!observation(i)->IsEMC()) continue;
            const TimeSeries<timeseriesprecision>& c = *observation(i)->GetModeledTimeSeries();
            const TimeSeries<timeseriesprecision>& w = *observation(emc_weight[i])->GetModeledTimeSeries();
            TimeSeries<timeseriesprecision> flux, weight;
            for (size_t s = 0; s < c.size(); s++)
            {
                const double t = c.getTime(s), wv = w.interpol(t);
                weight.append(t, wv);
                flux.append(t, c.getValue(s) * wv);
            }
            observation(i)->SetEMCSeries(flux, weight);
        }
        // G2b. Objective functions score their OWN stored_time_series, which
        // System::Solve fills via Objective_Function::append_value each step --
        // and this method shadows System::Solve, so without this the series stays
        // empty and every objective comes back exactly 0. (That is what made a
        // kernel-driven GA on an objective-only model report
        // "best objective 0.000000e+00" while the interpreter scored 1.55e+01.)
        // The kernel records the same expression on the same object; hand it over.
        if (k.n_objectives)
        {
            // NB: hand over the RAW per-step series. System::Solve appends raw
            // values with Objective_Function::append_value(t) and GetObjective()
            // does the make_uniform(dt0) itself (Objective_Function.cpp:133), so
            // uniformizing here too would resample twice and score a different
            // curve than the interpreter. (Observations differ: there the
            // interpreter's FinalizeOutputs really does uniformize first.)
            const int nk = k.n_objectives();
            for (int i = 0; i < (int)ObjectiveFunctionsCount() && i < nk; i++)
            {
                const int n = k.objective_count(h_, i);
                TimeSeries<timeseriesprecision> ts;
                double t = 0, v = 0;
                for (int s = 0; s < n; s++)
                    if (k.objective_at(h_, i, s, &t, &v)) ts.addPoint(t, v);
                objectivefunction(i)->SetTimeSeries(ts);
            }
        }
        // the interpreter records this in Solve(); without it the MCMC detail log
        // reports simulation_duration: 0 for every sample
        SetSimulationDuration(time_t(std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count()));
        SetSolutionFailed(!ok);
        return ok;
    }

private:
    // For each EMC observation, the index of a non-EMC observation recording
    // its weighting expression (default "flow") on its weighting object (default
    // the observation's own object); -1 if there is none. Non-EMC entries are -1.
    std::vector<int> EMCWeightObservations()
    {
        auto prop = [](Observation* o, const std::string& name) {
            return o->Variable(name) ? aquiutils::trim(o->Variable(name)->GetProperty()) : std::string();
        };
        std::vector<int> idx(ObservationsCount(), -1);
        for (int i = 0; i < (int)ObservationsCount(); i++)
        {
            Observation* oi = observation(i);
            if (!oi->IsEMC()) continue;
            std::string wloc = prop(oi, "emc_weighting_object");
            if (wloc.empty()) wloc = oi->GetLocation();
            std::string wexp = prop(oi, "emc_weighting_expression");
            if (wexp.empty()) wexp = "flow";
            for (int j = 0; j < (int)ObservationsCount(); j++)
            {
                Observation* oj = observation(j);
                if (j == i || oj->IsEMC()) continue;
                if (aquiutils::trim(oj->GetLocation()) == wloc && prop(oj, "expression") == wexp) { idx[i] = j; break; }
            }
        }
        return idx;
    }
    void release() { if (h_) { TheKernel().destroy(h_); h_ = nullptr; } }
    void* h_ = nullptr;
public:
    static long kernelTimeouts() { return kernel_timeouts_; }
private:
    static inline long kernel_timeouts_ = 0;
};

} // namespace ohq
