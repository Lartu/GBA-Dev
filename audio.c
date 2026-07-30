#include "lamadil.h"

#define REG_SOUNDCNT_L (*(volatile uint16_t *)0x04000080)
#define REG_SOUNDCNT_H (*(volatile uint16_t *)0x04000082)
#define REG_SOUNDCNT_X (*(volatile uint16_t *)0x04000084)
#define REG_FIFO_A (*(volatile uint32_t *)0x040000a0)

#define REG_DMA1SAD (*(volatile uint32_t *)0x040000bc)
#define REG_DMA1DAD (*(volatile uint32_t *)0x040000c0)
#define REG_DMA1CNT (*(volatile uint32_t *)0x040000c4)

#define REG_TM0CNT_L (*(volatile uint16_t *)0x04000100)
#define REG_TM0CNT_H (*(volatile uint16_t *)0x04000102)
#define REG_TM1CNT_L (*(volatile uint16_t *)0x04000104)
#define REG_TM1CNT_H (*(volatile uint16_t *)0x04000106)

#define REG_IE (*(volatile uint16_t *)0x04000200)
#define REG_IME (*(volatile uint16_t *)0x04000208)

#define IRQ_TIMER1 (1u << 4)
#define IRQ_DMA1 (1u << 9)

#define DMA_DEST_FIXED (2u << 21)
#define DMA_REPEAT (1u << 25)
#define DMA_32BIT (1u << 26)
#define DMA_START_FIFO (3u << 28)
#define DMA_IRQ (1u << 30)
#define DMA_ENABLE (1u << 31)

#define TIMER_CASCADE (1u << 2)
#define TIMER_IRQ (1u << 6)
#define TIMER_ENABLE (1u << 7)

#define AUDIO_CHUNK_BYTES 16u
#define FIFO_CAPACITY_BYTES 32u
#define CPU_FREQUENCY 16777216u

typedef void (*InterruptHandler)(void);

static const int8_t *audio_samples;
static uint32_t audio_sample_count;
static uint32_t audio_bytes_queued;
static uint32_t audio_dma_control;
static bool audio_loop;
static volatile bool audio_playing;

static void interrupt_noop(void)
{
}

static void audio_start_dma(void)
{
    REG_DMA1CNT = 0;
    REG_DMA1SAD = (uint32_t)(uintptr_t)audio_samples;
    REG_DMA1DAD = (uint32_t)(uintptr_t)&REG_FIFO_A;
    REG_DMA1CNT = audio_dma_control;
    audio_bytes_queued = 0;
}

static void audio_dma_interrupt(void)
{
    audio_bytes_queued += AUDIO_CHUNK_BYTES;
    if (audio_bytes_queued < audio_sample_count)
        return;

    if (audio_loop)
    {
        audio_start_dma();
        return;
    }

    /*
     * The final DMA block has reached FIFO A but has not finished playing.
     * Stop DMA now and let Timer 1 drain the (at most) 32-byte FIFO.
     */
    REG_DMA1CNT = 0;
    REG_TM1CNT_H = 0;
    REG_TM1CNT_L = (uint16_t)(0x10000u - FIFO_CAPACITY_BYTES);
    REG_TM1CNT_H = TIMER_CASCADE | TIMER_IRQ | TIMER_ENABLE;
}

static void audio_drain_interrupt(void)
{
    lamadil_audio_stop();
}

/*
 * The startup dispatcher indexes this table in hardware IRQ-bit order:
 * VBlank, HBlank, VCount, timers 0-3, serial, DMA 0-3, keypad, cartridge.
 */
InterruptHandler IntrTable[14] = {
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    audio_drain_interrupt,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    audio_dma_interrupt,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
    interrupt_noop,
};

bool lamadil_audio_play(const int8_t *samples, uint32_t sample_count,
                        uint32_t sample_rate, bool loop)
{
    uint32_t timer_period;

    if (samples == 0 || ((uintptr_t)samples & 3u) != 0u ||
        sample_count < AUDIO_CHUNK_BYTES ||
        (sample_count % AUDIO_CHUNK_BYTES) != 0u ||
        sample_rate < 2000u || sample_rate > 32768u)
        return false;

    lamadil_audio_stop();

    timer_period = CPU_FREQUENCY / sample_rate;
    audio_samples = samples;
    audio_sample_count = sample_count;
    audio_loop = loop;
    audio_playing = true;
    audio_dma_control = DMA_DEST_FIXED | DMA_REPEAT | DMA_32BIT |
                        DMA_START_FIFO | DMA_IRQ | DMA_ENABLE | 4u;

    REG_SOUNDCNT_X = 0x0080u;
    REG_SOUNDCNT_L = 0;
    /* FIFO A at full volume, routed left/right, Timer 0, FIFO reset. */
    REG_SOUNDCNT_H = 0x0b04u;

    REG_IME = 0;
    REG_IE |= IRQ_DMA1 | IRQ_TIMER1;
    REG_IME = 1;

    REG_TM0CNT_H = 0;
    REG_TM0CNT_L = (uint16_t)(0x10000u - timer_period);
    audio_start_dma();
    REG_TM0CNT_H = TIMER_ENABLE;
    return true;
}

void lamadil_audio_stop(void)
{
    REG_DMA1CNT = 0;
    REG_TM0CNT_H = 0;
    REG_TM1CNT_H = 0;
    REG_SOUNDCNT_H = 0x0800u;
    audio_playing = false;
}

bool lamadil_audio_is_playing(void)
{
    return audio_playing;
}
