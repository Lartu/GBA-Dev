#ifndef LAMADIL_H
#define LAMADIL_H

#include <stdbool.h>
#include <stdint.h>

#define LAMADIL_SCREEN_WIDTH 240
#define LAMADIL_SCREEN_HEIGHT 160
#define LAMADIL_SCREEN_PIXELS \
    (LAMADIL_SCREEN_WIDTH * LAMADIL_SCREEN_HEIGHT)

#define LAMADIL_PI 65536u
#define LAMADIL_COLOR_COMPONENT_MASK 0x1fu

typedef uint16_t Color;

typedef enum LamadilKey
{
    LAMADIL_KEY_A = 1u << 0,
    LAMADIL_KEY_B = 1u << 1,
    LAMADIL_KEY_SELECT = 1u << 2,
    LAMADIL_KEY_START = 1u << 3,
    LAMADIL_KEY_RIGHT = 1u << 4,
    LAMADIL_KEY_LEFT = 1u << 5,
    LAMADIL_KEY_UP = 1u << 6,
    LAMADIL_KEY_DOWN = 1u << 7,
    LAMADIL_KEY_R = 1u << 8,
    LAMADIL_KEY_L = 1u << 9
} LamadilKey;

extern Color screen_buffer[LAMADIL_SCREEN_PIXELS];

void lamadil_init(void);
void lamadil_present(void);

Color create_color(uint32_t red, uint32_t green, uint32_t blue);
void fill_screen(Color color);
void set_pixel(int32_t x, int32_t y, Color color);
bool set_pixel_clipped(int32_t x, int32_t y, Color color);
void draw_sprite(int32_t x, int32_t y, const Color *sprite,
                 uint32_t width, uint32_t height);

int32_t lam_sin(uint32_t angle);
int32_t lam_cos(uint32_t angle);

void lamadil_input_poll(void);
bool lamadil_key_down(LamadilKey key);
bool lamadil_key_pressed(LamadilKey key);
bool lamadil_key_released(LamadilKey key);

/*
 * Plays signed 8-bit mono PCM through Direct Sound FIFO A.
 * sample_count must be a multiple of 16. The data must remain alive and
 * word-aligned until playback ends.
 */
bool lamadil_audio_play(const int8_t *samples, uint32_t sample_count,
                        uint32_t sample_rate, bool loop);
void lamadil_audio_stop(void);
bool lamadil_audio_is_playing(void);

/* Implement these two callbacks in the game. */
void game_start(void);
void game_update(void);

#endif
