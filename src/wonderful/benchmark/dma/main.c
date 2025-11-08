#define COLOR

#include <string.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];
__attribute__((section(".iramx_4000")))
uint8_t dst_data[16384];

#include "test/pass_fail.h"

static const uint8_t __far data[16384] = {0};
uint16_t copy_length = 1;

void b_dma_copyp(void) {
    ws_gdma_copyp(dst_data, data, copy_length);
}

void b_dma_copyi(void) {
    ws_gdma_copyi(dst_data, 0x80000, copy_length);
}

void b_memcpy(void) {
    memcpy(dst_data, data, copy_length);
}

int main(void) {
    benchmark_init();
    init_pass_fail();

    int i = 0;
    text_printf(screen_1, 0, 0, i++,
    //123456789012345678901234567
    "length dma_p  dma_i  memcpy");

    for (; copy_length <= 8192; copy_length <<= 1, i++) {
    	text_printf(screen_1, 0, 0, i, "%d", copy_length);
    	text_printf(screen_1, 0, 7, i, "%d", benchmark_run(b_dma_copyp));
    	text_printf(screen_1, 0, 14, i, "%d", benchmark_run(b_dma_copyi));
    	text_printf(screen_1, 0, 21, i, "%d", benchmark_run(b_memcpy));
    }
    while(1);
}
