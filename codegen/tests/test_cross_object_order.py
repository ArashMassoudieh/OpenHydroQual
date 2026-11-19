#!/usr/bin/env python3
"""Generate/compile/run block<-link velocity dependencies with/without outputs.

Usage: python3 test_cross_object_order.py GENERATOR RESOURCES NEW_OUTPUT
All inputs are synthetic; no fitting, observations or repository output writes.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess


def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def run(generator,resources,output):
    generator,resources,output=(Path(p).resolve() for p in (generator,resources,output))
    output.mkdir(exist_ok=False,parents=True)
    receipt={'generator_sha256':sha(generator),'cases':[]}
    for cached in (False,True):
        folder=output/('cached_outputs' if cached else 'locals_only');folder.mkdir()
        model=folder/'model.ohq';model.write_text(f'''loadtemplate;filename={resources}/main_components.json
addtemplate;filename={resources}/open_channel.json
create block;type=Trapezoidal Channel Segment,name=A,base_width=3,side_slope=2,ManningCoeff=.035,bottom_elevation=.5,length=100,depth=.05,inflow=,ag_area=0,non_ag_area=0,dam_height=0
create block;type=Trapezoidal Channel Segment,name=B,base_width=3,side_slope=2,ManningCoeff=.035,bottom_elevation=.3,length=100,depth=.05,inflow=,ag_area=0,non_ag_area=0,dam_height=0
create link;from=A,to=B,type=Trapezoidal_Channel_link,name=AB
setvalue;object=system,quantity=simulation_start_time,value=0
setvalue;object=system,quantity=simulation_end_time,value=.01
setvalue;object=system,quantity=initial_time_step,value=.001
setvalue;object=system,quantity=nr_block_tolerance,value=1e-8
''')
        generated=folder/'generated'
        command=[str(generator),str(model),str(resources),str(generated),'OrderModel','--project','lib']
        if cached:
            selected=folder/'outputs.txt';selected.write_text('* velocity\n');command+=['--outputs',str(selected)]
        record={'cached_outputs':cached,'commands':[command]}
        with (folder/'generation.log').open('w') as log:
            rc=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=15).returncode
        record['generation_returncode']=rc
        if rc==0:
            cpp=folder/'main.cpp'
            cpp.write_text('''#include "OrderModel.h"
#include <cmath>
#include <cstring>
#include <iostream>
int main(){
 OrderModel m;m.initialize();double initial=m.state(0)+m.state(1);
 if(!m.runTo(.01)||m.solutionFailed())return 1;
 if(std::abs(m.state(0)+m.state(1)-initial)>1e-4)return 2;
 if(m.state(0)<0||m.state(1)<0)return 3;
#if CHECK_OUTPUTS
 double values[OrderModel::N_OUTPUTS];m.computeOutputs(values);
 double a=NAN,b=NAN,link=NAN;
 for(int i=0;i<OrderModel::N_OUTPUTS;++i){
  if(!std::strcmp(m.outputName(i),"A:velocity"))a=values[i];
  if(!std::strcmp(m.outputName(i),"B:velocity"))b=values[i];
  if(!std::strcmp(m.outputName(i),"AB:velocity"))link=values[i];
 }
 if(!std::isfinite(a)||!std::isfinite(b)||!std::isfinite(link)||std::abs(link)<1e-5)return 4;
 if(std::abs(a-std::abs(link))>1e-12||std::abs(b-std::abs(link))>1e-12){
  std::cerr<<"stale block/link velocities: "<<a<<","<<b<<","<<link<<"\\n";return 5;
 }
#endif
 std::cout<<"PASS cross-object dependency order\\n";
}
''')
            command=[os.environ.get('CXX','g++'),'-std=c++17','-O2','-DCHECK_OUTPUTS='+str(int(cached)),
                     '-I'+str(generated),'-I'+str(generated/'runtime'),str(cpp),'-o',str(folder/'check')]
            record['commands'].append(command)
            with (folder/'compile.log').open('w') as log:rc=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=30).returncode
            record['compile_returncode']=rc
            if rc==0:
                with (folder/'check.log').open('w') as log:rc=subprocess.run([str(folder/'check')],stdout=log,stderr=subprocess.STDOUT,timeout=15).returncode
                record['run_returncode']=rc;record['binary_sha256']=sha(folder/'check')
        receipt['cases'].append(record)
        (output/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
        print('cached' if cached else 'locals',record,flush=True)
    return all(c.get('run_returncode')==0 for c in receipt['cases'])


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('generator','resources','new_output'):p.add_argument(name,type=Path)
    a=p.parse_args();raise SystemExit(0 if run(a.generator,a.resources,a.new_output) else 1)
