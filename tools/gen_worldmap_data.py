"""
gen_worldmap_data.py
--------------------
Regenerates everything the world-select screen takes from the original movies,
straight from their Director scores, so none of it has to be estimated by eye:

  include/wb_worldmap_data.h     mission icon points, decorations, mini-units and
                                 mini-unit obstacles for worlds 1-7
  romfs/meta/worldmask_wN.bin    where the mini-units may walk (land / lake)
  gfx/<sprite>.png               world-map sprites the port did not carry yet

Inputs are the ProjectorRays chunk dumps (for the VWSC score and VWLB labels)
and the DirectorCastRipper export (member images, names and regpoints):

    python tools/gen_worldmap_data.py [<00WBWORKFOLDER>]

Score format: Director 7/8 VWSC as ScummVM reads it (engines/director/score.cpp,
frame.cpp readChannelD7): a 288-byte main-channel block, then 48-byte sprite
records whose first 24 bytes share the D6 layout. Frames are delta-coded.

Where things are on the original stage:
  - The world map is the sprite whose 'registered sprite beh' says
    #world_map_sprite. Everything is stored in that bitmap's own pixel space,
    image = stage loc - (map loc - map regpoint), which is what worldMapLayout()
    and missionPointToBottom() in main.c expect.
  - A mission icon is the sprite carrying 'mission icon behavior 2'
    [#world, #mission]; its loc is the icon's rest position (pLoc).
  - Mini-units carry 'mini-unit behavior' [#world, #mission, #bonus, #lake].
  - Sprites carrying 'mini-obstacle behavior' block mini-units with
    rect(left-10, bottom-10-(30 if mountain), right+m, bottom+m), m = 6 on
    world 5 and 10 elsewhere, taken from the sprite's rect at beginSprite.
  - No world-map sprite has the stretch ink bit set, so Director draws each at
    its member's natural size; the score's stored width/height are stale.

The walk masks follow okPos() in 'mini-unit behavior'. The exported 8-bit
images store the palette reversed (Director index n = exported index 255-n,
and n = 1-n for the 1-bit masks), checked against all three WB1 maps.
"""

import csv
import glob
import os
import struct
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
WORK = sys.argv[1] if len(sys.argv) > 1 else r"D:\LEGOWB-Project\00WBWORKFOLDER"

GAMES = {
    "wb1": dict(chunks=os.path.join(WORK, r"analysis\projectorrays_out\worldbuilder\chunks"),
                cast=os.path.join(WORK, r"assets\cast_ripper_d12\worldbuilder"),
                members="{name}_Members.csv", image="{name}_{num}_*.png"),
    "wb2": dict(chunks=os.path.join(WORK, r"analysis\projectorrays_out\worldbuilder2\worldbuilder2\chunks"),
                cast=os.path.join(WORK, r"assets\cast_ripper_d12\worldbuilder2\worldbuilder2"),
                members="{lib}_Members.csv", image="{lib}_{num}.png"),
}
CASTLIBS = {1: "Internal", 2: "title and levels", 3: "audio", 4: "terrain and models", 5: "translation", 6: "tutorial"}
# Port world -> (movie, score label)
WORLDS = {1: ("wb1", "world 1"), 2: ("wb1", "world 2"), 3: ("wb1", "world 3"), 4: ("wb1", "world 4"),
          5: ("wb1", "world 5"), 6: ("wb2", "world 1"), 7: ("wb2", "world 2")}

# Member name -> (gfx png, ASSET_ id). Existing PNGs are left alone; missing ones are extracted.
SPRITES = {
    ("wb1", "question_mark"): ("world_question_mark", "ASSET_WORLD_QUESTION_MARK"),
    ("wb1", "tree"): ("map_tree", "ASSET_MAP_TREE"),
    ("wb1", "mountain"): ("map_mountain", "ASSET_MAP_MOUNTAIN"),
    ("wb1", "worldmap rocks"): ("map_rocks", "ASSET_MAP_ROCKS"),
    ("wb1", "question_mark_blink"): ("question_mark_blink", "ASSET_QUESTION_MARK_BLINK"),
    ("wb1", "flag_rollover_blink"): ("flag_rollover_blink", "ASSET_FLAG_ROLLOVER_BLINK"),
    ("wb1", "buggy_mini"): ("buggy_mini", "ASSET_BUGGY_MINI"),
    ("wb1", "duck_mini"): ("duck_mini", "ASSET_DUCK_MINI"),
    ("wb1", "duck_water_mini"): ("duck_water_mini", "ASSET_DUCK_WATER_MINI"),
    ("wb1", "fish_mini"): ("fish_mini", "ASSET_FISH_MINI"),
    ("wb1", "snail_mini"): ("snail_mini", "ASSET_SNAIL_MINI"),
    ("wb1", "dirtbuggy_mini"): ("dirtbuggy_mini", "ASSET_DIRTBUGGY_MINI"),
    ("wb1", "boat_mini"): ("boat_mini", "ASSET_BOAT_MINI"),
    ("wb1", "forklift_mini"): ("forklift_mini", "ASSET_FORKLIFT_MINI"),
    ("wb1", "steamshovel_mini"): ("steamshovel_mini", "ASSET_STEAMSHOVEL_MINI"),
    ("wb1", "defender.mini"): ("defender_mini", "ASSET_DEFENDER_MINI"),
    ("wb1", "frog.mini"): ("frog_mini", "ASSET_FROG_MINI"),
    ("wb1", "dozer.mini"): ("dozer_mini", "ASSET_DOZER_MINI"),
    ("wb1", "freighter.mini"): ("freighter_mini", "ASSET_FREIGHTER_MINI"),
    ("wb2", "new worldmap rocks"): ("wb2_map_rocks", "ASSET_WB2_MAP_ROCKS"),
    ("wb2", "new worldmap minitree 2"): ("wb2_map_minitree2", "ASSET_WB2_MAP_MINITREE2"),
    ("wb2", "tree_1"): ("wb2_map_tree1", "ASSET_WB2_MAP_TREE1"),
    ("wb2", "tree_2"): ("wb2_map_tree2", "ASSET_WB2_MAP_TREE2"),
}
DECO_NAMES = {"tree", "mountain", "worldmap rocks", "new worldmap rocks", "new worldmap minitree 2", "tree_1", "tree_2"}


def u16(b, o): return struct.unpack_from(">H", b, o)[0]
def s16(b, o): return struct.unpack_from(">h", b, o)[0]
def u32(b, o): return struct.unpack_from(">I", b, o)[0]
def s32(b, o): return struct.unpack_from(">i", b, o)[0]


def chunk(game, tag):
    return open(glob.glob(os.path.join(GAMES[game]["chunks"], f"{tag}-*.bin"))[0], "rb").read()


def labels(game):
    b = chunk(game, "VWLB")
    n = u16(b, 0)
    ents = [(u16(b, 2 + i * 4), u16(b, 4 + i * 4)) for i in range(n)]
    slen = u32(b, 2 + n * 4)
    s = b[6 + n * 4: 6 + n * 4 + slen].decode("latin1")
    return {s[off:(ents[i + 1][1] if i + 1 < n else slen)]: fr for i, (fr, off) in enumerate(ents)}


class Members:
    def __init__(self, game):
        self.game = game
        self.by_key, self.by_name = {}, {}
        g = GAMES[game]
        for lib, name in CASTLIBS.items():
            p = os.path.join(g["cast"], g["members"].format(name=name, lib=lib))
            if not os.path.exists(p):
                continue
            for row in csv.DictReader(open(p, encoding="utf-8", errors="replace")):
                try:
                    num = int(row["Number"])
                except ValueError:
                    continue
                rx, ry = (int(v) for v in row["Registration Point"].strip("()").split(","))
                m = dict(lib=lib, num=num, name=row["Name"], type=row["Type"], reg=(rx, ry))
                self.by_key[(lib, num)] = m
                if row["Type"] == "bitmap":
                    self.by_name.setdefault(row["Name"], m)  # lowest number wins, as member("name") does

    def image(self, m):
        g = GAMES[self.game]
        pat = g["image"].format(name=glob.escape(CASTLIBS[m["lib"]]), lib=m["lib"], num=m["num"])
        return Image.open(glob.glob(os.path.join(g["cast"], pat))[0])


class Score:
    def __init__(self, game):
        b = self.b = chunk(game, "VWSC")
        ls = u32(b, 8)
        n_ent, n_list = s32(b, ls), s32(b, ls + 4)
        idx = ls + 12
        base = idx + n_list * 4
        self.detail = [base + u32(b, idx + i * 4) for i in range(n_ent)]
        p = self.detail[0]
        size, rec, shown = u32(b, p), u16(b, p + 14), u16(b, p + 18)
        assert rec == 48, rec
        self.frames = []
        buf = bytearray(288 + 48 * (shown + 2))
        pos, end = p + 20, p + size
        while pos < end:
            fsize = u16(b, pos)
            if not fsize:
                break
            fend, pos = pos + fsize, pos + 2
            while pos < fend:
                csize, coff = u16(b, pos), u16(b, pos + 2)
                pos += 4
                if coff + csize > len(buf):
                    buf.extend(bytes(coff + csize - len(buf)))
                buf[coff:coff + csize] = b[pos:pos + csize]
                pos += csize
            self.frames.append(bytes(buf))

    def details(self, i):
        return self.b[self.detail[i]:self.detail[i + 1]] if 0 < i < len(self.detail) - 1 else b""

    def sprites(self, frame):
        f = self.frames[frame - 1]
        out = []
        for ch in range(1, (len(f) - 288) // 48 + 1):
            r = f[288 + (ch - 1) * 48: 288 + ch * 48]
            if not u16(r, 6):
                continue
            beh = []
            li = u32(r, 8)
            if li:
                d = self.details(li + 1)
                for i in range(0, len(d) - 7, 8):
                    init = self.details(u32(d, i + 4)).decode("latin1").replace(chr(0), "").strip()
                    beh.append(((u16(d, i), u16(d, i + 2)), props(init)))
            out.append(dict(channel=ch, ink=r[1] & 0x3F, stretch=bool(r[1] & 0x80), lib=s16(r, 4),
                            num=u16(r, 6), locV=s16(r, 12), locH=s16(r, 14), behaviors=beh))
        return out


def props(s):
    out = {}
    for part in s.strip().strip("[]").split(","):
        if ":" in part:
            k, v = (x.strip() for x in part.split(":", 1))
            try:
                out[k.lstrip("#")] = int(v)
            except ValueError:
                out[k.lstrip("#")] = v
    return out


def matte(im):
    """Director matte ink: white connected to the edge of the bounding box is transparent.
    Reproduces the existing gfx/ sprites exactly."""
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


def director_index(im, x, y):
    """Director palette index of an exported 8-bit or 1-bit pixel; None outside the image."""
    if not (0 <= x < im.size[0] and 0 <= y < im.size[1]):
        return None
    v = im.getpixel((x, y))
    return (1 - v) if len(im.getpalette()) // 3 <= 2 else 255 - v


def world_data(w, members, scores, label_maps):
    game, lab = WORLDS[w]
    mem = members[game]
    rows = scores[game].sprites(label_maps[game][lab])
    beh_name = lambda key: mem.by_key.get(key, {}).get("name")
    world_map = None
    for s in rows:
        s["member"] = mem.by_key.get((s["lib"], s["num"]))
        s["beh"] = {beh_name(k): p for k, p in s["behaviors"]}
        if s["beh"].get("registered sprite beh", {}).get("thingy") == "#world_map_sprite":
            world_map = s
    ox = world_map["locH"] - world_map["member"]["reg"][0]
    oy = world_map["locV"] - world_map["member"]["reg"][1]
    W, H = mem.image(world_map["member"]).size
    missions, decos, minis, obstacles = {}, [], [], []
    for s in sorted(rows, key=lambda s: s["channel"]):
        m = s["member"]
        if m is None or m["type"] != "bitmap":
            continue
        x, y = s["locH"] - ox, s["locV"] - oy
        if "mission icon behavior 2" in s["beh"]:
            missions[s["beh"]["mission icon behavior 2"]["mission"]] = (x, y)
        elif "mini-unit behavior" in s["beh"]:
            p = s["beh"]["mini-unit behavior"]
            if s["locH"] > 640:  # parked off stage in the score; okPos() can never let it back on
                continue
            minis.append(dict(name=m["name"], x=x, y=y, reg=m["reg"], mission=p["mission"],
                              bonus=int(p.get("bonus", 0)), lake=int(p.get("lake", 0))))
        elif m["name"] in DECO_NAMES:
            decos.append(dict(name=m["name"], x=x, y=y, reg=m["reg"]))
        if "mini-obstacle behavior" in s["beh"]:
            assert not s["stretch"]
            mw, mh = mem.image(m).size
            left, top = x - m["reg"][0], y - m["reg"][1]
            mountain = 30 if "mountain" in m["name"] else 0
            bm = 6 if (game == "wb1" and lab == "world 5") else 10
            obstacles.append((left - 10, top + mh - 10 - mountain, left + mw + bm, top + mh + bm))
    assert sorted(missions) == list(range(1, 13)), (w, sorted(missions))
    return dict(game=game, label=lab, map=world_map, origin=(ox, oy), size=(W, H),
                missions=missions, decos=decos, minis=minis, obstacles=obstacles)


def walk_mask(w, d, members):
    """bit0: a land mini may stand here; bit1: a lake mini may. Image space of the world map."""
    W, H = d["size"]
    mem = members[d["game"]]
    out = bytearray(W * H)
    if w == 4:  # okPos() sets no terrain rule for the ocean world, only the 380px^2 radius
        return bytearray([3]) * (W * H), True
    if d["game"] == "wb2":
        mask = mem.image(mem.by_name["wb2-map-mask"])  # read at map-image coordinates
        for y in range(H):
            for x in range(W):
                out[y * W + x] = 3 if director_index(mask, x, y) == 1 else 0
        return out, False
    if w == 5:  # member("prehistoric_world_map_guide") is #87, aligned by its own regpoint
        guide = mem.by_name["prehistoric_world_map_guide"]
        gim = mem.image(guide)
        gx = d["map"]["member"]["reg"][0] - guide["reg"][0]
        gy = d["map"]["member"]["reg"][1] - guide["reg"][1]
        for y in range(H):
            for x in range(W):
                i = director_index(gim, x - gx, y - gy)
                out[y * W + x] = (1 if (i is not None and i != 0) else 0) | (2 if i == 2 else 0)
        return out, False
    im = mem.image(d["map"]["member"])
    land_bad = {1: {0, 3, 4}, 2: None, 3: None}[w]
    for y in range(H):
        for x in range(W):
            i = director_index(im, x, y)
            if w == 1:
                land, lake = i not in land_bad, i == 3
            elif w == 2:
                land, lake = i in (3, 4), i == 1
            else:
                land, lake = i == 1, i == 2
            out[y * W + x] = (1 if land else 0) | (2 if lake else 0)
    return out, False


def pack_mask(path, W, H, cells):
    packed = bytearray((W * H + 3) // 4)
    for i, v in enumerate(cells):
        packed[i >> 2] |= (v & 3) << ((i & 3) * 2)
    with open(path, "wb") as f:
        f.write(b"WBMK" + struct.pack("<HH", W, H) + packed)


def main():
    members = {g: Members(g) for g in GAMES}
    scores = {g: Score(g) for g in GAMES}
    label_maps = {g: labels(g) for g in GAMES}
    worlds = {w: world_data(w, members, scores, label_maps) for w in WORLDS}

    # Sprites: extract what gfx/ lacks.
    used = set()
    for w, d in worlds.items():
        used |= {(d["game"] if (d["game"], n["name"]) in SPRITES else "wb1", n["name"]) for n in d["decos"] + d["minis"]}
    used |= {("wb1", "question_mark_blink"), ("wb1", "flag_rollover_blink")}
    for key in sorted(used):
        png, _ = SPRITES[key]
        dst = os.path.join(REPO, "gfx", png + ".png")
        if not os.path.exists(dst):
            mem = members[key[0]]
            matte(mem.image(mem.by_name[key[1]])).save(dst)
            print("extracted", key, "->", os.path.relpath(dst, REPO))

    def asset(game, name):
        return SPRITES[(game, name) if (game, name) in SPRITES else ("wb1", name)][1]

    L = ["/* Generated by tools/gen_worldmap_data.py from the original Director scores.",
         "   Do not edit by hand; rerun the script. All coordinates are in the pixel space",
         "   of the world's map bitmap (worldmapN.png), as worldMapLayout() expects. */",
         "#ifndef WB_WORLDMAP_DATA_H", "#define WB_WORLDMAP_DATA_H", ""]
    L += ["typedef struct WBMapDeco { int assetId; float x; float y; float anchorX; float anchorY; } WBMapDeco;",
          "typedef struct WBMiniDef { int assetId; float x; float y; float anchorX; float anchorY;",
          "                           unsigned char mission; unsigned char bonus; unsigned char lake; } WBMiniDef;",
          "typedef struct WBObstacleRect { float left; float top; float right; float bottom; } WBObstacleRect;",
          "typedef struct WBWorldMapDef {",
          "    float width, height;              /* map bitmap size */",
          "    const WBMapDeco* decos; int decoCount;",
          "    const WBMiniDef* minis; int miniCount;",
          "    const WBObstacleRect* obstacles; int obstacleCount;",
          "    bool obstaclesActive;             /* okPos() skips them on world 5 */",
          "    bool radiusLimited;               /* world 4: stay within sqrt(380) of the start */",
          "} WBWorldMapDef;", ""]
    L.append("static const WBMissionPoint g_worldMissionPoints[8][13] = {")
    L.append("    { {0} },")
    for w in range(1, 8):
        d = worlds[w]
        m = d["map"]
        L.append(f"    /* {d['game']} '{d['label']}': {m['member']['name']} at ({m['locH']},{m['locV']}) reg "
                 f"({m['member']['reg'][0]},{m['member']['reg'][1]}), origin ({d['origin'][0]},{d['origin'][1]}) */")
        pts = [f"{{{d['missions'][i][0]}.0f, {d['missions'][i][1]}.0f}}" for i in range(1, 13)]
        L.append("    { {0}, " + ", ".join(pts[:6]) + ",")
        L.append("           " + ", ".join(pts[6:]) + " }" + ("," if w < 7 else ""))
    L += ["};", ""]
    for w in range(1, 8):
        d = worlds[w]
        if d["decos"]:
            L.append(f"static const WBMapDeco g_world{w}Decos[] = {{")
            for n in d["decos"]:
                L.append(f"    {{{asset(d['game'], n['name'])}, {n['x']}.0f, {n['y']}.0f, {n['reg'][0]}.0f, {n['reg'][1]}.0f}},")
            L += ["};", ""]
        L.append(f"static const WBMiniDef g_world{w}Minis[] = {{")
        for n in d["minis"]:
            L.append(f"    {{{asset(d['game'], n['name'])}, {n['x']}.0f, {n['y']}.0f, {n['reg'][0]}.0f, {n['reg'][1]}.0f, "
                     f"{n['mission']}, {n['bonus']}, {n['lake']}}},")
        L += ["};", ""]
        L.append(f"static const WBObstacleRect g_world{w}Obstacles[] = {{")
        for r in d["obstacles"]:
            L.append(f"    {{{r[0]}.0f, {r[1]}.0f, {r[2]}.0f, {r[3]}.0f}},")
        L += ["};", ""]
    L.append("static const WBWorldMapDef g_worldMapDefs[8] = {")
    L.append("    {0},")
    for w in range(1, 8):
        d = worlds[w]
        decos = f"g_world{w}Decos, {len(d['decos'])}" if d["decos"] else "NULL, 0"
        L.append(f"    {{{d['size'][0]}.0f, {d['size'][1]}.0f, {decos}, g_world{w}Minis, {len(d['minis'])}, "
                 f"g_world{w}Obstacles, {len(d['obstacles'])}, {'false' if w == 5 else 'true'}, {'true' if w == 4 else 'false'}}}"
                 + ("," if w < 7 else ""))
    L += ["};", "", "#endif", ""]
    hdr = os.path.join(REPO, "include", "wb_worldmap_data.h")
    open(hdr, "w", newline="\n").write("\n".join(L))
    print("wrote", os.path.relpath(hdr, REPO))

    for w, d in worlds.items():
        cells, _ = walk_mask(w, d, members)
        path = os.path.join(REPO, "romfs", "meta", f"worldmask_w{w}.bin")
        pack_mask(path, d["size"][0], d["size"][1], cells)
        land = sum(1 for c in cells if c & 1) * 100 // len(cells)
        lake = sum(1 for c in cells if c & 2) * 100 // len(cells)
        print(f"world {w}: {len(d['decos'])} decos, {len(d['minis'])} minis, {len(d['obstacles'])} obstacles, "
              f"mask land {land}% lake {lake}% -> {os.path.relpath(path, REPO)}")


if __name__ == "__main__":
    main()
