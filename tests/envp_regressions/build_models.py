#!/usr/bin/env python3
"""Small models for template regressions found while building the CEE 466/566
course models (see run_test.sh):

  asr_particle   Activated_Sludge_Reactor with a Particle constituent, drained
                 into a fixed_head boundary (needs inflow, bottom_elevation)
  fixed_head     Reactor -> fixed_head boundary with a plain Constituent
  sorbed_reactor Reactor with a dissolved and a Sorbed constituent exchanging
                 mass by a first-order reaction (needs volume)

Usage: build_models.py <resources-folder>
"""
import os, sys

RES = sys.argv[1].replace("\\", "/").rstrip("/") + "/"
HERE = os.path.dirname(os.path.abspath(__file__))
G = os.path.join(HERE, "generated")
os.makedirs(G, exist_ok=True)

SETTINGS = """setvalue; object=system, quantity=simulation_start_time, value=0
setvalue; object=system, quantity=simulation_end_time, value={tend}
setvalue; object=system, quantity=initial_time_step, value=0.01
setvalue; object=system, quantity=max_timestep_increase_factor, value=1
setvalue; object=system, quantity=minimum_timestep, value=1e-06
setvalue; object=system, quantity=alloutputfile, value={name}_output.txt
setvalue; object=system, quantity=observed_outputfile, value={name}_observed.txt
setvalue; object=system, quantity=write_solution_details, value=No
"""


def header(name, tend, extra=()):
    s = f"loadtemplate; filename = {RES}main_components.json\n"
    s += f"addtemplate; filename = {RES}mass_transfer.json\n"
    for e in extra:
        s += f"addtemplate; filename = {RES}{e}\n"
    return s + SETTINGS.format(name=name, tend=tend)


MODELS = {}

# 1. Activated sludge reactor, Particle constituent, fixed-head outlet.
MODELS["asr_particle"] = header("asr_particle", 5, ["wastewater.json"]) + """create constituent;type=Particle,concentration=0,constant_inflow_concentration=0,external_mass_flow_timeseries=,external_source=,name=X,settling_velocity=0,stoichiometric_constant=0,time_variable_inflow_concentration=,turbidity_coefficient=0,turbidity_exponent=0
create block;type=Activated_Sludge_Reactor,X:concentration=0,X:constant_inflow_concentration=100,X:external_mass_flow_timeseries=,X:external_source=,X:time_variable_inflow_concentration=,Storage=1000,bottom_elevation=0,constant_inflow=500,time_variable_inflow=,name=Aeration,x=1000,y=900
create block;type=fixed_head,X:concentration=0,X:constant_inflow_concentration=0,X:external_mass_flow_timeseries=,X:external_source=,X:time_variable_inflow_concentration=,Storage=100000,head=0,name=Outfall,x=1600,y=900
create link;from=Aeration,to=Outfall,type=Fixed flow,flow=500,name=Aeration - Outfall
"""

# 2. Plain Reactor draining into a fixed-head boundary.
MODELS["fixed_head"] = header("fixed_head", 5) + """create constituent;type=Constituent,concentration=0,constant_inflow_concentration=0,diffusion_coefficient=0,dispersivity=0,external_mass_flow_timeseries=,external_source=,name=C,stoichiometric_constant=0,time_variable_inflow_concentration=
create block;type=Reactor,C:concentration=0,C:constant_inflow_concentration=100,C:external_mass_flow_timeseries=,C:external_source=,C:time_variable_inflow_concentration=,Storage=1000,bottom_elevation=0,constant_inflow=500,time_variable_inflow=,name=Tank,x=1000,y=900
create block;type=fixed_head,C:concentration=0,C:constant_inflow_concentration=0,C:external_mass_flow_timeseries=,C:external_source=,C:time_variable_inflow_concentration=,Storage=100000,head=0,name=Outfall,x=1600,y=900
create link;from=Tank,to=Outfall,type=Fixed flow,flow=500,name=Tank - Outfall
"""

# 3. Batch Reactor: dissolved D -> sorbed S, first order, k = 0.5 /day.
MODELS["sorbed_reactor"] = header("sorbed_reactor", 10) + """create constituent;type=Constituent,concentration=0,constant_inflow_concentration=0,diffusion_coefficient=0,dispersivity=0,external_mass_flow_timeseries=,external_source=,name=D,stoichiometric_constant=-1,time_variable_inflow_concentration=
create constituent;type=Sorbed_Constituent,concentration=0,external_mass_flow_timeseries=,external_source=,name=S,stoichiometric_constant=1
create reaction_parameter;type=ReactionParameter,Arrhenius_factor=1,base_value=0.5,name=k,reference_temperature=20,temperature=
create reaction;type=Reaction,D:stoichiometric_constant=-1,S:stoichiometric_constant=1,name=sorption,rate_expression=(k*D)
create block;type=Reactor,D:concentration=100,D:constant_inflow_concentration=0,D:external_mass_flow_timeseries=,D:external_source=,D:time_variable_inflow_concentration=,S:concentration=0,S:external_mass_flow_timeseries=,S:external_source=,Storage=10,bottom_elevation=0,constant_inflow=0,time_variable_inflow=,name=Batch,x=1000,y=900
"""

for name, text in MODELS.items():
    open(os.path.join(G, name + ".ohq"), "w", encoding="utf8").write(text)
print("wrote", ", ".join(MODELS))
