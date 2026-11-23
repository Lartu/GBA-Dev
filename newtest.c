#include "assets/assets.h"
#include "lamadil.h"
#include "soundbank.h"
#include "soundbank_bin.h"

static int32_t sprite_x = 88;
static int32_t sprite_y = 48;
static uint32_t animation_angle;
static bool song_paused;

void game_start(void)
{
    lamadil_audio_init(soundbank_bin);
    lamadil_song_play(MOD_FLATOUTLIES, true);
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
        lamadil_sound_play(SFX_BOOM);
    if (lamadil_key_pressed(LAMADIL_KEY_B))
        lamadil_sound_play(SFX_AMBULANCE);
    if (lamadil_key_pressed(LAMADIL_KEY_START))
    {
        if (song_paused)
            lamadil_song_resume();
        else
            lamadil_song_pause();
        song_paused = !song_paused;
    }
    if (lamadil_key_pressed(LAMADIL_KEY_SELECT))
    {
        lamadil_song_play(MOD_FLATOUTLIES, true);
        song_paused = false;
    }

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
            (LAMADIL_SCREEN_HEIGHT - 24) +
            (lam_cos(animation_angle + offset) >> 8);

        for (int32_t thickness = -3; thickness <= 3; ++thickness)
            set_pixel_clipped(x, wave_y + thickness,
                              create_color(0, 5, 30));
    }

    animation_angle += LAMADIL_PI / 8u;
}
