#include <string.h>
#include <ws.h>
#include <wsx/planar_unpack.h>
#include "resources.h"
#include "text.h"

// Reserve 0x58 bytes of space in RAM
__attribute__((section(".iramx_0040.backup_regs")))
volatile uint16_t backup_regs[0x18 >> 1];
uint8_t backup_io_regs[0xC0];

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom reg_names[] = "AXBXCXDXSPBPSIDIDSESSSFL";
static const char __wf_rom hex_chars[] = "0123456789ABCDEF";

// DS pointers...
__attribute__((optimize("-O0")))
int main(void) {
    for (int i = 0; i < sizeof(backup_io_regs); i++)
        backup_io_regs[i] = inportb(i);

    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    text_init();
    wsx_planar_unpack(MEM_TILE(256), 256 * 8, hex_00_ff, WSX_PLANAR_UNPACK_1BPP_TO_2BPP(0, 1));
    ws_screen_fill_tiles(screen_1, 32, 0, 0, 28, 18);
    outportb(IO_SCR_BASE, SCR1_BASE(screen_1));
    outportw(IO_SCR1_SCRL_X, 0);
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    // Print initial register state
    for (int i = 0; i < (0x18 >> 1); i++) {
        ws_screen_put_tile(screen_1, reg_names[i*2], 0, i);
        ws_screen_put_tile(screen_1, reg_names[i*2+1], 1, i);
        ws_screen_put_tile(screen_1, (backup_regs[i] >> 8) | 256, 2, i);
        ws_screen_put_tile(screen_1, (backup_regs[i] & 0xFF) | 256, 3, i); 
    }

    // Print I/O ports
    for (int i = 0; i < 16; i++) {
        ws_screen_put_tile(screen_1, hex_chars[i], 8+i, 0);
        if (i < sizeof(backup_io_regs)>>4) ws_screen_put_tile(screen_1, hex_chars[i], 7, 1+i);
    }
    for (int i = 0; i < sizeof(backup_io_regs); i++) {
        ws_screen_put_tile(screen_1, backup_io_regs[i] | 256, 8+(i&0xF), 1+(i>>4));
    }

    while(1);
}
