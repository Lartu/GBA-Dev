#include "lamadil.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void lamadil_input_poll(void)
{
}

void lamadil_audio_update(void)
{
}

void game_start(void)
{
}

void game_update(void)
{
}

static void expect_all(Color expected)
{
    for (int32_t y = 0; y < LAMADIL_SCREEN_HEIGHT; ++y)
        for (int32_t x = 0; x < LAMADIL_SCREEN_WIDTH; ++x)
            assert(lamadil_get_pixel(x, y) == expected);
}

static Color mode_color(Color color)
{
#if LAMADIL_VIDEO_MODE == 4
    const uint32_t red3 = (color & 0x1fu) >> 2;
    const uint32_t green3 = ((color >> 5) & 0x1fu) >> 2;
    const uint32_t blue2 = ((color >> 10) & 0x1fu) >> 3;
    const uint32_t red5 = (red3 << 2) | (red3 >> 1);
    const uint32_t green5 = (green3 << 2) | (green3 >> 1);
    const uint32_t blue5 = (blue2 << 3) | (blue2 << 1) | (blue2 >> 1);
    return (Color)((blue5 << 10) | (green5 << 5) | red5);
#else
    return color;
#endif
}

int main(void)
{
    static const Color sprite[] __attribute__((aligned(4))) = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
    };

    fill_screen(0x1234);
    expect_all(mode_color(0x1234));

    fill_screen(0);
    draw_sprite(-1, -1, sprite, 4, 3);
    assert(lamadil_get_pixel(0, 0) == mode_color(6));
    assert(lamadil_get_pixel(1, 0) == mode_color(7));
    assert(lamadil_get_pixel(2, 0) == mode_color(8));
    assert(lamadil_get_pixel(0, 1) == mode_color(10));
    assert(lamadil_get_pixel(1, 1) == mode_color(11));
    assert(lamadil_get_pixel(2, 1) == mode_color(12));

    fill_screen(0);
    draw_sprite(LAMADIL_SCREEN_WIDTH - 1, LAMADIL_SCREEN_HEIGHT - 1,
                sprite, 4, 3);
    assert(lamadil_get_pixel(LAMADIL_SCREEN_WIDTH - 1,
                             LAMADIL_SCREEN_HEIGHT - 1) == mode_color(1));

    fill_screen(0x55aa);
    draw_sprite(INT32_MIN, INT32_MIN, sprite, 4, 3);
    expect_all(mode_color(0x55aa));

    fill_screen(0);
    draw_sprite(1, 1, sprite, 3, 2);
    assert(lamadil_get_pixel(1, 1) == mode_color(1));
    assert(lamadil_get_pixel(2, 1) == mode_color(2));
    assert(lamadil_get_pixel(3, 1) == mode_color(3));
    assert(lamadil_get_pixel(1, 2) == mode_color(4));
    assert(lamadil_get_pixel(2, 2) == mode_color(5));
    assert(lamadil_get_pixel(3, 2) == mode_color(6));

    assert(!set_pixel_clipped(-1, 0, 1));
    assert(!set_pixel_clipped(0, LAMADIL_SCREEN_HEIGHT, 1));
    assert(set_pixel_clipped(10, 10, 0x7fff));
    assert(lamadil_get_pixel(10, 10) == mode_color(0x7fff));

    puts("renderer tests passed");
    return 0;
}
