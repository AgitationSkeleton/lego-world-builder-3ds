# LEGO WB 3DS Audio Status

## Current Situation
Audio assets are present and load, but runtime audio playback still fails in the current Citra setup.

The port has already:
- extracted WAV files into `romfs/audio`
- implemented audio clip loading
- attempted `ndsp` initialization
- attempted a DSP fallback path inspired by NetPass / Citra guidance

## Diagnostic Result Seen In-Game
The user reported these diagnostics on the top screen:

```text
Audio OFF clips 12 music -1 sfx -1
init D88DA7FA open CBB0447B
rb 00000000
```

The exact `8/B` glyphs were slightly ambiguous at low resolution, but the meaning was consistent.

## Interpretation
- `clips 12`:
  - game WAV assets are loading successfully
- `music -1 sfx -1`:
  - no channels are active because audio never initialized
- `init D88DA7FA`:
  - `ndspInit()` is failing
- `open CBB0447B`:
  - fallback attempt to open the DSP/system-title source also fails
- `rb 00000000`:
  - fallback path never reached a successful retry/init state

Conclusion:
- This is not mainly a WAV or playback-queue bug.
- The runtime has no working DSP backend in the current emulator configuration.

## Relevant Code
Main audio code lives in:
- `d:\PsyDoom-Project\LEGOWB3DS\source\main.c`

Useful functions/areas:
- `initAudioSystem`
- `loadWavClip`
- `setupAudioChannel`
- `playClip`
- `playMusicClip`
- `playSfxClip`
- `updateAudioRuntime`
- the top-screen audio debug rendering

## External Reference Used
The session referenced NetPass as a known 3DS homebrew with working menu music:
- `D:\PsyDoom-Project\Misc Sources\NetPass\netpass-main`

The idea was to follow its DSP init / fallback approach where possible.

## User-Provided Resources
The user later added dumped 3DS data and Citra references:
- `D:\LEGOWB-Project\RequestedSources\nand`
- `D:\LEGOWB-Project\RequestedSources\NAND_03-24-26_22-50-49.bin`
- `D:\LEGOWB-Project\RequestedSources\3dsutils.3dsx`
- `D:\LEGOWB-Project\RequestedSources\Citra's User Directory.mhtml`
- `D:\LEGOWB-Project\RequestedSources\Dumping System Archives and the Shared Fonts.mhtml`

Important finding:
- At handoff time, the dumped NAND is present as source material in `RequestedSources`, but there is no confirmed Citra user-directory layout in the workspace showing those files placed where the running emulator is actually reading them from.

## Practical Meaning
If audio still fails with the same diagnostics:
- the game code is likely not the main blocker
- the actual Citra user directory / NAND setup being used at runtime still needs verification

## Recommended Next Audio Step
Do not spend more time changing WAV playback logic first.

Instead:
1. Locate the exact Citra user directory used by the emulator instance the user is launching.
2. Verify the dumped NAND/system files are in the correct structure for that Citra instance.
3. Retest the current build.
4. Only if `ndspInit()` still fails after that, resume code-level audio debugging.

## What Not To Assume
- Do not assume that because NAND files exist somewhere in the workspace, Citra is using them.
- Do not assume this is a mute-toggle bug.
- Do not assume the game’s audio assets are missing; clip loading already proved otherwise.

