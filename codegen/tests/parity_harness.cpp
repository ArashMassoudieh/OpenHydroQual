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
    sys.Solve();
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
    if (!m.runTo(tend)) { std::fprintf(stderr, "generated model failed to solve\n"); return 2; }
    for (unsigned i = 0; i < nB && i < (unsigned)m.nBlocks(); ++i)
        gen[i] = m.state(i);

    // ---- compare -----------------------------------------------------------
    std::printf("  block   interpreter        generated          abs.err   rel.err\n");
    std::printf("-----------------------------------------------------------------------\n");
    double max_rel = 0.0, max_abs = 0.0;
    for (unsigned i = 0; i < nB; ++i) {
        double a = interp[i], b = gen[i];
        double abserr = std::fabs(a - b);
        double relerr = abserr / (std::fabs(a) + 1e-12);
        max_abs = std::max(max_abs, abserr);
        max_rel = std::max(max_rel, relerr);
        std::printf("  %-6u  %16.8g  %16.8g  %10.2e %9.2e\n", i, a, b, abserr, relerr);
    }
    std::printf("-----------------------------------------------------------------------\n");
    std::printf("tend=%.4g   max abs err=%.3e   max rel err=%.3e -> %s\n",
                tend, max_abs, max_rel, (max_rel < 1e-4 || max_abs < 1e-6) ? "PASS" : "FAIL");
    return (max_rel < 1e-4 || max_abs < 1e-6) ? 0 : 3;
}
