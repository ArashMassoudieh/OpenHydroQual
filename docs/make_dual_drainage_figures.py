"""Figures for the dual-drainage section of hydrologic_response_units.tex.

Run the example model first (OpenHydroQual-Console dual_drainage_example.ohq in
Examples/Dual_Drainage), then this script. Writes figures/dual_drainage_structure.pdf and
figures/dual_drainage_example.pdf.
"""
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

ROOT = Path(__file__).resolve().parents[1]
EX = ROOT / "Examples/Dual_Drainage"
FIG = Path(__file__).resolve().parent / "figures"
TEXT, TEXT2 = "#0b0b0b", "#52514e"
BLUE, ORANGE, GREEN, GREY, PURPLE = "#2a78d6", "#eb6834", "#1baf7a", "#8a8985", "#4a3aa7"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 9, "savefig.bbox": "tight"})


def structure():
    fig, ax = plt.subplots(figsize=(8.6, 3.6))
    ax.set_xlim(0, 11.6)
    ax.set_ylim(0, 5.0)
    ax.axis("off")

    def box(x0, y0, x1, y1, title, sub, fc):
        ax.add_patch(FancyBboxPatch((x0, y0), x1 - x0, y1 - y0, boxstyle="round,pad=0.04,rounding_size=0.12",
                                    fc=fc, ec="none"))
        ax.text((x0 + x1) / 2, (y0 + y1) / 2 + 0.13, title, ha="center", va="center", color="white",
                fontsize=9, weight="bold")
        ax.text((x0 + x1) / 2, (y0 + y1) / 2 - 0.2, sub, ha="center", va="center", color="white", fontsize=7.5)

    def arrow(p, q, label, color=TEXT2, lpos=None, ha="center"):
        ax.add_patch(FancyArrowPatch(p, q, arrowstyle="-|>", mutation_scale=11, lw=1.3, color=color))
        x, y = lpos if lpos else ((p[0] + q[0]) / 2, (p[1] + q[1]) / 2 + 0.12)
        ax.text(x, y, label, ha=ha, va="bottom", fontsize=7.5, color=color, family="monospace")

    box(0.1, 3.3, 2.5, 4.4, "Urban_HRU", "pervious + impervious", GREY)
    box(4.0, 3.3, 7.0, 4.4, "Surface segment", "street / swale", ORANGE)
    box(4.0, 0.5, 7.0, 1.6, "Sewer segment", "Sewerchannelsegment", PURPLE)
    box(8.9, 1.9, 11.5, 3.0, "Open channel", "receiving stream", BLUE)
    arrow((2.5, 3.85), (4.0, 3.85), "", lpos=(3.25, 3.95))
    ax.text(3.25, 4.5, "Reach_link\nImpervious_Reach_link", ha="center", va="bottom", fontsize=7.5, color=TEXT2, family="monospace")
    arrow((5.5, 3.3), (5.5, 1.6), "Swale_to_pipe_inlet", color=GREEN, lpos=(5.6, 2.4), ha="left")
    arrow((7.0, 1.05), (8.9, 2.2), "Pipe_outfall_link", color=GREEN, lpos=(7.9, 1.15), ha="left")
    arrow((7.0, 3.85), (8.9, 2.7), "Compound_Channel_link\n(overland overflow)", lpos=(8.0, 3.45), ha="left")
    arrow((1.3, 3.3), (8.9, 2.05), "unconfined_groundwater_to_stream", lpos=(1.0, 2.55), ha="left")
    ax.text(5.5, 0.12, "new links in dual_drainage.json shown in green", ha="center", fontsize=7.5, color=GREEN)
    FIG.mkdir(exist_ok=True)
    fig.savefig(FIG / "dual_drainage_structure.pdf")


def read(f):
    raw = pd.read_csv(f, skipinitialspace=True)
    cols = list(raw.columns)

    def s(n):
        i = cols.index(n)
        t, v = raw.iloc[:, i - 1], raw.iloc[:, i]
        m = t.notna() & v.notna()
        return pd.Series(v[m].values, index=t[m].values)
    return s


def example():
    s = read(EX / "output.txt")
    windows = [("10 mm/h for 3 h", 36531.35, 36531.75), ("50 mm/h for 1 h", 36541.52, 36541.82)]
    fig, axs = plt.subplots(2, 2, figsize=(8.4, 5.2), sharex="col", gridspec_kw=dict(height_ratios=[2, 1.2]))
    series = [("Impervious_outlet_flow", "runoff into the street", GREY, "-"),
              ("Inlets_flow", "inlets (street to pipe)", GREEN, "-"),
              ("Outfall_flow", "pipe outfall", PURPLE, "-"),
              ("Overland_overflow_flow", "overland overflow", ORANGE, "-"),
              ("Stream_outlet_flow", "stream outflow", BLUE, "--")]
    for k, (title, a, b) in enumerate(windows):
        ax, ax2 = axs[0, k], axs[1, k]
        for name, lab, c, ls in series:
            x = s(name)
            x = x[(x.index >= a) & (x.index <= b)]
            ax.plot((x.index - a) * 24, x.values / 86400, color=c, ls=ls, lw=1.5, label=lab)
        ax.set_title(title)
        ax.set_ylabel("flow (m$^3$/s)")
        h = s("Storm_drain_head")
        h = h[(h.index >= a) & (h.index <= b)]
        ax2.plot((h.index - a) * 24, h.values, color=PURPLE, lw=1.5, label="sewer head")
        d = s("Street_swale_head")
        d = d[(d.index >= a) & (d.index <= b)]
        ax2.plot((d.index - a) * 24, d.values, color=ORANGE, lw=1.5, label="street water surface")
        ax2.axhline(93.6, color=GREY, lw=0.8, ls=":")
        ax2.text(0.02, 93.62, "pipe crown", fontsize=7, color=TEXT2, transform=ax2.get_yaxis_transform())
        ax2.axhline(95.0, color=GREY, lw=0.8, ls=":")
        ax2.text(0.02, 95.02, "inlets", fontsize=7, color=TEXT2, transform=ax2.get_yaxis_transform())
        ax2.set_ylabel("elevation (m)")
        ax2.set_xlabel("hours from the window start")
        ax2.set_ylim(92.9, 95.5)
        if k == 0:
            ax.legend(fontsize=7.5, frameon=False)
            ax2.legend(fontsize=7.5, frameon=False, loc="center right")
    fig.tight_layout()
    fig.savefig(FIG / "dual_drainage_example.pdf")


if __name__ == "__main__":
    structure()
    if (EX / "output.txt").exists():
        example()
