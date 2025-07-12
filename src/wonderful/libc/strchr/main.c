// https://github.com/DevSolar/pdclib/blob/master/functions/string/strchr.c

#include <string.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

void strchr_bench4(void) {
    strchr("abcx", 'x');
}

void strchr_bench8(void) {
    strchr("abcdabcx", 'x');
}

void strchr_bench16(void) {
    strchr("abcdabcdabcdabcx", 'x');
}

void strchr_bench32(void) {
    strchr("abcdabcdabcdabcdabcdabcdabcdabcx", 'x');
}

void strchr_bench64(void) {
    strchr("abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcx", 'x');
}

char abccd[] = "abccd";

int main(void) {
    benchmark_init();
    init_pass_fail();
    
    int i = 0;
    text_printf(screen_1, 0, 0, i, "strchr");
    draw_pass_fail(i, 4, strchr( abccd, 'x' ) == NULL);
    draw_pass_fail(i, 3, strchr( abccd, 'a' ) == &abccd[0]);
    draw_pass_fail(i, 2, strchr( abccd, 'd' ) == &abccd[4]);
    draw_pass_fail(i, 1, strchr( abccd, '\0' ) == &abccd[5]);
    draw_pass_fail(i, 0, strchr( abccd, 'c' ) == &abccd[2]);
    i++;

    text_printf(screen_1, 0, 0, i++, "strchr bench4  = %d", benchmark_run(strchr_bench4));
    text_printf(screen_1, 0, 0, i++, "strchr bench8  = %d", benchmark_run(strchr_bench8));
    text_printf(screen_1, 0, 0, i++, "strchr bench16 = %d", benchmark_run(strchr_bench16));
    text_printf(screen_1, 0, 0, i++, "strchr bench32 = %d", benchmark_run(strchr_bench32));
    text_printf(screen_1, 0, 0, i++, "strchr bench64 = %d", benchmark_run(strchr_bench64));
    
    while(1);
}
