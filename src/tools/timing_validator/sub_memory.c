#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include "key.h"
#include "main.h"

/* MEMORY SUBSYSTEM */

void subsystem_memory(void) {
    static uint16_t offset = 0;
    static uint8_t selected_byte = 0;

    if (subsystem_redraw) {
        uint8_t b = 0;
        draw_hex(offset, 4, false, 0, 24, 17);
        ws_screen_fill_tiles(SCREEN_1, (uint8_t) ':', 3, 1, 1, 16);
        for (uint8_t y = 1; y < 17; y++) {
            draw_hex(offset + b, 2, false, 0, 1, y);
            for (uint8_t x = 4; x <= 25; x += 3, b++) {
                draw_hex(*((uint16_t __wf_iram*) (offset + b)), 2, true, selected_byte - (b * 2), x, y);
            }
        }
        subsystem_redraw = false;
    }

    if (keys_pressed != 0) {
        uint8_t __wf_iram *ptr = (uint8_t __wf_iram*) (offset + (selected_byte >> 1));
        uint8_t val = (selected_byte & 1) ? (*ptr) : ((*ptr) >> 4);
        bool write_val = false;
        
        subsystem_redraw = true;

        if (keys_pressed & KEY_X1) {
            selected_byte -= 0x10;
        }        
        if (keys_pressed & KEY_X3) {
            selected_byte += 0x10;
        }
        if (keys_pressed & KEY_X4) {
            selected_byte = ((selected_byte - 1) & 0x0F) | (selected_byte & 0xF0);
        }
        if (keys_pressed & KEY_X2) {
            selected_byte = ((selected_byte + 1) & 0x0F) | (selected_byte & 0xF0);
        }
        if (keys_pressed & KEY_Y1) {
            val++;
            write_val = true;
        }
        if (keys_pressed & KEY_Y3) {
            val--;
            write_val = true;
        }
        if (keys_pressed & KEY_A) {
            offset += 0x0080;
        }
        if (keys_pressed & KEY_B) {
            offset -= 0x0080;
        }
        if (!ws_system_color_active()) {
            offset &= 0x3FFF;
        }
        if (write_val) {
            if (selected_byte & 1) {
                *ptr = (*ptr & 0xF0) | (val & 0x0F);
            } else {
                *ptr = (*ptr & 0x0F) | (val << 4);
            }
        }
    }
}
