#include "Precipitation.h"
#include "IntervalRainfall.h"
#include <cassert>
#include <fstream>
#include <filesystem>
#include <iostream>
int main(int argc,char**argv){
 if(argc!=2)return 2;std::filesystem::create_directories(argv[1]);
 struct Case{const char* name;const char* text;bool valid;};
 Case cases[]={
  {"lf","0,.02,0\n.02,.04,.002\n.04,.1,0\n",true},
  {"crlf","0,.02,0\r\n.02,.04,.002\r\n.04,.1,0\r\n",true},
  {"whitespace"," \t0 , .02 , 0\t\r\n.02,.04,.002\t\r\n.04,.1,0\r\n",true},
  {"comments","// native depth bins\r\n\r\n0,.02,0\r\n.02,.04,.002\r\n.04,.1,0\r\n",true},
  {"junk","0,.1,.002junk\r\n",false},
  {"empty_start",",.1,.002\r\n",false},
  {"negative","0,.1,-.002\r\n",false},
  {"nan","0,.1,nan\r\n",false},
  {"overlap","0,.02,.001\r\n.01,.04,.001\r\n",false},
  {"extra","0,.1,.002,extra\r\n",false}
 };
 for(const auto& c:cases){
  auto file=std::filesystem::path(argv[1])/(std::string(c.name)+".csv");
  {std::ofstream out(file,std::ios::binary);out<<c.text;}
  assert(CPrecipitation::isFileValid(file.string())==c.valid);
  if(c.valid){CPrecipitation p;p.getfromfile(file.string());auto table=p.getflow(1)[0];assert(std::abs(intervalRainfallAverage(table,0,.1)*.1-.002)<1e-14);}
 }
 std::cout<<"PASS 10 native precipitation format checks, including CRLF and padded tokens\n";
}
