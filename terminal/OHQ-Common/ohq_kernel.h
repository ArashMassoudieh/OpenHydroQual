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
#include <dlfcn.h>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace ohq {

// ---- the fixed-name ABI emitted by CodeGenerator (keep the two in sync) -----
struct KernelABI
{
    void*  lib = nullptr;
    int    (*abi_version)()                                   = nullptr;
    const char* (*class_name)()                               = nullptr;
    void*  (*create)()                                        = nullptr;
    void   (*destroy)(void*)                                  = nullptr;
    void   (*initialize)(void*)                               = nullptr;
    int    (*run_to)(void*, double)                           = nullptr;
    int    (*step_to)(void*, double)                          = nullptr;
    double (*time)(const void*)                               = nullptr;
    double (*simulation_end)()                                = nullptr;
    int    (*n_parameters)()                                  = nullptr;
    const char* (*parameter_name)(int)                        = nullptr;
    void   (*set_parameter)(void*, int, double)               = nullptr;
    void   (*apply_parameters)(void*)                         = nullptr;
    int    (*n_observations)()                                = nullptr;
    const char* (*observation_name)(int)                      = nullptr;
    int    (*observation_count)(const void*, int)             = nullptr;
    int    (*observation_at)(const void*, int, int, double*, double*) = nullptr;
    void   (*clear_observations)(void*)                       = nullptr;
    void   (*uniformize_observations)(void*)                  = nullptr;

    bool valid() const { return lib && create && run_to && observation_at; }
};

// One process-wide kernel library; every KernelSystem copy makes its own handle
// so parallel GA individuals never share solver state.
inline KernelABI& TheKernel()
{
    static KernelABI k;
    return k;
}

inline bool LoadKernel(const std::string& path)
{
    KernelABI& k = TheKernel();
    k.lib = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!k.lib)
    {
        std::cout << "Cannot load kernel '" << path << "': " << dlerror() << std::endl;
        return false;
    }
    auto sym = [&](const char* n) -> void* {
        void* p = dlsym(k.lib, n);
        if (!p) std::cout << "  kernel is missing symbol '" << n << "'" << std::endl;
        return p;
    };
    k.abi_version       = (int (*)())sym("ohq_kernel_abi_version");
    k.class_name        = (const char* (*)())sym("ohq_kernel_class_name");
    k.create            = (void* (*)())sym("ohq_kernel_create");
    k.destroy           = (void (*)(void*))sym("ohq_kernel_destroy");
    k.initialize        = (void (*)(void*))sym("ohq_kernel_initialize");
    k.run_to            = (int (*)(void*, double))sym("ohq_kernel_run_to");
    k.step_to           = (int (*)(void*, double))sym("ohq_kernel_step_to");
    k.time              = (double (*)(const void*))sym("ohq_kernel_time");
    k.simulation_end    = (double (*)())sym("ohq_kernel_simulation_end");
    k.n_parameters      = (int (*)())sym("ohq_kernel_n_parameters");
    k.parameter_name    = (const char* (*)(int))sym("ohq_kernel_parameter_name");
    k.set_parameter     = (void (*)(void*, int, double))sym("ohq_kernel_set_parameter");
    k.apply_parameters  = (void (*)(void*))sym("ohq_kernel_apply_parameters");
    k.n_observations    = (int (*)())sym("ohq_kernel_n_observations");
    k.observation_name  = (const char* (*)(int))sym("ohq_kernel_observation_name");
    k.observation_count = (int (*)(const void*, int))sym("ohq_kernel_observation_count");
    k.observation_at    = (int (*)(const void*, int, int, double*, double*))sym("ohq_kernel_observation_at");
    k.clear_observations= (void (*)(void*))sym("ohq_kernel_clear_observations");
    k.uniformize_observations = (void (*)(void*))sym("ohq_kernel_uniformize_observations");

    if (!k.valid())
    {
        std::cout << "'" << path << "' does not export the ohq_kernel ABI. Generate it with\n"
                  << "  ohq_generate <model.ohq> <resources> <out> <Class> Storage --project shared"
                  << std::endl;
        return false;
    }
    if (k.abi_version && k.abi_version() != 1)
    {
        std::cout << "Kernel ABI version " << k.abi_version() << ", expected 1." << std::endl;
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
    std::cout << "Kernel verified : " << np << " parameters, " << no
              << " observations, names match the model." << std::endl;
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

    // Shadows System::Solve. Same signature so the GA/MCMC templates bind to it.
    bool Solve(bool ApplyParams = false, bool uniformize_outputs = true)
    {
        (void)uniformize_outputs;
        KernelABI& k = TheKernel();
        if (!k.valid()) return System::Solve(ApplyParams, uniformize_outputs);
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
        // the interpreter records this in Solve(); without it the MCMC detail log
        // reports simulation_duration: 0 for every sample
        SetSimulationDuration(time_t(std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count()));
        SetSolutionFailed(!ok);
        return ok;
    }

private:
    void release() { if (h_) { TheKernel().destroy(h_); h_ = nullptr; } }
    void* h_ = nullptr;
public:
    static long kernelTimeouts() { return kernel_timeouts_; }
private:
    static inline long kernel_timeouts_ = 0;
};

} // namespace ohq
