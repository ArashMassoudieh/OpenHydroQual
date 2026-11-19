#include "IntervalRainfall.h"
#include "ohq_timeseries.h"
#include <cassert>
#include <iostream>
struct Point{double t,c;};
std::vector<Point> series(const std::vector<double>& a,const std::vector<double>& b,const std::vector<double>& d){
 std::vector<Point> p;for(auto x:intervalRainfallKnots(a,b,d,1))p.push_back({x.first,x.second});return p;
}
int main(){
 std::vector<double>a{0,.02,.04},b{.02,.04,.1},d{0,.002,0};
 auto p=series(a,b,d);ohq::TimeSeries k;assert(k.setPrecipitation(a.data(),b.data(),d.data(),3));
 for(double step:{.001,.007,.03,.13}){
  double total=0;for(double t=-.01;t<.11;t+=step){
   double x=intervalRainfallAverage(p,t,step),y=k.average(t,step);
   assert(std::abs(x-y)<1e-13);total+=x*step;
  }
  assert(std::abs(total-.002)<1e-14);
 }
 assert(intervalRainfallAverage(p,.005,.005)==0);
 assert(std::abs(intervalRainfallAverage(p,.025,.005)-.1)<1e-13);
 assert(intervalRainfallAverage(p,.05,.005)==0);
 assert(std::abs(intervalRainfallAverage(p,.015,.01)-.05)<1e-13);
 a={0,.04};b={.01,.06};d={.001,.002};p=series(a,b,d);
 assert(intervalRainfallAverage(p,.02,.01)==0);
 assert(std::abs(intervalRainfallAverage(p,-.01,.1)*.1-.003)<1e-14);
 for(int bad=0;bad<4;++bad){
  auto aa=a,bb=b,dd=d;
  if(bad==0)aa[1]=.005;if(bad==1)dd[0]=-1;if(bad==2)bb[0]=0;if(bad==3)dd[0]=NAN;
  bool threw=false;try{series(aa,bb,dd);}catch(const std::invalid_argument&){threw=true;}
  assert(threw);assert(!k.setPrecipitation(aa.data(),bb.data(),dd.data(),2));
 }
 // Single interval, no extrapolated rain; ordinary series interpolation unchanged.
 p=series({1},{2},{.01});assert(std::abs(intervalRainfallAverage(p,0,3)*3-.01)<1e-14);
 ohq::TimeSeries ordinary;ordinary.push(0,0);ordinary.push(1,2);assert(ordinary.interpol(.5)==1);
 std::cout<<"Interval rainfall analytical and runtime parity tests passed\n";
}
