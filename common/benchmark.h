#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <stdint.h>
#include <wonderful.h>

void benchmark_init(void);
uint16_t benchmark_run(void(*run)());

#endif /* _TEXT_H_ */
