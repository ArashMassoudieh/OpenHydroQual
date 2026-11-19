// Generate AdaptiveRainModel from tests/fixtures/interval_rainfall_adaptive.ohq.
// Compile with -I<generated> -I<generated>/runtime. No OHQ library dependency.
#include "AdaptiveRainModel.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
int main(){
 AdaptiveRainModel m;m.initialize();unsigned steps=0;double worst=0;
 while(m.time()<.1-1e-12){
  if(!m.stepTo(.1))return 2;
  double expected=10+100*std::max(0.,std::min(.02,m.time()-.02));
  double error=std::abs(m.state(0)-expected);worst=std::max(worst,error);++steps;
  if(!std::isfinite(m.state(0)) || error>1e-7){
   std::cout<<std::setprecision(17)<<"FAIL t="<<m.time()<<" storage="<<m.state(0)<<" expected="<<expected<<"\n";return 1;
  }
 }
 if(steps<2 || std::abs(m.time()-.1)>1e-12)return 3;
 std::cout<<std::setprecision(17)<<"PASS adaptive steps="<<steps<<" max_storage_error="<<worst<<"\n";
}
