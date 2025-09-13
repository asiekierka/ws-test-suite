#include <stdarg.h>
#include <stdio.h>
#include <ws.h>
#include <ws/display.h>
#include <wsx/planar_unpack.h>
#include "resources.h"
#include "text.h"

void text_init(void) {
    wsx_planar_unpack(WS_TILE_MEM(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
}

void text_init_4bpp(void) {
    wsx_planar_unpack(WS_TILE_4BPP_MEM(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_4BPP_ZERO(1));
}

void text_puts(void __wf_iram *_dest, uint16_t tile, uint16_t x, uint16_t y, const char __far* text) {
    uint16_t __wf_iram *dest = (uint16_t __wf_iram*) _dest;
    dest += (y << 5); dest += x;
    while (*text != 0) {
        if (*text == 10) {
            dest = ((void __wf_iram*) ((((uintptr_t) dest) & ~63) + 64));
            text++;
        } else {
            *(dest++) = ((uint8_t) *(text++)) | tile;
        }
    }
}

__attribute__((format(printf, 5, 6)))
void text_printf(void __wf_iram *_dest, uint16_t tile, uint16_t x, uint16_t y, const char __far* format, ...) {
    uint16_t __wf_iram *dest = (uint16_t __wf_iram*) _dest;
    char buf[129];
    va_list val;
    va_start(val, format);
    vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    text_puts(dest, tile, x, y, buf);
}
