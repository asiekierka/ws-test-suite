#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/dma.h>
#include <ws/hardware.h>
#include <ws/system.h>
#include <ws/util.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_sdma_source_20_bit[] = "SDMA source 20-bit:";
static const char __wf_rom msg_sdma_length_20_bit[] = "SDMA length 20-bit:";
static const char __wf_rom msg_sdma_iram[] = "SDMA<-IRAM OK:";
static const char __wf_rom msg_sdma_fast_rom[] = "SDMA<-fast ROM OK:";
static const char __wf_rom msg_sdma_slow_rom[] = "SDMA<-slow ROM OK:";
static const char __wf_rom msg_sdma_fast_sram[] = "SDMA<-fast SRAM OK:";
static const char __wf_rom msg_sdma_slow_sram[] = "SDMA<-slow SRAM OK:";
static const char __wf_rom msg_sram_not_detected[] = "SRAM not detected!";
static const char __wf_rom msg_sdma_hold[] = "SDMA hold OK:";
static const char __wf_rom msg_sdma_finish_zeroes[] = "SDMA finish zeroes:";

static uint8_t sample_data[16];

__attribute__((section(".sram")))
static uint8_t __far sample_data_sram[16];

static const uint8_t __wf_rom sample_data_rom[16] = {
    0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55
};

static inline void run_sdma_test(const void __far *source, uint32_t length, uint8_t ctrl) {
    outportb(IO_SDMA_CTRL, 0x00);
    ws_busywait(512);
    outportb(IO_SND_VOL_CH2, 0x00);
    ws_sdma_set_source(source);
    ws_sdma_set_length(length);
    outportb(IO_SDMA_CTRL, DMA_TRANSFER_ENABLE | ctrl);
    ws_busywait(512);
}

#define COLOR
#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    memcpy(sample_data, sample_data_rom, sizeof(sample_data));
    int i = 0;

    outportw(IO_SDMA_SOURCE_L, 0x5555);
    outportw(IO_SDMA_LENGTH_L, 0x5555);
    outportw(IO_SDMA_SOURCE_H, 0xFFFF);
    outportw(IO_SDMA_LENGTH_H, 0xFFFF);
    text_puts(screen_1, 0, 0, i, msg_sdma_source_20_bit);
    draw_pass_fail(i, 1, inportw(IO_SDMA_SOURCE_H) == 0x000F);
    draw_pass_fail(i++, 0, inportw(IO_SDMA_SOURCE_L) == 0x5555);
    text_puts(screen_1, 0, 0, i, msg_sdma_length_20_bit);
    draw_pass_fail(i, 1, inportw(IO_SDMA_LENGTH_H) == 0x000F);
    draw_pass_fail(i++, 0, inportw(IO_SDMA_LENGTH_L) == 0x5555);

    outportb(IO_SYSTEM_CTRL1, inportb(IO_SYSTEM_CTRL1) | SYSTEM_CTRL1_ROM_WAIT);
    text_puts(screen_1, 0, 0, i, msg_sdma_slow_rom);
    run_sdma_test(sample_data_rom, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
    draw_pass_fail(i++, 0, inportb(IO_SND_VOL_CH2) == 0x55);

    outportb(IO_SYSTEM_CTRL1, inportb(IO_SYSTEM_CTRL1) & (~SYSTEM_CTRL1_ROM_WAIT));
    text_puts(screen_1, 0, 0, i, msg_sdma_fast_rom);
    run_sdma_test(sample_data_rom, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
    draw_pass_fail(i++, 0, inportb(IO_SND_VOL_CH2) == 0x55);

    text_puts(screen_1, 0, 0, i, msg_sdma_iram);
    run_sdma_test(sample_data, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
    draw_pass_fail(i++, 0, inportb(IO_SND_VOL_CH2) == 0x55);

#ifndef __IA16_CMODEL_TINY__
    memcpy(sample_data_sram, sample_data_rom, sizeof(sample_data));

    if (!memcmp(sample_data_rom, sample_data_sram, sizeof(sample_data))) {
        outportb(IO_SYSTEM_CTRL2, inportb(IO_SYSTEM_CTRL2) | SYSTEM_CTRL2_SRAM_WAIT);
        text_puts(screen_1, 0, 0, i, msg_sdma_slow_sram);
        run_sdma_test(sample_data_sram, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
        draw_pass_fail(i++, 0, inportb(IO_SND_VOL_CH2) == 0x55);

        outportb(IO_SYSTEM_CTRL2, inportb(IO_SYSTEM_CTRL2) & (~SYSTEM_CTRL2_SRAM_WAIT));
        text_puts(screen_1, 0, 0, i, msg_sdma_fast_sram);
        run_sdma_test(sample_data_sram, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
        draw_pass_fail(i++, 0, inportb(IO_SND_VOL_CH2) == 0x55);
    } else {
        text_puts(screen_1, 0, 0, i++, msg_sram_not_detected);
    }
#endif

    text_puts(screen_1, 0, 0, i, msg_sdma_hold);
    run_sdma_test(sample_data, sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_24000);
    outportb(IO_SDMA_CTRL, inportb(IO_SDMA_CTRL) | SDMA_HOLD);
    ws_busywait(512);
    bool hold_passed = inportb(IO_SND_VOL_CH2) == 0x00;
    draw_pass_fail(i, 4, hold_passed);
    uint16_t hold_source = inportw(IO_SDMA_SOURCE_L);
    uint16_t hold_length = inportw(IO_SDMA_LENGTH_L);
    ws_busywait(333);
    draw_pass_fail(i, 3, inportw(IO_SDMA_SOURCE_L) == hold_source && inportw(IO_SDMA_LENGTH_L) == hold_length);
    outportb(IO_SDMA_CTRL, inportb(IO_SDMA_CTRL) & ~SDMA_HOLD);
    ws_busywait(512);
    draw_pass_fail(i, 2, hold_passed && (inportb(IO_SND_VOL_CH2) == 0x55));

    run_sdma_test(MK_FP(0x2000, 0x0000), 0x80000, SDMA_TARGET_CH2 | SDMA_REPEAT | SDMA_RATE_12000);
    ws_busywait(2048);
    outportb(IO_SDMA_CTRL, inportb(IO_SDMA_CTRL) | SDMA_HOLD);
    hold_source = inportw(IO_SDMA_SOURCE_L);
    uint16_t hold_source_h = inportw(IO_SDMA_SOURCE_H);
    hold_length = inportw(IO_SDMA_LENGTH_L);
    uint16_t hold_length_h = inportw(IO_SDMA_LENGTH_H);
    ws_busywait(2048);
    draw_pass_fail(i, 1, inportw(IO_SDMA_SOURCE_L) == hold_source && inportw(IO_SDMA_LENGTH_L) == hold_length && inportw(IO_SDMA_SOURCE_H) == hold_source_h && inportw(IO_SDMA_LENGTH_H) == hold_length_h);
    outportb(IO_SDMA_CTRL, inportb(IO_SDMA_CTRL) & ~SDMA_HOLD);
    ws_busywait(2048);
    outportb(IO_SDMA_CTRL, inportb(IO_SDMA_CTRL) | SDMA_HOLD);
    ws_busywait(2048);
    draw_pass_fail(i++, 0, (inportw(IO_SDMA_SOURCE_L) != hold_source || inportw(IO_SDMA_SOURCE_H) != hold_source_h) && (inportw(IO_SDMA_LENGTH_H) != hold_length_h || inportw(IO_SDMA_LENGTH_L) != hold_length));

    text_puts(screen_1, 0, 0, i, msg_sdma_finish_zeroes);
    run_sdma_test(MK_FP(0x0000, 0x1234), sizeof(sample_data_rom), SDMA_TARGET_CH2 | SDMA_RATE_24000);
    ws_busywait(16384);
    draw_pass_fail(i, 4, inportw(IO_SDMA_SOURCE_L) == 0x1244);
    draw_pass_fail(i, 3, inportw(IO_SDMA_SOURCE_H) == 0);
    draw_pass_fail(i, 2, inportw(IO_SDMA_LENGTH_L) == 0);
    draw_pass_fail(i, 1, inportw(IO_SDMA_LENGTH_H) == 0);
    draw_pass_fail(i++, 0, inportw(IO_SDMA_CTRL) == 0x03);

    while(1);
}
