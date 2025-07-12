#include <string.h>
#include <ws.h>
#include "text.h"

char s[] = "xxxxxxxxxxx";
const char abcde[] = "abcde";

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    
    int i = 0;
    for (int k = 0; k < sizeof(s); k++) s[k] = 'x';
    text_printf(screen_1, 0, 0, i, "memccpy");
    draw_pass_fail(i, 7, memccpy( s, abcde, 'x', 5 ) == NULL);
    draw_pass_fail(i, 6, s[0] == 'a');
    draw_pass_fail(i, 5, s[4] == 'e');
    draw_pass_fail(i, 4, s[5] == 'x');
    draw_pass_fail(i, 3, memccpy( s + 4, abcde, 'c', 5 ) == s + 7);
    draw_pass_fail(i, 2, s[4] == 'a');
    draw_pass_fail(i, 1, s[6] == 'c');
    draw_pass_fail(i, 0, s[7] == 'x');
    i++;
    
    while(1);
}
