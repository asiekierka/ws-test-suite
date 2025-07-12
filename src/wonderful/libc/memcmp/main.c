// https://github.com/DevSolar/pdclib/blob/master/functions/string/memcmp.c

#include <string.h>
#include <ws.h>
#include "text.h"

const char abcde[] = "abcde";
const char abcdx[] = "abcdx";
const char xxxxx[] = "xxxxx";

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    
    int i = 0;
    text_printf(screen_1, 0, 0, i, "_fmemcmp");
    draw_pass_fail(i, 4, _fmemcmp( abcde, abcdx, 5 ) < 0);
    draw_pass_fail(i, 3, _fmemcmp( abcde, abcdx, 4 ) == 0);
    draw_pass_fail(i, 2, _fmemcmp( abcde, xxxxx, 0 ) == 0);
    draw_pass_fail(i, 1, _fmemcmp( abcde, abcde, 5 ) == 0);
    draw_pass_fail(i, 0, _fmemcmp( xxxxx, abcde, 1 ) > 0);
    i++;
    
    text_printf(screen_1, 0, 0, i, "_nmemcmp");
    draw_pass_fail(i, 4, _nmemcmp( abcde, abcdx, 5 ) < 0);
    draw_pass_fail(i, 3, _nmemcmp( abcde, abcdx, 4 ) == 0);
    draw_pass_fail(i, 2, _nmemcmp( abcde, xxxxx, 0 ) == 0);
    draw_pass_fail(i, 1, _nmemcmp( abcde, abcde, 5 ) == 0);
    draw_pass_fail(i, 0, _nmemcmp( xxxxx, abcde, 1 ) > 0);
    i++;

    while(1);
}
