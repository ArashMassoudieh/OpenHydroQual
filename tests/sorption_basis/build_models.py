#!/usr/bin/env python3
"""Build the batch test models for the immobile / sorbed constituent templates
(resources/mass_transfer.json).

One saturated Groundwater cell, no flow: area 1 m2, depth 1 m, porosity 0.4,
so the water volume (Storage) is 0.4 m3 and the total volume is 1 m3.
At t = 0 the water holds C0 = 10 g/m3 and nothing is sorbed. Linear sorption
with rho_b = 1600 kg/m3, K_d = 5e-4 m3/kg (rho_b*K_d = 0.8, rho_b*K_d/n = 2),
so the retardation factor is R = 1 + rho_b*K_d/n = 3 and the equilibrium
dissolved concentration is C0/R = 3.333 g/m3 in every correctly built case.

Cases (k_des = 1 /day in all):
  immobile_chem  Immobile_Constituent (per water volume) + Chemical reactions,
                 k_ads = k_des*rho_b*K_d/n
  sorbed_chem    Sorbed_Constituent (per total volume) + Chemical reactions,
                 k_ads = k_des*rho_b*K_d
  sorbed_desorp  Sorbed_Constituent + Desorption reactions (per total volume),
                 k_ads = k_des*rho_b*K_d
  porosity_stoich  as sorbed_chem but with the sorbed stoichiometric constant
                 n/rho_b instead of 1 (the form used in
                 Examples/LM_Calibration/01_Batch_Sorption); expected to
                 violate the mass balance.

Usage: python build_models.py RESOURCES_DIR   (writes generated/*.ohq)
"""
import os, sys

RES = sys.argv[1].replace("\\", "/").rstrip("/") + "/"
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "generated")
os.makedirs(OUT, exist_ok=True)
N, RHO, KD, KDES, C0 = 0.4, 1600.0, 5e-4, 1.0, 10.0

def model(name, sorbed_type, rxn_type, k_ads, sorbed_stoich=1.0):
    L = [f"loadtemplate; filename = {RES}main_components.json",
         f"addtemplate; filename = {RES}main_components.json",
         f"addtemplate; filename = {RES}groundwater.json",
         f"addtemplate; filename = {RES}mass_transfer.json",
         "setvalue; object=system, quantity=simulation_start_time, value=0",
         "setvalue; object=system, quantity=simulation_end_time, value=10",
         "setvalue; object=system, quantity=initial_time_step, value=0.001",
         "setvalue; object=system, quantity=max_timestep_increase_factor, value=1",
         f"setvalue; object=system, quantity=alloutputfile, value={name}_output.txt",
         "create constituent;type=Constituent,concentration=0,constant_inflow_concentration=0,diffusion_coefficient=0,dispersivity=0,"
         "external_mass_flow_timeseries=,external_source=,name=Cw,stoichiometric_constant=0,time_variable_inflow_concentration=",
         f"create constituent;type={sorbed_type},concentration=0,external_mass_flow_timeseries=,external_source=,name=Cs,stoichiometric_constant=0"]
    for nm, v in (("k_ads", k_ads), ("k_des", KDES)):
        L.append(f"create reaction_parameter;type=ReactionParameter,Arrhenius_factor=1,base_value={v},name={nm},reference_temperature=0,temperature=")
    L.append(f"create reaction;type={rxn_type},Cw:stoichiometric_constant=-1,Cs:stoichiometric_constant={sorbed_stoich},name=Adsorption,rate_expression=(k_ads*Cw)")
    L.append(f"create reaction;type={rxn_type},Cw:stoichiometric_constant=1,Cs:stoichiometric_constant={-sorbed_stoich},name=Desorption_rxn,rate_expression=(k_des*Cs)")
    L.append(f"create block;type=Groundwater cell,Cw:concentration={C0},Cw:constant_inflow_concentration=0,Cw:external_mass_flow_timeseries=,"
             "Cw:external_source=,Cw:time_variable_inflow_concentration=,Cs:concentration=0,Cs:external_mass_flow_timeseries=,Cs:external_source=,"
             f"area=1,bottom_elevation=0,depth=1,hydraulic_conductivity=1,inflow=,moisture_content={N},porosity={N},specific_storage=0.0001,name=Cell,x=0,y=0")
    open(os.path.join(OUT, name + ".ohq"), "w", encoding="utf-8").write("\n".join(L) + "\n")

model("immobile_chem", "Immobile_Constituent", "Reaction", KDES * RHO * KD / N)
model("sorbed_chem", "Sorbed_Constituent", "Reaction", KDES * RHO * KD)
model("sorbed_desorp", "Sorbed_Constituent", "Desorption", KDES * RHO * KD)
model("porosity_stoich", "Sorbed_Constituent", "Reaction", KDES * RHO * KD, sorbed_stoich=N / RHO)
print("wrote", sorted(os.listdir(OUT)))
