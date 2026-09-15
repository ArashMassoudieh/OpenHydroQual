/*
 * OpenHydroQual - Codegen: transport parity harness
 * Interpreter vs generated, comparing per-block constituent mass at final time.
 */
#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include GEN_HEADER

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const std::string model = argv[1], res = argv[2], cons = argv[3];  // constituent name

    System sys;
    sys.SetDefaultTemplatePath(res + "/");
    sys.SetWorkingFolder(QFileInfo(QString::fromStdString(model)).canonicalPath().toStdString() + "/");
    sys.SetSilent(true);
    Script scr(model, &sys);
    sys.CreateFromScript(scr, res + "/settings.json");
    sys.Solve();
    const double tend = sys.GetTime();

    const unsigned nB = sys.BlockCount();
    std::vector<double> interp(nB);
    for (unsigned i = 0; i < nB; ++i)
        interp[i] = sys.block(i)->GetVal("mass", cons, Expression::timing::past);

    GEN_CLASS m;
    m.initialize();
    if (!m.runTo(tend)) { std::fprintf(stderr, "generated failed\n"); return 2; }

    std::printf("  block   interp mass       gen mass          abs.err   rel.err\n");
    std::printf("-----------------------------------------------------------------------\n");
    double max_abs = 0, max_rel = 0, scale = 0;
    for (unsigned i = 0; i < nB; ++i) scale = std::max(scale, std::fabs(interp[i]));
    for (unsigned i = 0; i < nB; ++i) {
        double a = interp[i], b = m.constMass(i);   // nC=1 -> massIndex = i
        double abserr = std::fabs(a - b), relerr = abserr / (scale + 1e-12);
        max_abs = std::max(max_abs, abserr); max_rel = std::max(max_rel, relerr);
        std::printf("  %-6u  %16.8g  %16.8g  %10.2e %9.2e\n", i, a, b, abserr, relerr);
    }
    std::printf("-----------------------------------------------------------------------\n");
    std::printf("tend=%.4g  max abs=%.3e  max rel(scaled)=%.3e -> %s\n",
                tend, max_abs, max_rel, (max_rel < 5e-3) ? "PASS" : "FAIL");
    return (max_rel < 5e-3) ? 0 : 3;
}
