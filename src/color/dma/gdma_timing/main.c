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

static const char __wf_rom msg_gdma_fast_rom[] = "GDMA<-ROM (%03d):";
static const char __wf_rom msg_gdma_sram[] = "GDMA<-SRAM (fail):";
static const char __wf_rom msg_d[] = "%d";
static const char __wf_rom msg_0nop[] = "0 x NOP";
static const char __wf_rom msg_1nop[] = "1 x NOP";
static const char __wf_rom msg_2nop[] = "2 x NOP";

#define COLOR
#include "test/pass_fail.h"

extern uint16_t do_cycle_count_dma(void);
extern uint16_t do_cycle_count_0nop(void);
extern uint16_t do_cycle_count_1nop(void);
extern uint16_t do_cycle_count_2nop(void);

static const uint8_t __wf_rom gdma_rom_lengths[9] = {
	0,2,4,6,8,16,32,64,128
};
static const uint16_t __wf_rom gdma_rom_cycles[9] = {
	14,
	14 + 5 + 2,
	14 + 5 + 4,
	14 + 5 + 6,
	14 + 5 + 8,
	14 + 5 + 16,
	14 + 5 + 32,
	14 + 5 + 64,
	14 + 5 + 128
};

int main(void) {
    uint16_t i = 0;
    uint16_t j;
    uint16_t cycles;

    init_pass_fail();

    text_puts(screen_1, 0, 0, i, msg_0nop);
    cycles = do_cycle_count_0nop();
    text_printf(screen_1, 0, 24, i, msg_d, cycles);
    draw_pass_fail(i++, 0, cycles == 6);

    text_puts(screen_1, 0, 0, i, msg_1nop);
    cycles = do_cycle_count_1nop();
    text_printf(screen_1, 0, 24, i, msg_d, cycles);
    draw_pass_fail(i++, 0, cycles == 7);

    text_puts(screen_1, 0, 0, i, msg_2nop);
    cycles = do_cycle_count_2nop();
    text_printf(screen_1, 0, 24, i, msg_d, cycles);
    draw_pass_fail(i++, 0, cycles == 8);

    for (j = 0; j < 9; j++) {
        outportw(IO_DMA_SOURCE_H, 0x0008);
        outportw(IO_DMA_SOURCE_L, 0x0000);
        outportw(IO_DMA_DEST, 0xFF00);
        outportw(IO_DMA_LENGTH, gdma_rom_lengths[j]);
        text_printf(screen_1, 0, 0, i, msg_gdma_fast_rom, gdma_rom_lengths[j]);
        cycles = do_cycle_count_dma();
        text_printf(screen_1, 0, 24, i, msg_d, cycles);
        draw_pass_fail(i++, 0, cycles == gdma_rom_cycles[j]);
    }

    outportw(IO_DMA_SOURCE_H, 0x0001);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0xFF00);
    outportw(IO_DMA_LENGTH, 1);
    text_puts(screen_1, 0, 0, i, msg_gdma_sram);
    cycles = do_cycle_count_dma();
    text_printf(screen_1, 0, 24, i, msg_d, cycles);
    draw_pass_fail(i++, 0, cycles == 14);

    while(1);
}
