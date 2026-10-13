// Regression test: a parenthesised group of only numeric constants, e.g. "(3-1)", used to be
// taken by aquiutils::isnumber as a single number (it only checked that every character was a
// digit, '.' or '-'), so atof kept the first operand and dropped the rest: "(3-1)/2" -> 1.5,
// "(5-3.5)/0.5" -> 10, "(0-1)" -> 0.
//
// Build against the object files of a console build (all except main.o), e.g.:
//   g++ -std=gnu++17 -fopenmp -Dubuntu_version -DTerminal_version -DQ_JSON_SUPPORT -DARMA_USE_LAPACK \
//       -DARMA_USE_BLAS -DGSL -Iaquifolium/include -Iaquifolium/src -Iaquifolium/include/GA \
//       -Iaquifolium/include/MCMC -Ijsoncpp/include -I. $(pkg-config --cflags Qt6Core) \
//       tests/expression_constant_groups.cpp $(ls BUILD/*.o | grep -v main.o) \
//       -larmadillo -llapack -lblas -lgsl -lsuperlu $(pkg-config --libs Qt6Core) -o expr_test
#include "Expression.h"
#include "Block.h"
#include "Utilities.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static int failures = 0;

static void check(const char *text, double expected)
{
    Block b;
    Expression e(text);
    const double v = e.calc(&b, Expression::timing::present);
    const bool ok = std::abs(v - expected) < 1e-9 * std::max(1.0, std::abs(expected));
    std::printf("%-6s %-16s = %-12g (expected %g)\n", ok ? "ok" : "FAIL", text, v, expected);
    if (!ok) failures++;
}

int main()
{
    // isnumber: single literals only
    assert(aquiutils::isnumber("3"));
    assert(aquiutils::isnumber("-0.5"));
    assert(aquiutils::isnumber("1e-12"));
    assert(aquiutils::isnumber("2.5E+3"));
    assert(!aquiutils::isnumber("3-1"));
    assert(!aquiutils::isnumber("5-3.5"));
    assert(!aquiutils::isnumber("0-1"));
    assert(!aquiutils::isnumber("1-"));
    assert(!aquiutils::isnumber("2e3-1"));
    assert(!aquiutils::isnumber("e1"));
    assert(!aquiutils::isnumber("x"));

    check("(3-1)/2", 1.0);
    check("(5-3.5)/0.5", 3.0);
    check("(3-1)*2", 4.0);
    check("(0-1)", -1.0);
    check("2*(0-1)", -2.0);
    check("3-1", 2.0);
    check("10-2-3", 5.0);
    check("1e-3*(4-2)", 2e-3);
    check("2^(3-1)", 4.0);
    check("-0.5", -0.5);
    check("1e-12", 1e-12);
    check("_max(3-1;1)", 2.0);
    check("_min((5-3.5)/0.5;10)", 3.0);

    std::printf("%s\n", failures ? "FAILED" : "all passed");
    return failures ? 1 : 0;
}
