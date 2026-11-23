# Lamadil GBA

A tiny, bare-metal Game Boy Advance framework with page-flipped bitmap
rendering, input, tracker music, and sound effects.

## Features

- Switchable Mode 4 and Mode 5 page-flipped bitmap backends
- 240×160 with a fixed RGB332 palette in Mode 4
- 160×128 with direct 15-bit color in Mode 5
- Clipped sprites, including negative positions and odd widths
- Fast unchecked pixels plus a safe clipped-pixel helper
- Held, pressed, and released state for all ten GBA buttons
- MOD, S3M, XM, and IT tracker music through Maxmod
- WAV sound effects mixed with music
- Fixed-point sine and cosine lookup functions
- A host-side renderer test that does not require a GBA emulator
- Automatic PNG-to-GBA conversion through grit
- Automatic tracker/WAV soundbank conversion through mmutil

## Requirements

- A recent [devkitARM](https://devkitpro.org/wiki/Getting_Started), including
  Maxmod, mmutil, bin2s, and grit
- Perl, used by the included ROM-header fixer
- [mGBA](https://mgba.io/) for `make run`
- A native C compiler for `make host-test`

The Makefile defaults to the standard `/opt/devkitpro` installation. Override
`DEVKITPRO`, `DEVKITARM`, or `LIBGBA` when using another location.

## Build

```sh
make                    # Mode 4 (default)
make mode4              # clean Mode 4 build
make mode5              # clean Mode 5 build
make run
make host-test
```

`ccompilartu.sh` remains as a convenience wrapper around `make run`.

`VIDEO_MODE` is a build-time switch because Mode 4 and Mode 5 have different
pixel formats and resolutions. Both modes use the same `Color`, drawing, input,
and audio API.

## Assets

Put source PNG files in `assets/`. During `make`, every PNG is converted into
16-bit GBA bitmap data under `build/assets/` using grit. Generated files are
ignored by Git and recreated whenever the corresponding PNG changes.

For `assets/name.png`, grit generates:

- `build/assets/name.c`
- `build/assets/name.h`
- the pixel array `nameBitmap`
- the byte-length constant `nameBitmapLen`

Include the generated header through `assets/assets.h` or directly from game
code. Image dimensions remain game metadata and should be declared alongside
the asset aliases in `assets/assets.h`.

### Audio assets

Put tracker modules and effects in `assets/audio/`. Supported source extensions
are `.mod`, `.s3m`, `.xm`, `.it`, and `.wav`. During `make`, mmutil combines
them into `build/audio/soundbank.bin` and generates `soundbank.h` containing
numeric identifiers derived from the filenames.

For example, `Overworld.xm` and `Jump.wav` become identifiers resembling
`MOD_OVERWORLD` and `SFX_JUMP`. Initialize the generated bank and start music
from `game_start()`:

```c
#include "soundbank.h"
#include "soundbank_bin.h"

void game_start(void)
{
    lamadil_audio_init(soundbank_bin);
    lamadil_song_play(MOD_OVERWORLD, true);
}
```

Call `lamadil_sound_play(SFX_JUMP)` for a sound effect. Lamadil calls Maxmod's
per-frame update automatically after `game_update()`.

## Game callbacks

Include `lamadil.h` and implement:

```c
void game_start(void);
void game_update(void);
```

The framework polls input immediately before each `game_update()`. Maxmod is
serviced at the full GBA frame rate, while drawing and page flipping alternate
to produce a stable 30 FPS visual update without skipping audio frames.

## Drawing

`set_pixel()` is intentionally unchecked and is appropriate for coordinates
already known to be valid. Use `set_pixel_clipped()` for generated or
potentially off-screen coordinates.

`draw_sprite()` accepts signed positions and clips all four edges. Sprites are
arrays of row-major `Color` pixels.

Rendering goes directly to the hidden VRAM page; `lamadil_present()` swaps that
page into view. Mode 4 converts 15-bit `Color` values to a fixed RGB332
palette. Mode 5 retains the original 15-bit values but has a smaller viewport.

## Input

```c
if (lamadil_key_down(LAMADIL_KEY_LEFT))
    player_x--;

if (lamadil_key_pressed(LAMADIL_KEY_A))
    jump();
```

Available keys are `A`, `B`, `SELECT`, `START`, `RIGHT`, `LEFT`, `UP`, `DOWN`,
`R`, and `L`, each prefixed with `LAMADIL_KEY_`.

## Music and sound

```c
lamadil_song_play(MOD_OVERWORLD, true);
LamadilSoundHandle handle = lamadil_sound_play(SFX_JUMP);
lamadil_sound_stop(handle);
```

Music can be played once or looped, paused, resumed, stopped, and assigned a
master volume from 0 through 1024. Maxmod shares eight software mixer channels
between the song and effects. It owns the Direct Sound audio DMA/timers and
uses Lamadil's VBlank interrupt slot; DMA3 is used for fast page fills.

## Demo controls

- D-pad: move the bitmap
- A: play the explosion effect
- B: play the ambulance effect
- Start: pause or resume the tracker song
- Select: restart the tracker song

Call `lamadil_set_rendering_enabled(false)` when an audio-only screen is useful.
This forces a blank display and skips framebuffer drawing and DMA3
presentation while retaining one Maxmod update per hardware frame.

## Design notes

Mode 4 and Mode 5 both fit two bitmap pages in VRAM, so presentation changes a
display-page bit instead of copying an entire framebuffer. Drawing and page
flipping occur on alternating hardware frames so Maxmod's mixer remains on its
required 60 Hz schedule. Mode 4 is the default because it retains the full
240×160 viewport; Mode 5 is selected when direct 15-bit color matters more
than resolution.

The included `crt0.s` and `lnkscript` originate from Jeff Frohwein's
public-domain GBA startup package. Interrupt support is enabled in single
handler-table mode for audio.
