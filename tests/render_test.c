#include "lamadil.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void lamadil_input_poll(void)
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
    for (size_t i = 0; i < LAMADIL_SCREEN_PIXELS; ++i)
        assert(screen_buffer[i] == expected);
}

int main(void)
{
    static const Color sprite[] __attribute__((aligned(4))) = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
    };

    fill_screen(0x1234);
    expect_all(0x1234);

    fill_screen(0);
    draw_sprite(-1, -1, sprite, 4, 3);
    assert(screen_buffer[0] == 6);
    assert(screen_buffer[1] == 7);
    assert(screen_buffer[2] == 8);
    assert(screen_buffer[LAMADIL_SCREEN_WIDTH] == 10);
    assert(screen_buffer[LAMADIL_SCREEN_WIDTH + 1] == 11);
    assert(screen_buffer[LAMADIL_SCREEN_WIDTH + 2] == 12);

    fill_screen(0);
    draw_sprite(LAMADIL_SCREEN_WIDTH - 1, LAMADIL_SCREEN_HEIGHT - 1,
                sprite, 4, 3);
    assert(screen_buffer[LAMADIL_SCREEN_PIXELS - 1] == 1);

    fill_screen(0x55aa);
    draw_sprite(INT32_MIN, INT32_MIN, sprite, 4, 3);
    expect_all(0x55aa);

    fill_screen(0);
    draw_sprite(1, 1, sprite, 3, 2);
    assert(screen_buffer[1 + LAMADIL_SCREEN_WIDTH] == 1);
    assert(screen_buffer[2 + LAMADIL_SCREEN_WIDTH] == 2);
    assert(screen_buffer[3 + LAMADIL_SCREEN_WIDTH] == 3);
    assert(screen_buffer[1 + 2 * LAMADIL_SCREEN_WIDTH] == 4);
    assert(screen_buffer[2 + 2 * LAMADIL_SCREEN_WIDTH] == 5);
    assert(screen_buffer[3 + 2 * LAMADIL_SCREEN_WIDTH] == 6);

    assert(!set_pixel_clipped(-1, 0, 1));
    assert(!set_pixel_clipped(0, LAMADIL_SCREEN_HEIGHT, 1));
    assert(set_pixel_clipped(10, 10, 0x7fff));
    assert(screen_buffer[10 + 10 * LAMADIL_SCREEN_WIDTH] == 0x7fff);

    puts("renderer tests passed");
    return 0;
}
