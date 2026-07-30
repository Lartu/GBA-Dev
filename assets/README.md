# Source assets

Place PNG images in this directory. The root Makefile automatically runs grit
for each image and writes generated C/header files to `build/assets/`.

Images are converted as linear, uncompressed, 16-bit GBA BGR555 bitmaps. No
palette or tile map is emitted. A file called `example.png` becomes:

```text
build/assets/example.c
build/assets/example.h
```

Its pixel array is named `exampleBitmap`. Add dimensions and any convenient
aliases to `assets/assets.h`, then include that header from the game.

The generated directory is disposable. Edit the PNG sources, not generated C.
