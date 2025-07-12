#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/keypad.h>
#include <ws/system.h>
#include "text.h"

__attribute__((section(".iramx.audio_buffer")))
uint8_t audio_buffer[256];
__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

const char __wf_rom hex_to_chr[] = "0123456789ABCDEF";

void draw_hex(uint16_t value, uint8_t digits, uint16_t entry, int x, int y) {
    uint8_t max = (digits - 1) << 2;
    for (uint8_t i = 0; i < digits; i++) {
        uint8_t h = (value >> (max - (i << 2))) & 0xF;
        ws_screen_put_tile(screen_1, ((uint8_t) hex_to_chr[h]) | entry, x+i, y);
    }
}

static const char __wf_rom msg_frequency[] = "Freq.: ";
static const char __wf_rom msg_frequency0[] = "24 kHz";
static const char __wf_rom msg_frequency1[] = "12 kHz";
static const char __wf_rom msg_frequency2[] = " 8 kHz";
static const char __wf_rom msg_frequency3[] = " 6 kHz";
static const char __wf_rom msg_frequency4[] = "4.8kHz";
static const char __wf_rom msg_frequency5[] = " 4 kHz";
static const char __wf_rom msg_frequency6[] = " 3 kHz";
static const char __wf_rom msg_frequency7[] = " 2 kHz";
static const char __wf_rom *__wf_rom msg_frequency_param[] = {
    msg_frequency0,
    msg_frequency1,
    msg_frequency2,
    msg_frequency3,
    msg_frequency4,
    msg_frequency5,
    msg_frequency6,
    msg_frequency7
};

static const char __wf_rom msg_dma_freq[] = "DMA F.:";
static const char __wf_rom *__wf_rom msg_dma_freq_param[] = {
    msg_frequency0,
    msg_frequency1,
    msg_frequency3,
    msg_frequency5
};

static const char __wf_rom msg_target[] = "Target:";
static const char __wf_rom msg_target0[] = "Stereo";
static const char __wf_rom msg_target1[] = "Left";
static const char __wf_rom msg_target2[] = "Right";
static const char __wf_rom msg_target3[] = "Mono";
static const char __wf_rom *__wf_rom msg_target_param[] = {
    msg_target0,
    msg_target1,
    msg_target2,
    msg_target3
};

static const char __wf_rom msg_volume[] = "Volume:";
static const char __wf_rom msg_volume0[] = "100%";
static const char __wf_rom msg_volume1[] = "50%";
static const char __wf_rom msg_volume2[] = "25%";
static const char __wf_rom msg_volume3[] = "12.5%";
static const char __wf_rom *__wf_rom msg_volume_param[] = {
    msg_volume0,
    msg_volume1,
    msg_volume2,
    msg_volume3
};

static const char __wf_rom msg_mode[] = "Mode:    ";
static const char __wf_rom msg_mode0[] = "Unsign";
static const char __wf_rom msg_mode1[] = "!Unsign";
static const char __wf_rom msg_mode2[] = "Signed";
static const char __wf_rom msg_mode3[] = "Signed F";
static const char __wf_rom *__wf_rom msg_mode_param[] = {
    msg_mode0,
    msg_mode1,
    msg_mode2,
    msg_mode3
};

static const char __wf_rom msg_enable[] = "Enable ";
static const char __wf_rom msg_disable[] = "Disable";
static const char __wf_rom msg_reset[] = "Reset";

static const char __wf_rom msg_sample[] = "Sample:    ";
static const char __wf_rom msg_sample0[] = "12kHz Sq S";
static const char __wf_rom msg_sample1[] = " 6kHz Sq S";
static const char __wf_rom msg_sample2[] = " 4kHz Sq S";
static const char __wf_rom msg_sample3[] = " 3kHz Sq S";
static const char __wf_rom msg_sample4[] = "12kHz Sq U";
static const char __wf_rom msg_sample5[] = " 6kHz Sq U";
static const char __wf_rom msg_sample6[] = " 4kHz Sq U";
static const char __wf_rom msg_sample7[] = " 3kHz Sq U";
static const char __wf_rom msg_sample8[] = "Triangle";
static const char __wf_rom msg_sample9[] = "All 1s";
static const char __wf_rom msg_sample10[] = "All 0s";
static const char __wf_rom *__wf_rom msg_sample_param[] = {
    msg_sample0,
    msg_sample1,
    msg_sample2,
    msg_sample3,
    msg_sample4,
    msg_sample5,
    msg_sample6,
    msg_sample7,
    msg_sample8,
    msg_sample9,
    msg_sample10
};
#define MSG_SAMPLE_COUNT 11

static const char __wf_rom msg_hvtester[] = "hvtester";
static const char __wf_rom msg_hvtester2[] = "asie";
static const char __wf_rom msg_hvtester3[] = "2024";

void draw_selector(const char __wf_rom *title, const char __wf_rom *__wf_rom *params, uint16_t params_count, uint8_t x, uint8_t y, bool selected, int *position) {
    text_puts(screen_1, SCR_ENTRY_PALETTE(selected ? 1 : 3), x, y, title);
    if (selected) {
        if (*position < 0) *position = 0;
        if (*position >= params_count) *position = params_count - 1;
    }
    for (int i = 0; i < params_count; i++) {
        text_puts(screen_1, SCR_ENTRY_PALETTE(selected ? 0 : 3), x+1, 1+y+i, params[i]);
        ws_screen_put_tile(screen_1, SCR_ENTRY_PALETTE(selected ? 3 : 2) | (uint8_t)((i == *position) ? '>' : ' '), x, 1+y+i);
    }
}

void gen_sample(int id) {
    int i = 0;
    switch(id) {
    case 0: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 1) == 0) ? 0x7F : 0x80; i++; } break;
    case 1: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 2) == 0) ? 0x7F : 0x80; i++; } break;
    case 2: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i % 6) <= 2) ? 0x7F : 0x80; i++; } break;
    case 3: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 4) == 0) ? 0x7F : 0x80; i++; } break;
    case 4: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 1) == 0) ? 0xFF : 0x00; i++; } break;
    case 5: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 2) == 0) ? 0xFF : 0x00; i++; } break;
    case 6: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i % 6) <= 2) ? 0xFF : 0x00; i++; } break;
    case 7: while (i < sizeof(audio_buffer)) { audio_buffer[i] = ((i & 4) == 0) ? 0xFF : 0x00; i++; } break;
    case 8: while (i < sizeof(audio_buffer)) { audio_buffer[i] = i; i++; } break;
    case 9: memset(audio_buffer, 0xFF, sizeof(audio_buffer)); break;
    case 10: memset(audio_buffer, 0x00, sizeof(audio_buffer)); break;
    }

}

int main(void) {
    if (!ws_system_mode_set(WS_MODE_COLOR)) {
        while(1);
    }
    MEM_COLOR_PALETTE(0)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(0)[1] = 0x0000;
    MEM_COLOR_PALETTE(1)[0] = 0x0000;
    MEM_COLOR_PALETTE(1)[1] = 0x0FFF;
    MEM_COLOR_PALETTE(2)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(2)[1] = 0x0CCC;
    MEM_COLOR_PALETTE(3)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(3)[1] = 0x0777;
    MEM_COLOR_PALETTE(8)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(8)[1] = 0x022A;
    MEM_COLOR_PALETTE(9)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(9)[1] = 0x09AF;

    text_init();
    ws_screen_fill_tiles(screen_1, 32, 0, 0, 28, 18);
    outportb(IO_SCR_BASE, SCR1_BASE(screen_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    ws_hwint_set_default_handler_vblank();
    ws_hwint_enable(HWINT_VBLANK);
    cpu_irq_enable();

    text_puts(screen_1, SCR_ENTRY_PALETTE(8), 0, 16, msg_hvtester);
    text_puts(screen_1, SCR_ENTRY_PALETTE(0), 0, 17, msg_hvtester2);
    text_puts(screen_1, SCR_ENTRY_PALETTE(9), 4, 17, msg_hvtester3);

    int option_pos = 0;
    int subopt_pos[8] = {0};

    uint16_t old_keys = 0, keys, keys_pressed;

    outportw(IO_SDMA_SOURCE_L, (uint16_t) audio_buffer);
    outportb(IO_SDMA_SOURCE_H, 0);
    outportw(IO_SDMA_LENGTH_L, sizeof(audio_buffer));
    outportb(IO_SDMA_LENGTH_H, 0);

    while(1) {
        if (option_pos < 0) option_pos = 0;
        if (option_pos >= 8) option_pos = 7;

        draw_selector(msg_frequency, msg_frequency_param, 8, 0, 0, option_pos == 0, subopt_pos);
        draw_selector(msg_dma_freq, msg_dma_freq_param, 4, 0, 10, option_pos == 4, subopt_pos + 4);
        draw_selector(msg_target, msg_target_param, 4, 8, 0, option_pos == 1, subopt_pos + 1);
        draw_selector(msg_volume, msg_volume_param, 4, 8, 6, option_pos == 2, subopt_pos + 2);
        draw_selector(msg_mode, msg_mode_param, 4, 8, 12, option_pos == 5, subopt_pos + 5);
        draw_selector(msg_sample, msg_sample_param, MSG_SAMPLE_COUNT, 17, 0, option_pos == 3, subopt_pos + 3);

        outportb(IO_SND_OUT_CTRL, 0);
        gen_sample(subopt_pos[3]);
        outportb(IO_SDMA_CTRL, DMA_TRANSFER_ENABLE
            | (subopt_pos[4] ^ 3)
            | SDMA_REPEAT
            | SDMA_TARGET_HYPERV);
        outportw(IO_HYPERV_CTRL,
            (subopt_pos[0] << 4)
            | (subopt_pos[1] << 13)
            | (subopt_pos[2])
            | (subopt_pos[5] << 2)
            | ((subopt_pos[6] & 1) ? HYPERV_ENABLE : 0)
            | (subopt_pos[7] ? HYPERV_RESET : 0)
        );
        outportb(IO_SND_OUT_CTRL, SND_OUT_HEADPHONES_ENABLE);
        if (subopt_pos[7]) subopt_pos[7] = 0;

        text_puts(screen_1, SCR_ENTRY_PALETTE((option_pos == 6) ? 1 : 3), 19, 14, (subopt_pos[6] & 1) ? msg_disable : msg_enable);
        text_puts(screen_1, SCR_ENTRY_PALETTE((option_pos == 7) ? 1 : 3), 19, 16, msg_reset);

        keys_pressed = 0;
        while (keys_pressed == 0) {
            cpu_halt();

            keys = ws_keypad_scan();
            keys_pressed = keys & ~old_keys;
            old_keys = keys;
        }

        if (keys_pressed & KEY_X1) {
            subopt_pos[option_pos]--;
        } else if ((keys_pressed & KEY_X3) || (keys_pressed & KEY_A)) {
            subopt_pos[option_pos]++;
        } else if (keys_pressed & KEY_X4) {
            option_pos--;
        } else if (keys_pressed & KEY_X2) {
            option_pos++;
        }
    }
}
