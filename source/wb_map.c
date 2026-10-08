#include "wb_map.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const WBResourcePile g_buggyRecipe      = { 10, 0,  0,  0,  4, 1 };
static const WBResourcePile g_duckRecipe       = {  0, 0,  0,  5,  0, 1 };
static const WBResourcePile g_dirtbuggyRecipe  = {  0, 0,  0, 10,  4, 1 };
static const WBResourcePile g_steamshovelRecipe= {  0, 0,  0, 25,  4, 1 };
static const WBResourcePile g_dumptruckRecipe  = {  0, 0,  0, 20,  6, 1 };
static const WBResourcePile g_forkliftRecipe   = { 15, 0,  0,  0,  4, 1 };
static const WBResourcePile g_dozerRecipe      = {  0, 0,  0, 15,  6, 1 };
static const WBResourcePile g_speedboatRecipe  = { 10,10,  0, 40,  0, 1 };
static const WBResourcePile g_tugboatRecipe    = {  0,15,  0,  0,  0, 1 };
static const WBResourcePile g_freighterRecipe  = {  0,25,  0,  0,  0, 1 };
static const WBResourcePile g_frogRecipe       = {  0, 0,  5,  0,  0, 1 };
static const WBResourcePile g_fishRecipe       = {  0, 5,  0,  0,  0, 1 };
static const WBResourcePile g_snailRecipe      = {  5, 0,  0,  0,  0, 1 };
static const WBResourcePile g_treebotRecipe    = {  0,25, 20,  0,  0, 1 };
static const WBResourcePile g_repairbotRecipe  = {  0,20, 10,  5,  0, 1 };
static const WBResourcePile g_defenderRecipe   = {  0,40,  0, 25,  0, 1 };
static const WBResourcePile g_gasStationRecipe = { 40, 0,  5,  0,  0, 0 };
static const WBResourcePile g_marinaRecipe     = {  0,40,  5,  0,  0, 0 };
static const WBResourcePile g_robotLabRecipe   = {  0,25, 10, 25,  0, 0 };
static const WBResourcePile g_guardTowerRecipe = { 25, 0,  0, 25,  0, 1 };
static const WBResourcePile g_freezebotRecipe  = {  5, 5,  0, 25,  0, 1,  10 };
/* WB2 config: the production buildings each need a battery. */
static const WBResourcePile g_houseRecipe      = {  0,20, 20, 20,  0, 0,  25 };
static const WBResourcePile g_factoryRecipe    = { 15,10,  0,  0,  0, 1,  25 };
static const WBResourcePile g_windmillRecipe   = { 15, 0,  0,  0,  5, 1,  25 };
static const WBResourcePile g_garageRecipe     = { 15, 0,  0, 10,  0, 1,  25 };
static const WBResourcePile g_nurseryRecipe    = { 15, 0, 10,  0,  0, 1,  25 };

static void trimLine(char* line) {
    char* start = line;
    size_t len;

    while (*start && isspace((unsigned char) *start)) {
        ++start;
    }
    if (start != line) {
        memmove(line, start, strlen(start) + 1);
    }

    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || isspace((unsigned char) line[len - 1]))) {
        line[--len] = '\0';
    }
}

static char* trimToken(char* token) {
    char* end;
    if (!token) {
        return NULL;
    }
    while (*token && isspace((unsigned char) *token)) {
        ++token;
    }
    if (*token == '\0') {
        return token;
    }
    end = token + strlen(token) - 1;
    while (end > token && isspace((unsigned char) *end)) {
        *end-- = '\0';
    }
    return token;
}

static void initCell(WBCell* cell) {
    memset(cell, 0, sizeof(*cell));
    cell->terrain = WB_TERRAIN_NORMAL;
    cell->unitDirection = WB_DIR_RIGHT;
    cell->unitEnergy = 100;
    cell->unitEnergyDeci = 1000;
}

static void parsePileContents(WBResourcePile* pile, const char* desc) {
    char local[WB_MAX_ITEM_DESC];
    char* token;
    strncpy(local, desc, sizeof(local) - 1);
    local[sizeof(local) - 1] = '\0';
    token = strtok(local, ",");
    if (!token) {
        return;
    }
    while (1) {
        char* kind = trimToken(strtok(NULL, ","));
        char* value = trimToken(strtok(NULL, ","));
        int num;
        if (!kind || !value) {
            break;
        }
        num = atoi(value);
        if (strcmp(kind, "red") == 0) pile->red += num;
        else if (strcmp(kind, "blue") == 0) pile->blue += num;
        else if (strcmp(kind, "yellow") == 0) pile->yellow += num;
        else if (strcmp(kind, "green") == 0) pile->green += num;
        else if (strcmp(kind, "wheel") == 0) pile->wheel += num;
        else if (strcmp(kind, "energy") == 0) pile->energy += num;
        else if (strcmp(kind, "white") == 0) pile->white += num;
    }
}

bool wbTerrainIsBase(char c) {
    switch (c) {
        case '.':
        case 'T':
        case 'w':
        case 'M':
        case '_':
        case 'x':
        case 'r':
        case '@':
        case '#':
        case '^':
        case '~':
        case '`':
        case ',':
        case '(':
        case '[':
        case '{':
        case '\\':
        case '/':
        case ')':
        case ']':
        case '}':
        case '+':
        case '=':
        case '-':
        case '!':
        case '\'':
        case '?':
        case '%':
        case '&':
        case '$':
        case '*':
        case '>': /* roadblock */
        case ':': /* goal tile */
            return true;
        default:
            return false;
    }
}

WBTerrainType wbTerrainFromChar(char c) {
    switch (c) {
        case '.': return WB_TERRAIN_NORMAL;
        case 'T': return WB_TERRAIN_TREE;
        case 'w': return WB_TERRAIN_WATER;
        case 'M': return WB_TERRAIN_MOUNTAIN;
        case '_': return WB_TERRAIN_NORMAL_UNDIGGABLE;
        case 'x': return WB_TERRAIN_WATER_UNFILLABLE;
        case 'r': return WB_TERRAIN_WATER_REEFS;
        case '@': return WB_TERRAIN_HOLE;
        case '#': return WB_TERRAIN_SWAMP;
        case '^': return WB_TERRAIN_MOUNTAIN;
        case '~': return WB_TERRAIN_BILLBOARD;
        case '`': return WB_TERRAIN_CEMENT;
        case '(':  case '[':  case '{':  case '\\': case '/':
        case ')':  case ']':  case '}':  case '+':  case '=':
            return WB_TERRAIN_STREET;
        case '-': return WB_TERRAIN_STREET_UNDIGGABLE;
        case '!': return WB_TERRAIN_TREE2;
        case '\'': return WB_TERRAIN_TREE3;
        case '?': return WB_TERRAIN_TREE4;
        case '%':  case '&':  case '$':  case ',':
        case '*': return WB_TERRAIN_JUNGLE;
        case '>': return WB_TERRAIN_ROADBLOCK;
        case ':': return WB_TERRAIN_NORMAL;
        default: return WB_TERRAIN_NORMAL;
    }
}

/* Returns the 0-based variant index for STREET (0-9) and JUNGLE (0-3) tiles.
   All other terrain types return 0. */
static uint8_t wbTerrainVariantFromChar(char c) {
    switch (c) {
        /* Street variants 0-9 */
        case '(': return 0;
        case '[': return 1;
        case '{': return 2;
        case '\\': return 3;
        case '/': return 4;
        case ')': return 5;
        case ']': return 6;
        case '}': return 7;
        case '+': return 8;
        case '=': return 9;
        /* Jungle variants 0-3 */
        case '%': return 0;
        case '&': return 1;
        case '$': return 2;
        case ',': return 3;
        case '*': return 0; /* maps to jungle1 (same as %) */
        default:  return 0;
    }
}

const char* wbTerrainName(WBTerrainType terrain) {
    switch (terrain) {
        case WB_TERRAIN_NORMAL: return "normal";
        case WB_TERRAIN_TREE: return "tree";
        case WB_TERRAIN_WATER: return "water";
        case WB_TERRAIN_MOUNTAIN: return "mountain";
        case WB_TERRAIN_NORMAL_UNDIGGABLE: return "rocky";
        case WB_TERRAIN_WATER_UNFILLABLE: return "deep water";
        case WB_TERRAIN_WATER_REEFS: return "reefs";
        case WB_TERRAIN_HOLE: return "hole";
        case WB_TERRAIN_SWAMP: return "swamp";
        case WB_TERRAIN_WATER_WHIRLPOOL: return "whirlpool";
        case WB_TERRAIN_BILLBOARD: return "billboard";
        case WB_TERRAIN_CEMENT: return "cement";
        case WB_TERRAIN_JUNGLE: return "jungle";
        case WB_TERRAIN_ROADBLOCK: return "roadblock";
        case WB_TERRAIN_STREET: return "street";
        case WB_TERRAIN_STREET_UNDIGGABLE: return "street (solid)";
        case WB_TERRAIN_TREE2: return "tree2";
        case WB_TERRAIN_TREE3: return "tree3";
        case WB_TERRAIN_TREE4: return "tree4";
        default: return "unknown";
    }
}

WBMonsterType wbMonsterTypeFromName(const char* name) {
    if (!name) return WB_MONSTER_NONE;
    if (strcmp(name, "crab")       == 0) return WB_MONSTER_CRAB;
    if (strcmp(name, "red_crab")   == 0) return WB_MONSTER_CRAB;
    if (strcmp(name, "water_crab") == 0) return WB_MONSTER_WATER_CRAB;
    if (strcmp(name, "watercrab")  == 0) return WB_MONSTER_WATER_CRAB;
    if (strcmp(name, "water-crab") == 0) return WB_MONSTER_WATER_CRAB;
    if (strcmp(name, "gator")      == 0) return WB_MONSTER_GATOR;
    if (strcmp(name, "alligator")  == 0) return WB_MONSTER_GATOR;
    if (strcmp(name, "scorpion")   == 0) return WB_MONSTER_SCORPION;
    if (strcmp(name, "shark")      == 0) return WB_MONSTER_SHARK;
    if (strcmp(name, "boulder")    == 0) return WB_MONSTER_BOULDER;
    if (strcmp(name, "trex")       == 0) return WB_MONSTER_TREX;
    if (strcmp(name, "t_rex")      == 0) return WB_MONSTER_TREX;
    if (strcmp(name, "t-rex")      == 0) return WB_MONSTER_TREX;
    if (strcmp(name, "lion")       == 0) return WB_MONSTER_LION;
    return WB_MONSTER_NONE;
}

WBBuildingType wbBuildingTypeFromName(const char* name) {
    if (!name) return WB_BUILDING_NONE;
    if (strcmp(name, "gas_station") == 0) return WB_BUILDING_GAS_STATION;
    if (strcmp(name, "marina")      == 0) return WB_BUILDING_MARINA;
    if (strcmp(name, "robot_lab")   == 0) return WB_BUILDING_ROBOT_LAB;
    if (strcmp(name, "guard_tower") == 0) return WB_BUILDING_GUARD_TOWER;
    if (strcmp(name, "airport")     == 0) return WB_BUILDING_AIRPORT;
    if (strcmp(name, "house")       == 0) return WB_BUILDING_HOUSE;
    if (strcmp(name, "factory")     == 0) return WB_BUILDING_FACTORY;
    if (strcmp(name, "windmill")    == 0) return WB_BUILDING_WINDMILL;
    if (strcmp(name, "garage")      == 0) return WB_BUILDING_GARAGE;
    if (strcmp(name, "nursery")     == 0) return WB_BUILDING_NURSERY;
    return WB_BUILDING_NONE;
}

const char* wbMonsterName(WBMonsterType m) {
    switch (m) {
        case WB_MONSTER_CRAB:       return "Crab";
        case WB_MONSTER_WATER_CRAB: return "Water Crab";
        case WB_MONSTER_GATOR:      return "Gator";
        case WB_MONSTER_SCORPION:   return "Scorpion";
        case WB_MONSTER_SHARK:      return "Shark";
        case WB_MONSTER_BOULDER:    return "Boulder";
        case WB_MONSTER_TREX:       return "T-Rex";
        case WB_MONSTER_LION:       return "Lion";
        default: return "Unknown";
    }
}

const char* wbBuildingName(WBBuildingType b) {
    switch (b) {
        case WB_BUILDING_GAS_STATION: return "Gas Station";
        case WB_BUILDING_MARINA:      return "Marina";
        case WB_BUILDING_ROBOT_LAB:   return "Robot Lab";
        case WB_BUILDING_GUARD_TOWER: return "Guard Tower";
        case WB_BUILDING_AIRPORT:     return "Airport";
        case WB_BUILDING_HOUSE:       return "House";
        case WB_BUILDING_FACTORY:     return "Factory";
        case WB_BUILDING_WINDMILL:    return "Windmill";
        case WB_BUILDING_GARAGE:      return "Garage";
        case WB_BUILDING_NURSERY:     return "Nursery";
        default: return "Unknown";
    }
}

/* Each monster's #terrain list (WB1 config, plus WB2's #street, which covers every
   street variant and cement). Swamp is in the lists but monster.generic only lets a
   monster use it while chasing or when stuck on swamp (main.c). No monster lists
   #water_whirlpool, #water_reefs or #street_undiggable. */
bool wbMonsterCanTraverse(WBMonsterType m, WBTerrainType terrain) {
    bool street = terrain == WB_TERRAIN_STREET || terrain == WB_TERRAIN_CEMENT;
    switch (m) {
        case WB_MONSTER_CRAB:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_SWAMP;
        case WB_MONSTER_WATER_CRAB:
            return terrain == WB_TERRAIN_WATER || terrain == WB_TERRAIN_WATER_UNFILLABLE ||
                   terrain == WB_TERRAIN_SWAMP;
        case WB_MONSTER_GATOR:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_WATER ||
                   terrain == WB_TERRAIN_WATER_UNFILLABLE || terrain == WB_TERRAIN_SWAMP;
        case WB_MONSTER_SCORPION:
        case WB_MONSTER_TREX:
        case WB_MONSTER_LION:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_NORMAL_UNDIGGABLE ||
                   terrain == WB_TERRAIN_SWAMP;
        case WB_MONSTER_SHARK:
            return terrain == WB_TERRAIN_WATER || terrain == WB_TERRAIN_WATER_UNFILLABLE;
        case WB_MONSTER_BOULDER:
            return false; /* never moves itself; pushing has its own rule */
        default:
            return false;
    }
}

float wbMonsterSpeed(WBMonsterType m) {
    switch (m) {
        case WB_MONSTER_CRAB:       return 1.0f;
        case WB_MONSTER_WATER_CRAB: return 0.5f;
        case WB_MONSTER_GATOR:      return 0.5f;
        case WB_MONSTER_SCORPION:   return 1.0f;
        case WB_MONSTER_SHARK:      return 2.0f;
        case WB_MONSTER_BOULDER:    return 0.0f;
        case WB_MONSTER_TREX:       return 0.5f;
        case WB_MONSTER_LION:       return 4.0f;
        default: return 0.0f;
    }
}

int wbMonsterWalkFrames(WBMonsterType m) {
    switch (m) {
        case WB_MONSTER_CRAB:       return 4;
        case WB_MONSTER_WATER_CRAB: return 4;
        case WB_MONSTER_GATOR:      return 2;
        case WB_MONSTER_SCORPION:   return 3;
        case WB_MONSTER_SHARK:      return 3;
        case WB_MONSTER_BOULDER:    return 0;
        case WB_MONSTER_TREX:       return 10;
        case WB_MONSTER_LION:       return 2;
        default: return 0;
    }
}

bool wbMonsterIsAmphibious(WBMonsterType m) {
    return m == WB_MONSTER_GATOR;
}

/* How many moves before a monster might rest (original Lingo rest_every config). */
int wbMonsterRestEvery(WBMonsterType m) {
    switch (m) {
        case WB_MONSTER_CRAB:       return 6;
        case WB_MONSTER_WATER_CRAB: return 3;
        case WB_MONSTER_GATOR:      return 3;
        case WB_MONSTER_SCORPION:   return 6;
        case WB_MONSTER_SHARK:      return 4;
        case WB_MONSTER_TREX:       return 12;
        case WB_MONSTER_LION:       return 4;
        default: return 6;
    }
}

/* Rest duration in milliseconds (original Lingo rest_for config * random 750-1250ms). */
int wbMonsterRestForMs(WBMonsterType m) {
    switch (m) {
        case WB_MONSTER_CRAB:       return 2000;
        case WB_MONSTER_WATER_CRAB: return 1000;
        case WB_MONSTER_GATOR:      return 1000;
        case WB_MONSTER_SCORPION:   return 2000;
        case WB_MONSTER_SHARK:      return 1000;
        case WB_MONSTER_TREX:       return 2000;
        case WB_MONSTER_LION:       return 1000;
        default: return 1000;
    }
}

const char* wbPlanName(WBPlanType planType) {
    switch (planType) {
        case WB_PLAN_BUGGY:       return "Buggy";
        case WB_PLAN_DUCK:        return "Duck";
        case WB_PLAN_DIRTBUGGY:   return "Dirtbuggy";
        case WB_PLAN_STEAMSHOVEL: return "Steamshovel";
        case WB_PLAN_DUMPTRUCK:   return "Dumptruck";
        case WB_PLAN_FORKLIFT:    return "Forklift";
        case WB_PLAN_DOZER:       return "Bulldozer";
        case WB_PLAN_SPEEDBOAT:   return "Speedboat";
        case WB_PLAN_TUGBOAT:     return "Tugboat";
        case WB_PLAN_FREIGHTER:   return "Freighter";
        case WB_PLAN_FROG:        return "Frog";
        case WB_PLAN_FISH:        return "Fish";
        case WB_PLAN_SNAIL:       return "Snail";
        case WB_PLAN_TREEBOT:     return "Treebot";
        case WB_PLAN_REPAIRBOT:   return "Repairbot";
        case WB_PLAN_DEFENDER:    return "Defender";
        case WB_PLAN_GAS_STATION: return "Gas Station";
        case WB_PLAN_MARINA:      return "Marina";
        case WB_PLAN_ROBOT_LAB:   return "Robot Lab";
        case WB_PLAN_GUARD_TOWER: return "Guard Tower";
        case WB_PLAN_AIRPORT:     return "Airport";
        case WB_PLAN_FREEZEBOT:   return "Freezebot";
        case WB_PLAN_HOUSE:       return "House";
        case WB_PLAN_FACTORY:     return "Factory";
        case WB_PLAN_WINDMILL:    return "Windmill";
        case WB_PLAN_GARAGE:      return "Garage";
        case WB_PLAN_NURSERY:     return "Nursery";
        default: return "Unknown";
    }
}

const char* wbUnitName(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:       return "Buggy";
        case WB_UNIT_DUCK:        return "Duck";
        case WB_UNIT_DIRTBUGGY:   return "Dirtbuggy";
        case WB_UNIT_STEAMSHOVEL: return "Steamshovel";
        case WB_UNIT_DUMPTRUCK:   return "Dumptruck";
        case WB_UNIT_FORKLIFT:    return "Forklift";
        case WB_UNIT_DOZER:       return "Bulldozer";
        case WB_UNIT_SPEEDBOAT:   return "Speedboat";
        case WB_UNIT_TUGBOAT:     return "Tugboat";
        case WB_UNIT_FREIGHTER:   return "Freighter";
        case WB_UNIT_FROG:        return "Frog";
        case WB_UNIT_FISH:        return "Fish";
        case WB_UNIT_SNAIL:       return "Snail";
        case WB_UNIT_TREEBOT:     return "Treebot";
        case WB_UNIT_REPAIRBOT:   return "Repairbot";
        case WB_UNIT_DEFENDER:    return "Defender";
        case WB_UNIT_DEFENDER2:   return "Defender Mk.2";
        case WB_UNIT_FREEZEBOT:   return "Freezebot";
        default: return "Unknown";
    }
}

WBPlanType wbPlanTypeFromName(const char* name) {
    if (!name) return WB_PLAN_NONE;
    if (strcmp(name, "buggy")       == 0) return WB_PLAN_BUGGY;
    if (strcmp(name, "duck")        == 0) return WB_PLAN_DUCK;
    if (strcmp(name, "dirtbuggy")   == 0) return WB_PLAN_DIRTBUGGY;
    if (strcmp(name, "steamshovel") == 0) return WB_PLAN_STEAMSHOVEL;
    if (strcmp(name, "dumptruck")   == 0) return WB_PLAN_DUMPTRUCK;
    if (strcmp(name, "forklift")    == 0) return WB_PLAN_FORKLIFT;
    if (strcmp(name, "dozer")       == 0) return WB_PLAN_DOZER;
    if (strcmp(name, "speedboat")   == 0) return WB_PLAN_SPEEDBOAT;
    if (strcmp(name, "tugboat")     == 0) return WB_PLAN_TUGBOAT;
    if (strcmp(name, "freighter")   == 0) return WB_PLAN_FREIGHTER;
    if (strcmp(name, "frog")        == 0) return WB_PLAN_FROG;
    if (strcmp(name, "fish")        == 0) return WB_PLAN_FISH;
    if (strcmp(name, "snail")       == 0) return WB_PLAN_SNAIL;
    if (strcmp(name, "treebot")     == 0) return WB_PLAN_TREEBOT;
    if (strcmp(name, "repairbot")   == 0) return WB_PLAN_REPAIRBOT;
    if (strcmp(name, "defender")    == 0) return WB_PLAN_DEFENDER;
    if (strcmp(name, "gas_station") == 0) return WB_PLAN_GAS_STATION;
    if (strcmp(name, "marina")      == 0) return WB_PLAN_MARINA;
    if (strcmp(name, "robot_lab")   == 0) return WB_PLAN_ROBOT_LAB;
    if (strcmp(name, "guard_tower") == 0) return WB_PLAN_GUARD_TOWER;
    if (strcmp(name, "airport")     == 0) return WB_PLAN_AIRPORT;
    if (strcmp(name, "freezebot")   == 0) return WB_PLAN_FREEZEBOT;
    if (strcmp(name, "house")       == 0) return WB_PLAN_HOUSE;
    if (strcmp(name, "factory")     == 0) return WB_PLAN_FACTORY;
    if (strcmp(name, "windmill")    == 0) return WB_PLAN_WINDMILL;
    if (strcmp(name, "garage")      == 0) return WB_PLAN_GARAGE;
    if (strcmp(name, "nursery")     == 0) return WB_PLAN_NURSERY;
    return WB_PLAN_NONE;
}

WBUnitType wbUnitTypeFromName(const char* name) {
    if (!name) return WB_UNIT_NONE;
    if (strcmp(name, "buggy")       == 0) return WB_UNIT_BUGGY;
    if (strcmp(name, "duck")        == 0) return WB_UNIT_DUCK;
    if (strcmp(name, "dirtbuggy")   == 0) return WB_UNIT_DIRTBUGGY;
    if (strcmp(name, "steamshovel") == 0) return WB_UNIT_STEAMSHOVEL;
    if (strcmp(name, "dumptruck")   == 0) return WB_UNIT_DUMPTRUCK;
    if (strcmp(name, "forklift")    == 0) return WB_UNIT_FORKLIFT;
    if (strcmp(name, "dozer")       == 0) return WB_UNIT_DOZER;
    if (strcmp(name, "speedboat")   == 0) return WB_UNIT_SPEEDBOAT;
    if (strcmp(name, "tugboat")     == 0) return WB_UNIT_TUGBOAT;
    if (strcmp(name, "freighter")   == 0) return WB_UNIT_FREIGHTER;
    if (strcmp(name, "frog")        == 0) return WB_UNIT_FROG;
    if (strcmp(name, "fish")        == 0) return WB_UNIT_FISH;
    if (strcmp(name, "snail")       == 0) return WB_UNIT_SNAIL;
    if (strcmp(name, "treebot")     == 0) return WB_UNIT_TREEBOT;
    if (strcmp(name, "repairbot")   == 0) return WB_UNIT_REPAIRBOT;
    if (strcmp(name, "defender")    == 0) return WB_UNIT_DEFENDER;
    if (strcmp(name, "defender2")   == 0) return WB_UNIT_DEFENDER2;
    if (strcmp(name, "freezebot")   == 0) return WB_UNIT_FREEZEBOT;
    return WB_UNIT_NONE;
}

WBGoalType wbGoalTypeFromName(const char* name) {
    if (!name) return WB_GOAL_NONE;
    if (strcmp(name, "anything")   == 0) return WB_GOAL_ANYTHING;
    if (strcmp(name, "buggy")      == 0) return WB_GOAL_BUGGY;
    if (strcmp(name, "duck")       == 0) return WB_GOAL_DUCK;
    if (strcmp(name, "dirtbuggy")  == 0) return WB_GOAL_DIRTBUGGY;
    if (strcmp(name, "steamshovel")== 0) return WB_GOAL_STEAMSHOVEL;
    if (strcmp(name, "dumptruck")  == 0) return WB_GOAL_DUMPTRUCK;
    if (strcmp(name, "forklift")   == 0) return WB_GOAL_FORKLIFT;
    if (strcmp(name, "dozer")      == 0) return WB_GOAL_DOZER;
    if (strcmp(name, "speedboat")  == 0) return WB_GOAL_SPEEDBOAT;
    if (strcmp(name, "tugboat")    == 0) return WB_GOAL_TUGBOAT;
    if (strcmp(name, "freighter")  == 0) return WB_GOAL_FREIGHTER;
    if (strcmp(name, "frog")       == 0) return WB_GOAL_FROG;
    if (strcmp(name, "fish")       == 0) return WB_GOAL_FISH;
    if (strcmp(name, "snail")      == 0) return WB_GOAL_SNAIL;
    if (strcmp(name, "treebot")    == 0) return WB_GOAL_TREEBOT;
    if (strcmp(name, "repairbot")  == 0) return WB_GOAL_REPAIRBOT;
    if (strcmp(name, "defender")   == 0) return WB_GOAL_DEFENDER;
    if (strcmp(name, "gas_station") == 0) return WB_GOAL_GAS_STATION;
    if (strcmp(name, "marina")      == 0) return WB_GOAL_MARINA;
    if (strcmp(name, "robot_lab")   == 0) return WB_GOAL_ROBOT_LAB;
    if (strcmp(name, "guard_tower") == 0) return WB_GOAL_GUARD_TOWER;
    if (strcmp(name, "airport")     == 0) return WB_GOAL_AIRPORT;
    if (strcmp(name, "house")       == 0) return WB_GOAL_HOUSE;
    if (strcmp(name, "garage")      == 0) return WB_GOAL_GARAGE;
    if (strcmp(name, "freezebot")   == 0) return WB_GOAL_FREEZEBOT;
    if (strcmp(name, "lion")         == 0) return WB_GOAL_LION;
    if (strcmp(name, "crab")         == 0) return WB_GOAL_CRAB;
    if (strcmp(name, "factory")      == 0) return WB_GOAL_FACTORY;
    if (strcmp(name, "windmill")     == 0) return WB_GOAL_WINDMILL;
    if (strcmp(name, "nursery")      == 0) return WB_GOAL_NURSERY;
    if (strcmp(name, "gator")        == 0) return WB_GOAL_GATOR;
    return WB_GOAL_NONE;
}

const WBResourcePile* wbUnitRecipe(WBUnitType unitType) {
    switch (unitType) {
        case WB_UNIT_BUGGY:       return &g_buggyRecipe;
        case WB_UNIT_DUCK:        return &g_duckRecipe;
        case WB_UNIT_DIRTBUGGY:   return &g_dirtbuggyRecipe;
        case WB_UNIT_STEAMSHOVEL: return &g_steamshovelRecipe;
        case WB_UNIT_DUMPTRUCK:   return &g_dumptruckRecipe;
        case WB_UNIT_FORKLIFT:    return &g_forkliftRecipe;
        case WB_UNIT_DOZER:       return &g_dozerRecipe;
        case WB_UNIT_SPEEDBOAT:   return &g_speedboatRecipe;
        case WB_UNIT_TUGBOAT:     return &g_tugboatRecipe;
        case WB_UNIT_FREIGHTER:   return &g_freighterRecipe;
        case WB_UNIT_FROG:        return &g_frogRecipe;
        case WB_UNIT_FISH:        return &g_fishRecipe;
        case WB_UNIT_SNAIL:       return &g_snailRecipe;
        case WB_UNIT_TREEBOT:     return &g_treebotRecipe;
        case WB_UNIT_REPAIRBOT:   return &g_repairbotRecipe;
        case WB_UNIT_DEFENDER:    return &g_defenderRecipe;
        case WB_UNIT_DEFENDER2:   return &g_defenderRecipe;
        case WB_UNIT_FREEZEBOT:   return &g_freezebotRecipe;
        default: return NULL;
    }
}

const WBResourcePile* wbBuildingRecipe(WBBuildingType buildingType) {
    switch (buildingType) {
        case WB_BUILDING_GAS_STATION: return &g_gasStationRecipe;
        case WB_BUILDING_MARINA:      return &g_marinaRecipe;
        case WB_BUILDING_ROBOT_LAB:   return &g_robotLabRecipe;
        case WB_BUILDING_GUARD_TOWER: return &g_guardTowerRecipe;
        case WB_BUILDING_HOUSE:       return &g_houseRecipe;
        case WB_BUILDING_FACTORY:     return &g_factoryRecipe;
        case WB_BUILDING_WINDMILL:    return &g_windmillRecipe;
        case WB_BUILDING_GARAGE:      return &g_garageRecipe;
        case WB_BUILDING_NURSERY:     return &g_nurseryRecipe;
        default: return NULL;
    }
}

/* Building #terrain lists: marina [#water]; the rest [#normal] (+#street in WB2). */
bool wbBuildingCanBePlacedOn(WBBuildingType buildingType, WBTerrainType terrain) {
    switch (buildingType) {
        case WB_BUILDING_MARINA:
            return terrain == WB_TERRAIN_WATER;
        case WB_BUILDING_GAS_STATION:
        case WB_BUILDING_ROBOT_LAB:
        case WB_BUILDING_GUARD_TOWER:
        case WB_BUILDING_HOUSE:
        case WB_BUILDING_FACTORY:
        case WB_BUILDING_WINDMILL:
        case WB_BUILDING_GARAGE:
        case WB_BUILDING_NURSERY:
            return terrain == WB_TERRAIN_NORMAL || terrain == WB_TERRAIN_CEMENT || terrain == WB_TERRAIN_STREET;
        default:
            return false;
    }
}

/* Each unit's #terrain list (WB1 config, plus WB2's #street). */
bool wbUnitCanTraverse(WBUnitType unitType, WBTerrainType terrain) {
    bool street = terrain == WB_TERRAIN_STREET || terrain == WB_TERRAIN_CEMENT;
    switch (unitType) {
        case WB_UNIT_BUGGY:
        case WB_UNIT_STEAMSHOVEL:
        case WB_UNIT_DOZER:
        case WB_UNIT_FORKLIFT:
        case WB_UNIT_TREEBOT:
        case WB_UNIT_DEFENDER:
        case WB_UNIT_FREEZEBOT:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_SWAMP;
        case WB_UNIT_DIRTBUGGY:
        case WB_UNIT_DUMPTRUCK:
        case WB_UNIT_REPAIRBOT:
        case WB_UNIT_DEFENDER2:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_NORMAL_UNDIGGABLE ||
                   terrain == WB_TERRAIN_SWAMP;
        case WB_UNIT_DUCK:
        case WB_UNIT_FROG:
            return terrain == WB_TERRAIN_NORMAL || street || terrain == WB_TERRAIN_WATER ||
                   terrain == WB_TERRAIN_SWAMP || terrain == WB_TERRAIN_WATER_WHIRLPOOL;
        case WB_UNIT_SPEEDBOAT:
        case WB_UNIT_TUGBOAT:
        case WB_UNIT_FREIGHTER:
        case WB_UNIT_FISH:
            return terrain == WB_TERRAIN_WATER || terrain == WB_TERRAIN_WATER_UNFILLABLE ||
                   terrain == WB_TERRAIN_WATER_WHIRLPOOL;
        case WB_UNIT_SNAIL:
            return terrain == WB_TERRAIN_NORMAL || street;
        default:
            return false;
    }
}

const WBItemDef* wbMapFindItem(const WBMap* map, char symbol) {
    int i;
    for (i = 0; i < map->itemDefCount; ++i) {
        if (map->itemDefs[i].symbol == symbol) {
            return &map->itemDefs[i];
        }
    }
    return NULL;
}

const char* wbMapDescribeSymbol(const WBMap* map, char symbol) {
    const WBItemDef* def = wbMapFindItem(map, symbol);
    return def ? def->desc : NULL;
}

static void applyItemDef(WBMap* map, int row, int col, char symbol) {
    const WBItemDef* def = wbMapFindItem(map, symbol);
    WBCell* cell = &map->cells[row][col];
    char scratch[WB_MAX_ITEM_DESC];
    char* token;

    cell->rawSymbol = symbol;
    if (!def) {
        return;
    }

    strncpy(scratch, def->desc, sizeof(scratch) - 1);
    scratch[sizeof(scratch) - 1] = '\0';
    token = strtok(scratch, ",");
    if (!token) {
        return;
    }

    if (strcmp(token, "pile") == 0 || strcmp(token, "waterpile") == 0) {
        cell->hasResource = true;
        if (strcmp(token, "waterpile") == 0) {
            cell->terrain = WB_TERRAIN_WATER;
        }
        parsePileContents(&cell->pile, def->desc);
        return;
    }

    if (strcmp(token, "plan") == 0 || strcmp(token, "waterplan") == 0) {
        char* name = trimToken(strtok(NULL, ","));
        char* count = trimToken(strtok(NULL, ","));
        cell->hasResource = true;
        cell->resourceIsPlan = true;
        cell->resourcePlanType = wbPlanTypeFromName(name);
        cell->resourcePlanCount = count ? atoi(count) : 1;
        if (strcmp(token, "waterplan") == 0) {
            cell->terrain = WB_TERRAIN_WATER;
        }
        if (cell->resourcePlanType > WB_PLAN_NONE && cell->resourcePlanType < WB_PLAN_COUNT) {
            map->possiblePlans[cell->resourcePlanType] = true;
        }
        return;
    }

    if (strcmp(token, "unit") == 0 || strcmp(token, "waterunit") == 0) {
        char* className = trimToken(strtok(NULL, ","));
        char* unitName = trimToken(strtok(NULL, ","));
        bool isWater = strcmp(token, "waterunit") == 0;
        if (isWater) {
            cell->terrain = WB_TERRAIN_WATER;
        }
        if (className && strcmp(className, "monster") == 0) {
            WBMonsterType monsterType = wbMonsterTypeFromName(unitName);
            if (monsterType != WB_MONSTER_NONE) {
                cell->hasMonster = true;
                cell->monsterType = monsterType;
                cell->monsterDirection = WB_DIR_RIGHT;
            }
        } else if (className && strcmp(className, "building") == 0) {
            cell->hasBuilding = true;
            cell->buildingType = wbBuildingTypeFromName(unitName);
            cell->buildingDirection = WB_DIR_RIGHT;
            cell->buildingHp = 1000;
        } else {
            cell->hasUnit = true;
            cell->unitType = wbUnitTypeFromName(unitName);
            cell->unitDirection = WB_DIR_RIGHT;
            cell->unitEnergy = 100;
            cell->unitEnergyDeci = 1000;
        }
        return;
    }

    if (strcmp(token, "whirlpool") == 0) {
        char* idStr = strtok(NULL, ",");
        cell->isWhirlpool = true;
        cell->whirlpoolId = idStr ? atoi(idStr) : 0;
        cell->terrain = WB_TERRAIN_WATER_WHIRLPOOL;
        return;
    }

    if (strcmp(token, "goal") == 0 || strcmp(token, "bonusgoal") == 0) {
        char* goalName = strtok(NULL, ",");
        char* terrainName = strtok(NULL, ",");
        cell->hasGoal = true;
        cell->bonusGoal = strcmp(token, "bonusgoal") == 0;
        if (goalName) {
            while (*goalName == ' ') goalName++;
            if (strncmp(goalName, "collect ", 8) == 0) {
                char* rest = goalName + 8;
                char* sp;
                while (*rest == ' ') rest++;
                cell->goalIsCollect = true;
                cell->goalCollectCount = atoi(rest);
                if (cell->goalCollectCount < 1) cell->goalCollectCount = 1;
                sp = strchr(rest, ' ');
                if (sp) {
                    char* typeName = sp + 1;
                    while (*typeName == ' ') typeName++;
                    cell->goalCollectType = wbMonsterTypeFromName(typeName);
                }
            } else {
                cell->goalType = wbGoalTypeFromName(goalName);
            }
        }
        if (terrainName) {
            while (*terrainName == ' ') terrainName++;
            if (strcmp(terrainName, "water") == 0) cell->terrain = WB_TERRAIN_WATER;
            else if (strcmp(terrainName, "swamp") == 0) cell->terrain = WB_TERRAIN_SWAMP;
            else if (strcmp(terrainName, "normal_undiggable") == 0) cell->terrain = WB_TERRAIN_NORMAL_UNDIGGABLE;
        }
        return;
    }
}

bool wbMapLoad(const char* path, WBMap* outMap) {
    FILE* fp;
    char line[256];
    enum { SEC_NONE, SEC_MAP, SEC_MAPITEMS, SEC_INVENTORY } section = SEC_NONE;

    memset(outMap, 0, sizeof(*outMap));
    fp = fopen(path, "r");
    if (!fp) {
        return false;
    }

    /* Skip UTF-8 BOM (EF BB BF) if present */
    {
        int b0 = fgetc(fp);
        int b1 = fgetc(fp);
        int b2 = fgetc(fp);
        if (b0 != 0xEF || b1 != 0xBB || b2 != 0xBF) {
            rewind(fp);
        }
    }

    while (fgets(line, sizeof(line), fp)) {
        char* eq;
        trimLine(line);

        if (line[0] == '\0' || (line[0] == '-' && line[1] == '-')) {
            continue;
        }

        if (strcmp(line, "[map]") == 0) {
            section = SEC_MAP;
            continue;
        }
        if (strcmp(line, "[mapitems]") == 0) {
            section = SEC_MAPITEMS;
            continue;
        }
        if (strcmp(line, "[inventory]") == 0) {
            section = SEC_INVENTORY;
            continue;
        }

        eq = strchr(line, '=');
        if (!eq) {
            continue;
        }

        *eq = '\0';
        ++eq;

        if (section == SEC_MAP) {
            if (strcmp(line, "name") == 0) {
                strncpy(outMap->name, eq, sizeof(outMap->name) - 1);
                continue;
            }

            if (strcmp(line, " center") == 0 || strcmp(line, "center") == 0) {
                int cx;
                int cy;
                if (sscanf(eq, "%d,%d", &cx, &cy) == 2) {
                    outMap->hasCenter = true;
                    outMap->centerX = cx;
                    outMap->centerY = cy;
                }
                continue;
            }

            if (strcmp(line, " map") == 0 || strcmp(line, "map") == 0) {
                int row;
                int col;
                int width = (int) strlen(eq);
                if (outMap->height >= WB_MAX_MAP_H) {
                    continue;
                }
                row = outMap->height;
                if (width > WB_MAX_MAP_W) {
                    width = WB_MAX_MAP_W;
                }
                for (col = 0; col < width; ++col) {
                    WBCell* cell = &outMap->cells[row][col];
                    char tile = eq[col];
                    initCell(cell);
                    cell->rawSymbol = tile;
                    if (wbTerrainIsBase(tile)) {
                        cell->terrain = wbTerrainFromChar(tile);
                        cell->terrainVariant = wbTerrainVariantFromChar(tile);
                    if (tile == ':') { cell->hasGoalTerrain = true; }
                    }
                }
                if (width > outMap->width) {
                    outMap->width = width;
                }
                outMap->height++;
            }
        } else if (section == SEC_MAPITEMS) {
            if (outMap->itemDefCount < WB_MAX_ITEM_DEFS && strlen(line) == 1) {
                WBItemDef* def = &outMap->itemDefs[outMap->itemDefCount++];
                def->symbol = line[0];
                strncpy(def->desc, eq, sizeof(def->desc) - 1);
                def->desc[sizeof(def->desc) - 1] = '\0';
            }
        } else if (section == SEC_INVENTORY) {
            WBPlanType planType = wbPlanTypeFromName(line);
            if (planType > WB_PLAN_NONE && planType < WB_PLAN_COUNT) {
                outMap->planInventory[planType] = atoi(eq);
            }
        }
    }

    fclose(fp);

    if (outMap->width <= 0 || outMap->height <= 0) {
        return false;
    }

    {
        int row;
        int col;
        for (row = 0; row < outMap->height; ++row) {
            for (col = 0; col < outMap->width; ++col) {
                char symbol = outMap->cells[row][col].rawSymbol;
                if (!wbTerrainIsBase(symbol)) {
                    applyItemDef(outMap, row, col, symbol);
                }
            }
        }
    }

    return true;
}
