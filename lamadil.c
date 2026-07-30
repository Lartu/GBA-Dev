#include "lamadil.h"

#define REG_DISPCNT (*(volatile uint32_t *)0x04000000)
#define REG_VCOUNT (*(volatile uint16_t *)0x04000006)

#define REG_DMA3SAD (*(volatile uint32_t *)0x040000d4)
#define REG_DMA3DAD (*(volatile uint32_t *)0x040000d8)
#define REG_DMA3CNT (*(volatile uint32_t *)0x040000dc)

#define MEM_VRAM ((volatile Color *)0x06000000)

#define DMA_ENABLE (1u << 31)
#define DMA_32BIT (1u << 26)

extern const uint16_t lamadil_sine_lut[256];

#if defined(__arm__)
#define EWRAM_DATA __attribute__((section(".ewram"), aligned(4)))
#define IWRAM_CODE __attribute__((section(".iwram"), noinline))
#else
#define EWRAM_DATA __attribute__((aligned(4)))
#define IWRAM_CODE
#endif

Color screen_buffer[LAMADIL_SCREEN_PIXELS]
    EWRAM_DATA;

static IWRAM_CODE void copy_words(uint32_t *destination,
                                  const uint32_t *source,
                                  uint32_t word_count)
{
    while (word_count >= 8u)
    {
        destination[0] = source[0];
        destination[1] = source[1];
        destination[2] = source[2];
        destination[3] = source[3];
        destination[4] = source[4];
        destination[5] = source[5];
        destination[6] = source[6];
        destination[7] = source[7];
        destination += 8;
        source += 8;
        word_count -= 8u;
    }

    while (word_count-- != 0u)
        *destination++ = *source++;
}

void lamadil_init(void)
{
    /* Mode 3, BG2 enabled. */
    REG_DISPCNT = 0x0403u;
    fill_screen(0);
    lamadil_input_poll();
}

void lamadil_present(void)
{
    /*
     * Begin at scanline 50 and copy from the top down. This deliberately
     * follows the LCD beam instead of trying to fit a 75 KiB Mode 3 frame
     * into the much shorter VBlank interval.
     */
    while (REG_VCOUNT >= 50u)
    {
    }
    while (REG_VCOUNT < 50u)
    {
    }

    REG_DMA3CNT = 0;
    REG_DMA3SAD = (uint32_t)(uintptr_t)screen_buffer;
    REG_DMA3DAD = (uint32_t)(uintptr_t)MEM_VRAM;
    REG_DMA3CNT = DMA_ENABLE | DMA_32BIT | (LAMADIL_SCREEN_PIXELS / 2u);
}

Color create_color(uint32_t red, uint32_t green, uint32_t blue)
{
    red &= LAMADIL_COLOR_COMPONENT_MASK;
    green &= LAMADIL_COLOR_COMPONENT_MASK;
    blue &= LAMADIL_COLOR_COMPONENT_MASK;
    return (Color)((blue << 10) | (green << 5) | red);
}

IWRAM_CODE void fill_screen(Color color)
{
    const uint32_t pair = ((uint32_t)color << 16) | color;
    uint32_t *destination = (uint32_t *)screen_buffer;
    uint32_t count = LAMADIL_SCREEN_PIXELS / 2u;

    while (count >= 8u)
    {
        destination[0] = pair;
        destination[1] = pair;
        destination[2] = pair;
        destination[3] = pair;
        destination[4] = pair;
        destination[5] = pair;
        destination[6] = pair;
        destination[7] = pair;
        destination += 8;
        count -= 8u;
    }
}

void set_pixel(int32_t x, int32_t y, Color color)
{
    screen_buffer[(uint32_t)x +
                  (uint32_t)y * LAMADIL_SCREEN_WIDTH] = color;
}

bool set_pixel_clipped(int32_t x, int32_t y, Color color)
{
    if ((uint32_t)x >= LAMADIL_SCREEN_WIDTH ||
        (uint32_t)y >= LAMADIL_SCREEN_HEIGHT)
        return false;

    set_pixel(x, y, color);
    return true;
}

void draw_sprite(int32_t x, int32_t y, const Color *sprite,
                 uint32_t width, uint32_t height)
{
    uint32_t source_x = 0;
    uint32_t source_y = 0;
    uint32_t clipped_width = width;
    uint32_t clipped_height = height;

    if (sprite == 0 || width == 0u || height == 0u)
        return;

    if (x < 0)
    {
        source_x = (uint32_t)(-(int64_t)x);
        if (source_x >= width)
            return;
        clipped_width -= source_x;
        x = 0;
    }
    if (y < 0)
    {
        source_y = (uint32_t)(-(int64_t)y);
        if (source_y >= height)
            return;
        clipped_height -= source_y;
        y = 0;
    }
    if (x >= LAMADIL_SCREEN_WIDTH || y >= LAMADIL_SCREEN_HEIGHT)
        return;

    if (clipped_width > LAMADIL_SCREEN_WIDTH - (uint32_t)x)
        clipped_width = LAMADIL_SCREEN_WIDTH - (uint32_t)x;
    if (clipped_height > LAMADIL_SCREEN_HEIGHT - (uint32_t)y)
        clipped_height = LAMADIL_SCREEN_HEIGHT - (uint32_t)y;

    for (uint32_t row = 0; row < clipped_height; ++row)
    {
        Color *destination =
            screen_buffer + (uint32_t)x +
            ((uint32_t)y + row) * LAMADIL_SCREEN_WIDTH;
        const Color *source =
            sprite + source_x + (source_y + row) * width;
        uint32_t remaining = clipped_width;

        /* A halfword advances two equally aligned pointers to a word boundary. */
        if (((uintptr_t)destination & 3u) == 2u &&
            ((uintptr_t)source & 3u) == 2u)
        {
            *destination++ = *source++;
            --remaining;
        }

        if ((((uintptr_t)destination | (uintptr_t)source) & 3u) == 0u)
        {
            copy_words((uint32_t *)destination, (const uint32_t *)source,
                       remaining / 2u);
        }
        else
        {
            for (uint32_t column = 0; column < remaining; ++column)
                destination[column] = source[column];
            continue;
        }

        if ((remaining & 1u) != 0u)
            destination[remaining - 1u] = source[remaining - 1u];
    }
}

int32_t lam_sin(uint32_t angle)
{
    const uint32_t index = (angle >> 8) & 0xffu;
    const int32_t sign = (((angle >> 8) & 0x1ffu) > 255u) ? -1 : 1;
    return (int32_t)lamadil_sine_lut[index] * sign;
}

int32_t lam_cos(uint32_t angle)
{
    return lam_sin(angle + (LAMADIL_PI / 2u));
}

int lamadil_start(void)
{
    lamadil_init();
    game_start();

    for (;;)
    {
        lamadil_input_poll();
        game_update();
        lamadil_present();
    }
}
