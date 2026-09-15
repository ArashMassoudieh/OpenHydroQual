/*
 * OpenHydroQual - Codegen: parameter (G1) + observation (G2) parity harness
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * The Phase-1 gate from OpenHydroTwin/CODEGEN_ROADMAP.md: perturb every model
 * parameter, run the INTERPRETER the way CMCMC/CGA do
 * (SetParameterValue -> ApplyParameters -> Solve) and the GENERATED model the
 * way the kernel will be driven (setParameter -> applyParameters -> step...),
 * then compare every observation's recorded series and the final storages.
 *
 * Comparison basis: after Solve the interpreter resamples each observation to
 * dt0 (MakeObservationsExpressionUniform); we compare at those uniform times by
 * linear interpolation of the kernel's per-step series. Empirically the
 * interpreter's series align with END-of-step time stamps (the kernel's
 * convention): on the dt-capped Wetland model every observation agrees to
 * <= 3e-3 rms with end stamps vs ~2e-2 with a start-of-step emulation
 * (UpdateObservations(t) is called before t += dt in System::Solve, but the
 * values it records already belong to the advanced time). The start-stamped
 * emulation is still printed as a diagnostic column.
 *
 * Run the interpreter with dt capped (max_timestep_increase_factor small): its
 * dt clamp follows PRECIPITATION series only, so on dry days it samples the
 * hourly ET inputs twice a day (issues.md ISSUE 8) and its Evaporation series
 * is then not a valid reference (rms ~0.16 vs 3e-3 when capped).
 *
 * usage: parity_obs <model.ohq> <resources_dir> [perturbation=0.15]
 *   GEN_HEADER / GEN_CLASS are given at compile time (see run_parity_obs.sh).
 *   PARITY_DUMP=<dir> writes every series to CSV.
 */
#include "System.h"
#include "Script.h"
#include "Parameter.h"
#include "observation.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <string>
#include <vector>

#include GEN_HEADER

struct Stat { double max_abs = 0, max_rel = 0, rms_rel = 0, last_i = 0, last_g = 0; };

static Stat compare(TimeSeries<timeseriesprecision>* ts, const ohq::TimeSeries& g)
{
    Stat s; double scale = 1e-30, se = 0; size_t n = 0;
    for (size_t k = 0; k < ts->size(); ++k) scale = std::max(scale, std::fabs((double)ts->getValue(k)));
    for (size_t k = 0; k < ts->size(); ++k) {
        const double t = ts->getTime(k), vi = ts->getValue(k), vg = g.interpol(t);
        const double e = std::fabs(vi - vg);
        s.max_abs = std::max(s.max_abs, e); se += e * e; ++n; s.last_i = vi; s.last_g = vg;
    }
    s.max_rel = s.max_abs / scale; s.rms_rel = n ? std::sqrt(se / n) / scale : 0;
    return s;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 3) { std::fprintf(stderr, "usage: parity_obs <model.ohq> <resources_dir> [perturbation]\n"); return 1; }
    const std::string model = argv[1], res = argv[2];
    const double pert = argc > 3 ? std::atof(argv[3]) : 0.15;

    System sys;
    sys.SetDefaultTemplatePath(res + "/");
    sys.SetWorkingFolder(QFileInfo(QString::fromStdString(model)).canonicalPath().toStdString() + "/");
    sys.SetSilent(true);
    Script scr(model, &sys);
    sys.CreateFromScript(scr, res + "/settings.json");

    // ---- perturbed parameter vector (alternating +/- pert, like a proposal) --
    const unsigned nP = sys.ParametersCount();
    std::vector<double> pv(nP);
    for (unsigned i = 0; i < nP; ++i)
        pv[i] = sys.GetParameter(i)->GetValue() * (1.0 + ((i % 2) ? -pert : pert));
    std::printf("parameters (%u):\n", nP);
    for (unsigned i = 0; i < nP; ++i)
        std::printf("  [%u] %-32s %12.6g -> %12.6g   gen: %s\n", i, sys.GetParameter(i)->GetName().c_str(),
                    sys.GetParameter(i)->GetValue(), pv[i], GEN_CLASS::parameterName(i));

    // ---- interpreter, driven exactly like CMCMC::model() ---------------------
    sys.SetRecordResults(false);
    sys.SetNumThreads(1);
    for (unsigned i = 0; i < nP; ++i) sys.SetParameterValue(i, pv[i]);
    sys.ApplyParameters();
    auto ti0 = std::chrono::steady_clock::now();
    sys.Solve();
    const double interp_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - ti0).count();
    const double tend = sys.GetTime();
    std::printf("interpreter: solved to t=%.6g in %.3f s, failed=%d, objective=%.6g\n",
                tend, interp_sec, (int)sys.GetSolutionFailed(), sys.GetObjectiveFunctionValue());

    // ---- generated, driven like the kernel will be ---------------------------
    GEN_CLASS m;
    m.initialize();
    for (unsigned i = 0; i < nP && (int)i < GEN_CLASS::N_PARAMETERS; ++i) m.setParameter(i, pv[i]);
    m.applyParameters();
    const int nO = GEN_CLASS::N_OBSERVATIONS;
    std::vector<ohq::TimeSeries> stamped(nO > 0 ? nO : 1);   // interpreter convention: (t_prev, value after step)
    std::vector<double> v(nO > 0 ? nO : 1);
    auto tg0 = std::chrono::steady_clock::now();
    bool okg = true; long steps = 0;
    while (m.time() < tend - 1e-30) {
        const double tprev = m.time();
        if (!m.stepTo(tend)) { okg = false; break; }
        ++steps;
        m.computeObservations(v.data(), m.time());
        for (int i = 0; i < nO; ++i) stamped[i].push(tprev, v[i]);
    }
    const double gen_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tg0).count();
    std::printf("generated:   solved to t=%.6g in %.3f s, %ld steps (%s)\n", m.time(), gen_sec, steps, okg ? "OK" : "FAILED");
    if (!okg) return 2;

    // ---- observations ----------------------------------------------------------
    const unsigned nOi = sys.ObservationsCount();
    bool pass = true;
    std::printf("\n%-32s | kernel series (end-of-step stamps)  | start-stamp emul. (diag) | finals\n", "");
    std::printf("%-32s | %9s %9s %9s | %9s %9s | %11s %11s\n", "observation", "rms rel", "max rel", "max abs", "rms rel", "max rel", "interp", "gen");
    const char* dump = std::getenv("PARITY_DUMP");
    for (unsigned i = 0; i < nOi && (int)i < nO; ++i) {
        TimeSeries<timeseriesprecision>* ts = sys.observation(i)->GetModeledTimeSeries();
        const Stat a = compare(ts, m.observationSeries(i));
        const Stat b = compare(ts, stamped[i]);
        const bool bad = a.rms_rel > 2e-2;
        if (bad) pass = false;
        std::printf("%-32s | %9.2e %9.2e %9.2e | %9.2e %9.2e | %11.5g %11.5g%s\n", sys.observation(i)->GetName().c_str(),
                    a.rms_rel, a.max_rel, a.max_abs, b.rms_rel, b.max_rel, a.last_i, a.last_g, bad ? "  <--" : "");
        if (dump) {
            std::string fn = std::string(dump) + "/obs_" + std::to_string(i) + ".csv";
            if (std::FILE* f = std::fopen(fn.c_str(), "w")) {
                std::fprintf(f, "t,interp,gen_stamped_at_t,gen_kernel_at_t\n");
                for (size_t k = 0; k < ts->size(); ++k)
                    std::fprintf(f, "%.8f,%.10g,%.10g,%.10g\n", (double)ts->getTime(k), (double)ts->getValue(k),
                                 stamped[i].interpol(ts->getTime(k)), m.observationSeries(i).interpol(ts->getTime(k)));
                std::fclose(f);
            }
            fn = std::string(dump) + "/obs_" + std::to_string(i) + "_gen.csv";
            if (std::FILE* f = std::fopen(fn.c_str(), "w")) {
                const ohq::TimeSeries& g = m.observationSeries(i);
                std::fprintf(f, "t,gen\n");
                for (size_t k = 0; k < g.size(); ++k) std::fprintf(f, "%.8f,%.10g\n", g.t[k], g.c[k]);
                std::fclose(f);
            }
        }
    }
    // ---- final storages ----------------------------------------------------------
    double max_rel_s = 0, scale = 1e-30;
    for (unsigned b = 0; b < sys.BlockCount(); ++b) scale = std::max(scale, std::fabs(sys.block(b)->GetVal("Storage", Expression::timing::past)));
    for (unsigned b = 0; b < sys.BlockCount() && (int)b < m.nBlocks(); ++b)
        max_rel_s = std::max(max_rel_s, std::fabs(sys.block(b)->GetVal("Storage", Expression::timing::past) - m.state(b)) / scale);
    if (max_rel_s > 5e-3) pass = false;
    std::printf("\nfinal storages: max rel err (scaled by %.4g) = %.3e\n", scale, max_rel_s);
    std::printf("RUNTIME: interpreter=%.3f s  generated=%.3f s  speedup=%.1fx\n", interp_sec, gen_sec, interp_sec / (gen_sec + 1e-30));
    std::printf("%s  (criteria: every observation rms rel <= 2e-2 [scaled by max |interpreter value|]; final storages <= 5e-3)\n",
                pass ? "PASS" : "FAIL");
    return pass ? 0 : 3;
}
