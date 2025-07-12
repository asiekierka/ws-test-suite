#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/system.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_gdma_ignores_bit_0[] = "GDMA ignores bit 0:";
static const char __wf_rom msg_gdma_source_20_bit[] = "GDMA source 20-bit:";
static const char __wf_rom msg_gdma_sram_fails[] = "GDMA<-SRAM fails:";
static const char __wf_rom msg_gdma_slow_rom_fails[] = "GDMA<-slow ROM fails:";
static const char __wf_rom msg_gdma_iram_ok[] = "GDMA<-IRAM OK:";
static const char __wf_rom msg_gdma_fast_rom_ok[] = "GDMA<-fast ROM OK:";

#define COLOR
#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();

    int i = 0;

    // source, destination, length don't set bit 0
    outportw(IO_DMA_SOURCE_L, 0xB001);
    outportw(IO_DMA_SOURCE_H, 0xFFFF);
    outportw(IO_DMA_DEST, 0x7001);
    outportw(IO_DMA_LENGTH, 0xFFFF);
    text_puts(screen_1, 0, 0, i, msg_gdma_ignores_bit_0);
    draw_pass_fail(i, 2, (inportw(IO_DMA_SOURCE_L) & 1) == 0); 
    draw_pass_fail(i, 1, (inportw(IO_DMA_DEST) & 1) == 0); 
    draw_pass_fail(i++, 0, (inportw(IO_DMA_LENGTH) & 1) == 0); 
    text_puts(screen_1, 0, 0, i, msg_gdma_source_20_bit);
    draw_pass_fail(i++, 0, inportw(IO_DMA_SOURCE_H) == 0x000F);

    outportw(IO_DMA_SOURCE_H, 0x0001);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0x7000);
    outportw(IO_DMA_LENGTH, 0x1000);
    text_puts(screen_1, 0, 0, i, msg_gdma_sram_fails);
    outportb(IO_DMA_CTRL, DMA_TRANSFER_ENABLE);
    draw_pass_fail(i++, 0, inportw(IO_DMA_LENGTH) == 0x1000);

    outportw(IO_DMA_SOURCE_H, 0x0008);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0x7000);
    outportw(IO_DMA_LENGTH, 0x1000);
    outportb(IO_SYSTEM_CTRL1, inportb(IO_SYSTEM_CTRL1) | SYSTEM_CTRL1_ROM_WAIT);
    text_puts(screen_1, 0, 0, i, msg_gdma_slow_rom_fails);
    outportb(IO_DMA_CTRL, DMA_TRANSFER_ENABLE);
    draw_pass_fail(i++, 0, inportw(IO_DMA_LENGTH) == 0x1000);
    outportb(IO_SYSTEM_CTRL1, inportb(IO_SYSTEM_CTRL1) & (~SYSTEM_CTRL1_ROM_WAIT));

    outportw(IO_DMA_SOURCE_H, 0x0000);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0x7000);
    outportw(IO_DMA_LENGTH, 0x1000);
    text_puts(screen_1, 0, 0, i, msg_gdma_iram_ok);
    outportb(IO_DMA_CTRL, DMA_TRANSFER_ENABLE);
    draw_pass_fail(i++, 0, inportw(IO_DMA_LENGTH) == 0x0000);

    outportw(IO_DMA_SOURCE_H, 0x0008);
    outportw(IO_DMA_SOURCE_L, 0x0000);
    outportw(IO_DMA_DEST, 0x7000);
    outportw(IO_DMA_LENGTH, 0x1000);
    text_puts(screen_1, 0, 0, i, msg_gdma_fast_rom_ok);
    outportb(IO_DMA_CTRL, DMA_TRANSFER_ENABLE);
    draw_pass_fail(i++, 0, inportw(IO_DMA_LENGTH) == 0x0000);

    while(1);
}
