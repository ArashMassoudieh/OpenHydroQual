// Generate IntervalRainModel from the saved small native-rain fixture.
// Compile with -I<generated_dir> -I<generated_dir>/runtime.
// See tests/interval_rainfall.md and the hash-pinned audit for reproduction.
#include "IntervalRainModel.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 IntervalRainModel baked;baked.initialize();assert(baked.runTo(.1));
 std::cout<<"baked final storage "<<baked.state(0)<<"\n";
 assert(std::abs(baked.state(0)-12)<1e-7);
 IntervalRainModel injected;injected.initialize();
 double a[]={0,.02,.04},b[]={.02,.04,.1},d[]={0,.002,0};
 assert(injected.setPrecipitation("Rain","timeseries",a,b,d,3));
 double invalid[]={0,-1,0};assert(!injected.setPrecipitation("Rain","timeseries",a,b,invalid,3));
 assert(injected.runTo(.1));assert(std::abs(injected.state(0)-12)<1e-7);
 std::cout<<"injected final storage "<<injected.state(0)<<"\n";
}
