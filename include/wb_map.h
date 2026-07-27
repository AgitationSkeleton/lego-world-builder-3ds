#ifndef WB_MAP_H
#define WB_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define WB_MAX_MAP_W 64
#define WB_MAX_MAP_H 64
#define WB_MAX_ITEM_DEFS 128
#define WB_MAX_ITEM_DESC 96
#define WB_MAX_NAME 64

typedef enum WBTerrainType {
    WB_TERRAIN_NORMAL = 0,
    WB_TERRAIN_TREE,
    WB_TERRAIN_WATER,
    WB_TERRAIN_MOUNTAIN,
    WB_TERRAIN_NORMAL_UNDIGGABLE,
    WB_TERRAIN_WATER_UNFILLABLE,
    WB_TERRAIN_WATER_REEFS,
    WB_TERRAIN_HOLE,
    WB_TERRAIN_SWAMP,
    WB_TERRAIN_WATER_WHIRLPOOL,
    WB_TERRAIN_BILLBOARD,
    WB_TERRAIN_CEMENT,
    WB_TERRAIN_JUNGLE,
    WB_TERRAIN_ROADBLOCK,
    WB_TERRAIN_STREET,
    WB_TERRAIN_STREET_UNDIGGABLE,
    WB_TERRAIN_TREE2,
    WB_TERRAIN_TREE3,
    WB_TERRAIN_TREE4,
    WB_TERRAIN_UNKNOWN
} WBTerrainType;

typedef enum WBPlanType {
    WB_PLAN_NONE = 0,
    WB_PLAN_BUGGY,
    WB_PLAN_DUCK,
    WB_PLAN_DIRTBUGGY,
    WB_PLAN_STEAMSHOVEL,
    WB_PLAN_DUMPTRUCK,
    WB_PLAN_FORKLIFT,
    WB_PLAN_DOZER,
    WB_PLAN_SPEEDBOAT,
    WB_PLAN_TUGBOAT,
    WB_PLAN_FREIGHTER,
    WB_PLAN_FROG,
    WB_PLAN_FISH,
    WB_PLAN_SNAIL,
    WB_PLAN_TREEBOT,
    WB_PLAN_REPAIRBOT,
    WB_PLAN_DEFENDER,
    WB_PLAN_GAS_STATION,
    WB_PLAN_MARINA,
    WB_PLAN_ROBOT_LAB,
    WB_PLAN_GUARD_TOWER,
    WB_PLAN_AIRPORT,
    WB_PLAN_FREEZEBOT,
    WB_PLAN_HOUSE,
    WB_PLAN_FACTORY,
    WB_PLAN_WINDMILL,
    WB_PLAN_GARAGE,
    WB_PLAN_NURSERY,
    WB_PLAN_COUNT
} WBPlanType;

typedef enum WBUnitType {
    WB_UNIT_NONE = 0,
    WB_UNIT_BUGGY,
    WB_UNIT_DUCK,
    WB_UNIT_DIRTBUGGY,
    WB_UNIT_STEAMSHOVEL,
    WB_UNIT_DUMPTRUCK,
    WB_UNIT_FORKLIFT,
    WB_UNIT_DOZER,
    WB_UNIT_SPEEDBOAT,
    WB_UNIT_TUGBOAT,
    WB_UNIT_FREIGHTER,
    WB_UNIT_FROG,
    WB_UNIT_FISH,
    WB_UNIT_SNAIL,
    WB_UNIT_TREEBOT,
    WB_UNIT_REPAIRBOT,
    WB_UNIT_DEFENDER,
    WB_UNIT_DEFENDER2,
    WB_UNIT_FREEZEBOT,
    WB_UNIT_COUNT
} WBUnitType;

typedef enum WBMonsterType {
    WB_MONSTER_NONE = 0,
    WB_MONSTER_CRAB,
    WB_MONSTER_WATER_CRAB,
    WB_MONSTER_GATOR,
    WB_MONSTER_SCORPION,
    WB_MONSTER_SHARK,
    WB_MONSTER_BOULDER,
    WB_MONSTER_TREX,
    WB_MONSTER_LION,
    WB_MONSTER_COUNT
} WBMonsterType;

typedef enum WBBuildingType {
    WB_BUILDING_NONE = 0,
    WB_BUILDING_GAS_STATION,
    WB_BUILDING_MARINA,
    WB_BUILDING_ROBOT_LAB,
    WB_BUILDING_GUARD_TOWER,
    WB_BUILDING_AIRPORT,
    WB_BUILDING_HOUSE,
    WB_BUILDING_FACTORY,
    WB_BUILDING_WINDMILL,
    WB_BUILDING_GARAGE,
    WB_BUILDING_NURSERY,
    WB_BUILDING_COUNT
} WBBuildingType;

typedef enum WBDirection {
    WB_DIR_RIGHT = 0,
    WB_DIR_LEFT,
    WB_DIR_UP,
    WB_DIR_DOWN
} WBDirection;

typedef enum WBGoalType {
    WB_GOAL_NONE = 0,
    WB_GOAL_ANYTHING,
    WB_GOAL_BUGGY,
    WB_GOAL_DUCK,
    WB_GOAL_DIRTBUGGY,
    WB_GOAL_STEAMSHOVEL,
    WB_GOAL_DUMPTRUCK,
    WB_GOAL_FORKLIFT,
    WB_GOAL_DOZER,
    WB_GOAL_SPEEDBOAT,
    WB_GOAL_TUGBOAT,
    WB_GOAL_FREIGHTER,
    WB_GOAL_FROG,
    WB_GOAL_FISH,
    WB_GOAL_SNAIL,
    WB_GOAL_TREEBOT,
    WB_GOAL_REPAIRBOT,
    WB_GOAL_DEFENDER,
    WB_GOAL_GAS_STATION,
    WB_GOAL_MARINA,
    WB_GOAL_ROBOT_LAB,
    WB_GOAL_GUARD_TOWER,
    WB_GOAL_AIRPORT,
    WB_GOAL_HOUSE,
    WB_GOAL_GARAGE,
    WB_GOAL_FREEZEBOT,
    WB_GOAL_LION,
    WB_GOAL_CRAB,
    WB_GOAL_FACTORY,
    WB_GOAL_WINDMILL,
    WB_GOAL_NURSERY,
    WB_GOAL_GATOR
} WBGoalType;

typedef struct WBItemDef {
    char symbol;
    char desc[WB_MAX_ITEM_DESC];
} WBItemDef;

typedef struct WBResourcePile {
    int red;
    int blue;
    int green;
    int yellow;
    int wheel;
    int energy;
    int white;
} WBResourcePile;

typedef struct WBCell {
    char rawSymbol;
    WBTerrainType terrain;
    bool hasUnit;
    WBUnitType unitType;
    WBDirection unitDirection;
    int unitEnergy;
    int unitEnergyDeci;
    WBResourcePile unitCargo;
    int unitCargoEnergyValue;  /* charge level (0-100) of carried energy brick; 0 = depleted */
    bool hasResource;
    WBResourcePile pile;
    int pileEnergyValue;
    bool resourceIsPlan;
    WBPlanType resourcePlanType;
    int resourcePlanCount;
    bool hasGoal;
    bool bonusGoal;
    WBGoalType goalType;
    bool goalSatisfied;
    bool hasGoalTerrain;      /* true for ':' tiles — part of a collect-goal zone */
    bool goalIsCollect;       /* true if goal = collect N monsters in zone */
    int  goalCollectCount;    /* number of monsters needed in zone */
    WBMonsterType goalCollectType; /* monster type required; WB_MONSTER_NONE = any */
    bool hasMonster;
    WBMonsterType monsterType;
    WBDirection monsterDirection;
    bool monsterFrozen;
    uint64_t monsterFrozenUntilMs;
    bool hasBuilding;
    WBBuildingType buildingType;
    WBDirection buildingDirection;
    int buildingHp;              /* 1000 = full health; 0 = no building */
    int buildingEnergy;          /* factory fuel level (0-1000); depletes 75/cycle (7.5%); 0 = dormant */
    bool isWhirlpool;
    int whirlpoolId;
    int factoryColor;
    WBTerrainType unitCargoTreeType; /* tree type the treebot is carrying (WB_TERRAIN_NORMAL when empty) */
    uint8_t terrainVariant;          /* variant index for JUNGLE (0-3) and STREET (0-9) tiles */
} WBCell;

typedef struct WBMap {
    char name[WB_MAX_NAME];
    int width;
    int height;
    WBCell cells[WB_MAX_MAP_H][WB_MAX_MAP_W];
    WBItemDef itemDefs[WB_MAX_ITEM_DEFS];
    int itemDefCount;
    int planInventory[WB_PLAN_COUNT];
    bool possiblePlans[WB_PLAN_COUNT];
    bool bonusAvailable;
    int goalScore;
    int bonusScore;
    bool isWB2Level;
} WBMap;

bool wbMapLoad(const char* path, WBMap* outMap);
const WBItemDef* wbMapFindItem(const WBMap* map, char symbol);
const char* wbMapDescribeSymbol(const WBMap* map, char symbol);
bool wbTerrainIsBase(char c);
WBTerrainType wbTerrainFromChar(char c);
const char* wbTerrainName(WBTerrainType terrain);
const char* wbPlanName(WBPlanType planType);
const char* wbUnitName(WBUnitType unitType);
WBPlanType wbPlanTypeFromName(const char* name);
WBUnitType wbUnitTypeFromName(const char* name);
WBGoalType wbGoalTypeFromName(const char* name);
const WBResourcePile* wbUnitRecipe(WBUnitType unitType);
const WBResourcePile* wbBuildingRecipe(WBBuildingType buildingType);
bool wbUnitCanTraverse(WBUnitType unitType, WBTerrainType terrain);
WBMonsterType wbMonsterTypeFromName(const char* name);
WBBuildingType wbBuildingTypeFromName(const char* name);
const char* wbMonsterName(WBMonsterType monsterType);
const char* wbBuildingName(WBBuildingType buildingType);
bool wbBuildingCanBePlacedOn(WBBuildingType buildingType, WBTerrainType terrain);
bool wbMonsterCanTraverse(WBMonsterType m, WBTerrainType terrain);
float wbMonsterSpeed(WBMonsterType m);
int wbMonsterWalkFrames(WBMonsterType m);
bool wbMonsterIsAmphibious(WBMonsterType m);
int wbMonsterRestEvery(WBMonsterType m);
int wbMonsterRestForMs(WBMonsterType m);

#endif
