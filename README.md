# Lamadil GBA

A tiny, bare-metal Game Boy Advance framework for full-color Mode 3 games.
It keeps the original project's software framebuffer and beam-racing
presentation strategy while adding safe drawing, input, and PCM audio.

## Features

- 240×160, 15-bit Mode 3 software framebuffer in EWRAM
- DMA3 frame presentation
- Clipped sprites, including negative positions and odd widths
- Fast unchecked pixels plus a safe clipped-pixel helper
- Held, pressed, and released state for all ten GBA buttons
- Signed 8-bit mono PCM through Direct Sound FIFO A, Timer 0, and DMA1
- Fixed-point sine and cosine lookup functions
- A host-side renderer test that does not require a GBA emulator
- Automatic PNG-to-GBA conversion through grit

## Requirements

- A recent [devkitARM](https://devkitpro.org/wiki/Getting_Started)
- Perl, used by the included ROM-header fixer
- [mGBA](https://mgba.io/) for `make run`
- A native C compiler for `make host-test`

If `DEVKITARM` is set, the Makefile uses `$DEVKITARM/bin`. Otherwise it looks
for `arm-none-eabi-*` tools on `PATH`.

## Build

```sh
make
make run
make host-test
```

`ccompilartu.sh` remains as a convenience wrapper around `make run`.

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

## Game callbacks

Include `lamadil.h` and implement:

```c
void game_start(void);
void game_update(void);
```

The framework polls input immediately before each `game_update()` and presents
the framebuffer immediately afterward.

## Drawing

`set_pixel()` is intentionally unchecked and is appropriate for coordinates
already known to be valid. Use `set_pixel_clipped()` for generated or
potentially off-screen coordinates.

`draw_sprite()` accepts signed positions and clips all four edges. Sprites are
arrays of row-major `Color` pixels.

The framebuffer occupies 76,800 bytes of the GBA's 256 KiB EWRAM.

## Input

```c
if (lamadil_key_down(LAMADIL_KEY_LEFT))
    player_x--;

if (lamadil_key_pressed(LAMADIL_KEY_A))
    jump();
```

Available keys are `A`, `B`, `SELECT`, `START`, `RIGHT`, `LEFT`, `UP`, `DOWN`,
`R`, and `L`, each prefixed with `LAMADIL_KEY_`.

## PCM audio

```c
lamadil_audio_play(samples, sample_count, 8192, false);
```

Audio data must:

- contain signed 8-bit mono PCM;
- be aligned to four bytes;
- contain a multiple of 16 samples;
- remain allocated until playback finishes.

Supported sample rates are 2,000–32,768 Hz. Passing `true` as the final
argument loops the sample. `lamadil_audio_stop()` stops it explicitly.

PCM uses DMA1 and timers 0–1. DMA3 is reserved for video presentation.

## Demo controls

- D-pad: move the bitmap
- A: play the demo tone once
- B: loop the demo tone
- Start: stop audio

## Design notes

Mode 3 consumes too much VRAM for hardware page flipping. A complete frame
also does not comfortably fit inside VBlank. Presentation therefore begins at
scanline 50 and copies the image top-to-bottom behind the LCD beam. This keeps
the distinctive approach of the original experiment while DMA3 performs the
bulk transfer.

The included `crt0.s` and `lnkscript` originate from Jeff Frohwein's
public-domain GBA startup package. Interrupt support is enabled in single
handler-table mode for audio.
