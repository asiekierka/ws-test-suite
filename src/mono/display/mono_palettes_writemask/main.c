#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/util.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_cmd[] = "Palette %d:";

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();

    cpu_irq_disable();

    int i;
    for (i = 0; i < 16; i++) {
        text_printf(screen_1, 0, 0, i, msg_cmd, i);
	volatile uint16_t orig = inportw(0x20 + (i << 1));
        outportw(0x20 + (i << 1), 0xFFFF);
        volatile uint16_t result1 = inportw(0x20 + (i << 1));
        outportw(0x20 + (i << 1), 0x4321);
        volatile uint16_t result2 = inportw(0x20 + (i << 1));
        outportw(0x20 + (i << 1), orig);

        draw_pass_fail(i, 1, result1 == ((i & 4) ? 0x7770 : 0x7777));
        draw_pass_fail(i, 0, result2 == ((i & 4) ? 0x4320 : 0x4321));
    }

    while(1);
}
