#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    benchmark_init();
    init_pass_fail();

    int i = 0;
    uint8_t *ptr1 = malloc(64);
    text_printf(screen_1, 0, 0, i, "malloc 64 = %04X", ptr1);
    i++;
    uint8_t *ptr2 = malloc(128);
    text_printf(screen_1, 0, 0, i, "malloc 128 = %04X", ptr2);
    i++;
    uint8_t *ptr3 = malloc(256);
    text_printf(screen_1, 0, 0, i, "malloc 256 = %04X", ptr3);
    i++;
    // with 16K of heap, a 32K allocation should not pass
    uint8_t *ptr4 = malloc(32767);
    text_printf(screen_1, 0, 0, i, "malloc 32767 = %04X", ptr4);
    draw_pass_fail(i, 0, ptr4 == NULL);
    i++;

    memset(ptr1, 1, 64);
    memset(ptr2, 2, 128);
    memset(ptr3, 3, 256);
    draw_pass_fail(i - 2, 1, ptr3[255] == 3);
    draw_pass_fail(i - 2, 0, ptr3[0] == 3);
    draw_pass_fail(i - 3, 1, ptr2[0] == 2);
    draw_pass_fail(i - 3, 0, ptr2[127] == 2);
    draw_pass_fail(i - 4, 1, ptr1[0] == 1);
    draw_pass_fail(i - 4, 0, ptr1[63] == 1);

    free(ptr1);
    text_printf(screen_1, 0, 0, i, "free 64");
    i++;
    uint8_t *ptr5 = malloc(62);
    uint8_t *ptr6 = malloc(2);
    text_printf(screen_1, 0, 0, i, "malloc 62 = %04X", ptr5);
    i++;
    text_printf(screen_1, 0, 0, i, "malloc 2 = %04X", ptr6);
    i++;
    free(ptr2);
    text_printf(screen_1, 0, 0, i, "free 128");
    i++;
    uint8_t *ptr7 = malloc(66);
    uint8_t *ptr8 = malloc(60);
    text_printf(screen_1, 0, 0, i, "malloc 66 = %04X", ptr7);
    i++;
    text_printf(screen_1, 0, 0, i, "malloc 60 = %04X", ptr8);
    i++;

    free(ptr8);
    text_printf(screen_1, 0, 0, i, "free 60");
    i++;
    ptr7 = realloc(ptr7, 62);
    text_printf(screen_1, 0, 0, i, "realloc 66->62 = %04X\n", ptr7);
    i++;
    ptr7 = realloc(ptr7, 90);
    text_printf(screen_1, 0, 0, i, "realloc 62->90 = %04X\n", ptr7);
    i++;
    ptr7 = realloc(ptr7, 480);
    text_printf(screen_1, 0, 0, i, "realloc 90->480 = %04X\n", ptr7);
    i++;

    while(1);
}
