#ifndef _TEXT_H_
#define _TEXT_H_

#include <wonderful.h>

void text_init(void);
void text_puts(uint16_t __wf_iram *dest, uint16_t tile, uint16_t x, uint16_t y, const char __far* text);

__attribute__((format(printf, 5, 6)))
void text_printf(uint16_t __wf_iram *dest, uint16_t tile, uint16_t x, uint16_t y, const char __far* format, ...);

#endif /* _TEXT_H_ */
