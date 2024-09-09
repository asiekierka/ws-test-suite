#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/system.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_gdma_fast_rom[] = "GDMA<-ROM (%02d):";
static const char __wf_rom msg_gdma_sram[] = "GDMA<-SRAM (fail):";
static const char __wf_rom msg_d[] = "%d";

#define COLOR
#include "test/pass_fail.h"

extern uint16_t do_cycle_count_dma(void);

int main(void) {
    uint16_t i = 0;
    uint16_t cycles;

    init_pass_fail();

    uint16_t len = 16;
    for (; len >= 2; len -= 2) {
        outportw(IO_DMA_SOURCE_H, 0x0008);
        outportw(IO_DMA_SOURCE_L, 0x0000);
        outportw(IO_DMA_DEST, 0xF000);
        outportw(IO_DMA_LENGTH, len);
        text_printf(screen_1, 0, 0, i, msg_gdma_fast_rom, len);
        cycles = do_cycle_count_dma();
        text_printf(screen_1, 0, 24, i, msg_d, cycles);
        draw_pass_fail(i++, 0, true);
    }

    outportw(IO_DMA_SOURCE_H, 0x0001);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0xF000);
    outportw(IO_DMA_LENGTH, 1);
    text_puts(screen_1, 0, 0, i, msg_gdma_sram);
    cycles = do_cycle_count_dma();
    text_printf(screen_1, 0, 24, i, msg_d, cycles);
    draw_pass_fail(i++, 0, true);

    while(1);
}
