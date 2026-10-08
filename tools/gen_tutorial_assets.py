"""
gen_tutorial_assets.py
----------------------
Extracts the tutorial's pointing arrows from the original movie's 'tutorial'
cast into gfx/, for the World 1 Mission 1 tutorial.

The original flashes each arrow with 'tutorial colorflip': every 4 frames the
sprite's foreground colour swaps between black and rgb(255,128,0), which on
these 1-bit-style bitmaps recolours the outline and leaves the white fill. A
GPU tint cannot do that (it would colour the fill too), so both versions are
written out:

    gfx/tutorial_arrow_<shape>.png          black outline
    gfx/tutorial_arrow_<shape>_orange.png   orange outline

    python tools/gen_tutorial_assets.py [<00WBWORKFOLDER>]
"""

import glob
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
WORK = sys.argv[1] if len(sys.argv) > 1 else r"D:\LEGOWB-Project\00WBWORKFOLDER"
CAST = os.path.join(WORK, r"assets\cast_ripper_d12\worldbuilder")

# Member numbers in the 'tutorial' cast, in the order main.c indexes them.
ARROWS = [(10, "left_down"), (11, "left_up"), (12, "right"), (13, "right_down"), (14, "right_up")]


def matte(im):
    """Director matte ink: white connected to the bounding-box edge is transparent."""
    im = im.convert("RGBA")
    px = im.load()
    w, h = im.size
    seen = bytearray(w * h)
    stack = [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for y in range(h) for x in (0, w - 1)]
    while stack:
        x, y = stack.pop()
        if x < 0 or y < 0 or x >= w or y >= h or seen[y * w + x]:
            continue
        seen[y * w + x] = 1
        p = px[x, y]
        if p[0] < 250 or p[1] < 250 or p[2] < 250:
            continue
        px[x, y] = (0, 0, 0, 0)
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return im


def main():
    for num, shape in ARROWS:
        src = glob.glob(os.path.join(CAST, f"tutorial_{num}_tutorial_arrow_{shape}.png"))[0]
        black = matte(Image.open(src))
        orange = black.copy()
        px = orange.load()
        for y in range(orange.size[1]):
            for x in range(orange.size[0]):
                r, g, b, a = px[x, y]
                if a and r < 128 and g < 128 and b < 128:
                    px[x, y] = (255, 128, 0, a)
        black.save(os.path.join(REPO, "gfx", f"tutorial_arrow_{shape}.png"))
        orange.save(os.path.join(REPO, "gfx", f"tutorial_arrow_{shape}_orange.png"))
        print("wrote", shape, black.size)


if __name__ == "__main__":
    main()
