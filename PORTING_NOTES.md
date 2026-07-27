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
