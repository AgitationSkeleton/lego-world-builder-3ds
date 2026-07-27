"""
convert_sprites.py
------------------
Converts palette-mode PNGs exported by DirectorCastRipper (cast_ripper_d12)
into RGBA PNGs suitable for the LEGOWB3DS gfx/ folder.

The source sprites use palette index 255 = solid white (255,255,255) as the
background.  Any near-white matte pixels are also keyed out so converted sprites
do not keep the white fringe from Director exports.

Usage:
    python tools/convert_sprites.py [--dry-run]

Set SRC_DIR and DST_DIR constants below, or pass them as positional args:
    python tools/convert_sprites.py <src_dir> <dst_dir> [--dry-run]
"""

import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

SRC_DIR = r"D:\LEGOWB-Project\00WBWORKFOLDER\assets\cast_ripper_d12\worldbuilder"
DST_DIR = r"D:\PsyDoom-Project\LEGOWB3DS\gfx"

# Mapping: source filename (as exported by cast_ripper) => dest filename in gfx/
# All source files live directly in SRC_DIR (no subdirectories).
# The white background (palette index 255 = RGB 255,255,255) is keyed out.
SPRITE_MAP = {
    # ── DIRTBUGGY ───────────────────────────────────────────────────────────
    "terrain and models_122_vehicle.dirtbuggy.up.png":              "vehicle_dirtbuggy_up.png",
    "terrain and models_123_vehicle.dirtbuggy.down.png":            "vehicle_dirtbuggy_down.png",
    "terrain and models_124_vehicle.dirtbuggy.left.png":            "vehicle_dirtbuggy_left.png",
    "terrain and models_125_vehicle.dirtbuggy.right.png":           "vehicle_dirtbuggy_right.png",

    # ── STEAMSHOVEL (idle / dig / full) ─────────────────────────────────────
    "terrain and models_194_vehicle.steamshovel.up.png":            "vehicle_steamshovel_up.png",
    "terrain and models_197_vehicle.steamshovel.down.png":          "vehicle_steamshovel_down.png",
    "terrain and models_188_vehicle.steamshovel.left.png":          "vehicle_steamshovel_left.png",
    "terrain and models_191_vehicle.steamshovel.right.png":         "vehicle_steamshovel_right.png",
    "terrain and models_195_vehicle.steamshovel.up.dig.png":        "vehicle_steamshovel_up_dig.png",
    "terrain and models_198_vehicle.steamshovel.down.dig.png":      "vehicle_steamshovel_down_dig.png",
    "terrain and models_189_vehicle.steamshovel.left.dig.png":      "vehicle_steamshovel_left_dig.png",
    "terrain and models_192_vehicle.steamshovel.right.dig.png":     "vehicle_steamshovel_right_dig.png",
    "terrain and models_196_vehicle.steamshovel.up.full.png":       "vehicle_steamshovel_up_full.png",
    "terrain and models_199_vehicle.steamshovel.down.full.png":     "vehicle_steamshovel_down_full.png",
    "terrain and models_190_vehicle.steamshovel.left.full.png":     "vehicle_steamshovel_left_full.png",
    "terrain and models_193_vehicle.steamshovel.right.full.png":    "vehicle_steamshovel_right_full.png",

    # ── DUMPTRUCK ───────────────────────────────────────────────────────────
    "terrain and models_128_vehicle.dumptruck.up.png":              "vehicle_dumptruck_up.png",
    "terrain and models_129_vehicle.dumptruck.down.png":            "vehicle_dumptruck_down.png",
    "terrain and models_130_vehicle.dumptruck.left.png":            "vehicle_dumptruck_left.png",
    "terrain and models_131_vehicle.dumptruck.right.png":           "vehicle_dumptruck_right.png",

    # ── FORKLIFT ────────────────────────────────────────────────────────────
    "terrain and models_110_vehicle.forklift.up.png":               "vehicle_forklift_up.png",
    "terrain and models_111_vehicle.forklift.down.png":             "vehicle_forklift_down.png",
    "terrain and models_112_vehicle.forklift.left.png":             "vehicle_forklift_left.png",
    "terrain and models_113_vehicle.forklift.right.png":            "vehicle_forklift_right.png",

    # ── DOZER ───────────────────────────────────────────────────────────────
    "terrain and models_118_vehicle.dozer.up.png":                  "vehicle_dozer_up.png",
    "terrain and models_119_vehicle.dozer.down.png":                "vehicle_dozer_down.png",
    "terrain and models_120_vehicle.dozer.left.png":                "vehicle_dozer_left.png",
    "terrain and models_121_vehicle.dozer.right.png":               "vehicle_dozer_right.png",

    # ── SPEEDBOAT ───────────────────────────────────────────────────────────
    "terrain and models_63_vehicle.speedboat.up.png":               "vehicle_speedboat_up.png",
    "terrain and models_64_vehicle.speedboat.down.png":             "vehicle_speedboat_down.png",
    "terrain and models_66_vehicle.speedboat.left.png":             "vehicle_speedboat_left.png",
    "terrain and models_65_vehicle.speedboat.right.png":            "vehicle_speedboat_right.png",
    "terrain and models_62_vehicle.speedboat.hero.png":             "vehicle_speedboat_hero.png",

    # ── TUGBOAT ─────────────────────────────────────────────────────────────
    "terrain and models_133_vehicle.tugboat.up.png":                "vehicle_tugboat_up.png",
    "terrain and models_134_vehicle.tugboat.down.png":              "vehicle_tugboat_down.png",
    "terrain and models_136_vehicle.tugboat.left.png":              "vehicle_tugboat_left.png",
    "terrain and models_135_vehicle.tugboat.right.png":             "vehicle_tugboat_right.png",

    # ── FREIGHTER ───────────────────────────────────────────────────────────
    "terrain and models_141_vehicle.freighter.up.png":              "vehicle_freighter_up.png",
    "terrain and models_143_vehicle.freighter.down.png":            "vehicle_freighter_down.png",
    "terrain and models_142_vehicle.freighter.left.png":            "vehicle_freighter_left.png",
    "terrain and models_144_vehicle.freighter.right.png":           "vehicle_freighter_right.png",

    # ── FROG (land + jump + water + water-jump) ──────────────────────────────
    "terrain and models_175_vehicle.frog.up.png":                   "vehicle_frog_up.png",
    "terrain and models_173_vehicle.frog.down.png":                 "vehicle_frog_down.png",
    "terrain and models_177_vehicle.frog.left.png":                 "vehicle_frog_left.png",
    "terrain and models_171_vehicle.frog.right.png":                "vehicle_frog_right.png",
    "terrain and models_176_vehicle.frog.up.jump.png":              "vehicle_frog_up_jump.png",
    "terrain and models_174_vehicle.frog.down.jump.png":            "vehicle_frog_down_jump.png",
    "terrain and models_178_vehicle.frog.left.jump.png":            "vehicle_frog_left_jump.png",
    "terrain and models_172_vehicle.frog.right.jump.png":           "vehicle_frog_right_jump.png",
    "terrain and models_183_vehicle.frog.water.up.png":             "vehicle_frog_water_up.png",
    "terrain and models_181_vehicle.frog.water.down.png":           "vehicle_frog_water_down.png",
    "terrain and models_185_vehicle.frog.water.left.png":           "vehicle_frog_water_left.png",
    "terrain and models_179_vehicle.frog.water.right.png":          "vehicle_frog_water_right.png",
    "terrain and models_184_vehicle.frog.water.up.jump.png":        "vehicle_frog_water_up_jump.png",
    "terrain and models_182_vehicle.frog.water.down.jump.png":      "vehicle_frog_water_down_jump.png",
    "terrain and models_186_vehicle.frog.water.left.jump.png":      "vehicle_frog_water_left_jump.png",
    "terrain and models_180_vehicle.frog.water.right.jump.png":     "vehicle_frog_water_right_jump.png",

    # ── FISH (water-only directionals) ──────────────────────────────────────
    "terrain and models_157_vehicle.fish.water.up.png":             "vehicle_fish_water_up.png",
    "terrain and models_158_vehicle.fish.water.down.png":           "vehicle_fish_water_down.png",
    "terrain and models_155_vehicle.fish.water.left.png":           "vehicle_fish_water_left.png",
    "terrain and models_156_vehicle.fish.water.right.png":          "vehicle_fish_water_right.png",

    # ── SNAIL ───────────────────────────────────────────────────────────────
    "terrain and models_150_vehicle.snail.up.png":                  "vehicle_snail_up.png",
    "terrain and models_151_vehicle.snail.down.png":                "vehicle_snail_down.png",
    "terrain and models_148_vehicle.snail.left.png":                "vehicle_snail_left.png",
    "terrain and models_149_vehicle.snail.right.png":               "vehicle_snail_right.png",

    # ── TREEBOT idle ────────────────────────────────────────────────────────
    "terrain and models_229_vehicle.treebot.up.png":                "vehicle_treebot_up.png",
    "terrain and models_232_vehicle.treebot.down.png":              "vehicle_treebot_down.png",
    "terrain and models_230_vehicle.treebot.left.png":              "vehicle_treebot_left.png",
    "terrain and models_231_vehicle.treebot.right.png":             "vehicle_treebot_right.png",

    # ── TREEBOT walk (up, 6 frames) ──────────────────────────────────────────
    "terrain and models_223_vehicle.treebot.up.walk.1.png":         "vehicle_treebot_up_walk1.png",
    "terrain and models_224_vehicle.treebot.up.walk.2.png":         "vehicle_treebot_up_walk2.png",
    "terrain and models_225_vehicle.treebot.up.walk.3.png":         "vehicle_treebot_up_walk3.png",
    "terrain and models_226_vehicle.treebot.up.walk.4.png":         "vehicle_treebot_up_walk4.png",
    "terrain and models_227_vehicle.treebot.up.walk.5.png":         "vehicle_treebot_up_walk5.png",
    "terrain and models_228_vehicle.treebot.up.walk.6.png":         "vehicle_treebot_up_walk6.png",

    # ── TREEBOT walk (down, 6 frames) ────────────────────────────────────────
    "terrain and models_205_vehicle.treebot.down.walk.1.png":       "vehicle_treebot_down_walk1.png",
    "terrain and models_206_vehicle.treebot.down.walk.2.png":       "vehicle_treebot_down_walk2.png",
    "terrain and models_207_vehicle.treebot.down.walk.3.png":       "vehicle_treebot_down_walk3.png",
    "terrain and models_208_vehicle.treebot.down.walk.4.png":       "vehicle_treebot_down_walk4.png",
    "terrain and models_209_vehicle.treebot.down.walk.5.png":       "vehicle_treebot_down_walk5.png",
    "terrain and models_210_vehicle.treebot.down.walk.6.png":       "vehicle_treebot_down_walk6.png",

    # ── TREEBOT walk (left, 6 frames) ─────────────────────────────────────────
    "terrain and models_211_vehicle.treebot.left.walk.1.png":       "vehicle_treebot_left_walk1.png",
    "terrain and models_212_vehicle.treebot.left.walk.2.png":       "vehicle_treebot_left_walk2.png",
    "terrain and models_213_vehicle.treebot.left.walk.3.png":       "vehicle_treebot_left_walk3.png",
    "terrain and models_214_vehicle.treebot.left.walk.4.png":       "vehicle_treebot_left_walk4.png",
    "terrain and models_215_vehicle.treebot.left.walk.5.png":       "vehicle_treebot_left_walk5.png",
    "terrain and models_216_vehicle.treebot.left.walk.6.png":       "vehicle_treebot_left_walk6.png",

    # ── TREEBOT walk (right, 6 frames) ────────────────────────────────────────
    "terrain and models_217_vehicle.treebot.right.walk.1.png":      "vehicle_treebot_right_walk1.png",
    "terrain and models_218_vehicle.treebot.right.walk.2.png":      "vehicle_treebot_right_walk2.png",
    "terrain and models_219_vehicle.treebot.right.walk.3.png":      "vehicle_treebot_right_walk3.png",
    "terrain and models_220_vehicle.treebot.right.walk.4.png":      "vehicle_treebot_right_walk4.png",
    "terrain and models_221_vehicle.treebot.right.walk.5.png":      "vehicle_treebot_right_walk5.png",
    "terrain and models_222_vehicle.treebot.right.walk.6.png":      "vehicle_treebot_right_walk6.png",
    "terrain and models_233_vehicle.treebot.up.lift.1.png":         "vehicle_treebot_up_lift1.png",
    "terrain and models_234_vehicle.treebot.up.lift.2.png":         "vehicle_treebot_up_lift2.png",
    "terrain and models_237_vehicle.treebot.down.lift.1.png":       "vehicle_treebot_down_lift1.png",
    "terrain and models_238_vehicle.treebot.down.lift.2.png":       "vehicle_treebot_down_lift2.png",
    "terrain and models_235_vehicle.treebot.left.lift.1.png":       "vehicle_treebot_left_lift1.png",
    "terrain and models_236_vehicle.treebot.left.lift.2.png":       "vehicle_treebot_left_lift2.png",
    "terrain and models_239_vehicle.treebot.right.lift.1.png":      "vehicle_treebot_right_lift1.png",
    "terrain and models_240_vehicle.treebot.right.lift.2.png":      "vehicle_treebot_right_lift2.png",

    # ── DEFENDER ────────────────────────────────────────────────────────────
    "terrain and models_302_vehicle.defender.up.png":               "vehicle_defender_up.png",
    "terrain and models_303_vehicle.defender.down.png":             "vehicle_defender_down.png",
    "terrain and models_304_vehicle.defender.left.png":             "vehicle_defender_left.png",
    "terrain and models_305_vehicle.defender.right.png":            "vehicle_defender_right.png",
    "terrain and models_285_vehicle.defender.up.walk.1.png":        "vehicle_defender_up_walk1.png",
    "terrain and models_286_vehicle.defender.up.walk.2.png":        "vehicle_defender_up_walk2.png",
    "terrain and models_287_vehicle.defender.up.walk.3.png":        "vehicle_defender_up_walk3.png",
    "terrain and models_288_vehicle.defender.up.walk.4.png":        "vehicle_defender_up_walk4.png",
    "terrain and models_282_vehicle.defender.down.walk.1.png":      "vehicle_defender_down_walk1.png",
    "terrain and models_281_vehicle.defender.down.walk.2.png":      "vehicle_defender_down_walk2.png",
    "terrain and models_283_vehicle.defender.down.walk.3.png":      "vehicle_defender_down_walk3.png",
    "terrain and models_284_vehicle.defender.down.walk.4.png":      "vehicle_defender_down_walk4.png",
    "terrain and models_277_vehicle.defender.left.walk.1.png":      "vehicle_defender_left_walk1.png",
    "terrain and models_278_vehicle.defender.left.walk.2.png":      "vehicle_defender_left_walk2.png",
    "terrain and models_279_vehicle.defender.left.walk.3.png":      "vehicle_defender_left_walk3.png",
    "terrain and models_280_vehicle.defender.left.walk.4.png":      "vehicle_defender_left_walk4.png",
    "terrain and models_273_vehicle.defender.right.walk.1.png":     "vehicle_defender_right_walk1.png",
    "terrain and models_274_vehicle.defender.right.walk.2.png":     "vehicle_defender_right_walk2.png",
    "terrain and models_275_vehicle.defender.right.walk.3.png":     "vehicle_defender_right_walk3.png",
    "terrain and models_276_vehicle.defender.right.walk.4.png":     "vehicle_defender_right_walk4.png",
    "terrain and models_79_vehicle.defender.hero.png":              "vehicle_defender_hero.png",

    # ── REPAIRBOT ───────────────────────────────────────────────────────────
    "terrain and models_562_vehicle.repairBot.up.png":              "vehicle_repairbot_up.png",
    "terrain and models_563_vehicle.repairBot.down.png":            "vehicle_repairbot_down.png",
    "terrain and models_564_vehicle.repairBot.left.png":            "vehicle_repairbot_left.png",
    "terrain and models_561_vehicle.repairBot.right.png":           "vehicle_repairbot_right.png",
    "terrain and models_578_vehicle.repairBot.up.walk.1.png":       "vehicle_repairbot_up_walk1.png",
    "terrain and models_582_vehicle.repairBot.up.walk.2.png":       "vehicle_repairbot_up_walk2.png",
    "terrain and models_579_vehicle.repairBot.down.walk.1.png":     "vehicle_repairbot_down_walk1.png",
    "terrain and models_583_vehicle.repairBot.down.walk.2.png":     "vehicle_repairbot_down_walk2.png",
    "terrain and models_580_vehicle.repairBot.left.walk.1.png":     "vehicle_repairbot_left_walk1.png",
    "terrain and models_584_vehicle.repairBot.left.walk.2.png":     "vehicle_repairbot_left_walk2.png",
    "terrain and models_577_vehicle.repairBot.right.walk.1.png":    "vehicle_repairbot_right_walk1.png",
    "terrain and models_581_vehicle.repairBot.right.walk.2.png":    "vehicle_repairbot_right_walk2.png",
    "terrain and models_570_vehicle.repairBot.up.repair.1.png":     "vehicle_repairbot_up_repair1.png",
    "terrain and models_574_vehicle.repairBot.up.repair.2.png":     "vehicle_repairbot_up_repair2.png",
    "terrain and models_571_vehicle.repairBot.down.repair.1.png":   "vehicle_repairbot_down_repair1.png",
    "terrain and models_575_vehicle.repairBot.down.repair.2.png":   "vehicle_repairbot_down_repair2.png",
    "terrain and models_572_vehicle.repairBot.left.repair.1.png":   "vehicle_repairbot_left_repair1.png",
    "terrain and models_576_vehicle.repairBot.left.repair.2.png":   "vehicle_repairbot_left_repair2.png",
    "terrain and models_569_vehicle.repairBot.right.repair.1.png":  "vehicle_repairbot_right_repair1.png",
    "terrain and models_573_vehicle.repairBot.right.repair.2.png":  "vehicle_repairbot_right_repair2.png",
    "terrain and models_567_vehicle.repairBot.hero.png":            "vehicle_repairbot_hero.png",

    # ── TREEBOT full-carry variants ────────────────────────────────────────
    "terrain and models_241_vehicle.treebot.up.full.png":           "vehicle_treebot_up_full.png",
    "terrain and models_244_vehicle.treebot.down.full.png":         "vehicle_treebot_down_full.png",
    "terrain and models_242_vehicle.treebot.left.full.png":         "vehicle_treebot_left_full.png",
    "terrain and models_243_vehicle.treebot.right.full.png":        "vehicle_treebot_right_full.png",
    "terrain and models_245_vehicle.treebot.up.full.walk.1.png":    "vehicle_treebot_up_full_walk1.png",
    "terrain and models_246_vehicle.treebot.up.full.walk.2.png":    "vehicle_treebot_up_full_walk2.png",
    "terrain and models_247_vehicle.treebot.up.full.walk.3.png":    "vehicle_treebot_up_full_walk3.png",
    "terrain and models_248_vehicle.treebot.up.full.walk.4.png":    "vehicle_treebot_up_full_walk4.png",
    "terrain and models_253_vehicle.treebot.down.full.walk.1.png":  "vehicle_treebot_down_full_walk1.png",
    "terrain and models_254_vehicle.treebot.down.full.walk.2.png":  "vehicle_treebot_down_full_walk2.png",
    "terrain and models_255_vehicle.treebot.down.full.walk.3.png":  "vehicle_treebot_down_full_walk3.png",
    "terrain and models_256_vehicle.treebot.down.full.walk.4.png":  "vehicle_treebot_down_full_walk4.png",
    "terrain and models_257_vehicle.treebot.left.full.walk.1.png":  "vehicle_treebot_left_full_walk1.png",
    "terrain and models_258_vehicle.treebot.left.full.walk.2.png":  "vehicle_treebot_left_full_walk2.png",
    "terrain and models_259_vehicle.treebot.left.full.walk.3.png":  "vehicle_treebot_left_full_walk3.png",
    "terrain and models_260_vehicle.treebot.left.full.walk.4.png":  "vehicle_treebot_left_full_walk4.png",
    "terrain and models_249_vehicle.treebot.right.full.walk.1.png": "vehicle_treebot_right_full_walk1.png",
    "terrain and models_250_vehicle.treebot.right.full.walk.2.png": "vehicle_treebot_right_full_walk2.png",
    "terrain and models_251_vehicle.treebot.right.full.walk.3.png": "vehicle_treebot_right_full_walk3.png",
    "terrain and models_252_vehicle.treebot.right.full.walk.4.png": "vehicle_treebot_right_full_walk4.png",
    "terrain and models_263_vehicle.treebot.up.full.lift.1.png":    "vehicle_treebot_up_full_lift1.png",
    "terrain and models_264_vehicle.treebot.up.full.lift.2.png":    "vehicle_treebot_up_full_lift2.png",
    "terrain and models_267_vehicle.treebot.down.full.lift.1.png":  "vehicle_treebot_down_full_lift1.png",
    "terrain and models_268_vehicle.treebot.down.full.lift.2.png":  "vehicle_treebot_down_full_lift2.png",
    "terrain and models_261_vehicle.treebot.left.full.lift.1.png":  "vehicle_treebot_left_full_lift1.png",
    "terrain and models_262_vehicle.treebot.left.full.lift.2.png":  "vehicle_treebot_left_full_lift2.png",
    "terrain and models_265_vehicle.treebot.right.full.lift.1.png": "vehicle_treebot_right_full_lift1.png",
    "terrain and models_266_vehicle.treebot.right.full.lift.2.png": "vehicle_treebot_right_full_lift2.png",

    # ── PLANS ────────────────────────────────────────────────────────────────
    "terrain and models_98_vehicle.dirtbuggy.plan.png":             "plan_dirtbuggy.png",
    "terrain and models_103_vehicle.steamshovel.plan.png":          "plan_steamshovel.png",
    "terrain and models_71_vehicle.dumptruck.plan.png":             "plan_dumptruck.png",
    "terrain and models_101_vehicle.forklift.plan.png":             "plan_forklift.png",
    "terrain and models_70_vehicle.dozer.plan.png":                 "plan_dozer.png",
    "terrain and models_140_vehicle.speedboat.plan.png":            "plan_speedboat.png",
    "terrain and models_106_vehicle.tugboat.plan.png":              "plan_tugboat.png",
    "terrain and models_109_vehicle.freighter.plan.png":            "plan_freighter.png",
    "terrain and models_69_vehicle.frog.plan.png":                  "plan_frog.png",
    "terrain and models_100_vehicle.fish.plan.png":                 "plan_fish.png",
    "terrain and models_102_vehicle.snail.plan.png":                "plan_snail.png",
    "terrain and models_104_vehicle.treebot.plan.png":              "plan_treebot.png",
    "terrain and models_97_vehicle.defender.plan.png":              "plan_defender.png",
    "terrain and models_555_vehicle.repairBot.plan.png":            "plan_repairbot.png",

    # ── MONSTERS: CRAB ──────────────────────────────────────────────────────
    "terrain and models_307_monster.crab.up.png":                   "monster_crab_up.png",
    "terrain and models_312_monster.crab.down.png":                 "monster_crab_down.png",
    "terrain and models_317_monster.crab.left.png":                 "monster_crab_left.png",
    "terrain and models_322_monster.crab.right.png":                "monster_crab_right.png",
    "terrain and models_308_monster.crab.up.walk.1.png":            "monster_crab_up_walk1.png",
    "terrain and models_309_monster.crab.up.walk.2.png":            "monster_crab_up_walk2.png",
    "terrain and models_310_monster.crab.up.walk.3.png":            "monster_crab_up_walk3.png",
    "terrain and models_311_monster.crab.up.walk.4.png":            "monster_crab_up_walk4.png",
    "terrain and models_313_monster.crab.down.walk.1.png":          "monster_crab_down_walk1.png",
    "terrain and models_314_monster.crab.down.walk.2.png":          "monster_crab_down_walk2.png",
    "terrain and models_315_monster.crab.down.walk.3.png":          "monster_crab_down_walk3.png",
    "terrain and models_316_monster.crab.down.walk.4.png":          "monster_crab_down_walk4.png",
    "terrain and models_318_monster.crab.left.walk.1.png":          "monster_crab_left_walk1.png",
    "terrain and models_319_monster.crab.left.walk.2.png":          "monster_crab_left_walk2.png",
    "terrain and models_320_monster.crab.left.walk.3.png":          "monster_crab_left_walk3.png",
    "terrain and models_321_monster.crab.left.walk.4.png":          "monster_crab_left_walk4.png",
    "terrain and models_323_monster.crab.right.walk.1.png":         "monster_crab_right_walk1.png",
    "terrain and models_324_monster.crab.right.walk.2.png":         "monster_crab_right_walk2.png",
    "terrain and models_325_monster.crab.right.walk.3.png":         "monster_crab_right_walk3.png",
    "terrain and models_326_monster.crab.right.walk.4.png":         "monster_crab_right_walk4.png",
    "terrain and models_327_monster.crab.up.attack.1.png":          "monster_crab_up_atk1.png",
    "terrain and models_328_monster.crab.up.attack.2.png":          "monster_crab_up_atk2.png",
    "terrain and models_329_monster.crab.down.attack.1.png":        "monster_crab_down_atk1.png",
    "terrain and models_330_monster.crab.down.attack.2.png":        "monster_crab_down_atk2.png",
    "terrain and models_331_monster.crab.left.attack.1.png":        "monster_crab_left_atk1.png",
    "terrain and models_332_monster.crab.left.attack.2.png":        "monster_crab_left_atk2.png",
    "terrain and models_333_monster.crab.right.attack.1.png":       "monster_crab_right_atk1.png",
    "terrain and models_334_monster.crab.right.attack.2.png":       "monster_crab_right_atk2.png",

    # ── MONSTERS: WATER_CRAB ────────────────────────────────────────────────
    "terrain and models_402_monster.water_crab.up.png":             "monster_water_crab_up.png",
    "terrain and models_387_monster.water_crab.down.png":           "monster_water_crab_down.png",
    "terrain and models_392_monster.water_crab.left.png":           "monster_water_crab_left.png",
    "terrain and models_397_monster.water_crab.right.png":          "monster_water_crab_right.png",
    "terrain and models_403_monster.water_crab.up.walk.1.png":      "monster_water_crab_up_walk1.png",
    "terrain and models_404_monster.water_crab.up.walk.2.png":      "monster_water_crab_up_walk2.png",
    "terrain and models_405_monster.water_crab.up.walk.3.png":      "monster_water_crab_up_walk3.png",
    "terrain and models_406_monster.water_crab.up.walk.4.png":      "monster_water_crab_up_walk4.png",
    "terrain and models_388_monster.water_crab.down.walk.1.png":    "monster_water_crab_down_walk1.png",
    "terrain and models_389_monster.water_crab.down.walk.2.png":    "monster_water_crab_down_walk2.png",
    "terrain and models_390_monster.water_crab.down.walk.3.png":    "monster_water_crab_down_walk3.png",
    "terrain and models_391_monster.water_crab.down.walk.4.png":    "monster_water_crab_down_walk4.png",
    "terrain and models_393_monster.water_crab.left.walk.1.png":    "monster_water_crab_left_walk1.png",
    "terrain and models_394_monster.water_crab.left.walk.2.png":    "monster_water_crab_left_walk2.png",
    "terrain and models_395_monster.water_crab.left.walk.3.png":    "monster_water_crab_left_walk3.png",
    "terrain and models_396_monster.water_crab.left.walk.4.png":    "monster_water_crab_left_walk4.png",
    "terrain and models_398_monster.water_crab.right.walk.1.png":   "monster_water_crab_right_walk1.png",
    "terrain and models_399_monster.water_crab.right.walk.2.png":   "monster_water_crab_right_walk2.png",
    "terrain and models_400_monster.water_crab.right.walk.3.png":   "monster_water_crab_right_walk3.png",
    "terrain and models_401_monster.water_crab.right.walk.4.png":   "monster_water_crab_right_walk4.png",
    "terrain and models_413_monster.water_crab.up.attack.1.png":    "monster_water_crab_up_atk1.png",
    "terrain and models_414_monster.water_crab.up.attack.2.png":    "monster_water_crab_up_atk2.png",
    "terrain and models_407_monster.water_crab.down.attack.1.png":  "monster_water_crab_down_atk1.png",
    "terrain and models_408_monster.water_crab.down.attack.2.png":  "monster_water_crab_down_atk2.png",
    "terrain and models_409_monster.water_crab.left.attack.1.png":  "monster_water_crab_left_atk1.png",
    "terrain and models_410_monster.water_crab.left.attack.2.png":  "monster_water_crab_left_atk2.png",
    "terrain and models_411_monster.water_crab.right.attack.1.png": "monster_water_crab_right_atk1.png",
    "terrain and models_412_monster.water_crab.right.attack.2.png": "monster_water_crab_right_atk2.png",

    # ── MONSTERS: SCORPION ──────────────────────────────────────────────────
    "terrain and models_436_monster.scorpion.up.png":               "monster_scorpion_up.png",
    "terrain and models_438_monster.scorpion.down.png":             "monster_scorpion_down.png",
    "terrain and models_437_monster.scorpion.left.png":             "monster_scorpion_left.png",
    "terrain and models_439_monster.scorpion.right.png":            "monster_scorpion_right.png",
    "terrain and models_422_monster.scorpion.up.walk.1.png":        "monster_scorpion_up_walk1.png",
    "terrain and models_423_monster.scorpion.up.walk.2.png":        "monster_scorpion_up_walk2.png",
    "terrain and models_424_monster.scorpion.up.walk.3.png":        "monster_scorpion_up_walk3.png",
    "terrain and models_419_monster.scorpion.down.walk.1.png":      "monster_scorpion_down_walk1.png",
    "terrain and models_420_monster.scorpion.down.walk.2.png":      "monster_scorpion_down_walk2.png",
    "terrain and models_421_monster.scorpion.down.walk.3.png":      "monster_scorpion_down_walk3.png",
    "terrain and models_425_monster.scorpion.left.walk.1.png":      "monster_scorpion_left_walk1.png",
    "terrain and models_426_monster.scorpion.left.walk.2.png":      "monster_scorpion_left_walk2.png",
    "terrain and models_427_monster.scorpion.left.walk.3.png":      "monster_scorpion_left_walk3.png",
    "terrain and models_416_monster.scorpion.right.walk.1.png":     "monster_scorpion_right_walk1.png",
    "terrain and models_417_monster.scorpion.right.walk.2.png":     "monster_scorpion_right_walk2.png",
    "terrain and models_418_monster.scorpion.right.walk.3.png":     "monster_scorpion_right_walk3.png",
    "terrain and models_434_monster.scorpion.up.attack.1.png":      "monster_scorpion_up_atk1.png",
    "terrain and models_435_monster.scorpion.up.attack.2.png":      "monster_scorpion_up_atk2.png",
    "terrain and models_430_monster.scorpion.down.attack.1.png":    "monster_scorpion_down_atk1.png",
    "terrain and models_431_monster.scorpion.down.attack.2.png":    "monster_scorpion_down_atk2.png",
    "terrain and models_432_monster.scorpion.left.attack.1.png":    "monster_scorpion_left_atk1.png",
    "terrain and models_433_monster.scorpion.left.attack.2.png":    "monster_scorpion_left_atk2.png",
    "terrain and models_428_monster.scorpion.right.attack.1.png":   "monster_scorpion_right_atk1.png",
    "terrain and models_429_monster.scorpion.right.attack.2.png":   "monster_scorpion_right_atk2.png",

    # ── MONSTERS: SHARK ─────────────────────────────────────────────────────
    "terrain and models_487_monster.shark.up.png":                  "monster_shark_up.png",
    "terrain and models_484_monster.shark.down.png":                "monster_shark_down.png",
    "terrain and models_485_monster.shark.left.png":                "monster_shark_left.png",
    "terrain and models_486_monster.shark.right.png":               "monster_shark_right.png",
    "terrain and models_504_monster.shark.up.walk.1.png":           "monster_shark_up_walk1.png",
    "terrain and models_505_monster.shark.up.walk.2.png":           "monster_shark_up_walk2.png",
    "terrain and models_512_monster.shark.up.walk.3.png":           "monster_shark_up_walk3.png",
    "terrain and models_500_monster.shark.down.walk.1.png":         "monster_shark_down_walk1.png",
    "terrain and models_501_monster.shark.down.walk.2.png":         "monster_shark_down_walk2.png",
    "terrain and models_508_monster.shark.down.walk.3.png":         "monster_shark_down_walk3.png",
    "terrain and models_506_monster.shark.left.walk.1.png":         "monster_shark_left_walk1.png",
    "terrain and models_507_monster.shark.left.walk.2.png":         "monster_shark_left_walk2.png",
    "terrain and models_514_monster.shark.left.walk.3.png":         "monster_shark_left_walk3.png",
    "terrain and models_502_monster.shark.right.walk.1.png":        "monster_shark_right_walk1.png",
    "terrain and models_503_monster.shark.right.walk.2.png":        "monster_shark_right_walk2.png",
    "terrain and models_510_monster.shark.right.walk.3.png":        "monster_shark_right_walk3.png",
    "terrain and models_494_monster.shark.up.attack.1.png":         "monster_shark_up_atk1.png",
    "terrain and models_495_monster.shark.up.attack.2.png":         "monster_shark_up_atk2.png",
    "terrain and models_488_monster.shark.down.attack.1.png":       "monster_shark_down_atk1.png",
    "terrain and models_489_monster.shark.down.attack.2.png":       "monster_shark_down_atk2.png",
    "terrain and models_490_monster.shark.left.attack.1.png":       "monster_shark_left_atk1.png",
    "terrain and models_491_monster.shark.left.attack.2.png":       "monster_shark_left_atk2.png",
    "terrain and models_492_monster.shark.right.attack.1.png":      "monster_shark_right_atk1.png",
    "terrain and models_493_monster.shark.right.attack.2.png":      "monster_shark_right_atk2.png",

    # ── MONSTERS: BOULDER ───────────────────────────────────────────────────
    "terrain and models_339_monster.boulder.png":                   "monster_boulder.png",
    "terrain and models_342_monster.boulder.up.png":                "monster_boulder_up.png",
    "terrain and models_343_monster.boulder.down.png":              "monster_boulder_down.png",
    "terrain and models_340_monster.boulder.left.bmp":              "monster_boulder_left.png",
    "terrain and models_341_monster.boulder.right.bmp":             "monster_boulder_right.png",

    # ── MONSTERS: TREX ──────────────────────────────────────────────────────
    "terrain and models_598_monster.trex.up.png":                   "monster_trex_up.png",
    "terrain and models_599_monster.trex.down.png":                 "monster_trex_down.png",
    "terrain and models_600_monster.trex.left.png":                 "monster_trex_left.png",
    "terrain and models_597_monster.trex.right.png":                "monster_trex_right.png",
    "terrain and models_602_monster.trex.up.walk.1.png":            "monster_trex_up_walk1.png",
    "terrain and models_606_monster.trex.up.walk.2.png":            "monster_trex_up_walk2.png",
    "terrain and models_610_monster.trex.up.walk.3.png":            "monster_trex_up_walk3.png",
    "terrain and models_614_monster.trex.up.walk.4.png":            "monster_trex_up_walk4.png",
    "terrain and models_618_monster.trex.up.walk.5.png":            "monster_trex_up_walk5.png",
    "terrain and models_622_monster.trex.up.walk.6.png":            "monster_trex_up_walk6.png",
    "terrain and models_626_monster.trex.up.walk.7.png":            "monster_trex_up_walk7.png",
    "terrain and models_630_monster.trex.up.walk.8.png":            "monster_trex_up_walk8.png",
    "terrain and models_634_monster.trex.up.walk.9.png":            "monster_trex_up_walk9.png",
    "terrain and models_638_monster.trex.up.walk.10.png":           "monster_trex_up_walk10.png",
    "terrain and models_603_monster.trex.down.walk.1.png":          "monster_trex_down_walk1.png",
    "terrain and models_607_monster.trex.down.walk.2.png":          "monster_trex_down_walk2.png",
    "terrain and models_611_monster.trex.down.walk.3.png":          "monster_trex_down_walk3.png",
    "terrain and models_615_monster.trex.down.walk.4.png":          "monster_trex_down_walk4.png",
    "terrain and models_619_monster.trex.down.walk.5.png":          "monster_trex_down_walk5.png",
    "terrain and models_623_monster.trex.down.walk.6.png":          "monster_trex_down_walk6.png",
    "terrain and models_627_monster.trex.down.walk.7.png":          "monster_trex_down_walk7.png",
    "terrain and models_631_monster.trex.down.walk.8.png":          "monster_trex_down_walk8.png",
    "terrain and models_635_monster.trex.down.walk.9.png":          "monster_trex_down_walk9.png",
    "terrain and models_639_monster.trex.down.walk.10.png":         "monster_trex_down_walk10.png",
    "terrain and models_604_monster.trex.left.walk.1.png":          "monster_trex_left_walk1.png",
    "terrain and models_608_monster.trex.left.walk.2.png":          "monster_trex_left_walk2.png",
    "terrain and models_612_monster.trex.left.walk.3.png":          "monster_trex_left_walk3.png",
    "terrain and models_616_monster.trex.left.walk.4.png":          "monster_trex_left_walk4.png",
    "terrain and models_620_monster.trex.left.walk.5.png":          "monster_trex_left_walk5.png",
    "terrain and models_624_monster.trex.left.walk.6.png":          "monster_trex_left_walk6.png",
    "terrain and models_628_monster.trex.left.walk.7.png":          "monster_trex_left_walk7.png",
    "terrain and models_632_monster.trex.left.walk.8.png":          "monster_trex_left_walk8.png",
    "terrain and models_636_monster.trex.left.walk.9.png":          "monster_trex_left_walk9.png",
    "terrain and models_640_monster.trex.left.walk.10.png":         "monster_trex_left_walk10.png",
    "terrain and models_601_monster.trex.right.walk.1.png":         "monster_trex_right_walk1.png",
    "terrain and models_605_monster.trex.right.walk.2.png":         "monster_trex_right_walk2.png",
    "terrain and models_609_monster.trex.right.walk.3.png":         "monster_trex_right_walk3.png",
    "terrain and models_613_monster.trex.right.walk.4.png":         "monster_trex_right_walk4.png",
    "terrain and models_617_monster.trex.right.walk.5.png":         "monster_trex_right_walk5.png",
    "terrain and models_621_monster.trex.right.walk.6.png":         "monster_trex_right_walk6.png",
    "terrain and models_625_monster.trex.right.walk.7.png":         "monster_trex_right_walk7.png",
    "terrain and models_629_monster.trex.right.walk.8.png":         "monster_trex_right_walk8.png",
    "terrain and models_633_monster.trex.right.walk.9.png":         "monster_trex_right_walk9.png",
    "terrain and models_637_monster.trex.right.walk.10.png":        "monster_trex_right_walk10.png",
    "terrain and models_642_monster.trex.up.attack.1.png":          "monster_trex_up_atk1.png",
    "terrain and models_646_monster.trex.up.attack.2.png":          "monster_trex_up_atk2.png",
    "terrain and models_643_monster.trex.down.attack.1.png":        "monster_trex_down_atk1.png",
    "terrain and models_647_monster.trex.down.attack.2.png":        "monster_trex_down_atk2.png",
    "terrain and models_644_monster.trex.left.attack.1.png":        "monster_trex_left_atk1.png",
    "terrain and models_648_monster.trex.left.attack.2.png":        "monster_trex_left_atk2.png",
    "terrain and models_641_monster.trex.right.attack.1.png":       "monster_trex_right_atk1.png",
    "terrain and models_645_monster.trex.right.attack.2.png":       "monster_trex_right_atk2.png",

    # ── MONSTERS: GATOR (land) ──────────────────────────────────────────────
    "terrain and models_344_monster.gator.up.png":                  "monster_gator_up.png",
    "terrain and models_346_monster.gator.down.png":                "monster_gator_down.png",
    "terrain and models_347_monster.gator.left.png":                "monster_gator_left.png",
    "terrain and models_345_monster.gator.right.png":               "monster_gator_right.png",
    "terrain and models_348_monster.gator.up.walk.1.png":           "monster_gator_up_walk1.png",
    "terrain and models_349_monster.gator.up.walk.2.png":           "monster_gator_up_walk2.png",
    "terrain and models_352_monster.gator.down.walk.1.png":         "monster_gator_down_walk1.png",
    "terrain and models_353_monster.gator.down.walk.2.png":         "monster_gator_down_walk2.png",
    "terrain and models_354_monster.gator.left.walk.1.png":         "monster_gator_left_walk1.png",
    "terrain and models_355_monster.gator.left.walk.2.png":         "monster_gator_left_walk2.png",
    "terrain and models_350_monster.gator.right.walk.1.png":        "monster_gator_right_walk1.png",
    "terrain and models_351_monster.gator.right.walk.2.png":        "monster_gator_right_walk2.png",
    "terrain and models_356_monster.gator.up.attack.1.png":         "monster_gator_up_atk1.png",
    "terrain and models_357_monster.gator.up.attack.2.png":         "monster_gator_up_atk2.png",
    "terrain and models_360_monster.gator.down.attack.1.png":       "monster_gator_down_atk1.png",
    "terrain and models_361_monster.gator.down.attack.2.png":       "monster_gator_down_atk2.png",
    "terrain and models_362_monster.gator.left.attack.1.png":       "monster_gator_left_atk1.png",
    "terrain and models_363_monster.gator.left.attack.2.png":       "monster_gator_left_atk2.png",
    "terrain and models_358_monster.gator.right.attack.1.png":      "monster_gator_right_atk1.png",
    "terrain and models_359_monster.gator.right.attack.2.png":      "monster_gator_right_atk2.png",

    # ── MONSTERS: GATOR (water) ─────────────────────────────────────────────
    "terrain and models_364_monster.gator.water.up.png":            "monster_gator_water_up.png",
    "terrain and models_366_monster.gator.water.down.png":          "monster_gator_water_down.png",
    "terrain and models_367_monster.gator.water.left.png":          "monster_gator_water_left.png",
    "terrain and models_365_monster.gator.water.right.png":         "monster_gator_water_right.png",
    "terrain and models_368_monster.gator.water.up.walk.1.png":     "monster_gator_water_up_walk1.png",
    "terrain and models_369_monster.gator.water.up.walk.2.png":     "monster_gator_water_up_walk2.png",
    "terrain and models_372_monster.gator.water.down.walk.1.png":   "monster_gator_water_down_walk1.png",
    "terrain and models_373_monster.gator.water.down.walk.2.png":   "monster_gator_water_down_walk2.png",
    "terrain and models_374_monster.gator.water.left.walk.1.png":   "monster_gator_water_left_walk1.png",
    "terrain and models_375_monster.gator.water.left.walk.2.png":   "monster_gator_water_left_walk2.png",
    "terrain and models_370_monster.gator.water.right.walk.1.png":  "monster_gator_water_right_walk1.png",
    "terrain and models_371_monster.gator.water.right.walk.2.png":  "monster_gator_water_right_walk2.png",
    "terrain and models_376_monster.gator.water.up.attack.1.png":   "monster_gator_water_up_atk1.png",
    "terrain and models_377_monster.gator.water.up.attack.2.png":   "monster_gator_water_up_atk2.png",
    "terrain and models_380_monster.gator.water.down.attack.1.png": "monster_gator_water_down_atk1.png",
    "terrain and models_381_monster.gator.water.down.attack.2.png": "monster_gator_water_down_atk2.png",
    "terrain and models_382_monster.gator.water.left.attack.1.png": "monster_gator_water_left_atk1.png",
    "terrain and models_383_monster.gator.water.left.attack.2.png": "monster_gator_water_left_atk2.png",
    "terrain and models_378_monster.gator.water.right.attack.1.png":"monster_gator_water_right_atk1.png",
    "terrain and models_379_monster.gator.water.right.attack.2.png":"monster_gator_water_right_atk2.png",

    # ── BUILDINGS ────────────────────────────────────────────────────────────
    "terrain and models_126_building.gas_station.png":              "building_gas_station.png",
    "terrain and models_58_building.guard_tower.png":               "building_guard_tower.png",
    "terrain and models_533_building.guard_tower.up.png":           "building_guard_tower_up.png",
    "terrain and models_530_building.guard_tower.down.png":         "building_guard_tower_down.png",
    "terrain and models_531_building.guard_tower.left.png":         "building_guard_tower_left.png",
    "terrain and models_532_building.guard_tower.right.png":        "building_guard_tower_right.png",
    "terrain and models_67_building.marina.png":                    "building_marina.png",
    "terrain and models_68_building.robot_lab.png":                 "building_robot_lab.png",

    # ── WHIRLPOOL ────────────────────────────────────────────────────────────
    "terrain and models_17_whirlpool.generic.png":                  "whirlpool_static.png",
    "terrain and models_18_whirlpool.generic.whirl.1.png":          "whirlpool_whirl1.png",
    "terrain and models_19_whirlpool.generic.whirl.2.png":          "whirlpool_whirl2.png",
    "terrain and models_20_whirlpool.generic.whirl.3.png":          "whirlpool_whirl3.png",
    "terrain and models_21_whirlpool.generic.whirl.4.png":          "whirlpool_whirl4.png",

    # ── BUILD FX ─────────────────────────────────────────────────────────────
    "Internal_401_build_cloud1.png":                                "build_cloud1.png",
    "Internal_402_build_cloud2.png":                                "build_cloud2.png",

    # ── DAMAGE / DISASSEMBLY EFFECTS ───────────────────────────────────────
    "Internal_497_damage.small.1.png":                              "damage_small_1.png",
    "Internal_498_damage.small.2.png":                              "damage_small_2.png",
    "Internal_499_damage.small.3.png":                              "damage_small_3.png",
    "Internal_500_damage.small.4.png":                              "damage_small_4.png",
    "Internal_501_damage.small.5.png":                              "damage_small_5.png",
    "Internal_403_take_apart_cloud1.png":                           "take_apart_cloud1.png",
    "Internal_404_take_apart_cloud2.png":                           "take_apart_cloud2.png",
    "Internal_405_take_apart_cloud3.png":                           "take_apart_cloud3.png",
}



def convert_file(src_path, dst_path, dry_run=False):
    """Convert a palette-mode PNG with near-white background to RGBA with transparent bg."""
    img = Image.open(src_path).convert("RGBA")
    pixels = img.load()
    w, h = img.size
    changed = 0
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if a != 0 and r >= 248 and g >= 248 and b >= 248:
                pixels[x, y] = (255, 255, 255, 0)
                changed += 1
    if not dry_run:
        img.save(dst_path)
    return changed


def main():
    args = sys.argv[1:]
    dry_run = "--dry-run" in args
    if dry_run:
        args = [a for a in args if a != "--dry-run"]

    src_dir = args[0] if len(args) > 0 else SRC_DIR
    dst_dir = args[1] if len(args) > 1 else DST_DIR

    if not dry_run:
        os.makedirs(dst_dir, exist_ok=True)

    ok = 0
    missing = []

    for src_name, dst_name in SPRITE_MAP.items():
        src_path = os.path.join(src_dir, src_name)
        dst_path = os.path.join(dst_dir, dst_name)
        if not os.path.exists(src_path):
            missing.append(src_name)
            continue
        n = convert_file(src_path, dst_path, dry_run)
        mode = "[DRY] " if dry_run else ""
        print(f"  {mode}{src_name}\n      -> {dst_name}  ({n} px keyed)")
        ok += 1

    print(f"\nDone: {ok} converted, {len(missing)} missing.")
    if missing:
        print("Missing source files:")
        for m in missing:
            print(f"  {m}")


if __name__ == "__main__":
    main()
