# LEGO World Builder 3DS Parity Check

Date: 2026-03-24

Source of truth:
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\MovieScript 89 - _main.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\ParentScript 129 - map display manager.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\ParentScript 153 - vehicle.generic parent.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\BehaviorScript 142 - build plan icon behavior.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\ParentScript 140 - goal.goal parent.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\ParentScript 141 - goal.bonus parent.ls`
- `D:\LEGOWB-Project\00WBWORKFOLDER\analysis\projectorrays_out\worldbuilder\casts\Internal\BehaviorScript 113 - right side info display behavior.ls`

## Summary

The port already has a good foundation in these areas:
- recovered map data and object/resource layout
- directional vehicle sprites
- energy depletion and disassembly resources
- hidden-vs-known plans in inventory
- bottom-screen world / top-screen sidebar adaptation

The biggest remaining parity gaps are still in:
- vehicle movement state machine
- goal popup / completion flow
- build preview and plan behavior details
- sidebar presentation details
- audio playback

## Findings

### 1. High: Vehicle movement logic is still simplified relative to the original

Original:
- `vehicle.generic parent.ls` uses `mapclick`, `gotoPos`, `followPath`, `checkDestination`, and `moveTo` as a linked state machine.
- `#pick` and `#drop` use `gotoPos(..., [#reason: ..., #nextto: #true])` when the target is not the same tile.
- `followPath` has distinct blocked states:
  - short wait/retry
  - side-step to a random neighbor
  - re-path to goal after enough failed side-steps
  - `bouncyVehicle()` is separate from blocked pacing
- `checkDestination` performs the actual action after movement completes.

Port:
- `beginMove`, `updateMovement`, `computeAdjacentActionDestination`, and `handleWorldTouch` in `source/main.c` implement a much lighter version of this.
- The current code does have wait / sidestep / re-path, but it still compresses several original states together and does not yet mirror the original `checkDestination` flow closely enough.

Impact:
- movement edge cases still differ from the original game
- interrupted actions and occupied-target behavior are closer now, but still not exact
- visual motion and action timing still feel adapted rather than reconstructed

### 2. High: Goal popup and completion flow are still adapted, not reconstructed

Original:
- `goal.goal parent.ls` and `goal.bonus parent.ls` use separate info popup and completion popup/menu paths.
- Hover/info popup and completion popup are distinct behaviors.
- Completion goes through `goal_popup_menu.show(me.pTile.pos, #goal/#bonus)` and button-driven follow-up behavior.

Port:
- `showGoalPopup` and `drawBottomGoalPopup` in `source/main.c` collapse this into one simplified bubble system with a completion variant.

Impact:
- text content is functionally there, but presentation flow still diverges
- button placement, popup sizing, and lifetime are still approximations
- this is one of the most visible remaining UI parity gaps

### 3. High: Build-plan behavior is only partially ported

Original:
- `build plan icon behavior.ls` distinguishes:
  - unknown plan icon vs known plan
  - hover `checkBuild`
  - recipe/info text
  - build-outline ghost with terrain/resource legality
  - actual `doBuild` after `checkBuild`
- the build ghost chooses a member based on the true class and uses original `locZ` placement and resource checks.

Port:
- `drawBuildPreview`, `canBuildPlanAt`, `checkResourcesAround`, and `tryBuildPlan` cover the basic path, but the behavior is still narrower than the original script.

Impact:
- the general behavior is recognizable, but still lacks full parity in ghost presentation and information flow
- the original plan-click / hover / build feedback loop is not fully mirrored

### 4. Medium: Sidebar behavior is structurally correct but still too custom

Original:
- `right side info display behavior.ls` drives the entire selected-unit sidebar.
- `showEnergy` explicitly sizes the green/red energy stripe, swaps the empty icon, and updates the recipe energy sprite to match battery state.
- `showPlanBricks` and `showUnit` have exact separator and sprite visibility rules.

Port:
- `drawSidebarSelectionPanel`, `drawSidebarPlanPanel`, `drawSidebarRecipe`, and `drawSidebarEnergy` in `source/main.c` reproduce the basic content, but they still place things manually rather than reconstructing the original layout behavior sprite-for-sprite.

Impact:
- the sidebar is useful and close in spirit
- alignment and icon placement still drift from the original

### 5. Medium: Audio parity is still missing entirely at runtime

Original:
- the decompiled scripts call `SndSFX(...)` in expected places for movement, goals, plans, digging, assembly, menu interaction, and more.

Port:
- the code now loads clips and attempts to initialize `ndsp`, but runtime audio is still not confirmed working.

Impact:
- this is a direct missing feature, not just a polish gap
- even when gameplay logic is close, the game still feels incomplete without audio

### 6. Medium: Goal/resource/object display rules are not yet fully script-driven

Original:
- `map display manager.ls` and object/resource parents use a sprite/member-driven placement system with `locZ`, drift, popup hooks, fog handling, and per-object placement rules.

Port:
- rendering is much closer than before, but still consolidated into custom draw helpers in `source/main.c`.

Impact:
- remaining visual artifacts are harder to eliminate until more of the original object display model is mirrored directly

## Feature Status

### Already close
- Map loading and map-derived world layout
- Camera panning and minimap concept
- Selection and directional facing persistence
- Resource pile composition
- Plan secrecy in inventory
- Energy depletion and disassembly energy carry-through

### Partially faithful
- Vehicle movement
- Goal popup/info flow
- Build ghost and build legality checks
- Sidebar unit/plan presentation
- Main menu placement and behavior

### Not yet faithful
- Audio/music playback
- Full original goal completion menu flow
- Full `followPath` parity
- Full script-driven sidebar/menu presentation

## Enacted Plan

### Phase 1: Runtime parity blockers
1. Add in-app audio diagnostics and finish `ndsp` playback until music/SFX work reliably.
2. Finish `vehicle.generic parent.followPath` parity:
   - preserve original wait/retry cadence
   - preserve side-step / re-path thresholds
   - keep actions resolving through a `checkDestination`-style post-move step
3. Tighten occupied-tile and action-target behavior to exactly match `mapclick` + `gotoPos(... #nextto ...)`.

### Phase 2: Goal and plan flow parity
1. Split goal hover/info popup from goal completion popup/menu the way `goal.goal parent` and `goal.bonus parent` do.
2. Rebuild goal completion UI from `goal popup menu behavior` and `goal popup button beh`.
3. Rebuild build-plan hover/info flow directly from `build plan icon behavior`.

### Phase 3: Sidebar and menu presentation parity
1. Refactor the top-screen sidebar to follow `right side info display behavior` more literally.
2. Move more of the current hard-coded layout into member/anchor-driven rules taken from the original assets and scripts.
3. Re-check all selected-unit, plan, and info layouts against the original sidebar.

### Phase 4: Render-model cleanup
1. Continue moving per-object placement rules toward the original object/resource parent scripts.
2. Eliminate remaining sprite artifacts only after the underlying placement/scaling rules match the original more closely.

## Recommended Next Implementation Order

1. Audio diagnostics and playback fix
2. Full `followPath` and `checkDestination` parity pass
3. Goal popup/menu reconstruction
4. Build-plan hover/build reconstruction
5. Sidebar layout reconstruction
