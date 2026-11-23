#include "lamadil.h"

#define REG_DISPCNT (*(volatile uint32_t *)0x04000000)
#define REG_VCOUNT (*(volatile uint16_t *)0x04000006)

#define REG_DMA3SAD (*(volatile uint32_t *)0x040000d4)
#define REG_DMA3DAD (*(volatile uint32_t *)0x040000d8)
#define REG_DMA3CNT (*(volatile uint32_t *)0x040000dc)

#define MEM_VRAM ((volatile uint8_t *)0x06000000)
#define MEM_PALETTE ((volatile Color *)0x05000000)

#define VIDEO_PAGE_BYTES 0x0000a000u
#define DISPLAY_BG2 (1u << 10)
#define DISPLAY_PAGE (1u << 4)
#define DISPLAY_FORCED_BLANK (1u << 7)

#define DMA_ENABLE (1u << 31)
#define DMA_32BIT (1u << 26)
#define DMA_SOURCE_FIXED (2u << 23)

extern const uint16_t lamadil_sine_lut[256];

#if defined(__arm__)
#define IWRAM_CODE __attribute__((section(".iwram"), noinline))
#else
#define IWRAM_CODE
#if LAMADIL_VIDEO_MODE == 4
static uint8_t host_pages[2][LAMADIL_SCREEN_PIXELS];
#else
static Color host_pages[2][LAMADIL_SCREEN_PIXELS];
#endif
#endif

static bool rendering_enabled = true;
static uint32_t displayed_page;
static uint32_t drawing_page = 1u;

static uint32_t display_control(void)
{
    return DISPLAY_BG2 | LAMADIL_VIDEO_MODE |
           (displayed_page != 0u ? DISPLAY_PAGE : 0u);
}

#if LAMADIL_VIDEO_MODE == 4
static uint8_t color_to_palette_index(Color color)
{
    const uint8_t red = (uint8_t)(color & 0x1fu);
    const uint8_t green = (uint8_t)((color >> 5) & 0x1fu);
    const uint8_t blue = (uint8_t)((color >> 10) & 0x1fu);
    return (uint8_t)((red >> 2) | ((green >> 2) << 3) |
                     ((blue >> 3) << 6));
}

static Color palette_index_to_color(uint8_t index)
{
    const uint32_t red3 = index & 7u;
    const uint32_t green3 = (index >> 3) & 7u;
    const uint32_t blue2 = (index >> 6) & 3u;
    const uint32_t red5 = (red3 << 2) | (red3 >> 1);
    const uint32_t green5 = (green3 << 2) | (green3 >> 1);
    const uint32_t blue5 = (blue2 << 3) | (blue2 << 1) | (blue2 >> 1);
    return (Color)((blue5 << 10) | (green5 << 5) | red5);
}

static void initialize_palette(void)
{
#if defined(__arm__)
    for (uint32_t index = 0; index < 256u; ++index)
        MEM_PALETTE[index] = palette_index_to_color((uint8_t)index);
#endif
}

static volatile uint8_t *drawing_buffer(void)
{
#if defined(__arm__)
    return MEM_VRAM + drawing_page * VIDEO_PAGE_BYTES;
#else
    return host_pages[drawing_page];
#endif
}
#else
static volatile Color *drawing_buffer(void)
{
#if defined(__arm__)
    return (volatile Color *)(MEM_VRAM + drawing_page * VIDEO_PAGE_BYTES);
#else
    return host_pages[drawing_page];
#endif
}
#endif

static void fill_drawing_page(Color color)
{
#if LAMADIL_VIDEO_MODE == 4
    const uint32_t palette_value = color_to_palette_index(color);
    volatile uint8_t *destination = drawing_buffer();
#else
    volatile Color *destination = drawing_buffer();
#endif

#if defined(__arm__)
#if LAMADIL_VIDEO_MODE == 4
    volatile uint32_t pair =
        palette_value | (palette_value << 8) |
        (palette_value << 16) | (palette_value << 24);
#else
    volatile uint32_t pair = (uint32_t)color | ((uint32_t)color << 16);
#endif
    REG_DMA3CNT = 0;
    REG_DMA3SAD = (uint32_t)(uintptr_t)&pair;
    REG_DMA3DAD = (uint32_t)(uintptr_t)destination;
    REG_DMA3CNT =
        DMA_ENABLE | DMA_32BIT | DMA_SOURCE_FIXED |
        (LAMADIL_SCREEN_PIXELS * sizeof(*destination) / 4u);
#else
    for (uint32_t offset = 0; offset < LAMADIL_SCREEN_PIXELS; ++offset)
    {
#if LAMADIL_VIDEO_MODE == 4
        destination[offset] = (uint8_t)palette_value;
#else
        destination[offset] = color;
#endif
    }
#endif
}

void lamadil_init(void)
{
    displayed_page = 0u;
    drawing_page = 1u;
    rendering_enabled = true;

#if LAMADIL_VIDEO_MODE == 4
    initialize_palette();
#endif

    REG_DISPCNT = display_control();
    fill_drawing_page(0);
    lamadil_present();
    fill_drawing_page(0);
    lamadil_input_poll();
}

void lamadil_present(void)
{
    displayed_page = drawing_page;
    drawing_page ^= 1u;
    if (rendering_enabled)
        REG_DISPCNT = display_control();
}

void lamadil_set_rendering_enabled(bool enabled)
{
    rendering_enabled = enabled;
    REG_DISPCNT = enabled ? display_control() : DISPLAY_FORCED_BLANK;
}

static void lamadil_wait_for_vblank(void)
{
    while (REG_VCOUNT >= 160u)
    {
    }
    while (REG_VCOUNT < 160u)
    {
    }
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
    fill_drawing_page(color);
}

void set_pixel(int32_t x, int32_t y, Color color)
{
    const uint32_t offset =
        (uint32_t)x + (uint32_t)y * LAMADIL_SCREEN_WIDTH;

#if LAMADIL_VIDEO_MODE == 4
    const uint8_t index = color_to_palette_index(color);
#if defined(__arm__)
    volatile uint16_t *word =
        (volatile uint16_t *)(drawing_buffer() + (offset & ~1u));
    uint16_t pixels = *word;
    if ((offset & 1u) != 0u)
        pixels = (uint16_t)((pixels & 0x00ffu) | ((uint16_t)index << 8));
    else
        pixels = (uint16_t)((pixels & 0xff00u) | index);
    *word = pixels;
#else
    drawing_buffer()[offset] = index;
#endif
#else
    drawing_buffer()[offset] = color;
#endif
}

bool set_pixel_clipped(int32_t x, int32_t y, Color color)
{
    if ((uint32_t)x >= LAMADIL_SCREEN_WIDTH ||
        (uint32_t)y >= LAMADIL_SCREEN_HEIGHT)
        return false;

    set_pixel(x, y, color);
    return true;
}

Color lamadil_get_pixel(int32_t x, int32_t y)
{
    if ((uint32_t)x >= LAMADIL_SCREEN_WIDTH ||
        (uint32_t)y >= LAMADIL_SCREEN_HEIGHT)
        return 0;

    const uint32_t offset =
        (uint32_t)x + (uint32_t)y * LAMADIL_SCREEN_WIDTH;
#if LAMADIL_VIDEO_MODE == 4
    return palette_index_to_color(drawing_buffer()[offset]);
#else
    return drawing_buffer()[offset];
#endif
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
        const Color *source =
            sprite + source_x + (source_y + row) * width;
        const int32_t destination_y = y + (int32_t)row;
        for (uint32_t column = 0; column < clipped_width; ++column)
            set_pixel(x + (int32_t)column, destination_y, source[column]);
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
    bool draw_phase = true;

    lamadil_init();
    game_start();

    for (;;)
    {
        lamadil_wait_for_vblank();
        lamadil_audio_update();

        if (!rendering_enabled || draw_phase)
        {
            lamadil_input_poll();
            game_update();
        }
        else
        {
            lamadil_present();
        }

        if (rendering_enabled)
            draw_phase = !draw_phase;
    }
}
