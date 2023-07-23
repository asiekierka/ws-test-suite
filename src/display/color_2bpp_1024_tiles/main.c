#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <wsx/planar_unpack.h>
#include "font_ascii_bin.h"
#include "resources.h"

#define SCREEN_1 ((uint8_t __wf_iram*) 0x1800)

static const uint16_t __wf_rom header_text[] = {
    'c', 'o', 'l', 'o', 'r', '_', '2',
    'b', 'p', 'p', '_', '1', '0', '2',
    '4', '_', 't', 'i', 'l', 'e', 's',
    ' ', 't', 'e', 's', 't'
};

static const uint16_t __wf_rom ws_mono_text[] = {
    'W', 'S', 'C', ' ', 'o', 'n', 'l', 'y'
};

int main(void) {
    // Load ASCII font, prepare test header.
    wsx_planar_unpack(MEM_TILE(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    ws_screen_put_tiles(SCREEN_1, header_text, 1, 17, 26, 1);

    if (ws_mode_set(WS_MODE_COLOR)) {
        // In 2BPP color mode, 0x2000~0x5FFF is used to map 1024 tiles.
        // Some older emulators get this wrong and ignore the additional
        // WSC-specific bit, mapping only the first 512 tiles.
        // Write text (tiles 512 + (508, 509, 510, 511))
        for (uint8_t i = 0; i < 4; i++) {
            ws_screen_put_tile(SCREEN_1, SCR_ENTRY_BANK(1) | SCR_ENTRY_PALETTE(0) | 508 | i, 12 + i, 8);
        }

        // Unpack tile data (PASS/FAIL)
        wsx_planar_unpack(MEM_TILE(508), 8, font_ascii + ('F' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 2));
        wsx_planar_unpack(MEM_TILE(509), 8, font_ascii + ('A' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 2));
        wsx_planar_unpack(MEM_TILE(510), 8, font_ascii + ('I' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 2));
        wsx_planar_unpack(MEM_TILE(511), 8, font_ascii + ('L' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 2));

        wsx_planar_unpack(MEM_TILE(512+508), 8, font_ascii + ('P' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 3));
        wsx_planar_unpack(MEM_TILE(512+509), 8, font_ascii + ('A' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 3));
        wsx_planar_unpack(MEM_TILE(512+510), 8, font_ascii + ('S' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 3));
        wsx_planar_unpack(MEM_TILE(512+511), 8, font_ascii + ('S' * 8), WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 3));

        MEM_COLOR_PALETTE(0)[0] = RGB(15, 15, 15);
        MEM_COLOR_PALETTE(0)[1] = RGB(0, 0, 0);
        MEM_COLOR_PALETTE(0)[2] = RGB(15, 0, 0);
        MEM_COLOR_PALETTE(0)[3] = RGB(0, 15, 0);
    } else {
        ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
        outportw(IO_SCR_PAL_0, MONO_PAL_COLORS(7, 0, 0, 0));

        // This test is WSC-only.
        ws_screen_put_tiles(SCREEN_1, ws_mono_text, 10, 8, 8, 1);
    }

    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    while(1);
}
