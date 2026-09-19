#!/usr/bin/env python3
"""Checks that each culvert example actually exercises the regime it is named for,
and that volume is conserved across the structure."""
import numpy as np, sys, glob, os

def read(fn):
    with open(fn) as f: hdr=[h.strip() for h in f.readline().split(',')]
    d=np.genfromtxt(fn,delimiter=',',skip_header=1)
    return {hdr[i]:(d[:,i-1],d[:,i]) for i in range(1,len(hdr),2)}
def g(S,n): return S[n][1] if n in S else None
def itg(t,q): return (np.trapezoid if hasattr(np,'trapezoid') else np.trapz)(q,t)

def balance(S,t,inflow_cols,out_col,storage_cols):
    vin=sum(itg(t,g(S,c)) for c in inflow_cols if g(S,c) is not None)
    vout=itg(t,g(S,out_col))
    dS=sum(g(S,c)[-1]-g(S,c)[0] for c in storage_cols if g(S,c) is not None)
    err=vin-vout-dS
    return vin,vout,dS,err,(100*abs(err)/vin if vin else 0)

CH=['US_Channel_Storage','Barrel_Storage','DS_Channel_Storage']
def show(name,line): print(f"  {line}")

for fn in sorted(glob.glob('T*_out.txt')):
    if 'T7_' in fn: continue   # reported separately below
    tag=fn.replace('_out.txt','')
    S=read(fn); t=next(iter(S.values()))[0]
    print(f"\n=== {tag} ===")
    D=1.0
    hw=g(S,'Inlet_control_head'); qs=g(S,'Inlet_qstar'); w=g(S,'Inlet_submergence_weight')
    dep=g(S,'Barrel_depth'); sur=g(S,'Barrel_surcharge_head')
    qi=g(S,'Inlet_flow'); qo=g(S,'Outlet_flow')
    if qi is not None:
        show(tag,f"culvert flow  peak {qi.max()/86400:7.3f}  min {qi.min()/86400:8.3f} m3/s")
    if hw is not None:
        rise = 1.2 if 'box' in tag else 1.0
        show(tag,f"max HW/rise {hw.max()/rise:6.3f}   max q* {qs.max():6.2f}   max submergence weight {w.max():.2f}")
    if dep is not None:
        show(tag,f"barrel depth max {dep.max():.3f}   surcharge max {sur.max():.4f} m")
    # regime split
    icf=g(S,'Inlet_inlet_control_flow')
    if icf is not None and qi is not None:
        m=np.abs(qi)>0.02*np.abs(qi).max()
        inlet_gov=np.mean(np.abs(icf[m]-qi[m])<1e-3*np.maximum(np.abs(qi[m]),1))
        show(tag,f"inlet control governs {100*inlet_gov:5.1f}% of flowing steps")
    for extra in ['Road_flow','Road_head_upstream']:
        if g(S,extra) is not None:
            v=g(S,extra); show(tag,f"{extra:22s} peak {v.max()/(86400 if 'flow' in extra else 1):.4f}")
    if g(S,'Road_flow') is not None:
        qr=g(S,'Road_flow'); tot=qi+qr
        show(tag,f"overtopping share of total volume: {100*itg(t,qr)/itg(t,tot):.2f}%")
    # mass balance
    if g(S,'Tailwater_flow') is not None and g(S,'US_Channel_inflow') is not None:
        extra=['Road_flow'] if g(S,'Road_flow') is not None else []
        vin,vout,dS,err,pc=balance(S,t,['US_Channel_inflow'],'Tailwater_flow',CH)
        show(tag,f"volume in {vin:9.1f}  out {vout:9.1f}  dS {dS:8.1f}  closure {err:+8.2f} m3 ({pc:.3f}%)")
    print()

# ---- T7 is a side-by-side comparison, so it needs its own reporting ----
if os.path.exists('T7_lumped_vs_routed_out.txt'):
    S=read('T7_lumped_vs_routed_out.txt'); t=next(iter(S.values()))[0]
    a=g(S,'Culvert_A_flow'); b=g(S,'Inlet_B_flow')
    print("\n=== T7_lumped_vs_routed (single link A vs storage-routed barrel B) ===")
    print(f"  lumped  peak {a.max()/86400:7.4f} m3/s   volume {itg(t,a):10.1f} m3")
    print(f"  routed  peak {b.max()/86400:7.4f} m3/s   volume {itg(t,b):10.1f} m3")
    print(f"  peak difference {100*(b.max()-a.max())/a.max():+.2f}%   volume difference {100*(itg(t,b)-itg(t,a))/itg(t,a):+.2f}%")
    ca=g(S,'Culvert_A_culvert_flow'); ot=g(S,'Culvert_A_overtopping_flow')
    if ca is not None:
        print(f"  single-link accounting: culvert_flow peak {ca.max()/86400:.4f}, overtopping_flow peak {ot.max()/86400:.4f}, "
              f"sum matches flow: {np.allclose(ca+ot,a,rtol=1e-9)}")
    print()

# ---- the HDS-5 (dc+D)/2 grade line rule ----
if os.path.exists('T11_outlet_control_hgl_out.txt'):
    S=read('T11_outlet_control_hgl_out.txt'); t=next(iter(S.values()))[0]
    qo=g(S,'Outlet_flow'); m=np.abs(qo)>0.02*np.abs(qo).max()
    gate=g(S,'Outlet_hgl_gate'); dc=g(S,'Outlet_critical_depth'); tw=g(S,'Outlet_tailwater_hgl')
    icf=g(S,'Inlet_inlet_control_flow'); qi=g(S,'Inlet_flow')
    print("\n=== T11_outlet_control_hgl: HDS-5 (dc+D)/2 grade line rule ===")
    print(f"  rule gate max {gate.max():.3f}, active on {100*np.mean(gate[m]>0.01):.1f}% of flowing steps")
    print(f"  critical depth {dc[m].min():.3f}..{dc[m].max():.3f} m; tailwater used {tw[m].min():.3f}..{tw[m].max():.3f} m")
    print(f"  inlet control governs {100*np.mean(np.abs(icf[m]-qi[m])<1e-3*np.maximum(np.abs(qi[m]),1)):.1f}% of flowing steps")
    print()
