"""
Parity test for the OHQ stepping API (begin_stepping / advance_to / end_stepping,
set_timeseries, get_value, objective_integral).

A reservoir with a constant inflow, a controlled withdrawal (User_flow link,
whose 'flow' property is a time series) and a spillway. The withdrawal is
piecewise constant over N intervals. Run A: the whole control series is set
up front and the model is solved in one call. Run B: the model is stepped
interval by interval and the control for each interval is written just
before it is simulated. Final storage and the integrated objective (spilled
volume) must agree.

    python -m ohqrl.tests.test_stepping       (from the OpenHydroQual folder)
"""
import os
import sys
import tempfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from ohqrl.env import _import_ohq, OHQ_RESOURCES

N, T, DT = 10, 10.0, 0.05            # intervals, days, solver step
U = np.array([200, 800, 50, 1200, 0, 600, 900, 100, 400, 700], float)   # m3/day


def script(workdir, inflow_file, control_file):
    R = OHQ_RESOURCES
    return "\n".join([
        f"loadtemplate; filename = {R}main_components.json",
        f"addtemplate; filename = {R}Reservoir_operation.json",
        "setvalue; object=system, quantity=simulation_start_time, value=0",
        f"setvalue; object=system, quantity=simulation_end_time, value={T}",
        f"setvalue; object=system, quantity=initial_time_step, value={DT}",
        "setvalue; object=system, quantity=max_timestep_increase_factor, value=1",
        "setvalue; object=system, quantity=n_threads, value=1",
        f"setvalue; object=system, quantity=alloutputfile, value={workdir}/out.txt",
        f"create block;type=Reservoir,Storage=3000,inflow={inflow_file},name=Res",
        "create block;type=fixed_head,Storage=0,head=0,name=User",
        "create block;type=fixed_head,Storage=0,head=0,name=Spill",
        f"create link;from=Res,to=User,type=User_flow,flow={control_file},name=withdrawal",
        "create link;from=Res,to=Spill,type=Reservoir_link_rule,Q_min=0,Q_max=20000,"
        "S_min=5000,S_max=5250,name=spillway",
        "create objectivefunction;type=Objective_Function,name=Spilled,object=spillway,"
        "expression=flow,method=integrate,weight=1",
        ""])


def build(ohq, workdir, control):
    inflow = os.path.join(workdir, "inflow.txt")
    np.savetxt(inflow, [[0, 900], [T + 1, 900]], fmt="%.6f", delimiter=", ")
    ctrl = os.path.join(workdir, "control.txt")
    np.savetxt(ctrl, control, fmt="%.10f", delimiter=", ")
    sf = os.path.join(workdir, "model.ohq")
    open(sf, "w").write(script(workdir, inflow, ctrl))
    s = ohq.System()
    s.set_default_template_path(OHQ_RESOURCES)
    s.set_working_folder(workdir + "/")
    s.set_silent(True)
    assert s.create_from_script(sf, OHQ_RESOURCES + "settings.json")
    return s


def piecewise(u):
    """Step series: value u[k] on [k, k+1), as closely spaced points."""
    pts = []
    for k, v in enumerate(u):
        pts += [[k, v], [k + 1 - 1e-9, v]]
    pts.append([len(u) + 1, u[-1]])
    return np.array(pts)


def main():
    ohq = _import_ohq()
    wa, wb = tempfile.mkdtemp(), tempfile.mkdtemp()

    a = build(ohq, wa, piecewise(U))
    assert a.solve(True, False)
    S_a = a.get_value("Res", "Storage")
    J_a, Jint_a = a.objective_value("Spilled"), a.objective_integral("Spilled")

    b = build(ohq, wb, piecewise(np.zeros(N)))
    assert b.begin_stepping(True)
    for k in range(N):
        assert b.set_timeseries("withdrawal", "flow", [k, k + 1 - 1e-9, T + 1], [U[k], U[k], U[k]])
        assert b.advance_to(k + 1.0), f"advance failed at interval {k}"
        assert abs(b.current_time() - (k + 1)) < 1e-9
    S_b = b.get_value("Res", "Storage")
    Jint_b = b.objective_integral("Spilled")
    b.end_stepping(False)

    # Mass balance of run B: S0 + inflow - withdrawal - spill
    S_expect = 3000 + 900 * T - U.sum() - Jint_b
    print(f"storage    full {S_a:.6f}  stepped {S_b:.6f}  balance {S_expect:.6f}")
    print(f"spilled    full {Jint_a:.6f}  stepped {Jint_b:.6f}  (OHQ objective, full run: {J_a:.6f})")
    assert abs(S_a - S_b) < 1e-6 * max(1, abs(S_a)), "stepped storage differs from full solve"
    assert abs(Jint_a - Jint_b) < 1e-6 * max(1, abs(Jint_a)), "stepped objective differs"
    assert abs(S_b - S_expect) < 1e-3 * max(1, abs(S_b)), "mass balance"

    # A value-type property changed mid-run with set_value takes effect at once:
    # close the spillway (Q_max = 0) halfway; spilling must stop from then on.
    c = build(ohq, tempfile.mkdtemp(), piecewise(np.zeros(N)))
    assert c.begin_stepping(True)
    assert c.advance_to(T / 2)
    spilled_half = c.objective_integral("Spilled")
    assert c.set_value("spillway", "Q_max", 0.0)
    assert c.advance_to(T)
    print(f"spill with spillway closed at T/2: {spilled_half:.3f} -> {c.objective_integral('Spilled'):.3f}")
    assert spilled_half > 0 and abs(c.objective_integral("Spilled") - spilled_half) < 1e-6
    c.end_stepping(False)

    limited_outflow()
    print("OK")


def limited_outflow():
    """A withdrawal (5000 m3/day) larger than the inflow (900 m3/day) empties the
    reservoir; from then on the solver's outflow limiter lets only the inflow
    through. get_value must return that limited flow, not the raw 5000, and an
    objective on an expression built on the flow (a constituent's
    advective_masstransfer = _ups(flow;concentration)) must integrate the limited
    flow too: it must close the tracer mass balance."""
    ohq = _import_ohq()
    w = tempfile.mkdtemp()
    R, Cin, Q = OHQ_RESOURCES, 2.0, 5000.0
    inflow, ctrl = os.path.join(w, "inflow.txt"), os.path.join(w, "control.txt")
    np.savetxt(inflow, [[0, 900], [T + 1, 900]], fmt="%.6f", delimiter=", ")
    np.savetxt(ctrl, [[0, Q], [T + 1, Q]], fmt="%.6f", delimiter=", ")
    sf = os.path.join(w, "model.ohq")
    open(sf, "w").write("\n".join([
        f"loadtemplate; filename = {R}main_components.json",
        f"addtemplate; filename = {R}Reservoir_operation.json",
        "setvalue; object=system, quantity=simulation_start_time, value=0",
        f"setvalue; object=system, quantity=simulation_end_time, value={T}",
        f"setvalue; object=system, quantity=initial_time_step, value={DT}",
        "setvalue; object=system, quantity=n_threads, value=1",
        "create constituent;type=Constituent,name=C,diffusion_coefficient=0,dispersivity=0",
        f"create block;type=Reservoir,Storage=100,inflow={inflow},C:constant_inflow_concentration={Cin},name=Res",
        "create block;type=fixed_head,Storage=0,head=0,name=User",
        f"create link;from=Res,to=User,type=User_flow,flow={ctrl},C:exchange_velocity=0,name=withdrawal",
        "create objectivefunction;type=Objective_Function,name=Withdrawn,object=withdrawal,"
        "expression=flow,method=integrate,weight=1",
        "create objectivefunction;type=Objective_Function,name=Load,object=withdrawal,"
        "expression=C:advective_masstransfer,method=integrate,weight=1",
        ""]))
    s = ohq.System()
    s.set_default_template_path(R)
    s.set_working_folder(w + "/")
    s.set_silent(True)
    assert s.create_from_script(sf, R + "settings.json")
    assert s.begin_stepping(True)
    for k in range(N):
        assert s.advance_to(k + 1.0)
    q = s.get_value("withdrawal", "flow")
    S, M = s.get_value("Res", "Storage"), s.get_value("Res", "C:mass")
    W, L = s.objective_integral("Withdrawn"), s.objective_integral("Load")
    s.end_stepping(False)
    print(f"limited outflow: flow {q:.3f} (raw {Q:.0f}), withdrawn {W:.3f} vs {100 + 900 * T - S:.3f}, "
          f"tracer load {L:.3f} vs {Cin * 900 * T - M:.3f}")
    assert abs(q - 900) < 0.01 * 900, "get_value returned the unlimited flow"
    assert abs(W - (100 + 900 * T - S)) < 0.01 * W, "water balance"
    assert abs(L - (Cin * 900 * T - M)) < 0.01 * L, "objective on advective_masstransfer used the unlimited flow"


if __name__ == "__main__":
    main()
