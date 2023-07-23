#ifndef __KEY_H__
#define __KEY_H__

#include <wonderful.h>

extern uint16_t keys_pressed;
extern uint16_t keys_released;
extern uint16_t keys_held;

void key_update(void);

#endif /* __KEY_H__ */
