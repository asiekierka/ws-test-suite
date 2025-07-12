#include <string.h>
#include <ws.h>
#include <wsx/planar_unpack.h>
#include "resources.h"
#include "text.h"

#define _HANDLE ws_eeprom_handle_cartridge(6)
#define MAX_COMMAND 15

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom hex_chars[] = "0123456789ABCDEF";

// DS pointers...
__attribute__((optimize("-O0")))
int main(void) {
    ws_eeprom_handle_t handle = _HANDLE;

    cpu_irq_disable();

    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    text_init();
    wsx_planar_unpack(MEM_TILE(256), 256 * 8, hex_00_ff, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
    ws_screen_fill_tiles(screen_1, 32, 0, 0, 28, 18);
    outportb(IO_SCR_BASE, SCR1_BASE(screen_1));
    outportw(IO_SCR1_SCRL_X, 0);
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    for (int i = 0; i < 16; i++) {
        ws_screen_put_tile(screen_1, hex_chars[i], 1+i, 0);
        if (i < 8) ws_screen_put_tile(screen_1, hex_chars[i], 0, 1+i);
    }
    for (int i = 0; i < 128; i+=2) {
        uint16_t word = ws_eeprom_read_word(handle, i);
        ws_screen_put_tile(screen_1, ((word >> 8) & 0xFF) | 256, 1+(i&0xF), 1+(i>>4));
        ws_screen_put_tile(screen_1, (word & 0xFF) | 256, 2+(i&0xF), 1+(i>>4));
    }

    while(1);
}
