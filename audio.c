#include "lamadil.h"

#include <maxmod.h>

#define REG_DISPSTAT (*(volatile uint16_t *)0x04000004)
#define REG_IE (*(volatile uint16_t *)0x04000200)
#define REG_IME (*(volatile uint16_t *)0x04000208)

#define IRQ_VBLANK (1u << 0)
#define DISPSTAT_VBLANK_IRQ (1u << 3)

#define AUDIO_CHANNELS 8u

typedef void (*InterruptHandler)(void);

/*
 * mmInitDefault() obtains this storage through malloc. Lamadil is
 * freestanding, so provide the same buffers statically and call mmInit()
 * directly.
 */
static uint8_t module_channels[MM_SIZEOF_MODCH * AUDIO_CHANNELS]
    __attribute__((aligned(4)));
static uint8_t active_channels[MM_SIZEOF_ACTCH * AUDIO_CHANNELS]
    __attribute__((aligned(4)));
static uint8_t mixing_channels[MM_SIZEOF_MIXCH * AUDIO_CHANNELS]
    __attribute__((aligned(4)));
static uint8_t mixing_memory[MM_MIXLEN_16KHZ]
    __attribute__((aligned(4)));
static uint8_t wave_memory[MM_MIXLEN_16KHZ]
    __attribute__((aligned(4)));

static bool audio_initialized;

static void interrupt_noop(void)
{
}

/*
 * The startup dispatcher indexes this table in hardware IRQ-bit order:
 * VBlank, HBlank, VCount, timers 0-3, serial, DMA 0-3, keypad, cartridge.
 */
InterruptHandler IntrTable[14] = {
    mmVBlank,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
};

void lamadil_audio_init(const void *soundbank)
{
    mm_gba_system system;

    if (soundbank == 0 || audio_initialized)
        return;

    system.mixing_mode = MM_MIX_16KHZ;
    system.mod_channel_count = AUDIO_CHANNELS;
    system.mix_channel_count = AUDIO_CHANNELS;
    system.module_channels = module_channels;
    system.active_channels = active_channels;
    system.mixing_channels = mixing_channels;
    system.mixing_memory = mixing_memory;
    system.wave_memory = wave_memory;
    system.soundbank = (mm_addr)soundbank;
    mmInit(&system);

    REG_DISPSTAT |= DISPSTAT_VBLANK_IRQ;
    REG_IE |= IRQ_VBLANK;
    REG_IME = 1u;
    audio_initialized = true;
}

void lamadil_audio_update(void)
{
    if (audio_initialized)
        mmFrame();
}

void lamadil_song_play(uint32_t song_id, bool loop)
{
    if (audio_initialized)
        mmStart(song_id, loop ? MM_PLAY_LOOP : MM_PLAY_ONCE);
}

void lamadil_song_stop(void)
{
    if (audio_initialized)
        mmStop();
}

void lamadil_song_pause(void)
{
    if (audio_initialized)
        mmPause();
}

void lamadil_song_resume(void)
{
    if (audio_initialized)
        mmResume();
}

bool lamadil_song_is_playing(void)
{
    return audio_initialized && mmActive() != 0;
}

void lamadil_song_set_volume(uint32_t volume)
{
    if (volume > 1024u)
        volume = 1024u;
    if (audio_initialized)
        mmSetModuleVolume(volume);
}

LamadilSoundHandle lamadil_sound_play(uint32_t sound_id)
{
    if (!audio_initialized)
        return 0u;
    return mmEffect(sound_id);
}

void lamadil_sound_stop(LamadilSoundHandle handle)
{
    if (audio_initialized && handle != 0u)
        mmEffectCancel(handle);
}

void lamadil_sound_stop_all(void)
{
    if (audio_initialized)
        mmEffectCancelAll();
}
