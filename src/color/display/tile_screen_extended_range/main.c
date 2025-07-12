#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <wsx/planar_unpack.h>
#include "resources.h"

typedef struct {
	union {
		struct {
			uint16_t tile : 9;
			uint8_t palette : 3;
			bool inside : 1;
			bool priority : 1;
			bool flip_h : 1;
			bool flip_v : 1;
		};
		uint16_t attr;
	};
	uint8_t y;
	uint8_t x;
} ws_legacy_sprite_t;

__attribute__((section(".iramcx_1800")))
uint16_t SCREEN_1[32 * 32];
__attribute__((section(".iramcx_5800")))
uint16_t SCREEN_1_EXT[32 * 32];
__attribute__((section(".iramcx_1600")))
ws_legacy_sprite_t SPRITES[128];
__attribute__((section(".iramcx_5600")))
ws_legacy_sprite_t SPRITES_EXT[128];
__attribute__((section(".iramx_2000")))
ws_display_tile_t tiles_2bpp[512];

static const uint16_t __wf_rom header_text[] = {
    'c', 'o', 'l', 'o', 'r', '_', '2',
    'b', 'p', 'p', '_', 'e', 'x', 't',
    'e', 'n', 'd', 'e', 'd', '_', 'r',
    'a', 'n', 'g', 'e', 's'
};

static const uint16_t __wf_rom ws_mono_text[] = {
    'W', 'S', 'C', ' ', 'o', 'n', 'l', 'y'
};

static const uint16_t __wf_rom bg_text[] = {
    '@', 'S', 'C', 'R', 'E', 'E', 'N', ':', ' ',
    'B', 'G', ' ', 'T', 'I', 'L', 'E', ':', ' ',
    '@', 'S', 'P', 'R', 'I', 'T', 'E', ':', ' ',
};

static const uint16_t __wf_rom pass_text[] = {
    SCR_ENTRY_PALETTE(9) | 'P', SCR_ENTRY_PALETTE(9) | 'A', SCR_ENTRY_PALETTE(9) | 'S', SCR_ENTRY_PALETTE(9) | 'S'
};

static const uint16_t __wf_rom fail_text[] = {
    SCR_ENTRY_PALETTE(8) | 'F', SCR_ENTRY_PALETTE(8) | 'A', SCR_ENTRY_PALETTE(8) | 'I', SCR_ENTRY_PALETTE(8) | 'L'
};

int main(void) {
    // Load ASCII font, prepare test header.
    wsx_planar_unpack(MEM_TILE(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    ws_screen_put_tiles(SCREEN_1, header_text, 1, 17, 26, 1);

    if (ws_mode_set(WS_MODE_COLOR)) {
        ws_screen_put_tiles(SCREEN_1, bg_text, 7, 7, 9, 3);

        MEM_COLOR_PALETTE(0)[0] = RGB(15, 15, 15);
        MEM_COLOR_PALETTE(8)[0] = RGB(15, 15, 15);
        MEM_COLOR_PALETTE(9)[0] = RGB(15, 15, 15);

        MEM_COLOR_PALETTE(0)[1] = RGB(0, 0, 0);
        MEM_COLOR_PALETTE(0)[2] = RGB(12, 0, 0);
        MEM_COLOR_PALETTE(0)[3] = RGB(0, 13, 0);
        MEM_COLOR_PALETTE(8)[1] = RGB(12, 0, 0);
        MEM_COLOR_PALETTE(9)[1] = RGB(0, 13, 0);

        // BG TILE test:
        // In 2BPP color mode, 0x2000~0x5FFF is used to map 1024 tiles.
        // Some older emulators get this wrong and ignore the additional
        // WSC-specific bit, mapping only the first 512 tiles.

        // Write text (tiles 512 + (508, 509, 510, 511))
        for (uint8_t i = 0; i < 4; i++) {
            ws_screen_put_tile(SCREEN_1, SCR_ENTRY_BANK(1) | SCR_ENTRY_PALETTE(0) | 508 | i, 16 + i, 8);
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

        // @SPRITE test:
        // Sprite tables can be stored at 0x4000~0x7FFF when color mode is enabled.
        memset(SPRITES, 0, 16);
        SPRITES[0].x = 16 * 8; SPRITES[0].y = 9 * 8; SPRITES[0].tile = 'F';
        SPRITES[1].x = 17 * 8; SPRITES[1].y = 9 * 8; SPRITES[1].tile = 'A';
        SPRITES[2].x = 18 * 8; SPRITES[2].y = 9 * 8; SPRITES[2].tile = 'I';
        SPRITES[3].x = 19 * 8; SPRITES[3].y = 9 * 8; SPRITES[3].tile = 'L';
        memcpy(SPRITES_EXT, SPRITES, 16);
        SPRITES_EXT[0].tile = 'P'; SPRITES_EXT[0].palette = 1;
        SPRITES_EXT[1].tile = 'A'; SPRITES_EXT[1].palette = 1;
        SPRITES_EXT[2].tile = 'S'; SPRITES_EXT[2].palette = 1;
        SPRITES_EXT[3].tile = 'S'; SPRITES_EXT[3].palette = 1;
        
        outportb(IO_SPR_BASE, SPR_BASE(SPRITES_EXT));
        outportb(IO_SPR_FIRST, 0);
        outportb(IO_SPR_COUNT, 4);

        // @SCREEN test:
        // Screens can be stored at 0x4000~0x7FFF when color mode is enabled.
        // This must run last, to copy the rest of the screen state as-is.
        // Avoid copying the end of the screen - tiles 512+N live there.
        memcpy(SCREEN_1_EXT, SCREEN_1, 18 * 32 * 2);
        ws_screen_put_tiles(SCREEN_1, fail_text, 16, 7, 4, 1);
        ws_screen_put_tiles(SCREEN_1_EXT, pass_text, 16, 7, 4, 1);
    } else {
        ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
        outportw(IO_SCR_PAL_0, MONO_PAL_COLORS(7, 0, 0, 0));

        // This test is WSC-only.
        ws_screen_put_tiles(SCREEN_1, ws_mono_text, 10, 8, 8, 1);
        outportb(IO_SPR_COUNT, 0);
    }

    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1_EXT));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SPR_ENABLE);

    while(1);
}
