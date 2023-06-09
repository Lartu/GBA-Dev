uint16 screen_buffer[screen_pixels] __attribute__((section(".ewram")));

inline void set_video_mode()
{
    // Bitmap mode: 3
    *(uint32 *)reg_display_control = 0x0403;
}

inline Color create_color(uint32 red, uint32 green, uint32 blue)
{
    blue = ColorComponentMask & blue;
    red = ColorComponentMask & red;
    green = ColorComponentMask & green;
    return (blue << 10) | (green << 5) | red;
}

#define blocksize 8
typedef struct BLOCK
{
    uint32 data[blocksize];
} BLOCK;

void memcpy32(void *dst, const void *src, uint32 wdcount)
{
    uint32 blkN = wdcount / blocksize, wdN = wdcount & (blocksize - 1);
    uint32 *dstw = (uint32 *)dst, *srcw = (uint32 *)src;
    if (blkN)
    {
        // 8-word copies
        BLOCK *dst2 = (BLOCK *)dst, *src2 = (BLOCK *)src;
        while (blkN--)
            *dst2++ = *src2++;
        dstw = (uint32 *)dst2;
        srcw = (uint32 *)src2;
    }
    // Residual words
    while (wdN--)
        *dstw++ = *srcw++;
}

void memset32(void *dst, uint32 value, uint32 wdcount)
{
    uint32 valuex[8] = {value, value, value, value, value, value, value, value};
    uint32 blkN = wdcount / blocksize, wdN = wdcount & (blocksize - 1);
    uint32 *dstw = (uint32 *)dst, *srcw = valuex;
    if (blkN)
    {
        // 8-word copies
        BLOCK *dst2 = (BLOCK *)dst, *src2 = (BLOCK *)&valuex;
        while (blkN--)
            *dst2++ = *src2;
        dstw = (uint32 *)dst2;
        srcw = (uint32 *)src2;
    }
    // Residual words
    while (wdN--)
        *dstw++ = *srcw;
}

inline void fill_screen(Color color)
{
    uint32 color_value = (color << 16) | color;
    memset32(screen_buffer, color_value, screen_pixels / 2);
}

inline void set_pixel(uint32 x, uint32 y, Color color)
{
    screen_buffer[x + y * screen_width] = color;
}

inline void draw_sprite(uint32 x, uint32 y, const uint16 *sprite, uint32 w, uint32 h)
{
    const uint32 original_w = w;
    w = x+w >= 240 ? 240 - x : w;
    h = y+h >= 160 ? 160 - h : h;
    const uint32 half_width = w / 2;
    for (uint32 _h = 0; _h < h; ++_h)
    {
        memcpy32(screen_buffer + (x + (y + _h) * screen_width), sprite + (_h * original_w), half_width);
    }
}

inline int32 lam_sin(uint32 alpha)
{
    // Alpha goes from 0 to 512.
    // Alpha is shifted 8 bits for precission.
    // If we were to shift it less bits, smaller increases
    // in alpha would produce bigger increases in sin(alpha).
    // In this case (8 shifts):
    // 0 = 0
    // PI/2 = 32768
    // PI = 65536
    // PI * 4/3 = 98304
    return sin_lut[(alpha >> 8) & 0xFF] * (((alpha>>8)& 0x1FF) > 255 ? -1 : 1);
    // The output of this function should be >> an arbitrary
    // number of bits to be used, depending on the range of
    // values desired.
}

inline int32 lam_cos(uint32 alpha)
{
    return lam_sin(alpha + (PI/2));
}

int lamadil_start()
{
    set_video_mode();
    start();
    while (1)
    {
        // Primero actualizo el frame tranquilo, nadie me corre.
        update();

        // TODO: aca tiene que haber un sleep hasta el vblank
        // por un tema de bateria. Si no me jode llevar esto a
        // 30 fps, puedo hacer que un frame se renderice y el
        // otro se procese, lo cual tambien me permite optimizar
        // bastante la bateria, porque termino y duermo hasta el
        // proximo frame sin riesgo a tener framedrop.

        // Despues espero a estar por debajo del vcount 50
        // y dibujo tranquilo, nadie me corre tampoco.
        while (*(vuint16 *)reg_vcount < 50)
        {
        };
        memcpy32((uint32 *)mem_video_memory, (uint32 *)screen_buffer, screen_pixels / 2);

        // Asi me aseguro que puedo hacer todo sin tearing.
    }
    return 0;
}