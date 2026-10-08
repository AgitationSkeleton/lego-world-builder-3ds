# Porting Notes

This prototype is tied directly to recovered World Builder data and scripts.

Implementation rule:
- Rendering and gameplay changes must be justified against the recovered Director code first, then adapted to 3DS.
- If the prototype and the original disagree, the original code wins unless a platform constraint requires a documented deviation.
- Back up any existing file before editing it. Store backups under `backups/<timestamp>_<description>/`.

Recovered sources currently driving implementation:
- `map1.1.txt` exported from the original movie
- `MovieScript 89 - _main.ls`
- `ParentScript 129 - map display manager.ls`
- `ParentScript 130 - plan manager.ls`
- `ParentScript 131 - worlds manager.ls`
- `ParentScript 137 - object.generic parent.ls`

Prototype-to-original mapping:
- Visible map window: prototype uses `12x9`, matching `map display manager`
- Real mission data: prototype reads the original recovered text map format
- Touch screen: prototype uses a minimap/control surface, matching the original two-layer feel of map + UI
- Art: prototype now uses extracted original cast members packed into a native 3DS sprite atlas

Faithfulness gaps to close next:
1. Refine sprite alignment/registration to match the original Director renderer more closely
2. Recreate the original map manager's scroll, skew, and isometric presentation exactly
3. Parse `inventory`, `goal`, `bonusgoal`, `pile`, `plan`, and `unit` entries into gameplay structs
4. Implement object scripts from recovered Lingo parent scripts
5. Add save/progress handling based on recovered pref/world logic

World-select screen and tutorial (taken from the scores, October 2026):
- `tools/gen_worldmap_data.py` reads the VWSC score of `worldbuilder.dcr` (frames
  "world 1".."world 5") and `worldbuilder2.dcr` ("world 1", "world 2") and writes
  `include/wb_worldmap_data.h` and `romfs/meta/worldmask_wN.bin`. Mission icons,
  decorations, mini-units and their obstacles are placed exactly as the score
  places them; none of it is estimated by eye any more. Rerun it rather than
  editing the header.
- Mission icons follow 'mission icon behavior 2' (1:1 with the map, the 15 fps
  flag cycle from a random frame, the 2 px / 1800 ms bob, the 200 ms hover blink,
  'mission name rollover text'). Mini-units follow 'mini-unit behavior', including
  okPos() against the map's palette indices, the WB2 'wb2-map-mask' and the world 5
  'prehistoric_world_map_guide'.
- World 1 Mission 1 runs 'tutorial manager' over the 'tutorial_sequence' steps.
  `tools/gen_tutorial_assets.py` extracts its arrows.
- A mission opens on the map centre, or on `[map] center=x,y`, as readmap() does,
  and the pad scrolls a tile per 150 ms like clickToScroll().

Documented deviations (platform constraints):
- The world map is scaled to fit the bottom screen whole, under a white band for
  the world name and arrows (the score's white rectangle). The stage is 610x440;
  filling the 320x240 screen instead cut markers off its sides.
- The hover label is drawn in the system font with a shadow; the original's 6 pt
  pixel font would be unreadable at this scale.
- Rolling the mouse over a goal or a pile is a tap. Piles now show their bricks
  when tapped, as the original's rollover popup did.
- The tutorial's sentences that name a mouse action name the 3DS control instead
  (pad, tap, R, X, Y, A). Its bubble sits to the right of the sidebar so the plan
  recipe and info panel it points at stay visible; the scroll-button and sidebar
  arrows point at the matching 3DS place. Start is its 'quit tutorial' button.

Units, monsters and pathing (the original object model, October 2026):
- `behavior_audit/README.md` (one level up) lists what differed. All of it now
  follows `vehicle.generic`, `monster.generic`, `object.generic`, the building
  parents and `map display manager`:
  - Every unit, monster and building is an object stepped once per 15 fps movie
    frame, in the order it was made (the actorList).
  - A step claims the next tile at once (MoveObject), and the sprite slides in
    over 1000/speed ms. Moving units are therefore targets, block tiles, take
    swamp damage and die like standing ones.
- Pathing is the original A*: an estimate of 1.5 x Manhattan, with the swamp
  penalty only in the estimate; random tie-breaks; reopened nodes; the 1 s give-up.
  A goal that is occupied, or of the wrong terrain, ends the path beside it.
- When a path is blocked mid-way, the unit waits, sidesteps and re-paths as
  followPath does. A real no-path shudders for one retry window (about 1 s) and
  then gives up.
- Selecting or ordering a unit makes the view follow it (centerVehicle and
  checkScroll); the pad's manual scroll stops the follow.
- The selection never snaps the view any more.
- Selecting sets move mode; the special action is the menu's second entry (A).
- Batteries keep their own charge in piles and cargo. Building takes the best
  battery, then the bricks in getAll8Neighbors order. Take-apart returns every
  battery with the charge it had.
- WB2 production buildings run on their battery (7.5 a cycle, with no refuel). The
  freezebot has range 2, costs 2 per freeze, recharges for 3 s, and the frozen
  monster blinks for its last 4 s.
- Terrain lists, WB2 recipes, attack lists, shields, recharge lists, energy costs
  and the lion's rest time are as in the WB1/WB2 configs.
- The click spike (member spike2) is appended to `worldbuilder.t3s` as index 122.

Documented deviations (object model):
- Pixel offsets use the original's pixel counts as 3DS screen pixels rather than
  halving them with the map: shudder 2 px, driving bob 2 px, selection arrow 4 px,
  goal drift 3 px. At half size they would be 1 px and hard to see.
- The slide is drawn from the sim clock every 60 fps frame. The original moved the
  sprite once per 15 fps frame. The shudder, bob and arrow still change once a
  movie frame, as in the original.
- Opening the plan collection or the start menu pauses the game, so the sim clock
  stops there. The world does not catch up on gaps over 250 ms.
- A unit drops its pPixelOffset when its path ends. The original kept the last
  value, which could leave a unit drawn a pixel or two off.
- A destroyed charger stops charging its unit. The original kept charging from the
  dead object.
- The damage puff stays on the tile that was hit; the original's followed the
  object.
- Each goal's drift phase and period come from its tile, not random().
- The defender's standing attack frames have no sprites in the port's sheets, so
  it shows its standing pose while attacking.
- The click spike is drawn under the goal and pile popups.
