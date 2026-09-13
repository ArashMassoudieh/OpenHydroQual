/*
 * OpenHydroQual - Codegen: ExpressionEmitter live test
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Builds real aquifolium Expression trees from strings and checks the emitted
 * C++ against expected output. Uses simple resolvers that append a trailing
 * underscore to value names and prefix source/destination refs with src./dst.
 */
#include <cstdio>
#include <string>
#include "Expression.h"
#include "ExpressionEmitter.h"

using ohqcg::ExpressionEmitter;
using ohqcg::EmitContext;
using ohqcg::Loc;

static int failures = 0;

static std::string run(const std::string& src)
{
    Expression e(src);
    EmitContext ctx;
    ctx.timeVar = "t_new";
    ctx.resolveValue = [](const std::string& n, Loc loc) -> std::string {
        if (loc == Loc::source)      return "src." + n + "_";
        if (loc == Loc::destination) return "dst." + n + "_";
        return n + "_";
    };
    ctx.resolveSeries = [](const std::string& n, Loc) -> std::string {
        return "ts_" + n + "_";
    };
    ExpressionEmitter em(ctx);
    return em.translate(e);
}

static void check(const std::string& src, const std::string& expected)
{
    std::string got = run(src);
    bool ok = (got == expected);
    if (!ok) ++failures;
    std::printf("[%s] %-45s -> %s\n", ok ? "PASS" : "FAIL", src.c_str(), got.c_str());
    if (!ok) std::printf("        expected: %s\n", expected.c_str());
}

int main()
{
    // precedence: * binds tighter than +
    check("a+b*c", "(a_ + (b_ * c_))");
    // explicit parentheses become nested terms
    check("(a+b)*c", "((a_ + b_) * c_)");
    // power maps to ohq::powr; division
    check("3.1415*(diameter^2)/4",
          "((3.1415 * ohq::powr(diameter_, 2.0)) / 4.0)");
    // intrinsics + source/destination endpoints
    check("_pos(head.s-head.e)", "ohq::pos((src.head_ - dst.head_))");
    // exponential kernel: series handle + time + rate
    check("_ekr(timeseries;lambda)", "ts_timeseries_.ekr(t_new, lambda_)");
    // gaussian kernel: three args
    check("_gkr(timeseries;mu;sigma)", "ts_timeseries_.gkr(t_new, mu_, sigma_)");
    // the real forecast expression from precipitation_forcast.json (uncertain)
    check("ratio*_ekr(timeseries;mu)+_ekr(timeseries_corrupted;lambda)-ratio*_ekr(timeseries_corrupted;mu)",
          "(((ratio_ * ts_timeseries_.ekr(t_new, mu_)) + ts_timeseries_corrupted_.ekr(t_new, lambda_)) - (ratio_ * ts_timeseries_corrupted_.ekr(t_new, mu_)))");
    // nested intrinsics + orifice max_flow
    check("86400*_sqr(2*9.81*_pos(head.s-head.e)/valve_K)*area",
          "((86400.0 * ohq::f_sqr((((2.0 * 9.81) * ohq::pos((src.head_ - dst.head_))) / valve_K_))) * area_)");

    std::printf("\n%s (%d failure%s)\n", failures ? "TESTS FAILED" : "ALL TESTS PASSED",
                failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
