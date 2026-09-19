/*
 * OpenHydroQual - Codegen: model-independent kernel ABI test.
 *
 * Drives a generated SHARED library entirely through tools/ohq_kernel.h and
 * dlopen(), knowing nothing about the model at compile time -- exactly what
 * OHQ-GA / OHQ-MCMC `--kernel` will do.
 *
 *   g++ -O2 -std=c++17 -I<codegen>/tools kernel_abi_test.cpp -ldl -o kernel_abi_test
 *   ./kernel_abi_test /path/to/libWetland.so [forcing_dir]
 */
#include "ohq_kernel.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: kernel_abi_test <lib.so> [forcing_dir]\n"); return 1; }
    int fails = 0;
    ohq::Kernel k;
    if (!k.load(argv[1])) { std::printf("load FAILED: %s\n", k.error().c_str()); return 2; }

    std::printf("loaded '%s'  ABI v%d  class=%s\n", argv[1], k.abi_version(), k.class_name());
    std::printf("  states=%d  mass=%d  parameters=%d  observations=%d  series=%d\n",
                k.n_states(), k.n_mass(), k.n_parameters(), k.n_observations(), k.n_series());
    std::printf("  window: %.4f -> %.4f\n", k.simulation_start(), k.simulation_end());

    const double t0 = k.simulation_start();
    const double tend = t0 + 0.05 * (k.simulation_end() - t0);   // short window

    // ---- lifetime + solve + G6 status -------------------------------------
    void* h = k.create();
    k.initialize(h);
    if (k.time(h) != t0) { std::printf("initialize: time=%.6f expected %.6f FAIL\n", k.time(h), t0); ++fails; }
    const bool ok = k.run_to(h, tend);
    std::printf("run_to(%.4f): ok=%d failed=%d steps=%ld solve=%.4f s lastIters=%d %s\n",
                tend, (int)ok, k.solution_failed(h), k.step_count(h), k.simulation_duration(h),
                k.last_iterations(h), (ok && !k.solution_failed(h)) ? "OK" : "FAIL");
    if (!ok || k.solution_failed(h)) ++fails;
    if (k.step_count(h) <= 0 || k.simulation_duration(h) <= 0.0) { std::printf("G6 counters not moving FAIL\n"); ++fails; }

    // ---- G2 observations ---------------------------------------------------
    if (k.n_observations() > 0) {
        k.uniformize_observations(h);   // System::FinalizeOutputs grid, before scoring
        std::vector<double> t, v;
        k.observation_series(h, 0, t, v);
        std::printf("observation[0] '%s': %zu samples, first (%.4f, %.6g), last (%.4f, %.6g) %s\n",
                    k.observation_name(0), t.size(),
                    t.empty() ? 0 : t.front(), v.empty() ? 0 : v.front(),
                    t.empty() ? 0 : t.back(),  v.empty() ? 0 : v.back(),
                    t.size() > 1 ? "OK" : "FAIL");
        if (t.size() <= 1) ++fails;
    }

    // ---- G5 state values in/out -------------------------------------------
    std::vector<double> st(k.n_states()), lf(k.n_states()), ma(k.n_mass() > 0 ? k.n_mass() : 1);
    std::vector<int> li(k.n_states());
    k.export_state(h, st.data(), k.n_mass() > 0 ? ma.data() : nullptr, li.data(), lf.data());
    const double t_mid = k.time(h);
    void* h2 = k.create();
    k.initialize(h2);
    k.import_state(h2, t_mid, st.data(), k.n_mass() > 0 ? ma.data() : nullptr, li.data(), lf.data());
    const bool st_ok = (std::fabs(k.time(h2) - t_mid) < 1e-12)
                    && (k.n_states() == 0 || k.state(h2, 0) == st[0]);
    std::printf("export/import state at t=%.4f: time=%.4f state[0]=%.6g (exported %.6g) %s\n",
                t_mid, k.time(h2), k.n_states() ? k.state(h2, 0) : 0.0, k.n_states() ? st[0] : 0.0,
                st_ok ? "OK" : "FAIL");
    if (!st_ok) ++fails;
    k.destroy(h2);

    // ---- G4 forcing: the series table is addressable by name ---------------
    if (k.n_series() > 0) {
        std::printf("series[0] = %s.%s  (of %d)\n",
                    k.series_object(0), k.series_quantity(0), k.n_series());
        // a bogus name must be rejected, a real one accepted
        const double tt[2] = {t0, tend}, vv[2] = {0.0, 0.0};
        const int bad  = k.set_series(h, "no such object", "no such quantity", tt, vv, 2);
        const int good = k.set_series(h, k.series_object(0), k.series_quantity(0), tt, vv, 2);
        std::printf("set_series: unknown-name=%d (expect 0), known-name=%d (expect 1) %s\n",
                    bad, good, (!bad && good) ? "OK" : "FAIL");
        if (bad || !good) ++fails;
    }
    k.destroy(h);

    // ---- G1: every advertised parameter must actually move the model -------
    // (ISSUE 17 -- names and counts are not enough.)
    std::vector<std::string> host_owned;   // likelihood params the kernel rightly ignores
    for (int i = 0; i < k.n_parameters(); ++i) {
        const std::string n = k.parameter_name(i);
        if (n.find("_Std") != std::string::npos || n.find("std") != std::string::npos
            || n.find("_sigma") != std::string::npos) host_owned.push_back(n);
    }
    const int dead = k.self_test(0.05, tend - t0, host_owned);
    if (dead == -1)
        std::printf("self_test: all %d parameters move the model (host-owned skipped: %zu) OK\n",
                    k.n_parameters(), host_owned.size());
    else {
        std::printf("self_test: parameter %d '%s' is advertised but IGNORED FAIL\n",
                    dead, k.parameter_name(dead));
        ++fails;
    }

    std::printf("%s\n", fails ? "FAIL" : "PASS");
    return fails ? 3 : 0;
}
