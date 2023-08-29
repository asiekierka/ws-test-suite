#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

void draw_pass_fail(uint8_t y, uint8_t offset, bool result) {
    ws_screen_put_tile(screen_1, result ? 5 : 6, 27 - offset, y);
}

static const char __wf_rom msg_0xb0[] = "0xB4 => 0xB0:";

int main(void) {
    text_init();
    ws_screen_fill_tiles(screen_1, 32, 0, 0, 28, 18);
    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    outportb(IO_SCR_BASE, SCR1_BASE(screen_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    cpu_irq_disable();
    int i = 0;

    // "0xB4 => 0xB0"
    // Generally, IO[0xB0] & 0x7 == highest_set_bit(IO[0xB4]).
    outportb(IO_HWINT_ENABLE, 0x00);
    outportb(IO_HWINT_VECTOR, 0x87);
    outportb(IO_HWINT_ACK, 0xFF);

    // 0x00 => 0
    text_puts(screen_1, 0, 0, i, msg_0xb0);
    draw_pass_fail(i, 4, inportb(IO_HWINT_VECTOR) == 0x80);

    // 0x40 => 6
    outportb(IO_HWINT_ENABLE, HWINT_VBLANK);
    cpu_halt();
    cpu_halt();
    draw_pass_fail(i, 3, inportb(IO_HWINT_VECTOR) == 0x86); 

    // Enabling does not clear this value.
    outportb(IO_HWINT_ENABLE, 0);
    draw_pass_fail(i, 2, inportb(IO_HWINT_VECTOR) == 0x86);
    // Enabling and acknowledging does clear this value.
    outportb(IO_HWINT_ACK, 0xFF);
    draw_pass_fail(i, 1, inportb(IO_HWINT_VECTOR) == 0x80);

    // 0x80 => 7
    outportw(IO_HBLANK_TIMER, 1);
    outportw(IO_TIMER_CTRL, HBLANK_TIMER_ENABLE | HBLANK_TIMER_ONESHOT);
    outportb(IO_HWINT_ENABLE, HWINT_HBLANK_TIMER);
    cpu_halt();
    draw_pass_fail(i++, 0, inportb(IO_HWINT_VECTOR) == 0x87);

    while(1);
}
