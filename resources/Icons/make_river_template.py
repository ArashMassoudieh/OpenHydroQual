"""River Processes plugin icon (River_Template.png): an isometric block of floodplain with a meandering river cut into it,
the channel cross-section and the bed sediment layer on the front face, suspended sediment in the water.
Drawn at 4x and downsampled. python make_river_template.py"""
import math, os
from PIL import Image, ImageDraw, ImageFilter

S = 4                       # supersampling
N = 512 * S
HERE = os.path.dirname(os.path.abspath(__file__))
img = Image.new('RGBA', (N, N), (0, 0, 0, 0))
d = ImageDraw.Draw(img)

C30, S30 = math.cos(math.radians(30)), 0.5
SC = 300 * S                # size of the unit block on screen
CX, CY = N / 2, 150 * S     # screen position of the back corner (x=0, y=0, top)
H = 0.36                    # block height (in block units)
OUT = (20, 38, 62, 255)     # outline


def P(x, y, z):
    """isometric projection of block coordinates (x, y in 0..1, z up)"""
    return (CX + (x - y) * C30 * SC, CY + (x + y) * S30 * SC - z * SC)


def poly(points, fill, outline=None, width=0):
    d.polygon([P(*p) for p in points], fill=fill)
    if outline:
        pts = [P(*p) for p in points]
        d.line(pts + [pts[0]], fill=outline, width=width, joint='curve')


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))


# ---- the river centre line on top (x from back-left edge to the front-right face at x = 1)
def centre(t):
    return 0.52 + 0.17 * math.sin(2 * math.pi * (1.15 * t + 0.05))


HW = 0.10                  # half width of the channel at the surface
TS = [i / 120 for i in range(121)]

# ---- soil faces
ZB = 0.0
LAYERS = [(0.00, 0.10, (110, 72, 40)), (0.10, 0.22, (150, 102, 58)), (0.22, H - 0.035, (178, 128, 76)), (H - 0.035, H, (96, 160, 62))]
for z0, z1, col in LAYERS:   # left face (y = 1)
    poly([(0, 1, z0), (1, 1, z0), (1, 1, z1), (0, 1, z1)], lerp(col, (0, 0, 0), 0.10) + (255,))
for z0, z1, col in LAYERS:   # right face (x = 1)
    poly([(1, 1, z0), (1, 0, z0), (1, 0, z1), (1, 1, z1)], col + (255,))

# gravel speckles on the soil faces
import random
random.seed(4)
for _ in range(900):
    face = random.random() < 0.5
    a, z = random.random(), random.uniform(0.01, H - 0.05)
    p = P(a, 1, z) if face else P(1, a, z)
    r = random.uniform(1.2, 3.2) * S
    shade = random.choice([(90, 58, 32), (205, 165, 110), (130, 90, 52)])
    d.ellipse([p[0] - r, p[1] - r * 0.7, p[0] + r, p[1] + r * 0.7], fill=shade + (200,))

# ---- top: floodplain grass with a light gradient
for k in range(40):
    t0, t1 = k / 40, (k + 1) / 40
    col = lerp((128, 196, 78), (92, 166, 60), t0)
    poly([(t0, 0, H), (t1, 0, H), (t1, 1, H), (t0, 1, H)], col + (255,))

# ---- channel on top: banks (sand), then water; cut into the right face
BANK = 0.022
left_b = [(t, centre(t) - HW - BANK) for t in TS]; right_b = [(t, centre(t) + HW + BANK) for t in TS]
poly([(x, y, H) for x, y in left_b] + [(x, y, H) for x, y in reversed(right_b)], (214, 190, 132, 255))
left = [(t, centre(t) - HW) for t in TS]; right = [(t, centre(t) + HW) for t in TS]
WZ = H - 0.025
poly([(x, y, WZ) for x, y in left] + [(x, y, WZ) for x, y in reversed(right)], (58, 140, 214, 255))
# lighter core of the stream
core_l = [(t, centre(t) - HW * 0.45) for t in TS]; core_r = [(t, centre(t) + HW * 0.45) for t in TS]
poly([(x, y, WZ) for x, y in core_l] + [(x, y, WZ) for x, y in reversed(core_r)], (104, 182, 238, 255))
# flow streaks
for t0 in (0.12, 0.38, 0.62, 0.86):
    pts = [P(t, centre(t) + 0.02 * math.sin(9 * t), WZ) for t in [t0 + i * 0.008 for i in range(10)]]
    d.line(pts, fill=(235, 247, 255, 230), width=int(2.4 * S), joint='curve')

# ---- channel cross-section on the right face (x = 1): trapezoid of water over the bed sediment layer
yc = centre(1.0)
top_l, top_r = yc - HW - BANK, yc + HW + BANK
bot_l, bot_r = yc - HW * 0.45, yc + HW * 0.45
ZBED = H - 0.19
# bed sediment layer (darker, speckled) under the channel
poly([(1, top_l - 0.03, ZBED - 0.05), (1, top_r + 0.03, ZBED - 0.05), (1, top_r + 0.012, H - 0.02), (1, top_l - 0.012, H - 0.02)], (118, 96, 72, 255))
for _ in range(160):
    yy = random.uniform(top_l - 0.025, top_r + 0.025); zz = random.uniform(ZBED - 0.045, ZBED + 0.01)
    p = P(1, yy, zz); r = random.uniform(1.0, 2.4) * S
    d.ellipse([p[0] - r, p[1] - r * 0.7, p[0] + r, p[1] + r * 0.7], fill=random.choice([(70, 55, 40), (160, 135, 100), (95, 78, 58)]) + (255,))
# water in the section
poly([(1, top_l + 0.006, WZ), (1, top_r - 0.006, WZ), (1, bot_r, ZBED + 0.005), (1, bot_l, ZBED + 0.005)], (52, 128, 204, 255))
poly([(1, top_l + 0.02, WZ - 0.005), (1, top_r - 0.02, WZ - 0.005), (1, bot_r - 0.01, ZBED + 0.04), (1, bot_l + 0.01, ZBED + 0.04)], (82, 160, 226, 255))
# suspended sediment in the section, settling arrows
for _ in range(26):
    yy = random.uniform(bot_l + 0.005, bot_r - 0.005); zz = random.uniform(ZBED + 0.02, WZ - 0.02)
    p = P(1, yy, zz); r = random.uniform(1.6, 2.6) * S
    d.ellipse([p[0] - r, p[1] - r, p[0] + r, p[1] + r], fill=(196, 150, 90, 255))
for yy in (yc - 0.03, yc + 0.03):
    a, b = P(1, yy, WZ - 0.025), P(1, yy, ZBED + 0.03)
    d.line([a, b], fill=(255, 255, 255, 235), width=int(2.2 * S))
    d.polygon([(b[0] - 4 * S, b[1] - 6 * S), (b[0] + 4 * S, b[1] - 6 * S), (b[0], b[1] + 1 * S)], fill=(255, 255, 255, 235))

# ---- trees on the floodplain
def tree(x, y, s=1.0):
    base = P(x, y, H); h = 34 * S * s; w = 12 * S * s
    d.rectangle([base[0] - 1.6 * S * s, base[1] - 8 * S * s, base[0] + 1.6 * S * s, base[1]], fill=(110, 76, 44, 255))
    for k, (dy, ww) in enumerate(((0, 1.0), (-0.32, 0.8), (-0.58, 0.6))):
        top = (base[0], base[1] - 6 * S * s - h * (0.55 - dy * 0.0) + dy * h)
        y0 = base[1] - 6 * S * s + dy * h
        d.polygon([(base[0] - w * ww, y0), (base[0] + w * ww, y0), (base[0], y0 - h * 0.55)], fill=lerp((40, 120, 60), (70, 160, 80), k / 2) + (255,))


for (x, y, s) in ((0.12, 0.12, 0.9), (0.28, 0.2, 1.0), (0.18, 0.86, 1.0), (0.42, 0.9, 0.8), (0.7, 0.18, 1.1), (0.86, 0.3, 0.8), (0.6, 0.88, 0.9)):
    if abs(y - centre(x)) > HW + 0.07:
        tree(x, y, s)

# ---- outlines of the block
for face in ([(0, 0, H), (1, 0, H), (1, 1, H), (0, 1, H)], [(0, 1, 0), (1, 1, 0), (1, 1, H), (0, 1, H)], [(1, 1, 0), (1, 0, 0), (1, 0, H), (1, 1, H)]):
    pts = [P(*p) for p in face]
    d.line(pts + [pts[0]], fill=OUT, width=int(3.2 * S), joint='curve')

# soft drop shadow, then downsample
shadow = Image.new('RGBA', (N, N), (0, 0, 0, 0))
sd = ImageDraw.Draw(shadow)
sd.polygon([P(0.04, 1.03, -0.02), P(1.03, 1.03, -0.02), P(1.03, 0.04, -0.02), P(1.08, 1.08, -0.06)], fill=(0, 0, 0, 70))
shadow = shadow.filter(ImageFilter.GaussianBlur(10 * S))
out = Image.alpha_composite(shadow, img).resize((512, 512), Image.LANCZOS)
out.save(os.path.join(HERE, 'River_Template.png'))
print('wrote River_Template.png')
