#include "assets/assets.h"
#include "lamadil.h"

#define BEEP_SAMPLE_COUNT 2048u

static int32_t sprite_x = 88;
static int32_t sprite_y = 48;
static uint32_t animation_angle;
static int8_t beep_samples[BEEP_SAMPLE_COUNT] __attribute__((aligned(4)));

static void make_demo_tone(void)
{
    for (uint32_t i = 0; i < BEEP_SAMPLE_COUNT; ++i)
    {
        const uint32_t phase = i & 63u;
        beep_samples[i] = (int8_t)(phase < 32u
            ? -96 + (int32_t)phase * 6
            : 96 - (int32_t)(phase - 32u) * 6);
    }
}

void game_start(void)
{
    make_demo_tone();
}

void game_update(void)
{
    if (lamadil_key_down(LAMADIL_KEY_LEFT))
        --sprite_x;
    if (lamadil_key_down(LAMADIL_KEY_RIGHT))
        ++sprite_x;
    if (lamadil_key_down(LAMADIL_KEY_UP))
        --sprite_y;
    if (lamadil_key_down(LAMADIL_KEY_DOWN))
        ++sprite_y;

    if (lamadil_key_pressed(LAMADIL_KEY_A))
        lamadil_audio_play(beep_samples, BEEP_SAMPLE_COUNT, 8192u, false);
    if (lamadil_key_pressed(LAMADIL_KEY_B))
        lamadil_audio_play(beep_samples, BEEP_SAMPLE_COUNT, 8192u, true);
    if (lamadil_key_pressed(LAMADIL_KEY_START))
        lamadil_audio_stop();

    fill_screen(create_color(31, 20, 0));

    const int32_t bob = lam_cos(animation_angle) >> 8;
    draw_sprite(sprite_x, sprite_y + bob, demo_bitmapBitmap,
                DEMO_BITMAP_WIDTH, DEMO_BITMAP_HEIGHT);

    for (int32_t x = 0; x < LAMADIL_SCREEN_WIDTH; ++x)
    {
        const uint32_t offset =
            ((LAMADIL_PI * 3u / 4u) / LAMADIL_SCREEN_WIDTH) *
            (uint32_t)x;
        const int32_t wave_y =
            136 + (lam_cos(animation_angle + offset) >> 8);

        for (int32_t thickness = -3; thickness <= 3; ++thickness)
            set_pixel_clipped(x, wave_y + thickness,
                              create_color(0, 5, 30));
    }

    animation_angle += LAMADIL_PI / 16u;
}
