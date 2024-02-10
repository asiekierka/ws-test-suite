#include "benchmark.h"
#include <ws/hardware.h>

static void noop(void) {}

static uint16_t benchmark_overhead = 0;

void benchmark_init(void) {
    benchmark_overhead = benchmark_run(noop);
}

uint16_t benchmark_run(void(*run)()) {
    uint8_t i = 0;

    outportw(IO_HBLANK_TIMER, 65535);
    outportb(IO_TIMER_CTRL, HBLANK_TIMER_ONESHOT | HBLANK_TIMER_ENABLE);

    while (--i) {
        run();
    }

    outportb(IO_TIMER_CTRL, 0);
    return (inportw(IO_HBLANK_COUNTER) ^ 0xFFFF) - benchmark_overhead;
}
