#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/system.h>
#include "text.h"

__attribute__((section(".iramx_0dc0")))
ws_sound_wavetable_t wave;
__attribute__((section(".iramx_0e00")))
ws_sprite_table_t sprites;
__attribute__((section(".iramx_1000")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_1800")))
uint16_t screen_2[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];
__attribute__((section(".iramx_4000")))
ws_tile_4bpp_t tiles_4bpp[512];

uint8_t test_idx = 0;

static const char __wf_rom msg_test_counter[] = "%02d";
static const char __wf_rom msg_test_footer[] = "Press A to advance";
static const char __wf_rom msg_test_wait[] = "Please wait";

static const char __wf_rom msg_test01[] = "Display: Mono, SCR2\nOther devices: off\nHalting";
static const char __wf_rom msg_test01a[] = "Display: Mono, SCR2\nOther devices: off\nInverted";
static const char __wf_rom msg_test02[] = "Display: Mono, SCR2\nOther devices: off\nBusy loop";
static const char __wf_rom msg_test02a[] = "Display: Mono, SCR2\nOther devices: off\nIRAM busy loop";
static const char __wf_rom msg_test03[] = "Display: Mono, SCR2\nOther devices: off\nROM -> IRAM memcpy()\n(waitstate)";
static const char __wf_rom msg_test04[] = "Display: Mono, SCR2\nOther devices: off\nROM -> IRAM memcpy()\n(no waitstate)";
static const char __wf_rom msg_test05[] = "Display: Mono, SCR1+2\nOther devices: off";
static const char __wf_rom msg_test06[] = "Display: Mono, SCR1+2, sprite\nOther devices: off";
static const char __wf_rom msg_test07[] = "Display: Mono, SCR2\nSound: speaker 1ch\nUART: off";
static const char __wf_rom msg_test08[] = "Display: Mono, SCR2\nSound: speaker 4ch\nUART: off";
static const char __wf_rom msg_test08a[] = "Display: Mono, SCR2\nSound: speaker 1ch loud\nUART: off";
static const char __wf_rom msg_test08b[] = "Display: Mono, SCR2\nSound: speaker 0ch\nUART: off";
static const char __wf_rom msg_test09[] = "Display: Mono, SCR2\nSound: speaker+head.\nUART: off";
static const char __wf_rom msg_test10[] = "Display: Mono, SCR2\nSound: off\nUART: 9600";
static const char __wf_rom msg_test11[] = "Display: Mono, SCR2\nSound: off\nUART: 38400";
static const char __wf_rom msg_test12[] = "Display: Mono, sleep+idle\nSound: off\nUART: off";
static const char __wf_rom msg_test13[] = "Display: Mono, sleep+busy\nSound: off\nUART: off";
static const char __wf_rom msg_test14[] = "Display: Color2, SCR2";
static const char __wf_rom msg_test15[] = "Display: Color2, SCR1+2, sprite";
static const char __wf_rom msg_test16[] = "Display: Color4, SCR2";
static const char __wf_rom msg_test17[] = "Display: Color4, SCR1+2, sprite";
static const char __wf_rom msg_test18[] = "Display: Color2, SCR2\nSound: speaker+head.\nHyper Voice: on";
static const char __wf_rom msg_test19[] = "Display: Color2, SCR2\nROM -> IRAM GDMA";
static const char __wf_rom msg_test20[] = "Display: Color2, SCR2\nhigh contrast";

static const char __wf_rom msg_test_complete[] = "Testing complete";

void print_next_test(const char __far *name) {
	ws_screen_fill_tiles(screen_2, 32, 0, 0, 28, 18);	
	text_puts(screen_2, 0, 0, 0, name);
	text_printf(screen_2, 0, 26, 17, msg_test_counter, (int) (++test_idx));
	if (test_idx > 1) {
		text_puts(screen_2, 0, 0, 17, msg_test_wait);
		ws_delay_ms(1000);
	}
	text_puts(screen_2, 0, 0, 17, msg_test_footer);
}

void skip_next_test(int n) {
	test_idx += n;
}

uint16_t _keys_held = 0;

bool can_go_to_next_test(void) {
	uint16_t keys_current = ws_keypad_scan();
	uint16_t keys_pressed = keys_current & ~_keys_held;
	_keys_held = keys_current;
	ws_int_ack_all();
	return keys_pressed & WS_KEY_A;
}

__attribute__((noinline, section(".iram.wew")))
void busyloop_in_iram(void) {
	while (ws_display_get_current_line() == 144);
	while (ws_display_get_current_line() != 144);
}

int main(void) {
	// Initialize environment
	ws_sound_reset();
	ws_uart_close();
	outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_ENABLE);

	text_init();
	ws_display_set_shade_lut_default();
	outportw(WS_SCR_PAL_0_PORT, 0x7770);
	outportw(WS_SPR_PAL_0_PORT, 0x7730);
	ws_display_set_screen_addresses(screen_1, screen_2);
	ws_display_set_sprite_address(&sprites);
	outportb(WS_SPR_FIRST_PORT, 0);
	outportb(WS_SPR_COUNT_PORT, 128);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR2_ENABLE);

	for (int i = 0; i < 128; i++) {
		sprites.entry[i].x = ((i & 15) * 8) - (i >> 4) + 52;
		sprites.entry[i].y = (i >> 1) + 40;
		sprites.entry[i].attr = ((i & 63) + 64) | WS_SPRITE_ATTR_PRIORITY;
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 16; j++) {
			wave.wave[i].data[j] = (j & 8) ? 0xFF : 0x00;
		}
	}
	ws_sound_set_wavetable_address(&wave);

	ia16_disable_irq();
	ws_int_set_enabled(WS_INT_ENABLE_VBLANK);
	ws_int_ack_all();

	print_next_test(msg_test01);
	while (!can_go_to_next_test()) ia16_halt();

	outportw(WS_SCR_PAL_0_PORT, 0x7707);
	print_next_test(msg_test01a);
	while (!can_go_to_next_test()) ia16_halt();
	outportw(WS_SCR_PAL_0_PORT, 0x7770);

	print_next_test(msg_test02);
	while (!can_go_to_next_test()) {
		while (ws_display_get_current_line() == 144);
		while (ws_display_get_current_line() != 144);
	}

	print_next_test(msg_test02a);
	while (!can_go_to_next_test()) busyloop_in_iram();

	outportb(WS_SYSTEM_CTRL_PORT, inportb(WS_SYSTEM_CTRL_PORT) | WS_SYSTEM_CTRL_ROM_WAIT);
	print_next_test(msg_test03);
	while (!can_go_to_next_test()) {
		memcpy(screen_1, MK_FP(0xF000, 0x0000), sizeof(screen_1));
	}

	outportb(WS_SYSTEM_CTRL_PORT, inportb(WS_SYSTEM_CTRL_PORT) & ~WS_SYSTEM_CTRL_ROM_WAIT);
	print_next_test(msg_test04);
	while (!can_go_to_next_test()) {
		memcpy(screen_1, MK_FP(0xF000, 0x0000), sizeof(screen_1));
	}

	ws_screen_fill_tiles(screen_1, '.', 0, 0, 32, 32);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE);
	print_next_test(msg_test05);
	while (!can_go_to_next_test()) ia16_halt();

	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);
	print_next_test(msg_test06);
	while (!can_go_to_next_test()) ia16_halt();
	ws_display_set_control(WS_DISPLAY_CTRL_SCR2_ENABLE);

	outportb(WS_SOUND_OUT_CTRL_PORT, WS_SOUND_OUT_CTRL_SPEAKER_ENABLE);
	for (int i = 0; i < 4; i++) {
		outportw(WS_SOUND_FREQ_CH1_PORT + i*2, WS_SOUND_WAVE_HZ_TO_FREQ(1000, 32));
		outportb(WS_SOUND_VOL_CH1_PORT + i, 0x11);
	}
	outportb(WS_SOUND_CH_CTRL_PORT, 0x01);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR2_ENABLE);
	print_next_test(msg_test07);
	while (!can_go_to_next_test()) ia16_halt();

	outportb(WS_SOUND_CH_CTRL_PORT, 0x0F);
	print_next_test(msg_test08);
	while (!can_go_to_next_test()) ia16_halt();

	outportb(WS_SOUND_CH_CTRL_PORT, 0x01);
	outportb(WS_SOUND_VOL_CH1_PORT, 0xFF);
	print_next_test(msg_test08a);
	while (!can_go_to_next_test()) ia16_halt();

	outportb(WS_SOUND_CH_CTRL_PORT, 0x00);
	print_next_test(msg_test08b);
	while (!can_go_to_next_test()) ia16_halt();
	outportb(WS_SOUND_CH_CTRL_PORT, 0x01);

	outportb(WS_SOUND_OUT_CTRL_PORT, WS_SOUND_OUT_CTRL_SPEAKER_ENABLE | WS_SOUND_OUT_CTRL_HEADPHONE_ENABLE);
	print_next_test(msg_test09);
	while (!can_go_to_next_test()) ia16_halt();

	ws_sound_reset();
	ws_uart_open(WS_UART_BAUD_RATE_9600);
	print_next_test(msg_test10);
	while (!can_go_to_next_test()) ia16_halt();

	ws_uart_open(WS_UART_BAUD_RATE_38400);
	print_next_test(msg_test11);
	while (!can_go_to_next_test()) ia16_halt();
	ws_sound_reset();

	print_next_test(msg_test12);
	outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_DISABLE);
	while (!can_go_to_next_test()) ia16_halt();
	outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_ENABLE);

	print_next_test(msg_test13);
	outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_DISABLE);
	while (!can_go_to_next_test()) {
		memcpy(screen_1, MK_FP(0xF000, 0x0000), sizeof(screen_1));
	}
	outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_ENABLE);

	if (ws_system_is_color_model()) {
		ia16_halt();
		ws_screen_fill_tiles(screen_1, '.', 0, 0, 32, 32);
		ia16_halt();
		
		ws_system_set_mode(WS_MODE_COLOR);
		WS_DISPLAY_COLOR_MEM(0)[0] = 0xFFF;
		WS_DISPLAY_COLOR_MEM(0)[1] = 0x000;
		WS_DISPLAY_COLOR_MEM(8)[0] = 0xFFF;
		WS_DISPLAY_COLOR_MEM(8)[1] = 0x888;
		text_init_4bpp();

		print_next_test(msg_test14);
		while (!can_go_to_next_test()) ia16_halt();

		ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);
		print_next_test(msg_test15);
		while (!can_go_to_next_test()) ia16_halt();
		ws_display_set_control(WS_DISPLAY_CTRL_SCR2_ENABLE);

		ws_system_set_mode(WS_MODE_COLOR_4BPP);
		print_next_test(msg_test16);
		while (!can_go_to_next_test()) ia16_halt();

		ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);
		print_next_test(msg_test17);
		while (!can_go_to_next_test()) ia16_halt();
		ws_display_set_control(WS_DISPLAY_CTRL_SCR2_ENABLE);

		outportb(WS_SOUND_OUT_CTRL_PORT, WS_SOUND_OUT_CTRL_SPEAKER_ENABLE | WS_SOUND_OUT_CTRL_HEADPHONE_ENABLE);
		outportw(WS_HYPERV_CTRL_PORT, WS_HYPERV_CTRL_ENABLE | WS_HYPERV_CTRL_RATE_12000);
		ws_sdma_set_source(screen_1);
		ws_sdma_set_length(128);
		outportb(WS_SDMA_CTRL_PORT, WS_SDMA_CTRL_ENABLE | WS_SDMA_CTRL_RATE_24000 | WS_SDMA_CTRL_TARGET_HYPERV | WS_SDMA_CTRL_REPEAT);
		print_next_test(msg_test18);
		while (!can_go_to_next_test()) ia16_halt();
		outportb(WS_SDMA_CTRL_PORT, 0);
		outportw(WS_HYPERV_CTRL_PORT, 0);
		ws_sound_reset();

		print_next_test(msg_test19);
		while (!can_go_to_next_test()) {
			ws_gdma_copy(screen_1, MK_FP(0xF000, 0x0000), sizeof(screen_1));
		}

		if (ws_system_get_model() == WS_MODEL_COLOR) {
			outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_CONTRAST_HIGH | WS_LCD_CTRL_DISPLAY_ENABLE);
			print_next_test(msg_test20);
			while (!can_go_to_next_test()) ia16_halt();
			outportb(WS_LCD_CTRL_PORT, WS_LCD_CTRL_DISPLAY_ENABLE);
		}
	}

	ws_screen_fill_tiles(screen_2, 32, 0, 0, 28, 18);	
	text_puts(screen_2, 0, 0, 9, msg_test_complete);
	while(1);
}
