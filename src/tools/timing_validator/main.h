#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdint.h>
#include <wonderful.h>
#include <ws.h>

__attribute__((section(".iramx_1800")))
extern uint16_t SCREEN_1[32 * 32];

extern const char __wf_rom hex_to_chr[];

#define HIGHLIGHT(cond) ((selected && (cond)) ? SCR_ENTRY_PALETTE(1) : SCR_ENTRY_PALETTE(0))

static inline void draw_hex(uint16_t value, uint8_t digits, bool selected, int selected_part, int x, int y) {
    uint8_t max = (digits - 1) << 2;
    for (uint8_t i = 0; i < digits; i++) {
        uint8_t h = (value >> (max - (i << 2))) & 0xF;
        ws_screen_put_tile(SCREEN_1, ((uint8_t) hex_to_chr[h]) | HIGHLIGHT(selected_part == i), x+i, y);
    }
}

extern bool subsystem_redraw;

/**
 * @brief Modify hexadecimal value per-nibble.
 * 
 * @param v 16-bit hex value.
 * @param shift Shift for 4-bit changed value.
 * @param n Delta for 4-bit changed value.
 * @return uint16_t New 16-bit hex value.
 */
uint16_t modify_hex4(uint16_t v, int shift, int n);

#endif /* __MAIN_H__ */
