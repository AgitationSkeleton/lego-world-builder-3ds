#include <3ds.h>
#include <citro2d.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "wb_map.h"

#define TOP_W 400.0f
#define TOP_H 240.0f
#define BOTTOM_W 320.0f
#define BOTTOM_H 240.0f

#define VIEW_TILE_W 12
#define VIEW_TILE_H 9
#define DRAW_PAD 2

#define DIRECTOR_TILE_SIZE_X 50.0f
#define DIRECTOR_TILE_SIZE_Y 50.0f
#define DIRECTOR_PIXEL_TOPLEFT_X -58.0f
#define DIRECTOR_PIXEL_TOPLEFT_Y -13.0f
#define DIRECTOR_PIXEL_SKEW_X 25.0f
#define DIRECTOR_SCALE 0.5f
#define WB_SAVE_PATH "sdmc:/legowb3ds/save.txt"

/* Konami code sequence: Up Up Down Down Left Right Left Right B A Start (11 keys) */
static const u32 g_konamiSeq[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A, KEY_START
};
#define WB_KONAMI_LEN 11
#define WORLD_OFFSET_X 18.0f
#define WORLD_OFFSET_Y 24.0f
#define PICKUP_SCALE 0.32f
#define RESOURCE_WORLD_SCALE DIRECTOR_SCALE
#define PLAN_WORLD_SCALE DIRECTOR_SCALE
#define MUSIC_CHANNEL 0
#define SFX_CHANNEL   1
#define SFX2_CHANNEL  1  /* use same channel as SFX; channel 2 unreliable in emulators */
#define WB_MAX_WORLD_EFFECTS 32
#define WB_DAMAGE_EFFECT_DURATION_MS 320
#define WB_CLOUD_EFFECT_FRAME_MS 70
#define WB_MUSIC_INTRO_VARIANTS 2
#define WB_MUSIC_GAME6_VARIANTS 4
#define WB_MUSIC_GAME8_VARIANTS 3
#define WB_MUSIC_GAMEA_VARIANTS 3
#define WB_MUSIC_GAMEI_VARIANTS 3

typedef enum WBAssetId {
    ASSET_TERRAIN_NORMAL = 0,
    ASSET_TERRAIN_TREE,
    ASSET_TERRAIN_WATER,
    ASSET_TERRAIN_WATER_REEFS,
    ASSET_TERRAIN_WATER_UNDIGGABLE,
    ASSET_TERRAIN_NORMAL_UNDIGGABLE,
    ASSET_TERRAIN_MOUNTAIN,
    ASSET_TERRAIN_SWAMP,
    ASSET_TERRAIN_BILLBOARD,
    ASSET_VEHICLE_BUGGY,
    ASSET_VEHICLE_DUCK,
    ASSET_PLAN_BUGGY,
    ASSET_RESOURCE_RED,
    ASSET_RESOURCE_BLUE,
    ASSET_RESOURCE_YELLOW,
    ASSET_RESOURCE_WHEEL,
    ASSET_RESOURCE_ENERGY,
    ASSET_GOAL_MAIN,
    ASSET_GOAL_BONUS,
    ASSET_MINI_NORMAL,
    ASSET_MINI_TREE,
    ASSET_MINI_NORMAL_UNDIGGABLE,
    ASSET_MINI_MOUNTAIN,
    ASSET_MINI_WATER,
    ASSET_MINI_WATER_UNDIGGABLE,
    ASSET_MINI_WATER_REEFS,
    ASSET_MINI_RESOURCE,
    ASSET_MINI_PLAN,
    ASSET_MINI_VEHICLE,
    ASSET_MINI_HOLE,
    ASSET_MINI_SWAMP,
    ASSET_MINI_HIGHLIGHT,
    ASSET_SKY1,
    ASSET_UI_ENERGY_ICON,
    ASSET_UI_ENERGY_STRIPE_BG,
    ASSET_UI_UNIT_INFO_SEPARATOR,
    ASSET_UI_NO_ENERGY,
    ASSET_VEHICLE_BUGGY_HERO,
    ASSET_VEHICLE_DUCK_HERO,
    ASSET_PLAN_GENERIC,
    ASSET_PLAN_UNKNOWN,
    ASSET_RESOURCE_RED_1,
    ASSET_RESOURCE_RED_2,
    ASSET_RESOURCE_RED_3,
    ASSET_RESOURCE_RED_4,
    ASSET_RESOURCE_BLUE_1,
    ASSET_RESOURCE_BLUE_2,
    ASSET_RESOURCE_BLUE_3,
    ASSET_RESOURCE_BLUE_4,
    ASSET_RESOURCE_GREEN_1,
    ASSET_RESOURCE_GREEN_2,
    ASSET_RESOURCE_GREEN_3,
    ASSET_RESOURCE_GREEN_4,
    ASSET_RESOURCE_YELLOW_1,
    ASSET_RESOURCE_YELLOW_2,
    ASSET_RESOURCE_YELLOW_3,
    ASSET_RESOURCE_YELLOW_4,
    ASSET_RESOURCE_WHEEL_FULL,
    ASSET_RESOURCE_ENERGY_FULL,
    ASSET_RESOURCE_ENERGY_LOW,
    ASSET_RESOURCE_ENERGY_DEAD,
    ASSET_VEHICLE_BUGGY_UP,
    ASSET_VEHICLE_BUGGY_DOWN,
    ASSET_VEHICLE_BUGGY_LEFT,
    ASSET_VEHICLE_BUGGY_RIGHT,
    ASSET_VEHICLE_DUCK_UP,
    ASSET_VEHICLE_DUCK_DOWN,
    ASSET_VEHICLE_DUCK_LEFT,
    ASSET_VEHICLE_DUCK_RIGHT,
    ASSET_VEHICLE_DUCK_WATER_UP,
    ASSET_VEHICLE_DUCK_WATER_DOWN,
    ASSET_VEHICLE_DUCK_WATER_LEFT,
    ASSET_VEHICLE_DUCK_WATER_RIGHT,
    ASSET_OBJECT_HIGHLIGHT,
    ASSET_PLAN_HIGHLIGHT,
    ASSET_BUILD_YES,
    ASSET_BUILD_NO,
    ASSET_CHECK_MARK,
    ASSET_X_MARK,
    ASSET_MENU_PANEL,
    ASSET_BUILD_OUTLINE,
    ASSET_BUILD_OUTLINE_FRONT,
    ASSET_BUILD_OUTLINE_REAR,
    ASSET_BUILD_OUTLINE_L1,
    ASSET_BUILD_OUTLINE_L2,
    ASSET_BUILD_OUTLINE_R1,
    ASSET_BUILD_OUTLINE_R2,
    ASSET_GOAL_BUBBLE,
    ASSET_GOAL_COMPLETE_BUBBLE,
    ASSET_BONUS_GOAL_COMPLETE_BUBBLE,
    ASSET_DAMAGE_SMALL_1,
    ASSET_DAMAGE_SMALL_2,
    ASSET_DAMAGE_SMALL_3,
    ASSET_DAMAGE_SMALL_4,
    ASSET_DAMAGE_SMALL_5,
    ASSET_TAKE_APART_CLOUD_1,
    ASSET_TAKE_APART_CLOUD_2,
    ASSET_TAKE_APART_CLOUD_3,
    ASSET_BUILD_CLOUD_1,
    ASSET_BUILD_CLOUD_2,
    ASSET_TITLE_LOGO,
    ASSET_TITLE_FINAL_IMAGE,
    ASSET_TITLE_WB2_IMAGE,
    ASSET_TITLE_SKY1,
    ASSET_TITLE_SKY2,
    ASSET_TITLE_SKY3,
    ASSET_TITLE_SKY4,
    ASSET_TITLE_SKY5,
    ASSET_WORLDMAP1,
    ASSET_WORLDMAP2,
    ASSET_WORLDMAP3,
    ASSET_WORLDMAP4,
    ASSET_WORLDMAP5,
    ASSET_WORLD_QUESTION_MARK,
    ASSET_WORLD_QUESTION_MARK_SHADOW,
    ASSET_OCEAN_QUESTION_MARK_SHADOW,
    ASSET_WORLD_FLAG1,
    ASSET_WORLD_FLAG2,
    ASSET_WORLD_FLAG3,
    ASSET_WORLD_FLAG4,
    ASSET_WORLD_FLAG5,
    ASSET_WORLD_FLAG6,
    ASSET_WORLD_BONUS_FLAG1,
    ASSET_WORLD_BONUS_FLAG2,
    ASSET_WORLD_BONUS_FLAG3,
    ASSET_WORLD_BONUS_FLAG4,
    ASSET_WORLD_BONUS_FLAG5,
    ASSET_WORLD_BONUS_FLAG6,
    ASSET_OCEAN_FLAG1,
    ASSET_OCEAN_FLAG2,
    ASSET_OCEAN_FLAG3,
    ASSET_OCEAN_FLAG4,
    ASSET_OCEAN_FLAG5,
    ASSET_OCEAN_FLAG6,
    ASSET_OCEAN_BONUS_FLAG1,
    ASSET_OCEAN_BONUS_FLAG2,
    ASSET_OCEAN_BONUS_FLAG3,
    ASSET_OCEAN_BONUS_FLAG4,
    ASSET_OCEAN_BONUS_FLAG5,
    ASSET_OCEAN_BONUS_FLAG6,
    ASSET_PREV_WORLD_ARROW,
    ASSET_NEXT_WORLD_ARROW,
    ASSET_NEXT_WORLD_ARROW_FRAME,
    ASSET_NEXT_WORLD_ARROW_FRAME2,
    ASSET_BUGGY_MINI,
    ASSET_DUCK_MINI,
    ASSET_DUCK_WATER_MINI,
    ASSET_FISH_MINI,
    ASSET_SNAIL_MINI,
    ASSET_MAP_MOUNTAIN,
    ASSET_MAP_ROCKS,
    ASSET_MAP_TREE,
    /* Appended to worldicons.t3s for the world-select screen (tools/gen_worldmap_data.py). */
    ASSET_QUESTION_MARK_BLINK,
    ASSET_FLAG_ROLLOVER_BLINK,
    ASSET_DIRTBUGGY_MINI,
    ASSET_BOAT_MINI,
    ASSET_FORKLIFT_MINI,
    ASSET_STEAMSHOVEL_MINI,
    ASSET_DEFENDER_MINI,
    ASSET_FROG_MINI,
    ASSET_DOZER_MINI,
    ASSET_FREIGHTER_MINI,
    ASSET_WB2_MAP_ROCKS,
    ASSET_WB2_MAP_MINITREE2,
    ASSET_WB2_MAP_TREE1,
    ASSET_WB2_MAP_TREE2,

    /* ── vehicles_land.t3x ── index 0 from this sheet ── */
    ASSET_VEHICLES_LAND_BASE,
    ASSET_VEHICLE_DIRTBUGGY_UP = ASSET_VEHICLES_LAND_BASE,
    ASSET_VEHICLE_DIRTBUGGY_DOWN,
    ASSET_VEHICLE_DIRTBUGGY_LEFT,
    ASSET_VEHICLE_DIRTBUGGY_RIGHT,
    ASSET_VEHICLE_STEAMSHOVEL_UP,
    ASSET_VEHICLE_STEAMSHOVEL_DOWN,
    ASSET_VEHICLE_STEAMSHOVEL_LEFT,
    ASSET_VEHICLE_STEAMSHOVEL_RIGHT,
    ASSET_VEHICLE_STEAMSHOVEL_UP_DIG,
    ASSET_VEHICLE_STEAMSHOVEL_DOWN_DIG,
    ASSET_VEHICLE_STEAMSHOVEL_LEFT_DIG,
    ASSET_VEHICLE_STEAMSHOVEL_RIGHT_DIG,
    ASSET_VEHICLE_STEAMSHOVEL_UP_FULL,
    ASSET_VEHICLE_STEAMSHOVEL_DOWN_FULL,
    ASSET_VEHICLE_STEAMSHOVEL_LEFT_FULL,
    ASSET_VEHICLE_STEAMSHOVEL_RIGHT_FULL,
    ASSET_VEHICLE_DUMPTRUCK_UP,
    ASSET_VEHICLE_DUMPTRUCK_DOWN,
    ASSET_VEHICLE_DUMPTRUCK_LEFT,
    ASSET_VEHICLE_DUMPTRUCK_RIGHT,
    ASSET_VEHICLE_FORKLIFT_UP,
    ASSET_VEHICLE_FORKLIFT_DOWN,
    ASSET_VEHICLE_FORKLIFT_LEFT,
    ASSET_VEHICLE_FORKLIFT_RIGHT,
    ASSET_VEHICLE_DOZER_UP,
    ASSET_VEHICLE_DOZER_DOWN,
    ASSET_VEHICLE_DOZER_LEFT,
    ASSET_VEHICLE_DOZER_RIGHT,
    ASSET_VEHICLE_SPEEDBOAT_UP,
    ASSET_VEHICLE_SPEEDBOAT_DOWN,
    ASSET_VEHICLE_SPEEDBOAT_LEFT,
    ASSET_VEHICLE_SPEEDBOAT_RIGHT,
    ASSET_VEHICLE_SPEEDBOAT_HERO,
    ASSET_VEHICLE_DIRTBUGGY_HERO,
    ASSET_VEHICLE_STEAMSHOVEL_HERO,
    ASSET_VEHICLE_DUMPTRUCK_HERO,
    ASSET_VEHICLE_FORKLIFT_HERO,
    ASSET_VEHICLE_DOZER_HERO,
    ASSET_PLAN_DIRTBUGGY,
    ASSET_PLAN_STEAMSHOVEL,
    ASSET_PLAN_DUMPTRUCK,
    ASSET_PLAN_FORKLIFT,
    ASSET_PLAN_DOZER,
    ASSET_PLAN_SPEEDBOAT,

    /* ── vehicles_water.t3x ── index 0 from this sheet ── */
    ASSET_VEHICLES_WATER_BASE,
    ASSET_VEHICLE_TUGBOAT_UP = ASSET_VEHICLES_WATER_BASE,
    ASSET_VEHICLE_TUGBOAT_DOWN,
    ASSET_VEHICLE_TUGBOAT_LEFT,
    ASSET_VEHICLE_TUGBOAT_RIGHT,
    ASSET_VEHICLE_FREIGHTER_UP,
    ASSET_VEHICLE_FREIGHTER_DOWN,
    ASSET_VEHICLE_FREIGHTER_LEFT,
    ASSET_VEHICLE_FREIGHTER_RIGHT,
    ASSET_VEHICLE_TUGBOAT_HERO,
    ASSET_VEHICLE_FREIGHTER_HERO,
    ASSET_PLAN_TUGBOAT,
    ASSET_PLAN_FREIGHTER,

    /* ── vehicles_animal.t3x ── index 0 from this sheet ── */
    ASSET_VEHICLES_ANIMAL_BASE,
    ASSET_VEHICLE_FROG_UP = ASSET_VEHICLES_ANIMAL_BASE,
    ASSET_VEHICLE_FROG_DOWN,
    ASSET_VEHICLE_FROG_LEFT,
    ASSET_VEHICLE_FROG_RIGHT,
    ASSET_VEHICLE_FROG_UP_JUMP,
    ASSET_VEHICLE_FROG_DOWN_JUMP,
    ASSET_VEHICLE_FROG_LEFT_JUMP,
    ASSET_VEHICLE_FROG_RIGHT_JUMP,
    ASSET_VEHICLE_FROG_WATER_UP,
    ASSET_VEHICLE_FROG_WATER_DOWN,
    ASSET_VEHICLE_FROG_WATER_LEFT,
    ASSET_VEHICLE_FROG_WATER_RIGHT,
    ASSET_VEHICLE_FROG_WATER_UP_JUMP,
    ASSET_VEHICLE_FROG_WATER_DOWN_JUMP,
    ASSET_VEHICLE_FROG_WATER_LEFT_JUMP,
    ASSET_VEHICLE_FROG_WATER_RIGHT_JUMP,
    ASSET_VEHICLE_FISH_WATER_UP,
    ASSET_VEHICLE_FISH_WATER_DOWN,
    ASSET_VEHICLE_FISH_WATER_LEFT,
    ASSET_VEHICLE_FISH_WATER_RIGHT,
    ASSET_VEHICLE_SNAIL_UP,
    ASSET_VEHICLE_SNAIL_DOWN,
    ASSET_VEHICLE_SNAIL_LEFT,
    ASSET_VEHICLE_SNAIL_RIGHT,
    ASSET_VEHICLE_FROG_HERO,
    ASSET_VEHICLE_FISH_HERO,
    ASSET_VEHICLE_SNAIL_HERO,
    ASSET_PLAN_FROG,
    ASSET_PLAN_FISH,
    ASSET_PLAN_SNAIL,

    /* ── vehicles_robot.t3x ── index 0 from this sheet ── */
    ASSET_VEHICLES_ROBOT_BASE,
    ASSET_VEHICLE_TREEBOT_UP = ASSET_VEHICLES_ROBOT_BASE,
    ASSET_VEHICLE_TREEBOT_DOWN,
    ASSET_VEHICLE_TREEBOT_LEFT,
    ASSET_VEHICLE_TREEBOT_RIGHT,
    ASSET_VEHICLE_TREEBOT_UP_WALK1,
    ASSET_VEHICLE_TREEBOT_UP_WALK2,
    ASSET_VEHICLE_TREEBOT_UP_WALK3,
    ASSET_VEHICLE_TREEBOT_UP_WALK4,
    ASSET_VEHICLE_TREEBOT_UP_WALK5,
    ASSET_VEHICLE_TREEBOT_UP_WALK6,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK1,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK2,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK3,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK4,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK5,
    ASSET_VEHICLE_TREEBOT_DOWN_WALK6,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK1,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK2,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK3,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK4,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK5,
    ASSET_VEHICLE_TREEBOT_LEFT_WALK6,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK1,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK2,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK3,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK4,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK5,
    ASSET_VEHICLE_TREEBOT_RIGHT_WALK6,
    ASSET_VEHICLE_TREEBOT_UP_LIFT1,
    ASSET_VEHICLE_TREEBOT_UP_LIFT2,
    ASSET_VEHICLE_TREEBOT_DOWN_LIFT1,
    ASSET_VEHICLE_TREEBOT_DOWN_LIFT2,
    ASSET_VEHICLE_TREEBOT_LEFT_LIFT1,
    ASSET_VEHICLE_TREEBOT_LEFT_LIFT2,
    ASSET_VEHICLE_TREEBOT_RIGHT_LIFT1,
    ASSET_VEHICLE_TREEBOT_RIGHT_LIFT2,
    ASSET_VEHICLE_TREEBOT_UP_FULL,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL,
    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK1,
    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK2,
    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK3,
    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK4,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK1,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK2,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK3,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK4,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK1,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK2,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK3,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK4,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK1,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK2,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK3,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK4,
    ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT1,
    ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT2,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT1,
    ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT2,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT1,
    ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT2,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT1,
    ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT2,
    ASSET_VEHICLE_TREEBOT_HERO,
    ASSET_VEHICLE_DEFENDER_UP,
    ASSET_VEHICLE_DEFENDER_DOWN,
    ASSET_VEHICLE_DEFENDER_LEFT,
    ASSET_VEHICLE_DEFENDER_RIGHT,
    ASSET_VEHICLE_DEFENDER_UP_WALK1,
    ASSET_VEHICLE_DEFENDER_UP_WALK2,
    ASSET_VEHICLE_DEFENDER_UP_WALK3,
    ASSET_VEHICLE_DEFENDER_UP_WALK4,
    ASSET_VEHICLE_DEFENDER_DOWN_WALK1,
    ASSET_VEHICLE_DEFENDER_DOWN_WALK2,
    ASSET_VEHICLE_DEFENDER_DOWN_WALK3,
    ASSET_VEHICLE_DEFENDER_DOWN_WALK4,
    ASSET_VEHICLE_DEFENDER_LEFT_WALK1,
    ASSET_VEHICLE_DEFENDER_LEFT_WALK2,
    ASSET_VEHICLE_DEFENDER_LEFT_WALK3,
    ASSET_VEHICLE_DEFENDER_LEFT_WALK4,
    ASSET_VEHICLE_DEFENDER_RIGHT_WALK1,
    ASSET_VEHICLE_DEFENDER_RIGHT_WALK2,
    ASSET_VEHICLE_DEFENDER_RIGHT_WALK3,
    ASSET_VEHICLE_DEFENDER_RIGHT_WALK4,
    ASSET_VEHICLE_DEFENDER_HERO,
    ASSET_VEHICLE_REPAIRBOT_UP,
    ASSET_VEHICLE_REPAIRBOT_DOWN,
    ASSET_VEHICLE_REPAIRBOT_LEFT,
    ASSET_VEHICLE_REPAIRBOT_RIGHT,
    ASSET_VEHICLE_REPAIRBOT_UP_WALK1,
    ASSET_VEHICLE_REPAIRBOT_UP_WALK2,
    ASSET_VEHICLE_REPAIRBOT_DOWN_WALK1,
    ASSET_VEHICLE_REPAIRBOT_DOWN_WALK2,
    ASSET_VEHICLE_REPAIRBOT_LEFT_WALK1,
    ASSET_VEHICLE_REPAIRBOT_LEFT_WALK2,
    ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK1,
    ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK2,
    ASSET_VEHICLE_REPAIRBOT_UP_REPAIR1,
    ASSET_VEHICLE_REPAIRBOT_UP_REPAIR2,
    ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR1,
    ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR2,
    ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR1,
    ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR2,
    ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR1,
    ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR2,
    ASSET_VEHICLE_REPAIRBOT_HERO,
    ASSET_PLAN_TREEBOT,
    ASSET_PLAN_DEFENDER,
    ASSET_PLAN_REPAIRBOT,

    /* ── monsters_a.t3x (crab=28, water_crab=28, scorpion=24, shark=24, boulder=5 = 109) ── */
    ASSET_MONSTERS_A_BASE,
    /* assets accessed via ASSET_MONSTERS_A_BASE + offset; see WB_MON_A_* macros */
    ASSET_MONSTERS_A_SENTINEL = ASSET_MONSTERS_A_BASE + 108,

    /* ── monsters_b.t3x (trex=52, gator_land=20, gator_water=20 = 92) ── */
    ASSET_MONSTERS_B_BASE = ASSET_MONSTERS_A_BASE + 109,
    /* assets accessed via ASSET_MONSTERS_B_BASE + offset; see WB_MON_B_* macros */
    ASSET_MONSTERS_B_SENTINEL = ASSET_MONSTERS_B_BASE + 91,

    /* ── buildings.t3x ── */
    ASSET_BUILDINGS_BASE = ASSET_MONSTERS_B_BASE + 92,
    ASSET_BUILDING_GAS_STATION = ASSET_BUILDINGS_BASE,
    ASSET_BUILDING_GUARD_TOWER,
    ASSET_BUILDING_GUARD_TOWER_UP,
    ASSET_BUILDING_GUARD_TOWER_DOWN,
    ASSET_BUILDING_GUARD_TOWER_LEFT,
    ASSET_BUILDING_GUARD_TOWER_RIGHT,
    ASSET_BUILDING_MARINA,
    ASSET_BUILDING_ROBOT_LAB,
    ASSET_BUILDING_GAS_STATION_HERO,
    ASSET_BUILDING_MARINA_HERO,
    ASSET_BUILDING_ROBOT_LAB_HERO,
    ASSET_BUILDING_GUARD_TOWER_HERO,
    ASSET_PLAN_GAS_STATION,
    ASSET_PLAN_GUARD_TOWER,
    ASSET_PLAN_MARINA,
    ASSET_PLAN_ROBOT_LAB,
    ASSET_PLAN_DUCK,

    /* ── whirlpool.t3x ── */
    ASSET_WHIRLPOOL_BASE,
    ASSET_WHIRLPOOL_STATIC = ASSET_WHIRLPOOL_BASE,
    ASSET_WHIRLPOOL_WHIRL1,
    ASSET_WHIRLPOOL_WHIRL2,
    ASSET_WHIRLPOOL_WHIRL3,
    ASSET_WHIRLPOOL_WHIRL4,

    /* ── wb2_monsters.t3x (lion=23 + 7 frozen = 30) ── */
    ASSET_WB2_MONSTERS_BASE,
    ASSET_MONSTER_LION = ASSET_WB2_MONSTERS_BASE,
    ASSET_MONSTER_LION_DOWN,
    ASSET_MONSTER_LION_DOWN_WALK1,
    ASSET_MONSTER_LION_DOWN_WALK2,
    ASSET_MONSTER_LION_DOWN_ATK1,
    ASSET_MONSTER_LION_DOWN_ATK2,
    ASSET_MONSTER_LION_LEFT,
    ASSET_MONSTER_LION_LEFT_WALK1,
    ASSET_MONSTER_LION_LEFT_WALK2,
    ASSET_MONSTER_LION_LEFT_ATK1,
    ASSET_MONSTER_LION_LEFT_ATK2,
    ASSET_MONSTER_LION_RIGHT,
    ASSET_MONSTER_LION_RIGHT_WALK1,
    ASSET_MONSTER_LION_RIGHT_WALK2,
    ASSET_MONSTER_LION_RIGHT_ATK1,
    ASSET_MONSTER_LION_RIGHT_ATK2,
    ASSET_MONSTER_LION_UP,
    ASSET_MONSTER_LION_UP_WALK1,
    ASSET_MONSTER_LION_UP_WALK2,
    ASSET_MONSTER_LION_UP_ATK1,
    ASSET_MONSTER_LION_UP_ATK2,
    ASSET_MONSTER_LION_HERO,
    ASSET_PLAN_LION,
    ASSET_MONSTER_CRAB_FROZEN,
    ASSET_MONSTER_GATOR_FROZEN,
    ASSET_MONSTER_LION_FROZEN,
    ASSET_MONSTER_SCORPION_FROZEN,
    ASSET_MONSTER_SHARK_FROZEN,
    ASSET_MONSTER_TREX_FROZEN,
    ASSET_MONSTER_WATER_CRAB_FROZEN,

    /* ── wb2_vehicles.t3x (freezebot full set = 31) ── */
    ASSET_WB2_VEHICLES_BASE,
    ASSET_VEHICLE_FREEZEBOT = ASSET_WB2_VEHICLES_BASE,
    ASSET_VEHICLE_FREEZEBOT_DOWN,
    ASSET_VEHICLE_FREEZEBOT_DOWN_WALK1,
    ASSET_VEHICLE_FREEZEBOT_DOWN_WALK2,
    ASSET_VEHICLE_FREEZEBOT_DOWN_WALK3,
    ASSET_VEHICLE_FREEZEBOT_DOWN_WALK4,
    ASSET_VEHICLE_FREEZEBOT_DOWN_ATK1,
    ASSET_VEHICLE_FREEZEBOT_DOWN_ATK2,
    ASSET_VEHICLE_FREEZEBOT_LEFT,
    ASSET_VEHICLE_FREEZEBOT_LEFT_WALK1,
    ASSET_VEHICLE_FREEZEBOT_LEFT_WALK2,
    ASSET_VEHICLE_FREEZEBOT_LEFT_WALK3,
    ASSET_VEHICLE_FREEZEBOT_LEFT_WALK4,
    ASSET_VEHICLE_FREEZEBOT_LEFT_ATK1,
    ASSET_VEHICLE_FREEZEBOT_LEFT_ATK2,
    ASSET_VEHICLE_FREEZEBOT_RIGHT,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK1,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK2,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK3,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK4,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK1,
    ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK2,
    ASSET_VEHICLE_FREEZEBOT_UP,
    ASSET_VEHICLE_FREEZEBOT_UP_WALK1,
    ASSET_VEHICLE_FREEZEBOT_UP_WALK2,
    ASSET_VEHICLE_FREEZEBOT_UP_WALK3,
    ASSET_VEHICLE_FREEZEBOT_UP_WALK4,
    ASSET_VEHICLE_FREEZEBOT_UP_ATK1,
    ASSET_VEHICLE_FREEZEBOT_UP_ATK2,
    ASSET_VEHICLE_FREEZEBOT_HERO,
    ASSET_PLAN_FREEZEBOT,

    /* ── wb2_buildings.t3x (23 entries) ── */
    ASSET_WB2_BUILDINGS_BASE,
    ASSET_BUILDING_FACTORY = ASSET_WB2_BUILDINGS_BASE,
    ASSET_BUILDING_FACTORY_BLUE,
    ASSET_BUILDING_FACTORY_GREEN,
    ASSET_BUILDING_FACTORY_RED,
    ASSET_BUILDING_FACTORY_WHITE,
    ASSET_BUILDING_FACTORY_YELLOW,
    ASSET_BUILDING_FACTORY_HERO,
    ASSET_PLAN_FACTORY,
    ASSET_BUILDING_HOUSE,
    ASSET_BUILDING_HOUSE_HERO,
    ASSET_PLAN_HOUSE,
    ASSET_BUILDING_WINDMILL_1,
    ASSET_BUILDING_WINDMILL_2,
    ASSET_BUILDING_WINDMILL_3,
    ASSET_BUILDING_WINDMILL_4,
    ASSET_BUILDING_WINDMILL_HERO,
    ASSET_PLAN_WINDMILL,
    ASSET_BUILDING_GARAGE,
    ASSET_BUILDING_GARAGE_HERO,
    ASSET_PLAN_GARAGE,
    ASSET_BUILDING_NURSERY,
    ASSET_BUILDING_NURSERY_HERO,
    ASSET_PLAN_NURSERY,

    /* ── wb2_terrain.t3x (41 entries) ── */
    ASSET_WB2_TERRAIN_BASE,
    ASSET_TERRAIN_CEMENT = ASSET_WB2_TERRAIN_BASE,
    ASSET_TERRAIN_JUNGLE1,
    ASSET_TERRAIN_JUNGLE2,
    ASSET_TERRAIN_JUNGLE3,
    ASSET_TERRAIN_JUNGLE4,
    ASSET_TERRAIN_ROADBLOCK,
    ASSET_TERRAIN_STREET1,
    ASSET_TERRAIN_STREET2,
    ASSET_TERRAIN_STREET3,
    ASSET_TERRAIN_STREET4,
    ASSET_TERRAIN_STREET5,
    ASSET_TERRAIN_STREET6,
    ASSET_TERRAIN_STREET7,
    ASSET_TERRAIN_STREET8,
    ASSET_TERRAIN_STREET9,
    ASSET_TERRAIN_STREET10,
    ASSET_TERRAIN_STREET_UNDIGGABLE,
    ASSET_TERRAIN_TREE2,
    ASSET_TERRAIN_TREE3,
    ASSET_TERRAIN_TREE4,
    ASSET_MINI_CEMENT,
    ASSET_MINI_JUNGLE1,
    ASSET_MINI_JUNGLE2,
    ASSET_MINI_JUNGLE3,
    ASSET_MINI_JUNGLE4,
    ASSET_MINI_ROADBLOCK,
    ASSET_MINI_STREET1,
    ASSET_MINI_STREET2,
    ASSET_MINI_STREET3,
    ASSET_MINI_STREET4,
    ASSET_MINI_STREET5,
    ASSET_MINI_STREET6,
    ASSET_MINI_STREET7,
    ASSET_MINI_STREET8,
    ASSET_MINI_STREET9,
    ASSET_MINI_STREET10,
    ASSET_MINI_STREET_UNDIGGABLE,
    ASSET_MINI_TREE2,
    ASSET_MINI_TREE3,
    ASSET_MINI_TREE4,
    ASSET_TERRAIN_GOALZONE,

    /* ── wb2_resources.t3x (5 entries) ── */
    ASSET_WB2_RESOURCES_BASE,
    ASSET_RESOURCE_WHITE_1 = ASSET_WB2_RESOURCES_BASE,
    ASSET_RESOURCE_WHITE_2,
    ASSET_RESOURCE_WHITE_3,
    ASSET_RESOURCE_WHITE_4,
    ASSET_CARRY_WHITE,

    /* ── worldmap_d.t3x ── */
    ASSET_WORLDMAP6,
    ASSET_WORLDMAP7,

    /* ── worldsky_d.t3x ── */
    ASSET_WB2SKY1,
    ASSET_WB2SKY2,

    /* ── license.t3x ── */
    ASSET_LICENSE_BASE,
    ASSET_LICENSE_CLASS1 = ASSET_LICENSE_BASE,
    ASSET_LICENSE_CLASS2,
    ASSET_LICENSE_CLASS3,
    ASSET_LICENSE_CLASS4,
    ASSET_LICENSE_STICKER_W4,   /* world 4 (ocean) completion sticker */
    ASSET_LICENSE_STICKER_W5,   /* world 5 (prehistoric) completion sticker */
    ASSET_WB2_LICENSE_FRONT,    /* WB2 license card front */
    ASSET_WB2_REWARD_CLASS1,    /* WB2 "Class A Builder" label */
    ASSET_WB2_REWARD_CLASS2,    /* WB2 "Class B Builder" label */
    ASSET_WB2_REWARD_CLASS3     /* WB2 "Class C Builder" label */
} WBAssetId;

typedef struct WBAnchor {
    float x;
    float y;
} WBAnchor;

typedef enum WBActionMode {
    WB_ACTION_MOVE = 0,
    WB_ACTION_PICK,
    WB_ACTION_DROP,
    WB_ACTION_DIG,
    WB_ACTION_FILL,
    WB_ACTION_UPROOT,
    WB_ACTION_PLANT,
    WB_ACTION_PUSH,
    WB_ACTION_ATTACK
} WBActionMode;

typedef enum WBScreenMode {
    WB_SCREEN_TITLE = 0,
    WB_SCREEN_WORLD_SELECT,
    WB_SCREEN_GAME
} WBScreenMode;

/* ─── Original object model ──────────────────────────────────────────────────
   Units and monsters are 'vehicle.generic parent' objects and buildings are
   'building.generic parent' objects, all stepped once per movie frame (15 fps,
   the actorList order) exactly as the original's stepFrame handlers run. An
   object always occupies one tile: a step claims the destination tile at once
   (MoveObject) and the sprite then slides there over 1000/speed ms. */

#define WB_FRAME_MS (1000.0 / 15.0)
#define WB_PATH_MAX (WB_MAX_MAP_W * WB_MAX_MAP_H)
#define WB_MAX_AGENTS 96
#define WB_MAX_ACTORS 256

typedef enum WBObjKind { WB_OBJ_NONE = 0, WB_OBJ_UNIT, WB_OBJ_MONSTER, WB_OBJ_BUILDING } WBObjKind;

/* A reference to a live object (the original holds object references; pDead
   says whether one has gone). The generation makes a reused slot never match. */
typedef struct WBObjRef {
    u8 kind;
    u8 index;       /* unit: agent slot; monster: g_monsters index */
    u16 gen;
    s8 x;           /* building tile */
    s8 y;
} WBObjRef;

typedef enum WBReason {
    WB_REASON_NONE = 0,
    WB_REASON_PICK,
    WB_REASON_DROP,
    WB_REASON_DIG,
    WB_REASON_FILL,
    WB_REASON_UPROOT,
    WB_REASON_PLANT,
    WB_REASON_ATTACK
} WBReason;

typedef struct WBAttackProfile {
    int hitsPerMinute;
    int chanceOfSuccess;
    int damageMin;          /* original 0-100 energy units */
    int damageMax;
    int searchRange;
} WBAttackProfile;

/* vehicle.generic parent's movement properties. */
typedef struct WBMotion {
    int curX;               /* pTile */
    int curY;
    int fromX;              /* pOldPos */
    int fromY;
    bool sliding;           /* pMovePixelTime is set */
    double slideStartMs;
    float moveIndex;        /* pMoveIndex: 1 at step start, 0 once the slide is done */
    bool teleportStep;      /* the pop out of a whirlpool: no slide */
    double moveTimeMs;      /* pMoveTime */
    bool hasPath;           /* pPath */
    bool pathValid;         /* pPath.path */
    bool teleport;          /* pPath.teleport */
    int goalX;
    int goalY;
    int step;               /* index in path of the tile we stand on */
    int pathLen;
    u8 pathX[WB_PATH_MAX];
    u8 pathY[WB_PATH_MAX];
    u8 reason;              /* pPath.options */
    bool nextto;
    bool chase;
    int tryNum;
    bool tryAgainSet;
    double tryAgainMs;
    int randomizeNum;
    double randomizeMs;
    bool checkDest;         /* pCheckDestination */
    int checkX;
    int checkY;
    u8 checkReason;
    float pixelOffX;        /* pPixelOffset (stage px) */
    float pixelOffY;
} WBMotion;

/* Everything the original keeps on a unit object besides what the cell holds. */
typedef struct WBUnitAgent {
    bool used;
    u16 gen;
    WBMotion m;
    bool hasTarget;         /* pAttack */
    WBObjRef target;
    bool nextAttackSet;
    double nextAttackMs;
    WBAttackProfile stats;
    u8 actionAnim;          /* pActionCycle */
    double actionWhenMs;
    bool recharging;        /* pRechargingFrom */
    WBObjRef charger;
    bool checkRechargerSet;
    double checkRechargerMs;
    bool swampSet;          /* pSwampTime */
    double swampMs;
    bool dying;             /* pDying */
    double damageMs;
    bool pushWaiting;       /* pPushWaiting */
    bool stopPushing;       /* pStopPushing: the pushed thing no longer follows our slide */
    int pushX;
    int pushY;
    u8 freezeState;         /* freezebot: 0 ready, 1 targeted, 2 recharging */
    double freezeMs;
    WBObjRef freezeTarget;
    int repairFrame;        /* repairbot pRepairingFrame, 0 = VOID */
    u8 repairShow;          /* the repair frame (1/2) its getMember picked this frame, 0 = none */
} WBUnitAgent;

typedef struct WBMonsterState {
    bool active;
    u16 gen;
    int row;
    int col;
    WBMonsterType type;
    WBDirection dir;
    int hp;             /* battery charge, 0-1000 */
    int wanderTimer;    /* speed scale percentage (80..120) */
    float speed;        /* #speed x that scale */
    bool onWater;       /* gator: use water sprite set */
    bool dying;         /* pDying */
    double damageMs;
    WBMotion m;         /* it moves with vehicle.generic's code */
    bool resting;       /* pWandering[#rest] */
    double restUntilMs;
    bool swampInTerrain;/* #swamp currently added to its terrain list */
    bool hasTarget;     /* pAttack */
    WBObjRef target;
    bool nextAttackSet;
    double nextAttackMs;
    WBAttackProfile stats;
    bool actionActive;  /* pActionCycle (#attack) */
    double actionWhenMs;
    bool swampSet;      /* pSwampTime */
    double swampMs;
    u8 frozenState;     /* WB2 pFrozenState: 0 none, 1 #waiting, 2 #frozen, 3 #recovering */
    double frozenMs;
    bool pushed;        /* being pushed by the dozer: rides its slide */
    WBObjRef pushedBy;
} WBMonsterState;

#define WB_MAX_MONSTERS 32

typedef struct WBAudioClip {
    bool loaded;
    void* data;
    u32 size;
    u32 sampleRate;
    int channels;
    float volume;   /* 0.0 = default (1.0); otherwise applied as mix scale */
    ndspWaveBuf waveBuf;
} WBAudioClip;

typedef struct WBSelectedUnitView {
    bool valid;
    WBUnitType unitType;
    WBTerrainType terrain;
    int energy;
    int energyDeci;
    WBResourcePile cargo;
    WBDirection direction;
} WBSelectedUnitView;

typedef enum WBWorldEffectKind {
    WB_WORLD_EFFECT_DAMAGE_SMALL = 0,
    WB_WORLD_EFFECT_TAKE_APART_CLOUD,
    WB_WORLD_EFFECT_BUILD_CLOUD
} WBWorldEffectKind;

typedef struct WBWorldEffect {
    bool active;
    WBWorldEffectKind kind;
    int tileX;
    int tileY;
    u64 startMs;
    int particleKinds[5];
} WBWorldEffect;

/* One 'mini-unit behavior' sprite on the world map. Position is in map-image space
   (the original's stage px minus the map's origin), stepped at the movie's 15 fps. */
#define WB_MAX_WORLD_MINIS 40
typedef struct WBMiniWalker {
    bool active;
    int world;
    int def;        /* index into g_worldMapDefs[world].minis */
    float x;        /* pp */
    float y;
    int dirX;       /* dp; 0,0 until first shown (the original's VOID) */
    int dirY;
    bool stopped;   /* mode #stop */
    bool flipH;
} WBMiniWalker;

typedef struct WBPlanSwoop {
    bool active;
    int assetId;
    float fromX;
    float fromY;
    float toX;
    float toY;
    u64 startMs;
    u64 durationMs;
} WBPlanSwoop;

typedef struct AppState {
    WBScreenMode screenMode;
    int worldSelectWorld;
    bool worldSelectTouchActive;
    int hoverMissionWorld;
    int hoverMissionLevel;
    int activeWorld;
    int activeMission;
    int levelState[8][13];
    int levelOpenCount[8][13];
    int levelOpens[8][13][4];
    char missionNames[8][13][WB_MAX_NAME];
    WBMap map;
    int cameraX;
    int cameraY;
    int selectedX;
    int selectedY;
    bool hasSelection;
    bool infoOverlayOpen;
    bool planMenuOpen;
    bool startMenuOpen;
    int  startMenuCursor;
    WBPlanType armedPlan;
    int planMenuTouchIndex;
    int planMenuScrollOffset;
    int planMenuCursor;
    bool touchActiveLast;
    WBActionMode actionMode;
    bool buildPreviewValid;
    int buildPreviewX;
    int buildPreviewY;
    bool buildPreviewAllowed;
    WBPlanSwoop planSwoop;
    bool goalPopupVisible;
    bool goalPopupBonus;
    bool goalPopupComplete;
    int goalPopupX;
    int goalPopupY;
    char goalPopupText[96];
    bool muteMusic;
    bool muteSfx;
    bool audioReady;
    bool showDiagnostics;
    bool debugMenuOpen;
    int  debugMenuCursor;
    int  konamiProgress;
    Result audioInitResult;
    Result audioFallbackOpenResult;
    Result audioFallbackInitResult;
    int loadedClipCount;
    int sfxPlayCount;   /* increments each time playSfxClip/playSfxClipWorld fires */
    int linearFreeKB;   /* free linear heap (KB) after all clips loaded */
    u64 musicRetryAtMs;
    u64 goalSfxUntilMs;
    int musicBpm;
    bool musicUseGamePlaylist;
    int musicLastGameSongIndex;
    C2D_SpriteSheet spriteSheet;
    C2D_SpriteSheet titleMainSheet;
    C2D_SpriteSheet worldSkySheets[4];
    C2D_SpriteSheet worldMapSheets[4];
    C2D_SpriteSheet worldIconSheet;
    C2D_SpriteSheet vehiclesLandSheet;
    C2D_SpriteSheet vehiclesWaterSheet;
    C2D_SpriteSheet vehiclesAnimalSheet;
    C2D_SpriteSheet vehiclesRobotSheet;
    C2D_SpriteSheet monstersASheet;
    C2D_SpriteSheet monstersBSheet;
    C2D_SpriteSheet buildingsSheet;
    C2D_SpriteSheet whirlpoolSheet;
    C2D_TextBuf staticBuf;
    C2D_TextBuf dynamicBuf;
    C2D_Text titleText;
    C2D_Text screenTitleText;
    WBWorldEffect worldEffects[WB_MAX_WORLD_EFFECTS];
    /* ── the original's objects (see 'Original object model') ── */
    WBUnitAgent agents[WB_MAX_AGENTS];
    WBObjRef actorList[WB_MAX_ACTORS];          /* stepFrame order: the order objects were made */
    int actorCount;
    double simMs;                               /* the movie's clock, advanced 1000/15 ms a frame */
    double simAccumMs;
    u64 simLastMs;
    WBObjRef whirlpoolUser[WB_MAX_MAP_H][WB_MAX_MAP_W];
    u16 buildingGen[WB_MAX_MAP_H][WB_MAX_MAP_W];
    bool buildingHasTarget[WB_MAX_MAP_H][WB_MAX_MAP_W];
    WBObjRef buildingTarget[WB_MAX_MAP_H][WB_MAX_MAP_W];
    bool buildingNextAttackSet[WB_MAX_MAP_H][WB_MAX_MAP_W];
    double buildingNextAttackMs[WB_MAX_MAP_H][WB_MAX_MAP_W];
    WBAttackProfile buildingAttackStats[WB_MAX_MAP_H][WB_MAX_MAP_W];
    bool buildingDying[WB_MAX_MAP_H][WB_MAX_MAP_W];
    double buildingDamageMs[WB_MAX_MAP_H][WB_MAX_MAP_W];
    double buildingReadyMs[WB_MAX_MAP_H][WB_MAX_MAP_W]; /* windmill/garage/nursery pReadyTime */
    u8 factoryWhat[WB_MAX_MAP_H][WB_MAX_MAP_W][4];      /* factory: what each side holds (1 boulder, 2 tree) */
    double factoryWhen[WB_MAX_MAP_H][WB_MAX_MAP_W][4];
    WBTerrainType mapTreeTypes[4];              /* tree kinds on the map, for the nursery */
    int mapTreeTypeCount;
    bool centerGoalActive;                      /* map display pCenterGoal: the vehicle the view follows */
    WBObjRef centerGoal;
    bool buildSelectPending;                    /* build cloud: click the new object after 750 ms */
    double buildSelectMs;
    WBObjRef buildSelectRef;
    bool pilePushed;                            /* a pile the dozer pushed, riding its slide */
    int pilePushX;
    int pilePushY;
    WBObjRef pilePushedBy;
    bool pushOverrideActive;                    /* mapclickOverride: the dozer's push waits for a click */
    int pushOverrideX;
    int pushOverrideY;
    WBObjRef pushOverrideUnit;
    bool spikeActive;                           /* click spike on the ordered tile */
    int spikeX;
    int spikeY;
    double spikeMs;
    u64 musicStartMs;                           /* glob.musicStartTime: the selection arrow bobs on its beat */
    WBMiniWalker miniWalkers[WB_MAX_WORLD_MINIS];
    int miniWalkerWorld;           /* world the walkers (and icon flag frames) were set up for */
    u8 missionFlagFrame[13];       /* mission icon 'flagframe', random(6) at beginSprite */
    u64 worldSelectLastMs;
    float worldSelectTickAccum;    /* ms toward the next 15 fps world-map frame */
    u8* walkMask;                  /* romfs:/meta/worldmask_wN.bin, 2 bits per map pixel */
    int walkMaskW;
    int walkMaskH;
    int walkMaskWorld;
    u64 lastScrollMs;              /* clickToScroll(): at most one tile per 150 ms */
    bool pileInspectValid;         /* pile whose bricks are shown (the original's rollover) */
    int pileInspectX;
    int pileInspectY;
    bool tutorialActive;           /* World 1 Mission 1: 'tutorial manager' */
    int tutorialStep;
    u64 tutorialAdvanceAtMs;       /* pending gonext() timeout, 0 when none */
    bool tutorialCatcherLive;      /* the step's click hole still waits for its trigger */
    u64 tutorialDialogShownMs;
    u64 tutorialDialogHiddenMs;
    WBAudioClip musicIntro;
    WBAudioClip musicGame;
    WBAudioClip musicIntroVariants[WB_MUSIC_INTRO_VARIANTS];
    WBAudioClip musicGame6Variants[WB_MUSIC_GAME6_VARIANTS];
    WBAudioClip musicGame8Variants[WB_MUSIC_GAME8_VARIANTS];
    WBAudioClip musicGameAVariants[WB_MUSIC_GAMEA_VARIANTS];
    WBAudioClip musicGameIVariants[WB_MUSIC_GAMEI_VARIANTS];
    WBAudioClip sfxButton;
    WBAudioClip sfxUnitVehicle;
    WBAudioClip sfxUnitAnimal;
    WBAudioClip sfxUnitRobot;
    WBAudioClip sfxPlan;
    WBAudioClip sfxPickupPlan;
    WBAudioClip sfxMove;
    WBAudioClip sfxPickup;
    WBAudioClip sfxDrop;
    WBAudioClip sfxDigGround;
    WBAudioClip sfxFillGround;
    WBAudioClip sfxDigTree;
    WBAudioClip sfxPlantTree;
    WBAudioClip sfxRollover;
    WBAudioClip sfxWorldComingSoon;
    WBAudioClip sfxDamage;
    WBAudioClip sfxMonsterAttack;
    WBAudioClip sfxDisassemble;
    WBAudioClip sfxGoal;
    WBAudioClip sfxBonusGoal;
    WBAudioClip sfxAssembly;
    WBAudioClip sfxMoveMisc;
    WBAudioClip sfxBldgGeneric;
    WBAudioClip sfxBldgFactory;
    WBAudioClip sfxBldgGasStationMarina;
    WBAudioClip sfxBldgGuardTower;
    WBAudioClip sfxBldgRobotLab;
    C2D_SpriteSheet wb2MonstersSheet;
    C2D_SpriteSheet wb2VehiclesSheet;
    C2D_SpriteSheet wb2BuildingsSheet;
    C2D_SpriteSheet wb2TerrainSheet;
    C2D_SpriteSheet wb2ResourcesSheet;
    C2D_SpriteSheet licenseSheet; /* license card images (4 classes) */
    bool licenseVisible;          /* SELECT toggle on worldmap */
    u64 titleCycleStartMs;  /* timestamp when title screen was last entered; 0 = not started */
    bool titleShowWB2;      /* true when WB2 title is currently the active half of the cycle */
} AppState;

static AppState g_app;
static WBMonsterState g_monsters[WB_MAX_MONSTERS];
static int g_monsterCount = 0;
static int g_queueX[WB_MAX_MAP_W * WB_MAX_MAP_H]; /* collect-goal flood fill */
static int g_queueY[WB_MAX_MAP_W * WB_MAX_MAP_H];
static bool g_visited[WB_MAX_MAP_H][WB_MAX_MAP_W];

static const WBAnchor g_terrainAnchors[] = {
    [ASSET_TERRAIN_NORMAL] = { 24.0f, 0.0f },
    [ASSET_TERRAIN_TREE] = { 24.0f, 19.0f },
    [ASSET_TERRAIN_WATER] = { 24.0f, -12.0f },
    [ASSET_TERRAIN_WATER_REEFS] = { 24.0f, -4.0f },
    [ASSET_TERRAIN_WATER_UNDIGGABLE] = { 24.0f, -25.0f },
    [ASSET_TERRAIN_NORMAL_UNDIGGABLE] = { 24.0f, 0.0f },
    [ASSET_TERRAIN_MOUNTAIN] = { 24.0f, 6.0f },
    [ASSET_TERRAIN_BILLBOARD] = { 74.0f, 46.0f },
    [ASSET_TERRAIN_SWAMP] = { 24.0f, 0.0f },

    /* WB2 terrain */
    [ASSET_TERRAIN_CEMENT]            = { 24.0f, 0.0f },
    [ASSET_TERRAIN_JUNGLE1]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_JUNGLE2]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_JUNGLE3]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_JUNGLE4]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_ROADBLOCK]         = { 24.0f, 6.0f },
    [ASSET_TERRAIN_STREET1]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET2]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET3]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET4]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET5]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET6]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET7]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET8]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET9]           = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET10]          = { 24.0f, 0.0f },
    [ASSET_TERRAIN_STREET_UNDIGGABLE] = { 24.0f, 0.0f },
    [ASSET_TERRAIN_TREE2]             = { 24.0f, 37.0f },
    [ASSET_TERRAIN_TREE3]             = { 24.0f, 9.0f },
    [ASSET_TERRAIN_TREE4]             = { 24.0f, 19.0f },
    [ASSET_TERRAIN_GOALZONE]          = { 24.0f, 0.0f }
};

static const WBAnchor g_objectAnchors[] = {
    [ASSET_VEHICLE_BUGGY] = { 19.0f, 19.0f },
    [ASSET_VEHICLE_DUCK] = { 8.0f, 11.0f },
    [ASSET_PLAN_BUGGY] = { 14.0f, 10.0f },
    [ASSET_RESOURCE_RED] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_BLUE] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_YELLOW] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_WHEEL] = { 15.0f, 4.0f },
    [ASSET_RESOURCE_ENERGY] = { 8.0f, 8.0f },
    [ASSET_GOAL_MAIN] = { -3.0f, 30.0f },
    [ASSET_GOAL_BONUS] = { 7.0f, 25.0f },
    [ASSET_PLAN_GENERIC] = { -7.0f, -10.0f },
    [ASSET_PLAN_UNKNOWN] = { 11.0f, 14.0f },
    [ASSET_RESOURCE_RED_1] = { 12.0f, 8.0f },
    [ASSET_RESOURCE_RED_2] = { 19.0f, 11.0f },
    [ASSET_RESOURCE_RED_3] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_RED_4] = { 34.0f, 25.0f },
    [ASSET_RESOURCE_BLUE_1] = { 12.0f, 8.0f },
    [ASSET_RESOURCE_BLUE_2] = { 19.0f, 11.0f },
    [ASSET_RESOURCE_BLUE_3] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_BLUE_4] = { 34.0f, 25.0f },
    [ASSET_RESOURCE_GREEN_1] = { 12.0f, 8.0f },
    [ASSET_RESOURCE_GREEN_2] = { 19.0f, 11.0f },
    [ASSET_RESOURCE_GREEN_3] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_GREEN_4] = { 34.0f, 25.0f },
    [ASSET_RESOURCE_YELLOW_1] = { 12.0f, 8.0f },
    [ASSET_RESOURCE_YELLOW_2] = { 19.0f, 11.0f },
    [ASSET_RESOURCE_YELLOW_3] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_YELLOW_4] = { 34.0f, 25.0f },
    [ASSET_RESOURCE_WHEEL_FULL] = { 15.0f, 4.0f },
    [ASSET_RESOURCE_ENERGY_FULL] = { 8.0f, 8.0f },
    [ASSET_RESOURCE_ENERGY_LOW] = { 8.0f, 8.0f },
    [ASSET_RESOURCE_ENERGY_DEAD] = { 8.0f, 8.0f },
    [ASSET_VEHICLE_BUGGY_UP] = { 19.0f, 14.0f },
    [ASSET_VEHICLE_BUGGY_DOWN] = { 19.0f, 19.0f },
    [ASSET_VEHICLE_BUGGY_LEFT] = { 22.0f, 16.0f },
    [ASSET_VEHICLE_BUGGY_RIGHT] = { 23.0f, 16.0f },
    [ASSET_VEHICLE_DUCK_UP] = { 8.0f, 14.0f },
    [ASSET_VEHICLE_DUCK_DOWN] = { 8.0f, 11.0f },
    [ASSET_VEHICLE_DUCK_LEFT] = { 16.0f, 15.0f },
    [ASSET_VEHICLE_DUCK_RIGHT] = { 16.0f, 13.0f },
    [ASSET_VEHICLE_DUCK_WATER_UP] = { 7.0f, 3.0f },
    [ASSET_VEHICLE_DUCK_WATER_DOWN] = { 7.0f, 5.0f },
    [ASSET_VEHICLE_DUCK_WATER_LEFT] = { 17.0f, 9.0f },
    [ASSET_VEHICLE_DUCK_WATER_RIGHT] = { 11.0f, 5.0f },
    [ASSET_OBJECT_HIGHLIGHT] = { 0.0f, 35.0f },
    [ASSET_PLAN_HIGHLIGHT] = { 17.0f, -8.0f },
    [ASSET_BUILD_YES] = { 10.0f, 11.0f },
    [ASSET_BUILD_NO] = { 7.0f, 7.0f },
    [ASSET_CHECK_MARK] = { 8.0f, 8.0f },
    [ASSET_X_MARK] = { 8.0f, 8.0f },
    [ASSET_MENU_PANEL] = { 0.0f, 0.0f },
    [ASSET_BUILD_OUTLINE] = { 76.0f, 1.0f },
    [ASSET_BUILD_OUTLINE_FRONT] = { 75.0f, -92.0f },
    [ASSET_BUILD_OUTLINE_REAR] = { 25.0f, 3.0f },
    [ASSET_BUILD_OUTLINE_L1] = { 50.0f, -50.0f },
    [ASSET_BUILD_OUTLINE_L2] = { 74.0f, -94.0f },
    [ASSET_BUILD_OUTLINE_R1] = { -97.0f, -50.0f },
    [ASSET_BUILD_OUTLINE_R2] = { -127.0f, 2.0f },
    [ASSET_GOAL_BUBBLE] = { 68.0f, 69.0f },
    [ASSET_GOAL_COMPLETE_BUBBLE] = { 68.0f, 109.0f },
    [ASSET_BONUS_GOAL_COMPLETE_BUBBLE] = { 68.0f, 82.0f },
    [ASSET_DAMAGE_SMALL_1] = { 10.0f, 15.0f },
    [ASSET_DAMAGE_SMALL_2] = { 15.0f, 26.0f },
    [ASSET_DAMAGE_SMALL_3] = { 29.0f, 41.0f },
    [ASSET_DAMAGE_SMALL_4] = { 40.0f, 53.0f },
    [ASSET_DAMAGE_SMALL_5] = { 53.0f, 59.0f },
    [ASSET_TAKE_APART_CLOUD_1] = { -1.0f, -2.0f },
    [ASSET_TAKE_APART_CLOUD_2] = { 22.0f, 23.0f },
    [ASSET_TAKE_APART_CLOUD_3] = { 24.0f, 21.0f },
    [ASSET_BUILD_CLOUD_1] = { 27.0f, 15.0f },
    [ASSET_BUILD_CLOUD_2] = { 30.0f, 24.0f },

    /* ── vehicles_land ─────────────────────────────────────────────────── */
    [ASSET_VEHICLE_DIRTBUGGY_UP]    = { 20.0f, 17.0f },
    [ASSET_VEHICLE_DIRTBUGGY_DOWN]  = { 20.0f, 17.0f },
    [ASSET_VEHICLE_DIRTBUGGY_LEFT]  = { 24.0f, 15.0f },
    [ASSET_VEHICLE_DIRTBUGGY_RIGHT] = { 25.0f, 15.0f },

    [ASSET_VEHICLE_STEAMSHOVEL_UP]         = { 16.0f, 20.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_DOWN]       = { 18.0f, 17.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_LEFT]       = { 33.0f, 22.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_RIGHT]      = { 29.0f, 21.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_UP_DIG]     = { 16.0f, 13.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_DOWN_DIG]   = { 21.0f, 17.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_LEFT_DIG]   = { 40.0f, 12.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_RIGHT_DIG]  = { 29.0f, 12.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_UP_FULL]    = { 16.0f, 33.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_DOWN_FULL]  = { 18.0f, 25.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_LEFT_FULL]  = { 31.0f, 33.0f },
    [ASSET_VEHICLE_STEAMSHOVEL_RIGHT_FULL] = { 29.0f, 31.0f },

    [ASSET_VEHICLE_DUMPTRUCK_UP]    = { 19.0f, 17.0f },
    [ASSET_VEHICLE_DUMPTRUCK_DOWN]  = { 19.0f, 18.0f },
    [ASSET_VEHICLE_DUMPTRUCK_LEFT]  = { 23.0f, 16.0f },
    [ASSET_VEHICLE_DUMPTRUCK_RIGHT] = { 23.0f, 17.0f },

    [ASSET_VEHICLE_FORKLIFT_UP]    = { 17.0f, 25.0f },
    [ASSET_VEHICLE_FORKLIFT_DOWN]  = { 17.0f, 21.0f },
    [ASSET_VEHICLE_FORKLIFT_LEFT]  = { 27.0f, 18.0f },
    [ASSET_VEHICLE_FORKLIFT_RIGHT] = { 29.0f, 19.0f },

    [ASSET_VEHICLE_DOZER_UP]    = { 24.0f, 21.0f },
    [ASSET_VEHICLE_DOZER_DOWN]  = { 25.0f, 25.0f },
    [ASSET_VEHICLE_DOZER_LEFT]  = { 38.0f, 18.0f },
    [ASSET_VEHICLE_DOZER_RIGHT] = { 38.0f, 19.0f },

    [ASSET_VEHICLE_SPEEDBOAT_UP]    = { 20.0f, 23.0f },
    [ASSET_VEHICLE_SPEEDBOAT_DOWN]  = { 21.0f, 23.0f },
    [ASSET_VEHICLE_SPEEDBOAT_LEFT]  = { 38.0f, 14.0f },
    [ASSET_VEHICLE_SPEEDBOAT_RIGHT] = { 37.0f, 14.0f },
    [ASSET_VEHICLE_SPEEDBOAT_HERO]  = { 59.0f, 44.0f },

    [ASSET_PLAN_DIRTBUGGY]   = { 16.0f, 10.0f },
    [ASSET_PLAN_STEAMSHOVEL] = { 23.0f, 16.0f },
    [ASSET_PLAN_DUMPTRUCK]   = { 22.0f, 13.0f },
    [ASSET_PLAN_FORKLIFT]    = { 20.0f, 13.0f },
    [ASSET_PLAN_DOZER]       = { 23.0f,  9.0f },
    [ASSET_PLAN_SPEEDBOAT]   = { 14.0f, 10.0f },

    /* ── vehicles_water ────────────────────────────────────────────────── */
    [ASSET_VEHICLE_TUGBOAT_UP]    = { 18.0f, 19.0f },
    [ASSET_VEHICLE_TUGBOAT_DOWN]  = { 16.0f, 17.0f },
    [ASSET_VEHICLE_TUGBOAT_LEFT]  = { 35.0f, 12.0f },
    [ASSET_VEHICLE_TUGBOAT_RIGHT] = { 31.0f, 11.0f },

    [ASSET_VEHICLE_FREIGHTER_UP]    = { 14.0f, 17.0f },
    [ASSET_VEHICLE_FREIGHTER_DOWN]  = { 10.0f, 22.0f },
    [ASSET_VEHICLE_FREIGHTER_LEFT]  = { 30.0f,  7.0f },
    [ASSET_VEHICLE_FREIGHTER_RIGHT] = { 29.0f, 14.0f },

    [ASSET_PLAN_TUGBOAT]   = { 22.0f,  9.0f },
    [ASSET_PLAN_FREIGHTER] = { 26.0f,  8.0f },

    /* ── vehicles_animal ───────────────────────────────────────────────── */
    [ASSET_VEHICLE_FROG_UP]    = { 13.0f, 20.0f },
    [ASSET_VEHICLE_FROG_DOWN]  = { 13.0f, 20.0f },
    [ASSET_VEHICLE_FROG_LEFT]  = { 12.0f, 19.0f },
    [ASSET_VEHICLE_FROG_RIGHT] = { 12.0f, 20.0f },

    [ASSET_VEHICLE_FROG_UP_JUMP]    = { 13.0f, 28.0f },
    [ASSET_VEHICLE_FROG_DOWN_JUMP]  = { 13.0f, 29.0f },
    [ASSET_VEHICLE_FROG_LEFT_JUMP]  = { 11.0f, 29.0f },
    [ASSET_VEHICLE_FROG_RIGHT_JUMP] = { 12.0f, 31.0f },

    [ASSET_VEHICLE_FROG_WATER_UP]    = { 15.0f, 11.0f },
    [ASSET_VEHICLE_FROG_WATER_DOWN]  = { 13.0f, 14.0f },
    [ASSET_VEHICLE_FROG_WATER_LEFT]  = { 15.0f, 15.0f },
    [ASSET_VEHICLE_FROG_WATER_RIGHT] = { 13.0f, 17.0f },

    [ASSET_VEHICLE_FROG_WATER_UP_JUMP]    = { 13.0f, 17.0f },
    [ASSET_VEHICLE_FROG_WATER_DOWN_JUMP]  = { 13.0f, 16.0f },
    [ASSET_VEHICLE_FROG_WATER_LEFT_JUMP]  = { 13.0f, 16.0f },
    [ASSET_VEHICLE_FROG_WATER_RIGHT_JUMP] = { 14.0f, 19.0f },

    [ASSET_VEHICLE_FISH_WATER_UP]    = { 13.0f, 16.0f },
    [ASSET_VEHICLE_FISH_WATER_DOWN]  = {  8.0f, 10.0f },
    [ASSET_VEHICLE_FISH_WATER_LEFT]  = { 23.0f,  7.0f },
    [ASSET_VEHICLE_FISH_WATER_RIGHT] = { 20.0f,  7.0f },

    [ASSET_VEHICLE_SNAIL_UP]    = {  7.0f, 12.0f },
    [ASSET_VEHICLE_SNAIL_DOWN]  = {  7.0f, 10.0f },
    [ASSET_VEHICLE_SNAIL_LEFT]  = { 11.0f, 10.0f },
    [ASSET_VEHICLE_SNAIL_RIGHT] = { 11.0f,  9.0f },

    [ASSET_PLAN_FROG]  = { 12.0f, 14.0f },
    [ASSET_PLAN_FISH]  = { 15.0f,  6.0f },
    [ASSET_PLAN_SNAIL] = { 11.0f,  9.0f },

    /* ── vehicles_robot ────────────────────────────────────────────────── */
    [ASSET_VEHICLE_TREEBOT_UP]    = { 25.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN]  = { 27.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT]  = {  7.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT] = { 15.0f, 52.0f },

    [ASSET_VEHICLE_TREEBOT_UP_WALK1] = { 24.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_WALK2] = { 24.0f, 41.0f },
    [ASSET_VEHICLE_TREEBOT_UP_WALK3] = { 24.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_WALK4] = { 28.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_WALK5] = { 26.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_WALK6] = { 24.0f, 42.0f },

    [ASSET_VEHICLE_TREEBOT_DOWN_WALK1] = { 26.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_WALK2] = { 25.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_WALK3] = { 25.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_WALK4] = { 28.0f, 38.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_WALK5] = { 27.0f, 38.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_WALK6] = { 25.0f, 39.0f },

    [ASSET_VEHICLE_TREEBOT_LEFT_WALK1] = { 23.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_WALK2] = { 17.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_WALK3] = { 23.0f, 50.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_WALK4] = { 30.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_WALK5] = { 20.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_WALK6] = { 15.0f, 49.0f },

    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK1] = { 21.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK2] = { 16.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK3] = { 12.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK4] = { 23.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK5] = { 14.0f, 50.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_WALK6] = { 14.0f, 50.0f },

    [ASSET_VEHICLE_TREEBOT_UP_LIFT1] = { 24.0f, 50.0f },
    [ASSET_VEHICLE_TREEBOT_UP_LIFT2] = { 25.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_LIFT1] = { 25.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_LIFT2] = { 27.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_LIFT1] = { 12.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_LIFT2] = { 7.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_LIFT1] = { 17.0f, 52.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_LIFT2] = { 15.0f, 52.0f },

    [ASSET_VEHICLE_TREEBOT_UP_FULL] = { 25.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL] = { 27.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL] = { 7.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL] = { 15.0f, 52.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_WALK1] = { 24.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_WALK2] = { 24.0f, 41.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_WALK3] = { 24.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_WALK4] = { 28.0f, 42.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK1] = { 26.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK2] = { 25.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK3] = { 25.0f, 40.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK4] = { 28.0f, 38.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK1] = { 23.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK2] = { 17.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK3] = { 23.0f, 50.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK4] = { 30.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK1] = { 21.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK2] = { 16.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK3] = { 12.0f, 49.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK4] = { 23.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT1] = { 24.0f, 50.0f },
    [ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT2] = { 25.0f, 48.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT1] = { 25.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT2] = { 27.0f, 47.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT1] = { 12.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT2] = { 7.0f, 51.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT1] = { 17.0f, 52.0f },
    [ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT2] = { 15.0f, 52.0f },

    [ASSET_VEHICLE_DEFENDER_UP]    = { 33.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_DOWN]  = { 30.0f, 47.0f },
    [ASSET_VEHICLE_DEFENDER_LEFT]  = { 26.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_RIGHT] = { 29.0f, 53.0f },
    [ASSET_VEHICLE_DEFENDER_UP_WALK1] = { 33.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_UP_WALK2] = { 33.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_UP_WALK3] = { 33.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_UP_WALK4] = { 33.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_DOWN_WALK1] = { 30.0f, 47.0f },
    [ASSET_VEHICLE_DEFENDER_DOWN_WALK2] = { 30.0f, 47.0f },
    [ASSET_VEHICLE_DEFENDER_DOWN_WALK3] = { 30.0f, 47.0f },
    [ASSET_VEHICLE_DEFENDER_DOWN_WALK4] = { 30.0f, 47.0f },
    [ASSET_VEHICLE_DEFENDER_LEFT_WALK1] = { 26.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_LEFT_WALK2] = { 26.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_LEFT_WALK3] = { 26.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_LEFT_WALK4] = { 26.0f, 49.0f },
    [ASSET_VEHICLE_DEFENDER_RIGHT_WALK1] = { 29.0f, 53.0f },
    [ASSET_VEHICLE_DEFENDER_RIGHT_WALK2] = { 29.0f, 53.0f },
    [ASSET_VEHICLE_DEFENDER_RIGHT_WALK3] = { 29.0f, 53.0f },
    [ASSET_VEHICLE_DEFENDER_RIGHT_WALK4] = { 29.0f, 53.0f },
    [ASSET_VEHICLE_DEFENDER_HERO]  = { 60.0f, 83.0f },

    [ASSET_VEHICLE_REPAIRBOT_UP]    = { 23.0f, 15.0f },
    [ASSET_VEHICLE_REPAIRBOT_DOWN]  = { 23.0f, 16.0f },
    [ASSET_VEHICLE_REPAIRBOT_LEFT]  = { 12.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_RIGHT] = { 11.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_UP_WALK1] = { 23.0f, 15.0f },
    [ASSET_VEHICLE_REPAIRBOT_UP_WALK2] = { 23.0f, 15.0f },
    [ASSET_VEHICLE_REPAIRBOT_DOWN_WALK1] = { 23.0f, 16.0f },
    [ASSET_VEHICLE_REPAIRBOT_DOWN_WALK2] = { 23.0f, 16.0f },
    [ASSET_VEHICLE_REPAIRBOT_LEFT_WALK1] = { 12.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_LEFT_WALK2] = { 12.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK1] = { 11.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK2] = { 11.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_UP_REPAIR1] = { 23.0f, 15.0f },
    [ASSET_VEHICLE_REPAIRBOT_UP_REPAIR2] = { 23.0f, 15.0f },
    [ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR1] = { 23.0f, 16.0f },
    [ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR2] = { 23.0f, 16.0f },
    [ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR1] = { 12.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR2] = { 12.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR1] = { 11.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR2] = { 11.0f, 18.0f },
    [ASSET_VEHICLE_REPAIRBOT_HERO]  = { 44.0f, 54.0f },

    [ASSET_PLAN_TREEBOT]   = { 14.0f, 20.0f },
    [ASSET_PLAN_DEFENDER]  = { 13.0f, 20.0f },
    [ASSET_PLAN_REPAIRBOT] = { 16.0f, 11.0f },

    /* ── buildings ─────────────────────────────────────────────────────── */
    [ASSET_BUILDING_GAS_STATION]       = {  8.0f, 12.0f },
    [ASSET_BUILDING_GUARD_TOWER]       = { 12.0f, 12.0f },
    [ASSET_BUILDING_GUARD_TOWER_UP]    = {  9.0f, 20.0f },
    [ASSET_BUILDING_GUARD_TOWER_DOWN]  = {  9.0f, 12.0f },
    [ASSET_BUILDING_GUARD_TOWER_LEFT]  = { 13.0f, 13.0f },
    [ASSET_BUILDING_GUARD_TOWER_RIGHT] = {  9.0f, 15.0f },
    [ASSET_BUILDING_MARINA]            = { 14.0f,  8.0f },
    [ASSET_BUILDING_ROBOT_LAB]         = {  4.0f, 28.0f },
    [ASSET_BUILDING_GAS_STATION_HERO]   = { 54.0f, 63.0f },
    [ASSET_BUILDING_MARINA_HERO]        = { 56.0f, 63.0f },
    [ASSET_BUILDING_ROBOT_LAB_HERO]     = { 35.0f, 64.0f },
    [ASSET_BUILDING_GUARD_TOWER_HERO]   = { 37.0f, 56.0f },
    [ASSET_PLAN_GAS_STATION]           = { 18.0f, 15.0f },
    [ASSET_PLAN_GUARD_TOWER]           = { 12.0f, 14.0f },
    [ASSET_PLAN_MARINA]                = { 15.0f, 20.0f },
    [ASSET_PLAN_ROBOT_LAB]             = { 11.0f, 20.0f },
    [ASSET_PLAN_DUCK]                  = { 11.0f, 14.0f },

    /* WB2 buildings */
    [ASSET_BUILDING_FACTORY]       = { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_BLUE]  = { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_GREEN] = { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_RED]   = { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_WHITE] = { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_YELLOW]= { 19.0f, 15.0f },
    [ASSET_BUILDING_FACTORY_HERO]  = { 47.0f, 59.0f },
    [ASSET_PLAN_FACTORY]           = { 12.0f, 14.0f },
    [ASSET_BUILDING_HOUSE]         = { 19.0f, 10.0f },
    [ASSET_BUILDING_HOUSE_HERO]    = { 52.0f, 35.0f },
    [ASSET_PLAN_HOUSE]             = { 15.0f, 8.0f },
    [ASSET_BUILDING_WINDMILL_1]    = { 20.0f, 32.0f },
    [ASSET_BUILDING_WINDMILL_2]    = { 20.0f, 32.0f },
    [ASSET_BUILDING_WINDMILL_3]    = { 20.0f, 32.0f },
    [ASSET_BUILDING_WINDMILL_4]    = { 20.0f, 32.0f },
    [ASSET_BUILDING_WINDMILL_HERO] = { 41.0f, 49.0f },
    [ASSET_PLAN_WINDMILL]          = { 14.0f, 18.0f },
    [ASSET_BUILDING_GARAGE]        = { 7.0f, 14.0f },
    [ASSET_BUILDING_GARAGE_HERO]   = { 49.0f, 60.0f },
    [ASSET_PLAN_GARAGE]            = { 10.0f, 13.0f },
    [ASSET_BUILDING_NURSERY]       = { 17.0f, 11.0f },
    [ASSET_BUILDING_NURSERY_HERO]  = { 54.0f, 46.0f },
    [ASSET_PLAN_NURSERY]           = { 14.0f, 13.0f },

    /* WB2 monsters (lion) */
    [ASSET_MONSTER_LION]            = { 42.0f, 23.0f },
    [ASSET_MONSTER_LION_HERO]       = { 42.0f, 23.0f },
    [ASSET_PLAN_LION]               = { 42.0f, 23.0f },
    [ASSET_MONSTER_LION_DOWN]       = { 18.0f, 35.0f },
    [ASSET_MONSTER_LION_DOWN_WALK1] = { 18.0f, 34.0f },
    [ASSET_MONSTER_LION_DOWN_WALK2] = { 20.0f, 26.0f },
    [ASSET_MONSTER_LION_DOWN_ATK1]  = { 20.0f, 37.0f },
    [ASSET_MONSTER_LION_DOWN_ATK2]  = { 19.0f, 25.0f },
    [ASSET_MONSTER_LION_LEFT]       = { 42.0f, 23.0f },
    [ASSET_MONSTER_LION_LEFT_WALK1] = { 34.0f, 32.0f },
    [ASSET_MONSTER_LION_LEFT_WALK2] = { 42.0f, 26.0f },
    [ASSET_MONSTER_LION_LEFT_ATK1]  = { 33.0f, 31.0f },
    [ASSET_MONSTER_LION_LEFT_ATK2]  = { 38.0f, 31.0f },
    [ASSET_MONSTER_LION_RIGHT]      = { 43.0f, 23.0f },
    [ASSET_MONSTER_LION_RIGHT_WALK1]= { 35.0f, 32.0f },
    [ASSET_MONSTER_LION_RIGHT_WALK2]= { 43.0f, 26.0f },
    [ASSET_MONSTER_LION_RIGHT_ATK1] = { 36.0f, 29.0f },
    [ASSET_MONSTER_LION_RIGHT_ATK2] = { 40.0f, 36.0f },
    [ASSET_MONSTER_LION_UP]         = { 18.0f, 28.0f },
    [ASSET_MONSTER_LION_UP_WALK1]   = { 18.0f, 26.0f },
    [ASSET_MONSTER_LION_UP_WALK2]   = { 19.0f, 31.0f },
    [ASSET_MONSTER_LION_UP_ATK1]    = { 20.0f, 27.0f },
    [ASSET_MONSTER_LION_UP_ATK2]    = { 20.0f, 38.0f },
    [ASSET_MONSTER_LION_FROZEN]     = { 22.0f, 37.0f },
    [ASSET_MONSTER_CRAB_FROZEN]     = { 22.0f, 15.0f },
    [ASSET_MONSTER_GATOR_FROZEN]    = { 22.0f, 15.0f },
    [ASSET_MONSTER_SCORPION_FROZEN] = { 22.0f, 15.0f },
    [ASSET_MONSTER_SHARK_FROZEN]    = { 22.0f, 15.0f },
    [ASSET_MONSTER_TREX_FROZEN]     = { 22.0f, 15.0f },
    [ASSET_MONSTER_WATER_CRAB_FROZEN] = { 22.0f, 15.0f },

    /* WB2 vehicles (freezebot) */
    [ASSET_VEHICLE_FREEZEBOT]            = { 22.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_HERO]       = { 53.0f, 56.0f },
    [ASSET_PLAN_FREEZEBOT]               = { 13.0f, 14.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN]       = { 22.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_WALK1] = { 22.0f, 26.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_WALK2] = { 23.0f, 22.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_WALK3] = { 23.0f, 24.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_WALK4] = { 23.0f, 28.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_ATK1]  = { 18.0f, 28.0f },
    [ASSET_VEHICLE_FREEZEBOT_DOWN_ATK2]  = { 18.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT]       = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_WALK1] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_WALK2] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_WALK3] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_WALK4] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_ATK1]  = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_LEFT_ATK2]  = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT]      = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK1]= { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK2]= { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK3]= { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK4]= { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK1] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK2] = { 23.0f, 27.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP]         = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_WALK1]   = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_WALK2]   = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_WALK3]   = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_WALK4]   = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_ATK1]    = { 22.0f, 23.0f },
    [ASSET_VEHICLE_FREEZEBOT_UP_ATK2]    = { 22.0f, 23.0f },

    /* WB2 resources */
    [ASSET_RESOURCE_WHITE_1] = { 12.0f, 8.0f },
    [ASSET_RESOURCE_WHITE_2] = { 19.0f, 11.0f },
    [ASSET_RESOURCE_WHITE_3] = { 19.0f, 13.0f },
    [ASSET_RESOURCE_WHITE_4] = { 34.0f, 25.0f },
    [ASSET_CARRY_WHITE]      = { 12.0f, 8.0f }
};

static const WBAnchor g_pileRegPoints[] = {
    { 22.0f, 7.0f },
    { 17.0f, 18.0f },
    { 12.0f, 28.0f },
    { 6.0f, 39.0f }
};

typedef struct WBMissionPoint {
    float x;
    float y;
} WBMissionPoint;

static const int g_worldSkyAssets[8] = {
    0,
    ASSET_TITLE_SKY1,
    ASSET_TITLE_SKY2,
    ASSET_TITLE_SKY3,
    ASSET_TITLE_SKY4,
    ASSET_TITLE_SKY5,
    ASSET_WB2SKY1,
    ASSET_WB2SKY2
};

static const int g_worldMapAssets[8] = {
    0,
    ASSET_WORLDMAP1,
    ASSET_WORLDMAP2,
    ASSET_WORLDMAP3,
    ASSET_WORLDMAP4,
    ASSET_WORLDMAP5,
    ASSET_WORLDMAP6,
    ASSET_WORLDMAP7
};

/* Mission icon points, decorations, mini-units, obstacles: taken from the original
   Director scores by tools/gen_worldmap_data.py rather than estimated. */
#include "wb_worldmap_data.h"

static bool getSelectedUnitView(const AppState* app, WBSelectedUnitView* outView);
static void clampCamera(AppState* app);
static void loadGameplaySheets(AppState* app);
static WBUnitType planTypeToUnit(WBPlanType planType);
static void saveLevelProgress(const AppState* app);
static bool parseWorldMetadata(AppState* app);
static WBBuildingType planTypeToBuilding(WBPlanType planType);
static int buildingPanelIconAssetId(WBBuildingType buildingType);
static void startPlanSwoop(AppState* app, WBPlanType planType, float fromX, float fromY);
static void drawPlanSwoop(AppState* app);
static void appendDebugLog(const char* msg);
static void tickCollectGoals(AppState* app);
static void applyMissionSuccess(AppState* app, int world, int mission, int state);
static void tutorialStart(AppState* app);
static void drawClickSpike(AppState* app);
static bool pointInRect(float px, float py, float x, float y, float w, float h);
static void drawUnitCargoOverlay(AppState* app, const WBCell* cell, float unitX, float unitY);

static int worldSheetIndexForWorld(int world) {
    if (world <= 2) return 0;
    if (world <= 4) return 1;
    if (world <= 5) return 2;
    return 3;
}

static void ensureMenuSheetsLoaded(AppState* app) {
    if (!app->titleMainSheet) app->titleMainSheet = C2D_SpriteSheetLoad("romfs:/gfx/titlemain.t3x");
    if (!app->worldIconSheet) app->worldIconSheet = C2D_SpriteSheetLoad("romfs:/gfx/worldicons.t3x");
    if (!app->licenseSheet)   app->licenseSheet   = C2D_SpriteSheetLoad("romfs:/gfx/license.t3x");
}

/* Frees all sky/map sheets except the one needed for `world`, then loads
   that sheet if not already present. Call whenever worldSelectWorld changes. */
static void switchWorldSelectSheets(AppState* app, int world) {
    static const char* const skyPaths[4] = {
        "romfs:/gfx/worldsky_a.t3x",
        "romfs:/gfx/worldsky_b.t3x",
        "romfs:/gfx/worldsky_c.t3x",
        "romfs:/gfx/worldsky_d.t3x"
    };
    static const char* const mapPaths[4] = {
        "romfs:/gfx/worldmap_a.t3x",
        "romfs:/gfx/worldmap_b.t3x",
        "romfs:/gfx/worldmap_c.t3x",
        "romfs:/gfx/worldmap_d.t3x"
    };
    int needed = worldSheetIndexForWorld(world);
    int i;
    for (i = 0; i < 4; ++i) {
        if (i == needed) continue;
        if (app->worldSkySheets[i]) { C2D_SpriteSheetFree(app->worldSkySheets[i]); app->worldSkySheets[i] = NULL; }
        if (app->worldMapSheets[i]) { C2D_SpriteSheetFree(app->worldMapSheets[i]); app->worldMapSheets[i] = NULL; }
    }
    if (!app->worldSkySheets[needed]) app->worldSkySheets[needed] = C2D_SpriteSheetLoad(skyPaths[needed]);
    if (!app->worldMapSheets[needed]) app->worldMapSheets[needed] = C2D_SpriteSheetLoad(mapPaths[needed]);
}

static void trimMenuSheetsForGameplay(AppState* app, int world) {
    int keepSky = worldSheetIndexForWorld(world);
    int i;
    if (app->titleMainSheet) {
        C2D_SpriteSheetFree(app->titleMainSheet);
        app->titleMainSheet = NULL;
    }
    for (i = 0; i < 4; ++i) {
        if (i == keepSky) {
            continue;
        }
        if (app->worldSkySheets[i]) {
            C2D_SpriteSheetFree(app->worldSkySheets[i]);
            app->worldSkySheets[i] = NULL;
        }
    }
    if (!app->worldSkySheets[keepSky]) {
        static const char* skyPath[4] = {
            "romfs:/gfx/worldsky_a.t3x",
            "romfs:/gfx/worldsky_b.t3x",
            "romfs:/gfx/worldsky_c.t3x",
            "romfs:/gfx/worldsky_d.t3x"
        };
        app->worldSkySheets[keepSky] = C2D_SpriteSheetLoad(skyPath[keepSky]);
    }
    for (i = 0; i < 4; ++i) {
        if (app->worldMapSheets[i]) {
            C2D_SpriteSheetFree(app->worldMapSheets[i]);
            app->worldMapSheets[i] = NULL;
        }
    }
    if (app->worldIconSheet) {
        C2D_SpriteSheetFree(app->worldIconSheet);
        app->worldIconSheet = NULL;
    }
}

static int terrainAssetId(WBTerrainType terrain, uint8_t variant) {
    switch (terrain) {
        case WB_TERRAIN_NORMAL: return ASSET_TERRAIN_NORMAL;
        case WB_TERRAIN_TREE: return ASSET_TERRAIN_TREE;
        case WB_TERRAIN_WATER: return ASSET_TERRAIN_WATER;
        case WB_TERRAIN_MOUNTAIN: return ASSET_TERRAIN_MOUNTAIN;
        case WB_TERRAIN_NORMAL_UNDIGGABLE: return ASSET_TERRAIN_NORMAL_UNDIGGABLE;
        case WB_TERRAIN_WATER_UNFILLABLE: return ASSET_TERRAIN_WATER_UNDIGGABLE;
        case WB_TERRAIN_WATER_REEFS: return ASSET_TERRAIN_WATER_REEFS;
        case WB_TERRAIN_HOLE: return -1;
        case WB_TERRAIN_BILLBOARD: return ASSET_TERRAIN_BILLBOARD;
        case WB_TERRAIN_SWAMP: return ASSET_TERRAIN_SWAMP;
        case WB_TERRAIN_WATER_WHIRLPOOL: return ASSET_TERRAIN_WATER;
        case WB_TERRAIN_CEMENT: return ASSET_TERRAIN_CEMENT;
        case WB_TERRAIN_JUNGLE: return ASSET_TERRAIN_JUNGLE1 + (variant < 4 ? variant : 3);
        case WB_TERRAIN_ROADBLOCK: return ASSET_TERRAIN_ROADBLOCK;
        case WB_TERRAIN_STREET: return ASSET_TERRAIN_STREET1 + (variant < 10 ? variant : 9);
        case WB_TERRAIN_STREET_UNDIGGABLE: return ASSET_TERRAIN_STREET_UNDIGGABLE;
        case WB_TERRAIN_TREE2: return ASSET_TERRAIN_TREE2;
        case WB_TERRAIN_TREE3: return ASSET_TERRAIN_TREE3;
        case WB_TERRAIN_TREE4: return ASSET_TERRAIN_TREE4;
        default: return -1;
    }
}

static int minimapTerrainAssetId(WBTerrainType terrain, uint8_t variant) {
    switch (terrain) {
        case WB_TERRAIN_NORMAL: return ASSET_MINI_NORMAL;
        case WB_TERRAIN_TREE: return ASSET_MINI_TREE;
        case WB_TERRAIN_WATER: return ASSET_MINI_WATER;
        case WB_TERRAIN_MOUNTAIN: return ASSET_MINI_MOUNTAIN;
        case WB_TERRAIN_NORMAL_UNDIGGABLE: return ASSET_MINI_NORMAL_UNDIGGABLE;
        case WB_TERRAIN_WATER_UNFILLABLE: return ASSET_MINI_WATER_UNDIGGABLE;
        case WB_TERRAIN_WATER_REEFS: return ASSET_MINI_WATER_REEFS;
        case WB_TERRAIN_HOLE: return ASSET_MINI_HOLE;
        case WB_TERRAIN_BILLBOARD: return ASSET_MINI_HOLE;
        case WB_TERRAIN_SWAMP: return ASSET_MINI_SWAMP;
        case WB_TERRAIN_WATER_WHIRLPOOL: return ASSET_MINI_WATER;
        case WB_TERRAIN_CEMENT: return ASSET_MINI_CEMENT;
        case WB_TERRAIN_JUNGLE: return ASSET_MINI_JUNGLE1 + (variant < 4 ? variant : 3);
        case WB_TERRAIN_ROADBLOCK: return ASSET_MINI_ROADBLOCK;
        case WB_TERRAIN_STREET: return ASSET_MINI_STREET1 + (variant < 10 ? variant : 9);
        case WB_TERRAIN_STREET_UNDIGGABLE: return ASSET_MINI_STREET_UNDIGGABLE;
        case WB_TERRAIN_TREE2: return ASSET_MINI_TREE2;
        case WB_TERRAIN_TREE3: return ASSET_MINI_TREE3;
        case WB_TERRAIN_TREE4: return ASSET_MINI_TREE4;
        default: return -1;
    }
}

static C2D_Image getImage(const AppState* app, int assetId) {
    if (assetId >= ASSET_TITLE_LOGO && assetId <= ASSET_TITLE_WB2_IMAGE) {
        return C2D_SpriteSheetGetImage(app->titleMainSheet, assetId - ASSET_TITLE_LOGO);
    }
    if (assetId >= ASSET_TITLE_SKY1 && assetId <= ASSET_TITLE_SKY5) {
        static const int sheetIndex[5] = {0, 0, 1, 1, 2};
        static const int imageIndex[5] = {0, 1, 0, 1, 0};
        int idx = assetId - ASSET_TITLE_SKY1;
        return C2D_SpriteSheetGetImage(app->worldSkySheets[sheetIndex[idx]], imageIndex[idx]);
    }
    if (assetId >= ASSET_WORLDMAP1 && assetId <= ASSET_WORLDMAP5) {
        static const int sheetIndex[5] = {0, 0, 1, 1, 2};
        static const int imageIndex[5] = {0, 1, 0, 1, 0};
        int idx = assetId - ASSET_WORLDMAP1;
        return C2D_SpriteSheetGetImage(app->worldMapSheets[sheetIndex[idx]], imageIndex[idx]);
    }
    if (assetId == ASSET_WORLDMAP6) {
        return C2D_SpriteSheetGetImage(app->worldMapSheets[3], 0);
    }
    if (assetId == ASSET_WORLDMAP7) {
        return C2D_SpriteSheetGetImage(app->worldMapSheets[3], 1);
    }
    if (assetId == ASSET_WB2SKY1) {
        return C2D_SpriteSheetGetImage(app->worldSkySheets[3], 0);
    }
    if (assetId == ASSET_WB2SKY2) {
        return C2D_SpriteSheetGetImage(app->worldSkySheets[3], 1);
    }
    if (assetId >= ASSET_LICENSE_BASE && assetId <= ASSET_WB2_REWARD_CLASS3) {
        if (!app->licenseSheet) { C2D_Image blank; memset(&blank, 0, sizeof(blank)); return blank; }
        return C2D_SpriteSheetGetImage(app->licenseSheet, assetId - ASSET_LICENSE_BASE);
    }
    if (assetId >= ASSET_WB2_RESOURCES_BASE) {
        return C2D_SpriteSheetGetImage(app->wb2ResourcesSheet, assetId - ASSET_WB2_RESOURCES_BASE);
    }
    if (assetId >= ASSET_WB2_TERRAIN_BASE) {
        return C2D_SpriteSheetGetImage(app->wb2TerrainSheet, assetId - ASSET_WB2_TERRAIN_BASE);
    }
    if (assetId >= ASSET_WB2_BUILDINGS_BASE) {
        return C2D_SpriteSheetGetImage(app->wb2BuildingsSheet, assetId - ASSET_WB2_BUILDINGS_BASE);
    }
    if (assetId >= ASSET_WB2_VEHICLES_BASE) {
        return C2D_SpriteSheetGetImage(app->wb2VehiclesSheet, assetId - ASSET_WB2_VEHICLES_BASE);
    }
    if (assetId >= ASSET_WB2_MONSTERS_BASE) {
        return C2D_SpriteSheetGetImage(app->wb2MonstersSheet, assetId - ASSET_WB2_MONSTERS_BASE);
    }
    if (assetId >= ASSET_WHIRLPOOL_BASE) {
        return C2D_SpriteSheetGetImage(app->whirlpoolSheet, assetId - ASSET_WHIRLPOOL_BASE);
    }
    if (assetId >= ASSET_BUILDINGS_BASE) {
        return C2D_SpriteSheetGetImage(app->buildingsSheet, assetId - ASSET_BUILDINGS_BASE);
    }
    if (assetId >= ASSET_MONSTERS_B_BASE) {
        return C2D_SpriteSheetGetImage(app->monstersBSheet, assetId - ASSET_MONSTERS_B_BASE);
    }
    if (assetId >= ASSET_MONSTERS_A_BASE) {
        return C2D_SpriteSheetGetImage(app->monstersASheet, assetId - ASSET_MONSTERS_A_BASE);
    }
    if (assetId >= ASSET_VEHICLES_ROBOT_BASE) {
        return C2D_SpriteSheetGetImage(app->vehiclesRobotSheet, assetId - ASSET_VEHICLES_ROBOT_BASE);
    }
    if (assetId >= ASSET_VEHICLES_ANIMAL_BASE) {
        return C2D_SpriteSheetGetImage(app->vehiclesAnimalSheet, assetId - ASSET_VEHICLES_ANIMAL_BASE);
    }
    if (assetId >= ASSET_VEHICLES_WATER_BASE) {
        return C2D_SpriteSheetGetImage(app->vehiclesWaterSheet, assetId - ASSET_VEHICLES_WATER_BASE);
    }
    if (assetId >= ASSET_VEHICLES_LAND_BASE) {
        return C2D_SpriteSheetGetImage(app->vehiclesLandSheet, assetId - ASSET_VEHICLES_LAND_BASE);
    }
    if (assetId >= ASSET_WORLD_QUESTION_MARK) {
        return C2D_SpriteSheetGetImage(app->worldIconSheet, assetId - ASSET_WORLD_QUESTION_MARK);
    }
    return C2D_SpriteSheetGetImage(app->spriteSheet, assetId);
}

/* Extra icons appended to worldbuilder.t3s after build_cloud2. */
#define WB_EXTRA_UI_LOW_ENERGY_IDX   100
#define WB_EXTRA_UI_CHARGE_0_IDX     101
#define WB_EXTRA_UI_CHARGE_1_IDX     102
#define WB_EXTRA_UI_CHARGE_2_IDX     103
#define WB_EXTRA_CARRY_RED_IDX       104
#define WB_EXTRA_CARRY_YELLOW_IDX    105
#define WB_EXTRA_CARRY_GREEN_IDX     106
#define WB_EXTRA_CARRY_BLUE_IDX      107
#define WB_EXTRA_CARRY_WHEEL_IDX     108
#define WB_EXTRA_CARRY_ENERGY_IDX    109
#define WB_EXTRA_CARRY_ENERGY_LOW_IDX 110
#define WB_EXTRA_CARRY_ENERGY_DEAD_IDX 111

static C2D_Image getWorldbuilderExtraImage(const AppState* app, int sheetIndex) {
    C2D_Image img;
    img.subtex = NULL;
    img.tex = NULL;
    if (!app || !app->spriteSheet) {
        return img;
    }
    if (sheetIndex < 0 || sheetIndex >= (int)C2D_SpriteSheetCount(app->spriteSheet)) {
        return img;
    }
    return C2D_SpriteSheetGetImage(app->spriteSheet, sheetIndex);
}

static float imageWidth(C2D_Image image, float scale) {
    return image.subtex ? image.subtex->width * scale : 0.0f;
}

static float imageHeight(C2D_Image image, float scale) {
    return image.subtex ? image.subtex->height * scale : 0.0f;
}

static u32 readLE32(const u8* p) {
    return (u32) p[0] | ((u32) p[1] << 8) | ((u32) p[2] << 16) | ((u32) p[3] << 24);
}

static u16 readLE16(const u8* p) {
    return (u16) p[0] | ((u16) p[1] << 8);
}

static bool loadWavClip(const char* path, WBAudioClip* outClip) {
    FILE* fp = fopen(path, "rb");
    long fileSize;
    u8* fileData;
    u8* cursor;
    u8* end;
    u16 audioFormat = 0;
    u16 channels = 0;
    u32 sampleRate = 0;
    u16 bitsPerSample = 0;
    u8* pcmData = NULL;
    u32 pcmSize = 0;

    memset(outClip, 0, sizeof(*outClip));
    if (!fp) {
        return false;
    }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fileSize < 44) {
        fclose(fp);
        return false;
    }
    fileData = (u8*) malloc((size_t) fileSize);
    if (!fileData) {
        fclose(fp);
        return false;
    }
    if (fread(fileData, 1, (size_t) fileSize, fp) != (size_t) fileSize) {
        free(fileData);
        fclose(fp);
        return false;
    }
    fclose(fp);
    if (memcmp(fileData, "RIFF", 4) != 0 || memcmp(fileData + 8, "WAVE", 4) != 0) {
        free(fileData);
        return false;
    }
    cursor = fileData + 12;
    end = fileData + fileSize;
    while (cursor + 8 <= end) {
        u32 chunkSize = readLE32(cursor + 4);
        u8* chunkData = cursor + 8;
        if (memcmp(cursor, "fmt ", 4) == 0 && chunkSize >= 16 && chunkData + chunkSize <= end) {
            audioFormat = readLE16(chunkData + 0);
            channels = readLE16(chunkData + 2);
            sampleRate = readLE32(chunkData + 4);
            bitsPerSample = readLE16(chunkData + 14);
        } else if (memcmp(cursor, "data", 4) == 0 && chunkData + chunkSize <= end) {
            pcmData = chunkData;
            pcmSize = chunkSize;
        }
        cursor = chunkData + chunkSize + (chunkSize & 1);
    }
    if (audioFormat != 1 || bitsPerSample != 16 || !pcmData || pcmSize == 0 || (channels != 1 && channels != 2)) {
        free(fileData);
        return false;
    }
    if (channels == 1) {
        u32 sampleCount = pcmSize / sizeof(int16_t);
        s16* src = (s16*) pcmData;
        s16* dst;
        u32 i;
        outClip->data = linearAlloc(sampleCount * sizeof(int16_t) * 2);
        if (!outClip->data) {
            free(fileData);
            return false;
        }
        dst = (s16*) outClip->data;
        for (i = 0; i < sampleCount; ++i) {
            dst[i * 2 + 0] = src[i];
            dst[i * 2 + 1] = src[i];
        }
        DSP_FlushDataCache(outClip->data, sampleCount * sizeof(int16_t) * 2);
        outClip->size = sampleCount * sizeof(int16_t) * 2;
        outClip->sampleRate = sampleRate;
        outClip->channels = 2;
        outClip->loaded = true;
        free(fileData);
        return true;
    }
    outClip->data = linearAlloc(pcmSize);
    if (!outClip->data) {
        free(fileData);
        return false;
    }
    memcpy(outClip->data, pcmData, pcmSize);
    DSP_FlushDataCache(outClip->data, pcmSize);
    outClip->size = pcmSize;
    outClip->sampleRate = sampleRate;
    outClip->channels = channels;
    outClip->loaded = true;
    free(fileData);
    return true;
}

static void freeWavClip(WBAudioClip* clip) {
    if (clip->data) {
        linearFree(clip->data);
    }
    memset(clip, 0, sizeof(*clip));
}

static void setupAudioChannel(int channel, int sampleRate, int channels, float volume) {
    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = volume;
    mix[1] = volume;
    ndspChnReset(channel);
    ndspChnWaveBufClear(channel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspSetMasterVol(1.0f);
    ndspChnSetInterp(channel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(channel, (float) sampleRate);
    ndspChnSetFormat(channel, channels == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);
    ndspChnSetMix(channel, mix);
}

static void playClip(AppState* app, int channel, WBAudioClip* clip, bool looped) {
    float vol;
    if (!app->audioReady || !clip || !clip->loaded) {
        return;
    }
    vol = (clip->volume > 0.0f) ? clip->volume : 1.0f;
    setupAudioChannel(channel, (int) clip->sampleRate, clip->channels, vol);
    memset(&clip->waveBuf, 0, sizeof(clip->waveBuf));
    clip->waveBuf.data_vaddr = clip->data;
    clip->waveBuf.data_pcm16 = (s16*) clip->data;
    clip->waveBuf.nsamples = clip->size / (sizeof(int16_t) * clip->channels);
    clip->waveBuf.looping = looped;
    DSP_FlushDataCache(clip->data, clip->size);
    ndspChnWaveBufAdd(channel, &clip->waveBuf);
}

static void playMusicClip(AppState* app, WBAudioClip* clip, bool looped) {
    if (!app->audioReady || app->muteMusic) {
        ndspChnWaveBufClear(MUSIC_CHANNEL);
        return;
    }
    /* Timer-based advancement: looped clips never auto-advance; for one-shot clips
       schedule the next track after the clip's exact duration plus a small buffer. */
    if (looped || !clip || !clip->loaded) {
        app->musicRetryAtMs = (u64)-1; /* never auto-advance */
    } else {
        u64 nsamples = (u64)clip->size / ((u64)sizeof(int16_t) * (u64)clip->channels);
        u64 durationMs = nsamples * 1000ULL / (u64)clip->sampleRate;
        app->musicRetryAtMs = osGetTime() + durationMs + 150ULL;
    }
    app->musicStartMs = osGetTime();
    playClip(app, MUSIC_CHANNEL, clip, looped);
}

static void playSfxClip(AppState* app, WBAudioClip* clip) {
    if (!app->audioReady || app->muteSfx) {
        return;
    }
    if (osGetTime() < app->goalSfxUntilMs) {
        return;  /* goal sound in progress; protect it from interruption */
    }
    ++app->sfxPlayCount;
    playClip(app, SFX_CHANNEL, clip, false);
}

/* For world/combat/tick-based sounds (plays on SFX2_CHANNEL so they don't
   override user-triggered sounds on SFX_CHANNEL). */
static void playSfxClipWorld(AppState* app, WBAudioClip* clip) {
    if (!app->audioReady || app->muteSfx) {
        return;
    }
    if (osGetTime() < app->goalSfxUntilMs) {
        return;  /* goal sound in progress; protect it from interruption */
    }
    ++app->sfxPlayCount;
    playClip(app, SFX2_CHANNEL, clip, false);
}

/* Priority SFX: plays immediately regardless of other guards, and blocks
   subsequent SFX for its full duration (used for goal/bonus-goal sounds). */
static void playSfxClipPriority(AppState* app, WBAudioClip* clip) {
    u64 nsamples;
    u64 durationMs;
    if (!app->audioReady || app->muteSfx || !clip || !clip->loaded) {
        return;
    }
    nsamples   = (u64)clip->size / ((u64)sizeof(int16_t) * (u64)clip->channels);
    durationMs = nsamples * 1000ULL / (u64)clip->sampleRate;
    app->goalSfxUntilMs = osGetTime() + durationMs;
    ++app->sfxPlayCount;
    playClip(app, SFX_CHANNEL, clip, false);
}

static WBAudioClip* chooseRandomLoadedClip(WBAudioClip* clips, int count) {
    int i;
    int loadedCount = 0;
    int pick;
    for (i = 0; i < count; ++i) {
        if (clips[i].loaded) {
            ++loadedCount;
        }
    }
    if (loadedCount <= 0) {
        return NULL;
    }
    pick = (rand() % loadedCount) + 1;
    for (i = 0; i < count; ++i) {
        if (!clips[i].loaded) {
            continue;
        }
        --pick;
        if (pick == 0) {
            return &clips[i];
        }
    }
    return NULL;
}

static int chooseNextGameMusicFamilyIndex(AppState* app) {
    static const char songKeys[] = { 'a', 'i', '6', '8', 'i', '6', '8' };
    int pick = app->musicLastGameSongIndex;
    int tries = 0;

    while (tries < 12) {
        pick = rand() % 7;
        if (pick != app->musicLastGameSongIndex) {
            break;
        }
        ++tries;
    }
    app->musicLastGameSongIndex = pick;

    switch (songKeys[pick]) {
        case '6': return '6';
        case '8': return '8';
        case 'a': return 'a';
        case 'i': return 'i';
        default: return -1;
    }
}

/* Pick a random loaded segment from the given music family. */
static WBAudioClip* chooseRandomGameMusicClipForFamily(int familyIndex, AppState* app) {
    switch (familyIndex) {
        case '6': return chooseRandomLoadedClip(app->musicGame6Variants, WB_MUSIC_GAME6_VARIANTS);
        case '8': return chooseRandomLoadedClip(app->musicGame8Variants, WB_MUSIC_GAME8_VARIANTS);
        case 'a': return chooseRandomLoadedClip(app->musicGameAVariants, WB_MUSIC_GAMEA_VARIANTS);
        case 'i': return chooseRandomLoadedClip(app->musicGameIVariants, WB_MUSIC_GAMEI_VARIANTS);
        default: return NULL;
    }
}

static void playCurrentMusicTrack(AppState* app) {
    WBAudioClip* clip = NULL;
    bool looped = false;
    if (!app->audioReady || app->muteMusic) {
        ndspChnWaveBufClear(MUSIC_CHANNEL);
        return;
    }
    if (app->musicUseGamePlaylist) {
        /* Pick a random segment from the family chosen at level load; loop it. */
        clip = chooseRandomGameMusicClipForFamily(app->musicBpm, app);
        if (!clip || !clip->loaded) {
            clip = &app->musicGame;
        }
        looped = true;
    } else {
        clip = chooseRandomLoadedClip(app->musicIntroVariants, WB_MUSIC_INTRO_VARIANTS);
        if (!clip) {
            clip = &app->musicIntro;
            looped = true;
        }
    }
    if (clip && clip->loaded) {
        playMusicClip(app, clip, looped);
    }
}

static void startMenuMusic(AppState* app) {
    app->musicUseGamePlaylist = false;
    app->musicBpm = 120;
    playCurrentMusicTrack(app);
}

static void startGameMusic(AppState* app) {
    app->musicUseGamePlaylist = true;
    app->musicBpm = chooseNextGameMusicFamilyIndex(app);
    playCurrentMusicTrack(app);
}

static void showGoalPopup(AppState* app, const char* text, bool bonus, bool complete, int tx, int ty) {
    app->goalPopupVisible = true;
    app->goalPopupBonus = bonus;
    app->goalPopupComplete = complete;
    app->goalPopupX = tx;
    app->goalPopupY = ty;
    strncpy(app->goalPopupText, text ? text : "", sizeof(app->goalPopupText) - 1);
    app->goalPopupText[sizeof(app->goalPopupText) - 1] = '\0';
}

static void updateAudioRuntime(AppState* app) {
    if (!app->audioReady || app->muteMusic) {
        return;
    }
    if ((u64) osGetTime() < app->musicRetryAtMs) {
        return;
    }
    playCurrentMusicTrack(app);
}

static void drawAnchoredImage(C2D_Image image, float x, float y, float scale, float anchorX, float anchorY) {
    if (!image.subtex) {
        return;
    }
    C2D_DrawImageAt(image, x - (anchorX * scale), y - (anchorY * scale), 0.0f, NULL, scale, scale);
}

static bool imageIntersectsScreen(C2D_Image image, float x, float y, float scale, float anchorX, float anchorY, float screenW, float screenH) {
    if (!image.subtex) {
        return false;
    }
    float left = x - (anchorX * scale);
    float top = y - (anchorY * scale);
    float w = imageWidth(image, scale);
    float h = imageHeight(image, scale);
    return !((left + w) < 0.0f || left >= screenW || (top + h) < 0.0f || top >= screenH);
}

static void drawTextLine(C2D_TextBuf buf, float x, float y, float scale, const char* text) {
    C2D_Text tmp;
    C2D_TextParse(&tmp, buf, text);
    C2D_TextOptimize(&tmp);
    C2D_DrawText(&tmp, 0, x, y, 0.5f, scale, scale);
}

static void drawTextLineWhite(C2D_TextBuf buf, float x, float y, float scale, const char* text) {
    C2D_Text tmp;
    C2D_TextParse(&tmp, buf, text);
    C2D_TextOptimize(&tmp);
    C2D_DrawText(&tmp, C2D_WithColor, x, y, 0.5f, scale, scale, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
}

static void drawTextLineBlack(C2D_TextBuf buf, float x, float y, float scale, const char* text) {
    C2D_Text tmp;
    C2D_TextParse(&tmp, buf, text);
    C2D_TextOptimize(&tmp);
    C2D_DrawText(&tmp, C2D_WithColor, x, y, 0.5f, scale, scale, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
}

static bool inBounds(const AppState* app, int x, int y) {
    return x >= 0 && x < app->map.width && y >= 0 && y < app->map.height;
}

static int randRange(int minValue, int maxValue) {
    if (maxValue <= minValue) {
        return minValue;
    }
    return minValue + (rand() % ((maxValue - minValue) + 1));
}

static bool packagedMissionExists(int world, int mission) {
    char path[64];
    FILE* fp;
    snprintf(path, sizeof(path), "romfs:/maps/map%d_%d.txt", world, mission);
    fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }
    fclose(fp);
    return true;
}

static int worldSkyAssetId(int world) {
    if (world < 1 || world > 7) {
        return ASSET_TITLE_SKY1;
    }
    return g_worldSkyAssets[world];
}

static int worldMapAssetId(int world) {
    if (world < 1 || world > 7) {
        return ASSET_WORLDMAP1;
    }
    return g_worldMapAssets[world];
}

static const char* worldName(int world) {
    switch (world) {
        case 1: return "World 1";
        case 2: return "World 2";
        case 3: return "World 3";
        case 4: return "Ocean World";
        case 5: return "Prehistoric World";
        case 6: return "WB2 World 1";
        case 7: return "WB2 World 2";
        default: return "World";
    }
}

static int worldMissionState(const AppState* app, int world, int mission) {
    if (world < 1 || world > 7 || mission < 1 || mission > 12) {
        return -1;
    }
    return app->levelState[world][mission];
}

static bool parseWorldMetadata(AppState* app) {
    FILE* fp;
    char line[256];
    int world;
    int mission;
    memset(app->levelState, 0xFF, sizeof(app->levelState));
    memset(app->levelOpenCount, 0, sizeof(app->levelOpenCount));
    memset(app->levelOpens, 0, sizeof(app->levelOpens));
    memset(app->missionNames, 0, sizeof(app->missionNames));

    fp = fopen("romfs:/meta/worlds.txt", "r");
    if (!fp) {
        return false;
    }
    while (fgets(line, sizeof(line), fp)) {
        char opens[64] = {0};
        int read = sscanf(line, "%d %d %63s", &world, &mission, opens);
        if (read >= 2 && world >= 1 && world <= 7 && mission >= 1 && mission <= 12) {
            char* token;
            int idx = 0;
            if (read == 3) {
                token = strtok(opens, ",");
                while (token && idx < 4) {
                    app->levelOpens[world][mission][idx++] = atoi(token);
                    token = strtok(NULL, ",");
                }
            }
            app->levelOpenCount[world][mission] = idx;
            app->levelState[world][mission] = -1;
        }
    }
    fclose(fp);

    app->levelState[1][1] = 0;
    app->levelState[4][1] = 0;
    app->levelState[5][1] = 0;
    app->levelState[6][1] = 0;
    /* WB2 World 2 (world 7) requires completing WB2 World 1 — unlocked via applyMissionSuccess */

    fp = fopen("romfs:/meta/level_names.txt", "r");
    if (!fp) {
        return false;
    }
    while (fgets(line, sizeof(line), fp)) {
        char* p = line;
        char* name;
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
        world = (int) strtol(p, &p, 10);
        mission = (int) strtol(p, &p, 10);
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
        name = p;
        while (*name && (*name == ' ' || *name == '\t')) {
            ++name;
        }
        if (world >= 1 && world <= 7 && mission >= 1 && mission <= 12) {
            int len = (int)strlen(name);
            while (len > 0 && (name[len - 1] == '\r' || name[len - 1] == '\n')) {
                name[--len] = '\0';
            }
            strncpy(app->missionNames[world][mission], name, WB_MAX_NAME - 1);
        }
    }
    fclose(fp);
    return true;
}

static void applyMissionSuccess(AppState* app, int world, int mission, int state) {
    int current;
    int i;
    current = app->levelState[world][mission];
    if (current < 1) {
        app->levelState[world][mission] = state;
    } else if (current != state) {
        app->levelState[world][mission] = 3;
    }
    if (app->levelState[world][mission] == 1 || app->levelState[world][mission] == 3) {
        for (i = 0; i < app->levelOpenCount[world][mission]; ++i) {
            int openMission = app->levelOpens[world][mission][i];
            if (openMission > 12) {
                if (world < 7 && app->levelState[world + 1][1] < 0) {
                    app->levelState[world + 1][1] = 0;
                }
            } else if (openMission >= 1 && openMission <= 12 && app->levelState[world][openMission] < 0) {
                app->levelState[world][openMission] = 0;
            }
        }
    }
    saveLevelProgress(app);
}

static void saveLevelProgress(const AppState* app) {
    int w, m;
    FILE* fp;
    /* Ensure directory exists (ignore error if already present) */
    mkdir("sdmc:/legowb3ds", 0777);
    fp = fopen(WB_SAVE_PATH, "w");
    if (!fp) { return; }
    for (w = 1; w <= 7; ++w) {
        for (m = 1; m <= 12; ++m) {
            int st = app->levelState[w][m];
            if (st >= 0) {
                fprintf(fp, "%d %d %d\n", w, m, st);
            }
        }
    }
    fclose(fp);
}

static void loadLevelProgress(AppState* app) {
    FILE* fp;
    char line[64];
    fp = fopen(WB_SAVE_PATH, "r");
    if (!fp) { return; }
    while (fgets(line, sizeof(line), fp)) {
        int w = 0, m = 0, st = 0;
        if (sscanf(line, "%d %d %d", &w, &m, &st) == 3) {
            if (w >= 1 && w <= 7 && m >= 1 && m <= 12 && st >= 0 && st <= 3) {
                /* Only update if the level is known (not permanently locked at -1 from worlds.txt) */
                if (app->levelState[w][m] >= -1) {
                    applyMissionSuccess(app, w, m, st);
                    if (st == 0 && app->levelState[w][m] < 0) {
                        app->levelState[w][m] = 0;
                    }
                }
            }
        }
    }
    fclose(fp);
    /* Re-validate cross-world unlocks: worlds 2, 3, and 7 are gated by completing
       the previous world. If a stale save has their mission 1 as 0 (unlocked-not-played)
       but no completed mission in the predecessor world actually unlocks it, revert. */
    {
        int w;
        for (w = 2; w <= 7; ++w) {
            int justified;
            int prev;
            int m;
            if (w == 4 || w == 5 || w == 6) continue; /* default-open worlds */
            if (app->levelState[w][1] != 0) continue;  /* locked or already played */
            justified = 0;
            prev = w - 1;
            for (m = 1; m <= 12 && !justified; ++m) {
                int st = app->levelState[prev][m];
                if (st == 1 || st == 3) {
                    int j;
                    for (j = 0; j < app->levelOpenCount[prev][m]; ++j) {
                        if (app->levelOpens[prev][m][j] > 12) {
                            justified = 1;
                            break;
                        }
                    }
                }
            }
            if (!justified) {
                app->levelState[w][1] = -1;
            }
        }
    }
}

static void clearSaveData(AppState* app) {
    FILE* fp;
    fp = fopen(WB_SAVE_PATH, "w");
    if (fp) { fclose(fp); }
    /* Re-parse fresh metadata, restoring only the default-open levels */
    parseWorldMetadata(app);
}

static void unlockAllLevels(AppState* app) {
    int w, m;
    for (w = 1; w <= 7; ++w) {
        for (m = 1; m <= 12; ++m) {
            /* Only unlock levels that are known in worlds.txt (state >= -1 means known) */
            if (app->levelState[w][m] >= -1) {
                applyMissionSuccess(app, w, m, 1);
            }
        }
    }
    saveLevelProgress(app);
}

static u8* memsearch(u8* buf, size_t bufLen, u8* cmp, size_t cmpLen) {
    u8* origin = buf;
    while (bufLen - ((size_t) (buf - origin)) > 0 && (buf = memchr(buf, *cmp, bufLen - ((size_t) (buf - origin))))) {
        if (memcmp(buf, cmp, cmpLen) == 0) {
            return buf;
        }
        buf++;
    }
    return NULL;
}

static u32 blzDecompressSize(u8* compressed, u32 compressedSize) {
    return compressedSize + *(u32*) (compressed + compressedSize - 4);
}

static bool blzDecompress(u8* compressed, u32 compressedSize, u8* decompressed, u32 decompressedSize) {
    u8* footer = compressed + compressedSize - 8;
    u32 bufferTopAndBottom = (footer[0] << 0) | (footer[1] << 8) | (footer[2] << 16) | (footer[3] << 24);
    u32 out = decompressedSize;
    u32 index = compressedSize - ((bufferTopAndBottom >> 24) & 0xFF);
    u32 stopIndex = compressedSize - (bufferTopAndBottom & 0xFFFFFF);
    u32 i;

    memset(decompressed, 0, decompressedSize);
    memcpy(decompressed, compressed, compressedSize);

    while (index > stopIndex) {
        u8 control = compressed[--index];
        for (i = 0; i < 8; ++i) {
            if (index <= stopIndex || out <= 0) {
                break;
            }
            if (control & 0x80) {
                u32 j;
                u32 segmentOffset;
                u32 segmentSize;
                if (index < 2) {
                    return false;
                }
                index -= 2;
                segmentOffset = compressed[index] | (compressed[index + 1] << 8);
                segmentSize = ((segmentOffset >> 12) & 15) + 3;
                segmentOffset = (segmentOffset & 0x0FFF) + 2;
                if (out < segmentSize) {
                    return false;
                }
                for (j = 0; j < segmentSize; ++j) {
                    u8 data;
                    if (out + segmentOffset >= decompressedSize) {
                        return false;
                    }
                    data = decompressed[out + segmentOffset];
                    decompressed[--out] = data;
                }
            } else {
                if (out < 1 || index < 1) {
                    return false;
                }
                decompressed[--out] = compressed[--index];
            }
            control <<= 1;
        }
    }
    return true;
}

static bool initAudioSystem(AppState* app) {
    static u8* dspBuf = NULL;
    Result res = ndspInit();
    app->audioInitResult = res;
    app->audioFallbackOpenResult = 0;
    app->audioFallbackInitResult = 0;
    if (R_SUCCEEDED(res)) {
        ndspSetOutputMode(NDSP_OUTPUT_STEREO);
        return true;
    }

    {
        static const u64 tidHigh = 0x0004003000000000ULL;
        static const u32 tidLowHome[6] = {
            0x0000F202, 0x00008202, 0x00009802, 0x0000A102, 0x0000A902, 0x0000B102
        };
        static const u32 filePathRaw[] = {0x00000000, 0x00000000, 0x00000002, 0x646F632E, 0x00000065};
        Handle file = 0;
        u64 fileSize = 0;
        u32 readSize = 0;
        u8* compressed = NULL;
        u8* decompressed = NULL;
        int i;

        for (i = 0; i < 6; ++i) {
            u64 tid = tidHigh | tidLowHome[i];
            u32 archPathRaw[] = {(u32) (tid & 0xFFFFFFFFULL), (u32) ((tid >> 32) & 0xFFFFFFFFULL), 0, 0};
            FS_Path archPath = {PATH_BINARY, 0x10, (u8*) archPathRaw};
            FS_Path filePath = {PATH_BINARY, 0x14, (u8*) filePathRaw};
            res = FSUSER_OpenFileDirectly(&file, (FS_ArchiveID) 0x2345678A, archPath, filePath, FS_OPEN_READ, 0);
            if (R_SUCCEEDED(res)) {
                break;
            }
        }
        app->audioFallbackOpenResult = res;
        if (R_FAILED(res)) {
            return false;
        }
        if (R_FAILED(FSFILE_GetSize(file, &fileSize))) {
            FSFILE_Close(file);
            return false;
        }
        compressed = (u8*) malloc((size_t) fileSize);
        if (!compressed) {
            FSFILE_Close(file);
            return false;
        }
        res = FSFILE_Read(file, &readSize, 0, compressed, (u32) fileSize);
        FSFILE_Close(file);
        if (R_FAILED(res) || readSize != (u32) fileSize) {
            free(compressed);
            return false;
        }
        {
            u32 decompressedSize = blzDecompressSize(compressed, (u32) fileSize);
            u8* dspLoc;
            const char* magic = "DSP1";
            decompressed = (u8*) malloc(decompressedSize);
            if (!decompressed) {
                free(compressed);
                return false;
            }
            if (!blzDecompress(compressed, (u32) fileSize, decompressed, decompressedSize)) {
                free(compressed);
                free(decompressed);
                return false;
            }
            dspLoc = memsearch(decompressed, decompressedSize, (u8*) magic, 4);
            free(compressed);
            if (!dspLoc) {
                free(decompressed);
                return false;
            }
            {
                u32 dspSize = *(u32*) (dspLoc + 4);
                dspLoc -= 0x100;
                dspBuf = (u8*) malloc(dspSize);
                if (!dspBuf) {
                    free(decompressed);
                    return false;
                }
                memcpy(dspBuf, dspLoc, dspSize);
                free(decompressed);
                ndspUseComponent(dspBuf, dspSize, 0xFF, 0xFF);
            }
        }
        res = ndspInit();
        app->audioFallbackInitResult = res;
        if (R_SUCCEEDED(res)) {
            ndspSetOutputMode(NDSP_OUTPUT_STEREO);
            return true;
        }
    }

    (void) app;
    return false;
}

static void clampCamera(AppState* app) {
    int maxX = app->map.width - VIEW_TILE_W + 1 + 5;
    int maxY = app->map.height - VIEW_TILE_H + 1 + 3;
    if (app->cameraX < -1) app->cameraX = -1;
    if (app->cameraY < -2) app->cameraY = -2;
    if (app->cameraX > maxX) app->cameraX = maxX;
    if (app->cameraY > maxY) app->cameraY = maxY;
}

/* readmap() in 'map display manager' opens on the middle of the map, or on its
   [map] center=x,y, scrolling the top-left of the 12x9 window from (1,1) by
   (c - 12/2 + 1, c - 9/2) in Lingo integer arithmetic, then clamping as clampCamera(). */
static void resetCameraForMap(AppState* app) {
    if (app->map.hasCenter) {
        app->cameraX = 1 + (app->map.centerX - (VIEW_TILE_W / 2) + 1);
        app->cameraY = 1 + (app->map.centerY - (VIEW_TILE_H / 2));
    } else {
        app->cameraX = 1 + ((app->map.width / 2) - (VIEW_TILE_W / 2) + 1);
        app->cameraY = 1 + ((app->map.height / 2) - (VIEW_TILE_H / 2));
    }
    clampCamera(app);
}

/* clickToScroll(): one tile per press of a scroll button, repeating while it is held
   but no faster than every 150 ms. (The pad used to move the view a tile per frame.) */
static bool scrollCameraStep(AppState* app, int dx, int dy) {
    u64 now = osGetTime();
    int oldX = app->cameraX;
    int oldY = app->cameraY;
    if (dx == 0 && dy == 0) {
        return false;
    }
    if (app->lastScrollMs != 0 && now - app->lastScrollMs < 150) {
        return false;
    }
    app->lastScrollMs = now;
    app->cameraX += dx;
    app->cameraY += dy;
    clampCamera(app);
    return app->cameraX != oldX || app->cameraY != oldY;
}

static void posToLoc(const AppState* app, int posX, int posY, float* outX, float* outY) {
    float x = ((posX - app->cameraX) * DIRECTOR_TILE_SIZE_X)
        + DIRECTOR_PIXEL_TOPLEFT_X
        + (DIRECTOR_PIXEL_SKEW_X * (VIEW_TILE_H + (app->cameraY - posY) - 2));
    float y = ((posY - app->cameraY) * DIRECTOR_TILE_SIZE_Y) + DIRECTOR_PIXEL_TOPLEFT_Y;
    *outX = roundf(WORLD_OFFSET_X + (x * DIRECTOR_SCALE));
    *outY = roundf(WORLD_OFFSET_Y + (y * DIRECTOR_SCALE));
}

static int unitAssetId(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:     return ASSET_VEHICLE_BUGGY;
        case WB_UNIT_DUCK:      return ASSET_VEHICLE_DUCK;
        case WB_UNIT_DIRTBUGGY: return ASSET_VEHICLE_DIRTBUGGY_RIGHT;
        case WB_UNIT_STEAMSHOVEL: return ASSET_VEHICLE_STEAMSHOVEL_RIGHT;
        case WB_UNIT_DUMPTRUCK: return ASSET_VEHICLE_DUMPTRUCK_RIGHT;
        case WB_UNIT_FORKLIFT:  return ASSET_VEHICLE_FORKLIFT_RIGHT;
        case WB_UNIT_DOZER:     return ASSET_VEHICLE_DOZER_RIGHT;
        case WB_UNIT_SPEEDBOAT: return ASSET_VEHICLE_SPEEDBOAT_RIGHT;
        case WB_UNIT_TUGBOAT:   return ASSET_VEHICLE_TUGBOAT_RIGHT;
        case WB_UNIT_FREIGHTER: return ASSET_VEHICLE_FREIGHTER_RIGHT;
        case WB_UNIT_FROG:      return ASSET_VEHICLE_FROG_RIGHT;
        case WB_UNIT_FISH:      return ASSET_VEHICLE_FISH_WATER_RIGHT;
        case WB_UNIT_SNAIL:     return ASSET_VEHICLE_SNAIL_RIGHT;
        case WB_UNIT_TREEBOT:   return ASSET_VEHICLE_TREEBOT_RIGHT;
        case WB_UNIT_REPAIRBOT: return ASSET_VEHICLE_REPAIRBOT_RIGHT;
        case WB_UNIT_DEFENDER:   return ASSET_VEHICLE_DEFENDER_RIGHT;
        case WB_UNIT_DEFENDER2:  return ASSET_VEHICLE_DEFENDER_RIGHT;
        case WB_UNIT_FREEZEBOT:  return ASSET_VEHICLE_FREEZEBOT;
        default: return -1;
    }
}

static int planAssetId(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_BUGGY:      return ASSET_PLAN_BUGGY;
        case WB_PLAN_DUCK:       return ASSET_PLAN_DUCK;
        case WB_PLAN_DIRTBUGGY:  return ASSET_PLAN_DIRTBUGGY;
        case WB_PLAN_STEAMSHOVEL:return ASSET_PLAN_STEAMSHOVEL;
        case WB_PLAN_DUMPTRUCK:  return ASSET_PLAN_DUMPTRUCK;
        case WB_PLAN_FORKLIFT:   return ASSET_PLAN_FORKLIFT;
        case WB_PLAN_DOZER:      return ASSET_PLAN_DOZER;
        case WB_PLAN_SPEEDBOAT:  return ASSET_PLAN_SPEEDBOAT;
        case WB_PLAN_TUGBOAT:    return ASSET_PLAN_TUGBOAT;
        case WB_PLAN_FREIGHTER:  return ASSET_PLAN_FREIGHTER;
        case WB_PLAN_FROG:       return ASSET_PLAN_FROG;
        case WB_PLAN_FISH:       return ASSET_PLAN_FISH;
        case WB_PLAN_SNAIL:      return ASSET_PLAN_SNAIL;
        case WB_PLAN_TREEBOT:    return ASSET_PLAN_TREEBOT;
        case WB_PLAN_REPAIRBOT:  return ASSET_PLAN_REPAIRBOT;
        case WB_PLAN_DEFENDER:   return ASSET_PLAN_DEFENDER;
        case WB_PLAN_FREEZEBOT:  return ASSET_PLAN_FREEZEBOT;
        case WB_PLAN_HOUSE:
        case WB_PLAN_FACTORY:
        case WB_PLAN_WINDMILL:
        case WB_PLAN_GARAGE:
        case WB_PLAN_NURSERY:    return buildingPanelIconAssetId(planTypeToBuilding(planType));
        case WB_PLAN_GUARD_TOWER: return ASSET_PLAN_GUARD_TOWER;
        case WB_PLAN_GAS_STATION: return ASSET_PLAN_GAS_STATION;
        case WB_PLAN_MARINA:      return ASSET_PLAN_MARINA;
        case WB_PLAN_ROBOT_LAB:   return ASSET_PLAN_ROBOT_LAB;
        case WB_PLAN_AIRPORT:    return ASSET_PLAN_GENERIC;
        default: return -1;
    }
}

static int worldPlanAssetId(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_GAS_STATION: return ASSET_PLAN_GENERIC;
        case WB_PLAN_MARINA:      return ASSET_PLAN_GENERIC;
        case WB_PLAN_ROBOT_LAB:   return ASSET_PLAN_GENERIC;
        case WB_PLAN_GUARD_TOWER: return ASSET_PLAN_GENERIC;
        case WB_PLAN_HOUSE:       return ASSET_PLAN_GENERIC;
        case WB_PLAN_FACTORY:     return ASSET_PLAN_GENERIC;
        case WB_PLAN_WINDMILL:    return ASSET_PLAN_GENERIC;
        case WB_PLAN_GARAGE:      return ASSET_PLAN_GENERIC;
        case WB_PLAN_NURSERY:     return ASSET_PLAN_GENERIC;
        default: return ASSET_PLAN_GENERIC;
    }
}

/* string(terrain) contains "water" */
static bool terrainUsesWaterSprite(WBTerrainType terrain) {
    return terrain == WB_TERRAIN_WATER || terrain == WB_TERRAIN_WATER_REEFS || terrain == WB_TERRAIN_WATER_UNFILLABLE ||
           terrain == WB_TERRAIN_WATER_WHIRLPOOL;
}

#define WB_UNIT_ANIM_NONE   0
#define WB_UNIT_ANIM_DIG    1
#define WB_UNIT_ANIM_FILL   2
#define WB_UNIT_ANIM_UPROOT 3
#define WB_UNIT_ANIM_PLANT  4
#define WB_UNIT_ANIM_REPAIR 5
#define WB_UNIT_ANIM_ATTACK 6

/* Lingo integer(): round to nearest, halves away from zero. */
static int lingoInteger(double v) {
    return (int) (v < 0.0 ? -floor(-v + 0.5) : floor(v + 0.5));
}

/* energyStatus(): the battery's charge rounded to a whole number (0-100). */
static int energyStatusFromDeci(int deci) {
    if (deci <= 0) {
        return 0;
    }
    return (deci + 5) / 10;
}

static int pileBatteryCount(const WBCell* c) {
    return c->pile.energy < WB_MAX_PILE_BATTERIES ? c->pile.energy : WB_MAX_PILE_BATTERIES;
}

/* The pile's last battery: what its sprite shows and what is taken first. */
static int pileLastBattery(const WBCell* c) {
    int n = pileBatteryCount(c);
    return n > 0 ? c->pileBatteries[n - 1] : -1;
}


/* What a unit's getMember() picks this frame. moveIndex is pMoveIndex (1 at the start
   of a step, 0 once it has landed); sliding says pMovePixelTime is set. */
typedef struct WBUnitPose {
    WBUnitType type;
    WBDirection dir;
    bool water;         /* the ".water" member set (waterversion units) */
    bool tileWater;     /* the tile it stands on is water (the frog's hop rule) */
    bool sliding;
    float moveIndex;
    bool full;          /* steamshovel with dirt/swamp, treebot with a tree: ".full" */
    u8 action;          /* WB_UNIT_ANIM_*: the running pActionCycle, freeze or repair */
    int actionFrame;    /* 1-based frame of that action */
} WBUnitPose;

static WBUnitPose unitStandPose(WBUnitType type, WBDirection dir, bool water) {
    WBUnitPose p;
    memset(&p, 0, sizeof(p));
    p.type = type;
    p.dir = dir;
    p.water = water;
    p.tileWater = water;
    return p;
}

#define WB_DIR4(d, U, D, L, R) ((d) == WB_DIR_UP ? (U) : (d) == WB_DIR_DOWN ? (D) : (d) == WB_DIR_LEFT ? (L) : (R))

/* Walk frame of the subclasses that count from 1.2: integer((1.2 - pMoveIndex) * n), 1..n. */
static int walkFrameFrom12(float moveIndex, int n) {
    int f = lingoInteger((1.2 - (double) moveIndex) * n);
    if (f < 1) f = 1;
    if (f > n) f = n;
    return f;
}

static int unitPoseAssetId(const WBUnitPose* p) {
    WBDirection d = p->dir;
    switch (p->type) {
        case WB_UNIT_BUGGY:
            return WB_DIR4(d, ASSET_VEHICLE_BUGGY_UP, ASSET_VEHICLE_BUGGY_DOWN, ASSET_VEHICLE_BUGGY_LEFT, ASSET_VEHICLE_BUGGY_RIGHT);
        case WB_UNIT_DUCK:
            if (p->water) {
                return WB_DIR4(d, ASSET_VEHICLE_DUCK_WATER_UP, ASSET_VEHICLE_DUCK_WATER_DOWN, ASSET_VEHICLE_DUCK_WATER_LEFT, ASSET_VEHICLE_DUCK_WATER_RIGHT);
            }
            return WB_DIR4(d, ASSET_VEHICLE_DUCK_UP, ASSET_VEHICLE_DUCK_DOWN, ASSET_VEHICLE_DUCK_LEFT, ASSET_VEHICLE_DUCK_RIGHT);
        case WB_UNIT_DIRTBUGGY:
            return WB_DIR4(d, ASSET_VEHICLE_DIRTBUGGY_UP, ASSET_VEHICLE_DIRTBUGGY_DOWN, ASSET_VEHICLE_DIRTBUGGY_LEFT, ASSET_VEHICLE_DIRTBUGGY_RIGHT);
        case WB_UNIT_STEAMSHOVEL:
            /* ".dig" for 200 ms after digging or filling */
            if (p->action == WB_UNIT_ANIM_DIG || p->action == WB_UNIT_ANIM_FILL) {
                return WB_DIR4(d, ASSET_VEHICLE_STEAMSHOVEL_UP_DIG, ASSET_VEHICLE_STEAMSHOVEL_DOWN_DIG, ASSET_VEHICLE_STEAMSHOVEL_LEFT_DIG, ASSET_VEHICLE_STEAMSHOVEL_RIGHT_DIG);
            }
            if (p->full) {
                return WB_DIR4(d, ASSET_VEHICLE_STEAMSHOVEL_UP_FULL, ASSET_VEHICLE_STEAMSHOVEL_DOWN_FULL, ASSET_VEHICLE_STEAMSHOVEL_LEFT_FULL, ASSET_VEHICLE_STEAMSHOVEL_RIGHT_FULL);
            }
            return WB_DIR4(d, ASSET_VEHICLE_STEAMSHOVEL_UP, ASSET_VEHICLE_STEAMSHOVEL_DOWN, ASSET_VEHICLE_STEAMSHOVEL_LEFT, ASSET_VEHICLE_STEAMSHOVEL_RIGHT);
        case WB_UNIT_DUMPTRUCK:
            return WB_DIR4(d, ASSET_VEHICLE_DUMPTRUCK_UP, ASSET_VEHICLE_DUMPTRUCK_DOWN, ASSET_VEHICLE_DUMPTRUCK_LEFT, ASSET_VEHICLE_DUMPTRUCK_RIGHT);
        case WB_UNIT_FORKLIFT:
            return WB_DIR4(d, ASSET_VEHICLE_FORKLIFT_UP, ASSET_VEHICLE_FORKLIFT_DOWN, ASSET_VEHICLE_FORKLIFT_LEFT, ASSET_VEHICLE_FORKLIFT_RIGHT);
        case WB_UNIT_DOZER:
            return WB_DIR4(d, ASSET_VEHICLE_DOZER_UP, ASSET_VEHICLE_DOZER_DOWN, ASSET_VEHICLE_DOZER_LEFT, ASSET_VEHICLE_DOZER_RIGHT);
        case WB_UNIT_SPEEDBOAT:
            return WB_DIR4(d, ASSET_VEHICLE_SPEEDBOAT_UP, ASSET_VEHICLE_SPEEDBOAT_DOWN, ASSET_VEHICLE_SPEEDBOAT_LEFT, ASSET_VEHICLE_SPEEDBOAT_RIGHT);
        case WB_UNIT_TUGBOAT:
            return WB_DIR4(d, ASSET_VEHICLE_TUGBOAT_UP, ASSET_VEHICLE_TUGBOAT_DOWN, ASSET_VEHICLE_TUGBOAT_LEFT, ASSET_VEHICLE_TUGBOAT_RIGHT);
        case WB_UNIT_FREIGHTER:
            return WB_DIR4(d, ASSET_VEHICLE_FREIGHTER_UP, ASSET_VEHICLE_FREIGHTER_DOWN, ASSET_VEHICLE_FREIGHTER_LEFT, ASSET_VEHICLE_FREIGHTER_RIGHT);
        case WB_UNIT_FROG: {
            /* vehicle.frog: four quarter-steps; on water it hops on 2 and 4, on land on 1 and 3 */
            bool jump = false;
            if (p->sliding) {
                int f = lingoInteger((1.0 - (double) p->moveIndex) * 4.0);
                if (f < 1) f = 1;
                if (f > 4) f -= 4;
                jump = p->tileWater ? (f == 2 || f == 4) : (f == 1 || f == 3);
            }
            if (p->water) {
                return jump ? WB_DIR4(d, ASSET_VEHICLE_FROG_WATER_UP_JUMP, ASSET_VEHICLE_FROG_WATER_DOWN_JUMP, ASSET_VEHICLE_FROG_WATER_LEFT_JUMP, ASSET_VEHICLE_FROG_WATER_RIGHT_JUMP)
                            : WB_DIR4(d, ASSET_VEHICLE_FROG_WATER_UP, ASSET_VEHICLE_FROG_WATER_DOWN, ASSET_VEHICLE_FROG_WATER_LEFT, ASSET_VEHICLE_FROG_WATER_RIGHT);
            }
            return jump ? WB_DIR4(d, ASSET_VEHICLE_FROG_UP_JUMP, ASSET_VEHICLE_FROG_DOWN_JUMP, ASSET_VEHICLE_FROG_LEFT_JUMP, ASSET_VEHICLE_FROG_RIGHT_JUMP)
                        : WB_DIR4(d, ASSET_VEHICLE_FROG_UP, ASSET_VEHICLE_FROG_DOWN, ASSET_VEHICLE_FROG_LEFT, ASSET_VEHICLE_FROG_RIGHT);
        }
        case WB_UNIT_FISH:
            return WB_DIR4(d, ASSET_VEHICLE_FISH_WATER_UP, ASSET_VEHICLE_FISH_WATER_DOWN, ASSET_VEHICLE_FISH_WATER_LEFT, ASSET_VEHICLE_FISH_WATER_RIGHT);
        case WB_UNIT_SNAIL:
            return WB_DIR4(d, ASSET_VEHICLE_SNAIL_UP, ASSET_VEHICLE_SNAIL_DOWN, ASSET_VEHICLE_SNAIL_LEFT, ASSET_VEHICLE_SNAIL_RIGHT);
        case WB_UNIT_TREEBOT: {
            static const int upWalk[6]    = { ASSET_VEHICLE_TREEBOT_UP_WALK1,    ASSET_VEHICLE_TREEBOT_UP_WALK2,    ASSET_VEHICLE_TREEBOT_UP_WALK3,    ASSET_VEHICLE_TREEBOT_UP_WALK4,    ASSET_VEHICLE_TREEBOT_UP_WALK5,    ASSET_VEHICLE_TREEBOT_UP_WALK6    };
            static const int downWalk[6]  = { ASSET_VEHICLE_TREEBOT_DOWN_WALK1,  ASSET_VEHICLE_TREEBOT_DOWN_WALK2,  ASSET_VEHICLE_TREEBOT_DOWN_WALK3,  ASSET_VEHICLE_TREEBOT_DOWN_WALK4,  ASSET_VEHICLE_TREEBOT_DOWN_WALK5,  ASSET_VEHICLE_TREEBOT_DOWN_WALK6  };
            static const int leftWalk[6]  = { ASSET_VEHICLE_TREEBOT_LEFT_WALK1,  ASSET_VEHICLE_TREEBOT_LEFT_WALK2,  ASSET_VEHICLE_TREEBOT_LEFT_WALK3,  ASSET_VEHICLE_TREEBOT_LEFT_WALK4,  ASSET_VEHICLE_TREEBOT_LEFT_WALK5,  ASSET_VEHICLE_TREEBOT_LEFT_WALK6  };
            static const int rightWalk[6] = { ASSET_VEHICLE_TREEBOT_RIGHT_WALK1, ASSET_VEHICLE_TREEBOT_RIGHT_WALK2, ASSET_VEHICLE_TREEBOT_RIGHT_WALK3, ASSET_VEHICLE_TREEBOT_RIGHT_WALK4, ASSET_VEHICLE_TREEBOT_RIGHT_WALK5, ASSET_VEHICLE_TREEBOT_RIGHT_WALK6 };
            static const int upFullWalk[4]    = { ASSET_VEHICLE_TREEBOT_UP_FULL_WALK1,    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK2,    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK3,    ASSET_VEHICLE_TREEBOT_UP_FULL_WALK4 };
            static const int downFullWalk[4]  = { ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK1,  ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK2,  ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK3,  ASSET_VEHICLE_TREEBOT_DOWN_FULL_WALK4 };
            static const int leftFullWalk[4]  = { ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK1,  ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK2,  ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK3,  ASSET_VEHICLE_TREEBOT_LEFT_FULL_WALK4 };
            static const int rightFullWalk[4] = { ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK1, ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK2, ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK3, ASSET_VEHICLE_TREEBOT_RIGHT_FULL_WALK4 };
            if (p->sliding) {
                if (p->full) {
                    int f = walkFrameFrom12(p->moveIndex, 4) - 1;
                    return WB_DIR4(d, upFullWalk[f], downFullWalk[f], leftFullWalk[f], rightFullWalk[f]);
                } else {
                    int f = walkFrameFrom12(p->moveIndex, 6) - 1;
                    return WB_DIR4(d, upWalk[f], downWalk[f], leftWalk[f], rightWalk[f]);
                }
            }
            if (p->action == WB_UNIT_ANIM_UPROOT || p->action == WB_UNIT_ANIM_PLANT) {
                /* uproot shows lift.1 then lift.2; plant runs it backwards */
                int lift = p->action == WB_UNIT_ANIM_UPROOT ? p->actionFrame : 3 - p->actionFrame;
                if (p->full) {
                    return lift == 1 ? WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT1, ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT1, ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT1, ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT1)
                                     : WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP_FULL_LIFT2, ASSET_VEHICLE_TREEBOT_DOWN_FULL_LIFT2, ASSET_VEHICLE_TREEBOT_LEFT_FULL_LIFT2, ASSET_VEHICLE_TREEBOT_RIGHT_FULL_LIFT2);
                }
                return lift == 1 ? WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP_LIFT1, ASSET_VEHICLE_TREEBOT_DOWN_LIFT1, ASSET_VEHICLE_TREEBOT_LEFT_LIFT1, ASSET_VEHICLE_TREEBOT_RIGHT_LIFT1)
                                 : WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP_LIFT2, ASSET_VEHICLE_TREEBOT_DOWN_LIFT2, ASSET_VEHICLE_TREEBOT_LEFT_LIFT2, ASSET_VEHICLE_TREEBOT_RIGHT_LIFT2);
            }
            if (p->full) {
                return WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP_FULL, ASSET_VEHICLE_TREEBOT_DOWN_FULL, ASSET_VEHICLE_TREEBOT_LEFT_FULL, ASSET_VEHICLE_TREEBOT_RIGHT_FULL);
            }
            return WB_DIR4(d, ASSET_VEHICLE_TREEBOT_UP, ASSET_VEHICLE_TREEBOT_DOWN, ASSET_VEHICLE_TREEBOT_LEFT, ASSET_VEHICLE_TREEBOT_RIGHT);
        }
        case WB_UNIT_REPAIRBOT:
            if (p->sliding) {
                int f = walkFrameFrom12(p->moveIndex, 2);
                return f == 1 ? WB_DIR4(d, ASSET_VEHICLE_REPAIRBOT_UP_WALK1, ASSET_VEHICLE_REPAIRBOT_DOWN_WALK1, ASSET_VEHICLE_REPAIRBOT_LEFT_WALK1, ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK1)
                              : WB_DIR4(d, ASSET_VEHICLE_REPAIRBOT_UP_WALK2, ASSET_VEHICLE_REPAIRBOT_DOWN_WALK2, ASSET_VEHICLE_REPAIRBOT_LEFT_WALK2, ASSET_VEHICLE_REPAIRBOT_RIGHT_WALK2);
            }
            if (p->action == WB_UNIT_ANIM_REPAIR) {
                return p->actionFrame == 1 ? WB_DIR4(d, ASSET_VEHICLE_REPAIRBOT_UP_REPAIR1, ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR1, ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR1, ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR1)
                                           : WB_DIR4(d, ASSET_VEHICLE_REPAIRBOT_UP_REPAIR2, ASSET_VEHICLE_REPAIRBOT_DOWN_REPAIR2, ASSET_VEHICLE_REPAIRBOT_LEFT_REPAIR2, ASSET_VEHICLE_REPAIRBOT_RIGHT_REPAIR2);
            }
            return WB_DIR4(d, ASSET_VEHICLE_REPAIRBOT_UP, ASSET_VEHICLE_REPAIRBOT_DOWN, ASSET_VEHICLE_REPAIRBOT_LEFT, ASSET_VEHICLE_REPAIRBOT_RIGHT);
        case WB_UNIT_DEFENDER:
            /* vehicle.defender: frameNum = integer((1 - pMoveIndex) * 4), 1..4 (the attack
               frames it shows standing have no sprites in the port's sheets) */
            if (p->sliding) {
                int f = lingoInteger((1.0 - (double) p->moveIndex) * 4.0);
                if (f < 1) f = 1;
                if (f > 4) f -= 4;
                switch (d) {
                    case WB_DIR_UP:   return ASSET_VEHICLE_DEFENDER_UP_WALK1 + (f - 1);
                    case WB_DIR_DOWN: return ASSET_VEHICLE_DEFENDER_DOWN_WALK1 + (f - 1);
                    case WB_DIR_LEFT: return ASSET_VEHICLE_DEFENDER_LEFT_WALK1 + (f - 1);
                    default:          return ASSET_VEHICLE_DEFENDER_RIGHT_WALK1 + (f - 1);
                }
            }
            return WB_DIR4(d, ASSET_VEHICLE_DEFENDER_UP, ASSET_VEHICLE_DEFENDER_DOWN, ASSET_VEHICLE_DEFENDER_LEFT, ASSET_VEHICLE_DEFENDER_RIGHT);
        case WB_UNIT_DEFENDER2:
            /* vehicle.defender2 has no getMember of its own: no walk cycle */
            return WB_DIR4(d, ASSET_VEHICLE_DEFENDER_UP, ASSET_VEHICLE_DEFENDER_DOWN, ASSET_VEHICLE_DEFENDER_LEFT, ASSET_VEHICLE_DEFENDER_RIGHT);
        case WB_UNIT_FREEZEBOT:
            /* attack.1/2 for 600 ms after freezing, else walk 1..4 (from 1.2), else stand */
            if (p->action == WB_UNIT_ANIM_ATTACK) {
                return p->actionFrame == 1 ? WB_DIR4(d, ASSET_VEHICLE_FREEZEBOT_UP_ATK1, ASSET_VEHICLE_FREEZEBOT_DOWN_ATK1, ASSET_VEHICLE_FREEZEBOT_LEFT_ATK1, ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK1)
                                           : WB_DIR4(d, ASSET_VEHICLE_FREEZEBOT_UP_ATK2, ASSET_VEHICLE_FREEZEBOT_DOWN_ATK2, ASSET_VEHICLE_FREEZEBOT_LEFT_ATK2, ASSET_VEHICLE_FREEZEBOT_RIGHT_ATK2);
            }
            if (p->sliding) {
                int f = walkFrameFrom12(p->moveIndex, 4);
                switch (d) {
                    case WB_DIR_UP:   return ASSET_VEHICLE_FREEZEBOT_UP_WALK1 + (f - 1);
                    case WB_DIR_DOWN: return ASSET_VEHICLE_FREEZEBOT_DOWN_WALK1 + (f - 1);
                    case WB_DIR_LEFT: return ASSET_VEHICLE_FREEZEBOT_LEFT_WALK1 + (f - 1);
                    default:          return ASSET_VEHICLE_FREEZEBOT_RIGHT_WALK1 + (f - 1);
                }
            }
            return WB_DIR4(d, ASSET_VEHICLE_FREEZEBOT_UP, ASSET_VEHICLE_FREEZEBOT_DOWN, ASSET_VEHICLE_FREEZEBOT_LEFT, ASSET_VEHICLE_FREEZEBOT_RIGHT);
        default:
            return unitAssetId(p->type);
    }
}

static int unitStandAssetId(WBUnitType type, WBDirection dir, bool water) {
    WBUnitPose p = unitStandPose(type, dir, water);
    return unitPoseAssetId(&p);
}

static int heroAssetIdForUnit(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:       return ASSET_VEHICLE_BUGGY_HERO;
        case WB_UNIT_DUCK:        return ASSET_VEHICLE_DUCK_HERO;
        case WB_UNIT_DIRTBUGGY:   return ASSET_VEHICLE_DIRTBUGGY_HERO;
        case WB_UNIT_STEAMSHOVEL: return ASSET_VEHICLE_STEAMSHOVEL_HERO;
        case WB_UNIT_DUMPTRUCK:   return ASSET_VEHICLE_DUMPTRUCK_HERO;
        case WB_UNIT_FORKLIFT:    return ASSET_VEHICLE_FORKLIFT_HERO;
        case WB_UNIT_DOZER:       return ASSET_VEHICLE_DOZER_HERO;
        case WB_UNIT_SPEEDBOAT:   return ASSET_VEHICLE_SPEEDBOAT_HERO;
        case WB_UNIT_TUGBOAT:     return ASSET_VEHICLE_TUGBOAT_HERO;
        case WB_UNIT_FREIGHTER:   return ASSET_VEHICLE_FREIGHTER_HERO;
        case WB_UNIT_FROG:        return ASSET_VEHICLE_FROG_HERO;
        case WB_UNIT_FISH:        return ASSET_VEHICLE_FISH_HERO;
        case WB_UNIT_SNAIL:       return ASSET_VEHICLE_SNAIL_HERO;
        case WB_UNIT_TREEBOT:     return ASSET_VEHICLE_TREEBOT_HERO;
        case WB_UNIT_DEFENDER:    return ASSET_VEHICLE_DEFENDER_HERO;
        case WB_UNIT_DEFENDER2:   return ASSET_VEHICLE_DEFENDER_HERO;
        case WB_UNIT_REPAIRBOT:   return ASSET_VEHICLE_REPAIRBOT_HERO;
        case WB_UNIT_FREEZEBOT:   return ASSET_VEHICLE_FREEZEBOT_HERO;
        default: return -1;
    }
}

static const char* unitDescription(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:
            /* WB1/WB2 guide verbatim */
            return "A basic land vehicle that can carry up to 3 bricks.\nTerrain: Normal\nSpeed: Fast\nActions: Pick Up,\nDrop Off\nCapacity: 3 Bricks";
        case WB_UNIT_DUCK:
            /* WB1/WB2 guide verbatim */
            return "A yellow creature that's amphibious -- it can travel over land and water!\nTerrain: Normal, Water\nSpeed: Fast\nActions: None";
        case WB_UNIT_DIRTBUGGY:
            /* WB1/WB2 guide verbatim */
            return "Extra-big tires get the Dirtbuggy over rough terrain. Carries 3 bricks.\nTerrain: Normal, Rocky\nSpeed: Fast\nActions: Pick Up,\nDrop Off\nCapacity: 3 Bricks";
        case WB_UNIT_STEAMSHOVEL:
            /* WB1/WB2 guide verbatim */
            return "Use the steamshovel to dig up ground, turning it into water. You can also fill water, turning it into ground.\nTerrain: Normal\nSpeed: Medium\nActions: Dig, Fill";
        case WB_UNIT_DUMPTRUCK:
            /* WB1/WB2 guide verbatim */
            return "The Dumptruck is slow and steady and can carry a heavy load.\nTerrain: Normal, Rocky\nSpeed: Slow\nActions: Pick Up,\nDrop Off\nCapacity: 25 Bricks";
        case WB_UNIT_FORKLIFT:
            /* WB1/WB2 guide verbatim */
            return "The forklift moves a little slow, but it can carry a whopping 10 bricks.\nTerrain: Normal\nSpeed: Medium\nActions: Pick Up,\nDrop Off\nCapacity: 10 Bricks";
        case WB_UNIT_DOZER:
            /* WB1/WB2 guide, adapted for 3DS controls */
            return "The Dozer can push boulders and piles of bricks. Move it next to the target and press A to push.\nTerrain: Normal\nSpeed: Medium\nActions: Push";
        case WB_UNIT_SPEEDBOAT:
            /* WB1/WB2 guide verbatim */
            return "Fast and furious, the speedboat has turbo-charged maneuverability and packs a nice defensive wallop.\nTerrain: Water\nSpeed: Fast\nActions: Attack";
        case WB_UNIT_TUGBOAT:
            /* WB1/WB2 guide verbatim */
            return "This sturdy vessel is guaranteed not to sink.\nTerrain: Water\nSpeed: Medium\nActions: Pick Up,\nDrop Off\nCapacity: 5 Bricks";
        case WB_UNIT_FREIGHTER:
            /* Derived from WB1 game context; not in guide */
            return "This massive ship can haul a gigantic load -- more than any other vessel. Not the fastest, but nothing beats it for big deliveries.\nTerrain: Water\nSpeed: Slow\nActions: Pick Up,\nDrop Off\nCapacity: 25 Bricks";
        case WB_UNIT_FROG:
            /* WB1/WB2 guide verbatim */
            return "Frogs are small, green, and entirely made out of LEGO bricks.\nTerrain: Normal,\nShallow Water\nSpeed: Fast\nActions: None";
        case WB_UNIT_FISH:
            /* WB1/WB2 guide verbatim */
            return "This slippery critter lives in the water, which is not unusual - for a fish.\nTerrain: Water\nSpeed: Fast\nActions: None";
        case WB_UNIT_SNAIL:
            /* WB1/WB2 guide verbatim */
            return "You'd be slow too if you had to carry your entire house on your back.\nTerrain: Normal\nSpeed: Very Slow\nActions: None";
        case WB_UNIT_TREEBOT:
            /* WB1/WB2 guide verbatim */
            return "It may take a lot of bricks, but this mighty mech can pull up trees and replant them in the ground.\nTerrain: Normal\nSpeed: Medium\nActions: Uproot Tree,\nPlant Tree";
        case WB_UNIT_REPAIRBOT:
            /* WB1/WB2 guide verbatim */
            return "This handy little mechanic loves to fix broken machinery.\nTerrain: Normal\nSpeed: Fast\nActions: Auto-recharges\nnearby robots";
        case WB_UNIT_DEFENDER:
            /* WB1/WB2 guide verbatim */
            return "Use the Defender to disassemble pesky monsters.\nTerrain: Normal\nSpeed: Medium\nActions: Attack";
        case WB_UNIT_DEFENDER2:
            /* Derived from WB2 context; not in guide */
            return "An upgraded Defender built for the prehistoric frontier. Bigger treads let it hunt down pesky monsters on rocky ground.\nTerrain: Normal, Rocky\nSpeed: Medium\nActions: Attack";
        case WB_UNIT_FREEZEBOT:
            /* WB2 guide verbatim */
            return "He's not the toughest robot, so he just freezes any enemy that gets within two paces! His freeze ray has a short recharge time...\nTerrain: Normal\nSpeed: Fast\nActions: Auto-freezes\nnearby enemies";
        case WB_UNIT_NONE:
            return "No description available.";
        default:
            return "No description available.";
    }
}

static int ordinaryResourceVariant(int amount, int asset1, int asset2, int asset3, int asset4) {
    if (amount >= 13) return asset4;
    if (amount >= 6) return asset3;
    if (amount >= 2) return asset2;
    return asset1;
}

static int pileEntryAssetId(const WBResourcePile* pile, int kind) {
    switch (kind) {
        case 0: return ordinaryResourceVariant(pile->red, ASSET_RESOURCE_RED_1, ASSET_RESOURCE_RED_2, ASSET_RESOURCE_RED_3, ASSET_RESOURCE_RED_4);
        case 1: return ordinaryResourceVariant(pile->blue, ASSET_RESOURCE_BLUE_1, ASSET_RESOURCE_BLUE_2, ASSET_RESOURCE_BLUE_3, ASSET_RESOURCE_BLUE_4);
        case 2: return ordinaryResourceVariant(pile->green, ASSET_RESOURCE_GREEN_1, ASSET_RESOURCE_GREEN_2, ASSET_RESOURCE_GREEN_3, ASSET_RESOURCE_GREEN_4);
        case 3: return ordinaryResourceVariant(pile->yellow, ASSET_RESOURCE_YELLOW_1, ASSET_RESOURCE_YELLOW_2, ASSET_RESOURCE_YELLOW_3, ASSET_RESOURCE_YELLOW_4);
        case 4: return ASSET_RESOURCE_WHEEL_FULL;
        case 5:
            return pile->energy > 0 ? ASSET_RESOURCE_ENERGY_FULL : -1;
        case 6:
            return ordinaryResourceVariant(pile->white, ASSET_RESOURCE_WHITE_1, ASSET_RESOURCE_WHITE_2, ASSET_RESOURCE_WHITE_3, ASSET_RESOURCE_WHITE_4);
        default:
            return -1;
    }
}

static int energyAssetForValue(int energyValue) {
    if (energyValue >= 80) {
        return ASSET_RESOURCE_ENERGY_FULL;
    }
    if (energyValue > 1) {
        return ASSET_RESOURCE_ENERGY_LOW;
    }
    return ASSET_RESOURCE_ENERGY_DEAD;
}

static void spawnWorldEffect(AppState* app, WBWorldEffectKind kind, int tileX, int tileY); /* forward decl */
static void cleanupResourceCell(WBCell* cell); /* forward decl */
static void drawUnitEnergyBadge(AppState* app, int energyDeci, float unitX, float unitY) {
    /* Charging animation: cycle ASSET_UI_ENERGY_ICON at ~230ms per frame.
       Only possible for stationary units (we don't have position in moving path here),
       so callers that know position pass cellX/cellY separately. */
    if (energyDeci <= 0) {
        /* Dead battery — show no_energy icon */
        int assetId = ASSET_UI_NO_ENERGY;
        drawAnchoredImage(getImage(app, assetId),
            unitX + (12.0f * DIRECTOR_SCALE),
            unitY + (10.0f * DIRECTOR_SCALE),
            DIRECTOR_SCALE,
            g_objectAnchors[assetId].x,
            g_objectAnchors[assetId].y);
    } else if (energyDeci <= 200) {
        /* Low battery — use proper icon.low_energy sprite when available. */
        C2D_Image lowImg = getWorldbuilderExtraImage(app, WB_EXTRA_UI_LOW_ENERGY_IDX);
        if (lowImg.subtex) {
            drawAnchoredImage(lowImg,
                unitX + (12.0f * DIRECTOR_SCALE),
                unitY + (10.0f * DIRECTOR_SCALE),
                DIRECTOR_SCALE,
                lowImg.subtex->width * 0.5f,
                lowImg.subtex->height * 0.5f);
        } else {
            int assetId = ASSET_RESOURCE_ENERGY_LOW;
            drawAnchoredImage(getImage(app, assetId),
                unitX + (12.0f * DIRECTOR_SCALE),
                unitY + (10.0f * DIRECTOR_SCALE),
                DIRECTOR_SCALE,
                g_objectAnchors[assetId].x,
                g_objectAnchors[assetId].y);
        }
    }
}

static int pileEntryAmount(const WBResourcePile* pile, int kind) {
    switch (kind) {
        case 0: return pile->red;
        case 1: return pile->blue;
        case 2: return pile->green;
        case 3: return pile->yellow;
        case 4: return pile->wheel;
        case 5: return pile->energy;
        default: return 0;
    }
}

static void sortPileKinds(const WBResourcePile* pile, int* kinds, int* count) {
    int n = 0;
    int i;
    int j;
    if (pile->red > 0) kinds[n++] = 0;
    if (pile->blue > 0) kinds[n++] = 1;
    if (pile->green > 0) kinds[n++] = 2;
    if (pile->yellow > 0) kinds[n++] = 3;
    if (pile->wheel > 0) kinds[n++] = 4;
    if (pile->energy > 0) kinds[n++] = 5;
    if (pile->white > 0) kinds[n++] = 6;
    for (i = 0; i < n; ++i) {
        for (j = i + 1; j < n; ++j) {
            if (pileEntryAmount(pile, kinds[j]) > pileEntryAmount(pile, kinds[i])) {
                int tmp = kinds[i];
                kinds[i] = kinds[j];
                kinds[j] = tmp;
            }
        }
    }
    *count = n;
}

static int resourceTotal(const WBResourcePile* pile) {
    return pile->red + pile->blue + pile->green + pile->yellow + pile->wheel + pile->energy + pile->white;
}

static bool pileEmpty(const WBResourcePile* pile) {
    return resourceTotal(pile) == 0;
}

static bool unitSupportsPickDrop(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:
        case WB_UNIT_DIRTBUGGY:
        case WB_UNIT_DUMPTRUCK:
        case WB_UNIT_FORKLIFT:
        case WB_UNIT_TUGBOAT:
        case WB_UNIT_FREIGHTER:
            return true;
        default:
            return false;
    }
}

static int unitCarryCapacity(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:     return 3;
        case WB_UNIT_DIRTBUGGY: return 3;
        case WB_UNIT_DUMPTRUCK: return 25;
        case WB_UNIT_FORKLIFT:  return 10;
        case WB_UNIT_TUGBOAT:   return 5;
        case WB_UNIT_FREIGHTER: return 25;
        case WB_UNIT_TREEBOT:   return 1;
        default: return 0;
    }
}

static const char* actionModeName(WBActionMode mode) {
    switch (mode) {
        case WB_ACTION_PICK: return "pick";
        case WB_ACTION_DROP: return "drop";
        case WB_ACTION_DIG: return "dig";
        case WB_ACTION_FILL: return "fill";
        case WB_ACTION_UPROOT: return "uproot";
        case WB_ACTION_PLANT: return "plant";
        case WB_ACTION_PUSH: return "push";
        case WB_ACTION_ATTACK: return "attack";
        case WB_ACTION_MOVE:
        default: return "move";
    }
}

static int unitMoveCostDeci(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:      return 7;
        case WB_UNIT_DUCK:       return 4;
        case WB_UNIT_DIRTBUGGY:  return 7;
        case WB_UNIT_STEAMSHOVEL:return 12;
        case WB_UNIT_DUMPTRUCK:  return 12;
        case WB_UNIT_FORKLIFT:   return 10;
        case WB_UNIT_DOZER:      return 10;
        case WB_UNIT_SPEEDBOAT:  return 10;
        case WB_UNIT_TUGBOAT:    return 12;
        case WB_UNIT_FREIGHTER:  return 12;
        case WB_UNIT_FROG:       return 4;
        case WB_UNIT_FISH:       return 4;
        case WB_UNIT_SNAIL:      return 4;
        case WB_UNIT_TREEBOT:    return 20;
        case WB_UNIT_REPAIRBOT:  return 7;
        case WB_UNIT_DEFENDER:   return 10;
        case WB_UNIT_DEFENDER2:  return 10;
        case WB_UNIT_FREEZEBOT:  return 12;   /* WB2 #move:1.2 */
        default: return 0;
    }
}

static int manhattanDistance(int ax, int ay, int bx, int by) {
    int dx = ax - bx;
    int dy = ay - by;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

static int chooseLeastBrickKind(const WBResourcePile* pile) {
    int bestKind = -1;
    int bestAmount = 2147483647;
    int amounts[7] = { pile->red, pile->blue, pile->green, pile->yellow, pile->wheel, pile->energy, pile->white };
    int i;
    for (i = 0; i < 7; ++i) {
        if (amounts[i] > 0 && amounts[i] < bestAmount) {
            bestAmount = amounts[i];
            bestKind = i;
        }
    }
    return bestKind;
}

static void takeOneKind(WBResourcePile* pile, int kind) {
    switch (kind) {
        case 0: if (pile->red > 0) --pile->red; break;
        case 1: if (pile->blue > 0) --pile->blue; break;
        case 2: if (pile->green > 0) --pile->green; break;
        case 3: if (pile->yellow > 0) --pile->yellow; break;
        case 4: if (pile->wheel > 0) --pile->wheel; break;
        case 5: if (pile->energy > 0) --pile->energy; break;
        case 6: if (pile->white > 0) --pile->white; break;
    }
}

static void addOneKind(WBResourcePile* pile, int kind) {
    switch (kind) {
        case 0: ++pile->red; break;
        case 1: ++pile->blue; break;
        case 2: ++pile->green; break;
        case 3: ++pile->yellow; break;
        case 4: ++pile->wheel; break;
        case 5: ++pile->energy; break;
        case 6: ++pile->white; break;
    }
}

static void addPile(WBResourcePile* dst, const WBResourcePile* src) {
    dst->red += src->red;
    dst->blue += src->blue;
    dst->green += src->green;
    dst->yellow += src->yellow;
    dst->wheel += src->wheel;
    dst->energy += src->energy;
    dst->white += src->white;
}

static void spawnWorldEffect(AppState* app, WBWorldEffectKind kind, int tileX, int tileY) {
    int slot = -1;
    int i;
    u64 oldestMs = 0;

    for (i = 0; i < WB_MAX_WORLD_EFFECTS; ++i) {
        if (!app->worldEffects[i].active) {
            slot = i;
            break;
        }
        if (slot < 0 || app->worldEffects[i].startMs < oldestMs) {
            slot = i;
            oldestMs = app->worldEffects[i].startMs;
        }
    }

    if (slot < 0) {
        return;
    }

    app->worldEffects[slot].active = true;
    app->worldEffects[slot].kind = kind;
    app->worldEffects[slot].tileX = tileX;
    app->worldEffects[slot].tileY = tileY;
    app->worldEffects[slot].startMs = osGetTime();
    memset(app->worldEffects[slot].particleKinds, 0xFF, sizeof(app->worldEffects[slot].particleKinds));
}

static void setBuildEffectParticleKinds(WBWorldEffect* effect, const WBResourcePile* recipe) {
    WBResourcePile tmp;
    int total;
    int threshold;
    int kindIndex = 0;
    int i;

    if (!effect || !recipe) {
        return;
    }

    tmp = *recipe;
    total = tmp.red + tmp.blue + tmp.green + tmp.yellow + tmp.wheel + tmp.energy;
    if (total <= 0) {
        for (i = 0; i < 5; ++i) {
            effect->particleKinds[i] = 4;
        }
        return;
    }

    threshold = total / 5;
    if (threshold < 1) {
        threshold = 1;
    }

    for (i = 0; i < 5; ++i) {
        int kind;
        if (kindIndex > 5) {
            kindIndex = 5;
        }
        while (kindIndex < 6 && pileEntryAmount(&tmp, kindIndex) <= 0) {
            ++kindIndex;
        }
        if (kindIndex >= 6) {
            effect->particleKinds[i] = 4;
            continue;
        }
        kind = kindIndex;
        effect->particleKinds[i] = kind;
        {
            int use = threshold;
            int have = pileEntryAmount(&tmp, kind);
            if (use > have) {
                use = have;
            }
            while (use-- > 0) {
                takeOneKind(&tmp, kind);
            }
        }
    }
}

static void spawnBuildCloudEffect(AppState* app, int tileX, int tileY, const WBResourcePile* recipe) {
    int i;

    spawnWorldEffect(app, WB_WORLD_EFFECT_BUILD_CLOUD, tileX, tileY);
    for (i = 0; i < WB_MAX_WORLD_EFFECTS; ++i) {
        WBWorldEffect* effect = &app->worldEffects[i];
        if (!effect->active) {
            continue;
        }
        if (effect->kind != WB_WORLD_EFFECT_BUILD_CLOUD) {
            continue;
        }
        if (effect->tileX != tileX || effect->tileY != tileY) {
            continue;
        }
        if (effect->startMs + 1000 < osGetTime()) {
            continue;
        }
        setBuildEffectParticleKinds(effect, recipe);
        return;
    }
}

static void drawWorldEffectsBottom(AppState* app) {
    int i;
    u64 now = osGetTime();

    for (i = 0; i < WB_MAX_WORLD_EFFECTS; ++i) {
        WBWorldEffect* effect = &app->worldEffects[i];
        u64 elapsed;
        float locX;
        float locY;
        int assetId;
        C2D_Image image;

        if (!effect->active) {
            continue;
        }

        elapsed = now - effect->startMs;
        assetId = -1;

        if (effect->kind == WB_WORLD_EFFECT_DAMAGE_SMALL) {
            int frame = (int) (elapsed / (WB_DAMAGE_EFFECT_DURATION_MS / 5));
            C2D_ImageTint tint;
            u32 alpha = 0xFF;

            if (elapsed >= WB_DAMAGE_EFFECT_DURATION_MS) {
                effect->active = false;
                continue;
            }
            if (frame > 4) {
                frame = 4;
            }
            assetId = ASSET_DAMAGE_SMALL_1 + frame;
            posToLoc(app, effect->tileX + 1, effect->tileY + 1, &locX, &locY);
            image = getImage(app, assetId);
            if (!imageIntersectsScreen(image, locX, locY, DIRECTOR_SCALE,
                    g_objectAnchors[assetId].x, g_objectAnchors[assetId].y, BOTTOM_W, BOTTOM_H)) {
                continue;
            }
            alpha = (frame == 4) ? 0x80 : 0xE6;   /* blend 90, the last frame 50 */
            C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, alpha), 1.0f);
            C2D_DrawImageAt(image,
                locX - (g_objectAnchors[assetId].x * DIRECTOR_SCALE),
                locY - (g_objectAnchors[assetId].y * DIRECTOR_SCALE),
                0.0f, &tint, DIRECTOR_SCALE, DIRECTOR_SCALE);
            continue;
        }

        if (effect->kind == WB_WORLD_EFFECT_TAKE_APART_CLOUD) {
            static const int cloudFrames[4] = {
                ASSET_TAKE_APART_CLOUD_1,
                ASSET_TAKE_APART_CLOUD_2,
                ASSET_TAKE_APART_CLOUD_3,
                ASSET_TAKE_APART_CLOUD_3
            };
            int frame = (int) (elapsed / WB_CLOUD_EFFECT_FRAME_MS);

            if (frame >= 4) {
                effect->active = false;
                continue;
            }
            assetId = cloudFrames[frame];
            posToLoc(app, effect->tileX + 1, effect->tileY + 1, &locX, &locY);
            image = getImage(app, assetId);
            if (!imageIntersectsScreen(image, locX, locY, DIRECTOR_SCALE,
                    g_objectAnchors[assetId].x, g_objectAnchors[assetId].y, BOTTOM_W, BOTTOM_H)) {
                continue;
            }
            drawAnchoredImage(image, locX, locY, DIRECTOR_SCALE,
                g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
            continue;
        }

        if (effect->kind == WB_WORLD_EFFECT_BUILD_CLOUD) {
            float t = (float) elapsed / 750.0f;
            int cloudAsset;

            if (t > 1.0f) {
                effect->active = false;
                continue;
            }

            posToLoc(app, effect->tileX + 1, effect->tileY + 1, &locX, &locY);
            cloudAsset = ((osGetTime() / 200) % 2) ? ASSET_BUILD_CLOUD_2 : ASSET_BUILD_CLOUD_1;
            drawAnchoredImage(getImage(app, cloudAsset), locX, locY, DIRECTOR_SCALE,
                g_objectAnchors[cloudAsset].x, g_objectAnchors[cloudAsset].y);

            for (int p = 0; p < 5; ++p) {
                int kind = effect->particleKinds[p];
                int asset;
                float theta;
                float phi;
                float a;
                float x0;
                float y0;
                float x1;
                float y1;
                u8 alpha;
                C2D_ImageTint tint;

                if (kind < 0 || kind > 5) {
                    continue;
                }
                if (kind == 5) {
                    asset = ASSET_RESOURCE_ENERGY_FULL;
                } else {
                    asset = (kind == 0) ? ASSET_RESOURCE_RED_2 :
                            (kind == 1) ? ASSET_RESOURCE_BLUE_2 :
                            (kind == 2) ? ASSET_RESOURCE_GREEN_2 :
                            (kind == 3) ? ASSET_RESOURCE_YELLOW_2 :
                            ASSET_RESOURCE_WHEEL_FULL;
                }

                theta = 2.0f * 3.14159265f * ((6.0f * t) + ((float) (p + 1) / 3.0f));
                phi = 2.0f * 3.14159265f * ((0.75f * t) + ((float) (p + 1) / 5.0f));
                a = 1.0f - (t / 1.1f);
                x0 = a * cosf(theta);
                y0 = a * sinf(theta) / 4.0f;
                x1 = (x0 * cosf(phi)) + (y0 * sinf(phi));
                y1 = (x0 * sinf(phi)) - (y0 * cosf(phi));
                alpha = (u8) (75 + (25 * cosf(phi)));
                C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, alpha), 1.0f);
                C2D_DrawImageAt(getImage(app, asset),
                    (locX + (10.0f * DIRECTOR_SCALE) + (40.0f * x1)) - (g_objectAnchors[asset].x * DIRECTOR_SCALE),
                    (locY + (18.0f * DIRECTOR_SCALE) + (40.0f * y1)) - (g_objectAnchors[asset].y * DIRECTOR_SCALE),
                    0.0f, &tint, DIRECTOR_SCALE, DIRECTOR_SCALE);
            }
        }
    }
}

static const WBCell* selectedCell(const AppState* app) {
    if (!app->hasSelection || !inBounds(app, app->selectedX, app->selectedY)) {
        return NULL;
    }
    return &app->map.cells[app->selectedY][app->selectedX];
}

static bool getSelectedUnitView(const AppState* app, WBSelectedUnitView* outView) {
    const WBCell* cell = selectedCell(app);
    memset(outView, 0, sizeof(*outView));
    if (!cell || !cell->hasUnit) {
        return false;
    }
    outView->valid = true;
    outView->unitType = cell->unitType;
    outView->terrain = cell->terrain;
    outView->energy = cell->unitEnergy;
    outView->energyDeci = cell->unitEnergyDeci;
    outView->cargo = cell->unitCargo;
    outView->direction = cell->unitDirection;
    return true;
}

static void sceneInit(AppState* app) {
    app->staticBuf = C2D_TextBufNew(1024);
    app->dynamicBuf = C2D_TextBufNew(4096);
    C2D_TextParse(&app->titleText, app->staticBuf, "LEGO World Builder 3DS Prototype");
    C2D_TextParse(&app->screenTitleText, app->staticBuf, "LEGO World Builder");
    C2D_TextOptimize(&app->titleText);
    C2D_TextOptimize(&app->screenTitleText);
    app->musicBpm = 151;
    app->screenMode = WB_SCREEN_TITLE;
    app->musicUseGamePlaylist = false;
    app->musicLastGameSongIndex = -1;
    app->worldSelectWorld = 1;
    app->activeWorld = 1;
    app->activeMission = 1;
    parseWorldMetadata(app);
    loadLevelProgress(app);
}

/* Forward declaration: defined later alongside the other per-level audio helpers */
static void freeLevelAudio(AppState* app);

static void sceneExit(AppState* app) {
    int i;
    C2D_TextBufDelete(app->dynamicBuf);
    C2D_TextBufDelete(app->staticBuf);
    if (app->spriteSheet) {
        C2D_SpriteSheetFree(app->spriteSheet);
    }
    if (app->titleMainSheet) C2D_SpriteSheetFree(app->titleMainSheet);
    for (int i = 0; i < 3; ++i) {
        if (app->worldSkySheets[i]) C2D_SpriteSheetFree(app->worldSkySheets[i]);
        if (app->worldMapSheets[i]) C2D_SpriteSheetFree(app->worldMapSheets[i]);
    }
    if (app->worldIconSheet) C2D_SpriteSheetFree(app->worldIconSheet);
    free(app->walkMask);
    app->walkMask = NULL;
    if (app->licenseSheet)   C2D_SpriteSheetFree(app->licenseSheet);
    if (app->vehiclesLandSheet)   C2D_SpriteSheetFree(app->vehiclesLandSheet);
    if (app->vehiclesWaterSheet)  C2D_SpriteSheetFree(app->vehiclesWaterSheet);
    if (app->vehiclesAnimalSheet) C2D_SpriteSheetFree(app->vehiclesAnimalSheet);
    if (app->vehiclesRobotSheet)  C2D_SpriteSheetFree(app->vehiclesRobotSheet);
    if (app->monstersASheet)      C2D_SpriteSheetFree(app->monstersASheet);
    if (app->monstersBSheet)      C2D_SpriteSheetFree(app->monstersBSheet);
    if (app->buildingsSheet)      C2D_SpriteSheetFree(app->buildingsSheet);
    if (app->whirlpoolSheet)      C2D_SpriteSheetFree(app->whirlpoolSheet);
    freeWavClip(&app->musicIntro);
    freeWavClip(&app->musicGame);
    for (i = 0; i < WB_MUSIC_INTRO_VARIANTS; ++i) {
        freeWavClip(&app->musicIntroVariants[i]);
    }
    /* Per-level SFX + game music variants (idempotent: clips may already be freed) */
    freeLevelAudio(app);
    /* Global SFX */
    freeWavClip(&app->sfxButton);
    freeWavClip(&app->sfxRollover);
    freeWavClip(&app->sfxWorldComingSoon);
    freeWavClip(&app->sfxPlan);
    freeWavClip(&app->sfxPickupPlan);
    freeWavClip(&app->sfxMove);
    freeWavClip(&app->sfxPickup);
    freeWavClip(&app->sfxDrop);
    freeWavClip(&app->sfxDamage);
    freeWavClip(&app->sfxDisassemble);
    freeWavClip(&app->sfxGoal);
    freeWavClip(&app->sfxBonusGoal);
    freeWavClip(&app->sfxAssembly);
    freeWavClip(&app->sfxMoveMisc);
}

static float unitMoveSpeed(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:      return 3.0f;
        case WB_UNIT_DUCK:       return 2.0f;
        case WB_UNIT_DIRTBUGGY:  return 3.0f;
        case WB_UNIT_STEAMSHOVEL:return 2.0f;
        case WB_UNIT_DUMPTRUCK:  return 1.0f;
        case WB_UNIT_FORKLIFT:   return 2.0f;
        case WB_UNIT_DOZER:      return 2.0f;
        case WB_UNIT_SPEEDBOAT:  return 3.0f;
        case WB_UNIT_TUGBOAT:    return 3.0f;
        case WB_UNIT_FREIGHTER:  return 2.0f;
        case WB_UNIT_FROG:       return 2.0f;
        case WB_UNIT_FISH:       return 2.0f;
        case WB_UNIT_SNAIL:      return 0.5f;
        case WB_UNIT_TREEBOT:    return 2.3f;
        case WB_UNIT_REPAIRBOT:  return 3.0f;
        case WB_UNIT_DEFENDER:   return 2.0f;
        case WB_UNIT_DEFENDER2:  return 2.0f;
        case WB_UNIT_FREEZEBOT:  return 2.5f;  /* WB2 #speed:2.5 */
        default: return 2.0f;
    }
}

static bool goalMatchesUnit(WBGoalType goalType, WBUnitType unitType) {
    switch (goalType) {
        case WB_GOAL_ANYTHING:   return unitType != WB_UNIT_NONE;
        case WB_GOAL_BUGGY:      return unitType == WB_UNIT_BUGGY;
        case WB_GOAL_DUCK:       return unitType == WB_UNIT_DUCK;
        case WB_GOAL_DIRTBUGGY:  return unitType == WB_UNIT_DIRTBUGGY;
        case WB_GOAL_STEAMSHOVEL:return unitType == WB_UNIT_STEAMSHOVEL;
        case WB_GOAL_DUMPTRUCK:  return unitType == WB_UNIT_DUMPTRUCK;
        case WB_GOAL_FORKLIFT:   return unitType == WB_UNIT_FORKLIFT;
        case WB_GOAL_DOZER:      return unitType == WB_UNIT_DOZER;
        case WB_GOAL_SPEEDBOAT:  return unitType == WB_UNIT_SPEEDBOAT;
        case WB_GOAL_TUGBOAT:    return unitType == WB_UNIT_TUGBOAT;
        case WB_GOAL_FREIGHTER:  return unitType == WB_UNIT_FREIGHTER;
        case WB_GOAL_FROG:       return unitType == WB_UNIT_FROG;
        case WB_GOAL_FISH:       return unitType == WB_UNIT_FISH;
        case WB_GOAL_SNAIL:      return unitType == WB_UNIT_SNAIL;
        case WB_GOAL_TREEBOT:    return unitType == WB_UNIT_TREEBOT;
        case WB_GOAL_REPAIRBOT:  return unitType == WB_UNIT_REPAIRBOT;
        case WB_GOAL_DEFENDER:   return unitType == WB_UNIT_DEFENDER;
        case WB_GOAL_FREEZEBOT:  return unitType == WB_UNIT_FREEZEBOT;
        default: return false;
    }
}

static bool goalMatchesBuilding(WBGoalType goalType, WBBuildingType buildingType) {
    switch (goalType) {
        case WB_GOAL_GAS_STATION: return buildingType == WB_BUILDING_GAS_STATION;
        case WB_GOAL_MARINA:      return buildingType == WB_BUILDING_MARINA;
        case WB_GOAL_ROBOT_LAB:   return buildingType == WB_BUILDING_ROBOT_LAB;
        case WB_GOAL_GUARD_TOWER: return buildingType == WB_BUILDING_GUARD_TOWER;
        case WB_GOAL_AIRPORT:     return buildingType == WB_BUILDING_AIRPORT;
        case WB_GOAL_HOUSE:       return buildingType == WB_BUILDING_HOUSE;
        case WB_GOAL_GARAGE:      return buildingType == WB_BUILDING_GARAGE;
        case WB_GOAL_FACTORY:     return buildingType == WB_BUILDING_FACTORY;
        case WB_GOAL_WINDMILL:    return buildingType == WB_BUILDING_WINDMILL;
        case WB_GOAL_NURSERY:     return buildingType == WB_BUILDING_NURSERY;
        default: return false;
    }
}

static bool goalMatchesMonster(WBGoalType goalType, WBMonsterType monsterType) {
    switch (goalType) {
        case WB_GOAL_LION: return monsterType == WB_MONSTER_LION;
        case WB_GOAL_CRAB: return monsterType == WB_MONSTER_CRAB;
        case WB_GOAL_GATOR: return monsterType == WB_MONSTER_GATOR;
        default: return false;
    }
}

static void satisfyGoalAt(AppState* app, WBCell* cell, int x, int y) {
    cell->goalSatisfied = true;
    if (cell->bonusGoal) {
        app->map.bonusScore += 1;
        applyMissionSuccess(app, app->activeWorld, app->activeMission, 3);
        playSfxClipPriority(app, &app->sfxBonusGoal);
        showGoalPopup(app, "Bonus Goal Complete", true, true, x, y);
    } else {
        app->map.goalScore += 1;
        app->map.bonusAvailable = true;
        applyMissionSuccess(app, app->activeWorld, app->activeMission, 1);
        playSfxClipPriority(app, &app->sfxGoal);
        showGoalPopup(app, "Goal Complete", false, true, x, y);
    }
}

static const char* goalRequiredUnitName(WBGoalType goalType) {
    switch (goalType) {
        case WB_GOAL_BUGGY: return "Buggy";
        case WB_GOAL_DUCK: return "Duck";
        case WB_GOAL_DIRTBUGGY: return "Dirtbuggy";
        case WB_GOAL_STEAMSHOVEL: return "Steamshovel";
        case WB_GOAL_DUMPTRUCK: return "Dumptruck";
        case WB_GOAL_FORKLIFT: return "Forklift";
        case WB_GOAL_DOZER: return "Bulldozer";
        case WB_GOAL_SPEEDBOAT: return "Speedboat";
        case WB_GOAL_TUGBOAT: return "Tugboat";
        case WB_GOAL_FREIGHTER: return "Freighter";
        case WB_GOAL_FROG: return "Frog";
        case WB_GOAL_FISH: return "Fish";
        case WB_GOAL_SNAIL: return "Snail";
        case WB_GOAL_TREEBOT: return "Treebot";
        case WB_GOAL_REPAIRBOT: return "Repairbot";
        case WB_GOAL_DEFENDER: return "Defender";
        case WB_GOAL_GAS_STATION: return "Gas Station";
        case WB_GOAL_MARINA: return "Marina";
        case WB_GOAL_ROBOT_LAB: return "Robot Lab";
        case WB_GOAL_GUARD_TOWER: return "Guard Tower";
        case WB_GOAL_AIRPORT: return "Airport";
        case WB_GOAL_HOUSE:   return "House";
        case WB_GOAL_GARAGE:  return "Garage";
        case WB_GOAL_FACTORY: return "Factory";
        case WB_GOAL_WINDMILL: return "Windmill";
        case WB_GOAL_NURSERY: return "Nursery";
        case WB_GOAL_GATOR:   return "Gator";
        case WB_GOAL_FREEZEBOT: return "Freezebot";
        case WB_GOAL_LION:      return "Lion";
        case WB_GOAL_CRAB:      return "Crab";
        case WB_GOAL_ANYTHING:
        default:
            return "Any unit";
    }
}

static bool findWhirlpoolPair(const AppState* app, int fromX, int fromY, int* outX, int* outY) {
    const WBCell* fromCell;
    int y;
    int x;
    if (!app || !inBounds(app, fromX, fromY) || !outX || !outY) {
        return false;
    }
    fromCell = &app->map.cells[fromY][fromX];
    if (!fromCell->isWhirlpool || fromCell->whirlpoolId == 0) {
        return false;
    }
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            const WBCell* c = &app->map.cells[y][x];
            if (x == fromX && y == fromY) {
                continue;
            }
            if (c->isWhirlpool && c->whirlpoolId == fromCell->whirlpoolId) {
                *outX = x;
                *outY = y;
                return true;
            }
        }
    }
    return false;
}

static bool tileDiamondHit(const AppState* app, int tx, int ty, float px, float py) {
    float cx;
    float cy;
    float dx;
    float dy;
    posToLoc(app, tx + 1, ty + 1, &cx, &cy);
    dx = fabsf(px - cx);
    dy = fabsf(py - cy - 6.0f);
    return (dx / 15.0f) + (dy / 8.0f) <= 1.0f;
}

static bool screenToTile(const AppState* app, float px, float py, int* outX, int* outY) {
    int y;
    int x;
    float bestScore = 1000000.0f;
    bool found = false;
    for (y = app->map.height - 1; y >= 0; --y) {
        for (x = app->map.width - 1; x >= 0; --x) {
            float cx;
            float cy;
            float score;
            posToLoc(app, x + 1, y + 1, &cx, &cy);
            score = (fabsf(px - cx) / 17.0f) + (fabsf(py - (cy + 6.0f)) / 10.0f);
            if (score < bestScore) {
                bestScore = score;
                *outX = x;
                *outY = y;
                found = true;
            }
        }
    }
    if (!found) {
        return false;
    }
    return bestScore <= 1.7f || tileDiamondHit(app, *outX, *outY, px, py);
}

/* Highlight arrow behavior: 4 px on the beat of the playing song (its 'rhythms' bpm),
   timed from when the song started and moved once a 15 fps frame. */
static void drawSelectionHighlight(AppState* app, float locX, float locY, bool planMode, int trackedAssetId) {
    double frameMs = 1000.0 / 15.0;
    double since = floor((double) (osGetTime() - app->musicStartMs) / frameMs) * frameMs;
    double bpm;
    float bob;
    int assetId = planMode ? ASSET_PLAN_HIGHLIGHT : ASSET_OBJECT_HIGHLIGHT;
    switch (app->musicBpm) {            /* the game playlist's family, or a menu bpm */
        case '8': bpm = 105.0; break;
        case '6': bpm = 140.0; break;
        case 'a': bpm = 120.0; break;
        case 'i': bpm = 151.0; break;
        default:  bpm = 120.0; break;
    }
    bob = (float) (4.0 * sin(2.0 * 3.14159265358979 * (since / 1000.0) * (bpm / 60.0)));
    if (!planMode && trackedAssetId >= 0) {
        locX -= 5.0f;
        locY -= imageHeight(getImage(app, trackedAssetId), DIRECTOR_SCALE) * 0.5f;
        locY -= 11.0f;
    }
    drawAnchoredImage(getImage(app, assetId), locX, locY + bob, DIRECTOR_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
}

static void drawSkyBottom(const AppState* app) {
    C2D_Image sky = getImage(app, worldSkyAssetId(app->activeWorld));
    float baseScale = 1.25f;
    float minScaleW;
    float minScaleH;
    float skyScale;
    float skyW;
    float skyH;
    float tX;
    float tY;
    float maxPanX;
    float maxPanY;
    float drawX;
    float drawY;
    int camRangeX = app->map.width - VIEW_TILE_W;
    int camRangeY = app->map.height - VIEW_TILE_H;

    if (!sky.subtex) {
        return;
    }

    minScaleW = (float) BOTTOM_W / sky.subtex->width;
    minScaleH = (float) BOTTOM_H / sky.subtex->height;
    skyScale = baseScale;
    if (skyScale < minScaleW) skyScale = minScaleW;
    if (skyScale < minScaleH) skyScale = minScaleH;
    skyW = imageWidth(sky, skyScale);
    skyH = imageHeight(sky, skyScale);

    if (camRangeX < 1) camRangeX = 1;
    if (camRangeY < 1) camRangeY = 1;
    tX = (float) (app->cameraX - 1) / (float) camRangeX;
    tY = (float) (app->cameraY - 1) / (float) camRangeY;
    if (tX < 0.0f) tX = 0.0f;
    if (tX > 1.0f) tX = 1.0f;
    if (tY < 0.0f) tY = 0.0f;
    if (tY > 1.0f) tY = 1.0f;

    maxPanX = skyW - (float) BOTTOM_W;
    maxPanY = skyH - (float) BOTTOM_H;
    if (maxPanX < 0.0f) maxPanX = 0.0f;
    if (maxPanY < 0.0f) maxPanY = 0.0f;
    drawX = -maxPanX * tX;
    drawY = -maxPanY * tY;

    C2D_DrawImageAt(sky, drawX, drawY, 0.0f, NULL, skyScale, skyScale);
}

static void startPlanSwoop(AppState* app, WBPlanType planType, float fromX, float fromY) {
    int assetId = planAssetId(planType);
    if (assetId < 0) {
        return;
    }
    app->planSwoop.active = true;
    app->planSwoop.assetId = assetId;
    app->planSwoop.fromX = fromX;
    app->planSwoop.fromY = fromY;
    app->planSwoop.toX = BOTTOM_W - 26.0f;
    app->planSwoop.toY = BOTTOM_H - 20.0f;
    app->planSwoop.startMs = osGetTime();
    app->planSwoop.durationMs = 600;
}

static void drawPlanSwoop(AppState* app) {
    u64 now;
    float r;
    float x;
    float y;
    float scale;
    C2D_Image img;
    if (!app->planSwoop.active) {
        return;
    }
    now = osGetTime();
    if (now <= app->planSwoop.startMs) {
        r = 0.0f;
    } else {
        r = (float) (now - app->planSwoop.startMs) / (float) app->planSwoop.durationMs;
    }
    if (r >= 1.0f) {
        app->planSwoop.active = false;
        return;
    }
    x = app->planSwoop.fromX + ((app->planSwoop.toX - app->planSwoop.fromX) * r);
    y = app->planSwoop.fromY + ((app->planSwoop.toY - app->planSwoop.fromY) * r);
    scale = 0.84f - (0.24f * r);
    img = getImage(app, app->planSwoop.assetId);
    drawAnchoredImage(img, x, y, scale, g_objectAnchors[app->planSwoop.assetId].x, g_objectAnchors[app->planSwoop.assetId].y);
}

static void drawTerrainBottom(AppState* app) {
    int j;
    int i;
    for (j = 1 - DRAW_PAD; j <= VIEW_TILE_H + DRAW_PAD; ++j) {
        int worldY = app->cameraY + j - 2;
        for (i = 1 - DRAW_PAD; i <= VIEW_TILE_W + DRAW_PAD; ++i) {
            int worldX = app->cameraX + i - 5 + (j / 2);
            float locX;
            float locY;
            int terrainId;
            C2D_Image image;

            if (!inBounds(app, worldX - 1, worldY - 1)) {
                continue;
            }

            posToLoc(app, worldX, worldY, &locX, &locY);
            terrainId = terrainAssetId(app->map.cells[worldY - 1][worldX - 1].terrain,
                                       app->map.cells[worldY - 1][worldX - 1].terrainVariant);
            if (terrainId < 0) {
                continue;
            }
            image = getImage(app, terrainId);
            if (!imageIntersectsScreen(image, locX, locY, DIRECTOR_SCALE, g_terrainAnchors[terrainId].x, g_terrainAnchors[terrainId].y, BOTTOM_W, BOTTOM_H)) {
                continue;
            }
            if (app->map.cells[worldY - 1][worldX - 1].hasGoalTerrain) {
                C2D_Image goalImage = getImage(app, ASSET_TERRAIN_GOALZONE);
                drawAnchoredImage(goalImage, locX, locY, DIRECTOR_SCALE, g_terrainAnchors[ASSET_TERRAIN_GOALZONE].x, g_terrainAnchors[ASSET_TERRAIN_GOALZONE].y);
            } else {
                drawAnchoredImage(image, locX, locY, DIRECTOR_SCALE, g_terrainAnchors[terrainId].x, g_terrainAnchors[terrainId].y);
            }
        }
    }
}

static void drawGreenPileVariant(AppState* app, int amount, float pileX, float pileY) {
    int assetId = ordinaryResourceVariant(
        amount,
        ASSET_RESOURCE_GREEN_1,
        ASSET_RESOURCE_GREEN_2,
        ASSET_RESOURCE_GREEN_3,
        ASSET_RESOURCE_GREEN_4);
    if (assetId < 0) {
        return;
    }
    drawAnchoredImage(getImage(app, assetId), pileX, pileY, RESOURCE_WORLD_SCALE,
        g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
}

static void drawResourceOrPlan(AppState* app, const WBCell* cell, float locX, float locY) {
    if (!cell->hasResource) {
        return;
    }
    if (cell->resourceIsPlan) {
        int assetId = worldPlanAssetId(cell->resourcePlanType);
        if (assetId >= 0) {
            drawAnchoredImage(getImage(app, assetId), locX, locY, PLAN_WORLD_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
        }
        return;
    }

    {
        int kinds[6];
        int count = 0;
        int i;
        float pileBaseY = locY;
        if (terrainUsesWaterSprite(cell->terrain)) {
            pileBaseY += 8.0f * DIRECTOR_SCALE;
        }
        sortPileKinds(&cell->pile, kinds, &count);
        if (count > 4) {
            count = 4;
        }
        for (i = 0; i < count; ++i) {
            int assetId = kinds[i] == 5 ? energyAssetForValue(energyStatusFromDeci(pileLastBattery(cell))) : pileEntryAssetId(&cell->pile, kinds[i]);
            float pileX = locX + (g_pileRegPoints[i].x * DIRECTOR_SCALE);
            float pileY = pileBaseY + (g_pileRegPoints[i].y * DIRECTOR_SCALE);
            if (kinds[i] == 2) {
                drawGreenPileVariant(app, cell->pile.green, pileX, pileY);
                continue;
            }
            if (assetId < 0) {
                continue;
            }
            drawAnchoredImage(getImage(app, assetId), pileX, pileY, RESOURCE_WORLD_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
        }
    }
}

/* goal.goal placeSprites(): it drifts round a 3 px circle (pDriftFreq 280-320 ms,
   a random phase); once satisfied it floats up 10 px a frame and away. */
static void drawGoal(AppState* app, const WBCell* cell, int x, int y, float locX, float locY) {
    int assetId;
    double freq;
    double phase;
    double t;
    float rise = 0.0f;
    if (!cell->hasGoal || (cell->bonusGoal && !app->map.bonusAvailable && !cell->goalSatisfied)) {
        return;
    }
    if (cell->goalSatisfied) {
        double frames = floor((app->simMs - cell->goalRiseStartMs) / (1000.0 / 15.0));
        rise = (float) (1.0 + 10.0 * (frames > 0.0 ? frames : 0.0));
        if (rise > 2000.0f) rise = 2000.0f;
        if (rise * DIRECTOR_SCALE > locY + 64.0f) {
            return;
        }
    }
    assetId = cell->bonusGoal ? ASSET_GOAL_BONUS : ASSET_GOAL_MAIN;
    freq = 281.0 + (double) (((x * 31) + (y * 17)) % 40);
    phase = (double) (((x * 7919) + (y * 104729)) % 628318) / 100000.0;
    t = phase + ((double) osGetTime() / freq);
    drawAnchoredImage(getImage(app, assetId),
        locX + (float) (3.0 * cos(t)),
        locY + (float) (3.0 * sin(t)) - rise * DIRECTOR_SCALE,
        DIRECTOR_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
}


/* ─── Monster sheet layout constants ─── */
/* monsters_a: crab=offset 0 (28), water_crab=28 (28), scorpion=56 (24), shark=80 (24), boulder=104 (5) */
/* monsters_b: trex=offset 0 (52), gator_land=52 (20), gator_water=72 (20) */
#define WB_MON_CRAB_BASE        (ASSET_MONSTERS_A_BASE + 0)
#define WB_MON_WATER_CRAB_BASE  (ASSET_MONSTERS_A_BASE + 28)
#define WB_MON_SCORPION_BASE    (ASSET_MONSTERS_A_BASE + 56)
#define WB_MON_SHARK_BASE       (ASSET_MONSTERS_A_BASE + 80)
#define WB_MON_BOULDER_BASE     (ASSET_MONSTERS_A_BASE + 104)
#define WB_MON_TREX_BASE        (ASSET_MONSTERS_B_BASE + 0)
#define WB_MON_GATOR_BASE       (ASSET_MONSTERS_B_BASE + 52)
#define WB_MON_GATOR_WATER_BASE (ASSET_MONSTERS_B_BASE + 72)

/* Within a 4-walk block (28 sprites):
   [0-3] idle UP,DOWN,LEFT,RIGHT
   [4..4+WALKS*4-1] walk: UP(walk1..N), DOWN(walk1..N), LEFT(walk1..N), RIGHT(walk1..N)
   [4+WALKS*4..end] atk: UP(1,2), DOWN(1,2), LEFT(1,2), RIGHT(1,2) = 8 */
static int monsterBlockFrame(int base, int walks, int dirIdx, int walkTick, bool attacking) {
    if (attacking) {
        return base + 4 + walks * 4 + dirIdx * 2 + (walkTick & 1);
    }
    if (walkTick > 0) {
        return base + 4 + dirIdx * walks + ((walkTick - 1) % walks);
    }
    return base + dirIdx;
}

static int monsterAnimAssetId(WBMonsterType type, WBDirection dir, bool onWater, int walkTick, bool attacking) {
    int dirIdx;
    switch (dir) {
        case WB_DIR_UP:    dirIdx = 0; break;
        case WB_DIR_DOWN:  dirIdx = 1; break;
        case WB_DIR_LEFT:  dirIdx = 2; break;
        case WB_DIR_RIGHT: default: dirIdx = 3; break;
    }
    switch (type) {
        case WB_MONSTER_CRAB:
            return monsterBlockFrame(WB_MON_CRAB_BASE, 4, dirIdx, walkTick, attacking);
        case WB_MONSTER_WATER_CRAB:
            return monsterBlockFrame(WB_MON_WATER_CRAB_BASE, 4, dirIdx, walkTick, attacking);
        case WB_MONSTER_SCORPION:
            return monsterBlockFrame(WB_MON_SCORPION_BASE, 3, dirIdx, walkTick, attacking);
        case WB_MONSTER_SHARK:
            return monsterBlockFrame(WB_MON_SHARK_BASE, 3, dirIdx, walkTick, attacking);
        case WB_MONSTER_BOULDER:
            /* boulder: [0]=generic,[1]=UP,[2]=DOWN,[3]=LEFT,[4]=RIGHT */
            return WB_MON_BOULDER_BASE + 1 + dirIdx;
        case WB_MONSTER_TREX:
            return monsterBlockFrame(WB_MON_TREX_BASE, 10, dirIdx, walkTick, attacking);
        case WB_MONSTER_GATOR:
            if (onWater) {
                return monsterBlockFrame(WB_MON_GATOR_WATER_BASE, 2, dirIdx, walkTick, attacking);
            }
            return monsterBlockFrame(WB_MON_GATOR_BASE, 2, dirIdx, walkTick, attacking);
        case WB_MONSTER_LION: {
            /* Lion sprites in ASSET_WB2_MONSTERS_BASE sheet, per-direction blocks of 5:
               [stand, walk1, walk2, atk1, atk2] each. Block offsets: DOWN+1, LEFT+6, RIGHT+11, UP+16 */
            static const int lionDirOffset[4] = {16, 1, 6, 11}; /* UP=0, DOWN=1, LEFT=2, RIGHT=3 */
            int base = ASSET_WB2_MONSTERS_BASE + lionDirOffset[dirIdx];
            if (attacking) return base + 3 + (walkTick & 1);
            if (walkTick > 0) return base + 1 + ((walkTick - 1) % 2);
            return base;
        }
        default:
            return -1;
    }
}

static C2D_Image monsterSheetFallbackImage(const AppState* app, const WBMonsterState* ms, int walkTick, bool attacking) {
    C2D_Image img;
    int dirIdx;
    int frame;
    int sheetIndex = -1;
    bool useSheetB = false;

    img.subtex = NULL;
    img.tex = NULL;
    if (!app || !ms) {
        return img;
    }

    switch (ms->dir) {
        case WB_DIR_UP:    dirIdx = 0; break;
        case WB_DIR_DOWN:  dirIdx = 1; break;
        case WB_DIR_LEFT:  dirIdx = 2; break;
        case WB_DIR_RIGHT: default: dirIdx = 3; break;
    }

    /* Lion uses wb2MonstersSheet - handle before the monstersA/B switch. */
    if (ms->type == WB_MONSTER_LION) {
        static const int lionDirOffset[4] = {16, 1, 6, 11};
        int base = lionDirOffset[dirIdx];
        int lframe;
        if (attacking) lframe = base + 3 + (walkTick & 1);
        else if (walkTick > 0) lframe = base + 1 + ((walkTick - 1) % 2);
        else lframe = base;
        if (app->wb2MonstersSheet) {
            img = C2D_SpriteSheetGetImage(app->wb2MonstersSheet, lframe);
        }
        return img;
    }

    switch (ms->type) {
        case WB_MONSTER_CRAB:
            frame = monsterBlockFrame(0, 4, dirIdx, walkTick, attacking);
            sheetIndex = frame;
            break;
        case WB_MONSTER_WATER_CRAB:
            frame = monsterBlockFrame(28, 4, dirIdx, walkTick, attacking);
            sheetIndex = frame;
            break;
        case WB_MONSTER_SCORPION:
            frame = monsterBlockFrame(56, 3, dirIdx, walkTick, attacking);
            sheetIndex = frame;
            break;
        case WB_MONSTER_SHARK:
            frame = monsterBlockFrame(80, 3, dirIdx, walkTick, attacking);
            sheetIndex = frame;
            break;
        case WB_MONSTER_BOULDER:
            sheetIndex = 1 + dirIdx;
            break;
        case WB_MONSTER_TREX:
            frame = monsterBlockFrame(0, 10, dirIdx, walkTick, attacking);
            sheetIndex = frame;
            useSheetB = true;
            break;
        case WB_MONSTER_GATOR:
            if (ms->onWater) {
                frame = monsterBlockFrame(72, 2, dirIdx, walkTick, attacking);
            } else {
                frame = monsterBlockFrame(52, 2, dirIdx, walkTick, attacking);
            }
            sheetIndex = frame;
            useSheetB = true;
            break;
        default:
            break;
    }

    if (sheetIndex >= 0) {
        C2D_SpriteSheet sheet = useSheetB ? app->monstersBSheet : app->monstersASheet;
        if (sheet) {
            img = C2D_SpriteSheetGetImage(sheet, sheetIndex);
        }
    }
    return img;
}

static int buildingAssetId(WBBuildingType btype, WBDirection dir, int factoryColor) {
    switch (btype) {
        case WB_BUILDING_GAS_STATION: return ASSET_BUILDING_GAS_STATION;
        case WB_BUILDING_MARINA:      return ASSET_BUILDING_MARINA;
        case WB_BUILDING_ROBOT_LAB:   return ASSET_BUILDING_ROBOT_LAB;
        case WB_BUILDING_GUARD_TOWER:
            switch (dir) {
                case WB_DIR_UP:    return ASSET_BUILDING_GUARD_TOWER_UP;
                case WB_DIR_DOWN:  return ASSET_BUILDING_GUARD_TOWER_DOWN;
                case WB_DIR_LEFT:  return ASSET_BUILDING_GUARD_TOWER_LEFT;
                case WB_DIR_RIGHT: return ASSET_BUILDING_GUARD_TOWER_RIGHT;
                default:           return ASSET_BUILDING_GUARD_TOWER;
            }
        case WB_BUILDING_AIRPORT:     return -1; /* not in asset dump */
        case WB_BUILDING_HOUSE:       return ASSET_BUILDING_HOUSE;
        case WB_BUILDING_FACTORY: {
            /* factoryColor 0-4: 0=RED, 1=YELLOW, 2=GREEN, 3=BLUE, 4=WHITE */
            static const int colorOffsets[5] = {3, 5, 2, 1, 4};
            int ci = (factoryColor < 0 || factoryColor > 4) ? 0 : factoryColor;
            return ASSET_BUILDING_FACTORY + colorOffsets[ci];
        }
        case WB_BUILDING_WINDMILL:    return ASSET_BUILDING_WINDMILL_1;
        case WB_BUILDING_GARAGE:      return ASSET_BUILDING_GARAGE;
        case WB_BUILDING_NURSERY:     return ASSET_BUILDING_NURSERY;
        default: return -1;
    }
}

/* Returns a suitably-sized icon asset for showing a building in the sidebar plan-style panel. */
static int buildingPanelIconAssetId(WBBuildingType buildingType) {
    switch (buildingType) {
        case WB_BUILDING_HOUSE:        return ASSET_PLAN_HOUSE;
        case WB_BUILDING_FACTORY:      return ASSET_PLAN_FACTORY;
        case WB_BUILDING_WINDMILL:     return ASSET_PLAN_WINDMILL;
        case WB_BUILDING_GARAGE:       return ASSET_PLAN_GARAGE;
        case WB_BUILDING_NURSERY:      return ASSET_PLAN_NURSERY;
        case WB_BUILDING_GAS_STATION:  return ASSET_PLAN_GAS_STATION;
        case WB_BUILDING_MARINA:       return ASSET_PLAN_MARINA;
        case WB_BUILDING_ROBOT_LAB:    return ASSET_PLAN_ROBOT_LAB;
        case WB_BUILDING_GUARD_TOWER:  return ASSET_PLAN_GUARD_TOWER;
        default: return buildingAssetId(buildingType, WB_DIR_RIGHT, 0);
    }
}

static void monsterDropRecipe(WBMonsterType type, WBResourcePile* out) {
    if (!out) {
        return;
    }
    memset(out, 0, sizeof(*out));
    switch (type) {
        case WB_MONSTER_CRAB:       out->red = 20; out->energy = 1; break;
        case WB_MONSTER_WATER_CRAB: out->blue = 40; out->energy = 1; break;
        case WB_MONSTER_GATOR:      out->green = 40; out->energy = 1; break;
        case WB_MONSTER_SCORPION:   out->red = 40; out->energy = 1; break;
        case WB_MONSTER_SHARK:      out->blue = 50; out->energy = 1; break;
        case WB_MONSTER_TREX:       out->red = 60; out->energy = 1; break;
        case WB_MONSTER_LION:       out->yellow = 40; out->energy = 1; break;
        default: break;
    }
}


/* ─── Object model helpers ─────────────────────────────────────────────── */

/* Lingo random(n): 1..n. ScummVM's Director model returns 1..65535 for n <= 0. */
static int lingoRandom(int n) {
    if (n <= 0) {
        return randRange(1, 65535);
    }
    return randRange(1, n);
}

static WBUnitAgent* agentAt(AppState* app, int x, int y) {
    const WBCell* cell;
    if (!inBounds(app, x, y)) {
        return NULL;
    }
    cell = &app->map.cells[y][x];
    if (!cell->hasUnit || cell->unitAgent == 0 || cell->unitAgent > WB_MAX_AGENTS) {
        return NULL;
    }
    return &app->agents[cell->unitAgent - 1];
}

static void actorListAdd(AppState* app, WBObjRef ref) {
    if (app->actorCount < WB_MAX_ACTORS) {
        app->actorList[app->actorCount++] = ref;
    }
}

static void actorListRemove(AppState* app, WBObjRef ref) {
    int i;
    for (i = 0; i < app->actorCount; ++i) {
        const WBObjRef* r = &app->actorList[i];
        if (r->kind == ref.kind && r->index == ref.index && r->gen == ref.gen && r->x == ref.x && r->y == ref.y) {
            memmove(&app->actorList[i], &app->actorList[i + 1], sizeof(WBObjRef) * (size_t) (app->actorCount - i - 1));
            --app->actorCount;
            return;
        }
    }
}

static WBObjRef unitRefFromAgent(AppState* app, int slot) {
    WBObjRef r;
    memset(&r, 0, sizeof(r));
    r.kind = WB_OBJ_UNIT;
    r.index = (u8) slot;
    r.gen = app->agents[slot].gen;
    return r;
}

static WBObjRef monsterRef(int mi) {
    WBObjRef r;
    memset(&r, 0, sizeof(r));
    r.kind = WB_OBJ_MONSTER;
    r.index = (u8) mi;
    r.gen = g_monsters[mi].gen;
    return r;
}

static WBObjRef buildingRef(AppState* app, int x, int y) {
    WBObjRef r;
    memset(&r, 0, sizeof(r));
    r.kind = WB_OBJ_BUILDING;
    r.x = (s8) x;
    r.y = (s8) y;
    r.gen = app->buildingGen[y][x];
    return r;
}

/* Give the unit in (x,y) its object: called whenever a unit appears in a cell. */
static int attachUnitAgent(AppState* app, int x, int y) {
    WBCell* cell = &app->map.cells[y][x];
    int slot;
    for (slot = 0; slot < WB_MAX_AGENTS; ++slot) {
        if (!app->agents[slot].used) {
            break;
        }
    }
    if (slot >= WB_MAX_AGENTS) {
        cell->unitAgent = 0;
        return -1;
    }
    {
        WBUnitAgent* a = &app->agents[slot];
        u16 gen = (u16) (a->gen + 1);
        memset(a, 0, sizeof(*a));
        a->used = true;
        a->gen = gen ? gen : 1;
        a->m.curX = x;
        a->m.curY = y;
        a->m.fromX = x;
        a->m.fromY = y;
    }
    cell->unitAgent = (u8) (slot + 1);
    actorListAdd(app, unitRefFromAgent(app, slot));
    return slot;
}

static void detachUnitAgent(AppState* app, int x, int y) {
    WBCell* cell = &app->map.cells[y][x];
    if (cell->unitAgent > 0 && cell->unitAgent <= WB_MAX_AGENTS) {
        int slot = cell->unitAgent - 1;
        actorListRemove(app, unitRefFromAgent(app, slot));
        app->agents[slot].used = false;
    }
    cell->unitAgent = 0;
}

static bool refAlive(AppState* app, WBObjRef ref, int* outX, int* outY) {
    switch (ref.kind) {
        case WB_OBJ_UNIT: {
            const WBUnitAgent* a;
            if (ref.index >= WB_MAX_AGENTS) return false;
            a = &app->agents[ref.index];
            if (!a->used || a->gen != ref.gen) return false;
            if (outX) *outX = a->m.curX;
            if (outY) *outY = a->m.curY;
            return true;
        }
        case WB_OBJ_MONSTER: {
            const WBMonsterState* ms;
            if (ref.index >= WB_MAX_MONSTERS) return false;
            ms = &g_monsters[ref.index];
            if (!ms->active || ms->gen != ref.gen) return false;
            if (outX) *outX = ms->col;
            if (outY) *outY = ms->row;
            return true;
        }
        case WB_OBJ_BUILDING:
            if (!inBounds(app, ref.x, ref.y)) return false;
            if (!app->map.cells[ref.y][ref.x].hasBuilding || app->buildingGen[ref.y][ref.x] != ref.gen) return false;
            if (outX) *outX = ref.x;
            if (outY) *outY = ref.y;
            return true;
        default:
            return false;
    }
}

static int monsterIndexAt(int x, int y) {
    int i;
    for (i = 0; i < g_monsterCount; ++i) {
        const WBMonsterState* ms = &g_monsters[i];
        if (ms->active && ms->col == x && ms->row == y) {
            return i;
        }
    }
    return -1;
}

/* tile[#occupant]: a unit, a monster or a building. */
static bool tileHasOccupant(const AppState* app, int x, int y) {
    const WBCell* cell = &app->map.cells[y][x];
    return cell->hasUnit || cell->hasBuilding || monsterIndexAt(x, y) >= 0;
}

/* The terrain symbol the original gives the tile (whirlpools are their own terrain). */
static WBTerrainType tileTerrain(const AppState* app, int x, int y) {
    const WBCell* cell = &app->map.cells[y][x];
    return cell->isWhirlpool ? WB_TERRAIN_WATER_WHIRLPOOL : cell->terrain;
}

/* vehicleStatus() of the unit in (x,y): #idle with no pPath. */
static bool unitIsIdleAt(AppState* app, int x, int y) {
    WBUnitAgent* a = agentAt(app, x, y);
    return !a || !a->m.hasPath;
}

typedef struct WBTerrainOwner {
    WBObjKind kind;
    int type;               /* WBUnitType or WBMonsterType */
    bool monsterSwamp;      /* monster: #swamp currently in its terrain list */
} WBTerrainOwner;

static bool ownerCanEnter(const WBTerrainOwner* o, WBTerrainType t) {
    if (o->kind == WB_OBJ_MONSTER) {
        if (t == WB_TERRAIN_SWAMP) {
            return o->monsterSwamp && wbMonsterCanTraverse((WBMonsterType) o->type, t);
        }
        return wbMonsterCanTraverse((WBMonsterType) o->type, t);
    }
    return wbUnitCanTraverse((WBUnitType) o->type, t);
}

/* ─── Path-finding: map display manager findPath() ────────────────────────
   A* over the 4-neighbourhood. Cost per step 1; the estimate is 1.5 x manhattan
   plus the swamp penalty (swamp_path_penalty 6, or swamp_chase_penalty 1 when
   chasing) on swamp nodes only. Equal-cost re-parents go one way or the other at
   random, closed nodes are reopened when a cheaper way in turns up, and after
   1000 ms it settles for the path to the node nearest the goal. A goal that is
   occupied or whose terrain the mover can't enter ends the path beside it, as
   does #nextto. */

typedef struct WBPathNode {
    float costFromStart;
    float costToGoal;
    float totalCost;
    s16 parentX;
    s16 parentY;
    u8 state;           /* 0 unseen, 1 open, 2 closed */
} WBPathNode;

static WBPathNode g_pathNodes[WB_MAX_MAP_H][WB_MAX_MAP_W];
static u16 g_openX[WB_PATH_MAX];
static u16 g_openY[WB_PATH_MAX];
static int g_openCount;

static float pathCostEstimate(const AppState* app, int ax, int ay, int bx, int by, bool chase) {
    float penalty = 0.0f;
    if (app->map.cells[ay][ax].terrain == WB_TERRAIN_SWAMP) {
        penalty = chase ? 1.0f : 6.0f;
    }
    return 1.5f * (float) manhattanDistance(ax, ay, bx, by) + penalty;
}

static bool openNodeLess(int ax, int ay, int bx, int by) {
    const WBPathNode* a = &g_pathNodes[ay][ax];
    const WBPathNode* b = &g_pathNodes[by][bx];
    if (a->totalCost != b->totalCost) return a->totalCost < b->totalCost;
    if (a->costFromStart != b->costFromStart) return a->costFromStart < b->costFromStart;
    if (a->costToGoal != b->costToGoal) return a->costToGoal < b->costToGoal;
    if (ax != bx) return ax < bx;
    return ay < by;
}

/* qopenSort.add(): insert into the sorted open list (entries are not re-sorted
   when their costs change later, as in the original). */
static void openListAdd(int x, int y) {
    int lo = 0;
    int hi = g_openCount;
    if (g_openCount >= WB_PATH_MAX) {
        return;
    }
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (openNodeLess(x, y, g_openX[mid], g_openY[mid])) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    memmove(&g_openX[lo + 1], &g_openX[lo], sizeof(u16) * (size_t) (g_openCount - lo));
    memmove(&g_openY[lo + 1], &g_openY[lo], sizeof(u16) * (size_t) (g_openCount - lo));
    g_openX[lo] = (u16) x;
    g_openY[lo] = (u16) y;
    ++g_openCount;
}

static int constructPath(int x, int y, u8* outX, u8* outY) {
    int len = 0;
    int cx = x;
    int cy = y;
    while (cx >= 0 && cy >= 0 && len < WB_PATH_MAX) {
        outX[len] = (u8) cx;
        outY[len] = (u8) cy;
        ++len;
        {
            int px = g_pathNodes[cy][cx].parentX;
            int py = g_pathNodes[cy][cx].parentY;
            cx = px;
            cy = py;
        }
    }
    {
        int i;
        for (i = 0; i < len / 2; ++i) {
            u8 t = outX[i]; outX[i] = outX[len - 1 - i]; outX[len - 1 - i] = t;
            t = outY[i]; outY[i] = outY[len - 1 - i]; outY[len - 1 - i] = t;
        }
    }
    return len;
}

/* Returns the path length (including the start tile), 0 for none. */
static int wbFindPath(AppState* app, int ax, int ay, int bx, int by, const WBTerrainOwner* owner,
                      bool nextto, bool chase, u8* outX, u8* outY) {
    static const int offs[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    u64 giveUp = osGetTime() + 1000;
    if (!inBounds(app, ax, ay) || !inBounds(app, bx, by)) {
        return 0;
    }
    memset(g_pathNodes, 0, sizeof(g_pathNodes));
    g_openCount = 0;
    {
        WBPathNode* s = &g_pathNodes[ay][ax];
        s->costFromStart = 0.0f;
        s->costToGoal = pathCostEstimate(app, ax, ay, bx, by, chase);
        s->totalCost = s->costToGoal;
        s->parentX = -1;
        s->parentY = -1;
        s->state = 1;
        openListAdd(ax, ay);
    }
    while (g_openCount > 0) {
        int nx = g_openX[0];
        int ny = g_openY[0];
        int k;
        memmove(&g_openX[0], &g_openX[1], sizeof(u16) * (size_t) (g_openCount - 1));
        memmove(&g_openY[0], &g_openY[1], sizeof(u16) * (size_t) (g_openCount - 1));
        --g_openCount;
        if (g_pathNodes[ny][nx].state != 1) {
            continue;
        }
        if (nx == bx && ny == by && !nextto) {
            return constructPath(nx, ny, outX, outY);
        }
        if (manhattanDistance(nx, ny, bx, by) == 1) {
            if (tileHasOccupant(app, bx, by) || !ownerCanEnter(owner, tileTerrain(app, bx, by)) || nextto) {
                return constructPath(nx, ny, outX, outY);
            }
        }
        for (k = 0; k < 4; ++k) {
            int qx = nx + offs[k][0];
            int qy = ny + offs[k][1];
            const WBCell* qc;
            WBPathNode* q;
            float g;
            if (!inBounds(app, qx, qy)) {
                continue;
            }
            qc = &app->map.cells[qy][qx];
            /* getNeighbors(pos, terrains, #vehicle): empty tiles of our terrain, or any
               tile holding a vehicle - and then only if that vehicle isn't idle. */
            if (qc->hasUnit) {
                if (unitIsIdleAt(app, qx, qy)) {
                    continue;
                }
            } else if (tileHasOccupant(app, qx, qy) || !ownerCanEnter(owner, tileTerrain(app, qx, qy))) {
                continue;
            }
            q = &g_pathNodes[qy][qx];
            g = g_pathNodes[ny][nx].costFromStart + 1.0f;
            if (q->state == 1) {
                if (g < q->costFromStart || (g == q->costFromStart && lingoRandom(2) == 1)) {
                    q->costFromStart = g;
                    q->totalCost = q->costFromStart + q->costToGoal;
                    q->parentX = (s16) nx;
                    q->parentY = (s16) ny;
                }
                continue;
            }
            if (q->state == 2) {
                if (g < q->costFromStart) {
                    q->state = 1;
                    q->costFromStart = g;
                    q->totalCost = q->costFromStart + q->costToGoal;
                    q->parentX = (s16) nx;
                    q->parentY = (s16) ny;
                    openListAdd(qx, qy);
                }
                continue;
            }
            q->costFromStart = g;
            q->costToGoal = pathCostEstimate(app, qx, qy, bx, by, chase);
            q->totalCost = q->costFromStart + q->costToGoal;
            q->parentX = (s16) nx;
            q->parentY = (s16) ny;
            q->state = 1;
            openListAdd(qx, qy);
        }
        g_pathNodes[ny][nx].state = 2;
        if (osGetTime() > giveUp) {
            float closest = 1.0e9f;
            int cx = -1;
            int cy = -1;
            int y2;
            int x2;
            for (y2 = 0; y2 < app->map.height; ++y2) {
                for (x2 = 0; x2 < app->map.width; ++x2) {
                    const WBPathNode* n = &g_pathNodes[y2][x2];
                    if (n->state != 0 && n->costToGoal < closest) {
                        closest = n->costToGoal;
                        cx = x2;
                        cy = y2;
                    }
                }
            }
            return cx < 0 ? 0 : constructPath(cx, cy, outX, outY);
        }
    }
    return 0;
}
/* ─── Movement: vehicle.generic parent ──────────────────────────────────── */

static WBMotion* refMotion(AppState* app, WBObjRef ref) {
    if (!refAlive(app, ref, NULL, NULL)) return NULL;
    if (ref.kind == WB_OBJ_UNIT) return &app->agents[ref.index].m;
    if (ref.kind == WB_OBJ_MONSTER) return &g_monsters[ref.index].m;
    return NULL;
}

static WBTerrainOwner refTerrainOwner(AppState* app, WBObjRef ref) {
    WBTerrainOwner o;
    memset(&o, 0, sizeof(o));
    o.kind = (WBObjKind) ref.kind;
    if (ref.kind == WB_OBJ_UNIT) {
        o.type = app->map.cells[app->agents[ref.index].m.curY][app->agents[ref.index].m.curX].unitType;
    } else if (ref.kind == WB_OBJ_MONSTER) {
        o.type = g_monsters[ref.index].type;
        o.monsterSwamp = g_monsters[ref.index].swampInTerrain;
    }
    return o;
}

static float refSpeed(AppState* app, WBObjRef ref) {
    if (ref.kind == WB_OBJ_UNIT) {
        const WBUnitAgent* a = &app->agents[ref.index];
        return unitMoveSpeed(app->map.cells[a->m.curY][a->m.curX].unitType);
    }
    if (ref.kind == WB_OBJ_MONSTER) {
        return g_monsters[ref.index].speed;
    }
    return 0.0f;
}

/* whirlpool.generic parent: a whirlpool takes one user at a time and is only
   active when it has a partner. */
static bool whirlpoolUserIs(AppState* app, int x, int y, WBObjRef ref) {
    const WBObjRef* u = &app->whirlpoolUser[y][x];
    return u->kind == ref.kind && u->index == ref.index && u->gen == ref.gen;
}

static bool whirlpoolHasUser(AppState* app, int x, int y) {
    return refAlive(app, app->whirlpoolUser[y][x], NULL, NULL);
}

static bool enterWhirlpool(AppState* app, int x, int y, WBObjRef unit, int* outX, int* outY) {
    int px;
    int py;
    if (whirlpoolHasUser(app, x, y) || !findWhirlpoolPair(app, x, y, &px, &py)) {
        return false;
    }
    if (tileHasOccupant(app, px, py)) {
        return false;
    }
    app->whirlpoolUser[y][x] = unit;
    app->whirlpoolUser[py][px] = unit;
    *outX = px;
    *outY = py;
    return true;
}

static void leaveWhirlpool(AppState* app, int x, int y, WBObjRef unit) {
    if (app->map.cells[y][x].isWhirlpool && whirlpoolUserIs(app, x, y, unit)) {
        memset(&app->whirlpoolUser[y][x], 0, sizeof(WBObjRef));
    }
}

static void moveUnitCell(AppState* app, int fx, int fy, int tx, int ty) {
    WBCell* f = &app->map.cells[fy][fx];
    WBCell* t = &app->map.cells[ty][tx];
    t->hasUnit = true;
    t->unitType = f->unitType;
    t->unitDirection = f->unitDirection;
    t->unitEnergy = f->unitEnergy;
    t->unitEnergyDeci = f->unitEnergyDeci;
    t->unitCargo = f->unitCargo;
    memcpy(t->unitCargoBatteries, f->unitCargoBatteries, sizeof(t->unitCargoBatteries));
    t->unitCargoSpecial = f->unitCargoSpecial;
    t->unitCargoStreetVariant = f->unitCargoStreetVariant;
    t->unitCargoTreeType = f->unitCargoTreeType;
    t->unitAgent = f->unitAgent;
    f->hasUnit = false;
    f->unitType = WB_UNIT_NONE;
    f->unitEnergy = 0;
    f->unitEnergyDeci = 0;
    memset(&f->unitCargo, 0, sizeof(f->unitCargo));
    memset(f->unitCargoBatteries, 0, sizeof(f->unitCargoBatteries));
    f->unitCargoSpecial = 0;
    f->unitCargoTreeType = WB_TERRAIN_NORMAL;
    f->unitAgent = 0;
    if (app->hasSelection && app->selectedX == fx && app->selectedY == fy) {
        app->selectedX = tx;
        app->selectedY = ty;
    }
}

/* map display manager MoveObject(): the destination must be on the map, free of
   any occupant, of the mover's terrain, and not a whirlpool someone else is using. */
static bool objMoveObject(AppState* app, WBObjRef ref, int tx, int ty) {
    WBMotion* m = refMotion(app, ref);
    WBTerrainOwner owner;
    if (!m || !inBounds(app, tx, ty) || tileHasOccupant(app, tx, ty)) {
        return false;
    }
    owner = refTerrainOwner(app, ref);
    if (!ownerCanEnter(&owner, tileTerrain(app, tx, ty))) {
        return false;
    }
    if (app->map.cells[ty][tx].isWhirlpool && whirlpoolHasUser(app, tx, ty) && !whirlpoolUserIs(app, tx, ty, ref)) {
        return false;
    }
    if (ref.kind == WB_OBJ_UNIT) {
        moveUnitCell(app, m->curX, m->curY, tx, ty);
        m->curX = tx;
        m->curY = ty;
    } else if (ref.kind == WB_OBJ_MONSTER) {
        g_monsters[ref.index].col = tx;
        g_monsters[ref.index].row = ty;
    }
    return true;
}

static void stopRecharging(AppState* app, WBObjRef ref) {
    if (ref.kind == WB_OBJ_UNIT) {
        app->agents[ref.index].recharging = false;
    }
}

static WBDirection directionFromOffset(int dx, int dy, WBDirection keep) {
    if (dx > 0) return WB_DIR_RIGHT;
    if (dx < 0) return WB_DIR_LEFT;
    if (dy > 0) return WB_DIR_DOWN;
    if (dy < 0) return WB_DIR_UP;
    return keep;
}

static void setRefDirection(AppState* app, WBObjRef ref, WBDirection dir) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return;
    if (ref.kind == WB_OBJ_UNIT) app->map.cells[y][x].unitDirection = dir;
    else if (ref.kind == WB_OBJ_MONSTER) g_monsters[ref.index].dir = dir;
}

static WBDirection refDirection(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return WB_DIR_RIGHT;
    if (ref.kind == WB_OBJ_UNIT) return app->map.cells[y][x].unitDirection;
    if (ref.kind == WB_OBJ_MONSTER) return g_monsters[ref.index].dir;
    return WB_DIR_RIGHT;
}

static void collectPlansAt(AppState* app, int x, int y);

/* moveTo(): the object now stands on its new tile; the sprite slides in from the
   old one. A unit pays its move energy and picks up any plan lying there. */
static void objMoveTo(AppState* app, WBObjRef ref, int fromX, int fromY) {
    WBMotion* m = refMotion(app, ref);
    int x;
    int y;
    if (!m || !refAlive(app, ref, &x, &y)) return;
    stopRecharging(app, ref);
    m->fromX = fromX;
    m->fromY = fromY;
    m->curX = x;
    m->curY = y;
    m->sliding = true;
    m->slideStartMs = app->simMs;
    m->moveIndex = 1.0f;
    m->teleportStep = false;
    setRefDirection(app, ref, directionFromOffset(x - fromX, y - fromY, refDirection(app, ref)));
    if (ref.kind == WB_OBJ_UNIT) {
        WBCell* cell = &app->map.cells[y][x];
        int cost = unitMoveCostDeci(cell->unitType);
        if (cost > 0) {
            cell->unitEnergyDeci -= cost;
            if (cell->unitEnergyDeci < 0) cell->unitEnergyDeci = 0;
            cell->unitEnergy = energyStatusFromDeci(cell->unitEnergyDeci);
        }
        collectPlansAt(app, x, y);
    } else if (ref.kind == WB_OBJ_MONSTER) {
        WBMonsterState* ms = &g_monsters[ref.index];
        ms->onWater = (app->map.cells[y][x].terrain == WB_TERRAIN_WATER ||
                       app->map.cells[y][x].terrain == WB_TERRAIN_WATER_UNFILLABLE);
    }
}

/* animateTileMove(): pMoveIndex runs 1 -> 0 over 1000/speed ms. */
static void objAnimateTileMove(AppState* app, WBObjRef ref) {
    WBMotion* m = refMotion(app, ref);
    float speed;
    double d;
    if (!m || !m->sliding) return;
    speed = refSpeed(app, ref);
    d = 1.0 - ((app->simMs - m->slideStartMs) * speed / 1000.0);
    if (d > 1.0 || d < 0.0 || speed <= 0.0f) {
        d = d > 1.0 ? 1.0 : 0.0;
        m->sliding = false;
        if (ref.kind == WB_OBJ_UNIT) app->agents[ref.index].stopPushing = true;
    }
    m->moveIndex = (float) d;
}

/* gotoPos(): a fresh pPath to pos; false when no path exists (pPath.path VOID). */
static bool objGotoPos(AppState* app, WBObjRef ref, int gx, int gy, u8 reason, bool nextto, bool chase) {
    WBMotion* m = refMotion(app, ref);
    WBTerrainOwner owner;
    int len;
    if (!m) return false;
    owner = refTerrainOwner(app, ref);
    len = wbFindPath(app, m->curX, m->curY, gx, gy, &owner, nextto, chase, m->pathX, m->pathY);
    m->hasPath = true;
    m->pathValid = len > 0;
    m->pathLen = len;
    m->step = 0;
    m->goalX = gx;
    m->goalY = gy;
    m->reason = reason;
    m->nextto = nextto;
    m->chase = chase;
    m->teleport = false;
    m->tryNum = 0;
    m->tryAgainSet = false;
    m->randomizeNum = 0;
    return m->pathValid;
}

static void objStopMoving(AppState* app, WBObjRef ref) {
    WBMotion* m = refMotion(app, ref);
    if (m) m->hasPath = false;
}

static int refEnergyStatus(AppState* app, WBObjRef ref);
static bool refHasBattery(AppState* app, WBObjRef ref);

/* bouncyVehicle(): ordinary vehicles bob 2 px while driving; the defender,
   treebot, frog, repairbot, freezebot and every monster override it to stand still. */
static float unitBounceY(WBUnitType t, double ms) {
    switch (t) {
        case WB_UNIT_DEFENDER:
        case WB_UNIT_TREEBOT:
        case WB_UNIT_FROG:
        case WB_UNIT_REPAIRBOT:
        case WB_UNIT_FREEZEBOT:
            return 0.0f;
        default:
            return (float) (2.0 * sin(ms / 50.0));
    }
}

/* followPath(), once per frame while the object has a pPath. */
static void objFollowPath(AppState* app, WBObjRef ref) {
    WBMotion* m = refMotion(app, ref);
    double t = app->simMs;
    float speed;
    if (!m || !m->hasPath) return;
    if (refHasBattery(app, ref) && refEnergyStatus(app, ref) == 0) {
        m->hasPath = false;
        return;
    }
    if (ref.kind == WB_OBJ_MONSTER && g_monsters[ref.index].frozenState != 0 && g_monsters[ref.index].frozenState != 3) {
        return;
    }
    if (!m->pathValid) {
        /* No way there: shudder, try once more straight away, then once the
           750-1250 ms wait is over, give up. */
        m->pixelOffX = (float) (2.0 * sin(t / 25.0));
        m->pixelOffY = 0.0f;
        if (!m->tryAgainSet || t > m->tryAgainMs) {
            WBTerrainOwner owner;
            int len;
            m->tryNum += 1;
            if (m->tryNum >= 2) {
                m->hasPath = false;
                return;
            }
            owner = refTerrainOwner(app, ref);
            len = wbFindPath(app, m->curX, m->curY, m->goalX, m->goalY, &owner, false, false, m->pathX, m->pathY);
            if (len <= 0) {
                m->tryAgainMs = t + 750 + lingoRandom(500);
                m->tryAgainSet = true;
                return;
            }
            m->tryNum = 0;
            m->pathLen = len;
            m->pathValid = true;
            m->step = 0;
            return;
        }
        return;
    }
    m->pixelOffX = 0.0f;
    m->pixelOffY = (ref.kind == WB_OBJ_UNIT)
        ? unitBounceY(app->map.cells[m->curY][m->curX].unitType, t) : 0.0f;
    speed = refSpeed(app, ref);
    if (speed <= 0.0f || (t - m->moveTimeMs) < (1000.0 / speed)) {
        return;
    }
    m->moveTimeMs = t;
    if (m->step >= m->pathLen - 1) {
        m->hasPath = false;
        m->checkDest = true;
        m->checkX = m->goalX;
        m->checkY = m->goalY;
        m->checkReason = m->reason;
        return;
    }
    {
        int prevX = m->curX;
        int prevY = m->curY;
        int nx = m->pathX[m->step + 1];
        int ny = m->pathY[m->step + 1];
        m->step += 1;
        if (objMoveObject(app, ref, nx, ny)) {
            int tx;
            int ty;
            bool teleport = false;
            if (app->map.cells[prevY][prevX].isWhirlpool) {
                leaveWhirlpool(app, prevX, prevY, ref);
            }
            if (m->teleport) {
                /* popping out of the partner whirlpool: no slide */
                objMoveTo(app, ref, prevX, prevY);
                m->teleportStep = true;
            } else {
                if (app->map.cells[ny][nx].isWhirlpool && enterWhirlpool(app, nx, ny, ref, &tx, &ty)) {
                    teleport = true;
                }
                objMoveTo(app, ref, prevX, prevY);
                if (teleport) {
                    m->goalX = tx;
                    m->goalY = ty;
                    m->pathLen = 2;
                    m->pathX[0] = (u8) nx;
                    m->pathY[0] = (u8) ny;
                    m->pathX[1] = (u8) tx;
                    m->pathY[1] = (u8) ty;
                    m->step = 0;
                    m->teleport = true;
                }
            }
            m->randomizeNum -= 1;
            if (m->randomizeNum < 0) m->randomizeNum = 0;
        } else {
            m->step -= 1;
            if (!m->randomizeNum) {
                m->randomizeMs = t + 500 + lingoRandom(1000);
                m->randomizeNum = 2;
            } else if (t > m->randomizeMs) {
                static const int offs[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
                WBTerrainOwner owner = refTerrainOwner(app, ref);
                int neiX[4];
                int neiY[4];
                int n = 0;
                int k;
                for (k = 0; k < 4; ++k) {
                    int qx = m->curX + offs[k][0];
                    int qy = m->curY + offs[k][1];
                    if (!inBounds(app, qx, qy) || tileHasOccupant(app, qx, qy)) continue;
                    if (!ownerCanEnter(&owner, tileTerrain(app, qx, qy))) continue;
                    neiX[n] = qx;
                    neiY[n] = qy;
                    ++n;
                }
                m->randomizeNum += 2;
                if (m->randomizeNum >= 6) {
                    /* gotoPos(goal) then pPath.path = VOID: the next frame re-paths
                       through the no-path branch (the options are lost, as there). */
                    objGotoPos(app, ref, m->goalX, m->goalY, WB_REASON_NONE, false, false);
                    m->pathValid = false;
                } else if (n == 0) {
                    m->randomizeMs = t + 1000 + lingoRandom(2000);
                } else {
                    int pick = lingoRandom(n) - 1;
                    int oldX = m->curX;
                    int oldY = m->curY;
                    if (!objMoveObject(app, ref, neiX[pick], neiY[pick])) {
                        m->pathValid = false;
                    } else {
                        objMoveTo(app, ref, oldX, oldY);
                        if (m->pathLen < WB_PATH_MAX) {
                            memmove(&m->pathX[m->step + 2], &m->pathX[m->step + 1], (size_t) (m->pathLen - m->step - 1));
                            memmove(&m->pathY[m->step + 2], &m->pathY[m->step + 1], (size_t) (m->pathLen - m->step - 1));
                            m->pathX[m->step + 1] = (u8) oldX;
                            m->pathY[m->step + 1] = (u8) oldY;
                            m->pathLen += 1;
                        }
                        m->moveTimeMs = t + 200 + lingoRandom(300);
                    }
                }
            }
        }
    }
}
/* ─── Energy, batteries and piles ──────────────────────────────────────── */

static bool refHasBattery(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return false;
    if (ref.kind == WB_OBJ_UNIT) return true;                       /* every unit recipe has #energy:1 */
    if (ref.kind == WB_OBJ_MONSTER) return g_monsters[ref.index].type != WB_MONSTER_BOULDER;
    if (ref.kind == WB_OBJ_BUILDING) {
        const WBResourcePile* r = wbBuildingRecipe(app->map.cells[y][x].buildingType);
        return r && r->energy > 0;
    }
    return false;
}

static int* refEnergyDeciPtr(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y) || !refHasBattery(app, ref)) return NULL;
    if (ref.kind == WB_OBJ_UNIT) return &app->map.cells[y][x].unitEnergyDeci;
    if (ref.kind == WB_OBJ_MONSTER) return &g_monsters[ref.index].hp;
    if (ref.kind == WB_OBJ_BUILDING) return &app->map.cells[y][x].buildingHp;
    return NULL;
}

/* energyStatus(); an object without batteries reads as VOID, which the original's
   "= 0" tests treat as 0. */
static int refEnergyStatus(AppState* app, WBObjRef ref) {
    int* e = refEnergyDeciPtr(app, ref);
    return e ? energyStatusFromDeci(*e) : 0;
}

static void syncUnitEnergy(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (ref.kind == WB_OBJ_UNIT && refAlive(app, ref, &x, &y)) {
        app->map.cells[y][x].unitEnergy = energyStatusFromDeci(app->map.cells[y][x].unitEnergyDeci);
    }
}

static void refUseEnergy(AppState* app, WBObjRef ref, int deci) {
    int* e = refEnergyDeciPtr(app, ref);
    if (!e || deci == 0) return;
    *e -= deci;
    if (*e < 0) *e = 0;
    syncUnitEnergy(app, ref);
}

static void refAddEnergy(AppState* app, WBObjRef ref, int deci) {
    int* e = refEnergyDeciPtr(app, ref);
    if (!e) return;
    *e += deci;
    if (*e > 1000) *e = 1000;
    syncUnitEnergy(app, ref);
}

static int pileTakeBattery(WBCell* c) {
    int n = pileBatteryCount(c);
    int charge = n > 0 ? c->pileBatteries[n - 1] : 1000;
    if (c->pile.energy > 0) c->pile.energy -= 1;
    return charge;
}

static void pileAppendBattery(WBCell* c, int charge) {
    int n = c->pile.energy;
    if (n < WB_MAX_PILE_BATTERIES) c->pileBatteries[n] = (u16) (charge < 0 ? 0 : (charge > 1000 ? 1000 : charge));
    c->pile.energy += 1;
}

static int compareU16(const void* a, const void* b) {
    return (int) *(const u16*) a - (int) *(const u16*) b;
}

/* getPlans(): a plan lying on the tile goes into the collection. */
static void collectPlansAt(AppState* app, int x, int y) {
    WBCell* cell = &app->map.cells[y][x];
    float locX;
    float locY;
    WBPlanType picked;
    if (!cell->hasResource || !cell->resourceIsPlan) return;
    picked = cell->resourcePlanType;
    if (picked > WB_PLAN_NONE && picked < WB_PLAN_COUNT) {
        app->map.planInventory[picked] += cell->resourcePlanCount;
    }
    playSfxClip(app, app->sfxPickupPlan.loaded ? &app->sfxPickupPlan : &app->sfxMove);
    posToLoc(app, x + 1, y + 1, &locX, &locY);
    startPlanSwoop(app, picked, locX + (10.0f * DIRECTOR_SCALE), locY + (18.0f * DIRECTOR_SCALE));
    cell->hasResource = false;
    cell->resourceIsPlan = false;
    cell->resourcePlanType = WB_PLAN_NONE;
    cell->resourcePlanCount = 0;
}

/* map display manager createResource(): a new pile sorts its batteries; adding to
   an existing pile appends them (giveBricks). energy == NULL means the original's
   VOID: every battery full. */
static void createResource(AppState* app, int x, int y, const WBResourcePile* bricks, const u16* energy, int energyCount) {
    WBCell* cell = &app->map.cells[y][x];
    int i;
    int batteries;
    if (cell->hasResource && cell->resourceIsPlan) {
        collectPlansAt(app, x, y);
    }
    batteries = energy ? energyCount : bricks->energy;
    if (!cell->hasResource) {
        cell->hasResource = true;
        cell->resourceIsPlan = false;
        memset(&cell->pile, 0, sizeof(cell->pile));
        memset(cell->pileBatteries, 0, sizeof(cell->pileBatteries));
        {
            WBResourcePile b = *bricks;
            b.energy = 0;
            addPile(&cell->pile, &b);
        }
        for (i = 0; i < batteries; ++i) {
            pileAppendBattery(cell, energy ? energy[i] : 1000);
        }
        qsort(cell->pileBatteries, (size_t) pileBatteryCount(cell), sizeof(u16), compareU16);
    } else {
        WBResourcePile b = *bricks;
        b.energy = 0;
        addPile(&cell->pile, &b);
        for (i = 0; i < batteries; ++i) {
            pileAppendBattery(cell, energy ? energy[i] : 1000);
        }
    }
    if (resourceTotal(&cell->pile) == 0) {
        cell->hasResource = false;
    }
}

/* ─── Combat: object.generic parent ───────────────────────────────────── */

static float refShield(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return 1.0f;
    if (ref.kind == WB_OBJ_UNIT) {
        switch (app->map.cells[y][x].unitType) {
            case WB_UNIT_DEFENDER:
            case WB_UNIT_DEFENDER2:
            case WB_UNIT_SPEEDBOAT:  return 0.05f;
            case WB_UNIT_FREEZEBOT:  return 0.5f;
            default:                 return 1.0f;
        }
    }
    if (ref.kind == WB_OBJ_BUILDING) {
        return app->map.cells[y][x].buildingType == WB_BUILDING_GUARD_TOWER ? 0.02f : 1.0f;
    }
    switch (g_monsters[ref.index].type) {
        case WB_MONSTER_CRAB:
        case WB_MONSTER_WATER_CRAB:
        case WB_MONSTER_GATOR:      return 0.1f;
        case WB_MONSTER_SCORPION:   return 0.045f;
        case WB_MONSTER_SHARK:      return 0.05f;
        case WB_MONSTER_TREX:       return 0.02f;
        case WB_MONSTER_LION:       return 0.03f;
        case WB_MONSTER_BOULDER:    return 0.00000001f;
        default:                    return 1.0f;
    }
}

/* pBuild.attack: which targets an attacker has stats for, from the WB1 / WB2 config. */
static bool attackStatsFor(AppState* app, WBObjRef self, WBObjRef target, WBAttackProfile* out) {
    int sx, sy, tx, ty;
    bool wb2 = app->map.isWB2Level;
    if (!refAlive(app, self, &sx, &sy) || !refAlive(app, target, &tx, &ty)) return false;
    if (self.kind == WB_OBJ_MONSTER) {
        /* monsters attack #units: buildings and vehicles */
        if (target.kind == WB_OBJ_MONSTER) return false;
        switch (g_monsters[self.index].type) {
            case WB_MONSTER_CRAB:
            case WB_MONSTER_WATER_CRAB:
            case WB_MONSTER_GATOR:    *out = (WBAttackProfile){25, 100, 100, 200, 4}; return true;
            case WB_MONSTER_SCORPION: *out = (WBAttackProfile){25, 100, 200, 400, 4}; return true;
            case WB_MONSTER_SHARK:    *out = (WBAttackProfile){25, 100, 300, 500, 4}; return true;
            case WB_MONSTER_TREX:     *out = (WBAttackProfile){25, 100, 400, 600, 4}; return true;
            case WB_MONSTER_LION:     *out = (WBAttackProfile){30, 100, 200, 400, 4}; return true;
            default: return false;
        }
    }
    if (target.kind != WB_OBJ_MONSTER) return false;
    {
        WBMonsterType mt = g_monsters[target.index].type;
        bool basic = (mt == WB_MONSTER_CRAB || mt == WB_MONSTER_WATER_CRAB || mt == WB_MONSTER_SCORPION || mt == WB_MONSTER_GATOR);
        if (self.kind == WB_OBJ_UNIT) {
            WBUnitType ut = app->map.cells[sy][sx].unitType;
            bool ok;
            switch (ut) {
                case WB_UNIT_DEFENDER:  ok = basic || mt == WB_MONSTER_SHARK || mt == WB_MONSTER_TREX || (wb2 && mt == WB_MONSTER_LION); break;
                case WB_UNIT_DEFENDER2: ok = basic; break;
                case WB_UNIT_SPEEDBOAT: ok = basic || mt == WB_MONSTER_SHARK || mt == WB_MONSTER_TREX; break;
                default: ok = false; break;
            }
            if (!ok) return false;
            *out = (WBAttackProfile){25, 75, 150, 250, 2};
            return true;
        }
        if (self.kind == WB_OBJ_BUILDING && app->map.cells[sy][sx].buildingType == WB_BUILDING_GUARD_TOWER) {
            bool ok = basic || mt == WB_MONSTER_SHARK || (wb2 && (mt == WB_MONSTER_LION || mt == WB_MONSTER_TREX));
            if (!ok) return false;
            *out = (WBAttackProfile){25, 75, 200, 300, 2};
            return true;
        }
    }
    return false;
}

/* pBuild[#attack_search_range], 0 for objects that don't hunt. */
static int attackSearchRange(AppState* app, WBObjRef self) {
    int x;
    int y;
    if (!refAlive(app, self, &x, &y)) return 0;
    if (self.kind == WB_OBJ_MONSTER) return g_monsters[self.index].type == WB_MONSTER_BOULDER ? 0 : 4;
    if (self.kind == WB_OBJ_UNIT) {
        WBUnitType t = app->map.cells[y][x].unitType;
        return (t == WB_UNIT_DEFENDER || t == WB_UNIT_DEFENDER2 || t == WB_UNIT_SPEEDBOAT) ? 2 : 0;
    }
    if (self.kind == WB_OBJ_BUILDING) {
        return app->map.cells[y][x].buildingType == WB_BUILDING_GUARD_TOWER ? 2 : 0;
    }
    return 0;
}

typedef struct WBAttackState {
    bool* hasTarget;
    WBObjRef* target;
    bool* nextAttackSet;
    double* nextAttackMs;
    WBAttackProfile* stats;
} WBAttackState;

static bool refAttackState(AppState* app, WBObjRef ref, WBAttackState* out) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return false;
    if (ref.kind == WB_OBJ_UNIT) {
        WBUnitAgent* a = &app->agents[ref.index];
        out->hasTarget = &a->hasTarget; out->target = &a->target;
        out->nextAttackSet = &a->nextAttackSet; out->nextAttackMs = &a->nextAttackMs; out->stats = &a->stats;
        return true;
    }
    if (ref.kind == WB_OBJ_MONSTER) {
        WBMonsterState* ms = &g_monsters[ref.index];
        out->hasTarget = &ms->hasTarget; out->target = &ms->target;
        out->nextAttackSet = &ms->nextAttackSet; out->nextAttackMs = &ms->nextAttackMs; out->stats = &ms->stats;
        return true;
    }
    out->hasTarget = &app->buildingHasTarget[y][x]; out->target = &app->buildingTarget[y][x];
    out->nextAttackSet = &app->buildingNextAttackSet[y][x]; out->nextAttackMs = &app->buildingNextAttackMs[y][x];
    out->stats = &app->buildingAttackStats[y][x];
    return true;
}

static bool acquireTarget(AppState* app, WBObjRef self, WBObjRef target) {
    WBAttackState st;
    WBAttackProfile stats;
    if (!refAttackState(app, self, &st) || !attackStatsFor(app, self, target, &stats)) return false;
    *st.hasTarget = true;
    *st.target = target;
    *st.stats = stats;
    return true;
}

/* disengageAttack(): a vehicle also stops, unless told not to. */
static void disengageAttack(AppState* app, WBObjRef self, bool dontStop) {
    WBAttackState st;
    if (!refAttackState(app, self, &st)) return;
    if (self.kind != WB_OBJ_BUILDING && !dontStop) {
        objStopMoving(app, self);
    }
    *st.hasTarget = false;
}

static void setActionCycle(AppState* app, WBObjRef ref, u8 anim) {
    if (ref.kind == WB_OBJ_UNIT) {
        app->agents[ref.index].actionAnim = anim;
        app->agents[ref.index].actionWhenMs = app->simMs;
    } else if (ref.kind == WB_OBJ_MONSTER) {
        g_monsters[ref.index].actionActive = true;
        g_monsters[ref.index].actionWhenMs = app->simMs;
    }
}

static bool* refDyingPtr(AppState* app, WBObjRef ref, double** damageMs) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return NULL;
    if (ref.kind == WB_OBJ_UNIT) { *damageMs = &app->agents[ref.index].damageMs; return &app->agents[ref.index].dying; }
    if (ref.kind == WB_OBJ_MONSTER) { *damageMs = &g_monsters[ref.index].damageMs; return &g_monsters[ref.index].dying; }
    *damageMs = &app->buildingDamageMs[y][x];
    return &app->buildingDying[y][x];
}

/* getAttacked(damage, attacker): damage in 0-100 energy units, reduced by the
   shield and rounded as Lingo's integer(). Swamp damage has no attacker. */
static void getAttacked(AppState* app, WBObjRef target, int damage, WBObjRef attacker) {
    double* damageMs = NULL;
    bool* dying = refDyingPtr(app, target, &damageMs);
    int dmg;
    int tx;
    int ty;
    if (!dying || !refAlive(app, target, &tx, &ty)) return;
    dmg = lingoInteger((double) damage * (double) refShield(app, target));
    if (dmg != 0) {
        refUseEnergy(app, target, dmg * 10);
        if (refEnergyStatus(app, target) == 0) {
            *dying = true;
        }
    }
    *damageMs = app->simMs;
    /* damageSize is worked out (13/17) but placeSprites always shows damage.small */
    spawnWorldEffect(app, WB_WORLD_EFFECT_DAMAGE_SMALL, tx, ty);
    if (attacker.kind != WB_OBJ_NONE) {
        playSfxClipWorld(app, &app->sfxDamage);
        if (target.kind == WB_OBJ_MONSTER && refAlive(app, attacker, NULL, NULL)) {
            acquireTarget(app, target, attacker);
        }
    }
}

static void followTarget(AppState* app, WBObjRef self) {
    WBAttackState st;
    int tx;
    int ty;
    if (self.kind == WB_OBJ_BUILDING) return;               /* object.generic: nothing */
    if (!refAttackState(app, self, &st) || !refAlive(app, *st.target, &tx, &ty)) return;
    if (!objGotoPos(app, self, tx, ty, WB_REASON_ATTACK, true, true)) {
        disengageAttack(app, self, false);
    }
}

static void doAttack(AppState* app, WBObjRef self) {
    WBAttackState st;
    int sx, sy, tx, ty;
    WBObjRef target;
    if (!refAttackState(app, self, &st) || !refAlive(app, self, &sx, &sy) || !refAlive(app, *st.target, &tx, &ty)) return;
    target = *st.target;
    if (self.kind != WB_OBJ_BUILDING) {
        setRefDirection(app, self, directionFromOffset(tx - sx, ty - sy, refDirection(app, self)));
    }
    setActionCycle(app, self, WB_UNIT_ANIM_ATTACK);
    if (lingoRandom(100) < st.stats->chanceOfSuccess) {
        int damage = lingoRandom(st.stats->damageMax - st.stats->damageMin) + st.stats->damageMin;
        getAttacked(app, target, damage, self);
    } else {
        getAttacked(app, target, 0, self);
    }
    if (self.kind == WB_OBJ_MONSTER) {
        playSfxClipWorld(app, &app->sfxMonsterAttack);
    }
}

static void checkDefending(AppState* app, WBObjRef self) {
    int r = attackSearchRange(app, self);
    int sx;
    int sy;
    int i;
    int j;
    if (r <= 0 || !refAlive(app, self, &sx, &sy)) return;
    for (i = -r; i <= r; ++i) {
        for (j = -r; j <= r; ++j) {
            int x = sx + i;
            int y = sy + j;
            WBObjRef occ;
            if (abs(i) + abs(j) > r || !inBounds(app, x, y)) continue;
            memset(&occ, 0, sizeof(occ));
            if (app->map.cells[y][x].hasUnit && agentAt(app, x, y)) {
                occ = unitRefFromAgent(app, app->map.cells[y][x].unitAgent - 1);
            } else if (app->map.cells[y][x].hasBuilding) {
                occ = buildingRef(app, x, y);
            } else {
                int mi = monsterIndexAt(x, y);
                if (mi < 0) continue;
                occ = monsterRef(mi);
            }
            if (acquireTarget(app, self, occ)) {
                followTarget(app, self);
                return;
            }
        }
    }
}

static void checkAttacking(AppState* app, WBObjRef self) {
    WBAttackState st;
    int sx, sy, tx, ty;
    if (!refAttackState(app, self, &st) || !refAlive(app, self, &sx, &sy)) return;
    if (!refAlive(app, *st.target, &tx, &ty)) {
        disengageAttack(app, self, false);
        return;
    }
    if (manhattanDistance(sx, sy, tx, ty) != 1) {
        followTarget(app, self);
    } else if (!*st.nextAttackSet || app->simMs > *st.nextAttackMs) {
        *st.nextAttackSet = true;
        *st.nextAttackMs = app->simMs + (60000.0 / st.stats->hitsPerMinute);
        doAttack(app, self);
    }
}

static void checkAttack(AppState* app, WBObjRef self) {
    WBAttackState st;
    if (!refAttackState(app, self, &st)) return;
    if (*st.hasTarget) {
        checkAttacking(app, self);
    } else {
        checkDefending(app, self);
    }
}

/* checkSwampDamage(): 50 every 0.7 s on swamp, through the shield, no sound. */
static void checkSwampDamage(AppState* app, WBObjRef self, bool* swampSet, double* swampMs) {
    int x;
    int y;
    WBObjRef none;
    if (!refAlive(app, self, &x, &y)) return;
    if (app->map.cells[y][x].terrain != WB_TERRAIN_SWAMP) {
        *swampSet = false;
        return;
    }
    if (!*swampSet) {
        *swampSet = true;
        *swampMs = app->simMs;
    }
    if ((app->simMs - *swampMs) > 700.0) {
        *swampMs = app->simMs;
        memset(&none, 0, sizeof(none));
        getAttacked(app, self, 50, none);
    }
}

/* ─── Recharging ──────────────────────────────────────────────────────── */

/* canRecharge(): the config's #recharges lists. */
static bool canRechargeUnit(AppState* app, WBObjRef charger, WBUnitType who) {
    int x;
    int y;
    bool wb2 = app->map.isWB2Level;
    if (!refAlive(app, charger, &x, &y)) return false;
    if (charger.kind == WB_OBJ_BUILDING) {
        switch (app->map.cells[y][x].buildingType) {
            case WB_BUILDING_GAS_STATION:
                return who == WB_UNIT_BUGGY || who == WB_UNIT_DIRTBUGGY || who == WB_UNIT_STEAMSHOVEL ||
                       who == WB_UNIT_DUMPTRUCK || who == WB_UNIT_FORKLIFT || who == WB_UNIT_DOZER;
            case WB_BUILDING_MARINA:
                return who == WB_UNIT_TUGBOAT || who == WB_UNIT_FREIGHTER || who == WB_UNIT_SPEEDBOAT;
            case WB_BUILDING_ROBOT_LAB:
                return who == WB_UNIT_TREEBOT || who == WB_UNIT_DEFENDER || who == WB_UNIT_DEFENDER2 ||
                       (wb2 && who == WB_UNIT_FREEZEBOT);
            default:
                return false;
        }
    }
    if (charger.kind == WB_OBJ_UNIT && app->map.cells[y][x].unitType == WB_UNIT_REPAIRBOT) {
        return who == WB_UNIT_TREEBOT || who == WB_UNIT_DEFENDER || who == WB_UNIT_DEFENDER2 ||
               who == WB_UNIT_REPAIRBOT || (wb2 && who == WB_UNIT_FREEZEBOT);
    }
    return false;
}

/* checkForRecharger(): idle and not full, look round every ~3 s; the last
   neighbour (right, down, left, up) that can charge us is the one we use. */
static void checkForRecharger(AppState* app, WBObjRef self) {
    static const int offs[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    WBUnitAgent* a;
    int x;
    int y;
    int k;
    if (self.kind != WB_OBJ_UNIT || !refAlive(app, self, &x, &y)) return;
    a = &app->agents[self.index];
    if (a->m.hasPath || refEnergyStatus(app, self) >= 100) return;
    if (a->checkRechargerSet && (app->simMs - a->checkRechargerMs) < 1500.0) return;
    a->checkRechargerSet = true;
    a->checkRechargerMs = app->simMs + 1500.0;
    for (k = 0; k < 4; ++k) {
        int nx = x + offs[k][0];
        int ny = y + offs[k][1];
        WBObjRef occ;
        if (!inBounds(app, nx, ny)) continue;
        memset(&occ, 0, sizeof(occ));
        if (app->map.cells[ny][nx].hasBuilding) occ = buildingRef(app, nx, ny);
        else if (app->map.cells[ny][nx].hasUnit && agentAt(app, nx, ny)) occ = unitRefFromAgent(app, app->map.cells[ny][nx].unitAgent - 1);
        else continue;
        if (canRechargeUnit(app, occ, app->map.cells[y][x].unitType)) {
            a->recharging = true;
            a->charger = occ;
        }
    }
}

/* doRecharge(): one point a frame. Buildings and the repairbot charge for free
   (the repairbot even when flat), turning to face the unit and animating. */
static void doRecharge(AppState* app, WBObjRef self) {
    WBUnitAgent* a;
    int x, y, cx, cy;
    if (self.kind != WB_OBJ_UNIT || !refAlive(app, self, &x, &y)) return;
    a = &app->agents[self.index];
    if (!a->recharging) return;
    if (!refAlive(app, a->charger, &cx, &cy)) {
        a->recharging = false;              /* the original keeps drawing on a destroyed charger */
        return;
    }
    if (a->charger.kind == WB_OBJ_UNIT) {
        WBUnitAgent* rb = &app->agents[a->charger.index];
        if (rb->repairFrame == 0) rb->repairFrame = 1;
        app->map.cells[cy][cx].unitDirection = directionFromOffset(x - cx, y - cy, WB_DIR_DOWN);
    }
    if (refEnergyStatus(app, self) == 100) {
        a->recharging = false;
        return;
    }
    refAddEnergy(app, self, 10);
}

/* ─── Death: disassemble() and recycle() ─────────────────────────────── */

static void destroyUnitAt(AppState* app, int x, int y);

static void disassembleRef(AppState* app, WBObjRef ref) {
    int x;
    int y;
    if (!refAlive(app, ref, &x, &y)) return;
    spawnWorldEffect(app, WB_WORLD_EFFECT_TAKE_APART_CLOUD, x, y);
    if (ref.kind == WB_OBJ_UNIT) {
        WBCell* cell = &app->map.cells[y][x];
        const WBResourcePile* recipe = wbUnitRecipe(cell->unitType);
        WBResourcePile bricks;
        u16 energy[1 + WB_MAX_CARGO_BATTERIES];
        int n = 0;
        int i;
        memset(&bricks, 0, sizeof(bricks));
        if (recipe) bricks = *recipe;
        /* recycle(): carried bricks come back too, but not dug earth or a sapling */
        if (unitSupportsPickDrop(cell->unitType)) {
            WBResourcePile cargo = cell->unitCargo;
            cargo.energy = 0;
            addPile(&bricks, &cargo);
        }
        bricks.energy = 0;
        energy[n++] = (u16) (cell->unitEnergyDeci < 0 ? 0 : cell->unitEnergyDeci);
        if (unitSupportsPickDrop(cell->unitType)) {
            for (i = 0; i < cell->unitCargo.energy && i < WB_MAX_CARGO_BATTERIES; ++i) {
                energy[n++] = cell->unitCargoBatteries[i];
            }
        }
        if (app->map.cells[y][x].isWhirlpool) leaveWhirlpool(app, x, y, ref);
        destroyUnitAt(app, x, y);
        bricks.energy = n;
        createResource(app, x, y, &bricks, energy, n);
    } else if (ref.kind == WB_OBJ_MONSTER) {
        WBMonsterState* ms = &g_monsters[ref.index];
        WBResourcePile drop;
        u16 energy[1] = { 1000 };   /* monster.disassemble: batteries back to full first */
        memset(&drop, 0, sizeof(drop));
        monsterDropRecipe(ms->type, &drop);
        ms->active = false;
        actorListRemove(app, ref);
        if (ms->type == WB_MONSTER_BOULDER) {
            drop.energy = 0;
            createResource(app, x, y, &drop, energy, 0);
        } else {
            drop.energy = 1;
            createResource(app, x, y, &drop, energy, 1);
        }
    } else {
        WBCell* cell = &app->map.cells[y][x];
        const WBResourcePile* recipe = wbBuildingRecipe(cell->buildingType);
        WBResourcePile bricks;
        u16 energy[1];
        int n = 0;
        memset(&bricks, 0, sizeof(bricks));
        if (recipe) bricks = *recipe;
        if (recipe && recipe->energy > 0) {
            energy[n++] = (u16) (cell->buildingHp < 0 ? 0 : cell->buildingHp);
        }
        actorListRemove(app, ref);
        cell->hasBuilding = false;
        cell->buildingType = WB_BUILDING_NONE;
        cell->buildingHp = 0;
        app->buildingGen[y][x] += 1;
        if (app->hasSelection && app->selectedX == x && app->selectedY == y) {
            app->hasSelection = false;
            app->infoOverlayOpen = false;
        }
        bricks.energy = n;
        createResource(app, x, y, &bricks, energy, n);
    }
    playSfxClipWorld(app, &app->sfxDisassemble);
}

static void checkDying(AppState* app, WBObjRef self) {
    double* damageMs = NULL;
    bool* dying = refDyingPtr(app, self, &damageMs);
    if (!dying || !*dying) return;
    if ((app->simMs - *damageMs) > 320.0) {
        disassembleRef(app, self);
    }
}
/* ─── Unit actions: vehicle.generic checkDestination() ───────────────── */

#define WB_CARGO_NONE   0
#define WB_CARGO_DIRT   1
#define WB_CARGO_SWAMP  2
#define WB_CARGO_STREET 3

static bool isTreeTerrain(WBTerrainType t) {
    return t == WB_TERRAIN_TREE || t == WB_TERRAIN_TREE2 || t == WB_TERRAIN_TREE3 || t == WB_TERRAIN_TREE4;
}

static bool isStreetTerrain(WBTerrainType t) {
    return t == WB_TERRAIN_STREET || t == WB_TERRAIN_CEMENT;
}

static int unitActionEnergyDeci(WBUnitType t, u8 reason) {
    switch (reason) {
        case WB_REASON_DIG:
        case WB_REASON_FILL:   return t == WB_UNIT_STEAMSHOVEL ? 12 : 0;
        case WB_REASON_UPROOT:
        case WB_REASON_PLANT:  return t == WB_UNIT_TREEBOT ? 10 : 0;
        default:               return 0;
    }
}

static void faceTile(AppState* app, WBObjRef self, int x, int y, int tx, int ty) {
    setRefDirection(app, self, directionFromOffset(tx - x, ty - y, refDirection(app, self)));
}

static void checkDestination(AppState* app, WBObjRef self, int tx, int ty, u8 reason) {
    int x;
    int y;
    WBCell* me;
    WBCell* tile;
    int dist;
    bool wb2 = app->map.isWB2Level;
    if (!refAlive(app, self, &x, &y) || !inBounds(app, tx, ty)) return;
    me = &app->map.cells[y][x];
    tile = &app->map.cells[ty][tx];
    dist = manhattanDistance(tx, ty, x, y);
    switch (reason) {
        case WB_REASON_DIG:
            if ((tile->terrain == WB_TERRAIN_NORMAL || tile->terrain == WB_TERRAIN_SWAMP || (wb2 && isStreetTerrain(tile->terrain))) &&
                    !tileHasOccupant(app, tx, ty) && me->unitCargoSpecial == WB_CARGO_NONE && dist == 1) {
                if (tile->terrain == WB_TERRAIN_NORMAL) {
                    me->unitCargoSpecial = WB_CARGO_DIRT;
                } else if (tile->terrain == WB_TERRAIN_SWAMP) {
                    me->unitCargoSpecial = WB_CARGO_SWAMP;
                } else {
                    me->unitCargoSpecial = WB_CARGO_STREET;
                    me->unitCargoStreetVariant = (u8) ((tile->terrain == WB_TERRAIN_CEMENT) ? 255 : tile->terrainVariant);
                }
                tile->terrain = WB_TERRAIN_WATER;
                refUseEnergy(app, self, unitActionEnergyDeci(me->unitType, reason));
                faceTile(app, self, x, y, tx, ty);
                setActionCycle(app, self, WB_UNIT_ANIM_DIG);
                playSfxClip(app, app->sfxDigGround.loaded ? &app->sfxDigGround : &app->sfxMoveMisc);
            }
            break;
        case WB_REASON_FILL:
            if (tile->terrain == WB_TERRAIN_WATER && !tileHasOccupant(app, tx, ty) && me->unitCargoSpecial != WB_CARGO_NONE && dist == 1) {
                if (me->unitCargoSpecial == WB_CARGO_DIRT) {
                    tile->terrain = WB_TERRAIN_NORMAL;
                } else if (me->unitCargoSpecial == WB_CARGO_STREET) {
                    if (me->unitCargoStreetVariant == 255) {
                        tile->terrain = WB_TERRAIN_CEMENT;
                    } else {
                        tile->terrain = WB_TERRAIN_STREET;
                        tile->terrainVariant = me->unitCargoStreetVariant;
                    }
                } else {
                    tile->terrain = WB_TERRAIN_SWAMP;
                }
                me->unitCargoSpecial = WB_CARGO_NONE;
                refUseEnergy(app, self, unitActionEnergyDeci(me->unitType, reason));
                faceTile(app, self, x, y, tx, ty);
                setActionCycle(app, self, WB_UNIT_ANIM_FILL);
                playSfxClip(app, app->sfxFillGround.loaded ? &app->sfxFillGround : &app->sfxMoveMisc);
            }
            break;
        case WB_REASON_PICK:
            if (pileEmpty(&me->unitCargo) && tile->hasResource && dist <= 1 && !tile->resourceIsPlan) {
                /* takeBricks(carries): the scarcest kind first, batteries from the end */
                int capacity = unitCarryCapacity(me->unitType);
                int carried = 0;
                while (carried < capacity && !pileEmpty(&tile->pile)) {
                    int kind = chooseLeastBrickKind(&tile->pile);
                    if (kind < 0) break;
                    if (kind == 5) {
                        int charge = pileTakeBattery(tile);
                        if (me->unitCargo.energy < WB_MAX_CARGO_BATTERIES) {
                            me->unitCargoBatteries[me->unitCargo.energy] = (u16) charge;
                        }
                        me->unitCargo.energy += 1;
                    } else {
                        takeOneKind(&tile->pile, kind);
                        addOneKind(&me->unitCargo, kind);
                    }
                    ++carried;
                }
                cleanupResourceCell(tile);
                playSfxClip(app, app->sfxPickup.loaded ? &app->sfxPickup : &app->sfxMove);
                faceTile(app, self, x, y, tx, ty);
            }
            break;
        case WB_REASON_DROP:
            if (dist <= 1) {
                bool dropped = false;
                if (!tile->hasResource && (tile->terrain == WB_TERRAIN_WATER || tile->terrain == WB_TERRAIN_NORMAL ||
                        (wb2 && isStreetTerrain(tile->terrain)))) {
                    dropped = true;
                } else if (tile->hasResource && !tile->resourceIsPlan) {
                    dropped = true;
                }
                if (dropped) {
                    u16 energy[WB_MAX_CARGO_BATTERIES];
                    int n = me->unitCargo.energy < WB_MAX_CARGO_BATTERIES ? me->unitCargo.energy : WB_MAX_CARGO_BATTERIES;
                    memcpy(energy, me->unitCargoBatteries, sizeof(u16) * (size_t) n);
                    createResource(app, tx, ty, &me->unitCargo, energy, n);
                    memset(&me->unitCargo, 0, sizeof(me->unitCargo));
                    memset(me->unitCargoBatteries, 0, sizeof(me->unitCargoBatteries));
                    playSfxClip(app, app->sfxDrop.loaded ? &app->sfxDrop : &app->sfxMove);
                    faceTile(app, self, x, y, tx, ty);
                }
            }
            break;
        case WB_REASON_UPROOT:
            if (isTreeTerrain(tile->terrain) && !tileHasOccupant(app, tx, ty) && me->unitCargoTreeType == WB_TERRAIN_NORMAL && dist == 1) {
                me->unitCargoTreeType = tile->terrain;
                tile->terrain = WB_TERRAIN_NORMAL;
                refUseEnergy(app, self, unitActionEnergyDeci(me->unitType, reason));
                setActionCycle(app, self, WB_UNIT_ANIM_UPROOT);
                faceTile(app, self, x, y, tx, ty);
                playSfxClip(app, app->sfxDigTree.loaded ? &app->sfxDigTree : &app->sfxMoveMisc);
            }
            break;
        case WB_REASON_PLANT:
            if (tile->terrain == WB_TERRAIN_NORMAL && !tile->hasResource && !tile->hasGoal && !tileHasOccupant(app, tx, ty) &&
                    me->unitCargoTreeType != WB_TERRAIN_NORMAL && dist == 1) {
                tile->terrain = isTreeTerrain(me->unitCargoTreeType) ? me->unitCargoTreeType : WB_TERRAIN_TREE;
                me->unitCargoTreeType = WB_TERRAIN_NORMAL;
                refUseEnergy(app, self, unitActionEnergyDeci(me->unitType, reason));
                setActionCycle(app, self, WB_UNIT_ANIM_PLANT);
                faceTile(app, self, x, y, tx, ty);
                playSfxClip(app, app->sfxPlantTree.loaded ? &app->sfxPlantTree : &app->sfxMoveMisc);
            }
            break;
        default:
            break;
    }
}

/* ─── Pushing: the dozer ──────────────────────────────────────────────── */

/* pushTowardsPos(): push what is on the tile (a unit-layer object that is
   pushable, else the pile) one tile on, then drive into the space. */
static void pushTowardsPos(AppState* app, WBObjRef self, int tx, int ty) {
    WBUnitAgent* a;
    int x;
    int y;
    if (self.kind != WB_OBJ_UNIT || !refAlive(app, self, &x, &y) || !inBounds(app, tx, ty)) return;
    a = &app->agents[self.index];
    if (a->m.sliding) {
        a->pushWaiting = true;
        a->pushX = tx;
        a->pushY = ty;
        return;
    }
    {
        WBCell* tile = &app->map.cells[ty][tx];
        int mi = monsterIndexAt(tx, ty);
        bool hasOccupant = tile->hasUnit || tile->hasBuilding || mi >= 0;
        int bx = tx + (tx - x);
        int by = ty + (ty - y);
        bool pushed = false;
        if (!hasOccupant && !tile->hasResource) {
            objGotoPos(app, self, tx, ty, WB_REASON_NONE, false, false);
            return;
        }
        if (!wbUnitCanTraverse(app->map.cells[y][x].unitType, tileTerrain(app, tx, ty))) {
            return;
        }
        if (hasOccupant) {
            if (mi >= 0) {
                WBMonsterState* ms = &g_monsters[mi];
                bool pushable = ms->type == WB_MONSTER_BOULDER ||
                                (app->map.isWB2Level && ms->frozenState != 0 && ms->frozenState != 3);
                if (pushable && inBounds(app, bx, by) && !tileHasOccupant(app, bx, by)) {
                    bool terrainOk;
                    if (ms->type == WB_MONSTER_BOULDER) {
                        /* boulder.pushTo: never onto a pile; its terrain is #normal (+#street) */
                        WBTerrainType bt = tileTerrain(app, bx, by);
                        terrainOk = !app->map.cells[by][bx].hasResource &&
                                    (bt == WB_TERRAIN_NORMAL || (app->map.isWB2Level && isStreetTerrain(bt)));
                    } else {
                        WBTerrainOwner o;
                        memset(&o, 0, sizeof(o));
                        o.kind = WB_OBJ_MONSTER;
                        o.type = ms->type;
                        o.monsterSwamp = ms->swampInTerrain;
                        terrainOk = ownerCanEnter(&o, tileTerrain(app, bx, by));
                    }
                    if (terrainOk) {
                        ms->col = bx;
                        ms->row = by;
                        ms->m.fromX = tx;
                        ms->m.fromY = ty;
                        ms->m.curX = bx;
                        ms->m.curY = by;
                        ms->pushedBy = self;
                        ms->pushed = true;
                        pushed = true;
                    }
                }
            }
        } else if (tile->hasResource && !tile->resourceIsPlan) {
            /* resource.pushTo: moveResource onto water or normal (+street) with no pile */
            if (inBounds(app, bx, by)) {
                WBCell* beyond = &app->map.cells[by][bx];
                WBTerrainType bt = tileTerrain(app, bx, by);
                if (!beyond->hasResource && (bt == WB_TERRAIN_WATER || bt == WB_TERRAIN_NORMAL ||
                        (app->map.isWB2Level && isStreetTerrain(bt)))) {
                    beyond->hasResource = true;
                    beyond->resourceIsPlan = false;
                    beyond->pile = tile->pile;
                    memcpy(beyond->pileBatteries, tile->pileBatteries, sizeof(beyond->pileBatteries));
                    tile->hasResource = false;
                    memset(&tile->pile, 0, sizeof(tile->pile));
                    memset(tile->pileBatteries, 0, sizeof(tile->pileBatteries));
                    app->pilePushX = bx;
                    app->pilePushY = by;
                    app->pilePushedBy = self;
                    app->pilePushed = true;
                    pushed = true;
                }
            }
        }
        if (pushed) {
            a->stopPushing = false;
            objGotoPos(app, self, tx, ty, WB_REASON_NONE, false, false);
            objFollowPath(app, self);
            refUseEnergy(app, self, 10);    /* #energy:[#push:1] */
        }
    }
}

/* ─── Freezebot (WB2) ─────────────────────────────────────────────────── */

static void freezeMonster(AppState* app, int mi) {
    static const WBDirection freezeDir[] = { WB_DIR_UP, WB_DIR_DOWN, WB_DIR_DOWN, WB_DIR_RIGHT, WB_DIR_LEFT, WB_DIR_UP };
    WBMonsterState* ms = &g_monsters[mi];
    ms->frozenMs = app->simMs;
    ms->frozenState = 1;            /* #waiting: freezes once its current step is done */
    objStopMoving(app, monsterRef(mi));
    playSfxClipWorld(app, &app->sfxDamage);
    switch (ms->type) {
        case WB_MONSTER_CRAB:     ms->dir = freezeDir[0]; break;
        case WB_MONSTER_GATOR:    ms->dir = freezeDir[1]; break;
        case WB_MONSTER_LION:     ms->dir = freezeDir[2]; break;
        case WB_MONSTER_SCORPION: ms->dir = freezeDir[3]; break;
        case WB_MONSTER_SHARK:    ms->dir = freezeDir[4]; break;
        case WB_MONSTER_TREX:     ms->dir = freezeDir[5]; break;
        default: break;             /* #watercrab never matched #water_crab in the original */
    }
}

static void checkFreezing(AppState* app, WBObjRef self) {
    WBUnitAgent* a;
    int x;
    int y;
    if (self.kind != WB_OBJ_UNIT || !refAlive(app, self, &x, &y)) return;
    a = &app->agents[self.index];
    switch (a->freezeState) {
        case 0: {                    /* #ready: findSomeoneToFreeze, the last one in range wins */
            int i;
            int j;
            if (refEnergyStatus(app, self) <= 1) break;
            for (i = -2; i <= 2; ++i) {
                for (j = -2; j <= 2; ++j) {
                    int mi;
                    if (abs(i) + abs(j) > 2 || !inBounds(app, x + i, y + j)) continue;
                    mi = monsterIndexAt(x + i, y + j);
                    if (mi < 0 || g_monsters[mi].type == WB_MONSTER_BOULDER) continue;
                    if (g_monsters[mi].frozenState == 0 || g_monsters[mi].frozenState == 3) {
                        a->freezeTarget = monsterRef(mi);
                        a->freezeState = 1;
                    }
                }
            }
            break;
        }
        case 1: {                    /* #targeted */
            int tx;
            int ty;
            if (refAlive(app, a->freezeTarget, &tx, &ty)) {
                faceTile(app, self, x, y, tx, ty);
                freezeMonster(app, a->freezeTarget.index);
            }
            a->freezeState = 2;
            a->freezeMs = app->simMs;
            refUseEnergy(app, self, 20);   /* #energy:[#freeze:2] */
            break;
        }
        case 2:                      /* #recharging */
            if ((app->simMs - a->freezeMs) / 1000.0 > 3.0) {
                a->freezeState = 0;
            }
            break;
    }
}

/* ─── Per-object frames ───────────────────────────────────────────────── */

static void stepUnit(AppState* app, WBObjRef self) {
    WBUnitAgent* a;
    int x;
    int y;
    if (!refAlive(app, self, &x, &y)) return;
    a = &app->agents[self.index];
    if (app->map.cells[y][x].unitType == WB_UNIT_FREEZEBOT && app->map.isWB2Level) {
        checkFreezing(app, self);
        if (!refAlive(app, self, NULL, NULL)) return;
    }
    objAnimateTileMove(app, self);
    if (a->pushWaiting && !a->m.sliding) {
        a->pushWaiting = false;
        pushTowardsPos(app, self, a->pushX, a->pushY);
    }
    if (!a->m.sliding && a->m.checkDest) {
        a->m.checkDest = false;
        checkDestination(app, self, a->m.checkX, a->m.checkY, a->m.checkReason);
    }
    if (a->m.hasPath) {
        objFollowPath(app, self);
    }
    checkForRecharger(app, self);
    /* vehicle.repairbot getMember(): repair.1/2 for 16 frames after each requestCharge */
    if (app->map.cells[a->m.curY][a->m.curX].unitType == WB_UNIT_REPAIRBOT) {
        if (a->repairFrame && !a->m.sliding) {
            a->repairShow = (u8) (1 + ((a->repairFrame / 4) % 2));
            if (++a->repairFrame > 15) {
                a->repairFrame = 0;
            }
        } else {
            a->repairShow = 0;
        }
    }
    doRecharge(app, self);
    if (!a->m.sliding) {
        checkAttack(app, self);
    }
    if (!refAlive(app, self, NULL, NULL)) return;
    checkSwampDamage(app, self, &a->swampSet, &a->swampMs);
    checkDying(app, self);
}

/* monster.generic wander(): one tile, then rest. The original's
   "if random(pWandering[#wander] = 1)" rolls random(0) or random(1), which is
   always truthy, so a monster rests after every step whatever #rest_every says. */
static void monsterWander(AppState* app, WBObjRef self) {
    static const int offs[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    WBMonsterState* ms = &g_monsters[self.index];
    double t = app->simMs;
    if (ms->hasTarget) {
        int tx;
        int ty;
        ms->swampInTerrain = true;
        if (refAlive(app, ms->target, &tx, &ty) && manhattanDistance(tx, ty, ms->col, ms->row) > 7) {
            disengageAttack(app, self, false);
            ms->swampInTerrain = false;
        }
        return;
    }
    ms->swampInTerrain = false;
    if (refEnergyStatus(app, self) == 0) return;
    if (ms->frozenState == 1 || ms->frozenState == 2) return;
    if (ms->resting) {
        if (t > ms->restUntilMs) ms->resting = false;
        return;
    }
    if (ms->m.sliding) return;
    {
        int destX[4];
        int destY[4];
        int n = 0;
        int k;
        int pick;
        for (k = 0; k < 4; ++k) {
            int qx = ms->col + offs[k][0];
            int qy = ms->row + offs[k][1];
            WBTerrainOwner o;
            if (!inBounds(app, qx, qy) || tileHasOccupant(app, qx, qy)) continue;
            o.kind = WB_OBJ_MONSTER; o.type = ms->type; o.monsterSwamp = false;
            if (!ownerCanEnter(&o, tileTerrain(app, qx, qy))) continue;
            destX[n] = qx; destY[n] = qy; ++n;
        }
        if (n == 0 && app->map.cells[ms->row][ms->col].terrain == WB_TERRAIN_SWAMP) {
            for (k = 0; k < 4; ++k) {
                int qx = ms->col + offs[k][0];
                int qy = ms->row + offs[k][1];
                WBTerrainOwner o;
                if (!inBounds(app, qx, qy) || tileHasOccupant(app, qx, qy)) continue;
                o.kind = WB_OBJ_MONSTER; o.type = ms->type; o.monsterSwamp = true;
                if (!ownerCanEnter(&o, tileTerrain(app, qx, qy))) continue;
                destX[n] = qx; destY[n] = qy; ++n;
            }
            if (n > 0) ms->swampInTerrain = true;
        }
        if (n == 0) {
            ms->resting = true;
            ms->restUntilMs = t + 500;
            return;
        }
        pick = lingoRandom(n) - 1;
        {
            int oldX = ms->col;
            int oldY = ms->row;
            bool moved = objMoveObject(app, self, destX[pick], destY[pick]);
            ms->swampInTerrain = false;
            if (!moved) {
                ms->resting = true;
                ms->restUntilMs = t + 500;
                return;
            }
            objMoveTo(app, self, oldX, oldY);
        }
        ms->resting = true;
        ms->restUntilMs = t + (double) wbMonsterRestForMs(ms->type) / 1000.0 * (lingoRandom(500) + 750);
    }
}

static void checkFrozen(AppState* app, WBObjRef self) {
    WBMonsterState* ms = &g_monsters[self.index];
    switch (ms->frozenState) {
        case 1:
            if (!ms->m.sliding) ms->frozenState = 2;
            break;
        case 2:
            if ((app->simMs - ms->frozenMs) / 1000.0 >= 25.0) {
                ms->frozenState = 3;
                ms->frozenMs = app->simMs;
                objStopMoving(app, self);
            }
            break;
        case 3:
            if ((app->simMs - ms->frozenMs) / 1000.0 > 0.3) ms->frozenState = 0;
            break;
        default:
            break;
    }
}

static void stepMonster(AppState* app, WBObjRef self) {
    WBMonsterState* ms;
    if (!refAlive(app, self, NULL, NULL)) return;
    ms = &g_monsters[self.index];
    if (app->map.isWB2Level) checkFrozen(app, self);
    monsterWander(app, self);
    objAnimateTileMove(app, self);
    if (ms->pushed && !refAlive(app, ms->pushedBy, NULL, NULL)) ms->pushed = false;
    if (ms->m.hasPath) objFollowPath(app, self);
    if (!ms->m.sliding && !(ms->frozenState == 1 || ms->frozenState == 2 || ms->frozenState == 3)) {
        checkAttack(app, self);
    }
    if (!refAlive(app, self, NULL, NULL)) return;
    checkSwampDamage(app, self, &ms->swampSet, &ms->swampMs);
    checkDying(app, self);
}

/* WB2 production buildings: each needs charge in its battery and pays 7.5 a cycle. */
static void stepProductionBuilding(AppState* app, WBObjRef self, int x, int y) {
    static const int offs[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    WBCell* cell = &app->map.cells[y][x];
    double t = app->simMs;
    int k;
    if (cell->buildingHp <= 0) return;              /* totalEnergy() = 0 */
    switch (cell->buildingType) {
        case WB_BUILDING_FACTORY: {
            int readyK = -1;
            for (k = 0; k < 4; ++k) {
                int nx = x + offs[k][0];
                int ny = y + offs[k][1];
                u8 found = 0;                        /* 1 boulder, 2 tree */
                if (inBounds(app, nx, ny)) {
                    int mi = monsterIndexAt(nx, ny);
                    if (mi >= 0) {
                        if (g_monsters[mi].type == WB_MONSTER_BOULDER) found = 1;
                    } else if (isTreeTerrain(app->map.cells[ny][nx].terrain)) {
                        found = 2;
                    }
                }
                if (app->factoryWhat[y][x][k] == found) {
                    if (found && (t - app->factoryWhen[y][x][k]) > 2000.0) readyK = k;
                    continue;
                }
                app->factoryWhen[y][x][k] = t;
                app->factoryWhat[y][x][k] = found;
            }
            if (readyK >= 0) {
                int nx = x + offs[readyK][0];
                int ny = y + offs[readyK][1];
                WBResourcePile bricks;
                u16 none[1] = { 0 };
                for (k = 0; k < 4; ++k) app->factoryWhen[y][x][k] = t;
                if (app->factoryWhat[y][x][readyK] == 1) {
                    int mi = monsterIndexAt(nx, ny);
                    if (mi >= 0) disassembleRef(app, monsterRef(mi));
                } else {
                    app->map.cells[ny][nx].terrain = WB_TERRAIN_NORMAL;
                }
                memset(&bricks, 0, sizeof(bricks));
                switch (cell->factoryColor) {        /* red, yellow, green, blue, white */
                    case 1: bricks.yellow = 25; break;
                    case 2: bricks.green = 25; break;
                    case 3: bricks.blue = 25; break;
                    case 4: bricks.white = 25; break;
                    default: bricks.red = 25; break;
                }
                createResource(app, nx, ny, &bricks, none, 0);
                refUseEnergy(app, self, 75);
                app->factoryWhat[y][x][readyK] = 0;
            }
            break;
        }
        case WB_BUILDING_WINDMILL:
        case WB_BUILDING_GARAGE:
        case WB_BUILDING_NURSERY: {
            int emptyX[4];
            int emptyY[4];
            int n = 0;
            bool blocked = false;
            for (k = 0; k < 4; ++k) {
                int nx = x + offs[k][0];
                int ny = y + offs[k][1];
                const WBCell* nc;
                if (!inBounds(app, nx, ny)) continue;
                nc = &app->map.cells[ny][nx];
                if (!tileHasOccupant(app, nx, ny) && !nc->hasResource && nc->terrain == WB_TERRAIN_NORMAL) {
                    emptyX[n] = nx;
                    emptyY[n] = ny;
                    ++n;
                }
                if (nc->hasResource && !nc->resourceIsPlan) {
                    if ((cell->buildingType == WB_BUILDING_WINDMILL && nc->pile.energy > 0) ||
                        (cell->buildingType == WB_BUILDING_GARAGE && nc->pile.wheel > 0)) {
                        blocked = true;
                        break;
                    }
                }
            }
            if (!blocked && n > 0) {
                if ((t - app->buildingReadyMs[y][x]) > 2000.0) {
                    int pick = lingoRandom(n) - 1;
                    if (cell->buildingType == WB_BUILDING_NURSERY) {
                        static const WBTerrainType trees[4] = { WB_TERRAIN_TREE, WB_TERRAIN_TREE2, WB_TERRAIN_TREE3, WB_TERRAIN_TREE4 };
                        int types = app->mapTreeTypeCount > 0 ? app->mapTreeTypeCount : 1;
                        int tp = lingoRandom(types) - 1;
                        app->map.cells[emptyY[pick]][emptyX[pick]].terrain =
                            app->mapTreeTypeCount > 0 ? app->mapTreeTypes[tp] : trees[0];
                    } else {
                        WBResourcePile bricks;
                        memset(&bricks, 0, sizeof(bricks));
                        if (cell->buildingType == WB_BUILDING_WINDMILL) bricks.energy = 1;
                        else bricks.wheel = 4;
                        createResource(app, emptyX[pick], emptyY[pick], &bricks, NULL, 0);
                    }
                    app->buildingReadyMs[y][x] = t;
                    refUseEnergy(app, self, 75);
                }
            } else {
                app->buildingReadyMs[y][x] = t;
            }
            break;
        }
        default:
            break;
    }
}

static void stepBuilding(AppState* app, WBObjRef self) {
    int x;
    int y;
    if (!refAlive(app, self, &x, &y)) return;
    checkAttack(app, self);
    if (!refAlive(app, self, NULL, NULL)) return;
    checkDying(app, self);
    if (!refAlive(app, self, NULL, NULL)) return;
    if (app->map.isWB2Level) stepProductionBuilding(app, self, x, y);
}

/* goal.goal parent checkGoal(): the occupant of the goal tile, if it is what the
   goal wants, satisfies it and stops. */
static void checkGoals(AppState* app) {
    int y;
    int x;
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            WBCell* cell = &app->map.cells[y][x];
            bool match = false;
            WBObjRef occ;
            if (!cell->hasGoal || cell->goalSatisfied || cell->goalIsCollect) continue;
            if (cell->bonusGoal && !app->map.bonusAvailable) continue;
            memset(&occ, 0, sizeof(occ));
            if (cell->hasUnit && agentAt(app, x, y)) {
                match = goalMatchesUnit(cell->goalType, cell->unitType);
                occ = unitRefFromAgent(app, cell->unitAgent - 1);
            } else if (cell->hasBuilding) {
                match = cell->goalType == WB_GOAL_ANYTHING || goalMatchesBuilding(cell->goalType, cell->buildingType);
            } else {
                int mi = monsterIndexAt(x, y);
                if (mi >= 0) {
                    match = cell->goalType == WB_GOAL_ANYTHING || goalMatchesMonster(cell->goalType, g_monsters[mi].type);
                    occ = monsterRef(mi);
                }
            }
            if (match) {
                satisfyGoalAt(app, cell, x, y);
                cell->goalRiseStartMs = app->simMs;
                if (occ.kind != WB_OBJ_NONE) objStopMoving(app, occ);
            }
        }
    }
}

/* map display manager checkScroll(): while the followed vehicle's tile is
   outside the autoscroll border (stage rect 130,111 - 376,262), scroll a tile. */
static void checkScroll(AppState* app) {
    int ux;
    int uy;
    int dx = 0;
    int dy = 0;
    float sx;
    float sy;
    if (!app->centerGoalActive) return;
    if (!refAlive(app, app->centerGoal, &ux, &uy)) {
        app->centerGoalActive = false;
        return;
    }
    sx = ((ux + 1 - app->cameraX) * DIRECTOR_TILE_SIZE_X) + DIRECTOR_PIXEL_TOPLEFT_X +
         (DIRECTOR_PIXEL_SKEW_X * (VIEW_TILE_H + (app->cameraY - (uy + 1)) - 2));
    sy = ((uy + 1 - app->cameraY) * DIRECTOR_TILE_SIZE_Y) + DIRECTOR_PIXEL_TOPLEFT_Y;
    if (sx < 130.0f) dx = -1;
    if (sy < 111.0f) dy = -1;
    if (sx > 376.0f) dx = 1;
    if (sy > 262.0f) dy = 1;
    if (dx || dy) {
        app->cameraX += dx;
        app->cameraY += dy;
        clampCamera(app);
    }
}

static void selectObjectAt(AppState* app, int x, int y, bool playSound);

/* One 15 fps movie frame: the map display first, then every actor in the order
   it was made, then the goals and the build cloud's hand-off. */
static void stepWorldFrame(AppState* app) {
    WBObjRef order[WB_MAX_ACTORS];
    int count = app->actorCount;
    int i;
    checkScroll(app);
    memcpy(order, app->actorList, sizeof(WBObjRef) * (size_t) count);
    for (i = 0; i < count; ++i) {
        switch (order[i].kind) {
            case WB_OBJ_UNIT:     stepUnit(app, order[i]); break;
            case WB_OBJ_MONSTER:  stepMonster(app, order[i]); break;
            case WB_OBJ_BUILDING: stepBuilding(app, order[i]); break;
            default: break;
        }
    }
    checkGoals(app);
    /* build cloud: 750 ms after building, the new object is clicked (selected) */
    if (app->buildSelectPending && app->simMs - app->buildSelectMs > 750.0) {
        int bx;
        int by;
        app->buildSelectPending = false;
        if (refAlive(app, app->buildSelectRef, &bx, &by)) {
            selectObjectAt(app, bx, by, false);
        }
    }
}

/* Runs the 15 fps movie frames due since the last call. A gap of more than a
   quarter second (a menu that pauses the game, a load) is not caught up: the
   world simply carries on from where it stopped. */
static void runWorldFrames(AppState* app) {
    u64 now = osGetTime();
    int steps = 0;
    if (app->simLastMs == 0 || now < app->simLastMs || now - app->simLastMs > 250) {
        app->simLastMs = now;
        return;
    }
    app->simAccumMs += (double) (now - app->simLastMs);
    app->simLastMs = now;
    while (app->simAccumMs >= WB_FRAME_MS) {
        app->simAccumMs -= WB_FRAME_MS;
        app->simMs += WB_FRAME_MS;
        stepWorldFrame(app);
        if (++steps >= 4) {
            app->simAccumMs = 0.0;
            break;
        }
    }
}
/* ─── World setup ─────────────────────────────────────────────────────── */

static void destroyUnitAt(AppState* app, int x, int y) {
    WBCell* cell = &app->map.cells[y][x];
    if (app->pushOverrideActive && agentAt(app, x, y) &&
            app->pushOverrideUnit.index == cell->unitAgent - 1) {
        app->pushOverrideActive = false;
    }
    detachUnitAgent(app, x, y);
    cell->hasUnit = false;
    cell->unitType = WB_UNIT_NONE;
    cell->unitEnergy = 0;
    cell->unitEnergyDeci = 0;
    memset(&cell->unitCargo, 0, sizeof(cell->unitCargo));
    memset(cell->unitCargoBatteries, 0, sizeof(cell->unitCargoBatteries));
    cell->unitCargoSpecial = WB_CARGO_NONE;
    cell->unitCargoTreeType = WB_TERRAIN_NORMAL;
    if (app->hasSelection && app->selectedX == x && app->selectedY == y) {
        app->hasSelection = false;
        app->infoOverlayOpen = false;
        app->actionMode = WB_ACTION_MOVE;
    }
}

/* A building appears (map load or build): give it its object. */
static void attachBuilding(AppState* app, int x, int y, int batteryDeci) {
    WBCell* cell = &app->map.cells[y][x];
    const WBResourcePile* recipe = wbBuildingRecipe(cell->buildingType);
    app->buildingGen[y][x] += 1;
    app->buildingHasTarget[y][x] = false;
    app->buildingNextAttackSet[y][x] = false;
    app->buildingDying[y][x] = false;
    app->buildingReadyMs[y][x] = app->simMs;
    memset(app->factoryWhat[y][x], 0, sizeof(app->factoryWhat[y][x]));
    cell->buildingHp = (recipe && recipe->energy > 0) ? batteryDeci : 0;
    actorListAdd(app, buildingRef(app, x, y));
}

static int addMonster(AppState* app, int x, int y, WBMonsterType type, WBDirection dir) {
    WBMonsterState* ms;
    float scale;
    if (g_monsterCount >= WB_MAX_MONSTERS) return -1;
    ms = &g_monsters[g_monsterCount];
    {
        u16 gen = (u16) (ms->gen + 1);
        memset(ms, 0, sizeof(*ms));
        ms->gen = gen ? gen : 1;
    }
    ms->active = true;
    ms->row = y;
    ms->col = x;
    ms->type = type;
    ms->dir = dir;
    ms->hp = 1000;
    /* init: speed x (80 + random(40)) / 100, then rest for random(2000) ms */
    scale = (float) (80 + lingoRandom(40)) / 100.0f;
    ms->wanderTimer = (int) (scale * 100.0f);
    ms->speed = wbMonsterSpeed(type) * scale;
    ms->onWater = (app->map.cells[y][x].terrain == WB_TERRAIN_WATER ||
                   app->map.cells[y][x].terrain == WB_TERRAIN_WATER_UNFILLABLE);
    ms->m.curX = x;
    ms->m.curY = y;
    ms->m.fromX = x;
    ms->m.fromY = y;
    ms->resting = true;
    ms->restUntilMs = app->simMs + lingoRandom(2000);
    actorListAdd(app, monsterRef(g_monsterCount));
    return g_monsterCount++;
}

/* readmap(): every object, in reading order, which is also the actorList order. */
static void initWorldObjects(AppState* app) {
    int x;
    int y;
    memset(app->agents, 0, sizeof(app->agents));
    app->actorCount = 0;
    memset(app->whirlpoolUser, 0, sizeof(app->whirlpoolUser));
    memset(app->buildingHasTarget, 0, sizeof(app->buildingHasTarget));
    memset(app->buildingDying, 0, sizeof(app->buildingDying));
    app->centerGoalActive = false;
    app->pushOverrideActive = false;
    app->buildSelectPending = false;
    app->spikeActive = false;
    app->pilePushed = false;
    app->mapTreeTypeCount = 0;
    app->simMs = (double) osGetTime();
    app->simLastMs = osGetTime();
    app->simAccumMs = 0.0;
    g_monsterCount = 0;
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            WBCell* cell = &app->map.cells[y][x];
            int i;
            if (isTreeTerrain(cell->terrain)) {
                bool seen = false;
                for (i = 0; i < app->mapTreeTypeCount; ++i) seen = seen || app->mapTreeTypes[i] == cell->terrain;
                if (!seen && app->mapTreeTypeCount < 4) app->mapTreeTypes[app->mapTreeTypeCount++] = cell->terrain;
            }
            if (cell->hasResource && !cell->resourceIsPlan) {
                for (i = 0; i < pileBatteryCount(cell); ++i) cell->pileBatteries[i] = 1000;
            }
            cell->unitAgent = 0;
            if (cell->hasUnit) {
                cell->unitEnergyDeci = 1000;
                cell->unitEnergy = 100;
                cell->unitCargoSpecial = WB_CARGO_NONE;
                memset(cell->unitCargoBatteries, 0, sizeof(cell->unitCargoBatteries));
                attachUnitAgent(app, x, y);
            }
            if (cell->hasMonster) {
                addMonster(app, x, y, cell->monsterType, cell->monsterDirection);
                cell->hasMonster = false;
                cell->monsterType = WB_MONSTER_NONE;
            }
            if (cell->hasBuilding) {
                attachBuilding(app, x, y, 1000);
            }
        }
    }
}

/* ─── Orders ──────────────────────────────────────────────────────────── */

static WBObjRef selectedUnitRef(AppState* app) {
    WBObjRef r;
    memset(&r, 0, sizeof(r));
    if (app->hasSelection && agentAt(app, app->selectedX, app->selectedY)) {
        r = unitRefFromAgent(app, app->map.cells[app->selectedY][app->selectedX].unitAgent - 1);
    }
    return r;
}

/* click(): select the unit or building in (x,y) - highlight, follow it, sound. */
static void selectObjectAt(AppState* app, int x, int y, bool playSound) {
    WBCell* cell = &app->map.cells[y][x];
    if (!cell->hasUnit && !cell->hasBuilding) return;
    app->hasSelection = true;
    app->selectedX = x;
    app->selectedY = y;
    app->infoOverlayOpen = false;
    app->actionMode = WB_ACTION_MOVE;           /* highlight(): pClickMode = VOID */
    if (cell->hasUnit && agentAt(app, x, y)) {
        app->centerGoal = unitRefFromAgent(app, cell->unitAgent - 1);
        app->centerGoalActive = true;
    }
    if (!playSound) return;
    if (cell->hasBuilding) {
        switch (cell->buildingType) {
            case WB_BUILDING_FACTORY:
                playSfxClip(app, app->sfxBldgFactory.loaded ? &app->sfxBldgFactory :
                                 app->sfxBldgGeneric.loaded ? &app->sfxBldgGeneric : &app->sfxUnitVehicle);
                break;
            case WB_BUILDING_GAS_STATION:
            case WB_BUILDING_MARINA:
                playSfxClip(app, app->sfxBldgGasStationMarina.loaded ? &app->sfxBldgGasStationMarina :
                                 app->sfxBldgGeneric.loaded ? &app->sfxBldgGeneric : &app->sfxUnitVehicle);
                break;
            case WB_BUILDING_GUARD_TOWER:
                playSfxClip(app, app->sfxBldgGuardTower.loaded ? &app->sfxBldgGuardTower :
                                 app->sfxBldgGeneric.loaded ? &app->sfxBldgGeneric : &app->sfxUnitVehicle);
                break;
            case WB_BUILDING_ROBOT_LAB:
                playSfxClip(app, app->sfxBldgRobotLab.loaded ? &app->sfxBldgRobotLab :
                                 app->sfxBldgGeneric.loaded ? &app->sfxBldgGeneric : &app->sfxUnitVehicle);
                break;
            default:
                playSfxClip(app, app->sfxBldgGeneric.loaded ? &app->sfxBldgGeneric : &app->sfxUnitVehicle);
                break;
        }
        return;
    }
    switch (cell->unitType) {
        case WB_UNIT_DUCK:
        case WB_UNIT_FROG:
        case WB_UNIT_FISH:
        case WB_UNIT_SNAIL:
            playSfxClip(app, &app->sfxUnitAnimal);
            break;
        case WB_UNIT_TREEBOT:
        case WB_UNIT_REPAIRBOT:
        case WB_UNIT_DEFENDER:
        case WB_UNIT_DEFENDER2:
        case WB_UNIT_FREEZEBOT:
            playSfxClip(app, app->sfxUnitRobot.loaded ? &app->sfxUnitRobot : &app->sfxUnitVehicle);
            break;
        default:
            playSfxClip(app, &app->sfxUnitVehicle);
            break;
    }
}

/* vehicle.generic mapclick(): order the selected unit to the tile. */
static void mapclickUnit(AppState* app, WBObjRef self, int tx, int ty) {
    WBMotion* m = refMotion(app, self);
    int x;
    int y;
    if (!m || !refAlive(app, self, &x, &y)) return;
    if (m->hasPath && m->teleport) return;
    app->spikeActive = true;
    app->spikeX = tx;
    app->spikeY = ty;
    app->spikeMs = (double) osGetTime();
    playSfxClip(app, &app->sfxButton);          /* sfx_game_move */
    app->centerGoal = self;
    app->centerGoalActive = true;
    switch (app->actionMode) {
        case WB_ACTION_PICK:
        case WB_ACTION_DROP: {
            u8 reason = app->actionMode == WB_ACTION_PICK ? WB_REASON_PICK : WB_REASON_DROP;
            objGotoPos(app, self, tx, ty, reason, !(tx == x && ty == y), false);
            app->actionMode = WB_ACTION_MOVE;
            break;
        }
        case WB_ACTION_DIG:
        case WB_ACTION_FILL:
        case WB_ACTION_UPROOT:
        case WB_ACTION_PLANT: {
            u8 reason = app->actionMode == WB_ACTION_DIG ? WB_REASON_DIG :
                        app->actionMode == WB_ACTION_FILL ? WB_REASON_FILL :
                        app->actionMode == WB_ACTION_UPROOT ? WB_REASON_UPROOT : WB_REASON_PLANT;
            objGotoPos(app, self, tx, ty, reason, true, false);
            app->actionMode = WB_ACTION_MOVE;
            break;
        }
        case WB_ACTION_ATTACK: {
            int mi = monsterIndexAt(tx, ty);
            if (mi >= 0 && acquireTarget(app, self, monsterRef(mi))) {
                objGotoPos(app, self, tx, ty, WB_REASON_ATTACK, true, false);
                app->actionMode = WB_ACTION_MOVE;
            }
            break;
        }
        default:
            objGotoPos(app, self, tx, ty, WB_REASON_NONE, false, false);
            disengageAttack(app, self, true);
            break;
    }
}

/* makeMenu(): the menu's second entry is the unit's special action. */
static WBActionMode unitSpecialAction(const WBCell* cell) {
    switch (cell->unitType) {
        case WB_UNIT_BUGGY:
        case WB_UNIT_DIRTBUGGY:
        case WB_UNIT_DUMPTRUCK:
        case WB_UNIT_FORKLIFT:
        case WB_UNIT_TUGBOAT:
        case WB_UNIT_FREIGHTER:
            return pileEmpty(&cell->unitCargo) ? WB_ACTION_PICK : WB_ACTION_DROP;
        case WB_UNIT_DOZER:
            return WB_ACTION_PUSH;
        case WB_UNIT_STEAMSHOVEL:
            return cell->unitCargoSpecial != WB_CARGO_NONE ? WB_ACTION_FILL : WB_ACTION_DIG;
        case WB_UNIT_TREEBOT:
            return cell->unitCargoTreeType != WB_TERRAIN_NORMAL ? WB_ACTION_PLANT : WB_ACTION_UPROOT;
        case WB_UNIT_DEFENDER:
        case WB_UNIT_DEFENDER2:
        case WB_UNIT_SPEEDBOAT:
            return WB_ACTION_ATTACK;
        default:
            return WB_ACTION_MOVE;
    }
}

/* menuclick(act) */
static void unitMenuClick(AppState* app, WBObjRef self, WBActionMode act) {
    int x;
    int y;
    if (!refAlive(app, self, &x, &y)) return;
    app->actionMode = WB_ACTION_MOVE;
    if (act != WB_ACTION_MOVE && act != WB_ACTION_ATTACK) {
        objStopMoving(app, self);
    }
    if (act != WB_ACTION_ATTACK) {
        disengageAttack(app, self, true);
    }
    if (act == WB_ACTION_PUSH) {
        /* doMenuclickPush(): the next click within a tile of here pushes */
        app->pushOverrideActive = true;
        app->pushOverrideUnit = self;
        app->pushOverrideX = x;
        app->pushOverrideY = y;
        app->actionMode = WB_ACTION_PUSH;
    } else if (act != WB_ACTION_MOVE) {
        app->actionMode = act;
    }
}

/* actionButton(): toggle between moving and the special action. */
static void handleActionButton(AppState* app) {
    WBObjRef self = selectedUnitRef(app);
    WBActionMode special;
    WBActionMode prev = app->actionMode;
    if (self.kind != WB_OBJ_UNIT) return;
    special = unitSpecialAction(&app->map.cells[app->selectedY][app->selectedX]);
    if (special == WB_ACTION_MOVE) return;
    if (app->actionMode == special && special != WB_ACTION_PUSH) {
        unitMenuClick(app, self, WB_ACTION_MOVE);
    } else {
        unitMenuClick(app, self, special);
    }
    if (app->actionMode != prev) {
        playSfxClip(app, &app->sfxMove);
    }
}

static void disassembleSelected(AppState* app) {
    WBCell* cell;
    if (!app->hasSelection || app->armedPlan != WB_PLAN_NONE || !inBounds(app, app->selectedX, app->selectedY)) return;
    cell = &app->map.cells[app->selectedY][app->selectedX];
    if (cell->hasUnit && agentAt(app, app->selectedX, app->selectedY)) {
        WBObjRef self = selectedUnitRef(app);
        unitMenuClick(app, self, WB_ACTION_MOVE);
        objStopMoving(app, self);
        disassembleRef(app, self);
    } else if (cell->hasBuilding) {
        disassembleRef(app, buildingRef(app, app->selectedX, app->selectedY));
    }
}

/*
 * tickCollectGoals — check every collect-goal each frame.
 * For each unsatisfied collect goal, flood-fill through adjacent ':' (hasGoalTerrain)
 * cells to build the capture zone, then count matching monsters inside it.
 * Reuses the global g_queueX/g_queueY/g_visited scratch buffers.
 */
static void tickCollectGoals(AppState* app) {
    static const int dx[4] = { 1, -1,  0,  0 };
    static const int dy[4] = { 0,  0, -1,  1 };
    int gy, gx;
    for (gy = 0; gy < app->map.height; ++gy) {
        for (gx = 0; gx < app->map.width; ++gx) {
            WBCell* goalCell = &app->map.cells[gy][gx];
            int head, tail, matches, i;
            if (!goalCell->hasGoal || goalCell->goalSatisfied || !goalCell->goalIsCollect) continue;
            if (goalCell->bonusGoal && !app->map.bonusAvailable) continue;

            /* Flood-fill to build zone: goal cell + adjacent hasGoalTerrain cells */
            memset(g_visited, 0, sizeof(g_visited));
            head = 0; tail = 0;
            g_queueX[tail] = gx; g_queueY[tail] = gy; ++tail;
            g_visited[gy][gx] = true;
            while (head < tail) {
                int cx = g_queueX[head];
                int cy = g_queueY[head];
                int d;
                ++head;
                for (d = 0; d < 4; ++d) {
                    int nx = cx + dx[d];
                    int ny = cy + dy[d];
                    if (!inBounds(app, nx, ny) || g_visited[ny][nx]) continue;
                    if (!app->map.cells[ny][nx].hasGoalTerrain) continue;
                    g_visited[ny][nx] = true;
                    g_queueX[tail] = nx; g_queueY[tail] = ny; ++tail;
                }
            }

            /* Count monsters of the required type present in the zone */
            matches = 0;
            for (i = 0; i < tail; ++i) {
                int cx = g_queueX[i];
                int cy = g_queueY[i];
                int mi;
                for (mi = 0; mi < g_monsterCount; ++mi) {
                    const WBMonsterState* ms = &g_monsters[mi];
                    if (!ms->active || ms->dying || ms->m.sliding) continue;
                    if (ms->col != cx || ms->row != cy) continue;
                    if (goalCell->goalCollectType == WB_MONSTER_NONE ||
                            ms->type == goalCell->goalCollectType) {
                        matches++;
                    }
                }
            }
            if (matches >= goalCell->goalCollectCount) {
                satisfyGoalAt(app, goalCell, gx, gy);
            }
        }
    }
}

/* vehicle.generic getCarryMember(): the most plentiful kind (ties go to the later
   kind); a battery shows the charge of the one on top. */
static void drawCargoIconAt(AppState* app, const WBResourcePile* cargo, int topBatteryDeci, float unitX, float unitY) {
    static const int order[7] = { 0, 2, 1, 3, 4, 5, 6 };   /* red, green, blue, yellow, wheel, energy, white */
    int topKind = -1;
    int topAmount = 0;
    int assetId;
    int i;
    if (!cargo || pileEmpty(cargo)) {
        return;
    }
    for (i = 0; i < 7; ++i) {
        int amount = order[i] == 6 ? cargo->white : pileEntryAmount(cargo, order[i]);
        if (amount > 0 && amount >= topAmount) {
            topAmount = amount;
            topKind = order[i];
        }
    }
    switch (topKind) {
        case 0: assetId = ASSET_RESOURCE_RED_1; break;
        case 1: assetId = ASSET_RESOURCE_BLUE_1; break;
        case 2: {
            C2D_Image greenImg = getWorldbuilderExtraImage(app, WB_EXTRA_CARRY_GREEN_IDX);
            if (greenImg.subtex) {
                drawAnchoredImage(greenImg,
                    unitX,
                    unitY - (12.0f * DIRECTOR_SCALE),
                    0.75f * DIRECTOR_SCALE,
                    greenImg.subtex->width * 0.5f,
                    greenImg.subtex->height * 0.5f);
                return;
            }
            assetId = ASSET_RESOURCE_YELLOW_1;
            break;
        }
        case 3: assetId = ASSET_RESOURCE_YELLOW_1; break;
        case 4: assetId = ASSET_RESOURCE_WHEEL_FULL; break;
        case 5: assetId = energyAssetForValue(energyStatusFromDeci(topBatteryDeci)); break;
        case 6: assetId = ASSET_CARRY_WHITE; break;
        default: assetId = -1; break;
    }
    if (assetId < 0) {
        return;
    }
    drawAnchoredImage(getImage(app, assetId),
        unitX,
        unitY - (12.0f * DIRECTOR_SCALE),
        0.75f * DIRECTOR_SCALE,
        g_objectAnchors[assetId].x,
        g_objectAnchors[assetId].y);
}

static void drawUnitCargoOverlay(AppState* app, const WBCell* cell, float unitX, float unitY) {
    int n;
    if (!cell || !unitSupportsPickDrop(cell->unitType) || pileEmpty(&cell->unitCargo)) {
        return;
    }
    n = cell->unitCargo.energy < WB_MAX_CARGO_BATTERIES ? cell->unitCargo.energy : WB_MAX_CARGO_BATTERIES;
    drawCargoIconAt(app, &cell->unitCargo, n > 0 ? cell->unitCargoBatteries[n - 1] : 0, unitX, unitY);
}

/* ─── Drawing the world's objects ─────────────────────────────────────── */

#define WB_EXTRA_SPIKE_IDX 122

/* The sim clock to the millisecond (simMs only moves in 15 fps steps). */
static double simNowMs(const AppState* app) {
    return app->simMs + app->simAccumMs;
}

/* pMoveIndex this instant: 1 at the start of a step, 0 once it has landed. The
   original only moves it once a frame; drawing it from the sim clock keeps the
   slide smooth at the 3DS's 60 fps. */
static float motionMoveIndex(const AppState* app, const WBMotion* m, float speed) {
    double d;
    if (!m->sliding || speed <= 0.0f) {
        return 0.0f;
    }
    d = 1.0 - ((simNowMs(app) - m->slideStartMs) * speed / 1000.0);
    if (d < 0.0) d = 0.0;
    if (d > 1.0) d = 1.0;
    return (float) d;
}

/* The slide from the tile the object left: (old loc - new loc) * pMoveIndex. The
   pop out of a whirlpool has no slide. */
static void motionSlideOffset(AppState* app, const WBMotion* m, float moveIndex, float* outDx, float* outDy) {
    float fx, fy, cx, cy;
    *outDx = 0.0f;
    *outDy = 0.0f;
    if (!m->sliding || m->teleportStep || moveIndex <= 0.0f) {
        return;
    }
    posToLoc(app, m->fromX + 1, m->fromY + 1, &fx, &fy);
    posToLoc(app, m->curX + 1, m->curY + 1, &cx, &cy);
    *outDx = (fx - cx) * moveIndex;
    *outDy = (fy - cy) * moveIndex;
}

/* Something the dozer pushed rides along with the dozer's slide until it stops. */
static bool pushedSlideOffset(AppState* app, WBObjRef pusher, int x, int y, float* outDx, float* outDy) {
    const WBUnitAgent* a;
    const WBCell* cell;
    float mi;
    float fx, fy, cx, cy;
    if (pusher.kind != WB_OBJ_UNIT || !refAlive(app, pusher, NULL, NULL)) {
        return false;
    }
    a = &app->agents[pusher.index];
    if (a->stopPushing || !a->m.sliding) {
        return false;
    }
    cell = &app->map.cells[a->m.curY][a->m.curX];
    mi = motionMoveIndex(app, &a->m, unitMoveSpeed(cell->unitType));
    posToLoc(app, a->m.curX + 1, a->m.curY + 1, &fx, &fy);
    posToLoc(app, x + 1, y + 1, &cx, &cy);
    *outDx = (fx - cx) * mi;
    *outDy = (fy - cy) * mi;
    return true;
}

static bool unitIsWaterVersion(WBUnitType t) {
    return t == WB_UNIT_DUCK || t == WB_UNIT_FROG || t == WB_UNIT_FISH;
}

/* Where the unit in agent slot 'slot' is drawn, and what it looks like. */
static void unitSpritePlace(AppState* app, int slot, float* outX, float* outY, WBUnitPose* outPose) {
    const WBUnitAgent* a = &app->agents[slot];
    const WBMotion* m = &a->m;
    const WBCell* cell = &app->map.cells[m->curY][m->curX];
    double now = simNowMs(app);
    float locX, locY, dx, dy, mi;
    WBUnitPose p;
    posToLoc(app, m->curX + 1, m->curY + 1, &locX, &locY);
    mi = motionMoveIndex(app, m, unitMoveSpeed(cell->unitType));
    motionSlideOffset(app, m, mi, &dx, &dy);
    /* pPixelOffset: the shudder and the driving bob, at the original's pixel sizes */
    *outX = roundf(locX + dx + (10.0f * DIRECTOR_SCALE) + (m->hasPath ? m->pixelOffX : 0.0f));
    *outY = roundf(locY + dy + (18.0f * DIRECTOR_SCALE) + (m->hasPath ? m->pixelOffY : 0.0f));
    memset(&p, 0, sizeof(p));
    p.type = cell->unitType;
    p.dir = cell->unitDirection;
    p.sliding = m->sliding;
    p.moveIndex = mi;
    p.tileWater = terrainUsesWaterSprite(cell->terrain);
    if (unitIsWaterVersion(cell->unitType)) {
        bool oldWater = m->sliding && terrainUsesWaterSprite(app->map.cells[m->fromY][m->fromX].terrain);
        p.water = (p.tileWater && (mi < 0.5f || oldWater)) || (oldWater && mi > 0.5f);
    }
    p.full = (cell->unitType == WB_UNIT_STEAMSHOVEL && cell->unitCargoSpecial != WB_CARGO_NONE) ||
             (cell->unitType == WB_UNIT_TREEBOT && cell->unitCargoTreeType != WB_TERRAIN_NORMAL);
    switch (cell->unitType) {
        case WB_UNIT_STEAMSHOVEL:
            if ((a->actionAnim == WB_UNIT_ANIM_DIG || a->actionAnim == WB_UNIT_ANIM_FILL) && now <= a->actionWhenMs + 200.0) {
                p.action = a->actionAnim;
                p.actionFrame = 1;
            }
            break;
        case WB_UNIT_TREEBOT:
            if (a->actionAnim == WB_UNIT_ANIM_UPROOT || a->actionAnim == WB_UNIT_ANIM_PLANT) {
                int f = 1 + lingoInteger((now - a->actionWhenMs) / 180.0);
                if (f <= 2) {
                    p.action = a->actionAnim;
                    p.actionFrame = f;
                }
            }
            break;
        case WB_UNIT_REPAIRBOT:
            if (a->repairShow) {
                p.action = WB_UNIT_ANIM_REPAIR;
                p.actionFrame = a->repairShow;
            }
            break;
        case WB_UNIT_FREEZEBOT:
            if (a->freezeState == 2 && (now - a->freezeMs) < 600.0) {
                p.action = WB_UNIT_ANIM_ATTACK;
                p.actionFrame = 1 + (((int) (now - a->freezeMs) / 100) % 2);
            }
            break;
        default:
            break;
    }
    *outPose = p;
}

static void drawUnitEnergyBadgeCharging(AppState* app, int energyDeci, float unitX, float unitY, bool charging) {
    if (charging && energyDeci > 0) {
        /* icon.charging.0/1/2 */
        int frame = (int) (osGetTime() / 230) % 3;
        int idx = (frame == 0) ? WB_EXTRA_UI_CHARGE_0_IDX :
                  (frame == 1) ? WB_EXTRA_UI_CHARGE_1_IDX : WB_EXTRA_UI_CHARGE_2_IDX;
        C2D_Image chargeImg = getWorldbuilderExtraImage(app, idx);
        if (chargeImg.subtex) {
            drawAnchoredImage(chargeImg,
                unitX + (12.0f * DIRECTOR_SCALE),
                unitY + (8.0f * DIRECTOR_SCALE),
                DIRECTOR_SCALE,
                chargeImg.subtex->width * 0.5f,
                chargeImg.subtex->height * 0.5f);
            return;
        }
    }
    drawUnitEnergyBadge(app, energyDeci, unitX, unitY);
}

static void drawUnitAgent(AppState* app, int slot) {
    const WBUnitAgent* a = &app->agents[slot];
    const WBCell* cell = &app->map.cells[a->m.curY][a->m.curX];
    float unitX;
    float unitY;
    WBUnitPose pose;
    int assetId;
    C2D_Image image;
    unitSpritePlace(app, slot, &unitX, &unitY, &pose);
    assetId = unitPoseAssetId(&pose);
    if (assetId < 0) {
        return;
    }
    image = getImage(app, assetId);
    if (!imageIntersectsScreen(image, unitX, unitY, DIRECTOR_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y, BOTTOM_W, BOTTOM_H)) {
        return;
    }
    drawAnchoredImage(image, unitX, unitY, DIRECTOR_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
    drawUnitCargoOverlay(app, cell, unitX, unitY);
    drawUnitEnergyBadgeCharging(app, cell->unitEnergyDeci, unitX, unitY, a->recharging);
    if (app->hasSelection && app->armedPlan == WB_PLAN_NONE && a->m.curX == app->selectedX && a->m.curY == app->selectedY) {
        drawSelectionHighlight(app, unitX, unitY, false, assetId);
    }
}

/* Monster walk frame (1-based), from each monster.* getMember(). */
static int monsterWalkTick(WBMonsterType type, float moveIndex) {
    double r = 1.0 - (double) moveIndex;
    int f;
    switch (type) {
        case WB_MONSTER_CRAB:
        case WB_MONSTER_WATER_CRAB:
            f = lingoInteger(r * 4 * 2); if (f < 1) f = 1; if (f > 4) f -= 4; return f;
        case WB_MONSTER_SHARK:
            f = lingoInteger(r * 3 * 2); if (f < 1) f = 1; if (f > 3) f -= 3; return f;
        case WB_MONSTER_GATOR:
            f = lingoInteger(r * 2 * 4); if (f < 1) f = 1; return 1 + ((f - 1) % 2);
        case WB_MONSTER_SCORPION:
            f = lingoInteger(r * 3 * 4); if (f < 1) f = 1; return 1 + ((f - 1) % 3);
        case WB_MONSTER_TREX:
            f = lingoInteger(r * 10 * 4); if (f < 1) f = 1; return 1 + ((f - 1) % 10);
        case WB_MONSTER_LION:
            f = lingoInteger(r * 2 * 2); if (f < 1) f = 1; return 1 + ((f - 1) % 2);
        default:
            return 0;
    }
}

static int monsterAttackFrames(WBMonsterType type) {
    return type == WB_MONSTER_TREX ? 3 : type == WB_MONSTER_LION ? 4 : 6;
}

static int monsterFrozenAsset(WBMonsterType type) {
    switch (type) {
        case WB_MONSTER_CRAB:       return ASSET_MONSTER_CRAB_FROZEN;
        case WB_MONSTER_WATER_CRAB: return ASSET_MONSTER_WATER_CRAB_FROZEN;
        case WB_MONSTER_GATOR:      return ASSET_MONSTER_GATOR_FROZEN;
        case WB_MONSTER_SCORPION:   return ASSET_MONSTER_SCORPION_FROZEN;
        case WB_MONSTER_SHARK:      return ASSET_MONSTER_SHARK_FROZEN;
        case WB_MONSTER_TREX:       return ASSET_MONSTER_TREX_FROZEN;
        case WB_MONSTER_LION:       return ASSET_MONSTER_LION_FROZEN;
        default:                    return -1;
    }
}

static void drawMonster(AppState* app, int mi) {
    WBMonsterState* ms = &g_monsters[mi];
    double now = simNowMs(app);
    float locX, locY, dx = 0.0f, dy = 0.0f, mIndex;
    float mX, mY;
    int walkTick = 0;
    bool attacking = false;
    bool frozenSprite = false;
    int mAssetId;
    C2D_Image mImg;
    posToLoc(app, ms->col + 1, ms->row + 1, &locX, &locY);
    mIndex = motionMoveIndex(app, &ms->m, ms->speed);
    if (ms->m.sliding) {
        motionSlideOffset(app, &ms->m, mIndex, &dx, &dy);
        walkTick = monsterWalkTick(ms->type, mIndex);
    } else if (ms->pushed && !pushedSlideOffset(app, ms->pushedBy, ms->col, ms->row, &dx, &dy)) {
        ms->pushed = false;
    }
    if (!ms->m.sliding && ms->actionActive) {
        int f = 1 + lingoInteger((now - ms->actionWhenMs) / 100.0);
        if (f <= monsterAttackFrames(ms->type)) {
            attacking = true;
            walkTick = f & 1;       /* attack.(1 + f mod 2) */
        } else {
            ms->actionActive = false;
        }
    }
    /* WB2 freeze: the frozen member, blinking with the live one in the last 4 s */
    if (app->map.isWB2Level && (ms->frozenState == 1 || ms->frozenState == 2)) {
        double remaining = 25.0 - (now - ms->frozenMs) / 1000.0;
        frozenSprite = remaining >= 4.0 || (lingoInteger(remaining * 5.0) % 2) != 0;
    }
    mX = roundf(locX + dx + (10.0f * DIRECTOR_SCALE) + (ms->m.hasPath ? ms->m.pixelOffX : 0.0f));
    mY = roundf(locY + dy + (18.0f * DIRECTOR_SCALE) + (ms->m.hasPath ? ms->m.pixelOffY : 0.0f));
    if (frozenSprite) {
        int frozenAsset = monsterFrozenAsset(ms->type);
        if (frozenAsset >= 0) {
            C2D_Image frozenImg = getImage(app, frozenAsset);
            if (frozenImg.subtex) {
                drawAnchoredImage(frozenImg, mX, mY, DIRECTOR_SCALE, g_objectAnchors[frozenAsset].x, g_objectAnchors[frozenAsset].y);
            }
        }
    } else {
        mAssetId = monsterAnimAssetId(ms->type, ms->dir, ms->onWater, walkTick, attacking);
        if (mAssetId < 0) {
            return;
        }
        mImg = getImage(app, mAssetId);
        if (!mImg.subtex) {
            loadGameplaySheets(app);
            mImg = getImage(app, mAssetId);
        }
        if (!mImg.subtex) {
            mImg = monsterSheetFallbackImage(app, ms, walkTick, attacking);
        }
        if (!mImg.subtex) {
            C2D_DrawRectSolid(mX - 2.0f, mY - 2.0f, 0.0f, 4.0f, 4.0f, C2D_Color32(0xFF, 0x30, 0x30, 0xFF));
            return;
        }
        /* centre X, bottom-align Y */
        C2D_DrawImageAt(mImg,
            mX - mImg.subtex->width * DIRECTOR_SCALE * 0.5f,
            mY - mImg.subtex->height * DIRECTOR_SCALE,
            0.0f, NULL, DIRECTOR_SCALE, DIRECTOR_SCALE);
    }
    if (ms->type != WB_MONSTER_BOULDER && ms->hp <= 200) {
        drawUnitEnergyBadge(app, ms->hp, mX, mY);
    }
}

/* building.guard_tower: faces its target, else turns left, up, right, down every 750 ms. */
static WBDirection guardTowerDirection(AppState* app, int x, int y) {
    static const WBDirection spin[4] = { WB_DIR_LEFT, WB_DIR_UP, WB_DIR_RIGHT, WB_DIR_DOWN };
    int tx;
    int ty;
    if (app->buildingHasTarget[y][x] && refAlive(app, app->buildingTarget[y][x], &tx, &ty)) {
        return directionFromOffset(tx - x, ty - y, WB_DIR_DOWN);
    }
    return spin[(osGetTime() / 750) % 4];
}

static void drawBuildingAt(AppState* app, int x, int y, float locX, float locY) {
    const WBCell* cell = &app->map.cells[y][x];
    WBDirection dir = cell->buildingType == WB_BUILDING_GUARD_TOWER ? guardTowerDirection(app, x, y) : cell->buildingDirection;
    int bAssetId = buildingAssetId(cell->buildingType, dir, cell->factoryColor);
    C2D_Image bImg;
    WBAnchor anchor;
    const WBResourcePile* recipe;
    if (bAssetId < 0) {
        return;
    }
    bImg = getImage(app, bAssetId);
    if (!bImg.subtex) {
        return;
    }
    anchor = g_objectAnchors[ASSET_BUILDING_GAS_STATION];
    if (bAssetId < (int) (sizeof(g_objectAnchors) / sizeof(g_objectAnchors[0]))) {
        anchor = g_objectAnchors[bAssetId];
    }
    drawAnchoredImage(bImg, locX, locY, DIRECTOR_SCALE, anchor.x, anchor.y);
    recipe = wbBuildingRecipe(cell->buildingType);
    if (recipe && recipe->energy > 0 && cell->buildingHp <= 200) {
        drawUnitEnergyBadge(app, cell->buildingHp, locX, locY);
    }
    if (app->hasSelection && app->armedPlan == WB_PLAN_NONE && x == app->selectedX && y == app->selectedY) {
        drawSelectionHighlight(app, locX, locY, false, bAssetId);
    }
}

typedef struct WBDrawItem {
    int key;            /* row-major index of the tile the sprite sorts with */
    u8 kind;
    u8 index;
} WBDrawItem;

static WBDrawItem g_drawItems[WB_MAX_AGENTS + WB_MAX_MONSTERS];

static int compareDrawItems(const void* a, const void* b) {
    const WBDrawItem* da = (const WBDrawItem*) a;
    const WBDrawItem* db = (const WBDrawItem*) b;
    if (da->key != db->key) return da->key - db->key;
    if (da->kind != db->kind) return (int) da->kind - (int) db->kind;
    return (int) da->index - (int) db->index;
}

/* A sliding sprite sorts with whichever of its two tiles is drawn later. */
static int motionSortKey(const WBMotion* m) {
    int a = m->curY * WB_MAX_MAP_W + m->curX;
    int b = m->fromY * WB_MAX_MAP_W + m->fromX;
    return (m->sliding && b > a) ? b : a;
}

static void drawPushedPileOffset(AppState* app, int x, int y, float* locX, float* locY) {
    float dx;
    float dy;
    if (!app->pilePushed || app->pilePushX != x || app->pilePushY != y) {
        return;
    }
    if (pushedSlideOffset(app, app->pilePushedBy, x, y, &dx, &dy)) {
        *locX += dx;
        *locY += dy;
    } else {
        app->pilePushed = false;
    }
}

static void drawObjectsBottom(AppState* app) {
    int y;
    int x;
    int itemCount = 0;
    int next = 0;
    int whirlFrame;
    int i;

    /* Recover from partial sheet-load failures to avoid invisible monsters. */
    if (!app->monstersASheet || !app->monstersBSheet) {
        loadGameplaySheets(app);
    }

    for (i = 0; i < WB_MAX_AGENTS; ++i) {
        if (app->agents[i].used) {
            g_drawItems[itemCount].key = motionSortKey(&app->agents[i].m);
            g_drawItems[itemCount].kind = WB_OBJ_UNIT;
            g_drawItems[itemCount].index = (u8) i;
            ++itemCount;
        }
    }
    for (i = 0; i < g_monsterCount; ++i) {
        if (g_monsters[i].active) {
            g_drawItems[itemCount].key = motionSortKey(&g_monsters[i].m);
            g_drawItems[itemCount].kind = WB_OBJ_MONSTER;
            g_drawItems[itemCount].index = (u8) i;
            ++itemCount;
        }
    }
    qsort(g_drawItems, (size_t) itemCount, sizeof(WBDrawItem), compareDrawItems);

    /* whirlpool animation frame (4 frames @ ~2fps) */
    whirlFrame = (int) ((osGetTime() / 500) % 4);
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            const WBCell* cell = &app->map.cells[y][x];
            int key = y * WB_MAX_MAP_W + x;
            float locX;
            float locY;
            posToLoc(app, x + 1, y + 1, &locX, &locY);
            drawGoal(app, cell, x, y, locX, locY);
            if (cell->hasResource) {
                float pileX = locX;
                float pileY = locY;
                drawPushedPileOffset(app, x, y, &pileX, &pileY);
                drawResourceOrPlan(app, cell, pileX, pileY);
            }
            /* Whirlpools above terrain/resources but below units/buildings. */
            if (cell->isWhirlpool) {
                int wAssetId = (cell->hasUnit || whirlpoolHasUser(app, x, y))
                    ? ASSET_WHIRLPOOL_STATIC
                    : (ASSET_WHIRLPOOL_WHIRL1 + whirlFrame);
                C2D_Image wImg = getImage(app, wAssetId);
                if (wImg.subtex) {
                    drawAnchoredImage(wImg, locX + (24.0f * DIRECTOR_SCALE), locY + (24.0f * DIRECTOR_SCALE), DIRECTOR_SCALE,
                        wImg.subtex->width * 0.5f,
                        wImg.subtex->height * 0.5f);
                }
            }
            while (next < itemCount && g_drawItems[next].key <= key) {
                if (g_drawItems[next].kind == WB_OBJ_UNIT) {
                    drawUnitAgent(app, g_drawItems[next].index);
                } else {
                    drawMonster(app, g_drawItems[next].index);
                }
                ++next;
            }
            if (cell->hasBuilding) {
                drawBuildingAt(app, x, y, locX, locY);
            }
        }
    }
    for (; next < itemCount; ++next) {
        if (g_drawItems[next].kind == WB_OBJ_UNIT) {
            drawUnitAgent(app, g_drawItems[next].index);
        } else {
            drawMonster(app, g_drawItems[next].index);
        }
    }
    drawWorldEffectsBottom(app);
    drawClickSpike(app);
}

/* The unit or building whose sprite is under the touch, topmost first (the
   'object click catcher' sends a click on a sprite to that object's tile). */
static bool objectSpriteAtTouch(AppState* app, const touchPosition* touch, int* outX, int* outY) {
    int bestKey = -1;
    int i;
    int y;
    int x;
    for (i = 0; i < WB_MAX_AGENTS; ++i) {
        const WBUnitAgent* a = &app->agents[i];
        float unitX;
        float unitY;
        WBUnitPose pose;
        int assetId;
        C2D_Image image;
        float left;
        float top;
        int key;
        if (!a->used) continue;
        unitSpritePlace(app, i, &unitX, &unitY, &pose);
        assetId = unitPoseAssetId(&pose);
        if (assetId < 0) continue;
        image = getImage(app, assetId);
        left = unitX - (g_objectAnchors[assetId].x * DIRECTOR_SCALE);
        top = unitY - (g_objectAnchors[assetId].y * DIRECTOR_SCALE);
        if (!pointInRect((float) touch->px, (float) touch->py, left, top, imageWidth(image, DIRECTOR_SCALE), imageHeight(image, DIRECTOR_SCALE))) continue;
        key = motionSortKey(&a->m);
        if (key >= bestKey) {
            bestKey = key;
            *outX = a->m.curX;
            *outY = a->m.curY;
        }
    }
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            const WBCell* cell = &app->map.cells[y][x];
            int bAssetId;
            C2D_Image bImg;
            WBAnchor anchor;
            float locX;
            float locY;
            int key = y * WB_MAX_MAP_W + x;
            if (!cell->hasBuilding || key < bestKey) continue;
            bAssetId = buildingAssetId(cell->buildingType, cell->buildingDirection, cell->factoryColor);
            if (bAssetId < 0) continue;
            bImg = getImage(app, bAssetId);
            if (!bImg.subtex) continue;
            anchor = g_objectAnchors[ASSET_BUILDING_GAS_STATION];
            if (bAssetId < (int) (sizeof(g_objectAnchors) / sizeof(g_objectAnchors[0]))) {
                anchor = g_objectAnchors[bAssetId];
            }
            posToLoc(app, x + 1, y + 1, &locX, &locY);
            if (!pointInRect((float) touch->px, (float) touch->py, locX - anchor.x * DIRECTOR_SCALE, locY - anchor.y * DIRECTOR_SCALE,
                    imageWidth(bImg, DIRECTOR_SCALE), imageHeight(bImg, DIRECTOR_SCALE))) continue;
            bestKey = key;
            *outX = x;
            *outY = y;
        }
    }
    return bestKey >= 0;
}

/* A tap on a unit's sprite selects it (used by the tutorial's 'occupant' step). */
static bool trySelectUnitAtTouch(AppState* app, const touchPosition* touch) {
    int x;
    int y;
    if (!objectSpriteAtTouch(app, touch, &x, &y) || !app->map.cells[y][x].hasUnit) {
        return false;
    }
    app->armedPlan = WB_PLAN_NONE;
    selectObjectAt(app, x, y, true);
    return true;
}

/* click spike behavior: an arrow on the clicked tile for 125 ms, then it rises 21 px
   and fades out over 400 ms. */
static void drawClickSpike(AppState* app) {
    double t;
    float locX;
    float locY;
    float rise = 0.0f;
    float blend = 1.0f;
    C2D_Image img;
    C2D_ImageTint tint;
    if (!app->spikeActive) {
        return;
    }
    t = (double) osGetTime() - app->spikeMs;
    if (t > 525.0) {
        app->spikeActive = false;
        return;
    }
    if (t > 125.0) {
        float tt = (float) ((t - 125.0) / 400.0);
        blend = 1.0f - tt;
        rise = 21.0f * tt;
    }
    img = getWorldbuilderExtraImage(app, WB_EXTRA_SPIKE_IDX);
    if (!img.subtex) {
        return;
    }
    posToLoc(app, app->spikeX + 1, app->spikeY + 1, &locX, &locY);
    C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, (u8) (blend * 255.0f)), 0.0f);
    /* member spike2: 11x13, regPoint (-10,-10) */
    C2D_DrawImageAt(img, locX + 10.0f * DIRECTOR_SCALE, locY + (10.0f - rise) * DIRECTOR_SCALE, 0.0f, &tint, DIRECTOR_SCALE, DIRECTOR_SCALE);
}

static void drawBuildPreview(AppState* app) {
    static const int outlineAssets[6] = {
        ASSET_BUILD_OUTLINE,
        ASSET_BUILD_OUTLINE_FRONT,
        ASSET_BUILD_OUTLINE_REAR,
        ASSET_BUILD_OUTLINE_L1,
        ASSET_BUILD_OUTLINE_L2,
        ASSET_BUILD_OUTLINE_R1
    };
    float outlineX;
    float outlineY;
    float ghostX;
    float ghostY;
    int iconAsset;
    int ghostAsset;
    C2D_ImageTint ghostTint;
    int i;
    if (!app->buildPreviewValid || app->armedPlan == WB_PLAN_NONE) {
        return;
    }
    {
        WBBuildingType buildBuilding = planTypeToBuilding(app->armedPlan);
        WBUnitType buildUnit = planTypeToUnit(app->armedPlan);
        if (buildBuilding != WB_BUILDING_NONE) {
            ghostAsset = buildingAssetId(buildBuilding, WB_DIR_RIGHT, 0);
        } else {
            WBResourcePile emptyPile;
            memset(&emptyPile, 0, sizeof(emptyPile));
            ghostAsset = unitStandAssetId(buildUnit, WB_DIR_RIGHT, false);
        }
        if (ghostAsset < 0) {
            ghostAsset = ASSET_VEHICLE_BUGGY_RIGHT;
        }
    }
    posToLoc(app, app->buildPreviewX, app->buildPreviewY, &ghostX, &ghostY);
    posToLoc(app, app->buildPreviewX - 1, app->buildPreviewY - 1, &outlineX, &outlineY);
    ghostX += 10.0f * DIRECTOR_SCALE;
    ghostY += 18.0f * DIRECTOR_SCALE;
    iconAsset = app->buildPreviewAllowed ? ASSET_BUILD_YES : ASSET_BUILD_NO;
    for (i = 0; i < 6; ++i) {
        int assetId = outlineAssets[i];
        drawAnchoredImage(getImage(app, assetId), outlineX, outlineY, DIRECTOR_SCALE, g_objectAnchors[assetId].x, g_objectAnchors[assetId].y);
    }
    drawAnchoredImage(getImage(app, ASSET_BUILD_OUTLINE_R2), outlineX, outlineY, DIRECTOR_SCALE, g_objectAnchors[ASSET_BUILD_OUTLINE_R2].x, g_objectAnchors[ASSET_BUILD_OUTLINE_R2].y);
    C2D_PlainImageTint(&ghostTint, C2D_Color32(0xFF, 0xFF, 0xFF, app->buildPreviewAllowed ? 0xBF : 0x66), 1.0f);
    C2D_DrawImageAt(getImage(app, ghostAsset), ghostX - (g_objectAnchors[ghostAsset].x * DIRECTOR_SCALE), ghostY - (g_objectAnchors[ghostAsset].y * DIRECTOR_SCALE), 0.0f, &ghostTint, DIRECTOR_SCALE, DIRECTOR_SCALE);
    drawAnchoredImage(getImage(app, iconAsset), outlineX + (40.0f * DIRECTOR_SCALE), outlineY + (75.0f * DIRECTOR_SCALE), DIRECTOR_SCALE, g_objectAnchors[iconAsset].x, g_objectAnchors[iconAsset].y);
}

static void drawMinimap(AppState* app, float centerX, float centerY, float scale) {
    float miniX = roundf(centerX - ((app->map.width * scale) * 0.5f));
    float miniY = roundf(centerY - ((app->map.height * scale) * 0.5f));
    int x;
    int y;
    for (y = 0; y < app->map.height; ++y) {
        for (x = 0; x < app->map.width; ++x) {
            const WBCell* cell = &app->map.cells[y][x];
            float px = miniX + (x * scale);
            float py = miniY + (y * scale);
            int miniTerrainAsset = minimapTerrainAssetId(cell->terrain, cell->terrainVariant);
            if (miniTerrainAsset >= 0) {
                C2D_DrawImageAt(getImage(app, miniTerrainAsset), px, py, 0.0f, NULL, scale / 3.0f, scale / 3.0f);
            }
            if (cell->hasGoal && (!cell->bonusGoal || app->map.bonusAvailable) && !cell->goalSatisfied) {
                C2D_DrawRectSolid(px, py, 0.0f, scale, scale, C2D_Color32(0xFF, 0xE0, 0x45, 0xFF));
            } else if (cell->hasUnit) {
                C2D_DrawImageAt(getImage(app, ASSET_MINI_VEHICLE), px, py, 0.0f, NULL, scale / 3.0f, scale / 3.0f);
            } else if (cell->hasResource) {
                C2D_DrawImageAt(getImage(app, cell->resourceIsPlan ? ASSET_MINI_PLAN : ASSET_MINI_RESOURCE), px, py, 0.0f, NULL, scale / 3.0f, scale / 3.0f);
            }
        }
    }

    for (y = 0; y < g_monsterCount; ++y) {
        const WBMonsterState* ms = &g_monsters[y];
        float px;
        float py;
        float dot = scale < 3.0f ? 1.0f : (scale * 0.35f);

        if (!ms->active || ms->dying || !inBounds(app, ms->col, ms->row)) {
            continue;
        }

        px = miniX + (ms->col * scale) + ((scale - dot) * 0.5f);
        py = miniY + (ms->row * scale) + ((scale - dot) * 0.5f);
        if (ms->type == WB_MONSTER_BOULDER) {
            C2D_DrawRectSolid(px, py, 0.0f, dot, dot, C2D_Color32(0x90, 0x90, 0x90, 0xFF));
        } else {
            C2D_DrawRectSolid(px, py, 0.0f, dot, dot, C2D_Color32(0xFF, 0x35, 0x35, 0xFF));
        }
    }

    C2D_DrawRectSolid(miniX + ((app->cameraX - 1) * scale), miniY + ((app->cameraY - 1) * scale), 0.0f, VIEW_TILE_W * scale, 1.0f, C2D_Color32(0xFF, 0xF2, 0x55, 0xFF));
    C2D_DrawRectSolid(miniX + ((app->cameraX - 1) * scale), miniY + ((app->cameraY - 1) * scale), 0.0f, 1.0f, VIEW_TILE_H * scale, C2D_Color32(0xFF, 0xF2, 0x55, 0xFF));
    C2D_DrawRectSolid(miniX + (((app->cameraX - 1) + VIEW_TILE_W) * scale) - 1.0f, miniY + ((app->cameraY - 1) * scale), 0.0f, 1.0f, VIEW_TILE_H * scale, C2D_Color32(0xFF, 0xF2, 0x55, 0xFF));
    C2D_DrawRectSolid(miniX + ((app->cameraX - 1) * scale), miniY + (((app->cameraY - 1) + VIEW_TILE_H) * scale) - 1.0f, 0.0f, VIEW_TILE_W * scale, 1.0f, C2D_Color32(0xFF, 0xF2, 0x55, 0xFF));
}

static float minimapScaleForMap(const AppState* app) {
    float preferred = 8.0f / 1.5f;
    float maxWidth = 132.0f;
    float maxHeight = 90.0f;
    float sx = maxWidth / (float) app->map.width;
    float sy = maxHeight / (float) app->map.height;
    float fit = sx < sy ? sx : sy;
    return fit < preferred ? fit : preferred;
}

/* Count how many lines drawWrappedText would produce for a given maxChars limit. */
static int countWrappedLines(const char* text, int maxChars) {
    int i = 0;
    int lineLen = 0;
    int lines = 0;
    if (text[0] == '\0') return 0;
    while (text[i] != '\0') {
        char word[64];
        int w = 0;
        while (text[i] == ' ') { ++i; }
        if (text[i] == '\n') {
            ++lines;
            lineLen = 0;
            ++i;
            continue;
        }
        while (text[i] != '\0' && text[i] != ' ' && text[i] != '\n' && w < (int)sizeof(word)-1) {
            word[w++] = text[i++];
        }
        word[w] = '\0';
        if (w == 0) continue;
        if (lineLen > 0 && (lineLen + 1 + w) > maxChars) {
            ++lines;
            lineLen = 0;
        }
        lineLen += (lineLen > 0 ? 1 : 0) + w;
    }
    if (lineLen > 0) ++lines;
    return lines;
}

static void drawWrappedText(C2D_TextBuf buf, float x, float y, float scale, const char* text, float lineStep, int maxChars) {
    char line[256];
    int i = 0;
    int lineLen = 0;
    line[0] = '\0';
    while (text[i] != '\0') {
        char word[64];
        int w = 0;
        while (text[i] == ' ') {
            ++i;
        }
        if (text[i] == '\n') {
            if (lineLen > 0) {
                drawTextLine(buf, x, y, scale, line);
                y += lineStep;
                line[0] = '\0';
                lineLen = 0;
            } else {
                y += lineStep;
            }
            ++i;
            continue;
        }
        while (text[i] != '\0' && text[i] != ' ' && text[i] != '\n' && w < (int) sizeof(word) - 1) {
            word[w++] = text[i++];
        }
        word[w] = '\0';
        if (w == 0) {
            continue;
        }
        if (lineLen > 0 && (lineLen + 1 + w) > maxChars) {
            drawTextLine(buf, x, y, scale, line);
            y += lineStep;
            line[0] = '\0';
            lineLen = 0;
        }
        if (lineLen > 0) {
            strncat(line, " ", sizeof(line) - strlen(line) - 1);
            lineLen += 1;
        }
        strncat(line, word, sizeof(line) - strlen(line) - 1);
        lineLen += w;
    }
    if (lineLen > 0) {
        drawTextLine(buf, x, y, scale, line);
    }
}

static void loadGameplaySheets(AppState* app) {
    if (!app->vehiclesLandSheet)   app->vehiclesLandSheet   = C2D_SpriteSheetLoad("romfs:/gfx/vehicles_land.t3x");
    if (!app->vehiclesWaterSheet)  app->vehiclesWaterSheet  = C2D_SpriteSheetLoad("romfs:/gfx/vehicles_water.t3x");
    if (!app->vehiclesAnimalSheet) app->vehiclesAnimalSheet = C2D_SpriteSheetLoad("romfs:/gfx/vehicles_animal.t3x");
    if (!app->vehiclesRobotSheet)  app->vehiclesRobotSheet  = C2D_SpriteSheetLoad("romfs:/gfx/vehicles_robot.t3x");
    if (!app->monstersASheet)      app->monstersASheet      = C2D_SpriteSheetLoad("romfs:/gfx/monsters_a.t3x");
    if (!app->monstersBSheet)      app->monstersBSheet      = C2D_SpriteSheetLoad("romfs:/gfx/monsters_b.t3x");
    if (!app->buildingsSheet)      app->buildingsSheet      = C2D_SpriteSheetLoad("romfs:/gfx/buildings.t3x");
    if (!app->whirlpoolSheet)      app->whirlpoolSheet      = C2D_SpriteSheetLoad("romfs:/gfx/whirlpool.t3x");
    if (!app->wb2MonstersSheet)    app->wb2MonstersSheet    = C2D_SpriteSheetLoad("romfs:/gfx/wb2_monsters.t3x");
    if (!app->wb2VehiclesSheet)    app->wb2VehiclesSheet    = C2D_SpriteSheetLoad("romfs:/gfx/wb2_vehicles.t3x");
    if (!app->wb2BuildingsSheet)   app->wb2BuildingsSheet   = C2D_SpriteSheetLoad("romfs:/gfx/wb2_buildings.t3x");
    if (!app->wb2TerrainSheet)     app->wb2TerrainSheet     = C2D_SpriteSheetLoad("romfs:/gfx/wb2_terrain.t3x");
    if (!app->wb2ResourcesSheet)   app->wb2ResourcesSheet   = C2D_SpriteSheetLoad("romfs:/gfx/wb2_resources.t3x");
}

static void freeGameplaySheets(AppState* app) {
    if (app->vehiclesLandSheet)   { C2D_SpriteSheetFree(app->vehiclesLandSheet);   app->vehiclesLandSheet   = NULL; }
    if (app->vehiclesWaterSheet)  { C2D_SpriteSheetFree(app->vehiclesWaterSheet);  app->vehiclesWaterSheet  = NULL; }
    if (app->vehiclesAnimalSheet) { C2D_SpriteSheetFree(app->vehiclesAnimalSheet); app->vehiclesAnimalSheet = NULL; }
    if (app->vehiclesRobotSheet)  { C2D_SpriteSheetFree(app->vehiclesRobotSheet);  app->vehiclesRobotSheet  = NULL; }
    if (app->monstersASheet)      { C2D_SpriteSheetFree(app->monstersASheet);      app->monstersASheet      = NULL; }
    if (app->monstersBSheet)      { C2D_SpriteSheetFree(app->monstersBSheet);      app->monstersBSheet      = NULL; }
    if (app->buildingsSheet)      { C2D_SpriteSheetFree(app->buildingsSheet);      app->buildingsSheet      = NULL; }
    if (app->whirlpoolSheet)      { C2D_SpriteSheetFree(app->whirlpoolSheet);      app->whirlpoolSheet      = NULL; }
    if (app->wb2MonstersSheet)    { C2D_SpriteSheetFree(app->wb2MonstersSheet);    app->wb2MonstersSheet    = NULL; }
    if (app->wb2VehiclesSheet)    { C2D_SpriteSheetFree(app->wb2VehiclesSheet);    app->wb2VehiclesSheet    = NULL; }
    if (app->wb2BuildingsSheet)   { C2D_SpriteSheetFree(app->wb2BuildingsSheet);   app->wb2BuildingsSheet   = NULL; }
    if (app->wb2TerrainSheet)     { C2D_SpriteSheetFree(app->wb2TerrainSheet);     app->wb2TerrainSheet     = NULL; }
    if (app->wb2ResourcesSheet)   { C2D_SpriteSheetFree(app->wb2ResourcesSheet);   app->wb2ResourcesSheet   = NULL; }
}

/* ---------------------------------------------------------------------------
   Per-level audio: scan the map and load only what the level actually needs.
   --------------------------------------------------------------------------- */

/* Bitmask of per-level audio requirements */
#define WBAUDIO_NEED_MONSTER_ATTACK   (1u <<  0)   /* any monster present */
#define WBAUDIO_NEED_VEHICLE          (1u <<  1)   /* buggy/dirtbuggy/speedboat/tugboat/freighter */
#define WBAUDIO_NEED_ANIMAL           (1u <<  2)   /* duck/frog/fish/snail */
#define WBAUDIO_NEED_BLDG_UNIT        (1u <<  3)   /* steamshovel/dumptruck/forklift/dozer selection sound */
#define WBAUDIO_NEED_DIG_GROUND       (1u <<  4)   /* steamshovel digs ground */
#define WBAUDIO_NEED_FILL_GROUND      (1u <<  5)   /* steamshovel fills ground */
#define WBAUDIO_NEED_DIG_TREE         (1u <<  6)   /* treebot uproots tree */
#define WBAUDIO_NEED_PLANT_TREE       (1u <<  7)   /* treebot plants tree */
#define WBAUDIO_NEED_ROBOT            (1u <<  8)   /* treebot/repairbot/defender/freezebot */
#define WBAUDIO_NEED_BLDG_GENERIC     (1u <<  9)   /* any building present */
#define WBAUDIO_NEED_BLDG_GUARD_TOWER (1u << 10)   /* guard tower */
#define WBAUDIO_NEED_BLDG_FACTORY     (1u << 11)   /* factory */
#define WBAUDIO_NEED_BLDG_GASMAR      (1u << 12)   /* gas station or marina */
#define WBAUDIO_NEED_BLDG_ROBOT_LAB   (1u << 13)   /* robot lab */

/* Return audio need bits for a single plan type. */
static unsigned int scanPlanAudioNeeds(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_DUCK:
        case WB_PLAN_FROG:
        case WB_PLAN_FISH:
        case WB_PLAN_SNAIL:
            return WBAUDIO_NEED_ANIMAL;
        case WB_PLAN_TREEBOT:
            return WBAUDIO_NEED_ROBOT | WBAUDIO_NEED_DIG_TREE | WBAUDIO_NEED_PLANT_TREE;
        case WB_PLAN_REPAIRBOT:
        case WB_PLAN_DEFENDER:
        case WB_PLAN_FREEZEBOT:
            return WBAUDIO_NEED_ROBOT;
        case WB_PLAN_STEAMSHOVEL:
            return WBAUDIO_NEED_VEHICLE | WBAUDIO_NEED_DIG_GROUND | WBAUDIO_NEED_FILL_GROUND;
        case WB_PLAN_DUMPTRUCK:
        case WB_PLAN_FORKLIFT:
        case WB_PLAN_DOZER:
            return WBAUDIO_NEED_VEHICLE;
        case WB_PLAN_FACTORY:
            return WBAUDIO_NEED_BLDG_GENERIC | WBAUDIO_NEED_BLDG_FACTORY;
        case WB_PLAN_GAS_STATION:
        case WB_PLAN_MARINA:
            return WBAUDIO_NEED_BLDG_GENERIC | WBAUDIO_NEED_BLDG_GASMAR;
        case WB_PLAN_GUARD_TOWER:
            return WBAUDIO_NEED_BLDG_GENERIC | WBAUDIO_NEED_BLDG_GUARD_TOWER;
        case WB_PLAN_ROBOT_LAB:
            return WBAUDIO_NEED_BLDG_GENERIC | WBAUDIO_NEED_BLDG_ROBOT_LAB;
        case WB_PLAN_HOUSE:
        case WB_PLAN_WINDMILL:
        case WB_PLAN_GARAGE:
        case WB_PLAN_NURSERY:
        case WB_PLAN_AIRPORT:
            return WBAUDIO_NEED_BLDG_GENERIC;
        default: /* all remaining are land/water vehicles */
            return WBAUDIO_NEED_VEHICLE;
    }
}

/* Walk every cell and inventory slot to determine which per-level SFX are needed. */
static unsigned int scanLevelAudioNeeds(const WBMap* map) {
    unsigned int needs = 0;
    int x, y, p;

    for (y = 0; y < map->height; ++y) {
        for (x = 0; x < map->width; ++x) {
            const WBCell* cell = &map->cells[y][x];

            if (cell->hasMonster) {
                needs |= WBAUDIO_NEED_MONSTER_ATTACK;
            }

            if (cell->hasUnit) {
                switch (cell->unitType) {
                    case WB_UNIT_DUCK:
                    case WB_UNIT_FROG:
                    case WB_UNIT_FISH:
                    case WB_UNIT_SNAIL:
                        needs |= WBAUDIO_NEED_ANIMAL;
                        break;
                    case WB_UNIT_TREEBOT:
                        needs |= WBAUDIO_NEED_ROBOT | WBAUDIO_NEED_DIG_TREE | WBAUDIO_NEED_PLANT_TREE;
                        break;
                    case WB_UNIT_REPAIRBOT:
                    case WB_UNIT_DEFENDER:
                    case WB_UNIT_DEFENDER2:
                    case WB_UNIT_FREEZEBOT:
                        needs |= WBAUDIO_NEED_ROBOT;
                        break;
                    case WB_UNIT_STEAMSHOVEL:
                        needs |= WBAUDIO_NEED_VEHICLE | WBAUDIO_NEED_DIG_GROUND | WBAUDIO_NEED_FILL_GROUND;
                        break;
                    case WB_UNIT_DUMPTRUCK:
                    case WB_UNIT_FORKLIFT:
                    case WB_UNIT_DOZER:
                        needs |= WBAUDIO_NEED_VEHICLE;
                        break;
                    default:
                        needs |= WBAUDIO_NEED_VEHICLE;
                        break;
                }
            }

            if (cell->hasBuilding) {
                needs |= WBAUDIO_NEED_BLDG_GENERIC;
                switch (cell->buildingType) {
                    case WB_BUILDING_FACTORY:
                        needs |= WBAUDIO_NEED_BLDG_FACTORY;
                        break;
                    case WB_BUILDING_GAS_STATION:
                    case WB_BUILDING_MARINA:
                        needs |= WBAUDIO_NEED_BLDG_GASMAR;
                        break;
                    case WB_BUILDING_GUARD_TOWER:
                        needs |= WBAUDIO_NEED_BLDG_GUARD_TOWER;
                        break;
                    case WB_BUILDING_ROBOT_LAB:
                        needs |= WBAUDIO_NEED_BLDG_ROBOT_LAB;
                        break;
                    default:
                        break;
                }
            }

            if (cell->resourceIsPlan) {
                needs |= scanPlanAudioNeeds(cell->resourcePlanType);
            }
        }
    }

    /* Plans in player's starting inventory */
    for (p = 0; p < WB_PLAN_COUNT; ++p) {
        if (map->planInventory[p] > 0) {
            needs |= scanPlanAudioNeeds((WBPlanType) p);
        }
    }

    return needs;
}

/* Free all per-level audio: unit/building SFX and game music variants. */
static void freeLevelAudio(AppState* app) {
    int i;
    /* Stop the SFX channel before freeing to avoid use-after-free in the DSP */
    if (app->audioReady) {
        ndspChnReset(SFX_CHANNEL);
        ndspChnWaveBufClear(SFX_CHANNEL);
    }
    freeWavClip(&app->sfxMonsterAttack);
    freeWavClip(&app->sfxUnitVehicle);
    freeWavClip(&app->sfxUnitAnimal);
    freeWavClip(&app->sfxDigGround);
    freeWavClip(&app->sfxFillGround);
    freeWavClip(&app->sfxDigTree);
    freeWavClip(&app->sfxPlantTree);
    freeWavClip(&app->sfxUnitRobot);
    freeWavClip(&app->sfxBldgGeneric);
    freeWavClip(&app->sfxBldgGuardTower);
    freeWavClip(&app->sfxBldgFactory);
    freeWavClip(&app->sfxBldgGasStationMarina);
    freeWavClip(&app->sfxBldgRobotLab);
    for (i = 0; i < WB_MUSIC_GAME6_VARIANTS; ++i) { freeWavClip(&app->musicGame6Variants[i]); }
    for (i = 0; i < WB_MUSIC_GAME8_VARIANTS; ++i) { freeWavClip(&app->musicGame8Variants[i]); }
    for (i = 0; i < WB_MUSIC_GAMEA_VARIANTS; ++i) { freeWavClip(&app->musicGameAVariants[i]); }
    for (i = 0; i < WB_MUSIC_GAMEI_VARIANTS; ++i) { freeWavClip(&app->musicGameIVariants[i]); }
}

/* Load per-level SFX based on need flags, then attempt music variants with remaining budget. */
static void loadLevelAudio(AppState* app, unsigned int needs) {
    /* Unit / entity SFX — in priority order (smallest files first within tier) */
    if (needs & WBAUDIO_NEED_MONSTER_ATTACK) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_monster_attack.wav", &app->sfxMonsterAttack) ? 1 : 0;
    }
    if (needs & WBAUDIO_NEED_VEHICLE) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_unit_vehicle.wav", &app->sfxUnitVehicle) ? 1 : 0;
    }
    if (needs & WBAUDIO_NEED_ANIMAL) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_unit_animal.wav", &app->sfxUnitAnimal) ? 1 : 0;
    }
    /* Terrain action SFX */
    if (needs & WBAUDIO_NEED_DIG_GROUND) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_dig_ground.wav", &app->sfxDigGround) ? 1 : 0;
    }
    if (needs & WBAUDIO_NEED_FILL_GROUND) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_fill_ground.wav", &app->sfxFillGround) ? 1 : 0;
    }
    if (needs & WBAUDIO_NEED_DIG_TREE) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_dig_tree.wav", &app->sfxDigTree) ? 1 : 0;
    }
    if (needs & WBAUDIO_NEED_PLANT_TREE) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_plant_tree.wav", &app->sfxPlantTree) ? 1 : 0;
    }
    /* Robot selection SFX */
    if (needs & WBAUDIO_NEED_ROBOT) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_unit_robot.wav", &app->sfxUnitRobot) ? 1 : 0;
    }
    /* Building SFX — generic loaded first (budget fallback), then specific sounds */
    if (needs & WBAUDIO_NEED_BLDG_GENERIC) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bldg_generic.wav", &app->sfxBldgGeneric) ? 1 : 0;
        app->sfxBldgGeneric.volume = 0.55f;
    }
    if (needs & WBAUDIO_NEED_BLDG_GUARD_TOWER) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bldg_guard_tower.wav", &app->sfxBldgGuardTower) ? 1 : 0;
        app->sfxBldgGuardTower.volume = 0.55f;
    }
    if (needs & WBAUDIO_NEED_BLDG_FACTORY) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bldg_factory.wav", &app->sfxBldgFactory) ? 1 : 0;
        app->sfxBldgFactory.volume = 0.55f;
    }
    if (needs & WBAUDIO_NEED_BLDG_GASMAR) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bldg_gas_station_marina.wav", &app->sfxBldgGasStationMarina) ? 1 : 0;
        app->sfxBldgGasStationMarina.volume = 0.55f;
    }
    if (needs & WBAUDIO_NEED_BLDG_ROBOT_LAB) {
        app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bldg_robot_lab.wav", &app->sfxBldgRobotLab) ? 1 : 0;
        app->sfxBldgRobotLab.volume = 0.55f;
    }
    /* Music variants — attempted after all SFX; gracefully skipped if heap is full.
       Ordered smallest variant first so partial loads remain musically useful. */
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_8_1.wav", &app->musicGame8Variants[0]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_8_4.wav", &app->musicGame8Variants[2]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_8_3.wav", &app->musicGame8Variants[1]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_a_1.wav", &app->musicGameAVariants[0]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_a_2.wav", &app->musicGameAVariants[1]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_a_3.wav", &app->musicGameAVariants[2]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_6_1.wav", &app->musicGame6Variants[0]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_6_3.wav", &app->musicGame6Variants[1]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_6_4.wav", &app->musicGame6Variants[2]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_6_5.wav", &app->musicGame6Variants[3]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_i_1.wav", &app->musicGameIVariants[0]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_i_2.wav", &app->musicGameIVariants[1]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game_i_3.wav", &app->musicGameIVariants[2]) ? 1 : 0;
    app->linearFreeKB = (int)(linearSpaceFree() / 1024);
}

static void loadMissionIntoGame(AppState* app, int world, int mission) {
    char path[64];
    snprintf(path, sizeof(path), "romfs:/maps/map%d_%d.txt", world, mission);
    if (!wbMapLoad(path, &app->map)) {
        appendDebugLog("  wbMapLoad FAILED");
        return;
    }
    {
        char _dbg[96];
        snprintf(_dbg, sizeof(_dbg), "  wbMapLoad OK world=%d mission=%d w=%d h=%d",
            world, mission, app->map.width, app->map.height);
        appendDebugLog(_dbg);
    }
    trimMenuSheetsForGameplay(app, world);
    loadGameplaySheets(app);
    app->screenMode = WB_SCREEN_GAME;
    app->activeWorld = world;
    app->activeMission = mission;
    resetCameraForMap(app);
    app->pileInspectValid = false;
    app->hasSelection = false;
    app->infoOverlayOpen = false;
    app->planMenuOpen = false;
    app->startMenuOpen = false;
    app->armedPlan = WB_PLAN_NONE;
    app->goalPopupVisible = false;
    app->goalPopupComplete = false;
    app->goalPopupBonus = false;
    app->planSwoop.active = false;
    app->actionMode = WB_ACTION_MOVE;
    memset(app->worldEffects, 0, sizeof(app->worldEffects));
    /* Scan audio needs BEFORE initWorldObjects clears cell->hasMonster flags. */
    {
        unsigned int needs = scanLevelAudioNeeds(&app->map);
        app->map.isWB2Level = (world >= 6);
        initWorldObjects(app);
        clampCamera(app);
        freeLevelAudio(app);
        loadLevelAudio(app, needs);
    }
    startGameMusic(app);
    app->tutorialActive = false;
    if (world == 1 && mission == 1) {
        /* 'mission icon behavior 2': golevel(1, 1), setTutorialMode(1), then
           reportSuccess(#goal), so the tutorial counts as done even if skipped. */
        tutorialStart(app);
        applyMissionSuccess(app, 1, 1, 1);
    }
}

/* The whole map bitmap is shown, scaled to fit the bottom screen and resting on its
   bottom edge, as the original shows all of it on the stage. The band left above it
   holds the world name and the world arrows, which in the original sit off the map.
   (Filling the screen instead cut 40-57 px off each side and pushed markers off it.) */
static void worldMapLayout(const AppState* app, int world, float* outX, float* outY, float* outScale) {
    float imageW;
    float imageH;
    float scaleX;
    float scaleY;
    float scale;
    (void) app;
    if (world < 1 || world > 7) {
        world = 1;
    }
    imageW = g_worldMapDefs[world].width;
    imageH = g_worldMapDefs[world].height;
    scaleX = BOTTOM_W / imageW;
    scaleY = BOTTOM_H / imageH;
    scale = scaleX < scaleY ? scaleX : scaleY;
    *outScale = scale;
    *outX = (BOTTOM_W - (imageW * scale)) * 0.5f;
    *outY = BOTTOM_H - (imageH * scale);
}

static void missionPointToBottom(const AppState* app, int world, int mission, float* outX, float* outY) {
    float mapX;
    float mapY;
    float scale;
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    *outX = mapX + (g_worldMissionPoints[world][mission].x * scale);
    *outY = mapY + (g_worldMissionPoints[world][mission].y * scale);
}

static int missionFlagAsset(int world, int frame, bool bonus) {
    static const int landFlags[6] = {0, ASSET_WORLD_FLAG1, ASSET_WORLD_FLAG2, ASSET_WORLD_FLAG3, ASSET_WORLD_FLAG4, ASSET_WORLD_FLAG5};
    static const int landFlags2[7] = {0, ASSET_WORLD_FLAG1, ASSET_WORLD_FLAG2, ASSET_WORLD_FLAG3, ASSET_WORLD_FLAG4, ASSET_WORLD_FLAG5, ASSET_WORLD_FLAG6};
    static const int landBonus[7] = {0, ASSET_WORLD_BONUS_FLAG1, ASSET_WORLD_BONUS_FLAG2, ASSET_WORLD_BONUS_FLAG3, ASSET_WORLD_BONUS_FLAG4, ASSET_WORLD_BONUS_FLAG5, ASSET_WORLD_BONUS_FLAG6};
    static const int oceanFlags[7] = {0, ASSET_OCEAN_FLAG1, ASSET_OCEAN_FLAG2, ASSET_OCEAN_FLAG3, ASSET_OCEAN_FLAG4, ASSET_OCEAN_FLAG5, ASSET_OCEAN_FLAG6};
    static const int oceanBonus[7] = {0, ASSET_OCEAN_BONUS_FLAG1, ASSET_OCEAN_BONUS_FLAG2, ASSET_OCEAN_BONUS_FLAG3, ASSET_OCEAN_BONUS_FLAG4, ASSET_OCEAN_BONUS_FLAG5, ASSET_OCEAN_BONUS_FLAG6};
    (void) landFlags;
    if (frame < 1) {
        frame = 1;
    }
    if (frame > 6) {
        frame = 6;
    }
    if (world == 4) {
        return bonus ? oceanBonus[frame] : oceanFlags[frame];
    }
    return bonus ? landBonus[frame] : landFlags2[frame];
}

/* World-map frame rate: the movie has no tempo channel, so it runs at the 15 fps
   in its DRCF; 'mission icon behavior 2' and 'mini-unit behavior' step per frame. */
#define WB_WORLDMAP_FRAME_MS (1000.0f / 15.0f)

static void loadWorldWalkMask(AppState* app, int world) {
    char path[48];
    u8 header[8];
    FILE* fp;
    if (app->walkMask && app->walkMaskWorld == world) {
        return;
    }
    free(app->walkMask);
    app->walkMask = NULL;
    app->walkMaskWorld = 0;
    app->walkMaskW = 0;
    app->walkMaskH = 0;
    snprintf(path, sizeof(path), "romfs:/meta/worldmask_w%d.bin", world);
    fp = fopen(path, "rb");
    if (!fp) {
        return;
    }
    if (fread(header, 1, sizeof(header), fp) == sizeof(header) && memcmp(header, "WBMK", 4) == 0) {
        int w = header[4] | (header[5] << 8);
        int h = header[6] | (header[7] << 8);
        size_t bytes = (((size_t) w * (size_t) h) + 3) / 4;
        app->walkMask = (u8*) malloc(bytes);
        if (app->walkMask && fread(app->walkMask, 1, bytes, fp) == bytes) {
            app->walkMaskW = w;
            app->walkMaskH = h;
            app->walkMaskWorld = world;
        } else {
            free(app->walkMask);
            app->walkMask = NULL;
        }
    }
    fclose(fp);
}

/* bit0: a land mini may stand here, bit1: a lake mini may. Off the map counts as
   neither (getPixel returns 0 there), except where okPos() sets no terrain rule. */
static int walkMaskAt(const AppState* app, float x, float y, bool unbounded) {
    int ix;
    int iy;
    size_t i;
    if (!app->walkMask) {
        return 3;
    }
    if (x < 0.0f || y < 0.0f) {
        return unbounded ? 3 : 0;
    }
    ix = (int) x;
    iy = (int) y;
    if (ix >= app->walkMaskW || iy >= app->walkMaskH) {
        return unbounded ? 3 : 0;
    }
    i = ((size_t) iy * (size_t) app->walkMaskW) + (size_t) ix;
    return (app->walkMask[i >> 2] >> ((i & 3) * 2)) & 3;
}

/* okPos() from 'mini-unit behavior': all four points 7 px around the new position
   must be clear of the mini-obstacles and on the unit's kind of ground. */
static bool miniOkPos(const AppState* app, int world, const WBMiniDef* def, float px, float py) {
    static const int offsets[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    const WBWorldMapDef* wd = &g_worldMapDefs[world];
    int need = def->lake ? 2 : 1;
    int k;
    if (wd->radiusLimited) {
        float dx = def->x - px;
        float dy = def->y - py;
        if ((dx * dx) + (dy * dy) > 380.0f) {
            return false;
        }
    }
    for (k = 0; k < 4; ++k) {
        float x = px + (float) (offsets[k][0] * 7);
        float y = py + (float) (offsets[k][1] * 7);
        if (wd->obstaclesActive) {
            int j;
            for (j = 0; j < wd->obstacleCount; ++j) {
                const WBObstacleRect* r = &wd->obstacles[j];
                if (x >= r->left && x < r->right && y >= r->top && y < r->bottom) {
                    return false;
                }
            }
        }
        if (!(walkMaskAt(app, x, y, wd->radiusLimited) & need)) {
            return false;
        }
    }
    return true;
}

static void miniRandomDir(WBMiniWalker* walker) {
    switch (randRange(1, 4)) {
        case 1: walker->dirX = -1; walker->dirY = 0; break;
        case 2: walker->dirX = 1; walker->dirY = 0; break;
        case 3: walker->dirX = 0; walker->dirY = -1; break;
        default: walker->dirX = 0; walker->dirY = 1; break;
    }
}

static bool miniWalkerVisible(const AppState* app, int world, const WBMiniDef* def) {
    int state = worldMissionState(app, world, def->mission);
    return !(state < 1 || (def->bonus && state < 3));
}

/* beginSprite for the world frame: every mini back at its score position, every
   mission icon given its random(6) starting flag frame. */
static void initWorldMiniWalkers(AppState* app, int world) {
    const WBWorldMapDef* wd;
    int i;
    memset(app->miniWalkers, 0, sizeof(app->miniWalkers));
    app->miniWalkerWorld = world;
    app->worldSelectLastMs = osGetTime();
    app->worldSelectTickAccum = 0.0f;
    if (world < 1 || world > 7) {
        return;
    }
    wd = &g_worldMapDefs[world];
    for (i = 0; i < wd->miniCount && i < WB_MAX_WORLD_MINIS; ++i) {
        WBMiniWalker* walker = &app->miniWalkers[i];
        walker->active = true;
        walker->world = world;
        walker->def = i;
        walker->x = wd->minis[i].x;
        walker->y = wd->minis[i].y;
    }
    for (i = 1; i <= 12; ++i) {
        app->missionFlagFrame[i] = (u8) randRange(1, 6);
    }
    loadWorldWalkMask(app, world);
}

static void ensureWorldMiniWalkers(AppState* app) {
    if (app->screenMode != WB_SCREEN_WORLD_SELECT) {
        return;
    }
    if (app->miniWalkerWorld != app->worldSelectWorld) {
        initWorldMiniWalkers(app, app->worldSelectWorld);
    }
}

/* One 15 fps frame of the world map: prepareFrame of every mission icon and mini. */
static void stepWorldSelectFrame(AppState* app) {
    int world = app->miniWalkerWorld;
    const WBWorldMapDef* wd;
    int i;
    if (world < 1 || world > 7) {
        return;
    }
    for (i = 1; i <= 12; ++i) {
        int state = worldMissionState(app, world, i);
        if (state >= 1 && state <= 3) {
            if (app->missionFlagFrame[i] <= 1) {
                app->missionFlagFrame[i] = 6;
            } else {
                app->missionFlagFrame[i]--;
            }
        }
    }
    wd = &g_worldMapDefs[world];
    for (i = 0; i < wd->miniCount && i < WB_MAX_WORLD_MINIS; ++i) {
        WBMiniWalker* walker = &app->miniWalkers[i];
        const WBMiniDef* def = &wd->minis[i];
        if (!walker->active || !miniWalkerVisible(app, world, def)) {
            continue;
        }
        if (walker->dirX == 0 && walker->dirY == 0) {
            walker->x = def->x;
            walker->y = def->y;
            miniRandomDir(walker);
        }
        if (!walker->stopped) {
            float skwX = (29.0f * (float) walker->dirX) + (-35.0f * (float) walker->dirY);
            float skwY = (-25.0f * (float) walker->dirX) + (-13.0f * (float) walker->dirY);
            float nextX = walker->x + (skwX / 40.0f);
            float nextY = walker->y + (skwY / 40.0f);
            if (miniOkPos(app, world, def, nextX, nextY)) {
                walker->x = nextX;
                walker->y = nextY;
            } else {
                miniRandomDir(walker);
            }
            walker->flipH = skwX < 0.0f;
            if (randRange(1, 80) == 1) {
                walker->stopped = true;
            }
        } else {
            if (randRange(1, 30) == 1) {
                walker->stopped = false;
            }
            if (randRange(1, 2) == 1) {
                miniRandomDir(walker);
            }
        }
    }
}

static void updateWorldMiniWalkers(AppState* app) {
    u64 now = osGetTime();
    int steps = 0;
    ensureWorldMiniWalkers(app);
    if (app->screenMode != WB_SCREEN_WORLD_SELECT || app->worldSelectLastMs == 0 || now < app->worldSelectLastMs) {
        app->worldSelectLastMs = now;
        return;
    }
    app->worldSelectTickAccum += (float) (now - app->worldSelectLastMs);
    app->worldSelectLastMs = now;
    while (app->worldSelectTickAccum >= WB_WORLDMAP_FRAME_MS) {
        app->worldSelectTickAccum -= WB_WORLDMAP_FRAME_MS;
        stepWorldSelectFrame(app);
        if (++steps >= 4) {
            app->worldSelectTickAccum = 0.0f;
            break;
        }
    }
}

static void drawDebugMenu(AppState* app) {
    static const char* items[] = {
        "Clear Save Data",
        "Unlock All Levels",
        "Toggle Debug Info",
        "Close Menu"
    };
    int i;
    int count = 4;
    float bx = 60.0f, by = 60.0f, bw = 280.0f, bh = 130.0f;
    C2D_DrawRectSolid(bx, by, 0.2f, bw, bh, C2D_Color32(0x00, 0x00, 0x00, 0xCC));
    C2D_DrawRectSolid(bx, by, 0.2f, bw, 2.0f, C2D_Color32(0xF0, 0xD2, 0x38, 0xFF));
    C2D_DrawRectSolid(bx, by + bh - 2.0f, 0.2f, bw, 2.0f, C2D_Color32(0xF0, 0xD2, 0x38, 0xFF));
    C2D_DrawRectSolid(bx, by, 0.2f, 2.0f, bh, C2D_Color32(0xF0, 0xD2, 0x38, 0xFF));
    C2D_DrawRectSolid(bx + bw - 2.0f, by, 0.2f, 2.0f, bh, C2D_Color32(0xF0, 0xD2, 0x38, 0xFF));
    drawTextLine(app->dynamicBuf, bx + 8.0f, by + 6.0f, 0.38f, "Debug Menu  (A=select  B=close)");
    for (i = 0; i < count; ++i) {
        float iy = by + 30.0f + (i * 22.0f);
        if (i == app->debugMenuCursor) {
            C2D_DrawRectSolid(bx + 4.0f, iy - 2.0f, 0.2f, bw - 8.0f, 18.0f, C2D_Color32(0xDA, 0x7D, 0x00, 0xCC));
            drawTextLine(app->dynamicBuf, bx + 14.0f, iy, 0.38f, items[i]);
        } else {
            drawTextLine(app->dynamicBuf, bx + 14.0f, iy, 0.38f, items[i]);
        }
    }
}

static void drawTitleBottom(AppState* app) {
    u64 now = osGetTime();
    u64 halfCycle = 15000ULL;   /* 15 s per title */
    u64 fadeMs    = 500ULL;     /* 0.5 s fade-through-black at each transition */
    u64 elapsed   = (app->titleCycleStartMs != 0) ? (now - app->titleCycleStartMs) : 0ULL;
    u64 which     = (elapsed / halfCycle) % 2; /* 0 = WB1, 1 = WB2 */
    u64 phase     = elapsed % halfCycle;
    u8  blackAlpha = 0;

    if (phase >= halfCycle - fadeMs) {
        /* Fading out current image towards black */
        blackAlpha = (u8)((phase - (halfCycle - fadeMs)) * 255ULL / fadeMs);
    } else if (phase < fadeMs) {
        /* Fading in from black */
        blackAlpha = (u8)((fadeMs - phase) * 255ULL / fadeMs);
    }

    if (which == 0) {
        /* WB1 title */
        C2D_Image splash = getImage(app, ASSET_TITLE_FINAL_IMAGE);
        C2D_Image logo   = getImage(app, ASSET_TITLE_LOGO);
        float splashScale = 0.50f;
        float logoScale   = 0.88f;
        drawAnchoredImage(splash, 160.0f, 154.0f, splashScale, 302.0f, 178.0f);
        drawAnchoredImage(logo,   160.0f,  36.0f, logoScale,   168.0f,  28.0f);
        C2D_DrawRectSolid(214.0f, 102.0f, 0.0f, 64.0f, 14.0f, C2D_Color32(0xDA, 0x7D, 0x00, 0xFF));
        C2D_DrawRectSolid(214.0f, 102.0f, 0.0f, 64.0f,  2.0f, C2D_Color32(0xF0, 0xD2, 0x38, 0xFF));
        drawTextLine(app->dynamicBuf, 220.0f, 105.0f, 0.31f, "Touch to Start");
    } else {
        /* WB2 title — logo and START button are baked into the sprite; stretch to fill screen */
        C2D_Image splash2 = getImage(app, ASSET_TITLE_WB2_IMAGE);
        if (splash2.subtex) {
            float scaleX = BOTTOM_W / (float)splash2.subtex->width;
            float scaleY = BOTTOM_H / (float)splash2.subtex->height;
            C2D_DrawImageAt(splash2, 0.0f, 0.0f, 0.0f, NULL, scaleX, scaleY);
        }
    }

    /* Fade-through-black overlay */
    if (blackAlpha > 0) {
        C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 320.0f, 240.0f, C2D_Color32(0, 0, 0, blackAlpha));
    }
}

/* Returns true if all missions in the given world have been completed (at least one must exist). */
static bool worldCompleteP(const AppState* app, int world) {
    bool hasAny = false;
    int m;
    for (m = 1; m <= 12; ++m) {
        int st = app->levelState[world][m];
        if (st < 0) continue;
        hasAny = true;
        if (st < 1) return false;
    }
    return hasAny;
}

/* Returns true if the Builder's License is accessible.
   Original requirement: worldCompleteP(3) — all missions in WB1 world 3 done. */
static bool licenseUnlocked(const AppState* app) {
    return worldCompleteP(app, 3);
}

/* Returns true if the WB2 Builder's License is accessible.
   WB2 original: all missions in WB2 world 1 (our world 6) done. */
static bool wb2LicenseUnlocked(const AppState* app) {
    return worldCompleteP(app, 6);
}

/* Returns WB2 license class 1-3 (A/B/C) based on bonus missions completed in worlds 6-7.
   Original thresholds (countBonuses over WB2 worlds): 0-11=A(1), 12-23=B(2), 24+=C(3).
   Note: WB2 class A is the best (most bonuses), C is the minimum (all missions done). */
static int wb2LicenseClass(const AppState* app) {
    int count = 0;
    int w, m;
    for (w = 6; w <= 7; ++w) {
        for (m = 1; m <= 12; ++m) {
            if (app->levelState[w][m] >= 2) ++count;
        }
    }
    if (count < 12) return 1;  /* Class A */
    if (count < 24) return 2;  /* Class B */
    return 3;                  /* Class C */
}

/* Returns license class 1-4 based on bonus missions completed.
   Original thresholds: 0-11 = class1, 12-23 = class2, 24-35 = class3, 36+ = class4.
   Only WB1 worlds (1-5) count, matching the original worlds_manager.countBonuses(). */
static int licenseClass(const AppState* app) {
    int count = 0;
    int w, m;
    for (w = 1; w <= 5; ++w) {
        for (m = 1; m <= 12; ++m) {
            if (app->levelState[w][m] >= 2) ++count;
        }
    }
    if (count < 12) return 1;
    if (count < 24) return 2;
    if (count < 36) return 3;
    return 4;
}

static void drawWorldSelectTop(AppState* app) {
    /* Ensure license sheet is loaded regardless of how world-select was entered */
    if (!app->licenseSheet)
        app->licenseSheet = C2D_SpriteSheetLoad("romfs:/gfx/license.t3x");
    C2D_Image sky = getImage(app, worldSkyAssetId(app->worldSelectWorld));
    float scale = app->worldSelectWorld == 1 ? 0.66f : 0.72f;
    float skyW = imageWidth(sky, scale);
    float skyH = imageHeight(sky, scale);
    float y;
    float x;
    for (y = 0.0f; y < TOP_H + skyH; y += skyH) {
        for (x = 0.0f; x < TOP_W + skyW; x += skyW) {
            C2D_DrawImageAt(sky, x, y, 0.0f, NULL, scale, scale);
        }
    }

    if (app->showDiagnostics) {
        C2D_Image mapImage = getImage(app, worldMapAssetId(app->worldSelectWorld));
        float mapX, mapY, mapScale;
        char hdr[80];
        int m;
        float lineY = 4.0f;
        float lineH = 12.0f;
        float panelW = 396.0f;

        worldMapLayout(app, app->worldSelectWorld, &mapX, &mapY, &mapScale);

        /* semi-transparent dark panel behind text */
        C2D_DrawRectSolid(2.0f, 2.0f, 0.1f, panelW, TOP_H - 4.0f,
            C2D_Color32(0x00, 0x00, 0x00, 0xB0));

        /* Header: world, image size, layout */
        snprintf(hdr, sizeof(hdr),
            "World %d  imgW=%.0f imgH=%.0f  scale=%.3f  mapX=%.1f mapY=%.1f",
            app->worldSelectWorld,
            imageWidth(mapImage, 1.0f), imageHeight(mapImage, 1.0f),
            mapScale, mapX, mapY);
        drawTextLineWhite(app->dynamicBuf, 6.0f, lineY, 0.35f, hdr);
        lineY += lineH;

        /* Two columns of 6 missions each */
        for (m = 1; m <= 12; ++m) {
            int st = worldMissionState(app, app->worldSelectWorld, m);
            float px, py;
            char mline[80];
            const char* stStr =
                st < -1  ? "???" :
                st == -1 ? "locked" :
                st == 0  ? "open" :
                st == 1  ? "done" :
                st == 3  ? "bonus" : "?";
            missionPointToBottom(app, app->worldSelectWorld, m, &px, &py);
            snprintf(mline, sizeof(mline),
                "M%02d [%s] pt=(%.0f,%.0f)  %s",
                m, stStr, px, py,
                app->missionNames[app->worldSelectWorld][m][0]
                    ? app->missionNames[app->worldSelectWorld][m]
                    : "-");
            {
                float col = (m <= 6) ? 6.0f : 200.0f;
                float row = ((m - 1) % 6) * lineH + lineY;
                drawTextLineWhite(app->dynamicBuf, col, row, 0.32f, mline);
            }
        }
    }

    /* Builder's License overlay — shown when SELECT is toggled and license is unlocked.
       WB1 card shown for worlds 1-5; WB2 card shown for worlds 6-7. */
    if (app->licenseVisible && app->worldSelectWorld < 6 && licenseUnlocked(app) && app->licenseSheet) {
        int cls = licenseClass(app);
        C2D_Image card = getImage(app, ASSET_LICENSE_CLASS1 + (cls - 1));
        float cardW = imageWidth(card, 1.0f);
        float cardH = imageHeight(card, 1.0f);
        /* Centre on the 400x240 top screen with a slight scale-up (1.25x) */
        float scale = 1.25f;
        float drawW = cardW * scale;
        float drawH = cardH * scale;
        float cx = (TOP_W - drawW) * 0.5f;
        float cy = (TOP_H - drawH) * 0.5f;
        /* Semi-transparent dark backdrop */
        C2D_DrawRectSolid(0.0f, 0.0f, 0.15f, (float)TOP_W, (float)TOP_H,
            C2D_Color32(0x00, 0x00, 0x00, 0xA0));
        C2D_DrawImageAt(card, cx, cy, 0.2f, NULL, scale, scale);
        /* World-completion stickers (ocean=world4, prehistoric=world5).
           The card PNGs are pre-downscaled by 192/275 ≈ 0.698 from the original
           275×175 Director art; the sticker PNGs are still at original resolution
           (47×47).  Apply the same pre-scale factor so the stickers are proportional
           to the displayed card.
           Positions are sticker-centre offsets into the 192×122 card image space,
           matching the lower-left area where the class-colour blocks appear.
           world4 (ocean) ≈ block 2, world5 (prehistoric) ≈ block 4. */
        {
            /* stickerScale: card PNGs are pre-downscaled to 192/275 of the original;
               sticker PNGs are at original resolution.  Apply only the downscale
               factor so the sticker is proportional to the card's source artwork.
               The screen-space 1.25× display zoom is already baked into cx/cy offsets. */
            float stickerScale = 192.0f / 275.0f;  /* ≈ 0.698 */
            if (worldCompleteP(app, 4)) {
                C2D_Image s = getImage(app, ASSET_LICENSE_STICKER_W4);
                if (s.subtex) {
                    float sw = imageWidth(s, stickerScale);
                    float sh = imageHeight(s, stickerScale);
                    float sx = cx + 28.0f * scale - sw * 0.5f;
                    float sy = cy + 88.0f * scale - sh * 0.5f;
                    C2D_DrawImageAt(s, sx, sy, 0.25f, NULL, stickerScale, stickerScale);
                }
            }
            if (worldCompleteP(app, 5)) {
                C2D_Image s = getImage(app, ASSET_LICENSE_STICKER_W5);
                if (s.subtex) {
                    float sw = imageWidth(s, stickerScale);
                    float sh = imageHeight(s, stickerScale);
                    float sx = cx + 63.0f * scale - sw * 0.5f;
                    float sy = cy + 88.0f * scale - sh * 0.5f;
                    C2D_DrawImageAt(s, sx, sy, 0.25f, NULL, stickerScale, stickerScale);
                }
            }
        }
        /* Hint label */
        drawTextLine(app->dynamicBuf, 6.0f, 228.0f, 0.34f, "SELECT: hide license");
    } else if (app->licenseVisible && app->worldSelectWorld >= 6 && wb2LicenseUnlocked(app) && app->licenseSheet) {
        /* WB2 Builder's License card — front only, no class art variants on the card image. */
        C2D_Image card = getImage(app, ASSET_WB2_LICENSE_FRONT);
        float cardW = imageWidth(card, 1.0f);
        float cardH = imageHeight(card, 1.0f);
        float scale = 1.25f;
        float drawW = cardW * scale;
        float drawH = cardH * scale;
        float cx = (TOP_W - drawW) * 0.5f;
        float cy = (TOP_H - drawH) * 0.5f;
        /* Semi-transparent dark backdrop */
        C2D_DrawRectSolid(0.0f, 0.0f, 0.15f, (float)TOP_W, (float)TOP_H,
            C2D_Color32(0x00, 0x00, 0x00, 0xA0));
        C2D_DrawImageAt(card, cx, cy, 0.2f, NULL, scale, scale);
        /* Reward class label overlay (wb2_reward_class1/2/3) centred near the card bottom. */
        {
            int cls = wb2LicenseClass(app);
            C2D_Image lbl = getImage(app, ASSET_WB2_REWARD_CLASS1 + (cls - 1));
            if (lbl.subtex) {
                float lblW = imageWidth(lbl, scale);
                float lblH = imageHeight(lbl, scale);
                float lx = cx + (drawW - lblW) * 0.5f;
                float ly = cy + drawH - lblH - 4.0f * scale;
                C2D_DrawImageAt(lbl, lx, ly, 0.25f, NULL, scale, scale);
            }
        }
        drawTextLine(app->dynamicBuf, 6.0f, 228.0f, 0.34f, "SELECT: hide license");
    }
}

static void drawWorldSelectSkyBackdrop(AppState* app, float surfaceW, float surfaceH) {
    C2D_Image sky = getImage(app, worldSkyAssetId(app->worldSelectWorld));
    float scale = app->worldSelectWorld == 1 ? 0.66f : 0.72f;
    float skyW = imageWidth(sky, scale);
    float skyH = imageHeight(sky, scale);
    float y;
    float x;
    for (y = 0.0f; y < surfaceH + skyH; y += skyH) {
        for (x = 0.0f; x < surfaceW + skyW; x += skyW) {
            C2D_DrawImageAt(sky, x, y, 0.0f, NULL, scale, scale);
        }
    }
}

/* World arrows and title sit in the band above the map (see worldMapLayout). */
#define WB_WORLD_ARROW_Y 16.0f
#define WB_WORLD_PREV_ARROW_X 18.0f
#define WB_WORLD_NEXT_ARROW_X 302.0f

/* 'mission icon behavior 2'. Icons are drawn 1:1 with the map, as on the stage.
   State 0 bobs round its rest point, 2 stage px over 1800 ms, phase flagframe/6, with
   the shadow following only the horizontal part; states 1-3 show the flag frame that
   stepWorldSelectFrame() counts down. Hovering blinks the icon every 200 ms. */
static void drawWorldMissionIcons(AppState* app) {
    int world = app->worldSelectWorld;
    u64 now = osGetTime();
    bool blinkOn = ((now / 200ULL) % 2ULL) != 0;
    float mapX;
    float mapY;
    float scale;
    int mission;
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    for (mission = 1; mission <= 12; ++mission) {
        int state = worldMissionState(app, world, mission);
        bool hovered = app->hoverMissionLevel == mission;
        int frame = app->missionFlagFrame[mission];
        float x;
        float y;
        if (state < 0) {
            continue;
        }
        if (frame < 1 || frame > 6) {
            frame = 1;
        }
        missionPointToBottom(app, world, mission, &x, &y);
        if (state == 0) {
            int shadowAsset = world == 4 ? ASSET_OCEAN_QUESTION_MARK_SHADOW : ASSET_WORLD_QUESTION_MARK_SHADOW;
            float t = 2.0f * 3.14159265f * (((float) frame / 6.0f) + ((float) (now % 1800ULL) / 1800.0f));
            float bobX = 2.0f * cosf(t) * scale;
            float bobY = 2.0f * sinf(t) * scale;
            drawAnchoredImage(getImage(app, shadowAsset), x + bobX, y, scale, 15.0f, world == 4 ? -19.0f : -17.0f);
            if (hovered && blinkOn) {
                drawAnchoredImage(getImage(app, ASSET_QUESTION_MARK_BLINK), x + bobX, y + bobY, scale, 32.0f, 27.0f);
            } else {
                drawAnchoredImage(getImage(app, ASSET_WORLD_QUESTION_MARK), x + bobX, y + bobY, scale, 18.0f, 12.0f);
            }
        } else {
            drawAnchoredImage(getImage(app, missionFlagAsset(world, frame, state == 3)), x, y, scale, 17.0f, 15.0f);
            if (hovered && blinkOn) {
                drawAnchoredImage(getImage(app, ASSET_FLAG_ROLLOVER_BLINK), x, y, scale, 27.0f, 31.0f);
            }
        }
    }
}

/* The icon's on-screen rect as the player sees it (question mark or flag), for touch. */
static void worldMissionIconRect(const AppState* app, int world, int mission, float* l, float* t, float* r, float* b) {
    float mapX;
    float mapY;
    float scale;
    float x;
    float y;
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    missionPointToBottom(app, world, mission, &x, &y);
    if (worldMissionState(app, world, mission) == 0) {
        *l = x - 18.0f * scale; *t = y - 12.0f * scale; *r = *l + 20.0f * scale; *b = *t + 27.0f * scale;
    } else {
        *l = x - 17.0f * scale; *t = y - 15.0f * scale; *r = *l + 33.0f * scale; *b = *t + 31.0f * scale;
    }
}

/* 'mission name rollover text': "MISSION n" over the mission's name, centred under
   the icon (member regpoint 65,-22 on a 124 px box). The original's 6 pt pixel font
   would be unreadable at this scale, so it is drawn larger, with a shadow. */
static void drawWorldMissionRollover(AppState* app) {
    int world = app->worldSelectWorld;
    int mission = app->hoverMissionLevel;
    char text[WB_MAX_NAME + 16];
    char name[WB_MAX_NAME];
    C2D_Text label;
    float mapX;
    float mapY;
    float scale;
    float x;
    float y;
    float w;
    float h;
    float cx;
    float top;
    const float textScale = 0.42f;
    int i;
    if (mission < 1 || mission > 12 || worldMissionState(app, world, mission) < 0) {
        return;
    }
    for (i = 0; i < WB_MAX_NAME - 1 && app->missionNames[world][mission][i]; ++i) {
        char c = app->missionNames[world][mission][i];
        name[i] = (c >= 'a' && c <= 'z') ? (char) (c - 'a' + 'A') : c;
    }
    name[i] = '\0';
    if (name[0]) {
        snprintf(text, sizeof(text), "MISSION %d\n%s", mission, name);
    } else {
        snprintf(text, sizeof(text), "MISSION %d", mission);
    }
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    missionPointToBottom(app, world, mission, &x, &y);
    C2D_TextParse(&label, app->dynamicBuf, text);
    C2D_TextOptimize(&label);
    C2D_TextGetDimensions(&label, textScale, textScale, &w, &h);
    cx = x - 3.0f * scale;
    top = y + 22.0f * scale;
    if (cx - w * 0.5f < 2.0f) cx = 2.0f + w * 0.5f;
    if (cx + w * 0.5f > BOTTOM_W - 2.0f) cx = BOTTOM_W - 2.0f - w * 0.5f;
    if (top + h > BOTTOM_H - 2.0f) top = BOTTOM_H - 2.0f - h;
    C2D_DrawText(&label, C2D_WithColor | C2D_AlignCenter, cx + 1.0f, top + 1.0f, 0.5f, textScale, textScale,
        C2D_Color32(0x00, 0x00, 0x00, 0xC0));
    C2D_DrawText(&label, C2D_WithColor | C2D_AlignCenter, cx, top, 0.5f, textScale, textScale,
        C2D_Color32(0xEE, 0xEE, 0xEE, 0xFF));
}

static void drawWorldMiniWalkers(AppState* app) {
    int world = app->worldSelectWorld;
    const WBWorldMapDef* wd;
    float mapX;
    float mapY;
    float scale;
    int i;
    if (world < 1 || world > 7 || app->miniWalkerWorld != world) {
        return;
    }
    wd = &g_worldMapDefs[world];
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    for (i = 0; i < wd->miniCount && i < WB_MAX_WORLD_MINIS; ++i) {
        const WBMiniWalker* walker = &app->miniWalkers[i];
        const WBMiniDef* def = &wd->minis[i];
        C2D_Image img;
        float sx;
        float sy;
        if (!walker->active || !miniWalkerVisible(app, world, def)) {
            continue;
        }
        img = getImage(app, def->assetId);
        if (!img.subtex) {
            continue;
        }
        sx = mapX + walker->x * scale;
        sy = mapY + walker->y * scale;
        if (walker->flipH) {
            /* flipH mirrors about the registration point */
            C2D_DrawImageAt(img, sx + def->anchorX * scale, sy - def->anchorY * scale, 0.0f, NULL, -scale, scale);
        } else {
            drawAnchoredImage(img, sx, sy, scale, def->anchorX, def->anchorY);
        }
    }
}

static void drawWorldMapDecorations(AppState* app) {
    int world = app->worldSelectWorld;
    const WBWorldMapDef* wd;
    float mapX;
    float mapY;
    float scale;
    int i;
    if (world < 1 || world > 7) {
        return;
    }
    wd = &g_worldMapDefs[world];
    worldMapLayout(app, world, &mapX, &mapY, &scale);
    for (i = 0; i < wd->decoCount; ++i) {
        const WBMapDeco* deco = &wd->decos[i];
        drawAnchoredImage(getImage(app, deco->assetId), mapX + deco->x * scale, mapY + deco->y * scale, scale,
            deco->anchorX, deco->anchorY);
    }
}

static void drawWorldSelectBottom(AppState* app) {
    C2D_Image mapImage = getImage(app, worldMapAssetId(app->worldSelectWorld));
    C2D_Text title;
    float mapX;
    float mapY;
    float scale;
    worldMapLayout(app, app->worldSelectWorld, &mapX, &mapY, &scale);
    drawWorldSelectSkyBackdrop(app, BOTTOM_W, BOTTOM_H);
    /* Every world frame has the score's white rectangle across the top of the stage,
       with the title on it; worlds 2-5 put their (opaque) map directly beneath it. */
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, BOTTOM_W, mapY, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
    drawAnchoredImage(mapImage, mapX, mapY, scale, 0.0f, 0.0f);
    /* Score order: map (channel 4), mission icons (10-33), minis (41-82), decorations (80-138). */
    drawWorldMissionIcons(app);
    drawWorldMiniWalkers(app);
    drawWorldMapDecorations(app);
    drawAnchoredImage(getImage(app, ASSET_PREV_WORLD_ARROW), WB_WORLD_PREV_ARROW_X, WB_WORLD_ARROW_Y, 1.0f, 12.0f, 12.0f);
    drawAnchoredImage(getImage(app, ASSET_NEXT_WORLD_ARROW), WB_WORLD_NEXT_ARROW_X, WB_WORLD_ARROW_Y, 1.0f, 12.0f, 12.0f);
    C2D_TextParse(&title, app->dynamicBuf, worldName(app->worldSelectWorld));
    C2D_TextOptimize(&title);
    C2D_DrawText(&title, C2D_AlignCenter, BOTTOM_W * 0.5f, 4.0f, 0.5f, 0.42f, 0.42f);
    drawWorldMissionRollover(app);
    if (app->showDiagnostics) {
        int _m;
        for (_m = 1; _m <= 12; ++_m) {
            float _mx, _my;
            if (worldMissionState(app, app->worldSelectWorld, _m) < 0) continue;
            missionPointToBottom(app, app->worldSelectWorld, _m, &_mx, &_my);
            C2D_DrawRectSolid(_mx - 2.0f, _my - 2.0f, 0.5f, 4.0f, 4.0f,
                C2D_Color32(0xFF, 0x00, 0x00, 0xB0));
        }
    }
}

static void drawSidebarEnergy(const AppState* app, int energy) {
    float iconX = 8.0f;
    float iconY = 84.0f;
    float bgX = 31.0f;
    float bgY = 87.0f;
    C2D_DrawImageAt(getImage(app, energy > 0 ? ASSET_UI_ENERGY_ICON : ASSET_UI_NO_ENERGY), iconX, iconY, 0.0f, NULL, 1.0f, 1.0f);
    C2D_DrawImageAt(getImage(app, ASSET_UI_ENERGY_STRIPE_BG), bgX, bgY, 0.0f, NULL, 1.0f, 1.0f);
    if (energy > 0) {
        float units = floorf((float) energy * 0.23f);
        u32 color = energy <= 20 ? C2D_Color32(0xFF, 0x00, 0x00, 0xFF) : C2D_Color32(0x00, 0xB3, 0x22, 0xFF);
        if (units < 1.0f) {
            units = 1.0f;
        }
        C2D_DrawRectSolid(bgX + 1.0f, bgY + 1.0f, 0.0f, units * 3.0f, 3.0f, color);
    }
}

static void drawSidebarRecipe(AppState* app, const WBResourcePile* recipe, bool planMode) {
    int assets[6];
    int kinds[6];
    int counts[6];
    int num = 0;
    float baseY = planMode ? 148.0f : 108.0f;
    float iconX[6] = { 18.0f, 46.0f, 74.0f, 102.0f, 130.0f, 158.0f };
    float textX[6] = { 30.0f, 58.0f, 86.0f, 114.0f, 142.0f, 170.0f };
    if (recipe->red > 0) { assets[num] = ASSET_RESOURCE_RED; kinds[num] = 0; counts[num++] = recipe->red; }
    if (recipe->blue > 0 && num < 6) { assets[num] = ASSET_RESOURCE_BLUE; kinds[num] = 1; counts[num++] = recipe->blue; }
    if (recipe->green > 0 && num < 6) { assets[num] = ASSET_RESOURCE_YELLOW; kinds[num] = 2; counts[num++] = recipe->green; }
    if (recipe->yellow > 0 && num < 6) { assets[num] = ASSET_RESOURCE_YELLOW; kinds[num] = 3; counts[num++] = recipe->yellow; }
    if (recipe->wheel > 0 && num < 6) { assets[num] = ASSET_RESOURCE_WHEEL; kinds[num] = 4; counts[num++] = recipe->wheel; }
    if (recipe->energy > 0 && num < 6) { assets[num] = ASSET_RESOURCE_ENERGY; kinds[num] = 5; counts[num++] = recipe->energy; }
    if (recipe->white > 0 && num < 6) { assets[num] = ASSET_RESOURCE_WHITE_1; kinds[num] = 6; counts[num++] = recipe->white; }
    if (num < 1) {
        num = 1;
    }
    C2D_DrawImageAt(getImage(app, ASSET_UI_UNIT_INFO_SEPARATOR), 12.0f, baseY - 8.0f, 0.0f, NULL, 1.0f, 1.0f);
    for (int i = 0; i < num; ++i) {
        char countText[8];
        int carryIdx = -1;
        if      (kinds[i] == 0) carryIdx = WB_EXTRA_CARRY_RED_IDX;
        else if (kinds[i] == 1) carryIdx = WB_EXTRA_CARRY_BLUE_IDX;
        else if (kinds[i] == 2) carryIdx = WB_EXTRA_CARRY_GREEN_IDX;
        else if (kinds[i] == 3) carryIdx = WB_EXTRA_CARRY_YELLOW_IDX;
        else if (kinds[i] == 4) carryIdx = WB_EXTRA_CARRY_WHEEL_IDX;
        else if (kinds[i] == 5) carryIdx = WB_EXTRA_CARRY_ENERGY_IDX;
        if (carryIdx >= 0) {
            C2D_Image img = getWorldbuilderExtraImage(app, carryIdx);
            if (img.subtex) {
                drawAnchoredImage(img, iconX[i], baseY + 18.0f, 0.75f, img.subtex->width * 0.5f, img.subtex->height * 0.5f);
            } else {
                drawAnchoredImage(getImage(app, assets[i]), iconX[i], baseY + 18.0f, 0.75f, g_objectAnchors[assets[i]].x, g_objectAnchors[assets[i]].y);
            }
        } else {
            drawAnchoredImage(getImage(app, assets[i]), iconX[i], baseY + 18.0f, 0.75f, g_objectAnchors[assets[i]].x, g_objectAnchors[assets[i]].y);
        }
        snprintf(countText, sizeof(countText), "%d", counts[i]);
        drawTextLineBlack(app->dynamicBuf, textX[i], baseY + 10.0f, 0.36f, countText);
    }
}

/* Map a unit type to its small plan-card sprite — same icons used on the plan screen. */
static int unitSidebarIconAssetId(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:       return ASSET_PLAN_BUGGY;
        case WB_UNIT_DUCK:        return ASSET_VEHICLE_DUCK;
        case WB_UNIT_DIRTBUGGY:   return ASSET_PLAN_DIRTBUGGY;
        case WB_UNIT_STEAMSHOVEL: return ASSET_PLAN_STEAMSHOVEL;
        case WB_UNIT_DUMPTRUCK:   return ASSET_PLAN_DUMPTRUCK;
        case WB_UNIT_FORKLIFT:    return ASSET_PLAN_FORKLIFT;
        case WB_UNIT_DOZER:       return ASSET_PLAN_DOZER;
        case WB_UNIT_SPEEDBOAT:   return ASSET_PLAN_SPEEDBOAT;
        case WB_UNIT_TUGBOAT:     return ASSET_PLAN_TUGBOAT;
        case WB_UNIT_FREIGHTER:   return ASSET_PLAN_FREIGHTER;
        case WB_UNIT_FROG:        return ASSET_PLAN_FROG;
        case WB_UNIT_FISH:        return ASSET_PLAN_FISH;
        case WB_UNIT_SNAIL:       return ASSET_PLAN_SNAIL;
        case WB_UNIT_TREEBOT:     return ASSET_PLAN_TREEBOT;
        case WB_UNIT_REPAIRBOT:   return ASSET_PLAN_REPAIRBOT;
        case WB_UNIT_DEFENDER:
        case WB_UNIT_DEFENDER2:   return ASSET_PLAN_DEFENDER;
        case WB_UNIT_FREEZEBOT:   return ASSET_PLAN_FREEZEBOT;
        default: return -1;
    }
}

static void drawSidebarSelectionPanel(AppState* app, const WBSelectedUnitView* unitView) {
    int iconAsset = unitSidebarIconAssetId(unitView->unitType);
    const WBResourcePile* recipe = wbUnitRecipe(unitView->unitType);
    drawTextLine(app->dynamicBuf, 48.0f, 22.0f, 0.42f, wbUnitName(unitView->unitType));
    if (iconAsset >= 0) {
        drawAnchoredImage(getImage(app, iconAsset), 59.0f, 51.0f, 1.0f, g_objectAnchors[iconAsset].x, g_objectAnchors[iconAsset].y);
    }
    drawSidebarEnergy(app, unitView->energy);
    if (recipe) {
        drawSidebarRecipe(app, recipe, false);
    }
}

static void drawSidebarPlanPanel(AppState* app, WBPlanType planType) {
    WBUnitType unitType = planTypeToUnit(planType);
    WBBuildingType buildingType = planTypeToBuilding(planType);
    const WBResourcePile* recipe = buildingType != WB_BUILDING_NONE ? wbBuildingRecipe(buildingType) : wbUnitRecipe(unitType);
    int iconAsset = planAssetId(planType);
    drawTextLine(app->dynamicBuf, 12.0f, 24.0f, 0.42f, wbPlanName(planType));
    if (iconAsset >= 0) {
        C2D_DrawImageAt(getImage(app, iconAsset), 18.0f, 46.0f, 0.0f, NULL, 1.0f, 1.0f);
    }
    if (recipe) {
        drawSidebarRecipe(app, recipe, true);
    }
}

static void drawSidebarBuildingPanel(AppState* app, WBBuildingType buildingType) {
    int iconAsset = buildingPanelIconAssetId(buildingType);
    const WBResourcePile* recipe = wbBuildingRecipe(buildingType);
    drawTextLine(app->dynamicBuf, 12.0f, 24.0f, 0.42f, wbBuildingName(buildingType));
    if (iconAsset >= 0) {
        C2D_DrawImageAt(getImage(app, iconAsset), 18.0f, 46.0f, 0.0f, NULL, 1.0f, 1.0f);
    }
    if (recipe) {
        drawSidebarRecipe(app, recipe, true);
    }
}

static void drawInfoOverlay(AppState* app, const WBSelectedUnitView* unitView) {
    int heroAsset = heroAssetIdForUnit(unitView->unitType);
    const char* desc = unitDescription(unitView->unitType);
    float descScale = 0.36f;
    float lineStep = 17.0f;
    int maxChars = 22;
    int numLines = countWrappedLines(desc, maxChars);
    float availH = 160.0f; /* y=210 prompt - y=50 start */
    if (numLines > 0 && numLines * lineStep > availH) {
        lineStep = availH / (float)numLines;
        if (lineStep < 11.0f) lineStep = 11.0f;
        descScale = lineStep / 17.0f * 0.36f;
        if (descScale < 0.24f) descScale = 0.24f;
    }
    drawTextLine(app->dynamicBuf, 14.0f, 24.0f, 0.46f, wbUnitName(unitView->unitType));
    drawWrappedText(app->dynamicBuf, 14.0f, 50.0f, descScale, desc, lineStep, maxChars);
    if (heroAsset >= 0) {
        C2D_DrawImageAt(getImage(app, heroAsset), 140.0f, 44.0f, 0.0f, NULL, 0.62f, 0.62f);
    }
    drawTextLine(app->dynamicBuf, 12.0f, 214.0f, 0.34f, "Y: close  B: deselect  X: disassemble");
}

static int heroAssetIdForBuilding(WBBuildingType buildingType) {
    switch (buildingType) {
        case WB_BUILDING_GAS_STATION:  return ASSET_BUILDING_GAS_STATION_HERO;
        case WB_BUILDING_MARINA:       return ASSET_BUILDING_MARINA_HERO;
        case WB_BUILDING_ROBOT_LAB:    return ASSET_BUILDING_ROBOT_LAB_HERO;
        case WB_BUILDING_GUARD_TOWER:  return ASSET_BUILDING_GUARD_TOWER_HERO;
        case WB_BUILDING_FACTORY:  return ASSET_BUILDING_FACTORY_HERO;
        case WB_BUILDING_HOUSE:    return ASSET_BUILDING_HOUSE_HERO;
        case WB_BUILDING_WINDMILL: return ASSET_BUILDING_WINDMILL_HERO;
        case WB_BUILDING_GARAGE:   return ASSET_BUILDING_GARAGE_HERO;
        case WB_BUILDING_NURSERY:  return ASSET_BUILDING_NURSERY_HERO;
        default: return -1;
    }
}

static const char* buildingDescription(WBBuildingType buildingType) {
    switch (buildingType) {
        case WB_BUILDING_GAS_STATION:
            return "Recharges land vehicles parked adjacent to it.";
        case WB_BUILDING_MARINA:
            return "Recharges water vessels parked adjacent to it.";
        case WB_BUILDING_ROBOT_LAB:
            return "Recharges robots parked adjacent to it.";
        case WB_BUILDING_GUARD_TOWER:
            return "Automatically attacks nearby hostile monsters within range.";
        case WB_BUILDING_FACTORY:
            return "Converts adjacent boulders and trees into LEGO bricks. Press A to cycle the color of bricks it produces.";
        case WB_BUILDING_HOUSE:
            return "A home for colonists. Provides a place to rest and recharge.";
        case WB_BUILDING_WINDMILL:
            return "Generates energy over time and deposits it on an adjacent tile.";
        case WB_BUILDING_GARAGE:
            return "Generates wheels over time and deposits them on an adjacent tile.";
        case WB_BUILDING_NURSERY:
            return "Grows new trees on adjacent normal terrain over time.";
        default:
            return "No description available.";
    }
}

static void drawBuildingInfoOverlay(AppState* app, WBBuildingType buildingType) {
    int heroAsset = heroAssetIdForBuilding(buildingType);
    const char* desc = buildingDescription(buildingType);
    float descScale = 0.36f;
    float lineStep = 17.0f;
    int maxChars = 22;
    int numLines = countWrappedLines(desc, maxChars);
    float availH = 160.0f;
    if (numLines > 0 && numLines * lineStep > availH) {
        lineStep = availH / (float)numLines;
        if (lineStep < 11.0f) lineStep = 11.0f;
        descScale = lineStep / 17.0f * 0.36f;
        if (descScale < 0.24f) descScale = 0.24f;
    }
    drawTextLine(app->dynamicBuf, 14.0f, 24.0f, 0.46f, wbBuildingName(buildingType));
    drawWrappedText(app->dynamicBuf, 14.0f, 50.0f, descScale, desc, lineStep, maxChars);
    if (heroAsset >= 0) {
        C2D_DrawImageAt(getImage(app, heroAsset), 140.0f, 44.0f, 0.0f, NULL, 0.62f, 0.62f);
    }
    drawTextLine(app->dynamicBuf, 12.0f, 214.0f, 0.34f, "Y: close  B: deselect  X: disassemble");
}

static int planMenuEntryCount(const AppState* app) {
    int count = 0;
    int planType;
    for (planType = 1; planType < WB_PLAN_COUNT; ++planType) {
        if (app->map.possiblePlans[planType] || app->map.planInventory[planType] > 0) {
            ++count;
        }
    }
    return count;
}

static WBPlanType planMenuEntryAt(const AppState* app, int targetIndex) {
    int index = 0;
    int planType;
    for (planType = 1; planType < WB_PLAN_COUNT; ++planType) {
        if (!(app->map.possiblePlans[planType] || app->map.planInventory[planType] > 0)) {
            continue;
        }
        if (index == targetIndex) {
            return (WBPlanType) planType;
        }
        ++index;
    }
    return WB_PLAN_NONE;
}

static void drawBottomPlanMenu(AppState* app) {
    int count = planMenuEntryCount(app);
    int i;
    float boxX = 26.0f;
    float boxY = 34.0f;
    float boxW = 268.0f;
    float boxH = 170.0f;
    C2D_DrawRectSolid(boxX, boxY, 0.0f, boxW, boxH, C2D_Color32(0xFF, 0xFF, 0xFF, 0xEA));
    C2D_DrawRectSolid(boxX, boxY, 0.0f, boxW, 2.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX, boxY + boxH - 2.0f, 0.0f, boxW, 2.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX, boxY, 0.0f, 2.0f, boxH, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX + boxW - 2.0f, boxY, 0.0f, 2.0f, boxH, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    drawTextLine(app->dynamicBuf, boxX + 100.0f, boxY + 14.0f, 0.42f, "Plans");
    drawTextLine(app->dynamicBuf, boxX + 18.0f, boxY + 34.0f, 0.34f, "D-pad/A:arm  B/SELECT:close");
    if (count == 0) {
        drawTextLine(app->dynamicBuf, boxX + 18.0f, boxY + 78.0f, 0.38f, "No plans available.");
        return;
    }
    {
        int maxVisible = 3;
        int scrollOffset = app->planMenuScrollOffset;
        int end = scrollOffset + maxVisible;
        if (end > count) end = count;
        for (i = scrollOffset; i < end; ++i) {
            WBPlanType planType = planMenuEntryAt(app, i);
            bool owned = app->map.planInventory[planType] > 0;
            bool armed = app->armedPlan == planType;
            float y = boxY + 56.0f + ((i - scrollOffset) * 34.0f);
            char line[96];
            int iconAsset = owned ? planAssetId(planType) : ASSET_PLAN_UNKNOWN;
            C2D_DrawRectSolid(boxX + 16.0f, y, 0.0f, boxW - 32.0f, 28.0f,
                armed   ? C2D_Color32(0xE3, 0xEC, 0x98, 0xE8) :
                (i == app->planMenuCursor) ? C2D_Color32(0xA0, 0xC8, 0xFF, 0xB0) :
                            C2D_Color32(0xFF, 0xFF, 0xFF, 0xD8));
            C2D_DrawRectSolid(boxX + 16.0f, y, 0.0f, boxW - 32.0f, 1.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
            C2D_DrawRectSolid(boxX + 16.0f, y + 27.0f, 0.0f, boxW - 32.0f, 1.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
            drawAnchoredImage(getImage(app, iconAsset), boxX + 34.0f, y + 15.0f, 0.72f, g_objectAnchors[iconAsset].x, g_objectAnchors[iconAsset].y);
            snprintf(line, sizeof(line), "%s  x%d", owned ? wbPlanName(planType) : "Unknown plan", app->map.planInventory[planType]);
            drawTextLine(app->dynamicBuf, boxX + 58.0f, y + 8.0f, 0.34f, line);
        }
        if (scrollOffset > 0) {
            drawTextLine(app->dynamicBuf, boxX + boxW - 22.0f, boxY + 54.0f, 0.38f, "^");
        }
        if (scrollOffset + maxVisible < count) {
            drawTextLine(app->dynamicBuf, boxX + boxW - 22.0f, boxY + 156.0f, 0.38f, "v");
        }
    }
}

static void drawBottomStartMenu(AppState* app) {
    float boxX = 66.0f;
    float boxY = 48.0f;
    float boxW = 188.0f;
    float boxH = 144.0f;
    static const float itemY[5] = { 36.0f, 56.0f, 76.0f, 98.0f, 118.0f };
    C2D_DrawRectSolid(boxX, boxY, 0.0f, boxW, boxH, C2D_Color32(0xFF, 0xFF, 0xFF, 0xEF));
    C2D_DrawRectSolid(boxX, boxY, 0.0f, boxW, 2.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX, boxY + boxH - 2.0f, 0.0f, boxW, 2.0f, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX, boxY, 0.0f, 2.0f, boxH, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(boxX + boxW - 2.0f, boxY, 0.0f, 2.0f, boxH, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    /* cursor highlight */
    {
        int cur = app->startMenuCursor;
        if (cur >= 0 && cur < 5) {
            float hy = boxY + itemY[cur] - 4.0f;
            float hh = (cur == 3 || cur == 4) ? 20.0f : 18.0f;
            C2D_DrawRectSolid(boxX + 4.0f, hy, 0.0f, boxW - 8.0f, hh, C2D_Color32(0xA0, 0xC8, 0xFF, 0xB0));
        }
    }
    drawTextLine(app->dynamicBuf, boxX + 66.0f, boxY + 12.0f, 0.42f, "Menu");
    drawTextLine(app->dynamicBuf, boxX + 30.0f, boxY + 36.0f, 0.34f, "Continue Mission");
    drawTextLine(app->dynamicBuf, boxX + 30.0f, boxY + 56.0f, 0.34f, "Restart Mission");
    drawTextLine(app->dynamicBuf, boxX + 30.0f, boxY + 76.0f, 0.34f, "End Mission");
    drawTextLine(app->dynamicBuf, boxX + 30.0f, boxY + 98.0f, 0.34f, "Music");
    drawTextLine(app->dynamicBuf, boxX + 30.0f, boxY + 118.0f, 0.34f, "Sound FX");
    drawAnchoredImage(getImage(app, app->muteMusic ? ASSET_X_MARK : ASSET_CHECK_MARK), boxX + 150.0f, boxY + 105.0f, 0.58f, g_objectAnchors[ASSET_CHECK_MARK].x, g_objectAnchors[ASSET_CHECK_MARK].y);
    drawAnchoredImage(getImage(app, app->muteSfx ? ASSET_X_MARK : ASSET_CHECK_MARK), boxX + 150.0f, boxY + 125.0f, 0.58f, g_objectAnchors[ASSET_CHECK_MARK].x, g_objectAnchors[ASSET_CHECK_MARK].y);
    drawTextLine(app->dynamicBuf, boxX + 22.0f, boxY + 132.0f, 0.30f, "D-pad/A: select  B: close");
}

/* 'resource popup display behavior': the pile's bricks, biggest count first, in a
   bubble just up and to the right of the tile (posToLoc + (20,-5) on the stage). */
static void drawPileInspectPopup(AppState* app) {
    const WBCell* cell;
    int idx[7];
    int count[7];
    int n = 0;
    int i;
    int j;
    float locX;
    float locY;
    float x;
    float y;
    float w;
    const float h = 18.0f;
    if (!app->pileInspectValid || !inBounds(app, app->pileInspectX, app->pileInspectY)) {
        return;
    }
    cell = &app->map.cells[app->pileInspectY][app->pileInspectX];
    if (!cell->hasResource || cell->resourceIsPlan || pileEmpty(&cell->pile)) {
        app->pileInspectValid = false;
        return;
    }
    if (cell->pile.red > 0)    { idx[n] = WB_EXTRA_CARRY_RED_IDX;    count[n++] = cell->pile.red; }
    if (cell->pile.yellow > 0) { idx[n] = WB_EXTRA_CARRY_YELLOW_IDX; count[n++] = cell->pile.yellow; }
    if (cell->pile.green > 0)  { idx[n] = WB_EXTRA_CARRY_GREEN_IDX;  count[n++] = cell->pile.green; }
    if (cell->pile.blue > 0)   { idx[n] = WB_EXTRA_CARRY_BLUE_IDX;   count[n++] = cell->pile.blue; }
    if (cell->pile.wheel > 0)  { idx[n] = WB_EXTRA_CARRY_WHEEL_IDX;  count[n++] = cell->pile.wheel; }
    if (cell->pile.energy > 0) {
        int e = energyStatusFromDeci(pileLastBattery(cell));
        idx[n] = e >= 80 ? WB_EXTRA_CARRY_ENERGY_IDX : e > 1 ? WB_EXTRA_CARRY_ENERGY_LOW_IDX : WB_EXTRA_CARRY_ENERGY_DEAD_IDX;
        count[n++] = cell->pile.energy;
    }
    if (cell->pile.white > 0)  { idx[n] = -1;                        count[n++] = cell->pile.white; }
    for (i = 1; i < n; ++i) {
        for (j = i; j > 0 && count[j] > count[j - 1]; --j) {
            int ti = idx[j]; int tc = count[j];
            idx[j] = idx[j - 1]; count[j] = count[j - 1];
            idx[j - 1] = ti; count[j - 1] = tc;
        }
    }
    posToLoc(app, app->pileInspectX + 1, app->pileInspectY + 1, &locX, &locY);
    w = 4.0f + (float) n * 26.0f;
    x = locX + 20.0f * DIRECTOR_SCALE;
    y = locY - 5.0f * DIRECTOR_SCALE - h;
    if (x + w > BOTTOM_W - 2.0f) x = BOTTOM_W - 2.0f - w;
    if (x < 2.0f) x = 2.0f;
    if (y < 2.0f) y = 2.0f;
    C2D_DrawRectSolid(x, y, 0.0f, w, h, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(x + 1.0f, y + 1.0f, 0.0f, w - 2.0f, h - 2.0f, C2D_Color32(0xFF, 0xFF, 0xEE, 0xFF));
    for (i = 0; i < n; ++i) {
        char text[8];
        float ex = x + 4.0f + (float) i * 26.0f;
        C2D_Image img = idx[i] >= 0 ? getWorldbuilderExtraImage(app, idx[i]) : getImage(app, ASSET_CARRY_WHITE);
        if (img.subtex) {
            drawAnchoredImage(img, ex + 5.0f, y + h * 0.5f, 0.6f, img.subtex->width * 0.5f, img.subtex->height * 0.5f);
        }
        snprintf(text, sizeof(text), "%d", count[i]);
        drawTextLineBlack(app->dynamicBuf, ex + 11.0f, y + 3.0f, 0.34f, text);
    }
}

static void drawBottomGoalPopup(AppState* app) {
    float locX;
    float locY;
    float drawX;
    float drawY;
    float boxW;
    float boxH;
    float pointerX;
    if (!app->goalPopupVisible) {
        return;
    }
    posToLoc(app, app->goalPopupX + 1, app->goalPopupY + 1, &locX, &locY);
    boxW = 132.0f;
    boxH = app->goalPopupComplete ? 86.0f : 64.0f;
    drawX = locX - 54.0f;
    drawY = locY - 74.0f;
    if (drawX < 4.0f) drawX = 4.0f;
    if (drawX + boxW > BOTTOM_W - 4.0f) drawX = BOTTOM_W - 4.0f - boxW;
    if (drawY < 4.0f) drawY = 4.0f;
    pointerX = locX;
    if (pointerX < drawX + 12.0f) pointerX = drawX + 12.0f;
    if (pointerX > drawX + boxW - 12.0f) pointerX = drawX + boxW - 12.0f;
    C2D_DrawRectSolid(drawX, drawY, 0.0f, boxW, boxH, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(drawX + 3.0f, drawY + 3.0f, 0.0f, boxW - 6.0f, boxH - 4.0f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
    C2D_DrawTriangle(pointerX - 12.0f, drawY + boxH - 1.0f, C2D_Color32(0x00,0x00,0x00,0xFF), pointerX + 12.0f, drawY + boxH - 1.0f, C2D_Color32(0x00,0x00,0x00,0xFF), pointerX, drawY + boxH + 12.0f, C2D_Color32(0x00,0x00,0x00,0xFF), 0.0f);
    C2D_DrawTriangle(pointerX - 8.0f, drawY + boxH + 1.0f, C2D_Color32(0xFF,0xFF,0xFF,0xFF), pointerX + 8.0f, drawY + boxH + 1.0f, C2D_Color32(0xFF,0xFF,0xFF,0xFF), pointerX, drawY + boxH + 9.0f, C2D_Color32(0xFF,0xFF,0xFF,0xFF), 0.0f);
    drawTextLine(app->dynamicBuf, drawX + (boxW * 0.5f) - 28.0f, drawY + 10.0f, 0.38f, app->goalPopupBonus ? "Bonus Goal" : "Mission Goal");
    drawWrappedText(app->dynamicBuf, drawX + 12.0f, drawY + 34.0f, 0.36f, app->goalPopupText, 16.0f, 18);
    if (app->goalPopupComplete) {
        drawTextLine(app->dynamicBuf, drawX + 14.0f, drawY + 68.0f, 0.26f, "A: Continue");
        drawTextLine(app->dynamicBuf, drawX + 78.0f, drawY + 68.0f, 0.26f, "B: End");
    }
}

static void drawTopScreen(AppState* app) {
    WBSelectedUnitView unitView;
    WBBuildingType selectedBuilding = WB_BUILDING_NONE;
    bool hasUnitView = getSelectedUnitView(app, &unitView);
    bool hasBuildingView = false;
    char controls[128];
    float miniScale = minimapScaleForMap(app);
    if (!hasUnitView && app->hasSelection && inBounds(app, app->selectedX, app->selectedY)) {
        const WBCell* selCell = &app->map.cells[app->selectedY][app->selectedX];
        if (selCell->hasBuilding) {
            hasBuildingView = true;
            selectedBuilding = selCell->buildingType;
        }
    }
    C2D_TextBufClear(app->dynamicBuf);
    C2D_DrawText(&app->titleText, 0, 10.0f, 4.0f, 0.5f, 0.45f, 0.45f);
    drawMinimap(app, 312.0f, 114.0f, miniScale);
    if (app->infoOverlayOpen && hasUnitView) {
        drawInfoOverlay(app, &unitView);
    } else if (app->infoOverlayOpen && hasBuildingView) {
        drawBuildingInfoOverlay(app, selectedBuilding);
    } else {
        if (app->armedPlan != WB_PLAN_NONE) {
            drawSidebarPlanPanel(app, app->armedPlan);
        } else if (hasUnitView) {
            drawSidebarSelectionPanel(app, &unitView);
        } else if (hasBuildingView) {
            drawSidebarBuildingPanel(app, selectedBuilding);
        } else {
            drawTextLine(app->dynamicBuf, 12.0f, 24.0f, 0.42f, app->map.name[0] ? app->map.name : "Mission");
            drawTextLine(app->dynamicBuf, 12.0f, 58.0f, 0.36f, "Select a unit or arm a plan.");
        }
    }
    if (!app->infoOverlayOpen && hasUnitView) {
        snprintf(controls, sizeof(controls), "A: %s  Y: info  X: disassemble  B: deselect", actionModeName(app->actionMode));
        drawTextLine(app->dynamicBuf, 12.0f, 198.0f, 0.34f, controls);
    } else if (!app->infoOverlayOpen && hasBuildingView) {
        if (selectedBuilding == WB_BUILDING_FACTORY) {
            drawTextLine(app->dynamicBuf, 12.0f, 198.0f, 0.34f, "A: Cycle  Y: info  X: disassemble  B: deselect");
        } else {
            drawTextLine(app->dynamicBuf, 12.0f, 198.0f, 0.34f, "Y: info  X: disassemble  B: deselect");
        }
    }
    if (app->showDiagnostics) {
        char audioLine1[96];
        char audioLine2[96];
        char audioLine3[64];
        char audioLine4[80];
        snprintf(audioLine1, sizeof(audioLine1),
            "Audio %s  clips %d  music %d  sfx %d",
            app->audioReady ? "ON" : "OFF",
            app->loadedClipCount,
            app->audioReady ? (int) ndspChnIsPlaying(MUSIC_CHANNEL) : -1,
            app->audioReady ? (int) ndspChnIsPlaying(SFX_CHANNEL) : -1);
        snprintf(audioLine2, sizeof(audioLine2),
            "init %08lX  open %08lX  sfxN %d",
            (unsigned long) app->audioInitResult,
            (unsigned long) app->audioFallbackOpenResult,
            app->sfxPlayCount);
        snprintf(audioLine3, sizeof(audioLine3),
            "fb %08lX  mv.ld %d  mu %d  lin %dKB",
            (unsigned long) app->audioFallbackInitResult,
            (int) app->sfxMove.loaded,
            (int) app->muteSfx,
            app->linearFreeKB);
        {
            int mACount = app->monstersASheet ? (int) C2D_SpriteSheetCount(app->monstersASheet) : -1;
            int mBCount = app->monstersBSheet ? (int) C2D_SpriteSheetCount(app->monstersBSheet) : -1;
            int sampleId = -1;
            int sampleSubtex = 0;
            if (g_monsterCount > 0 && g_monsters[0].active) {
                sampleId = monsterAnimAssetId(g_monsters[0].type, g_monsters[0].dir, g_monsters[0].onWater, 0, false);
                if (sampleId >= 0) {
                    C2D_Image si = getImage(app, sampleId);
                    sampleSubtex = si.subtex ? 1 : 0;
                }
            }
            snprintf(audioLine4, sizeof(audioLine4),
                "mc%d cA%d cB%d id%d ok%d",
                g_monsterCount, mACount, mBCount, sampleId, sampleSubtex);
        }
        drawTextLine(app->dynamicBuf, 8.0f, 206.0f, 0.34f, audioLine1);
        drawTextLine(app->dynamicBuf, 8.0f, 218.0f, 0.34f, audioLine2);
        drawTextLine(app->dynamicBuf, 8.0f, 230.0f, 0.34f, audioLine3);
        drawTextLine(app->dynamicBuf, 8.0f, 236.0f, 0.34f, audioLine4);
    }
}

static bool pointInRect(float px, float py, float x, float y, float w, float h) {
    return px >= x && px < (x + w) && py >= y && py < (y + h);
}

/* Rolling over a goal shows what it needs; on the 3DS a tap does the rolling over. */
static bool showGoalPopupForCell(AppState* app, int tx, int ty) {
    char goalText[96];
    const WBCell* goalCell;
    if (!inBounds(app, tx, ty)) {
        return false;
    }
    goalCell = &app->map.cells[ty][tx];
    if (!goalCell->hasGoal || goalCell->goalSatisfied || (goalCell->bonusGoal && !app->map.bonusAvailable)) {
        return false;
    }
    if (goalCell->goalIsCollect) {
        const char* mname = goalCell->goalCollectType != WB_MONSTER_NONE
            ? wbMonsterName(goalCell->goalCollectType) : "creature";
        snprintf(goalText, sizeof(goalText), "Collect: %d %s", goalCell->goalCollectCount, mname);
    } else {
        snprintf(goalText, sizeof(goalText), "Needs: %s", goalRequiredUnitName(goalCell->goalType));
    }
    showGoalPopup(app, goalText, goalCell->bonusGoal, false, tx, ty);
    return true;
}

/* Rolling over a pile shows its bricks ('resource popup display behavior'); the port
   had no way to see what a pile holds, so a tap on it now does the same. */
static void updatePileInspect(AppState* app, int tx, int ty) {
    const WBCell* cell = inBounds(app, tx, ty) ? &app->map.cells[ty][tx] : NULL;
    if (cell && cell->hasResource && !cell->resourceIsPlan && !pileEmpty(&cell->pile)) {
        app->pileInspectValid = true;
        app->pileInspectX = tx;
        app->pileInspectY = ty;
    } else {
        app->pileInspectValid = false;
    }
}

/* map display manager getAll8Neighbors(pos), then pos itself: x-major, the tile last. */
static int resourceTilesAround(const AppState* app, int tx, int ty, int* outX, int* outY) {
    int n = 0;
    int i;
    int j;
    for (i = -1; i <= 1; ++i) {
        for (j = -1; j <= 1; ++j) {
            if ((i == 0 && j == 0) || !inBounds(app, tx + i, ty + j)) {
                continue;
            }
            outX[n] = tx + i;
            outY[n] = ty + j;
            ++n;
        }
    }
    outX[n] = tx;
    outY[n] = ty;
    return n + 1;
}

/* checkResourcesAround(): nothing on the tile, and the piles around it hold the recipe. */
static bool checkResourcesAround(const AppState* app, int tx, int ty, const WBResourcePile* recipe) {
    WBResourcePile need = *recipe;
    int nx[9];
    int ny[9];
    int n;
    int k;
    if (tileHasOccupant(app, tx, ty)) {
        return false;
    }
    n = resourceTilesAround(app, tx, ty, nx, ny);
    for (k = 0; k < n; ++k) {
        const WBCell* cell = &app->map.cells[ny[k]][nx[k]];
        if (!cell->hasResource || cell->resourceIsPlan) {
            continue;
        }
        need.red -= cell->pile.red;
        need.blue -= cell->pile.blue;
        need.green -= cell->pile.green;
        need.yellow -= cell->pile.yellow;
        need.wheel -= cell->pile.wheel;
        need.energy -= cell->pile.energy;
        need.white -= cell->pile.white;
        if (need.red < 0) need.red = 0;
        if (need.blue < 0) need.blue = 0;
        if (need.green < 0) need.green = 0;
        if (need.yellow < 0) need.yellow = 0;
        if (need.wheel < 0) need.wheel = 0;
        if (need.energy < 0) need.energy = 0;
        if (need.white < 0) need.white = 0;
        if (resourceTotal(&need) == 0) {
            return true;
        }
    }
    return false;
}

static void consumeFromPile(WBResourcePile* pile, WBResourcePile* need) {
    int take;
    take = pile->red < need->red ? pile->red : need->red;
    pile->red -= take;
    need->red -= take;
    take = pile->blue < need->blue ? pile->blue : need->blue;
    pile->blue -= take;
    need->blue -= take;
    take = pile->green < need->green ? pile->green : need->green;
    pile->green -= take;
    need->green -= take;
    take = pile->yellow < need->yellow ? pile->yellow : need->yellow;
    pile->yellow -= take;
    need->yellow -= take;
    take = pile->wheel < need->wheel ? pile->wheel : need->wheel;
    pile->wheel -= take;
    need->wheel -= take;
    take = pile->energy < need->energy ? pile->energy : need->energy;
    pile->energy -= take;
    need->energy -= take;
    take = pile->white < need->white ? pile->white : need->white;
    pile->white -= take;
    need->white -= take;
}

static void cleanupResourceCell(WBCell* cell) {
    if (!cell->hasResource || cell->resourceIsPlan) {
        return;
    }
    if (resourceTotal(&cell->pile) == 0) {
        memset(&cell->pile, 0, sizeof(cell->pile));
        cell->hasResource = false;
    }
}

/* useResourcesAround(): each battery is the best one on top of any pile around; then
   the bricks come from the piles in order. (Its "exact pile first" pass compares the
   whole contents list against the recipe, so it never matches, and is left out.)
   Returns the batteries taken, for the new object. */
static int useResourcesAround(AppState* app, int tx, int ty, const WBResourcePile* recipe, u16* outEnergy, int maxEnergy) {
    WBResourcePile need = *recipe;
    int nx[9];
    int ny[9];
    int n;
    int k;
    int e;
    int count = 0;
    n = resourceTilesAround(app, tx, ty, nx, ny);
    for (e = 0; e < recipe->energy; ++e) {
        int best = -1;
        int bestCharge = -1;
        for (k = 0; k < n; ++k) {
            const WBCell* cell = &app->map.cells[ny[k]][nx[k]];
            if (!cell->hasResource || cell->resourceIsPlan || cell->pile.energy <= 0) {
                continue;
            }
            if (pileLastBattery(cell) > bestCharge) {
                best = k;
                bestCharge = pileLastBattery(cell);
            }
        }
        if (best >= 0) {
            WBCell* cell = &app->map.cells[ny[best]][nx[best]];
            int charge = pileTakeBattery(cell);
            if (count < maxEnergy) {
                outEnergy[count++] = (u16) charge;
            }
            cleanupResourceCell(cell);
        }
    }
    need.energy = 0;
    for (k = 0; k < n && resourceTotal(&need) > 0; ++k) {
        WBCell* cell = &app->map.cells[ny[k]][nx[k]];
        if (!cell->hasResource || cell->resourceIsPlan) {
            continue;
        }
        consumeFromPile(&cell->pile, &need);
        cleanupResourceCell(cell);
    }
    return count;
}

static WBUnitType planTypeToUnit(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_BUGGY:       return WB_UNIT_BUGGY;
        case WB_PLAN_DUCK:        return WB_UNIT_DUCK;
        case WB_PLAN_DIRTBUGGY:   return WB_UNIT_DIRTBUGGY;
        case WB_PLAN_STEAMSHOVEL: return WB_UNIT_STEAMSHOVEL;
        case WB_PLAN_DUMPTRUCK:   return WB_UNIT_DUMPTRUCK;
        case WB_PLAN_FORKLIFT:    return WB_UNIT_FORKLIFT;
        case WB_PLAN_DOZER:       return WB_UNIT_DOZER;
        case WB_PLAN_SPEEDBOAT:   return WB_UNIT_SPEEDBOAT;
        case WB_PLAN_TUGBOAT:     return WB_UNIT_TUGBOAT;
        case WB_PLAN_FREIGHTER:   return WB_UNIT_FREIGHTER;
        case WB_PLAN_FROG:        return WB_UNIT_FROG;
        case WB_PLAN_FISH:        return WB_UNIT_FISH;
        case WB_PLAN_SNAIL:       return WB_UNIT_SNAIL;
        case WB_PLAN_TREEBOT:     return WB_UNIT_TREEBOT;
        case WB_PLAN_REPAIRBOT:   return WB_UNIT_REPAIRBOT;
        case WB_PLAN_DEFENDER:    return WB_UNIT_DEFENDER;
        case WB_PLAN_FREEZEBOT:   return WB_UNIT_FREEZEBOT;
        default: return WB_UNIT_NONE;
    }
}

static WBBuildingType planTypeToBuilding(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_GAS_STATION: return WB_BUILDING_GAS_STATION;
        case WB_PLAN_MARINA:      return WB_BUILDING_MARINA;
        case WB_PLAN_ROBOT_LAB:   return WB_BUILDING_ROBOT_LAB;
        case WB_PLAN_GUARD_TOWER: return WB_BUILDING_GUARD_TOWER;
        case WB_PLAN_AIRPORT:     return WB_BUILDING_AIRPORT;
        case WB_PLAN_HOUSE:       return WB_BUILDING_HOUSE;
        case WB_PLAN_FACTORY:     return WB_BUILDING_FACTORY;
        case WB_PLAN_WINDMILL:    return WB_BUILDING_WINDMILL;
        case WB_PLAN_GARAGE:      return WB_BUILDING_GARAGE;
        case WB_PLAN_NURSERY:     return WB_BUILDING_NURSERY;
        default: return WB_BUILDING_NONE;
    }
}

/* build plan icon checkBuild(): the bricks are around and the tile's terrain is in
   the plan's list. A plan lying on the tile doesn't stop it. */
static bool canBuildPlanAt(const AppState* app, WBPlanType planType, int tx, int ty, WBUnitType* outUnit) {
    WBUnitType buildUnit = planTypeToUnit(planType);
    WBBuildingType buildBuilding = planTypeToBuilding(planType);
    const WBResourcePile* recipe = buildBuilding != WB_BUILDING_NONE ? wbBuildingRecipe(buildBuilding) : wbUnitRecipe(buildUnit);
    if (outUnit) {
        *outUnit = buildUnit;
    }
    if (!recipe || !inBounds(app, tx, ty)) {
        return false;
    }
    if (buildBuilding != WB_BUILDING_NONE) {
        if (!wbBuildingCanBePlacedOn(buildBuilding, tileTerrain(app, tx, ty))) {
            return false;
        }
    } else if (!wbUnitCanTraverse(buildUnit, tileTerrain(app, tx, ty))) {
        return false;
    }
    return checkResourcesAround(app, tx, ty, recipe);
}

/* doBuild(): use the plan and the bricks, make the object, drop the highlight; the
   build cloud clicks (selects) the new object when it clears 750 ms later. */
static void tryBuildPlan(AppState* app, int tx, int ty) {
    WBUnitType buildUnit;
    WBBuildingType buildBuilding;
    WBCell* cell;
    const WBResourcePile* recipe;
    u16 energy[8];
    int energyCount;
    WBObjRef made;

    if (app->armedPlan == WB_PLAN_NONE || !inBounds(app, tx, ty)) {
        return;
    }
    buildUnit = planTypeToUnit(app->armedPlan);
    buildBuilding = planTypeToBuilding(app->armedPlan);
    recipe = buildBuilding != WB_BUILDING_NONE ? wbBuildingRecipe(buildBuilding) : wbUnitRecipe(buildUnit);
    if (!recipe || app->map.planInventory[app->armedPlan] <= 0) {
        return;
    }
    if (!canBuildPlanAt(app, app->armedPlan, tx, ty, NULL)) {
        return;
    }
    app->map.planInventory[app->armedPlan] -= 1;
    playSfxClipWorld(app, &app->sfxAssembly);
    energyCount = useResourcesAround(app, tx, ty, recipe, energy, 8);
    cell = &app->map.cells[ty][tx];
    memset(&made, 0, sizeof(made));
    if (buildBuilding != WB_BUILDING_NONE) {
        cell->hasBuilding = true;
        cell->buildingType = buildBuilding;
        cell->buildingDirection = WB_DIR_RIGHT;
        attachBuilding(app, tx, ty, energyCount > 0 ? energy[0] : 0);
        made = buildingRef(app, tx, ty);
    } else {
        int slot;
        cell->hasUnit = true;
        cell->unitType = buildUnit;
        cell->unitDirection = WB_DIR_RIGHT;
        cell->unitEnergyDeci = energyCount > 0 ? energy[0] : 0;
        cell->unitEnergy = energyStatusFromDeci(cell->unitEnergyDeci);
        memset(&cell->unitCargo, 0, sizeof(cell->unitCargo));
        memset(cell->unitCargoBatteries, 0, sizeof(cell->unitCargoBatteries));
        cell->unitCargoSpecial = WB_CARGO_NONE;
        cell->unitCargoTreeType = WB_TERRAIN_NORMAL;
        slot = attachUnitAgent(app, tx, ty);
        if (slot >= 0) {
            made = unitRefFromAgent(app, slot);
        }
    }
    app->hasSelection = false;
    app->infoOverlayOpen = false;
    app->actionMode = WB_ACTION_MOVE;
    app->pileInspectValid = false;
    app->buildSelectPending = made.kind != WB_OBJ_NONE;
    app->buildSelectMs = app->simMs;
    app->buildSelectRef = made;
    spawnBuildCloudEffect(app, tx, ty, recipe);
    app->armedPlan = WB_PLAN_NONE;
    app->planMenuOpen = false;
}

/* A tap on tile (tx,ty): map display manager tilePosclick() -> checkOverride(), then
   forwardClick(): the pile and goal show their rollovers, a unit or building on the
   tile is clicked (selected), and otherwise the highlighted object gets the mapclick. */
static void handleWorldTap(AppState* app, int tx, int ty) {
    WBCell* cell;
    if (!inBounds(app, tx, ty)) {
        return;
    }
    if (app->pushOverrideActive) {
        if (!refAlive(app, app->pushOverrideUnit, NULL, NULL)) {
            app->pushOverrideActive = false;
        } else if (manhattanDistance(tx, ty, app->pushOverrideX, app->pushOverrideY) <= 1) {
            app->pushOverrideActive = false;
            if (app->actionMode == WB_ACTION_PUSH) {
                app->actionMode = WB_ACTION_MOVE;
            }
            pushTowardsPos(app, app->pushOverrideUnit, tx, ty);
            return;
        }
    }
    updatePileInspect(app, tx, ty);
    if (!showGoalPopupForCell(app, tx, ty) && !app->goalPopupComplete) {
        app->goalPopupVisible = false;
    }
    cell = &app->map.cells[ty][tx];
    if (cell->hasUnit || cell->hasBuilding) {
        app->armedPlan = WB_PLAN_NONE;
        app->planMenuOpen = false;
        app->buildPreviewValid = false;
        selectObjectAt(app, tx, ty, true);
        return;
    }
    if (app->armedPlan != WB_PLAN_NONE) {
        tryBuildPlan(app, tx, ty);
        return;
    }
    {
        WBObjRef self = selectedUnitRef(app);
        if (self.kind == WB_OBJ_UNIT) {
            mapclickUnit(app, self, tx, ty);
        }
    }
}

static void handleWorldTouch(AppState* app, const touchPosition* touch) {
    int tx;
    int ty;
    if (!objectSpriteAtTouch(app, touch, &tx, &ty) &&
            !screenToTile(app, (float) touch->px, (float) touch->py, &tx, &ty)) {
        return;
    }
    handleWorldTap(app, tx, ty);
}

static void appendDebugLog(const char* msg) {
    FILE* fp = fopen("sdmc:/legowb3ds_debug.log", "a");
    if (fp) {
        fprintf(fp, "%s\n", msg);
        fclose(fp);
    }
}

/* Returns the next world in direction (+1 right, -1 left) from current, skipping any
   world whose first mission is still locked. Wraps around 1..7. */
static int nextWorldInDirection(const AppState* app, int current, int dir) {
    int w = current;
    int steps;
    for (steps = 0; steps < 7; ++steps) {
        w += dir;
        if (w < 1) w = 7;
        if (w > 7) w = 1;
        if (worldMissionState(app, w, 1) >= 0)
            return w;
    }
    return current; /* fallback: all other worlds locked */
}

/* The icon under the stylus, as the original's sprite hit test; where padded rects of
   neighbouring icons overlap, the nearest icon wins rather than the lowest number. */
static int missionAtWorldTouch(const AppState* app, const touchPosition* touch) {
    const float pad = 4.0f;
    float bestDist = 1.0e9f;
    int best = 0;
    int mission;
    for (mission = 1; mission <= 12; ++mission) {
        float l, t, r, b;
        float x;
        float y;
        float dx;
        float dy;
        if (worldMissionState(app, app->worldSelectWorld, mission) < 0) {
            continue;
        }
        worldMissionIconRect(app, app->worldSelectWorld, mission, &l, &t, &r, &b);
        if ((float) touch->px < l - pad || (float) touch->px > r + pad ||
            (float) touch->py < t - pad || (float) touch->py > b + pad) {
            continue;
        }
        missionPointToBottom(app, app->worldSelectWorld, mission, &x, &y);
        dx = (float) touch->px - x;
        dy = (float) touch->py - y;
        if ((dx * dx) + (dy * dy) < bestDist) {
            bestDist = (dx * dx) + (dy * dy);
            best = mission;
        }
    }
    return best;
}

static void handleTitleScreenTouch(AppState* app, const touchPosition* touch) {
    (void) touch;
    freeGameplaySheets(app);
    ensureMenuSheetsLoaded(app);
    switchWorldSelectSheets(app, 1);
    app->screenMode = WB_SCREEN_WORLD_SELECT;
    app->worldSelectWorld = 1;
    app->hoverMissionWorld = 0;
    app->hoverMissionLevel = 0;
    initWorldMiniWalkers(app, app->worldSelectWorld);
    playSfxClip(app, &app->sfxMove);
}

static void handleWorldSelectTouch(AppState* app, const touchPosition* touch) {
    int mission;
    {
        char _dbg[128];
        float mx, my;
        missionPointToBottom(app, app->worldSelectWorld, 1, &mx, &my);
        snprintf(_dbg, sizeof(_dbg), "touch world=%d pos=(%d,%d) m1=%.0f,%.0f m1state=%d",
            app->worldSelectWorld, touch->px, touch->py, mx, my,
            worldMissionState(app, app->worldSelectWorld, 1));
        appendDebugLog(_dbg);
    }
    if (pointInRect((float) touch->px, (float) touch->py, WB_WORLD_PREV_ARROW_X - 16.0f, 0.0f, 34.0f, 32.0f)) {
        app->worldSelectWorld = nextWorldInDirection(app, app->worldSelectWorld, -1);
        switchWorldSelectSheets(app, app->worldSelectWorld);
        initWorldMiniWalkers(app, app->worldSelectWorld);
        app->hoverMissionLevel = 0;
        playSfxClip(app, &app->sfxMove);
        return;
    }
    if (pointInRect((float) touch->px, (float) touch->py, WB_WORLD_NEXT_ARROW_X - 18.0f, 0.0f, 34.0f, 32.0f)) {
        app->worldSelectWorld = nextWorldInDirection(app, app->worldSelectWorld, 1);
        switchWorldSelectSheets(app, app->worldSelectWorld);
        initWorldMiniWalkers(app, app->worldSelectWorld);
        app->hoverMissionLevel = 0;
        playSfxClip(app, &app->sfxMove);
        return;
    }
    mission = missionAtWorldTouch(app, touch);
    {
        char _dbg[64];
        snprintf(_dbg, sizeof(_dbg), "  missionHit=%d", mission);
        appendDebugLog(_dbg);
    }
    if (mission > 0) {
        if (packagedMissionExists(app->worldSelectWorld, mission)) {
            appendDebugLog("  packagedMissionExists=true -> loading");
            playSfxClip(app, &app->sfxMove);
            loadMissionIntoGame(app, app->worldSelectWorld, mission);
        } else {
            appendDebugLog("  packagedMissionExists=false");
            playSfxClip(app, app->sfxWorldComingSoon.loaded ? &app->sfxWorldComingSoon : &app->sfxMove);
        }
        return;
    }
}

static void updateBuildPreview(AppState* app, int tx, int ty) {
    app->buildPreviewValid = false;
    if (app->armedPlan == WB_PLAN_NONE) {
        return;
    }
    if (!inBounds(app, tx, ty)) {
        return;
    }
    app->buildPreviewValid = true;
    app->buildPreviewX = tx + 1;
    app->buildPreviewY = ty + 1;
    app->buildPreviewAllowed = canBuildPlanAt(app, app->armedPlan, tx, ty, NULL);
}

static void handlePlanMenuTouch(AppState* app, const touchPosition* touch) {
    int count = planMenuEntryCount(app);
    int maxVisible = 3;
    int i;
    if (!pointInRect((float) touch->px, (float) touch->py, 26.0f, 34.0f, 268.0f, 170.0f)) {
        app->planMenuOpen = false;
        return;
    }
    /* Scroll up arrow */
    if (app->planMenuScrollOffset > 0 && touch->px >= 272 && touch->px < 294 && touch->py >= 88 && touch->py < 108) {
        app->planMenuScrollOffset -= 1;
        return;
    }
    /* Scroll down arrow */
    if (app->planMenuScrollOffset + maxVisible < count && touch->px >= 272 && touch->px < 294 && touch->py >= 190 && touch->py < 210) {
        app->planMenuScrollOffset += 1;
        return;
    }
    for (i = app->planMenuScrollOffset; i < app->planMenuScrollOffset + maxVisible && i < count; ++i) {
        float y = 90.0f + ((i - app->planMenuScrollOffset) * 34.0f);
        if (touch->px >= 42 && touch->px < 278 && touch->py >= y && touch->py < y + 28.0f) {
            WBPlanType planType = planMenuEntryAt(app, i);
            if (app->map.planInventory[planType] > 0) {
                app->armedPlan = planType;
                app->hasSelection = false;
                app->planMenuOpen = false;
                app->infoOverlayOpen = false;
                playSfxClip(app, &app->sfxPlan);
            }
            return;
        }
    }
}

static void handleStartMenuTouch(AppState* app, const touchPosition* touch) {
    char path[64];
    if (!pointInRect((float) touch->px, (float) touch->py, 66.0f, 48.0f, 188.0f, 144.0f)) {
        app->startMenuOpen = false;
        return;
    }
    if (touch->px >= 96 && touch->px < 224 && touch->py >= 84 && touch->py < 102) {
        app->startMenuOpen = false;
        playSfxClip(app, &app->sfxMove);
        return;
    }
    if (touch->px >= 96 && touch->px < 224 && touch->py >= 104 && touch->py < 122) {
        snprintf(path, sizeof(path), "romfs:/maps/map%d_%d.txt", app->activeWorld, app->activeMission);
        wbMapLoad(path, &app->map);
        resetCameraForMap(app);
        app->pileInspectValid = false;
        app->hasSelection = false;
        app->infoOverlayOpen = false;
        app->planMenuOpen = false;
        app->armedPlan = WB_PLAN_NONE;
        app->actionMode = WB_ACTION_MOVE;
        memset(app->worldEffects, 0, sizeof(app->worldEffects));
        app->map.isWB2Level = (app->activeWorld >= 6);
        initWorldObjects(app);
        app->planSwoop.active = false;
        app->startMenuOpen = false;
        clampCamera(app);
        playSfxClip(app, &app->sfxMove);
        return;
    }
    if (touch->px >= 96 && touch->px < 224 && touch->py >= 124 && touch->py < 142) {
        freeGameplaySheets(app);
        ensureMenuSheetsLoaded(app);
        switchWorldSelectSheets(app, app->activeWorld);
        app->screenMode = WB_SCREEN_WORLD_SELECT;
        app->worldSelectWorld = app->activeWorld;
        app->startMenuOpen = false;
        initWorldMiniWalkers(app, app->worldSelectWorld);
        startMenuMusic(app);
        playSfxClip(app, &app->sfxMove);
        return;
    }
    if (touch->px >= 96 && touch->px < 224 && touch->py >= 146 && touch->py < 164) {
        app->muteMusic = !app->muteMusic;
        if (app->muteMusic) {
            ndspChnWaveBufClear(MUSIC_CHANNEL);
        } else {
            playCurrentMusicTrack(app);
        }
        playSfxClip(app, &app->sfxMove);
        return;
    }
    if (touch->px >= 96 && touch->px < 224 && touch->py >= 166 && touch->py < 184) {
        app->muteSfx = !app->muteSfx;
        playSfxClip(app, &app->sfxMove);
        return;
    }
}

/* ───────────────────────── World 1 Mission 1 tutorial ─────────────────────────
   'tutorial manager' (tutorial cast) driven by the 'tutorial_sequence' text member.
   Picking W1M1 on the world map calls setTutorialMode(1): a mouse mask then blocks
   every click except the current step's click hole; the step advances when the
   hole's trigger event arrives (default mouseDown), after the step's #delay, or
   from the dialog's buttons. Steps without dialog text hide the bubble.

   On the 3DS the click holes become the matching control: the scroll buttons are
   the +Control Pad, rolling over a goal or a pile is a tap, the plan slot is R/SELECT
   then the plan, and the sidebar's TAKE APART / info / PICK UP buttons are X / Y / A.
   Only the sentences that name a mouse action are reworded to name the 3DS one. */

typedef enum WBTutTarget {
    WB_TUT_NONE = 0,         /* no click hole: only the dialog buttons */
    WB_TUT_SCROLL_RIGHT,     /* #generic_button #arrow_right */
    WB_TUT_SCROLL_DOWN,      /* #generic_button #arrow_down */
    WB_TUT_GOAL_LOOK,        /* #goal, rolled over: shows what the goal says */
    WB_TUT_GOAL_CLICK,       /* #goal, clicked: the selected model heads there */
    WB_TUT_OCCUPANT,         /* #occupant: select the model on the tile */
    WB_TUT_RESOURCE_CLICK,   /* #resource, clicked: the selected model heads there */
    WB_TUT_RESOURCE_LOOK,    /* #resource, rolled over: shows the pile's bricks */
    WB_TUT_TILE,             /* #tile: build / move / pick up / drop off there */
    WB_TUT_PLAN,             /* #plan 10: pick the plan from the collection */
    WB_TUT_TAKE_APART,       /* #menu #disassemblebutton */
    WB_TUT_INFO_OPEN,        /* #menu #infobutton */
    WB_TUT_INFO_CLOSE,       /* #info #closebutton */
    WB_TUT_ACTION            /* #menu #actionbutton2: PICK UP / DROP OFF */
} WBTutTarget;

typedef enum WBTutAction { WB_TUT_ACT_NONE = 0, WB_TUT_ACT_NEXT, WB_TUT_ACT_QUIT, WB_TUT_ACT_FINISH } WBTutAction;

/* Same order as the tutorial_arrow_* sprites appended to worldbuilder.t3s. */
typedef enum WBTutArrow {
    WB_TUT_ARROW_NONE = -1,
    WB_TUT_ARROW_LEFT_DOWN = 0,
    WB_TUT_ARROW_LEFT_UP,
    WB_TUT_ARROW_RIGHT,
    WB_TUT_ARROW_RIGHT_DOWN,
    WB_TUT_ARROW_RIGHT_UP
} WBTutArrow;
#define WB_EXTRA_TUT_ARROW_IDX        112
#define WB_EXTRA_TUT_ARROW_ORANGE_IDX 117
static const WBAnchor g_tutArrowReg[5] = { {19.0f, 19.0f}, {19.0f, 20.0f}, {26.0f, 18.0f}, {19.0f, 19.0f}, {19.0f, 20.0f} };

typedef enum WBTutArrowPlace {
    WB_TUT_AT_TARGET = 0,    /* arrow_offsets[shape][clicktarget] from the click rect */
    WB_TUT_AT_BOTTOM,        /* a fixed point on the bottom screen */
    WB_TUT_AT_TOP            /* a fixed point on the top screen */
} WBTutArrowPlace;

typedef struct WBTutStep {
    const char* name;
    const char* text;        /* NULL: no #dialogtext, the bubble is hidden */
    bool center;             /* #justify = #center */
    const char* button2;     /* the bubble's buttons */
    WBTutAction button2Action;
    const char* button1;
    WBTutAction button1Action;
    WBTutTarget target;
    int tileX;               /* #activeTile, 1-based as in the original */
    int tileY;
    bool onDelay;            /* #trigger = #delay */
    bool neverTrigger;       /* #trigger = #null */
    int delayMs;             /* #delay */
    WBTutArrow arrow;
    WBTutArrowPlace arrowPlace;
    float arrowX;
    float arrowY;
} WBTutStep;

#define TUT_NOARROW WB_TUT_ARROW_NONE, WB_TUT_AT_TARGET, 0.0f, 0.0f
#define TUT_DELAY(nm, ms) { nm, NULL, false, NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_NONE, 0, 0, true, false, ms, TUT_NOARROW }

static const WBTutStep g_tutSteps[] = {
    { "step1", "\nWelcome to Worldbuilder.\n\nAre you ready to learn the basics?", false,
      "I'm ready", WB_TUT_ACT_NEXT, "Skip tutorial", WB_TUT_ACT_QUIT, WB_TUT_NONE, 0, 0, false, false, 0, TUT_NOARROW },
    TUT_DELAY("step1_delay", 500),
    { "step2", "\nEvery mission has a goal. Let's find the goal for this mission.\n\nPress Right on the +Control Pad to look around.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_SCROLL_RIGHT, 0, 0, false, false, 0,
      WB_TUT_ARROW_RIGHT, WB_TUT_AT_BOTTOM, 296.0f, 124.0f },
    TUT_DELAY("step2_delay", 300),
    { "step3", "\n\n\nKeep pressing.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_SCROLL_RIGHT, 0, 0, false, false, 0,
      WB_TUT_ARROW_RIGHT, WB_TUT_AT_BOTTOM, 295.0f, 123.0f },
    TUT_DELAY("step3_delay", 300),
    { "stepC", "\n\n\nKeep pressing - now Down.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_SCROLL_DOWN, 0, 0, false, false, 0,
      WB_TUT_ARROW_LEFT_DOWN, WB_TUT_AT_BOTTOM, 150.0f, 196.0f },
    TUT_DELAY("stepC_delay", 500),
    { "step4", "\n\nGood, now tap that goal to see what it says.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_GOAL_LOOK, 18, 7, false, false, 0,
      WB_TUT_ARROW_RIGHT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step4_delay", NULL, false, NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_GOAL_LOOK, 18, 7, true, false, 500,
      WB_TUT_ARROW_RIGHT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step5", "\nTo win this mission you need to get a buggy to that spot.\n\nYou don't have any buggies, so let's make one.", false,
      "next", WB_TUT_ACT_NEXT, NULL, WB_TUT_ACT_NONE, WB_TUT_GOAL_LOOK, 18, 7, false, true, 0, TUT_NOARROW },
    TUT_DELAY("step5_delay", 500),
    { "step6", "\nTo make a model, you need a plan. You can pick up plans that are scattered around the map by moving over them with a model.\n\nTap this duck model.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_OCCUPANT, 13, 5, false, false, 0,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step6_delay", 500),
    { "step7", "\n\nNow, tap this plan to tell the duck to move there.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_RESOURCE_CLICK, 16, 4, false, false, 0,
      WB_TUT_ARROW_RIGHT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step7_delay", 2000),
    { "step8", "\n\nGreat, you got the plan!  When you get a plan it goes into your collection.\n\nPress R to open your collection, then pick the plan to check it out.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_PLAN, 0, 0, false, false, 0,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step8_delay", 500),
    { "step9", "\nThese numbers tell you what bricks you need to build a buggy.\n\nTo build a model you need to tap an empty space next to the required bricks.", false,
      "next", WB_TUT_ACT_NEXT, NULL, WB_TUT_ACT_NONE, WB_TUT_NONE, 0, 0, false, false, 0,
      WB_TUT_ARROW_LEFT_UP, WB_TUT_AT_TOP, 57.0f, 194.0f },
    TUT_DELAY("step9_delay", 500),
    { "step10", "\n\nTap this pile to see what bricks are in it.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_RESOURCE_LOOK, 12, 4, false, false, 300,
      WB_TUT_ARROW_LEFT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step10_delay", NULL, false, NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_RESOURCE_LOOK, 12, 4, true, false, 250,
      WB_TUT_ARROW_LEFT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step10b", "\n\nThat's just what we need to build a buggy.", false,
      "next", WB_TUT_ACT_NEXT, NULL, WB_TUT_ACT_NONE, WB_TUT_RESOURCE_LOOK, 12, 4, false, true, 0, TUT_NOARROW },
    TUT_DELAY("step10b_delay", 250),
    { "step_twelve", "\n\n\nTap next to this pile to build the buggy.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 13, 4, false, false, 0,
      WB_TUT_ARROW_LEFT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step12_delay", 500),
    { "step_thirteen", "\nGood job. You built a buggy!\n\nYou can also take models apart with their TAKE APART button, X.  Press X to take this model apart.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_NONE, 0, 0, true, false, 500, TUT_NOARROW },
    { "step_fourteen", "\nGood job. You built a buggy!\n\nYou can also take models apart with their TAKE APART button, X.  Press X to take this model apart.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TAKE_APART, 0, 0, false, false, 0, TUT_NOARROW },
    TUT_DELAY("step14_delay", 500),
    { "step_fifteen", "\nGood, now let's build another buggy - first pick the plan (R).", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_PLAN, 0, 0, false, false, 500,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step15_delay", 500),
    { "step_sixteen", "\n\n\nNow tap here.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 14, 4, false, false, 0,
      WB_TUT_ARROW_LEFT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step_sixteenA", 500),
    { "step_seventeen", "\nGood.\n\nNow, press Y, the info button, to see what the buggy can do.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_INFO_OPEN, 0, 0, false, false, 0, TUT_NOARROW },
    { "step_eighteen", NULL, false, NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_INFO_CLOSE, 0, 0, false, false, 0,
      WB_TUT_ARROW_LEFT_DOWN, WB_TUT_AT_TOP, 47.0f, 193.0f },
    TUT_DELAY("step_18_delay", 500),
    { "step_18b", "\nYou can use the buggy to carry bricks around the map.\n\nTap here to move the buggy next to the blue bricks.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 12, 4, false, false, 0,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step_18b_delay", 500),
    { "step_nineteen", "\n\n\nPress A, the PICK UP button.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_ACTION, 0, 0, false, false, 0, TUT_NOARROW },
    TUT_DELAY("step_19_delay", 500),
    { "step_twenty", "\n\n\nTap these bricks.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 12, 3, false, false, 800,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step_20_delay", 500),
    { "step_20b", "\nGood! You picked up some blue bricks.\n\nNow let's move them somewhere else.\n\nTap here to move the buggy.", false,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 15, 4, false, false, 0,
      WB_TUT_ARROW_RIGHT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step_tone", "\n\nNow press A to DROP OFF.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_ACTION, 0, 0, false, false, 0, TUT_NOARROW },
    TUT_DELAY("step_21_delay", 500),
    { "step_ttwo", "\n\nAnd drop the bricks off here.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_TILE, 15, 3, false, false, 0,
      WB_TUT_ARROW_LEFT_DOWN, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    { "step_tthree", "\n\nNow that you know the basics, head for the goal and finish the mission.", true,
      NULL, WB_TUT_ACT_NONE, NULL, WB_TUT_ACT_NONE, WB_TUT_GOAL_CLICK, 18, 7, false, false, 0,
      WB_TUT_ARROW_LEFT_UP, WB_TUT_AT_TARGET, 0.0f, 0.0f },
    TUT_DELAY("step23_delay", 2500),
    { "step_end", "\nGood work.  You solved the mission!\n\nPress A to FINISH, then B to END MISSION and return to the world map, or A to keep searching for this level's bonus goal.", false,
      "finish", WB_TUT_ACT_FINISH, NULL, WB_TUT_ACT_NONE, WB_TUT_NONE, 0, 0, false, false, 0, TUT_NOARROW }
};
#define WB_TUT_STEP_COUNT ((int) (sizeof(g_tutSteps) / sizeof(g_tutSteps[0])))

/* arrow_offsets[shape][clicktarget] from 'tutorial manager', in stage px, measured
   from the click rect's top-left: tile, occupant, goal, resource, plan. */
static const WBAnchor g_tutArrowOffsets[5][5] = {
    { {80.0f, -21.0f}, {-10.0f, -25.0f}, {-14.0f, -17.0f}, {-10.0f, -25.0f}, {-10.0f, -25.0f} },  /* left_down */
    { {-10.0f, -25.0f}, {-10.0f, -25.0f}, {37.0f, 68.0f}, {55.0f, 45.0f}, {-10.0f, -25.0f} },     /* left_up */
    { {-10.0f, -21.0f}, {-10.0f, -25.0f}, {-14.0f, -17.0f}, {-10.0f, -25.0f}, {-10.0f, -25.0f} }, /* right */
    { {7.0f, -25.0f}, {-10.0f, -25.0f}, {-14.0f, -17.0f}, {-10.0f, -25.0f}, {-10.0f, -25.0f} },   /* right_down */
    { {-20.0f, 60.0f}, {-20.0f, 60.0f}, {-20.0f, 60.0f}, {-15.0f, 60.0f}, {0.0f, 0.0f} }          /* right_up */
};

static void tutorialQuit(AppState* app);

static void tutorialShowStep(AppState* app, int index) {
    const WBTutStep* step;
    u64 now = osGetTime();
    bool hadText = app->tutorialStep >= 0 && app->tutorialStep < WB_TUT_STEP_COUNT && g_tutSteps[app->tutorialStep].text != NULL;
    app->tutorialStep = index;
    step = &g_tutSteps[index];
    app->tutorialAdvanceAtMs = 0;
    app->tutorialCatcherLive = step->target != WB_TUT_NONE;
    /* Leaving a goal or pile step is the mouseLeave that closes its rollover. */
    if (step->target != WB_TUT_GOAL_LOOK && app->goalPopupVisible && !app->goalPopupComplete) {
        app->goalPopupVisible = false;
    }
    if (step->target != WB_TUT_RESOURCE_LOOK) {
        app->pileInspectValid = false;
    }
    if (step->text && !hadText) {
        app->tutorialDialogShownMs = now;
    } else if (!step->text && hadText) {
        app->tutorialDialogHiddenMs = now;
    }
    if (step->onDelay) {
        app->tutorialAdvanceAtMs = now + (u64) (step->delayMs > 0 ? step->delayMs : 1);
    }
}

static void tutorialGoNext(AppState* app, int delayMs) {
    if (delayMs > 0) {
        app->tutorialAdvanceAtMs = osGetTime() + (u64) delayMs;
        return;
    }
    if (app->tutorialStep + 1 < WB_TUT_STEP_COUNT) {
        tutorialShowStep(app, app->tutorialStep + 1);
    } else {
        tutorialQuit(app);
    }
}

/* The click hole saw its trigger: disable it and move on after the step's delay. */
static void tutorialFire(AppState* app) {
    const WBTutStep* step = &g_tutSteps[app->tutorialStep];
    if (!app->tutorialCatcherLive || step->neverTrigger || step->onDelay) {
        return;
    }
    app->tutorialCatcherLive = false;
    tutorialGoNext(app, step->delayMs);
}

static void tutorialStart(AppState* app) {
    app->tutorialActive = true;
    app->tutorialStep = -1;
    app->tutorialDialogShownMs = 0;
    app->tutorialDialogHiddenMs = 0;
    tutorialShowStep(app, 0);
}

static void returnToWorldSelect(AppState* app) {
    freeGameplaySheets(app);
    ensureMenuSheetsLoaded(app);
    switchWorldSelectSheets(app, app->activeWorld);
    app->screenMode = WB_SCREEN_WORLD_SELECT;
    app->worldSelectWorld = app->activeWorld;
    app->hasSelection = false;
    app->infoOverlayOpen = false;
    app->armedPlan = WB_PLAN_NONE;
    app->planMenuOpen = false;
    app->startMenuOpen = false;
    app->goalPopupVisible = false;
    app->goalPopupComplete = false;
    app->goalPopupBonus = false;
    app->pileInspectValid = false;
    initWorldMiniWalkers(app, app->worldSelectWorld);
    startMenuMusic(app);
}

/* quit(): setTutorialMode(0), then quitlevel() back to the world map. */
static void tutorialQuit(AppState* app) {
    app->tutorialActive = false;
    returnToWorldSelect(app);
    playSfxClip(app, &app->sfxMove);
}

static void tutorialRunAction(AppState* app, WBTutAction action) {
    playSfxClip(app, &app->sfxMove);
    switch (action) {
        case WB_TUT_ACT_NEXT: tutorialGoNext(app, 0); break;
        case WB_TUT_ACT_QUIT: tutorialQuit(app); break;
        case WB_TUT_ACT_FINISH: app->tutorialActive = false; break;
        default: break;
    }
}

static bool tutorialTouchOnTile(const AppState* app, const touchPosition* touch, int tileX, int tileY) {
    int tx;
    int ty;
    return screenToTile(app, (float) touch->px, (float) touch->py, &tx, &ty) && tx == tileX - 1 && ty == tileY - 1;
}

/* The tutorial's mouse mask: only the current click hole reaches the game. Returns
   false while the plan collection is open, which pauses the game as it does normally. */
static bool tutorialHandleInput(AppState* app, u32 kDown, u32 kHeld, const touchPosition* touch) {
    const WBTutStep* step = &g_tutSteps[app->tutorialStep];
    bool touchDown = (kHeld & KEY_TOUCH) && !app->touchActiveLast;
    app->touchActiveLast = (kHeld & KEY_TOUCH) != 0;

    if (kDown & KEY_START) {               /* the 'quit tutorial' button */
        tutorialQuit(app);
        return false;
    }
    if (step->text) {
        if ((kDown & KEY_A) && step->button2) {
            tutorialRunAction(app, step->button2Action);
            return true;
        }
        if ((kDown & KEY_B) && step->button1) {
            tutorialRunAction(app, step->button1Action);
            return true;
        }
    }
    if (app->planMenuOpen) {
        int pmCount = planMenuEntryCount(app);
        if ((kDown & KEY_DOWN) && pmCount > 0) {
            app->planMenuCursor = (app->planMenuCursor + 1) % pmCount;
            if (app->planMenuCursor >= app->planMenuScrollOffset + 3) app->planMenuScrollOffset = app->planMenuCursor - 2;
            playSfxClip(app, &app->sfxMove);
        }
        if ((kDown & KEY_UP) && pmCount > 0) {
            app->planMenuCursor = (app->planMenuCursor + pmCount - 1) % pmCount;
            if (app->planMenuCursor < app->planMenuScrollOffset) app->planMenuScrollOffset = app->planMenuCursor;
            playSfxClip(app, &app->sfxMove);
        }
        if (kDown & KEY_A) {
            WBPlanType pmPlan = planMenuEntryAt(app, app->planMenuCursor);
            if (pmPlan != WB_PLAN_NONE && app->map.planInventory[pmPlan] > 0) app->armedPlan = pmPlan;
            app->planMenuOpen = false;
            playSfxClip(app, &app->sfxPlan);
        }
        if (kDown & (KEY_B | KEY_R | KEY_SELECT)) {
            app->planMenuOpen = false;
            playSfxClip(app, &app->sfxMove);
        }
        if (touchDown) {
            handlePlanMenuTouch(app, touch);
        }
        if (step->target == WB_TUT_PLAN && app->armedPlan != WB_PLAN_NONE) {
            tutorialFire(app);
        }
        return false;
    }
    if (!app->tutorialCatcherLive && !step->neverTrigger) {
        return true;
    }
    switch (step->target) {
        case WB_TUT_SCROLL_RIGHT:
        case WB_TUT_SCROLL_DOWN: {
            bool right = step->target == WB_TUT_SCROLL_RIGHT;
            if (kDown & (right ? KEY_RIGHT : KEY_DOWN)) {
                app->lastScrollMs = 0;
                scrollCameraStep(app, right ? 1 : 0, right ? 0 : 1);
                tutorialFire(app);
            }
            break;
        }
        case WB_TUT_GOAL_LOOK:
            if (touchDown && tutorialTouchOnTile(app, touch, step->tileX, step->tileY)) {
                showGoalPopupForCell(app, step->tileX - 1, step->tileY - 1);
                tutorialFire(app);
            }
            break;
        case WB_TUT_RESOURCE_LOOK:
            if (touchDown && tutorialTouchOnTile(app, touch, step->tileX, step->tileY)) {
                updatePileInspect(app, step->tileX - 1, step->tileY - 1);
                tutorialFire(app);
            }
            break;
        case WB_TUT_OCCUPANT:
            if (touchDown && trySelectUnitAtTouch(app, touch)) {
                if (app->selectedX == step->tileX - 1 && app->selectedY == step->tileY - 1) {
                    tutorialFire(app);
                } else {
                    app->hasSelection = false;
                }
            }
            break;
        case WB_TUT_GOAL_CLICK:
        case WB_TUT_RESOURCE_CLICK:
        case WB_TUT_TILE:
            if (touchDown && tutorialTouchOnTile(app, touch, step->tileX, step->tileY)) {
                handleWorldTap(app, step->tileX - 1, step->tileY - 1);
                tutorialFire(app);
            }
            break;
        case WB_TUT_PLAN:
            if (kDown & (KEY_R | KEY_SELECT)) {
                app->planMenuOpen = true;
                app->planMenuScrollOffset = 0;
                app->planMenuCursor = 0;
                app->infoOverlayOpen = false;
                playSfxClip(app, &app->sfxPlan);
            }
            break;
        case WB_TUT_TAKE_APART:
            if ((kDown & KEY_X) && app->hasSelection) {
                disassembleSelected(app);
                tutorialFire(app);
            }
            break;
        case WB_TUT_INFO_OPEN:
            if ((kDown & KEY_Y) && app->hasSelection) {
                app->infoOverlayOpen = true;
                playSfxClip(app, &app->sfxMove);
                tutorialFire(app);
            }
            break;
        case WB_TUT_INFO_CLOSE:
            if ((kDown & KEY_Y) && app->infoOverlayOpen) {
                app->infoOverlayOpen = false;
                playSfxClip(app, &app->sfxMove);
                tutorialFire(app);
            }
            break;
        case WB_TUT_ACTION:
            if (kDown & (KEY_A | KEY_L)) {
                handleActionButton(app);
                if (app->actionMode == WB_ACTION_PICK || app->actionMode == WB_ACTION_DROP) {
                    tutorialFire(app);
                }
            }
            break;
        default:
            break;
    }
    return true;
}

static void tutorialUpdate(AppState* app) {
    if (!app->tutorialActive) {
        return;
    }
    if (app->tutorialAdvanceAtMs != 0 && osGetTime() >= app->tutorialAdvanceAtMs) {
        app->tutorialAdvanceAtMs = 0;
        tutorialGoNext(app, 0);
    }
}

/* 'tutorial colorflip': the outline swaps black / orange every 4 frames at 15 fps. */
static void tutorialDrawArrow(AppState* app, WBTutArrow shape, float x, float y, float scale) {
    bool orange = ((osGetTime() * 15ULL / 1000ULL) / 4ULL) % 2ULL == 0;
    C2D_Image img = getWorldbuilderExtraImage(app, (orange ? WB_EXTRA_TUT_ARROW_ORANGE_IDX : WB_EXTRA_TUT_ARROW_IDX) + (int) shape);
    if (!img.subtex) {
        return;
    }
    drawAnchoredImage(img, x, y, scale, g_tutArrowReg[shape].x, g_tutArrowReg[shape].y);
}

/* The click rect's top-left on the bottom screen (tileRect / goalRect / objRect /
   the occupant's sprite rect, as 'tutorial manager' builds them). */
static bool tutorialTargetRect(AppState* app, const WBTutStep* step, float* outX, float* outY, int* outKind) {
    float locX;
    float locY;
    switch (step->target) {
        case WB_TUT_TILE:
            posToLoc(app, step->tileX, step->tileY, &locX, &locY);
            *outX = locX - 25.0f * DIRECTOR_SCALE; *outY = locY; *outKind = 0;
            return true;
        case WB_TUT_OCCUPANT: {
            const WBCell* cell;
            int assetId;
            if (!inBounds(app, step->tileX - 1, step->tileY - 1)) return false;
            cell = &app->map.cells[step->tileY - 1][step->tileX - 1];
            if (!cell->hasUnit) return false;
            assetId = unitStandAssetId(cell->unitType, cell->unitDirection, terrainUsesWaterSprite(cell->terrain));
            if (assetId < 0) return false;
            posToLoc(app, step->tileX, step->tileY, &locX, &locY);
            *outX = locX + 10.0f * DIRECTOR_SCALE - g_objectAnchors[assetId].x * DIRECTOR_SCALE - 1.0f;
            *outY = locY + 18.0f * DIRECTOR_SCALE - g_objectAnchors[assetId].y * DIRECTOR_SCALE - 1.0f;
            *outKind = 1;
            return true;
        }
        case WB_TUT_GOAL_LOOK:
        case WB_TUT_GOAL_CLICK:
            posToLoc(app, step->tileX, step->tileY, &locX, &locY);
            *outX = locX; *outY = locY - 30.0f * DIRECTOR_SCALE; *outKind = 2;
            return true;
        case WB_TUT_RESOURCE_CLICK:
        case WB_TUT_RESOURCE_LOOK:
            posToLoc(app, step->tileX, step->tileY, &locX, &locY);
            *outX = locX; *outY = locY; *outKind = 3;
            return true;
        case WB_TUT_PLAN: {
            /* No plan bar on the 3DS: point at the plan's row once the collection is open. */
            int i;
            int count = planMenuEntryCount(app);
            if (!app->planMenuOpen) return false;
            for (i = app->planMenuScrollOffset; i < count && i < app->planMenuScrollOffset + 3; ++i) {
                if (app->map.planInventory[planMenuEntryAt(app, i)] > 0) {
                    *outX = 26.0f + 16.0f + 20.0f;
                    *outY = 34.0f + 56.0f + (float) (i - app->planMenuScrollOffset) * 34.0f + 2.0f;
                    *outKind = 4;
                    return true;
                }
            }
            return false;
        }
        default:
            return false;
    }
}

static void tutorialDrawBottom(AppState* app) {
    const WBTutStep* step;
    float x;
    float y;
    int kind;
    if (!app->tutorialActive) {
        return;
    }
    step = &g_tutSteps[app->tutorialStep];
    if (step->arrow == WB_TUT_ARROW_NONE || step->arrowPlace == WB_TUT_AT_TOP) {
        return;
    }
    if (step->arrowPlace == WB_TUT_AT_BOTTOM) {
        tutorialDrawArrow(app, step->arrow, step->arrowX, step->arrowY, DIRECTOR_SCALE);
        return;
    }
    if (tutorialTargetRect(app, step, &x, &y, &kind)) {
        x += g_tutArrowOffsets[step->arrow][kind].x * DIRECTOR_SCALE;
        y += g_tutArrowOffsets[step->arrow][kind].y * DIRECTOR_SCALE;
        tutorialDrawArrow(app, step->arrow, x, y, kind == 4 ? 0.75f : DIRECTOR_SCALE);
    }
}

/* The dialog bubble ('tutorial_bubble': white, black rounded border, grey title),
   kept to the right of the sidebar so the plan recipe and info stay visible. It fades
   in and out over 200 ms, as 'tutorial fade behavior' does. */
static void tutorialDrawTop(AppState* app) {
    const WBTutStep* step;
    const float bx = 206.0f;
    const float by = 4.0f;
    const float bw = 190.0f;
    const float bh = 232.0f;
    const float textScale = 0.38f;
    const float lineStep = 13.0f;
    const int maxChars = 31;
    u64 now = osGetTime();
    float alpha;
    u8 a;
    char line[128];
    const char* p;
    float ty;
    if (!app->tutorialActive) {
        return;
    }
    step = &g_tutSteps[app->tutorialStep];
    if (step->arrow != WB_TUT_ARROW_NONE && step->arrowPlace == WB_TUT_AT_TOP) {
        tutorialDrawArrow(app, step->arrow, step->arrowX, step->arrowY, 1.0f);
    }
    if (!step->text) {
        return;
    }
    alpha = (float) (now - app->tutorialDialogShownMs) / 200.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    if (alpha < 0.0f) alpha = 0.0f;
    a = (u8) (alpha * 255.0f);
    C2D_DrawRectSolid(bx, by, 0.6f, bw, bh, C2D_Color32(0x00, 0x00, 0x00, a));
    C2D_DrawRectSolid(bx + 2.0f, by + 2.0f, 0.6f, bw - 4.0f, bh - 4.0f, C2D_Color32(0xFF, 0xFF, 0xFF, a));
    {
        C2D_Text t;
        C2D_TextParse(&t, app->dynamicBuf, "Mission 1: Tutorial");
        C2D_TextOptimize(&t);
        C2D_DrawText(&t, C2D_WithColor, bx + 8.0f, by + 5.0f, 0.6f, 0.44f, 0.44f, C2D_Color32(0xA8, 0xA8, 0xA8, a));
    }
    /* body: the #dialogtext lines, word-wrapped, left or centred */
    ty = by + 20.0f;
    p = step->text;
    while (*p && ty < by + bh - 50.0f) {
        const char* eol = strchr(p, '\n');
        int len = eol ? (int) (eol - p) : (int) strlen(p);
        if (len == 0) {
            ty += lineStep;
        } else {
            int start = 0;
            while (start < len) {
                int take = len - start;
                C2D_Text t;
                if (take > maxChars) {
                    int cut = start + maxChars;
                    while (cut > start && p[cut] != ' ') --cut;
                    take = (cut > start) ? cut - start : maxChars;
                }
                if (take > (int) sizeof(line) - 1) take = (int) sizeof(line) - 1;
                memcpy(line, p + start, (size_t) take);
                line[take] = '\0';
                C2D_TextParse(&t, app->dynamicBuf, line);
                C2D_TextOptimize(&t);
                if (step->center) {
                    C2D_DrawText(&t, C2D_WithColor | C2D_AlignCenter, bx + bw * 0.5f, ty, 0.6f, textScale, textScale, C2D_Color32(0x00, 0x00, 0x00, a));
                } else {
                    C2D_DrawText(&t, C2D_WithColor, bx + 9.0f, ty, 0.6f, textScale, textScale, C2D_Color32(0x00, 0x00, 0x00, a));
                }
                ty += lineStep;
                start += take;
                while (start < len && p[start] == ' ') ++start;
            }
        }
        if (!eol) break;
        p = eol + 1;
    }
    /* buttons: the green 'large_green_button's, labelled with the 3DS button */
    {
        float btnY = by + bh - 44.0f;
        int nButtons = (step->button1 ? 1 : 0) + (step->button2 ? 1 : 0);
        int i;
        for (i = 0; i < 2; ++i) {
            const char* label = i == 0 ? step->button1 : step->button2;
            const char* key = i == 0 ? "B" : "A";
            float w = nButtons == 2 ? (bw - 24.0f) * 0.5f : bw - 60.0f;
            float x;
            char text[48];
            C2D_Text t;
            if (!label) continue;
            x = nButtons == 2 ? (i == 0 ? bx + 8.0f : bx + 16.0f + w) : bx + 30.0f;
            C2D_DrawRectSolid(x, btnY, 0.6f, w, 20.0f, C2D_Color32(0x1E, 0x6B, 0x16, a));
            C2D_DrawRectSolid(x + 1.0f, btnY + 1.0f, 0.6f, w - 2.0f, 18.0f, C2D_Color32(0x3C, 0xB4, 0x28, a));
            snprintf(text, sizeof(text), "%s: %s", key, label);
            C2D_TextParse(&t, app->dynamicBuf, text);
            C2D_TextOptimize(&t);
            C2D_DrawText(&t, C2D_WithColor | C2D_AlignCenter, x + w * 0.5f, btnY + 4.0f, 0.6f, 0.36f, 0.36f, C2D_Color32(0xFF, 0xFF, 0xFF, a));
        }
    }
    {
        C2D_Text t;
        C2D_TextParse(&t, app->dynamicBuf, "START: quit tutorial");
        C2D_TextOptimize(&t);
        C2D_DrawText(&t, C2D_WithColor | C2D_AlignCenter, bx + bw * 0.5f, by + bh - 17.0f, 0.6f, 0.32f, 0.32f, C2D_Color32(0x80, 0x80, 0x80, a));
    }
}

int main(int argc, char* argv[]) {
    AppState* app = &g_app;
    C3D_RenderTarget* top;
    C3D_RenderTarget* bottom;
    Result romfsResult;

    (void) argc;
    (void) argv;

    memset(app, 0, sizeof(*app));
    srand((unsigned int) osGetTime());

    romfsResult = romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    app->audioReady = initAudioSystem(app);

    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    if (R_SUCCEEDED(romfsResult)) {
        app->spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/worldbuilder.t3x");
        app->titleMainSheet = C2D_SpriteSheetLoad("romfs:/gfx/titlemain.t3x");
        app->worldSkySheets[0] = C2D_SpriteSheetLoad("romfs:/gfx/worldsky_a.t3x");
        app->worldSkySheets[1] = C2D_SpriteSheetLoad("romfs:/gfx/worldsky_b.t3x");
        app->worldSkySheets[2] = C2D_SpriteSheetLoad("romfs:/gfx/worldsky_c.t3x");
        /* worldsky_d loaded lazily when navigating to worlds 6/7 */
        app->worldMapSheets[0] = C2D_SpriteSheetLoad("romfs:/gfx/worldmap_a.t3x");
        app->worldMapSheets[1] = C2D_SpriteSheetLoad("romfs:/gfx/worldmap_b.t3x");
        app->worldMapSheets[2] = C2D_SpriteSheetLoad("romfs:/gfx/worldmap_c.t3x");
        /* worldmap_d loaded lazily when navigating to worlds 6/7 */
        app->worldIconSheet = C2D_SpriteSheetLoad("romfs:/gfx/worldicons.t3x");
        if (app->spriteSheet && C2D_SpriteSheetCount(app->spriteSheet) > 0) {
            C2D_Image firstImage = C2D_SpriteSheetGetImage(app->spriteSheet, 0);
            C3D_TexSetFilter(firstImage.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(firstImage.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->titleMainSheet && C2D_SpriteSheetCount(app->titleMainSheet) > 0) {
            C2D_Image firstTitleImage = C2D_SpriteSheetGetImage(app->titleMainSheet, 0);
            C3D_TexSetFilter(firstTitleImage.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(firstTitleImage.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldSkySheets[0] && C2D_SpriteSheetCount(app->worldSkySheets[0]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldSkySheets[0], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldSkySheets[1] && C2D_SpriteSheetCount(app->worldSkySheets[1]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldSkySheets[1], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldSkySheets[2] && C2D_SpriteSheetCount(app->worldSkySheets[2]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldSkySheets[2], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldSkySheets[3] && C2D_SpriteSheetCount(app->worldSkySheets[3]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldSkySheets[3], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldMapSheets[0] && C2D_SpriteSheetCount(app->worldMapSheets[0]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldMapSheets[0], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldMapSheets[1] && C2D_SpriteSheetCount(app->worldMapSheets[1]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldMapSheets[1], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldMapSheets[2] && C2D_SpriteSheetCount(app->worldMapSheets[2]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldMapSheets[2], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldMapSheets[3] && C2D_SpriteSheetCount(app->worldMapSheets[3]) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldMapSheets[3], 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
        if (app->worldIconSheet && C2D_SpriteSheetCount(app->worldIconSheet) > 0) {
            C2D_Image img = C2D_SpriteSheetGetImage(app->worldIconSheet, 0);
            C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
            C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
    }
    /* Gameplay sheets (vehicles/monsters/buildings/whirlpool) are loaded lazily
       in loadGameplaySheets() when entering a mission to stay within VRAM limits. */

    {
        FILE* mapTestFp = fopen("romfs:/maps/map1_1.txt", "rb");
        bool mapFileExists = (mapTestFp != NULL);
        bool mapLoaded = false;
        if (mapTestFp) { fclose(mapTestFp); mapLoaded = wbMapLoad("romfs:/maps/map1_1.txt", &app->map); }
        bool startupOk = R_SUCCEEDED(romfsResult) && app->spriteSheet && app->titleMainSheet &&
            app->worldSkySheets[0] && app->worldSkySheets[1] && app->worldSkySheets[2] &&
            app->worldMapSheets[0] && app->worldMapSheets[1] && app->worldMapSheets[2] &&
            app->worldIconSheet &&
            mapLoaded;
        if (!startupOk) {
        consoleInit(GFX_TOP, NULL);
        printf("Startup diagnostics:\n");
        printf("romfsInit: %s (0x%08lX)\n", R_FAILED(romfsResult) ? "FAIL" : "OK", (unsigned long)romfsResult);
        printf("spriteSheet: %s\n", app->spriteSheet ? "OK" : "FAIL");
        printf("titleMain: %s\n", app->titleMainSheet ? "OK" : "FAIL");
        printf("worldSky: %s/%s/%s/%s\n", app->worldSkySheets[0] ? "OK" : "FAIL", app->worldSkySheets[1] ? "OK" : "FAIL", app->worldSkySheets[2] ? "OK" : "FAIL", app->worldSkySheets[3] ? "OK" : "FAIL");
        printf("worldMap: %s/%s/%s/%s\n", app->worldMapSheets[0] ? "OK" : "FAIL", app->worldMapSheets[1] ? "OK" : "FAIL", app->worldMapSheets[2] ? "OK" : "FAIL", app->worldMapSheets[3] ? "OK" : "FAIL");
        printf("worldIcon: %s\n", app->worldIconSheet ? "OK" : "FAIL");
        printf("map fopen: %s\n", mapFileExists ? "OK" : "FAIL (no file)");
        printf("map load: %s\n", mapLoaded ? "OK" : (mapFileExists ? "FAIL (parse)" : "FAIL (no file)"));
        if (mapFileExists && !mapLoaded) {
            FILE* dbgFp = fopen("romfs:/maps/map1_1.txt", "r");
            if (dbgFp) {
                char dbgLine[64]; int i; int lc = 0; int mapSec = 0; int mapRows = 0;
                while (fgets(dbgLine, sizeof(dbgLine), dbgFp) && lc < 100) {
                    size_t len = strlen(dbgLine);
                    while (len > 0 && (dbgLine[len-1]=='\r' || dbgLine[len-1]=='\n' || dbgLine[len-1]==' ')) dbgLine[--len] = '\0';
                    ++lc;
                    if (strcmp(dbgLine, "[map]") == 0) mapSec = 1;
                    if (mapSec && (strncmp(dbgLine, " map=", 5)==0 || strncmp(dbgLine, "map=", 4)==0)) ++mapRows;
                }
                fclose(dbgFp);
                printf("lines=%d mapSec=%d mapRows=%d\n", lc, mapSec, mapRows);
                dbgFp = fopen("romfs:/maps/map1_1.txt", "r");
                if (dbgFp) {
                    fgets(dbgLine, sizeof(dbgLine), dbgFp);
                    fclose(dbgFp);
                    printf("line1(%d):", (int)strlen(dbgLine));
                    for (i = 0; i < 8 && i < (int)strlen(dbgLine); ++i) printf("%02X", (unsigned char)dbgLine[i]);
                    printf("\n");
                }
            }
        }
        printf("\nFailed to load sprite sheet or map.\n");
        printf("Press START to exit.\n");
        while (aptMainLoop()) {
            hidScanInput();
            if (hidKeysDown() & KEY_START) break;
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        romfsExit();
        return 1;
        }
    }

    sceneInit(app);
    /* Global audio: base music + UI/interaction/goal SFX needed on every screen.
       Per-level SFX (unit sounds, building sounds, terrain actions, music variants)
       are freed and reloaded fresh each time a level starts via loadLevelAudio(). */
    app->loadedClipCount += loadWavClip("romfs:/audio/music_intro.wav", &app->musicIntro) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_game.wav",  &app->musicGame)  ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_button.wav",           &app->sfxButton)         ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_rollover.wav",          &app->sfxRollover)       ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_world_coming_soon.wav", &app->sfxWorldComingSoon) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_move.wav",         &app->sfxMove)        ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_plan.wav",         &app->sfxPlan)        ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_pickup_plan.wav",  &app->sfxPickupPlan)  ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_pickup.wav",       &app->sfxPickup)      ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_drop.wav",         &app->sfxDrop)        ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_damage.wav",       &app->sfxDamage)      ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_disassemble.wav",  &app->sfxDisassemble) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_assembly.wav",     &app->sfxAssembly)    ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_move_misc.wav",    &app->sfxMoveMisc)    ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_goal.wav",         &app->sfxGoal)        ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/sfx_bonus_goal.wav",   &app->sfxBonusGoal)   ? 1 : 0;
    /* Intro music variants (title/world-map screen) — optional, load after core sounds */
    app->loadedClipCount += loadWavClip("romfs:/audio/music_intro_1.wav", &app->musicIntroVariants[0]) ? 1 : 0;
    app->loadedClipCount += loadWavClip("romfs:/audio/music_intro_2.wav", &app->musicIntroVariants[1]) ? 1 : 0;
    app->linearFreeKB = (int)(linearSpaceFree() / 1024);
    if (app->audioReady) {
        startMenuMusic(app);
    }
    app->cameraX = 1;
    app->cameraY = 1;
    clampCamera(app);

    while (aptMainLoop()) {
        circlePosition circle;
        touchPosition touch;
        u32 kDown;
        u32 kHeld;

        hidScanInput();
        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        hidCircleRead(&circle);
        hidTouchRead(&touch);
        if (app->screenMode != WB_SCREEN_GAME) {
            /* Konami code detection (title screen only) */
            if (app->screenMode == WB_SCREEN_TITLE && !app->debugMenuOpen) {
                if (kDown != 0) {
                    if (kDown & g_konamiSeq[app->konamiProgress]) {
                        app->konamiProgress += 1;
                        if (app->konamiProgress >= WB_KONAMI_LEN) {
                            app->konamiProgress = 0;
                            app->debugMenuOpen = true;
                            app->debugMenuCursor = 0;
                        }
                    } else {
                        /* Restart from 0, but also check if this key restarts the sequence */
                        app->konamiProgress = (kDown & g_konamiSeq[0]) ? 1 : 0;
                    }
                }
            }
            /* Debug menu navigation */
            if (app->debugMenuOpen && app->screenMode == WB_SCREEN_TITLE) {
                if (kDown & KEY_DOWN) {
                    app->debugMenuCursor = (app->debugMenuCursor + 1) % 4;
                }
                if (kDown & KEY_UP) {
                    app->debugMenuCursor = (app->debugMenuCursor + 3) % 4;
                }
                if (kDown & (KEY_A | KEY_B)) {
                    if ((kDown & KEY_B) || app->debugMenuCursor == 3) {
                        app->debugMenuOpen = false;
                        app->konamiProgress = 0;
                    } else if (kDown & KEY_A) {
                        switch (app->debugMenuCursor) {
                            case 0: /* Clear Save Data */
                                clearSaveData(app);
                                break;
                            case 1: /* Unlock All Levels */
                                unlockAllLevels(app);
                                break;
                            case 2: /* Toggle Debug Info */
                                app->showDiagnostics = !app->showDiagnostics;
                                break;
                            default: break;
                        }
                    }
                }
                /* Suppress touch-to-start while debug menu is open */
            } else {
            if ((kDown & KEY_B) != 0 && app->screenMode == WB_SCREEN_WORLD_SELECT) {
                app->screenMode = WB_SCREEN_TITLE;
                app->titleCycleStartMs = 0; /* restart cycle from WB1 */
                startMenuMusic(app);
                playSfxClip(app, &app->sfxMove);
            }
            /* Worldmap button navigation */
            if (app->screenMode == WB_SCREEN_WORLD_SELECT) {
                /* SELECT: toggle Builder's License overlay (WB1 when world 3 done; WB2 when world 6 done) */
                if (kDown & KEY_SELECT) {
                    bool isWb2World = (app->worldSelectWorld >= 6);
                    bool hasLicense = isWb2World ? wb2LicenseUnlocked(app) : licenseUnlocked(app);
                    if (hasLicense) {
                        app->licenseVisible = !app->licenseVisible;
                        playSfxClip(app, &app->sfxMove);
                    } else {
                        /* Not yet unlocked — play "coming soon" cue like original */
                        playSfxClip(app, app->sfxWorldComingSoon.loaded ? &app->sfxWorldComingSoon : &app->sfxMove);
                    }
                }
                /* L / ZL: previous world */
                if (kDown & (KEY_L | KEY_ZL)) {
                    app->worldSelectWorld = nextWorldInDirection(app, app->worldSelectWorld, -1);
                    switchWorldSelectSheets(app, app->worldSelectWorld);
                    initWorldMiniWalkers(app, app->worldSelectWorld);
                    app->hoverMissionLevel = 0;
                    playSfxClip(app, &app->sfxMove);
                }
                /* R / ZR: next world */
                if (kDown & (KEY_R | KEY_ZR)) {
                    app->worldSelectWorld = nextWorldInDirection(app, app->worldSelectWorld, 1);
                    switchWorldSelectSheets(app, app->worldSelectWorld);
                    initWorldMiniWalkers(app, app->worldSelectWorld);
                    app->hoverMissionLevel = 0;
                    playSfxClip(app, &app->sfxMove);
                }
                /* D-pad: cycle through available mission markers */
                if (kDown & (KEY_RIGHT | KEY_DOWN)) {
                    int cur = app->hoverMissionLevel;
                    int next = cur;
                    int m;
                    for (m = 1; m <= 12; ++m) {
                        int candidate = (cur % 12) + m;
                        if (candidate > 12) candidate -= 12;
                        if (worldMissionState(app, app->worldSelectWorld, candidate) >= 0) {
                            next = candidate;
                            break;
                        }
                    }
                    app->hoverMissionLevel = next;
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & (KEY_LEFT | KEY_UP)) {
                    int cur = app->hoverMissionLevel;
                    int next = cur;
                    int m;
                    for (m = 1; m <= 12; ++m) {
                        int candidate = ((cur - 2 + 12) % 12) + 1 - m + 1;
                        candidate = ((cur - 1 - m + 12 * 2) % 12) + 1;
                        if (worldMissionState(app, app->worldSelectWorld, candidate) >= 0) {
                            next = candidate;
                            break;
                        }
                    }
                    app->hoverMissionLevel = next;
                    playSfxClip(app, &app->sfxMove);
                }
                /* A: launch selected mission */
                if (kDown & KEY_A) {
                    int sel = app->hoverMissionLevel;
                    if (sel >= 1 && sel <= 12 && worldMissionState(app, app->worldSelectWorld, sel) >= 0) {
                        if (packagedMissionExists(app->worldSelectWorld, sel)) {
                            playSfxClip(app, &app->sfxMove);
                            loadMissionIntoGame(app, app->worldSelectWorld, sel);
                        } else {
                            playSfxClip(app, app->sfxWorldComingSoon.loaded ? &app->sfxWorldComingSoon : &app->sfxMove);
                        }
                    }
                }
            }
            if (kHeld & KEY_TOUCH) {
                if (!app->worldSelectTouchActive) {
                    if (app->screenMode == WB_SCREEN_TITLE) {
                        handleTitleScreenTouch(app, &touch);
                    } else {
                        handleWorldSelectTouch(app, &touch);
                    }
                }
                app->worldSelectTouchActive = true;
            } else {
                app->worldSelectTouchActive = false;
            }
            updateWorldMiniWalkers(app);
            updateAudioRuntime(app);
            }
        } else {
            if (app->tutorialActive) {
                bool simulate;
                tutorialUpdate(app);
                simulate = app->tutorialActive ? tutorialHandleInput(app, kDown, kHeld, &touch) : true;
                if (app->screenMode != WB_SCREEN_GAME) {
                    continue;
                }
                if (!simulate) {
                    updateAudioRuntime(app);
                    goto doneGameInput;
                }
                goto runGameSimulation;
            }
            if (kDown & KEY_START) {
                if (app->goalPopupVisible && app->goalPopupComplete) {
                    app->goalPopupVisible = false;
                    app->goalPopupComplete = false;
                    app->goalPopupBonus = false;
                }
                app->startMenuOpen = !app->startMenuOpen;
                app->startMenuCursor = 0;
                app->planMenuOpen = false;
                app->infoOverlayOpen = false;
                playSfxClip(app, &app->sfxMove);
            }
            /* ── Start-menu button navigation (takes priority over game controls) ── */
            if (app->startMenuOpen) {
                char smPath[64];
                if (kDown & KEY_DOWN) {
                    app->startMenuCursor = (app->startMenuCursor + 1) % 5;
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & KEY_UP) {
                    app->startMenuCursor = (app->startMenuCursor + 4) % 5;
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & KEY_B) {
                    app->startMenuOpen = false;
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & KEY_A) {
                    switch (app->startMenuCursor) {
                        case 0: /* Continue Mission */
                            app->startMenuOpen = false;
                            playSfxClip(app, &app->sfxMove);
                            break;
                        case 1: /* Restart Mission */
                            snprintf(smPath, sizeof(smPath), "romfs:/maps/map%d_%d.txt", app->activeWorld, app->activeMission);
                            wbMapLoad(smPath, &app->map);
                            resetCameraForMap(app);
                            app->pileInspectValid = false;
                            app->hasSelection = false;
                            app->infoOverlayOpen = false; app->planMenuOpen = false;
                            app->armedPlan = WB_PLAN_NONE;
                            app->actionMode = WB_ACTION_MOVE;
                            memset(app->worldEffects, 0, sizeof(app->worldEffects));
                            app->map.isWB2Level = (app->activeWorld >= 6);
                            initWorldObjects(app);
                            app->planSwoop.active = false;
                            app->startMenuOpen = false;
                            clampCamera(app);
                            playSfxClip(app, &app->sfxMove);
                            break;
                        case 2: /* End Mission */
                            freeGameplaySheets(app);
                            ensureMenuSheetsLoaded(app);
                            switchWorldSelectSheets(app, app->activeWorld);
                            app->screenMode = WB_SCREEN_WORLD_SELECT;
                            app->worldSelectWorld = app->activeWorld;
                            app->startMenuOpen = false;
                            initWorldMiniWalkers(app, app->worldSelectWorld);
                            startMenuMusic(app);
                            playSfxClip(app, &app->sfxMove);
                            break;
                        case 3: /* Music toggle */
                            app->muteMusic = !app->muteMusic;
                            if (app->muteMusic) {
                                ndspChnWaveBufClear(MUSIC_CHANNEL);
                            } else {
                                playCurrentMusicTrack(app);
                            }
                            playSfxClip(app, &app->sfxMove);
                            break;
                        case 4: /* Sound FX toggle */
                            app->muteSfx = !app->muteSfx;
                            playSfxClip(app, &app->sfxMove);
                            break;
                        default: break;
                    }
                }
                /* Keep music running and handle touch while start menu is open */
                if (kHeld & KEY_TOUCH) {
                    if (!app->touchActiveLast)
                        handleStartMenuTouch(app, &touch);
                    app->touchActiveLast = true;
                } else {
                    app->touchActiveLast = false;
                }
                updateAudioRuntime(app);
                goto doneGameInput;
            }
            if (kDown & KEY_SELECT) {
                app->planMenuOpen = !app->planMenuOpen;
                app->planMenuScrollOffset = 0;
                app->planMenuCursor = 0;
                app->infoOverlayOpen = false;
                app->startMenuOpen = false;
                playSfxClip(app, &app->sfxPlan);
            }
            if (kDown & KEY_R) {
                app->planMenuOpen = !app->planMenuOpen;
                app->planMenuScrollOffset = 0;
                app->planMenuCursor = 0;
                app->infoOverlayOpen = false;
                app->startMenuOpen = false;
                playSfxClip(app, &app->sfxPlan);
            }
            /* ── Plan-menu button navigation (takes priority over game controls) ── */
            if (app->planMenuOpen) {
                int pmCount = planMenuEntryCount(app);
                if (kDown & KEY_DOWN) {
                    if (pmCount > 0) {
                        app->planMenuCursor = (app->planMenuCursor + 1) % pmCount;
                        if (app->planMenuCursor >= app->planMenuScrollOffset + 3)
                            app->planMenuScrollOffset = app->planMenuCursor - 2;
                    }
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & KEY_UP) {
                    if (pmCount > 0) {
                        app->planMenuCursor = (app->planMenuCursor + pmCount - 1) % pmCount;
                        if (app->planMenuCursor < app->planMenuScrollOffset)
                            app->planMenuScrollOffset = app->planMenuCursor;
                    }
                    playSfxClip(app, &app->sfxMove);
                }
                if (kDown & KEY_A) {
                    WBPlanType pmPlan = planMenuEntryAt(app, app->planMenuCursor);
                    if (pmPlan != WB_PLAN_NONE && app->map.planInventory[pmPlan] > 0)
                        app->armedPlan = pmPlan;
                    app->planMenuOpen = false;
                    app->infoOverlayOpen = false;
                    playSfxClip(app, &app->sfxPlan);
                }
                if (kDown & KEY_B) {
                    app->planMenuOpen = false;
                    playSfxClip(app, &app->sfxMove);
                }
                if (kHeld & KEY_TOUCH) {
                    if (!app->touchActiveLast)
                        handlePlanMenuTouch(app, &touch);
                    app->touchActiveLast = true;
                } else {
                    app->touchActiveLast = false;
                }
                updateAudioRuntime(app);
                goto doneGameInput;
            }
            if ((kDown & KEY_A) != 0) {
                if (app->goalPopupVisible && app->goalPopupComplete) {
                    playSfxClip(app, &app->sfxMove);
                    app->goalPopupVisible = false;
                    app->goalPopupComplete = false;
                    app->goalPopupBonus = false;
                } else {
                    if (app->hasSelection) {
                        WBCell* sc = &app->map.cells[app->selectedY][app->selectedX];
                        if (sc->hasBuilding && sc->buildingType == WB_BUILDING_FACTORY) {
                            sc->factoryColor = (sc->factoryColor + 1) % 5;
                            playSfxClip(app, &app->sfxMove);
                        }
                    }
                    handleActionButton(app);
                }
            }
            if ((kDown & KEY_L) != 0) {
                if (app->goalPopupVisible && app->goalPopupComplete) {
                    playSfxClip(app, &app->sfxMove);
                    app->goalPopupVisible = false;
                    app->goalPopupComplete = false;
                    app->goalPopupBonus = false;
                } else {
                    if (app->hasSelection) {
                        WBCell* sc = &app->map.cells[app->selectedY][app->selectedX];
                        if (sc->hasBuilding && sc->buildingType == WB_BUILDING_FACTORY) {
                            sc->factoryColor = (sc->factoryColor + 1) % 5;
                            playSfxClip(app, &app->sfxMove);
                        }
                    }
                    handleActionButton(app);
                }
            }
            if ((kDown & KEY_B) != 0) {
                if (app->goalPopupVisible && app->goalPopupComplete) {
                    app->goalPopupVisible = false;
                    app->goalPopupComplete = false;
                    app->goalPopupBonus = false;
                    freeGameplaySheets(app);
                    ensureMenuSheetsLoaded(app);
                    switchWorldSelectSheets(app, app->activeWorld);
                    app->screenMode = WB_SCREEN_WORLD_SELECT;
                    app->worldSelectWorld = app->activeWorld;
                    app->hasSelection = false;
                    app->infoOverlayOpen = false;
                    app->armedPlan = WB_PLAN_NONE;
                    app->planMenuOpen = false;
                    app->startMenuOpen = false;
                    initWorldMiniWalkers(app, app->worldSelectWorld);
                    startMenuMusic(app);
                    playSfxClip(app, &app->sfxMove);
                    continue;
                }
                app->hasSelection = false;
                app->infoOverlayOpen = false;
                app->armedPlan = WB_PLAN_NONE;
                app->planMenuOpen = false;
                app->goalPopupVisible = false;
                app->pileInspectValid = false;
                app->actionMode = WB_ACTION_MOVE;
                app->startMenuOpen = false;
                playSfxClip(app, &app->sfxMove);
            }
            if ((kDown & KEY_X) != 0) {
                disassembleSelected(app);
            }
            if ((kDown & KEY_Y) != 0 && app->hasSelection) {
                app->infoOverlayOpen = !app->infoOverlayOpen;
                app->planMenuOpen = false;
                app->startMenuOpen = false;
                playSfxClip(app, &app->sfxMove);
            }

            {
                int scrollX = 0;
                int scrollY = 0;
                if (kHeld & KEY_LEFT) scrollX = -1;
                if (kHeld & KEY_RIGHT) scrollX = 1;
                if (kHeld & KEY_UP) scrollY = -1;
                if (kHeld & KEY_DOWN) scrollY = 1;
                if (circle.dx < -20) scrollX = -1;
                if (circle.dx > 20) scrollX = 1;
                if (circle.dy < -20) scrollY = 1;
                if (circle.dy > 20) scrollY = -1;
                if (scrollCameraStep(app, scrollX, scrollY)) {
                    app->centerGoalActive = false;     /* scrollmapManual(): pCenterGoal = VOID */
                }
            }

            if (kHeld & KEY_TOUCH) {
                int previewX;
                int previewY;
                if (app->armedPlan != WB_PLAN_NONE && screenToTile(app, (float) touch.px, (float) touch.py, &previewX, &previewY)) {
                    updateBuildPreview(app, previewX, previewY);
                }
                if (!app->touchActiveLast) {
                    if (app->planMenuOpen) {
                        handlePlanMenuTouch(app, &touch);
                    } else if (app->startMenuOpen) {
                        handleStartMenuTouch(app, &touch);
                    } else if (!app->startMenuOpen) {
                        handleWorldTouch(app, &touch);
                    }
                }
                app->touchActiveLast = true;
            } else {
                app->touchActiveLast = false;
                if (app->armedPlan == WB_PLAN_NONE) {
                    app->buildPreviewValid = false;
                }
            }

            runGameSimulation:
            runWorldFrames(app);
            tickCollectGoals(app);
            updateAudioRuntime(app);
            clampCamera(app);
            doneGameInput:;
        }

        C2D_TextBufClear(app->dynamicBuf);

        /* Update title screen cycle state */
        if (app->screenMode == WB_SCREEN_TITLE) {
            u64 titleNow = osGetTime();
            if (app->titleCycleStartMs == 0) {
                app->titleCycleStartMs = titleNow;
            }
            app->titleShowWB2 = (((titleNow - app->titleCycleStartMs) / 15000ULL) % 2) == 1;
        } else {
            app->titleCycleStartMs = 0;
            app->titleShowWB2 = false;
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(top, (app->screenMode == WB_SCREEN_TITLE && app->titleShowWB2)
            ? C2D_Color32(88, 123, 41, 0xFF)
            : C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
        C2D_SceneBegin(top);
        if (app->screenMode == WB_SCREEN_GAME) {
            drawTopScreen(app);
            tutorialDrawTop(app);
        } else if (app->screenMode == WB_SCREEN_WORLD_SELECT) {
            drawWorldSelectTop(app);
        }
        if (app->debugMenuOpen && app->screenMode == WB_SCREEN_TITLE) {
            drawDebugMenu(app);
        }

        C2D_TargetClear(bottom, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
        C2D_SceneBegin(bottom);
        if (app->screenMode == WB_SCREEN_TITLE) {
            drawTitleBottom(app);
        } else if (app->screenMode == WB_SCREEN_WORLD_SELECT) {
            drawWorldSelectBottom(app);
        } else {
            drawSkyBottom(app);
            drawTerrainBottom(app);
            drawObjectsBottom(app);
            drawPlanSwoop(app);
            drawBuildPreview(app);
            drawPileInspectPopup(app);
            drawBottomGoalPopup(app);
            if (app->startMenuOpen) {
                drawBottomStartMenu(app);
            } else if (app->planMenuOpen) {
                drawBottomPlanMenu(app);
            }
            tutorialDrawBottom(app);
        }

        C3D_FrameEnd(0);
    }

    sceneExit(app);
    if (app->audioReady) {
        ndspExit();
    }
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}
