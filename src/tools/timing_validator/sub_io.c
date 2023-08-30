#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include "key.h"
#include "main.h"

/* I/O SUBSYSTEM */

#define PORT_PAGES 7
#define PORT_BYTE 0x0000
#define PORT_WORD 0x8000
#define PORT_READ_ONLY 0x4000
static uint16_t port_values[16];
static const uint16_t __wf_rom port_page_values[] = {
    0x00 | PORT_BYTE,
    0x01 | PORT_BYTE,
    0x02 | PORT_BYTE,
    0x03 | PORT_BYTE,
    0x04 | PORT_BYTE,
    0x05 | PORT_BYTE,
    0x06 | PORT_BYTE,
    0x07 | PORT_BYTE,
    0x08 | PORT_BYTE,
    0x09 | PORT_BYTE,
    0x0A | PORT_BYTE,
    0x0B | PORT_BYTE,
    0x0C | PORT_BYTE,
    0x0D | PORT_BYTE,
    0x0E | PORT_BYTE,
    0x0F | PORT_BYTE,

    0x10 | PORT_BYTE,
    0x11 | PORT_BYTE,
    0x12 | PORT_BYTE,
    0x13 | PORT_BYTE,
    0x14 | PORT_BYTE,
    0x15 | PORT_BYTE,
    0x16 | PORT_BYTE,
    0x17 | PORT_BYTE,
    0x18 | PORT_BYTE,
    0x19 | PORT_BYTE,
    0x1A | PORT_BYTE,
    0x1B | PORT_BYTE,
    0x1C | PORT_BYTE,
    0x1D | PORT_BYTE,
    0x1E | PORT_BYTE,
    0x1F | PORT_BYTE,

    0x20 | PORT_WORD,
    0x22 | PORT_WORD,
    0x24 | PORT_WORD,
    0x26 | PORT_WORD,
    0x28 | PORT_WORD,
    0x2A | PORT_WORD,
    0x2C | PORT_WORD,
    0x2E | PORT_WORD,
    0x30 | PORT_WORD,
    0x32 | PORT_WORD,
    0x34 | PORT_WORD,
    0x36 | PORT_WORD,
    0x38 | PORT_WORD,
    0x3A | PORT_WORD,
    0x3C | PORT_WORD,
    0x3E | PORT_WORD,

    0x40 | PORT_WORD,
    0x42 | PORT_WORD,
    0x44 | PORT_WORD,
    0x46 | PORT_WORD,
    0x48 | PORT_BYTE,
    0x4A | PORT_WORD,
    0x4C | PORT_WORD,
    0x4E | PORT_WORD,
    0x50 | PORT_WORD,
    0x52 | PORT_BYTE,
    0xFFFF,
    0xFFFF,
    0xA0 | PORT_BYTE,
    0x60 | PORT_BYTE,
    0x62 | PORT_BYTE,
    0xAC | PORT_BYTE,

    0x64 | PORT_WORD,
    0x66 | PORT_WORD,
    0x68 | PORT_BYTE,
    0x69 | PORT_BYTE,
    0x6A | PORT_WORD,
    0x96 | PORT_WORD,
    0x98 | PORT_WORD,
    0x9A | PORT_WORD,
    0x9E | PORT_BYTE,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0x70 | PORT_WORD,
    0x72 | PORT_WORD,
    0x74 | PORT_WORD,
    0x76 | PORT_WORD,

    0x80 | PORT_WORD,
    0x88 | PORT_BYTE,
    0x82 | PORT_WORD,
    0x89 | PORT_BYTE,
    0x84 | PORT_WORD,
    0x8A | PORT_BYTE,
    0x86 | PORT_WORD,
    0x8B | PORT_BYTE,
    0x8C | PORT_BYTE,
    0x8D | PORT_BYTE,
    0x8E | PORT_BYTE,
    0x8F | PORT_BYTE,
    0x90 | PORT_BYTE,
    0x91 | PORT_BYTE,
    0x92 | PORT_WORD,
    0x94 | PORT_BYTE,

    0xA2 | PORT_WORD,
    0xA4 | PORT_WORD,
    0xA6 | PORT_WORD,
    0xA8 | PORT_WORD,
    0xAA | PORT_WORD,
    0xB0 | PORT_BYTE,
    0xB2 | PORT_BYTE,
    0xB4 | PORT_BYTE,
    0xB6 | PORT_BYTE,
    0xB7 | PORT_BYTE,
    0xB1 | PORT_BYTE,
    0xB3 | PORT_BYTE,
    0xB5 | PORT_BYTE,
    0xBA | PORT_WORD | PORT_READ_ONLY,
    0xBC | PORT_WORD | PORT_READ_ONLY,
    0xBE | PORT_WORD | PORT_READ_ONLY
};
static bool port_reload = true;

void subsystem_io(void) {
    static int8_t x = 0;
    static uint16_t y = 0;

    if (subsystem_redraw) {
        uint16_t page = (y & 0xFFF0);
        for (uint8_t iy = 0; iy < 16; iy++) {
            uint16_t port = port_page_values[page + iy];
            if (port == 0xFFFF) {
                ws_screen_fill_tiles(SCREEN_1, 0x20, 1, 1 + iy, 7, 1);
                continue;
            } else {
                ws_screen_fill_tiles(SCREEN_1, 0x20, 6, 1 + iy, 2, 1);
            }

            draw_hex(port, 2, false, 0, 1, 1+iy);
            if (port_reload) {
                if (port & PORT_READ_ONLY) {
                    port_values[iy] = 0;
                } else if (port & PORT_WORD) {
                    port_values[iy] = inportw(port & 0xFF);
                } else {
                    port_values[iy] = inportb(port & 0xFF);
                }
            }
            draw_hex(port_values[iy], port & PORT_WORD ? 4 : 2, (y & 0x0F) == iy, x, 4, 1+iy);
        }
        subsystem_redraw = false;
        port_reload = false;
    }

    if (keys_pressed != 0) {
        subsystem_redraw = true;

        if (keys_pressed & KEY_X1) {
            do {
                y = (y & 0xFFF0) | ((y - 1) & 0xF);
            } while (port_page_values[y] == 0xFFFF);
        }        
        if (keys_pressed & KEY_X3) {
            do {
                y = (y & 0xFFF0) | ((y + 1) & 0xF);
            } while (port_page_values[y] == 0xFFFF);
        }
        if (keys_pressed & KEY_Y4) {
            if (y >= 16) {
                y -= 16;
                port_reload = true;
            }
        }
        if (keys_pressed & KEY_Y2) {
            if (y < (PORT_PAGES - 1) * 16) {
                y += 16;
                port_reload = true;
            }
        }
        if (keys_pressed & KEY_X4) {
            if (x > 0) {
                x--;
            }
        }
        if (keys_pressed & KEY_X2) {
            x++;
        }

        int8_t max_x = (port_page_values[y] & PORT_WORD ? 3 : 1);
        if (x > max_x) x = max_x;
        if (!port_reload) {
            uint8_t shift = (max_x - x) << 2;
            uint8_t val = port_values[y & 0xF] >> shift;
            if (keys_pressed & KEY_Y1) {
                val++;
            }
            if (keys_pressed & KEY_Y3) {
                val--;
            }
            port_values[y & 0xF] = (port_values[y & 0xF] & ~(0xF << shift)) | ((val & 0xF) << shift);
            if (keys_pressed & KEY_A) {
                if (port_page_values[y] & PORT_WORD) {
                    outportw(port_page_values[y] & 0xFF, port_values[y & 0xF]);
                } else {
                    outportb(port_page_values[y] & 0xFF, port_values[y & 0xF]);
                }
            }
        }
        if (keys_pressed & KEY_B) {
            port_reload = true;
        }
    }
}
