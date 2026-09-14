/*
 * OpenHydroQual - Codegen: G4 (runtime forcing injection) + G5 (state values
 * in/out) test. Standalone: includes only the generated header + runtime.
 *
 *   g++ -O2 -std=c++17 -I<gen> -I<gen>/runtime -DGEN_HEADER='"X.h"' -DGEN_CLASS=X \
 *       kernel_api_test.cpp -o kernel_api_test && ./kernel_api_test <forcing_dir>
 *
 * 1. G4: injecting the SAME precipitation bins (rain.txt) through
 *    setPrecipitation() and the same temperature samples through setSeries()
 *    must reproduce the baked series exactly (bit-identical trajectory).
 * 2. G5: run A straight t0->tend; run B t0->tmid, exportState, fresh C
 *    importState at tmid and run to tend; C must match A (dt_base restarts at
 *    tmid, so the step path differs slightly: expect ~1e-4 rel, not 0).
 * 3. G4 semantic: scaling the injected rain by 0.5 must change the result.
 */
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include GEN_HEADER

static bool readCols(const std::string& path, std::vector<std::vector<double>>& cols)
{
    std::ifstream f(path); if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        std::stringstream ss(line); std::string cell; std::vector<double> row;
        while (std::getline(ss, cell, ',')) { try { row.push_back(std::stod(cell)); } catch (...) { row.clear(); break; } }
        if (row.empty()) continue;
        if (cols.size() < row.size()) cols.resize(row.size());
        for (size_t k = 0; k < row.size(); ++k) cols[k].push_back(row[k]);
    }
    return !cols.empty();
}
static double maxRelDiff(const GEN_CLASS& a, const GEN_CLASS& b)
{
    double scale = 1e-30, m = 0;
    for (int i = 0; i < a.nBlocks(); ++i) scale = std::max(scale, std::fabs(a.state(i)));
    for (int i = 0; i < a.nBlocks(); ++i) m = std::max(m, std::fabs(a.state(i) - b.state(i)));
    return m / scale;
}

int main(int argc, char** argv)
{
    const std::string dir = argc > 1 ? argv[1] : ".";
    const double t0 = GEN_CLASS::simulationStart(), tend = t0 + 60.0, tmid = t0 + 25.0;
    int fails = 0;

    // ---- reference: baked series -------------------------------------------
    GEN_CLASS A; A.initialize(); if (!A.runTo(tend)) { std::printf("A failed\n"); return 2; }

    // ---- G4: inject identical forcing by name --------------------------------
    std::vector<std::vector<double>> rain, temp;
    if (!readCols(dir + "/forcing/rain.txt", rain) || !readCols(dir + "/forcing/temperature.txt", temp)) { std::printf("cannot read forcing files in %s\n", dir.c_str()); return 1; }
    GEN_CLASS B; B.initialize();
    std::printf("series table (%d):", (int)GEN_CLASS::N_SERIES);
    for (int k = 0; k < GEN_CLASS::N_SERIES; ++k) std::printf(" [%s.%s]", GEN_CLASS::seriesObject(k), GEN_CLASS::seriesQuantity(k));
    std::printf("\n");
    bool ok1 = B.setPrecipitation("Rain", "timeseries", rain[0].data(), rain[1].data(), rain[2].data(), (int)rain[0].size());
    bool ok2 = B.setSeries("Evapotranspiration_Penman (Soil)", "Temperature", temp[0].data(), temp[1].data(), (int)temp[0].size());
    std::printf("G4 setPrecipitation(Rain.timeseries)=%d  setSeries(Penman.Temperature)=%d\n", ok1, ok2);
    if (!ok1 || !ok2) ++fails;
    // the injected series must equal the baked one sample by sample
    const ohq::TimeSeries* sr = B.series("Rain", "timeseries"); const ohq::TimeSeries* sa = A.series("Rain", "timeseries");
    double dmax = 0; if (sr->size() != sa->size()) dmax = 1e300; else for (size_t k = 0; k < sr->size(); ++k) dmax = std::max(dmax, std::max(std::fabs(sr->t[k]-sa->t[k]), std::fabs(sr->c[k]-sa->c[k])));
    std::printf("G4 injected vs baked rain series: n=%zu/%zu max|diff|=%.3g %s\n", sr->size(), sa->size(), dmax, dmax < 1e-9 ? "OK" : "FAIL");
    if (dmax >= 1e-9) ++fails;
    if (!B.runTo(tend)) { std::printf("B failed\n"); return 2; }
    const double r1 = maxRelDiff(A, B);
    std::printf("G4 trajectory with injected (identical) forcing vs baked: max rel diff = %.3e %s\n", r1, r1 < 1e-12 ? "OK (bit-identical)" : (r1 < 1e-9 ? "OK" : "FAIL"));
    if (r1 >= 1e-9) ++fails;

    // ---- G4 semantic: halved rain must change the answer ------------------------
    GEN_CLASS D; D.initialize();
    std::vector<double> half(rain[2]); for (double& x : half) x *= 0.5;
    D.setPrecipitation("Rain", "timeseries", rain[0].data(), rain[1].data(), half.data(), (int)rain[0].size());
    if (!D.runTo(tend)) { std::printf("D failed\n"); return 2; }
    const double r2 = maxRelDiff(A, D);
    std::printf("G4 halved rain vs baked: max rel diff = %.3e %s\n", r2, r2 > 1e-3 ? "OK (changed)" : "FAIL (no effect)");
    if (r2 <= 1e-3) ++fails;

    // ---- G5: export at tmid, import into a fresh instance, continue ----------------
    GEN_CLASS E; E.initialize(); if (!E.runTo(tmid)) { std::printf("E failed\n"); return 2; }
    std::vector<double> st(GEN_CLASS::N_STATES), lf(GEN_CLASS::N_STATES); std::vector<int> li(GEN_CLASS::N_STATES);
    std::vector<double> ma(64, 0.0);
    E.exportState(st.data(), ma.data(), li.data(), lf.data());
    GEN_CLASS C; C.initialize();
    C.importState(E.time(), st.data(), ma.data(), li.data(), lf.data());
    C.clearObservations();
    std::printf("G5 importState at t=%.3f: time()=%.3f state[2]=%.6g (exported %.6g) %s\n", tmid, C.time(), C.state(2), st[2],
                (std::fabs(C.time() - tmid) < 1e-12 && C.state(2) == st[2]) ? "OK" : "FAIL");
    if (!(std::fabs(C.time() - tmid) < 1e-12 && C.state(2) == st[2])) ++fails;
    if (!C.runTo(tend)) { std::printf("C failed\n"); return 2; }
    const double r3 = maxRelDiff(A, C);
    std::printf("G5 restarted run vs straight run at tend: max rel diff = %.3e %s (dt_base restarts at tmid -> step path differs)\n",
                r3, r3 < 1e-3 ? "OK" : "FAIL");
    if (r3 >= 1e-3) ++fails;
    // observations recorded only after the restart
    if (GEN_CLASS::N_OBSERVATIONS > 0) {
        const ohq::TimeSeries& o = C.observationSeries(0);
        std::printf("G5 observations after restart: %zu points, first t=%.3f %s\n", o.size(), o.size() ? o.t[0] : -1.0,
                    (o.size() && o.t[0] > tmid) ? "OK" : "FAIL");
        if (!(o.size() && o.t[0] > tmid)) ++fails;
    }
    std::printf("%s\n", fails ? "FAIL" : "PASS");
    return fails ? 3 : 0;
}
