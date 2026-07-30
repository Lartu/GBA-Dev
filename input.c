#include "lamadil.h"

#define REG_KEYINPUT (*(volatile uint16_t *)0x04000130)
#define KEY_MASK 0x03ffu

static uint16_t current_keys;
static uint16_t previous_keys;

void lamadil_input_poll(void)
{
    previous_keys = current_keys;
    current_keys = (uint16_t)(~REG_KEYINPUT) & KEY_MASK;
}

bool lamadil_key_down(LamadilKey key)
{
    return (current_keys & (uint16_t)key) != 0u;
}

bool lamadil_key_pressed(LamadilKey key)
{
    const uint16_t mask = (uint16_t)key;
    return (current_keys & mask) != 0u && (previous_keys & mask) == 0u;
}

bool lamadil_key_released(LamadilKey key)
{
    const uint16_t mask = (uint16_t)key;
    return (current_keys & mask) == 0u && (previous_keys & mask) != 0u;
}
