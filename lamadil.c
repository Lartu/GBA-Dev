uint16 screen_buffer[screen_pixels] __attribute__ ((section (".ewram")));

void set_video_mode()
{
    // Bitmap mode: 3
    *(uint32*) reg_display_control = 0x0403;
}

Color create_color(uint32 red, uint32 green, uint32 blue)
{
    blue = ColorComponentMask & blue;
    red = ColorComponentMask & red;
    green = ColorComponentMask & green;
    return (blue << 10) | (green << 5) | red;
}

void fill_screen(Color color)
{
    uint32 * screen_buffer_pointer = (uint32*) screen_buffer;
    uint32 color_value = (color << 16) | color;
    for(uint32 i = 0; i < 160; ++i)
    {
        REPEAT240(*screen_buffer_pointer++ = color_value;)
    }
}

typedef struct BLOCK { uint32 data[8]; } BLOCK;

void memcpy32(void *dst, const void *src, uint32 wdcount)
{
    uint32 blkN= wdcount/8, wdN= wdcount&7;
    uint32 *dstw= (uint32*)dst, *srcw= (uint32*)src;
    if(blkN)
    {
        // 8-word copies
        BLOCK *dst2= (BLOCK*)dst, *src2= (BLOCK*)src;
        while(blkN--)
            *dst2++ = *src2++;
        dstw= (uint32*)dst2;  srcw= (uint32*)src2;
    }
    // Residual words
    while(wdN--)
        *dstw++ = *srcw++;
}

void set_pixel(uint32 x, uint32 y, Color color)
{
    screen_buffer[x + y * screen_width] = color;
}

int lamadil_start()
{
    set_video_mode();
    start();
    while(1)
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
        while(*(vuint16*) reg_vcount < 50){};
        memcpy32((uint32*) mem_video_memory, (uint32 *) screen_buffer, screen_pixels/2);

        // Asi me aseguro que puedo hacer todo sin tearing.
    }
    return 0;
}