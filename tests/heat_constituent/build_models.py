#!/usr/bin/env python3
"""Build the test models for the Heat (temperature) constituent and the
Surface heat exchange source (resources/mass_transfer.json).

Heat is carried as heat content rho*c_p*Storage*T (MJ) with rho*c_p = 4.186
MJ/m3/C; the block "concentration" is the temperature T (C).

Cases (Reactor blocks joined by Fixed flow links):
  mixing      Tank 1000 m3, 200 m3/d inflow at 25 C plus a 300 m3/d river at
              10 C, starts at 10 C: T -> 16 C with tau = V/Q = 2 d.
  heater      Tank 1000 m3, 500 m3/d inflow at 10 C, 200 kW heater:
              T -> 10 + H/(rho*c_p*Q) = 18.256 C with tau = 2 d.
  surface     Closed tank 1000 m3 at 10 C, air at 25 C, K = 20 W/m2/C,
              A = 500 m2: T = 25 - 15*exp(-k*t), k = K*A/(rho*c_p*V).
  timeseries  As surface, but the air temperature comes from a time series
              (constant 25 C) and a 50 kW heater switches on at day 10 through
              the time-variable heat input: after day 10 T relaxes with the
              same k toward 25 + H/(K*A) = 30 C.

Usage: python build_models.py RESOURCES_DIR   (writes generated/*.ohq)
"""
import os, sys

RES = sys.argv[1].replace("\\", "/").rstrip("/") + "/"
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "generated")
os.makedirs(OUT, exist_ok=True)

HEAT = ("create constituent;type=Heat,name=Temperature,volumetric_heat_capacity=4.186,"
        "thermal_diffusivity=0,dispersivity=0,stoichiometric_constant=0,concentration=0,"
        "constant_inflow_temperature=0,time_variable_inflow_temperature=,constant_heat_input=0,"
        "time_variable_heat_input=,external_source=")


def header(name, tend):
    return [f"loadtemplate; filename = {RES}main_components.json",
            f"addtemplate; filename = {RES}mass_transfer.json",
            "setvalue; object=system, quantity=simulation_start_time, value=0",
            f"setvalue; object=system, quantity=simulation_end_time, value={tend}",
            "setvalue; object=system, quantity=initial_time_step, value=0.01",
            "setvalue; object=system, quantity=max_timestep_increase_factor, value=1",
            f"setvalue; object=system, quantity=alloutputfile, value={name}_output.txt"]


def reactor(name, V, Qin, T0, Tin=0, H=0, src="", x=0, tv_heat=""):
    return (f"create block;type=Reactor,name={name},Storage={V},constant_inflow={Qin},time_variable_inflow=,"
            f"bottom_elevation=0,_height=200,_width=200,x={x},y=900,"
            f"Temperature:concentration={T0},Temperature:constant_inflow_temperature={Tin},"
            f"Temperature:time_variable_inflow_temperature=,Temperature:constant_heat_input={H},"
            f"Temperature:time_variable_heat_input={tv_heat},Temperature:external_source={src}")


def link(a, b, Q):
    return f"create link;from={a},to={b},type=Fixed flow,flow={Q},name={a} - {b}"


def air(tconst, tseries):
    return ("create source;type=surface_heat_exchange,name=Air,surface_area=500,"
            f"heat_exchange_coefficient=20,air_temperature={tconst},time_variable_air_temperature={tseries}")


MODELS = {
    "mixing": (20, [HEAT, reactor("River", 1e6, 0, 10, x=400),
                    reactor("Tank", 1000, 200, 10, Tin=25, x=1000),
                    reactor("Outlet", 1e6, 0, 0, x=1600),
                    link("River", "Tank", 300), link("Tank", "Outlet", 500)]),
    "heater": (20, [HEAT, reactor("Tank", 1000, 500, 10, Tin=10, H=200, x=1000),
                    reactor("Outlet", 1e6, 0, 0, x=1600), link("Tank", "Outlet", 500)]),
    "surface": (30, [HEAT, air(25, ""), reactor("Tank", 1000, 0, 10, src="Air", x=1000)]),
    "timeseries": (30, [HEAT, air(0, "air25.txt"),
                        reactor("Tank", 1000, 0, 10, src="Air", x=1000, tv_heat="heat_step.txt")]),
}

if __name__ == "__main__":
    with open(os.path.join(OUT, "air25.txt"), "w") as f:
        f.write("0,25\n100,25\n")
    with open(os.path.join(OUT, "heat_step.txt"), "w") as f:
        f.write("0,0\n9.9999,0\n10,50\n100,50\n")
    for name, (tend, body) in MODELS.items():
        with open(os.path.join(OUT, name + ".ohq"), "w", encoding="utf8") as f:
            f.write("\n".join(header(name, tend) + body) + "\n")
    print("wrote", ", ".join(MODELS))
