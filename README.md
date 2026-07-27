# LEGO World Builder for Nintendo 3DS

A native Nintendo 3DS port of **LEGO World Builder**, the Shockwave strategy
game published on LEGO.com in the early 2000s.

The original was a Macromedia Director title: an isometric world where you
scout terrain, uncover build plans, gather resources and assemble vehicles and
buildings to complete each mission's goals. It went offline with the rest of
LEGO's Shockwave catalogue when browser plugins died, and it has not been
playable in a browser for many years.

This port reimplements the game in C against the 3DS hardware, working from the
decompiled Director scripts as the reference for behaviour. It is not an
emulator and not a wrapper: the map display, plan and goal handling, resource
model and vehicle logic were rebuilt to match what the original code does.

## Credits

**LEGO World Builder was created by [Gamelab](https://en.wikipedia.org/wiki/Gamelab)
for The LEGO Group.** All of the game's design, artwork, music and sound are
their work, and this port would not exist without it. Gamelab closed in 2009.

LEGO® is a trademark of The LEGO Group, which does not sponsor, authorise or
endorse this project. This is an unofficial, non-commercial fan port.

The original game files were recovered from the
[BioMedia Project](https://www.biomediaproject.com/), which
has preserved LEGO's Shockwave catalogue for years.

## Getting it

Grab the newest [release](../../releases). Two files, and you only need one:

| File | What to do with it |
| --- | --- |
| `legowb3ds.cia` | Install it with FBI. It appears on the HOME menu with its own icon. |
| `legowb3ds.3dsx` | Copy it to `sdmc:/3ds/` and start it from the Homebrew Launcher. |

Either needs a 3DS running custom firmware. Everything the game needs is packed
inside, so there is nothing else to copy across.

## The two screens

The bottom screen is the world: the isometric map, scrollable and touchable.
The top screen is the sidebar the original kept to the right of the play area —
the selected tile, your inventory of plans and resources, and mission goals.

## Controls

In a mission:

| Button | Action |
| --- | --- |
| D-pad, Circle Pad | Scroll the map |
| Touch | Tap a tile to select it, drag to scroll |
| L | Act with the selected unit: pick up, drop, build. On a factory, cycles its colour |
| B | Cancel, and dismiss a goal popup |
| X | Disassemble what is selected |
| Y | Show or hide the info panel for the selected tile |
| R, Select | Open the plan menu |
| Start | Menu |

On the world select screen:

| Button | Action |
| --- | --- |
| L, ZL | Previous world |
| R, ZR | Next world |
| Select | Show your licence, once you have earned it |

There is a cheat, entered the way you would expect a game from that era to
want it.

Progress is saved to `sdmc:/legowb3ds/save.txt`.

## Building it

You need [devkitPro](https://devkitpro.org/) with devkitARM, libctru, citro3d,
citro2d and the 3DS tools (`tex3ds`, `3dsxtool`, `smdhtool`). On Linux:

```sh
dkp-pacman -S 3ds-dev
make -j"$(nproc)"
```

That produces `legowb3ds.3dsx`, and then `legowb3ds.cia` via bannertool and
makerom. Windows builds of both are in `tools/`; on anything else, point the
makefile at your own:

```sh
make MAKEROM=/path/to/makerom BANNERTOOL=/path/to/bannertool
```

`gfx/*.t3s` and the PNGs beside them are compiled into texture atlases by
tex3ds during the build and land in `romfs/gfx`, so that directory is generated
rather than committed. They rebuild byte for byte from the sources.

### Reproducing a particular build

The build is deterministic: the same sources and the same toolchain give the
same bytes. Built with devkitARM r67.1, libctru 2.7.0, citro3d 1.7.1 and
citro2d 1.7.0, this tree reproduces the 3dsx and the SMDH of the original
release exactly.

CI pins `devkitpro/devkitarm:20260221`, which matches on everything except
citro2d, where it carries 1.6.0. That is worth about eight hundred bytes in the
3dsx. The game is the same either way; only the bytes differ.

The CIA is a separate matter. It carries debug information recording the
absolute path it was built from, so two builds of the same sources in two
different directories produce two different CIAs. Stripping the debug sections
from both leaves the ELFs identical.

### Releasing

Push a tag beginning with `v`. The release workflow builds through the same
path as every other build and publishes the 3dsx and the CIA against that tag.

```sh
git tag v1.0.0
git push origin v1.0.0
```

## State of the port

Playable and complete enough to finish missions, with known gaps. `PARITY_CHECK.md`
records where the port still diverges from the original Director behaviour, and
`AUDIO_STATUS.md` covers the sound work. `PORTING_NOTES.md` describes the rule
the port is built on: when the port and the original disagree, the original
wins unless the hardware forces a documented deviation.

## Licence

The port's own code, everything under `source/` and `include/`, is offered for
anyone to read, build and learn from.

The game's data — the artwork in `gfx/`, and the audio and map files in
`romfs/` — is LEGO's and Gamelab's, included here so the port is playable. No
ownership of it is claimed and none is transferred. If The LEGO Group would
rather it were not distributed, it will be taken down.
