#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <wsx/planar_unpack.h>
#include "resources.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t SCREEN_1[32 * 32];
__attribute__((section(".iramx_1600")))
ws_sprite_t SPRITES[128];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const uint16_t __wf_rom header_text[] = {
    's', 'p', 'r', 'i', 't', 'e', '_',
    's', 'c', 'a', 'n', 'l', 'i', 'n',
    'e', '_', 'l', 'i', 'm', 'i', 't'
};

static const uint8_t pass_text[] = { 'P', 'A', 'S', 'S' };
static const uint8_t fail_text[] = { 'F', 'A', 'I', 'L' };

__attribute__((optimize("-O0")))
int main(void) {
    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    outportb(IO_SPR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));

    if (ws_system_color_active()) {
        MEM_COLOR_PALETTE(0)[0] = 0xFFF;
        MEM_COLOR_PALETTE(0)[1] = 0x000;
        MEM_COLOR_PALETTE(8)[0] = 0xFFF;
        MEM_COLOR_PALETTE(8)[1] = 0x000;
    }

    // Load ASCII font, prepare test header.
    wsx_planar_unpack(MEM_TILE(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    ws_screen_put_tiles(SCREEN_1, header_text, 3, 17, 21, 1);

    // Sprite test: The first 32 matching sprites are rendered in a scanline, from last to first.
    // PASS.... - success!
    // FAIL.... - sprite scanline limit implemented, sprite priority incorrect
    // PASSFAIL - sprite scanline limit not implemented, sprite priority correct
    // FAILFAIL - sprite scanline limit not implemented, sprite priority incorrect
    memset(SPRITES, 0x00, 36 * 4);
    int id = 0;
    for (int j = 0; j < 9; j++) {
        int xo = j == 8 ? (2 * 8) : (-2 * 8);
        const uint8_t *text = j == 0 ? pass_text : fail_text;
        for (int i = 0; i < 4; i++, id++) {
            SPRITES[id].x = (12 + i) * 8 + 4 + xo;
            SPRITES[id].y = 8 * 8;
            SPRITES[id].tile = text[i];
        }
    }

    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1));
    outportb(IO_SPR_BASE, SPR_BASE(SPRITES));
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 36);

    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SPR_ENABLE);

    while(1);
}
