/*
 * OpenHydroQual - Codegen demo driver
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Demonstrates the external API of a generated model: construct, set inputs,
 * initialize, then either single-step or run to a time -- exactly how another
 * program would embed the compiled solver. Also checks the analytical steady
 * state so we know the runtime is solving correctly.
 */
#include <cstdio>
#include "two_pond_model.h"

int main()
{
    TwoPondModel m;
    const double Qin = 30.0;         // m^3/day constant inflow to pond 0
    m.setConstantInflow(Qin);
    m.initialize(/*tstart=*/0.0, /*dt0=*/0.01);

    std::printf("  t        S0        S1        h0        h1     iters\n");
    std::printf("--------------------------------------------------------\n");

    // Run to t=200 days, printing every ~20 days using single steps + runTo.
    double next_report = 0.0;
    while (m.time() < 200.0) {
        if (!m.runTo(std::min(next_report, 200.0))) { std::printf("SOLVE FAILED\n"); return 1; }
        std::printf("%6.1f  %8.3f  %8.3f  %8.4f  %8.4f   %4d\n",
                    m.time(), m.storage(0), m.storage(1),
                    m.head(0), m.head(1), m.lastIterations());
        next_report += 20.0;
        if (next_report > 200.0 && m.time() >= 200.0) break;
    }

    // Analytical steady state: qin = qout = K_OUT*h1  -> h1 = Qin/K_OUT
    //   q01 = qin -> K_LINK*(h0-h1) = Qin -> h0 = h1 + Qin/K_LINK
    const double h1_ss = Qin / TwoPondModel::K_OUT;
    const double h0_ss = h1_ss + Qin / TwoPondModel::K_LINK;
    std::printf("--------------------------------------------------------\n");
    std::printf("analytical steady state: h0=%.4f  h1=%.4f\n", h0_ss, h1_ss);
    std::printf("numerical  at t=%.1f    : h0=%.4f  h1=%.4f\n",
                m.time(), m.head(0), m.head(1));

    const double err = std::fabs(m.head(0) - h0_ss) + std::fabs(m.head(1) - h1_ss);
    std::printf("steady-state error       : %.3e  -> %s\n",
                err, err < 1e-3 ? "PASS" : "CHECK");
    return err < 1e-3 ? 0 : 2;
}
