#ifndef __IA16_CMODEL_TINY__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include <ws/hardware.h>
#include <ws/keypad.h>
#include "key.h"
#include "main.h"
#include "text.h"

/* BENCHMARK SUBSYSTEM */

static const char __wf_rom msg_bench_type_0[] = "NULL";
static const char __wf_rom msg_bench_type_1[] = "NULL IRAM";
static const char __wf_rom msg_bench_type_2[] = "MEMORY R";
static const char __wf_rom msg_bench_type_3[] = "MEMORY W";
static const char __wf_rom msg_bench_type_4[] = "PORT R";
static const char __wf_rom msg_bench_type_5[] = "PORT W";
static const char __wf_rom msg_bench_type_6[] = "DMA>7000+";
static const char __wf_rom msg_bench_type_7[] = "DMA>7000-";

static const char __wf_rom* const __wf_rom msg_bench_types[] = {
    msg_bench_type_0,
    msg_bench_type_1,
    msg_bench_type_2,
    msg_bench_type_3,
    msg_bench_type_4,
    msg_bench_type_5,
    msg_bench_type_6,
    msg_bench_type_7
};
#define BENCH_TYPES 8

static const char __wf_rom msg_bench_start[] = "START";

extern uint16_t run_benchmark_null(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_null_iram(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_read_byte(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_read_word(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_write_byte(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_write_word(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_io_read_byte(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_io_read_word(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_io_write_byte(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_io_write_word(uint16_t arg1, uint16_t arg2) __far;
extern uint16_t run_benchmark_dma(uint16_t arg1, uint16_t arg2, uint8_t ctrl) __far;

static const char __wf_rom msg_bench_result[] = "%d.%02X";
static const char __wf_rom msg_bench_result_raw[] = "%d";

static void draw_benchmark_result(uint16_t result, int y) {
    text_printf(SCREEN_1, 0, 1, y, msg_bench_result, (result >> 5), ((result << 3) & 0xF));
}

static void draw_benchmark_result_raw(uint16_t result, int y) {
    text_printf(SCREEN_1, 0, 1, y, msg_bench_result_raw, result);
}

void subsystem_benchmark(void) {
    static uint16_t arg1 = 0, arg2 = 0;
    static uint8_t selected_part = 0, bench_type = 0;
    bool selected = true;

    if (subsystem_redraw) {
        draw_hex(arg1, 4, true, selected_part - 0, 1, 1);
        draw_hex(arg2, 4, true, selected_part - 4, 6, 1);
        ws_screen_fill_tiles(SCREEN_1, 0x20, 11, 1, 11, 1);
        text_puts(SCREEN_1, HIGHLIGHT(selected_part == 8), 11, 1, msg_bench_types[bench_type]);
        text_puts(SCREEN_1, HIGHLIGHT(selected_part == 9), 22, 1, msg_bench_start);
        subsystem_redraw = false;
    }

    if (keys_pressed != 0) {
        subsystem_redraw = true;

        if ((keys_pressed & KEY_X1) || ((keys_pressed & KEY_A) && selected_part != 9)) {
            if (selected_part < 4) {
                arg1 = modify_hex4(arg1, 12 - ((selected_part & 3) * 4), 1);
            } else if (selected_part < 8) {
                arg2 = modify_hex4(arg2, 12 - ((selected_part & 3) * 4), 1);
            } else if (selected_part == 8) {
                if (bench_type < (BENCH_TYPES - 1)) {
                    bench_type++;
                } else {
                    bench_type = 0;
                }
            }
        }        
        if (keys_pressed & KEY_X3) {
            if (selected_part < 4) {
                arg1 = modify_hex4(arg1, 12 - ((selected_part & 3) * 4), -1);
            } else if (selected_part < 8) {
                arg2 = modify_hex4(arg2, 12 - ((selected_part & 3) * 4), -1);
            } else if (selected_part == 8) {
                if (bench_type > 0) {
                    bench_type--;
                } else {
                    bench_type = BENCH_TYPES - 1;
                }
            }
        }
        if (keys_pressed & KEY_X4 && selected_part > 0) {
            selected_part--;
        }
        if (keys_pressed & KEY_X2 && selected_part < 9) {
            selected_part++;
        }
        if (keys_pressed & KEY_B) {
            arg1 = 0;
            arg2 = 0;
        }
        if ((keys_pressed & KEY_A) && selected_part == 9) {
            ws_screen_fill_tiles(SCREEN_1, 0x20, 1, 2, 26, 15);
            int i = 3;
            switch (bench_type) {
            case 0:
                draw_benchmark_result(run_benchmark_null(arg1, arg2), i++);
                break;
            case 1:
                draw_benchmark_result(run_benchmark_null_iram(arg1, arg2), i++);
                break;
            case 2:
                draw_benchmark_result(run_benchmark_read_byte(arg1, arg2), i++);
                draw_benchmark_result(run_benchmark_read_word(arg1, arg2), i++);
                break;
            case 3:
                draw_benchmark_result(run_benchmark_write_byte(arg1, arg2), i++);
                draw_benchmark_result(run_benchmark_write_word(arg1, arg2), i++);
                break;
            case 4:
                draw_benchmark_result(run_benchmark_io_read_byte(arg2, arg1), i++);
                draw_benchmark_result(run_benchmark_io_read_word(arg2, arg1), i++);
                break;
            case 5:
                draw_benchmark_result(run_benchmark_io_write_byte(arg2, arg1), i++);
                draw_benchmark_result(run_benchmark_io_write_word(arg2, arg1), i++);
                break;
            case 6:
                draw_benchmark_result_raw(run_benchmark_dma(arg1, arg2, DMA_TRANSFER_ENABLE | DMA_ADDRESS_INC), i++);
                break;
            case 7:
                draw_benchmark_result_raw(run_benchmark_dma(arg1, arg2, DMA_TRANSFER_ENABLE | DMA_ADDRESS_DEC), i++);
                break;
            }
        }
    }
}

#endif
