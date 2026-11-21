"""Draw the River Processes plugin icons (256 x 256, transparent): python make_river_icons.py
One visual language for the whole plugin: a trapezoidal river section (navy outline, water, grassy banks),
the fixed-head symbol, layered bed sediment, and navy flow arrows."""
import os, numpy as np, matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, FancyArrowPatch, Circle, Rectangle, FancyBboxPatch
HERE = os.path.dirname(os.path.abspath(__file__))
NAVY, WATER, LIGHT, BANK, GRASS, SAND, SILT, PEB = '#0b2540', '#2f7fc1', '#9fd0f0', '#c8a46a', '#6aa84f', '#d9b778', '#b08a52', '#8a6a3c'


def canvas():
    fig = plt.figure(figsize=(2.56, 2.56), dpi=100); ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(0, 256); ax.set_ylim(0, 256); ax.axis('off'); return fig, ax


def save(fig, name):
    fig.savefig(os.path.join(HERE, name), transparent=True); plt.close(fig); print('wrote', name)


def section(ax, x0, y0, w, h, water=0.62, lw=5):
    """trapezoidal river cross-section, bottom-left corner at (x0, y0)"""
    b = w * 0.42; s = (w - b) / 2
    ax.add_patch(Polygon([(x0 - w * 0.12, y0 + h), (x0, y0 + h), (x0 + s, y0), (x0 + s + b, y0), (x0 + w, y0 + h), (x0 + w * 1.12, y0 + h),
                          (x0 + w * 1.12, y0 - h * 0.25), (x0 - w * 0.12, y0 - h * 0.25)], fc=BANK, ec='none'))
    ax.add_patch(Polygon([(x0 - w * 0.12, y0 + h), (x0, y0 + h), (x0 - w * 0.12, y0 + h * 0.86)], fc=GRASS, ec='none'))
    ax.add_patch(Polygon([(x0 + w * 1.12, y0 + h), (x0 + w, y0 + h), (x0 + w * 1.12, y0 + h * 0.86)], fc=GRASS, ec='none'))
    yw = y0 + h * water; xl = x0 + s * (1 - water); xr = x0 + w - s * (1 - water)
    ax.add_patch(Polygon([(xl, yw), (x0 + s, y0), (x0 + s + b, y0), (xr, yw)], fc=WATER, ec='none'))
    ax.plot([xl + 6, xr - 6], [yw - 6, yw - 6], color=LIGHT, lw=lw * 0.6, solid_capstyle='round')
    ax.plot([x0 - w * 0.12, x0, x0 + s, x0 + s + b, x0 + w, x0 + w * 1.12], [y0 + h, y0 + h, y0, y0, y0 + h, y0 + h], color=NAVY, lw=lw, solid_joinstyle='round')


def fixed_head(ax, xc, yc, s=1.0, lw=6):
    ax.plot([xc, xc], [yc + 46 * s, yc], color=NAVY, lw=lw)
    for k, half in enumerate((40, 27, 14)):
        ax.plot([xc - half * s, xc + half * s], [yc - 12 * k * s, yc - 12 * k * s], color=NAVY, lw=lw, solid_capstyle='round')


def arrow(ax, a, b, c=NAVY, lw=7, ms=34):
    ax.add_patch(FancyArrowPatch(a, b, arrowstyle='-|>', mutation_scale=ms, color=c, lw=lw, shrinkA=0, shrinkB=0))


def bed(ax, x0, y0, w, h, rng):
    ax.add_patch(Rectangle((x0, y0), w, h, fc=SAND, ec='none'))
    ax.add_patch(Rectangle((x0, y0), w, h * 0.42, fc=SILT, ec='none'))
    for _ in range(int(w * h / 420)):
        ax.add_patch(Circle((rng.uniform(x0 + 6, x0 + w - 6), rng.uniform(y0 + 5, y0 + h - 5)), rng.uniform(3, 7), fc=PEB, ec='none', alpha=0.85))
    ax.add_patch(Rectangle((x0, y0), w, h, fc='none', ec=NAVY, lw=5))


rng = np.random.default_rng(7)
# 1. River segment: a reach in perspective over its trapezoidal section
fig, ax = canvas()
ax.add_patch(Polygon([(70, 250), (186, 250), (214, 150), (42, 150)], fc=GRASS, ec='none', alpha=0.55))
ax.add_patch(Polygon([(108, 250), (148, 250), (172, 150), (84, 150)], fc=WATER, ec='none', alpha=0.9))
for y in (226, 200, 174): ax.plot([128 - (250 - y) * 0.25, 128 + (250 - y) * 0.25], [y, y], color=LIGHT, lw=3)
section(ax, 40, 40, 176, 110)
save(fig, 'river_segment.png')
# 2. River segment to river segment (Manning)
fig, ax = canvas()
section(ax, 22, 120, 96, 62, lw=4); section(ax, 138, 40, 96, 62, lw=4)
arrow(ax, (92, 118), (176, 112)); ax.text(128, 205, 'n', fontsize=34, color=NAVY, ha='center', style='italic', weight='bold')
save(fig, 'river_link.png')
# 3. Fixed head to river segment
fig, ax = canvas()
fixed_head(ax, 52, 150, 0.9); arrow(ax, (92, 170), (150, 132)); section(ax, 132, 40, 104, 70, lw=4)
save(fig, 'fixedhead_to_river.png')
# 4. River segment to fixed head
fig, ax = canvas()
section(ax, 20, 140, 104, 70, lw=4); arrow(ax, (104, 128), (166, 92)); fixed_head(ax, 204, 70, 0.9)
save(fig, 'river_to_fixedhead.png')
# 5. Bed sediment layer
fig, ax = canvas()
ax.add_patch(Rectangle((24, 168), 208, 52, fc=WATER, ec='none'))
for x in (60, 128, 196): ax.plot([x - 22, x + 22], [200, 200], color=LIGHT, lw=4, solid_capstyle='round')
bed(ax, 24, 40, 208, 128, rng)
save(fig, 'bed_sediment_layer.png')
# 6. River to bed sediment (exchange)
fig, ax = canvas()
ax.add_patch(Rectangle((24, 150), 208, 80, fc=WATER, ec='none'))
bed(ax, 24, 36, 208, 114, rng)
arrow(ax, (92, 214), (92, 110), lw=7); arrow(ax, (164, 96), (164, 200), c='#c0392b', lw=7)
save(fig, 'river_bed_exchange.png')
# 7. Bed sediment to fixed head
fig, ax = canvas()
bed(ax, 20, 140, 150, 96, rng); arrow(ax, (150, 132), (196, 92)); fixed_head(ax, 212, 40, 0.75, lw=5)
save(fig, 'bed_to_fixedhead.png')
# 8. Suspended sediment
fig, ax = canvas()
ax.add_patch(FancyBboxPatch((28, 28), 200, 200, boxstyle='round,pad=0,rounding_size=26', fc=WATER, ec=NAVY, lw=6))
ax.plot([46, 210], [206, 206], color=LIGHT, lw=4)
for _ in range(34):
    x, y, r = rng.uniform(48, 208), rng.uniform(60, 190), rng.uniform(4, 11)
    ax.add_patch(Circle((x, y), r, fc=SILT if r > 7 else SAND, ec=NAVY, lw=1.2))
ax.add_patch(Rectangle((34, 34), 188, 22, fc=SILT, ec='none'))
arrow(ax, (210, 170), (210, 92), c='white', lw=5, ms=26)
save(fig, 'suspended_sediment.png')
