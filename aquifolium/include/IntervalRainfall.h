#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

// Native bins contain total depth, not a rate. Adjacent representable knots
// retain a step in the existing TimeSeries/UI representation without duplicate
// timestamps. The transition's width is at most one floating-point ULP.
inline std::vector<std::pair<double,double>> intervalRainfallKnots(
    const std::vector<double>& start, const std::vector<double>& end,
    const std::vector<double>& depth, double area)
{
    if(start.size()!=end.size() || start.size()!=depth.size() || !std::isfinite(area) || area<0)
        throw std::invalid_argument("Invalid rainfall bin arrays/area");
    std::map<double,double> knots;
    for(size_t k=0;k<start.size();++k){
        double width=end[k]-start[k];
        if(!std::isfinite(start[k]) || !std::isfinite(end[k]) || !std::isfinite(depth[k])
           || !std::isfinite(width) || width<=0 || depth[k]<0 || (k && start[k]<end[k-1]))
            throw std::invalid_argument("Rainfall bins must be finite, ordered, nonoverlapping and nonnegative");
        double rate=depth[k]/width*area;
        if(!std::isfinite(rate))throw std::invalid_argument("Rainfall rate overflow");
        // The previous bin already supplies its own last interior knot.
        if(k==0 || start[k]>end[k-1]){
            double before=start[k]-std::max(1.0,std::abs(start[k]))*std::numeric_limits<double>::epsilon();
            if(!std::isfinite(before))throw std::invalid_argument("Rainfall start outside representable domain");
            knots[before]=0;
        }
        knots[start[k]]=rate;
        knots[std::nextafter(end[k],start[k])]=rate;
        knots[end[k]]=0;
    }
    return {knots.begin(),knots.end()};
}

// Integrate the rate representation over [t,t+dt], with zero outside support.
// Binary-search once, then visit only knots crossed by this solver step.
// Series elements expose .t and .c (the engine TimeSeries interface).
template<class Series>
inline double intervalRainfallAverage(const Series& series,double t,double dt)
{
    if(!std::isfinite(t) || !std::isfinite(dt) || dt<=0 || !std::isfinite(t+dt) || t+dt<=t)
        throw std::invalid_argument("Invalid rainfall evaluation step");
    if(series.size()<2)return 0;
    const double stop=std::min(t+dt,double(series[series.size()-1].t));
    double x=std::max(t,double(series[0].t));
    if(x>=stop)return 0;
    size_t lo=0,hi=series.size();
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(series[mid].t<=x)lo=mid+1;else hi=mid;}
    size_t j=lo?lo-1:0;
    long double integral=0;
    while(x<stop && j+1<series.size()){
        const auto& a=series[j];const auto& b=series[j+1];
        const double y=std::min(stop,double(b.t));
        const double f0=(x-a.t)/(b.t-a.t),f1=(y-a.t)/(b.t-a.t);
        const double v0=a.c+(b.c-a.c)*f0,v1=a.c+(b.c-a.c)*f1;
        integral+=(static_cast<long double>(v0)+v1)*.5L*(y-x);
        x=y;++j;
    }
    double result=double(integral/dt);
    if(!std::isfinite(result))throw std::invalid_argument("Rainfall step integral overflow");
    return result;
}
