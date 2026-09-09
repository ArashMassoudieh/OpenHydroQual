#!/usr/bin/env python3
"""Generates the culvert example set: inflow/boundary time series and .ohq models.
Re-run after editing to regenerate every input file."""
import math

# ---------------- time series ----------------
def write_ts(path, fn, tend, dt=0.005, comment="time (day), value"):
    with open(path,'w') as f:
        f.write(f"// {comment}\n")
        t=0.0
        while t<=tend+1e-9:
            f.write(f"{t:.5f}, {fn(t):.4f}\n"); t+=dt

def storm(peak, tpeak=0.5, width=0.15, base=0.02):
    return lambda t: (base+peak*math.exp(-((t-tpeak)/width)**2))*86400

# ---------------- ohq building blocks ----------------
HEAD = """loadtemplate; filename = main_components.json
addtemplate; filename = open_channel.json
addtemplate; filename = culvert.json
"""
def settings(end, out, dt0=0.0005, wi=20, tmax=900):
    return f"""setvalue; object=system, quantity=simulation_start_time, value=0
setvalue; object=system, quantity=simulation_end_time, value={end}
setvalue; object=system, quantity=alloutputfile, value={out}
setvalue; object=system, quantity=nr_tolerance, value=0.001
setvalue; object=system, quantity=minimum_timestep, value=1e-07
setvalue; object=system, quantity=initial_time_step, value={dt0}
setvalue; object=system, quantity=maximum_time_allowed, value={tmax}
setvalue; object=system, quantity=c_n_weight, value=1
setvalue; object=system, quantity=write_interval, value={wi}
"""
def channel(name,bot,inflow="",depth=0.05,bw=3,ss=2,n=0.035,L=100,x=0,y=0):
    return (f"create block;type=Trapezoidal Channel Segment,name={name},base_width={bw},side_slope={ss},"
            f"ManningCoeff={n},bottom_elevation={bot},length={L},depth={depth},inflow={inflow},"
            f"ag_area=0,non_ag_area=0,dam_height=0,x={x},y={y},_width=200,_height=200\n")
def barrel_circ(name,D,L,zi,zo,N=1,n=0.013,slot=0.05,depth=0,x=300,y=0):
    return (f"create block;type=Culvert Barrel (Circular),name={name},Number_of_barrels={N},diameter={D},"
            f"length={L},inlet_invert={zi},outlet_invert={zo},ManningCoeff={n},inlet_crest_width_factor=0.65,"
            f"slot_width_ratio={slot},inflow=,depth={depth},x={x},y={y},_width=200,_height=200\n")
def barrel_box(name,B,H,L,zi,zo,N=1,n=0.013,slot=0.05,depth=0,x=300,y=0):
    return (f"create block;type=Culvert Barrel (Box),name={name},Number_of_barrels={N},barrel_width={B},"
            f"barrel_height={H},length={L},inlet_invert={zi},outlet_invert={zo},ManningCoeff={n},"
            f"slot_width_ratio={slot},inflow=,depth={depth},x={x},y={y},_width=200,_height=200\n")
# HDS-5 Table A.1 coefficient sets used by the examples.
HDS5_CIRC_SQ   = "HDS5_K=0.0098,HDS5_M=2.0,HDS5_c=0.0398,HDS5_Y=0.67,HDS5_form1_switch=1,slope_correction_coeff=0.5"
HDS5_BOX_WING  = "HDS5_K=0.026,HDS5_M=1.0,HDS5_c=0.0347,HDS5_Y=0.81,HDS5_form1_switch=1,slope_correction_coeff=0.5"
def inlet(name,frm,to,shape="Circular",ke=0.5,hds=HDS5_CIRC_SQ):
    return f"create link;from={frm},to={to},type=Channel_to_culvert ({shape}),name={name},entrance_loss_coeff={ke},{hds}\n"
def outlet(name,frm,to,kx=1.0,shape="Circular"):
    return (f"create link;from={frm},to={to},type=Culvert_to_channel ({shape}),name={name},"
            f"exit_loss_coeff={kx},hgl_rule_lower=0.75,hgl_rule_upper=1.8\n")
def fixed(name,head,x=900,y=0):
    return f"create block;type=fixed_head,name={name},head={head},area=1,Storage=100000,x={x},y={y},_width=200,_height=200\n"
def tvfixed(name,ts,x=900,y=0):
    return (f"create block;type=time_variable_fixed_head,name={name},head={ts},Dummy_timeseries=,"
            f"Storage=100000,x={x},y={y},_width=200,_height=200\n")
def c2f(name,frm,to): return f"create link;from={frm},to={to},type=channel2fixed,name={name}\n"
def f2c(name,frm,to): return f"create link;from={frm},to={to},type=Fixedhead_to_channel,name={name}\n"
def overtop(name,frm,to,crest,width,coef=1.7):
    return (f"create link;from={frm},to={to},type=Roadway_Overtopping,name={name},"
            f"road_crest_elevation={crest},road_width={width},road_weir_coeff={coef},road_submergence_factor=1\n")
def single_circ(name,frm,to,D,L,zi,zo,N=1,n=0.013,ke=0.5,kx=1.0,rc=100,rw=0):
    return (f"create link;from={frm},to={to},type=Circular Culvert,name={name},Number_of_barrels={N},"
            f"diameter={D},length={L},inlet_invert={zi},outlet_invert={zo},ManningCoeff={n},"
            f"entrance_loss_coeff={ke},{HDS5_CIRC_SQ},exit_loss_coeff={kx},"
            f"road_crest_elevation={rc},road_width={rw},road_weir_coeff=1.7,road_submergence_factor=1\n")

def std_train(inflow, zi=0.5, zo=0.3, D=1.0, L=20, N=1, tail=0.35, slot=0.05,
              us_bot=0.5, ds_bot=0.3, ke=0.5, kx=1.0, tailts=None):
    s  = channel("US_Channel",us_bot,inflow,x=0)
    s += barrel_circ("Barrel",D,L,zi,zo,N=N,slot=slot)
    s += channel("DS_Channel",ds_bot,x=600)
    s += (tvfixed("Outfall",tailts) if tailts else fixed("Outfall",tail))
    s += inlet("Inlet","US_Channel","Barrel",ke=ke)
    s += outlet("Outlet","Barrel","DS_Channel",kx=kx)
    s += c2f("Tailwater","DS_Channel","Outfall")
    return s

TESTS={}
# ---- T1 unsubmerged inlet control, free outfall -------------------------------
write_ts('inflow_moderate.csv', storm(0.80), 2.0)
TESTS['T1_inlet_control']=HEAD+settings(2,'T1_inlet_control_out.txt')+std_train('inflow_moderate.csv')

# ---- T2 submerged inlet, barrel surcharges ------------------------------------
write_ts('inflow_large.csv', storm(3.2), 2.0)
TESTS['T2_submerged_inlet']=HEAD+settings(2,'T2_submerged_inlet_out.txt')+std_train('inflow_large.csv')

# ---- T3 outlet control: tailwater rises and drowns the culvert -----------------
write_ts('tailwater_rise.csv', lambda t: 0.35 + 1.55*math.exp(-((t-0.55)/0.30)**2), 2.0)
TESTS['T3_outlet_control_tailwater']=(HEAD+settings(2,'T3_outlet_control_tailwater_out.txt')
    +std_train('inflow_moderate.csv',tailts='tailwater_rise.csv'))

# ---- T4 roadway overtopping ----------------------------------------------------
write_ts('inflow_extreme.csv', storm(9.0), 2.0)
TESTS['T4_roadway_overtopping']=(HEAD+settings(2,'T4_roadway_overtopping_out.txt')
    +std_train('inflow_extreme.csv')+overtop("Road","US_Channel","DS_Channel",crest=2.3,width=12))

# ---- T5 three-barrel box culvert ----------------------------------------------
s  = channel("US_Channel",0.5,'inflow_extreme.csv',x=0)
s += barrel_box("Barrel",1.5,1.2,20,0.5,0.3,N=3)
s += channel("DS_Channel",0.3,x=600)+fixed("Outfall",0.35)
s += inlet("Inlet","US_Channel","Barrel",shape="Box",hds=HDS5_BOX_WING)
s += outlet("Outlet","Barrel","DS_Channel",shape="Box")+c2f("Tailwater","DS_Channel","Outfall")
TESTS['T5_box_multibarrel']=HEAD+settings(2,'T5_box_multibarrel_out.txt')+s

# ---- T6 reverse flow driven by a downstream surge ------------------------------
write_ts('inflow_small.csv', storm(0.10,tpeak=0.3,width=0.10,base=0.01), 2.0)
write_ts('surge.csv', lambda t: 0.35 + 2.2*math.exp(-((t-1.0)/0.18)**2), 2.0)
# The surge arrives fast against a nearly empty barrel; the default starting step
# overshoots it and costs about 2.8% of the volume balance. A smaller first step
# and a tighter Newton tolerance bring the closure back in line with the others.
TESTS['T6_reverse_flow']=(HEAD+settings(2,'T6_reverse_flow_out.txt',dt0=0.00005,wi=2000)
    .replace('nr_tolerance, value=0.001','nr_tolerance, value=0.0001')
    +std_train('inflow_small.csv',tailts='surge.csv'))

# ---- T7 lumped single link vs storage-routed barrel, same inflow ---------------
s  = channel("US_A",0.5,'inflow_large.csv',x=0,y=0)+channel("DS_A",0.3,x=600,y=0)+fixed("Outfall_A",0.35,y=0)
s += single_circ("Culvert_A","US_A","DS_A",1.0,20,0.5,0.3)+c2f("Tail_A","DS_A","Outfall_A")
s += channel("US_B",0.5,'inflow_large.csv',x=0,y=400)+barrel_circ("Barrel_B",1.0,20,0.5,0.3,x=300,y=400)
s += channel("DS_B",0.3,x=600,y=400)+fixed("Outfall_B",0.35,y=400)
s += inlet("Inlet_B","US_B","Barrel_B")+outlet("Outlet_B","Barrel_B","DS_B")+c2f("Tail_B","DS_B","Outfall_B")
TESTS['T7_lumped_vs_routed']=HEAD+settings(2,'T7_lumped_vs_routed_out.txt')+s

# ---- T8 dry start, storm, full drain-down (mass balance) -----------------------
write_ts('inflow_pulse.csv', lambda t: (0.0 if t<0.2 else (1.2 if t<0.5 else 0.0))*86400, 3.0)
TESTS['T8_dry_to_dry']=(HEAD+settings(3,'T8_dry_to_dry_out.txt')
    +std_train('inflow_pulse.csv',tail=0.30))

# ---- T9 steep culvert (5 percent), strong inlet control ------------------------
s  = channel("US_Channel",1.0,'inflow_moderate.csv',x=0)
s += barrel_circ("Barrel",1.0,20,1.0,0.0)
s += channel("DS_Channel",0.0,x=600)+fixed("Outfall",0.05)
s += inlet("Inlet","US_Channel","Barrel")+outlet("Outlet","Barrel","DS_Channel")+c2f("Tailwater","DS_Channel","Outfall")
TESTS['T9_steep_slope']=HEAD+settings(2,'T9_steep_slope_out.txt')+s

# ---- T10 constituent transport through the culvert -----------------------------
write_ts('tracer_in.csv', lambda t: (50.0 if 0.30<=t<0.45 else 0.0), 3.0)
s  = ("create constituent;type=Constituent,name=Tracer,concentration=0,constant_inflow_concentration=0,"
      "diffusion_coefficient=0,dispersivity=5,external_mass_flow_timeseries=,external_source=,"
      "stoichiometric_constant=0,time_variable_inflow_concentration=\n")
s += channel("US_Channel",0.5,'inflow_moderate.csv',x=0).rstrip('\n')+",Tracer:time_variable_inflow_concentration=tracer_in.csv\n"
s += barrel_circ("Barrel",1.0,20,0.5,0.3)
s += channel("DS_Channel",0.3,x=600)
s += channel("Tail_Reach",0.25,depth=0.10,bw=50,L=500,x=900)
s += inlet("Inlet","US_Channel","Barrel")+outlet("Outlet","Barrel","DS_Channel")
s += "create link;from=DS_Channel,to=Tail_Reach,type=Trapezoidal_Channel_link,name=Tailwater\n"
TESTS['T10_water_quality']=HEAD+settings(3,'T10_water_quality_out.txt')+s

# ---- T11 outlet control with the HDS-5 (dc+D)/2 grade line rule active ---------
# A long, rough (corrugated metal) barrel on a flat grade puts the structure firmly
# in outlet control, which is the only regime where the (dc+D)/2 rule does anything.
s  = channel("US_Channel",0.5,'inflow_large.csv',x=0)
s += barrel_circ("Barrel",1.0,60,0.5,0.4,n=0.024)
s += channel("DS_Channel",0.4,x=600)+fixed("Outfall",0.45)
s += inlet("Inlet","US_Channel","Barrel")+outlet("Outlet","Barrel","DS_Channel")
s += c2f("Tailwater","DS_Channel","Outfall")
# As with T6, the default starting time step is too coarse for this configuration
# (0.62% volume error); a smaller first step brings closure to 0.004%.
TESTS['T11_outlet_control_hgl']=HEAD+settings(2,'T11_outlet_control_hgl_out.txt',dt0=0.00005,wi=200)+s

for name,body in TESTS.items():
    open(name+'.ohq','w').write(body)
print("wrote:", " ".join(sorted(TESTS)))
