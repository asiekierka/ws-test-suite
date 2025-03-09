#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/util.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];
__attribute__((section(".iramx_17c0")))
uint8_t wave_ram[4][16];

static const char __wf_rom msg_sound_output_ports[] = "Sound output test ports:";
static const char __wf_rom msg_no_update_disabled[] = "Counter off if chn off:";
static const char __wf_rom msg_if_alt_mode[] = "... on if alt mode:";
static const char __wf_rom msg_voice_on_without_channel[] = "Voice on w/o channel:";
static const char __wf_rom msg_noise_off_without_channel[] = "Noise off w/o channel:";

#include "test/pass_fail.h"

extern uint16_t portw_wait_change(uint16_t value, uint16_t port);

#define APPROX_ONE_SCANLINE 84

int main(void) {
    volatile uint16_t current_output = 0;
    int i = 0, j = 0;

    init_pass_fail();
    cpu_irq_disable();
    outportb(IO_SERIAL_STATUS, 0);

    for (i = 0; i < 4; i++) {
        // Initialize wave RAM - 0123456789ABCDEF pattern
        for (j = 0; j < 16; j++)
            wave_ram[i][j] = 0x10 + ((j & 7) * 0x22);

        // Initialize channels to move once per scanline (256 cycles)
        outportw(IO_SND_FREQ(i + 1), 2048 - 256);
        outportb(IO_SND_VOL(i + 1), 0x11);
    }
    outportb(IO_SND_WAVE_BASE, SND_WAVE_BASE(wave_ram));
    outportb(IO_SND_SWEEP, 0);
    outportb(IO_SND_SWEEP_TIME, 0);
    outportb(IO_SND_CH_CTRL, 0);
    outportb(IO_SND_OUT_CTRL, 0);
    outportb(IO_SND_VOL_CH2_VOICE, 0xF);

    // Test sound output ports
    i = 0;
    text_puts(screen_1, 0, 0, i, msg_sound_output_ports);

    outportb(IO_SND_CH_CTRL, SND_CH1_ENABLE);
    ws_busywait(1 * APPROX_ONE_SCANLINE);

    current_output = inportw(IO_SND_CH_OUT_L);
    current_output = portw_wait_change(current_output, IO_SND_CH_OUT_L);
    draw_pass_fail(i, 2, current_output < 0x10);

    current_output = inportw(IO_SND_CH_OUT_R);
    current_output = portw_wait_change(current_output, IO_SND_CH_OUT_R);
    draw_pass_fail(i, 1, current_output < 0x10);

    current_output = inportw(IO_SND_CH_OUT_LR);
    current_output |= portw_wait_change(current_output, IO_SND_CH_OUT_LR);
    draw_pass_fail(i++, 0, current_output < 0x20 && !(current_output & 1));

    // Disable CH1, check if the period counter is off
    text_puts(screen_1, 0, 0, i, msg_no_update_disabled);

    current_output = inportw(IO_SND_CH_OUT_L);
    // - Wait six scanlines while the channel is not playing.
    outportb(IO_SND_CH_CTRL, 0);
    ws_busywait(6 * APPROX_ONE_SCANLINE);
    // - Slow down period counter as much as possible
    outportw(IO_SND_FREQ(1), 1);
    // - Re-enable channel 1
    outportb(IO_SND_CH_CTRL, SND_CH1_ENABLE);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    // 1 = one scanline elapsed, 2 = two scanlines elapsed, 3 = ...
    // We're not testing exact timing here, so let's assume any result <= 3 is passing.
    draw_pass_fail(i++, 0, ((inportw(IO_SND_CH_OUT_L) - current_output) & 0xF) <= 3);
    outportw(IO_SND_FREQ(1), 2048 - 256);
    
    // Switch CH2,3,4 to alt modes, check if the period counter is on
    text_puts(screen_1, 0, 0, i, msg_if_alt_mode);

    for (j = 0; j < 3; j++) {
        // Enable channel (2, 3, 4) in wavetable mode
        outportb(IO_SND_CH_CTRL, 0);
        ws_busywait(2 * APPROX_ONE_SCANLINE);
        outportb(IO_SND_CH_CTRL, 0x2 << j);
        ws_busywait(2 * APPROX_ONE_SCANLINE);
        current_output = inportw(IO_SND_CH_OUT_L);
        // - Wait six scanlines while the channel is in alt mode
        outportb(IO_SND_CH_CTRL, 0x22 << j);
        ws_busywait(6 * APPROX_ONE_SCANLINE);
        // - Slow down period counter as much as possible
        outportw(IO_SND_FREQ(j + 2), 1);
        // - Re-enable channel (2, 3, 4) in wavetable mode
        outportb(IO_SND_CH_CTRL, 0x2 << j);
        ws_busywait(2 * APPROX_ONE_SCANLINE);
        // 1 = one scanline elapsed, 2 = two scanlines elapsed, 3 = ...
        // We're not testing exact timing here, so let's assume any result <= 3 is failing.
        volatile bool no_update = ((inportw(IO_SND_CH_OUT_L) - current_output) & 0xF) <= 3;
        draw_pass_fail(i, 2 - j, !no_update);
        outportw(IO_SND_FREQ(j + 2), 2048 - 256);
        outportb(IO_SND_CH_CTRL, 0);
    }
    i++;
    
    // Test if sound is on when channel is off but voice is on
    text_puts(screen_1, 0, 0, i, msg_voice_on_without_channel);
    outportb(IO_SND_CH_CTRL, 0);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    outportb(IO_SND_CH_CTRL, SND_CH2_VOICE);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    draw_pass_fail(i++, 0, inportw(IO_SND_CH_OUT_L) == 0x11);

    // Test if sound is off when channel is off but noise is on
    text_puts(screen_1, 0, 0, i, msg_noise_off_without_channel);
    outportb(IO_SND_NOISE_CTRL, SND_NOISE_ENABLE | SND_NOISE_LEN_28);
    outportb(IO_SND_CH_CTRL, 0);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    outportb(IO_SND_CH_CTRL, SND_CH4_ENABLE | SND_CH4_NOISE);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    draw_pass_fail(i, 1, portw_wait_change(0, IO_SND_CH_OUT_L) != 0xFFFF);

    outportb(IO_SND_CH_CTRL, 0);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    outportb(IO_SND_CH_CTRL, SND_CH4_NOISE);
    ws_busywait(2 * APPROX_ONE_SCANLINE);
    draw_pass_fail(i++, 0, portw_wait_change(0, IO_SND_CH_OUT_L) == 0xFFFF);
    
    while(1);
}
