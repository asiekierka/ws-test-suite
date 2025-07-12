// https://github.com/DevSolar/pdclib/blob/master/functions/string/strlen.c

#include <string.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

void strlen_bench4(void) {
    strlen("abcd");
}

void strlen_bench8(void) {
    strlen("abcdabcd");
}

void strlen_bench16(void) {
    strlen("abcdabcdabcdabcd");
}

void strlen_bench32(void) {
    strlen("abcdabcdabcdabcdabcdabcdabcdabcd");
}

void strlen_bench64(void) {
    strlen("abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd");
}

int main(void) {
    benchmark_init();
    init_pass_fail();
    
    int i = 0;
    text_printf(screen_1, 0, 0, i, "strlen");
    draw_pass_fail(i, 1, strlen("abcde") == 5);
    draw_pass_fail(i, 0, strlen("") == 0);
    i++;

    text_printf(screen_1, 0, 0, i++, "strlen(5) = %d", strlen("abcde"));
    text_printf(screen_1, 0, 0, i++, "strlen(0) = %d", strlen(""));
    text_printf(screen_1, 0, 0, i++, "strlen bench4  = %d", benchmark_run(strlen_bench4));
    text_printf(screen_1, 0, 0, i++, "strlen bench8  = %d", benchmark_run(strlen_bench8));
    text_printf(screen_1, 0, 0, i++, "strlen bench16 = %d", benchmark_run(strlen_bench16));
    text_printf(screen_1, 0, 0, i++, "strlen bench32 = %d", benchmark_run(strlen_bench32));
    text_printf(screen_1, 0, 0, i++, "strlen bench64 = %d", benchmark_run(strlen_bench64));
    
    while(1);
}
