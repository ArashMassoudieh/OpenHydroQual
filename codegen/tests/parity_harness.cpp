/*
 * OpenHydroQual - Codegen: numerical parity harness
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Loads a model, solves it with the INTERPRETER (System::Solve), then generates
 * the standalone model, compiles + loads it at build time (the generated header
 * is #included), runs it to the same tend, and compares per-block final state.
 *
 * The generated header is produced by ohq_generate before this is compiled; its
 * path is provided via the GEN_HEADER macro and GEN_CLASS class name.
 */
#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <string>

#include GEN_HEADER   // the generated model header (e.g. "ParallelModel.h")

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 3) { std::fprintf(stderr, "usage: parity <model.ohq> <resources_dir>\n"); return 1; }
    const std::string model = argv[1];
    const std::string res   = argv[2];

    // ---- interpreter run ---------------------------------------------------
    System sys;
    sys.SetDefaultTemplatePath(res + "/");
    sys.SetWorkingFolder(QFileInfo(QString::fromStdString(model)).canonicalPath().toStdString() + "/");
    sys.SetSilent(true);
    Script scr(model, &sys);
    sys.CreateFromScript(scr, res + "/settings.json");
    auto ti0 = std::chrono::steady_clock::now();
    sys.Solve();
    auto ti1 = std::chrono::steady_clock::now();
    const double interp_sec = std::chrono::duration<double>(ti1 - ti0).count();
    // The interpreter's outer loop runs while (t < tend + dt), so it overshoots
    // tend by up to one step. Compare at the ACTUAL final time it reached.
    const double tend = sys.GetTime();

    const unsigned nB = sys.BlockCount();
    std::vector<double> interp(nB), gen(nB);
    for (unsigned i = 0; i < nB; ++i)
        interp[i] = sys.block(i)->GetVal("Storage", Expression::timing::past);

    // ---- generated run -----------------------------------------------------
    GEN_CLASS m;
    m.initialize();
    auto tg0 = std::chrono::steady_clock::now();
    bool okg = m.runTo(tend);
    auto tg1 = std::chrono::steady_clock::now();
    const double gen_sec = std::chrono::duration<double>(tg1 - tg0).count();
    if (!okg) { std::fprintf(stderr, "generated model failed to solve\n"); return 2; }
    for (unsigned i = 0; i < nB && i < (unsigned)m.nBlocks(); ++i)
        gen[i] = m.state(i);

    // ---- compare (summary; rel error scaled by the largest storage) --------
    double scale = 0.0, tot_i = 0.0, tot_g = 0.0;
    for (unsigned i = 0; i < nB; ++i) { scale = std::max(scale, std::fabs(interp[i])); tot_i += interp[i]; tot_g += gen[i]; }
    double max_abs = 0.0, max_rel_scaled = 0.0; unsigned worst = 0;
    for (unsigned i = 0; i < nB; ++i) {
        double ae = std::fabs(interp[i] - gen[i]);
        if (ae > max_abs) { max_abs = ae; worst = i; }
        max_rel_scaled = std::max(max_rel_scaled, ae / (scale + 1e-30));
    }
    if (nB <= 20) {
        std::printf("  block   interpreter        generated          abs.err\n");
        for (unsigned i = 0; i < nB; ++i)
            std::printf("  %-6u  %16.8g  %16.8g  %10.2e\n", i, interp[i], gen[i], std::fabs(interp[i]-gen[i]));
    } else {
        std::printf("worst block %u: interp=%.8g  gen=%.8g  abs=%.3e\n",
                    worst, interp[worst], gen[worst], max_abs);
    }
    const bool pass = (max_rel_scaled < 5e-3);
    std::printf("-----------------------------------------------------------------------\n");
    std::printf("blocks=%u  tend=%.4g\n", nB, tend);
    std::printf("total storage: interp=%.8g  gen=%.8g  (rel diff %.2e)\n",
                tot_i, tot_g, std::fabs(tot_i - tot_g) / (std::fabs(tot_i) + 1e-30));
    std::printf("max abs err=%.3e  max rel err(scaled by %.4g)=%.3e -> %s\n",
                max_abs, scale, max_rel_scaled, pass ? "PASS" : "FAIL");
    std::printf("RUNTIME: interpreter=%.3f s   generated=%.3f s   speedup=%.2fx\n",
                interp_sec, gen_sec, interp_sec / (gen_sec + 1e-30));
    return pass ? 0 : 3;
}
