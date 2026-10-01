"""Figures and tables for docs/hydrologic_response_units.tex.

  figures/hru_structure.pdf          members, links and areas of Hydrologic_Response_Unit
  figures/urban_hru_structure.pdf    the same for Urban_HRU
  figures/urban_hru_example.pdf      the Examples/Urban_HRU run
  figures/hru_example_volumes.tex    water-balance table of that run
  figures/hru_properties.tex         property reference of both composites

Run the example model first (OpenHydroQual-Console Urban_HRU_example.ohq in Examples/Urban_HRU), then
    python3 docs/make_hru_figures.py
Requires numpy, pandas, matplotlib.
"""
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch

REPO = Path(__file__).resolve().parents[1]
FIG = REPO / "docs/figures"
EX = REPO / "Examples/Urban_HRU"
TEXT, TEXT2, MUTED = "#0b0b0b", "#52514e", "#8a8985"
ORANGE, BLUE = "#eb6834", "#2a78d6"
SOIL = ["#8a5a2b", "#a8743d", "#c49a5e"]
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 9, "savefig.bbox": "tight"})


def structure_urban():
    fig, ax = plt.subplots(figsize=(7.2, 6.0))
    ax.set_xlim(0, 11.6)
    ax.set_ylim(-0.1, 9.6)
    ax.set_axis_off()

    def box(x0, y0, x1, y1, title, sub, fc, tc="white"):
        ax.add_patch(FancyBboxPatch((x0, y0), x1 - x0, y1 - y0, boxstyle="round,pad=0.02,rounding_size=0.12",
                                    fc=fc, ec=TEXT2, lw=0.8))
        ax.text((x0 + x1) / 2, (y0 + y1) / 2 + (0.14 if sub else 0), title, ha="center", va="center",
                color=tc, fontsize=8.5, weight="bold")
        if sub:
            ax.text((x0 + x1) / 2, (y0 + y1) / 2 - 0.2, sub, ha="center", va="center", color=tc, fontsize=7.5)

    def arrow(p, q, label=None, color=TEXT2, lw=1.3, lpos=None):
        ax.annotate("", xy=q, xytext=p, arrowprops=dict(arrowstyle="-|>", color=color, lw=lw, shrinkA=0, shrinkB=0))
        if label:
            lx, ly = lpos if lpos else ((p[0] + q[0]) / 2 + 0.08, (p[1] + q[1]) / 2)
            ax.text(lx, ly, label, fontsize=6.8, color=color, va="center", family="monospace")

    perv = r"$A(1-f_i)$"
    box(0.3, 8.35, 3.8, 9.1, "Impervious_Reach", "trapezoidal", "#6f7680")
    box(4.3, 7.25, 8.3, 8.0, "Reach", "trapezoidal", "#6f7680")
    box(0.3, 5.95, 3.8, 6.75, "Impervious_Catchment", r"area $A\,f_i$;  P", "#55585e")
    box(4.3, 5.95, 8.3, 6.75, "Catchment (pervious)", perv + ";  P", "#4f9a2f")
    box(4.3, 4.75, 8.3, 5.45, "Soil_1", r"$D/7$;  " + perv + ";  ET", SOIL[0])
    box(4.3, 3.6, 8.3, 4.3, "Soil_2", r"$2D/7$;  " + perv + ";  ET$_2$", SOIL[1])
    box(4.3, 2.45, 8.3, 3.15, "Soil_3", r"$4D/7$;  " + perv + ";  ET$_2$", SOIL[2])
    box(0.3, 0.7, 8.3, 1.75, "Unconfined Groundwater cell", r"area $A$ (whole unit);  $h=z_g+V/(S_y A)$;  initial state: water-table elevation", BLUE)
    ax.add_patch(FancyBboxPatch((0.3, 1.95), 3.5, 3.5, boxstyle="round,pad=0.02,rounding_size=0.12",
                                fc="#e9e7e2", ec=MUTED, lw=0.6, ls="--"))
    ax.text(2.05, 3.7, "no infiltration\nbelow impervious\nsurfaces", ha="center", va="center", fontsize=7.5,
            color=TEXT2, style="italic")
    box(9.1, 0.7, 11.3, 9.1, "", "", "#9ec5f4")
    ax.text(10.2, 4.9, "receiving\nchannel\nsegment\n(external)", ha="center", va="center", fontsize=8, color=TEXT,
            weight="bold")
    arrow((2.05, 6.75), (2.05, 8.35), "Catchment_link", lpos=(2.13, 7.55))
    arrow((6.3, 6.75), (6.3, 7.25), "Catchment_link", lpos=(6.38, 7.0))
    arrow((6.3, 5.95), (6.3, 5.45), "surfacewater_to_soil_link", lpos=(6.38, 5.7))
    arrow((6.3, 4.75), (6.3, 4.3), "soil_to_soil_link", lpos=(6.38, 4.52))
    arrow((6.3, 3.6), (6.3, 3.15), "soil_to_soil_link", lpos=(6.38, 3.37))
    arrow((6.3, 2.45), (6.3, 1.75), "soil_to_unconfined_groundwater_link", color=ORANGE, lw=2.0, lpos=(6.38, 2.12))
    ax.text(6.38, 1.9, r"$K=K_{sat,3}k_r(S_{e,3})$ / $K_{sat,3}$;  " + perv, fontsize=6.6, color=ORANGE, va="center")
    arrow((3.8, 8.72), (9.1, 8.72), "Impervious_Reach_link", lpos=(5.0, 8.88))
    arrow((8.3, 7.62), (9.1, 7.62))
    ax.text(8.45, 7.9, "Reach_link", fontsize=6.8, color=TEXT2, family="monospace")
    arrow((8.3, 1.22), (9.1, 1.22))
    ax.text(10.2, 1.75, "unconfined_\ngroundwater_\nto_stream", fontsize=6.6, color=TEXT2, family="monospace", va="center", ha="center")
    foot = r"$f_i=\min(f_{imp}\,m_{imp},\,0.95)$;  "
    ax.text(0.3, 0.42, foot + r"$D$ = depth_to_groundwater;  $S_y$ = groundwater_porosity, $b$ = groundwater thickness",
            fontsize=7.2, color=TEXT2, va="center")
    ax.text(0.3, 0.1, "van Genuchten parameters and $K_{sat}$ set per soil layer; Soil_1 $K_{sat}$ also scaled by K_sat_scale_surface",
            fontsize=7.2, color=TEXT2, va="center")
    fig.savefig(FIG / "urban_hru_structure.pdf")
    plt.close(fig)


def structure_plain():
    """Hydrologic_Response_Unit: one infiltrating surface over three soil layers and groundwater."""
    fig, ax = plt.subplots(figsize=(7.2, 5.4))
    ax.set_xlim(0, 11.6)
    ax.set_ylim(0.1, 8.2)
    ax.set_axis_off()

    def box(x0, y0, x1, y1, title, sub, fc, tc="white"):
        ax.add_patch(FancyBboxPatch((x0, y0), x1 - x0, y1 - y0, boxstyle="round,pad=0.02,rounding_size=0.12",
                                    fc=fc, ec=TEXT2, lw=0.8))
        ax.text((x0 + x1) / 2, (y0 + y1) / 2 + (0.14 if sub else 0), title, ha="center", va="center",
                color=tc, fontsize=8.5, weight="bold")
        if sub:
            ax.text((x0 + x1) / 2, (y0 + y1) / 2 - 0.2, sub, ha="center", va="center", color=tc, fontsize=7.5)

    def arrow(p, q, label=None, color=TEXT2, lw=1.3, lpos=None):
        ax.annotate("", xy=q, xytext=p, arrowprops=dict(arrowstyle="-|>", color=color, lw=lw, shrinkA=0, shrinkB=0))
        if label:
            lx, ly = lpos if lpos else ((p[0] + q[0]) / 2 + 0.08, (p[1] + q[1]) / 2)
            ax.text(lx, ly, label, fontsize=6.8, color=color, va="center", family="monospace")

    box(2.3, 5.95, 6.3, 6.75, "Catchment", r"area $A$;  P", "#4f9a2f")
    box(2.3, 4.75, 6.3, 5.45, "Soil_1", r"$D/7$;  $A$;  ET", SOIL[0])
    box(2.3, 3.6, 6.3, 4.3, "Soil_2", r"$2D/7$;  $A$", SOIL[1])
    box(2.3, 2.45, 6.3, 3.15, "Soil_3", r"$4D/7$;  $A$", SOIL[2])
    box(2.3, 0.7, 6.3, 1.75, "Groundwater", r"$A$;  $S_s$ user-supplied", BLUE)
    arrow((4.3, 5.95), (4.3, 5.45), "surfacewater_to_soil_link", lpos=(4.38, 5.7))
    arrow((4.3, 4.75), (4.3, 4.3), "soil_to_soil_link", lpos=(4.38, 4.52))
    arrow((4.3, 3.6), (4.3, 3.15), "soil_to_soil_link", lpos=(4.38, 3.37))
    arrow((4.3, 2.45), (4.3, 1.75), "soil2groundwater_link", color=ORANGE, lw=2.0, lpos=(4.38, 2.12))
    ax.text(4.38, 1.9, r"$K$ = recharge_K (constant)", fontsize=6.6, color=ORANGE, va="center")
    box(8.2, 5.75, 11.3, 6.95, "", "", "#dfe9d6")
    ax.text(9.75, 6.35, "downslope HRU,\ncatchment or any block\n(external)", ha="center", va="center", fontsize=7.5)
    box(8.55, 0.55, 11.3, 1.9, "", "", "#9ec5f4")
    ax.text(9.925, 1.22, "neighbouring\ngroundwater, stream\nor fixed head (external)", ha="center", va="center",
            fontsize=7)
    arrow((6.3, 6.35), (8.2, 6.35))
    ax.text(6.4, 6.6, "Catchment_link", fontsize=6.8, color=TEXT2, family="monospace")
    arrow((6.3, 1.5), (8.55, 1.5))
    for i, lab in enumerate(("groundwater_link", "groundwater_to_stream", "Linear_baseflow", "groundwater_to_fixedhead")):
        ax.text(6.38, 1.3 - 0.21 * i, lab, fontsize=5.9, color=TEXT2, family="monospace", va="center")
    ax.text(0.3, 0.3, r"$D$ = depth_to_groundwater;  one set of van Genuchten parameters and $K_{sat}$ shared by the three layers",
            fontsize=7.2, color=TEXT2, va="center")
    fig.savefig(FIG / "hru_structure.pdf")
    plt.close(fig)


def read(f, names):
    hdr = [h.strip() for h in open(f).readline().strip().split(",")[1::2]]
    idx = {n: hdr.index(n) for n in names}
    d = pd.read_csv(f, skiprows=1, header=None, usecols=sorted(c for i in idx.values() for c in (2 * i, 2 * i + 1)))
    out = {}
    for n, i in idx.items():
        t, v = d[2 * i].dropna().values, d[2 * i + 1].dropna().values
        k = min(len(t), len(v))
        out[n] = pd.Series(v[:k], index=t[:k])
    return out


def example():
    names = ["Impervious_outlet_flow", "Pervious_outlet_flow", "Baseflow_flow", "Stream_outlet_flow",
             "Neighbourhood__recharge_flow", "Neighbourhood__infiltration_flow", "Neighbourhood__Groundwater_piezometric_head"]
    s = read(EX / "output.txt", names)
    A, fi, t0 = 500000.0, 0.25, 36526
    p = pd.read_csv(EX / "precipitation.csv", header=None)
    rain = float(p[2].sum()) * A
    fig, axs = plt.subplots(3, 1, figsize=(7.5, 6.4), sharex=True)
    axs[0].bar(p[0] - t0, 1000 * p[2] / (p[1] - p[0]), width=1 / 24, color=BLUE)
    axs[0].set_ylabel("Rain (mm/d)")
    for key, lab, c in (("Impervious_outlet_flow", "Impervious_Reach_link", TEXT2),
                        ("Pervious_outlet_flow", "Reach_link", "#1baf7a"),
                        ("Baseflow_flow", "groundwater_to_stream", ORANGE),
                        ("Stream_outlet_flow", "stream outflow", BLUE)):
        axs[1].plot(s[key].index - t0, s[key].values / 86400, lw=1.1, color=c, label=lab)
    axs[1].set_yscale("symlog", linthresh=1e-4)
    axs[1].set_ylabel("Flow (m$^3$/s)")
    axs[1].legend(fontsize=7, ncol=2)
    r = s["Neighbourhood__recharge_flow"]
    axs[2].plot(r.index - t0, 1000 * r.values / (A * (1 - fi)), color="#4a3aa7", lw=1.1, label="recharge")
    axs[2].set_ylabel("Recharge (mm/d)")
    axs[2].set_xlabel("Day")
    ax2 = axs[2].twinx()
    h = s["Neighbourhood__Groundwater_piezometric_head"]
    ax2.plot(h.index - t0, h.values, color="#e34948", lw=1.1, label="groundwater head")
    ax2.set_ylabel("Groundwater head (m)")
    axs[2].legend(fontsize=7, loc="upper center")
    ax2.legend(fontsize=7, loc="upper right")
    for a in axs:
        a.grid(True, color="#e4e3df", lw=0.6)
    fig.tight_layout()
    fig.savefig(FIG / "urban_hru_example.pdf")
    plt.close(fig)

    vol = lambda x: float(np.trapezoid(x.values, x.index))
    rows = [("Rain", rain)]
    for key, lab in (("Impervious_outlet_flow", "Impervious outlet (\\code{Impervious\\_Reach\\_link})"),
                     ("Pervious_outlet_flow", "Pervious outlet (\\code{Reach\\_link})"),
                     ("Neighbourhood__infiltration_flow", "Infiltration"),
                     ("Neighbourhood__recharge_flow", "Recharge"),
                     ("Baseflow_flow", "Baseflow (\\code{groundwater\\_to\\_stream})"),
                     ("Stream_outlet_flow", "Stream outflow")):
        rows.append((lab, vol(s[key])))
    hh = s["Neighbourhood__Groundwater_piezometric_head"]
    lines = [r"\begin{tabular}{lr}", r"\toprule", r"Volume over 30 days & m$^3$ \\", r"\midrule"]
    lines += [f"{a} & {b:,.0f} \\\\" for a, b in rows]
    lines += [r"\midrule", f"Groundwater head rise (m) & {hh.iloc[-1] - hh.iloc[0]:.2f} \\\\", r"\bottomrule",
              r"\end{tabular}"]
    (FIG / "hru_example_volumes.tex").write_text("\n".join(lines) + "\n")
    for r_ in rows:
        print(f"{r_[0]:60s} {r_[1]:12,.0f}")


def tex(s):
    s = str(s).replace("\\", "/").replace("_", "\\_").replace("#", ".").replace("%", "\\%")
    return s.replace("~^2", "@SQ@").replace("^", "\\^{}").replace("@SQ@", "$^2$")


def property_table():
    """Longtable of the exposed properties of the two HRU composites (from the resource files)."""
    import json
    units = {"HRU": json.load(open(REPO / "resources/hydrologic_response_unit.json"))["Hydrologic_Response_Unit"],
             "Urban": json.load(open(REPO / "resources/urban_hru.json"))["Urban_HRU"]}
    order = []
    for u in units.values():
        order += [k for k in u if k not in order]
    rows = []
    for k in order:
        have = [n for n, u in units.items() if isinstance(u.get(k), dict) and u[k].get("type") in ("value", "source")]
        if not have:
            continue
        v = units[have[-1]][k]
        unit = str(v.get("unit", "")).split(";")[0]
        targets = sorted({t.split("#")[0] for u in have for t in (units[u][k].get("applyto") or {})})
        if k in ("area", "impervious_fraction", "impervious_multiplier"):
            targets = ["member areas"]
        defaults = {units[u][k].get("default", "") for u in have}
        default = "/".join(sorted(map(str, defaults))) if len(defaults) > 1 else next(iter(defaults))
        where = "both" if len(have) == 2 else have[0]
        rows.append(f"\\code{{{tex(k)}}} & {tex(unit)} & {tex(default)} & {where} & {tex(', '.join(targets))} \\\\")
    head = [r"\begin{longtable}{p{4.9cm}p{1.3cm}p{1.2cm}p{1.9cm}p{4.7cm}}", r"\toprule",
            r"Property & Unit & Default & In & Applied to \\", r"\midrule", r"\endhead"]
    (FIG / "hru_properties.tex").write_text("\n".join(head + rows + [r"\bottomrule", r"\end{longtable}"]) + "\n")

if __name__ == "__main__":
    structure_plain()
    structure_urban()
    if (EX / "output.txt").exists():
        example()
    property_table()
