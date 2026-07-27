"""
fix_transparency.py
-------------------
Applies the same near-white background keying used by convert_sprites.py to
WB2 sprites that were added to gfx/ directly without going through that pipeline.
Any opaque pixel with r>=248, g>=248, b>=248 is made fully transparent.
Safe to run multiple times (already-transparent pixels are skipped).
"""

import os
from PIL import Image

GFX = r"D:\PsyDoom-Project\LEGOWB3DS\gfx"

FILES = [
    # WB2 building sprites
    "building_house.png",
    "building_house_hero.png",
    "building_factory.png",
    "building_factory_red.png",
    "building_factory_blue.png",
    "building_factory_green.png",
    "building_factory_yellow.png",
    "building_factory_white.png",
    "building_factory_hero.png",
    "building_windmill_1.png",
    "building_windmill_2.png",
    "building_windmill_3.png",
    "building_windmill_4.png",
    "building_windmill_hero.png",
    "building_garage.png",
    "building_garage_hero.png",
    "building_nursery.png",
    "building_nursery_hero.png",
    # WB2 terrain sprites
    "terrain_tree4.png",
    "terrain_jungle4.png",
    "terrain_roadblock.png",
    # WB2 plan sprites
    "plan_house.png",
    "plan_factory.png",
    "plan_windmill.png",
    "plan_garage.png",
    "plan_nursery.png",
    # carry icons (all colours)
    "carry_red.png",
    "carry_blue.png",
    "carry_green.png",
    "carry_yellow.png",
    "carry_wheel.png",
    "carry_energy.png",
    "carry_energy_low.png",
    "carry_energy_dead.png",
    "carry_white.png",
]

ok = 0
missing = []

for fname in FILES:
    path = os.path.join(GFX, fname)
    if not os.path.exists(path):
        missing.append(fname)
        continue
    img = Image.open(path).convert("RGBA")
    pixels = img.load()
    w, h = img.size
    changed = 0
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if a != 0 and r >= 248 and g >= 248 and b >= 248:
                pixels[x, y] = (255, 255, 255, 0)
                changed += 1
    img.save(path)
    print(f"  {fname}: {changed} px keyed")
    ok += 1

print(f"\nDone: {ok} fixed, {len(missing)} missing.")
if missing:
    print("Missing:")
    for m in missing:
        print(f"  {m}")
