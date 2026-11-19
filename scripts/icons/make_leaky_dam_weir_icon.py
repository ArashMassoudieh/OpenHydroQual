# Draws resources/Icons/leaky_dam_weir.png (256x256):  python3 make_leaky_dam_weir_icon.py ../../resources/Icons/leaky_dam_weir.png
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Polygon, Circle, FancyArrowPatch
import numpy as np, sys
OUT=sys.argv[1]
INK="#46362a"; CREAM="#fbe7cc"; GREEN="#6e9a3e"; BLUE="#2f8fb3"; LBLUE="#7cc4dd"; DBLUE="#1f6e8c"; BROWN="#b57a3c"; DBROWN="#8a5a2b"; SOIL="#d9b48a"
fig=plt.figure(figsize=(2.56,2.56),dpi=100); ax=fig.add_axes([0,0,1,1]); ax.set_xlim(0,100); ax.set_ylim(0,100); ax.axis("off")
lw=3.0
clip=FancyBboxPatch((6,6),88,88,boxstyle="round,pad=0,rounding_size=14",fc=CREAM,ec="none"); ax.add_patch(clip)
def add(p): p.set_clip_path(clip); ax.add_patch(p); return p
add(Polygon([[0,70],[30,78],[62,72],[100,79],[100,0],[0,0]],fc=GREEN,ec=INK,lw=lw))
bed=lambda x: 24-0.05*x
add(Polygon([[0,bed(0)],[100,bed(100)],[100,0],[0,0]],fc=SOIL,ec=INK,lw=lw))
# pools: upstream level 58, downstream 50, crest 52 (partly submerged)
add(Polygon([[0,58],[40,58],[40,bed(40)],[0,bed(0)]],fc=BLUE,ec="none"))
add(Polygon([[58,50],[100,50],[100,bed(100)],[58,bed(58)]],fc=BLUE,ec="none"))
# dam (trapezoid of logs), crest 52
add(Polygon([[36,bed(36)],[42,52],[56,52],[62,bed(62)]],fc=BROWN,ec=INK,lw=lw,joinstyle="round"))
for (cx,cy) in [(44,28),(50,28),(56,28),(47,36),(53,36),(50,44)]:
    add(Circle((cx,cy),2.8,fc=DBROWN,ec=INK,lw=1.5))
# overflow sheet over the crest
add(Polygon([[38,58],[50,57.5],[58,55],[64,51],[60,50],[56,52],[42,52],[38,52]],fc=LBLUE,ec=INK,lw=2.0,joinstyle="round"))
ax.plot([8,38],[58,58],color=INK,lw=lw,clip_path=clip); ax.plot([62,96],[50,50],color=INK,lw=lw,clip_path=clip)
for y0,x0,x1 in [(48,10,30),(38,12,32),(42,68,90),(33,70,92)]:
    ax.plot(np.linspace(x0,x1,30),y0+1.2*np.sin(np.linspace(0,2*np.pi,30)),color=DBLUE,lw=2.2,solid_capstyle="round",clip_path=clip)
# overflow arrow and seepage arrow (through the dam body)
ax.add_patch(FancyArrowPatch((30,64),(70,58),connectionstyle="arc3,rad=-0.25",arrowstyle="-|>",mutation_scale=13,color=INK,lw=2.4,zorder=6))
ax.add_patch(FancyArrowPatch((30,22),(70,20),arrowstyle="-|>",mutation_scale=11,color="white",lw=2.0,zorder=6,linestyle=(0,(2,1.5))))
ax.add_patch(FancyBboxPatch((6,6),88,88,boxstyle="round,pad=0,rounding_size=14",fc="none",ec=INK,lw=4.5))
fig.savefig(OUT,transparent=True)
