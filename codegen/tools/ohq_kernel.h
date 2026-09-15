/*
 * OpenHydroQual - ohq_kernel.h : the model-independent kernel ABI (v1)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * WHAT THIS IS
 * ------------
 * `ohq_generate --project lib|shared` emits a C API whose symbols carry the
 * generated class name (`Wetland_run_to`, `Col8_run_to`, ...), which a host can
 * only call if it already knows the model. Alongside them it emits a second,
 * FIXED set of names -- `ohq_kernel_*` -- so one generic host (OHQ-GA /
 * OHQ-MCMC `--kernel`, or the twin's DTRunner) can drive ANY generated model.
 * This header declares that fixed set, and provides a dlopen() loader for the
 * usual case where the kernel is chosen at run time.
 *
 * Keep in sync with the alias-ABI block in `codegen/src/CodeGenerator.cpp`.
 * `ohq_kernel_abi_version()` guards changes: bump OHQ_KERNEL_ABI_VERSION there
 * and here together, and the loader will refuse a mismatched library.
 *
 * WHAT THE ABI COVERS (and why -- see OpenHydroTwin/CODEGEN_ROADMAP.md)
 * --------------------------------------------------------------------
 *   lifetime   create / destroy / initialize[_at]
 *   solving    run_to / step_to / step / time
 *   G1 params  n_parameters / parameter_name / parameter / set_parameter /
 *              apply_parameters          (index = the model's parameter order,
 *              i.e. what CMCMC/CGA pass to SetParameterValue(i, .))
 *   G2 obs     n_observations / observation_name / observation_count /
 *              observation_at / clear_observations /
 *              uniformize_observations   (the last one matches
 *              System::FinalizeOutputs, which resamples to dt0 BEFORE the
 *              objective is computed -- a host that scores the kernel must call
 *              it or it compares on a different grid)
 *   G4 forcing n_series / series_object / series_quantity / set_series /
 *              set_precipitation        (maps 1:1 onto the twin's
 *              injectPrecipitation / injectWeather, addressed by name)
 *   G5 state   export_state / import_state -- state VALUES only. The kernel
 *              never reads or writes model structure; the full-model JSON
 *              snapshot stays with `System`, which hands the values across.
 *   G6 status  solution_failed / simulation_duration / step_count /
 *              last_iterations / reset_status
 *   results    n_states / state / state_name, n_mass / mass
 *
 * CONTRACTS THE HOST MUST HONOUR
 * ------------------------------
 *  - A failed solve must NOT be scored as a good fit. Check
 *    `solution_failed()` after running and reject the sample
 *    (`System::GetObjectiveFunctionValue` returns +1e18 in that case).
 *  - `set_parameter()` only stages a value; `apply_parameters()` is what
 *    rebuilds the derived constants (== System::ApplyParameters + a re-solve of
 *    every constant that depends on them).
 *  - `initialize()` resets the status counters and clears observations, i.e. it
 *    starts a fresh sample.
 *  - Handles are independent. One per thread/chain; nothing is shared.
 *  - `n_mass()` is 0 for a model without constituents; `mass()` then returns 0.
 *    The ABI is uniform either way, so a host need not branch.
 *
 * VERIFY BEFORE YOU TRUST IT (learned the hard way -- issues.md ISSUE 17)
 * ----------------------------------------------------------------------
 * Matching parameter NAMES and COUNTS is not enough: a kernel can expose a
 * parameter it silently ignores. Perturb each one and confirm the output moves.
 * `ohq_kernel_self_test()` below does exactly that; call it once at startup.
 */
#ifndef OHQ_KERNEL_H
#define OHQ_KERNEL_H

#define OHQ_KERNEL_ABI_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

/* ---- identity ------------------------------------------------------------ */
int         ohq_kernel_abi_version(void);
const char* ohq_kernel_class_name(void);

/* ---- lifetime ------------------------------------------------------------ */
void* ohq_kernel_create(void);
void  ohq_kernel_destroy(void* h);
void  ohq_kernel_initialize(void* h);                              /* model's own tstart/dt0 */
void  ohq_kernel_initialize_at(void* h, double tstart, double dt0);

/* ---- solving (non-zero = success) ---------------------------------------- */
int    ohq_kernel_run_to(void* h, double t);
int    ohq_kernel_step_to(void* h, double t);   /* one step, dt clamped to t */
int    ohq_kernel_step(void* h);                /* one step */
double ohq_kernel_time(const void* h);
double ohq_kernel_simulation_start(void);
double ohq_kernel_simulation_end(void);

/* ---- G1: parameters (index = the model's parameter order) ---------------- */
int         ohq_kernel_n_parameters(void);
const char* ohq_kernel_parameter_name(int i);
double      ohq_kernel_parameter(const void* h, int i);
void        ohq_kernel_set_parameter(void* h, int i, double v);
void        ohq_kernel_apply_parameters(void* h);

/* ---- G2: observations ---------------------------------------------------- */
int         ohq_kernel_n_observations(void);
const char* ohq_kernel_observation_name(int i);
int         ohq_kernel_observation_count(const void* h, int i);
/* writes sample k of observation i; returns 0 if k is out of range */
int         ohq_kernel_observation_at(const void* h, int i, int k, double* t, double* v);
void        ohq_kernel_clear_observations(void* h);
/* resample every observation onto the dt0 grid, as System::FinalizeOutputs does
   before the objective is computed. Call before scoring. */
void        ohq_kernel_uniformize_observations(void* h);

/* ---- G4: runtime forcing, addressed by (object, quantity) name ----------- */
int         ohq_kernel_n_series(void);
const char* ohq_kernel_series_object(int k);
const char* ohq_kernel_series_quantity(int k);
/* (t, value) samples in the quantity's SI unit; returns 0 if the name is unknown */
int ohq_kernel_set_series(void* h, const char* object, const char* quantity,
                          const double* t, const double* v, int n);
/* precipitation bins (start, end, depth) -> midpoint intensities, exactly as
   the interpreter converts a precipitation file / injection */
int ohq_kernel_set_precipitation(void* h, const char* object, const char* quantity,
                                 const double* start, const double* end,
                                 const double* depth, int n);

/* ---- G5: state VALUES in/out (hot restart) ------------------------------- */
/* storage[n_states]; mass[n_mass], limited[n_states], limitFactor[n_states] may be NULL */
void ohq_kernel_export_state(const void* h, double* storage, double* mass,
                             int* limited, double* limitFactor);
void ohq_kernel_import_state(void* h, double t, const double* storage, const double* mass,
                             const int* limited, const double* limitFactor);

/* ---- G6: solver status --------------------------------------------------- */
int    ohq_kernel_solution_failed(const void* h);     /* non-zero => reject the sample */
double ohq_kernel_simulation_duration(const void* h); /* wall seconds spent solving */
long   ohq_kernel_step_count(const void* h);
int    ohq_kernel_last_iterations(const void* h);
void   ohq_kernel_reset_status(void* h);

/* ---- results ------------------------------------------------------------- */
int         ohq_kernel_n_states(void);
double      ohq_kernel_state(const void* h, int i);
const char* ohq_kernel_state_name(int i);
int         ohq_kernel_n_mass(void);                  /* 0 when no constituents */
double      ohq_kernel_mass(const void* h, int i);

#ifdef __cplusplus
}   /* extern "C" */
#endif

/* ==========================================================================
 *  Run-time loader (C++ only; define OHQ_KERNEL_NO_LOADER to omit).
 *  Link with -ldl.
 *
 *      ohq::Kernel k;
 *      if (!k.load("libWetland.so")) { std::cerr << k.error() << "\n"; ... }
 *      void* h = k.create(); k.initialize(h);
 *      for (int i = 0; i < k.n_parameters(); ++i) k.set_parameter(h, i, par[i]);
 *      k.apply_parameters(h);
 *      const bool ok = k.run_to(h, k.simulation_end());
 *      if (!ok || k.solution_failed(h)) { reject } else { k.uniformize_observations(h); ... }
 *      k.destroy(h);
 * ========================================================================== */
#if defined(__cplusplus) && !defined(OHQ_KERNEL_NO_LOADER)

#include <dlfcn.h>
#include <string>
#include <vector>
#include <cmath>

namespace ohq {

class Kernel {
public:
    Kernel() = default;
    ~Kernel() { close(); }
    Kernel(const Kernel&) = delete;
    Kernel& operator=(const Kernel&) = delete;

    /* Loads the library and binds every symbol. Returns false (and sets
       error()) on a missing symbol or an ABI-version mismatch. */
    bool load(const std::string& path)
    {
        close();
        lib_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!lib_) { err_ = dlerror() ? dlerror() : ("cannot dlopen " + path); return false; }
        bool ok = true;
        // identity first, so a version mismatch is reported before anything else
        ok &= bind(abi_version, "ohq_kernel_abi_version");
        ok &= bind(class_name,  "ohq_kernel_class_name");
        if (!ok) { close(); return false; }
        if (abi_version() != OHQ_KERNEL_ABI_VERSION) {
            err_ = path + ": kernel ABI v" + std::to_string(abi_version())
                 + ", host expects v" + std::to_string(OHQ_KERNEL_ABI_VERSION);
            close(); return false;
        }
        ok &= bind(create, "ohq_kernel_create");
        ok &= bind(destroy, "ohq_kernel_destroy");
        ok &= bind(initialize, "ohq_kernel_initialize");
        ok &= bind(initialize_at, "ohq_kernel_initialize_at");
        ok &= bind(run_to, "ohq_kernel_run_to");
        ok &= bind(step_to, "ohq_kernel_step_to");
        ok &= bind(step, "ohq_kernel_step");
        ok &= bind(time, "ohq_kernel_time");
        ok &= bind(simulation_start, "ohq_kernel_simulation_start");
        ok &= bind(simulation_end, "ohq_kernel_simulation_end");
        ok &= bind(n_parameters, "ohq_kernel_n_parameters");
        ok &= bind(parameter_name, "ohq_kernel_parameter_name");
        ok &= bind(parameter, "ohq_kernel_parameter");
        ok &= bind(set_parameter, "ohq_kernel_set_parameter");
        ok &= bind(apply_parameters, "ohq_kernel_apply_parameters");
        ok &= bind(n_observations, "ohq_kernel_n_observations");
        ok &= bind(observation_name, "ohq_kernel_observation_name");
        ok &= bind(observation_count, "ohq_kernel_observation_count");
        ok &= bind(observation_at, "ohq_kernel_observation_at");
        ok &= bind(clear_observations, "ohq_kernel_clear_observations");
        ok &= bind(uniformize_observations, "ohq_kernel_uniformize_observations");
        ok &= bind(n_series, "ohq_kernel_n_series");
        ok &= bind(series_object, "ohq_kernel_series_object");
        ok &= bind(series_quantity, "ohq_kernel_series_quantity");
        ok &= bind(set_series, "ohq_kernel_set_series");
        ok &= bind(set_precipitation, "ohq_kernel_set_precipitation");
        ok &= bind(export_state, "ohq_kernel_export_state");
        ok &= bind(import_state, "ohq_kernel_import_state");
        ok &= bind(solution_failed, "ohq_kernel_solution_failed");
        ok &= bind(simulation_duration, "ohq_kernel_simulation_duration");
        ok &= bind(step_count, "ohq_kernel_step_count");
        ok &= bind(last_iterations, "ohq_kernel_last_iterations");
        ok &= bind(reset_status, "ohq_kernel_reset_status");
        ok &= bind(n_states, "ohq_kernel_n_states");
        ok &= bind(state, "ohq_kernel_state");
        ok &= bind(state_name, "ohq_kernel_state_name");
        ok &= bind(n_mass, "ohq_kernel_n_mass");
        ok &= bind(mass, "ohq_kernel_mass");
        if (!ok) { close(); return false; }
        return true;
    }

    void close()
    {
        if (lib_) { dlclose(lib_); lib_ = nullptr; }
    }
    bool loaded() const { return lib_ != nullptr; }
    const std::string& error() const { return err_; }

    /* Read every observation i back as (t, value) pairs. */
    void observation_series(const void* h, int i,
                            std::vector<double>& t, std::vector<double>& v) const
    {
        const int n = observation_count(h, i);
        t.clear(); v.clear(); t.reserve(n); v.reserve(n);
        for (int k = 0; k < n; ++k) {
            double tk = 0, vk = 0;
            if (observation_at(h, i, k, &tk, &vk)) { t.push_back(tk); v.push_back(vk); }
        }
    }

    /* Does this kernel actually RESPOND to every parameter it advertises?
     * ISSUE 17: a kernel that ignores a parameter reports a converged optimum
     * that does not reproduce. Perturbs each parameter by `rel`, runs a short
     * window, and requires some state to move. Returns the index of the first
     * dead parameter, or -1 if all are live. `skip` names parameters the HOST
     * owns (an observation's sigma), which are legitimately inert here.
     */
    int self_test(double rel = 0.05, double window = 0.0,
                  const std::vector<std::string>& skip = {}) const
    {
        if (!loaded()) return -2;
        const double t0 = simulation_start();
        const double tend = window > 0 ? t0 + window
                                       : t0 + 0.05 * (simulation_end() - t0);
        // Compare over the FULL state: storages AND constituent masses. A
        // chemistry parameter (a sorption rate, a decay constant) never moves a
        // flow-phase storage, so checking state() alone reports it as dead.
        const int ns = n_states(), nm = n_mass();
        std::vector<double> base(ns + nm);
        auto snapshot = [&](void* h, std::vector<double>& out) {
            for (int i = 0; i < ns; ++i) out[i] = state(h, i);
            for (int i = 0; i < nm; ++i) out[ns + i] = mass(h, i);
        };
        {
            void* h = create(); initialize(h); run_to(h, tend);
            snapshot(h, base);
            destroy(h);
        }
        for (int p = 0; p < n_parameters(); ++p) {
            bool skipped = false;
            for (const std::string& s : skip) if (s == parameter_name(p)) { skipped = true; break; }
            if (skipped) continue;
            void* h = create(); initialize(h);
            const double v0 = parameter(h, p);
            set_parameter(h, p, v0 == 0.0 ? rel : v0 * (1.0 + rel));
            apply_parameters(h);
            run_to(h, tend);
            std::vector<double> cur(ns + nm);
            snapshot(h, cur);
            destroy(h);
            // BITWISE comparison, deliberately: the kernel is deterministic, so a
            // parameter it actually reads changes the output in SOME bit, while
            // one it ignores reproduces the baseline exactly. A relative-magnitude
            // threshold is the wrong tool -- in the 8-column model a live sorption
            // parameter moves the answer by 5e-4 against a state vector dominated
            // by ~1e9 of (constant) bulk-density mass, i.e. 1e-12 relative, which
            // any sane threshold would call dead.
            bool moved = false;
            for (int i = 0; i < ns + nm && !moved; ++i) moved = (cur[i] != base[i]);
            if (!moved) return p;   // advertised but ignored
        }
        return -1;
    }

    /* ---- bound entry points (same names as the C ABI) -------------------- */
    int         (*abi_version)(void) = nullptr;
    const char* (*class_name)(void) = nullptr;
    void*       (*create)(void) = nullptr;
    void        (*destroy)(void*) = nullptr;
    void        (*initialize)(void*) = nullptr;
    void        (*initialize_at)(void*, double, double) = nullptr;
    int         (*run_to)(void*, double) = nullptr;
    int         (*step_to)(void*, double) = nullptr;
    int         (*step)(void*) = nullptr;
    double      (*time)(const void*) = nullptr;
    double      (*simulation_start)(void) = nullptr;
    double      (*simulation_end)(void) = nullptr;
    int         (*n_parameters)(void) = nullptr;
    const char* (*parameter_name)(int) = nullptr;
    double      (*parameter)(const void*, int) = nullptr;
    void        (*set_parameter)(void*, int, double) = nullptr;
    void        (*apply_parameters)(void*) = nullptr;
    int         (*n_observations)(void) = nullptr;
    const char* (*observation_name)(int) = nullptr;
    int         (*observation_count)(const void*, int) = nullptr;
    int         (*observation_at)(const void*, int, int, double*, double*) = nullptr;
    void        (*clear_observations)(void*) = nullptr;
    void        (*uniformize_observations)(void*) = nullptr;
    int         (*n_series)(void) = nullptr;
    const char* (*series_object)(int) = nullptr;
    const char* (*series_quantity)(int) = nullptr;
    int         (*set_series)(void*, const char*, const char*, const double*, const double*, int) = nullptr;
    int         (*set_precipitation)(void*, const char*, const char*, const double*, const double*, const double*, int) = nullptr;
    void        (*export_state)(const void*, double*, double*, int*, double*) = nullptr;
    void        (*import_state)(void*, double, const double*, const double*, const int*, const double*) = nullptr;
    int         (*solution_failed)(const void*) = nullptr;
    double      (*simulation_duration)(const void*) = nullptr;
    long        (*step_count)(const void*) = nullptr;
    int         (*last_iterations)(const void*) = nullptr;
    void        (*reset_status)(void*) = nullptr;
    int         (*n_states)(void) = nullptr;
    double      (*state)(const void*, int) = nullptr;
    const char* (*state_name)(int) = nullptr;
    int         (*n_mass)(void) = nullptr;
    double      (*mass)(const void*, int) = nullptr;

private:
    template <class Fn>
    bool bind(Fn& fn, const char* name)
    {
        dlerror();
        void* p = dlsym(lib_, name);
        const char* e = dlerror();
        if (!p || e) { err_ = std::string("missing symbol ") + name + (e ? std::string(": ") + e : ""); return false; }
        fn = reinterpret_cast<Fn>(p);
        return true;
    }

    void*       lib_ = nullptr;
    std::string err_;
};

} // namespace ohq

#endif /* __cplusplus && !OHQ_KERNEL_NO_LOADER */

#endif /* OHQ_KERNEL_H */
