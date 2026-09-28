#include "TimeSeries.h"
#include <cassert>
#include <cmath>
#include <limits>

int main() {
    TimeSeries<double> forcing;
    forcing.setName("rain"); forcing.setUnit("m/day"); forcing.setFilename("single.csv");
    forcing.append(0.5, 0.01);
    auto uniform = forcing.make_uniform(0.001, true);
    assert(uniform.size() == 1);
    assert(uniform.name() == "rain" && uniform.unit() == "m/day");
    assert(uniform.getFilename() == "single.csv");
    assert(uniform.interpol(0.0) == 0.01 && uniform.interpol(1.0) == 0.01);
    assert(uniform.front().d.has_value());

    forcing.append(1.0, std::numeric_limits<double>::quiet_NaN());
    assert(forcing.make_uniform(0.001).size() == 1);
    TimeSeries<double> invalid;
    invalid.append(0, std::numeric_limits<double>::quiet_NaN());
    assert(invalid.make_uniform(0.001).empty());
    assert(forcing.make_uniform(0).empty());
    TimeSeries<double> ramp;
    ramp.append(0, 0); ramp.append(1, 2);
    auto regular = ramp.make_uniform(0.25);
    assert(regular.size() == 5 && std::abs(regular.interpol(0.5)-1) < 1e-12);
}
