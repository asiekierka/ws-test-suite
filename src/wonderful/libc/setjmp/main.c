#include <setjmp.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

jmp_buf buf;

void func_1(void) {
    longjmp(buf, 1);
}

int main(void) {
    init_pass_fail();

    int i = setjmp(buf);
    text_printf(screen_1, 0, 0, i, "longjmp %d", i);
    draw_pass_fail(i, 0, true);
    if (i == 0) func_1();
    if (i == 1) longjmp(buf, 2);

    while(1);
}
