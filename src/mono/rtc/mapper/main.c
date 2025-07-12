#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/util.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_cmd[] = "Command 0x%02X:";
static const char __wf_rom msg_d[] = "%d";
static const uint8_t __wf_rom cmd_lengths[] = {1, 1, 1, 1, 7, 7, 3, 3, 2, 2, 2, 2, 0, 0, 0, 0};
static const char __wf_rom msg_ready_cleared_on_new[] = "Ready cleared on new:";

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();

    cpu_irq_disable();

    int i;
    for (i = 0; i < 13; i++) {
        text_printf(screen_1, 0, 0, i, msg_cmd, 0x10 + i);

        uint8_t bytes = 0;
        uint16_t timeout = 0;
        if (i & 1) {
            // read command
            outportb(0xCB, 0x00);
            outportb(0xCA, 0x10 + i);
            while (--timeout) {
                uint8_t status = inportb(0xCA);
                if (!(status & 0x90)) break; // not ready. not busy - done
                if (status & 0x80) { // ready - can read byte
                    inportb(0xCB);
                    bytes++;
                }
                if (status & 0x10) continue;
                break;
            }
        } else {
            // write command
            outportb(0xCB, 0x00);
            outportb(0xCA, 0x10 + i);
            bytes++;
            while (--timeout) {
                uint8_t status = inportb(0xCA);
                if (!(status & 0x90)) break; // not ready. not busy - done
                if (status & 0x80) { // ready - can write byte
                    outportb(0xCB, 0x00);
                    if (status & 0x10) bytes++;
                }
                if (status & 0x10) continue;
                break;
            }
        }

        if (i >= 0xC) {
            draw_pass_fail(i, 1, !timeout);
        } else {
            text_printf(screen_1, 0, 28 - 3, i, msg_d, (int) bytes);
            draw_pass_fail(i, 1, timeout);
            draw_pass_fail(i, 0, bytes == cmd_lengths[i]);
        }
    }

    outportb(0xCA, 0x10);
    text_puts(screen_1, 0, 0, i, msg_ready_cleared_on_new);
    while (inportb(0xCA) & 0x10);
    draw_pass_fail(i, 1, inportb(0xCA) & 0x80);
    outportb(0xCA, 0x93); // set ready, should not work
    volatile uint8_t status_on_new = inportb(0xCA);
    draw_pass_fail(i, 0, !(inportb(0xCA) & 0x80));

    while(1);
}
