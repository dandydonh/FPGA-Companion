#ifndef COMPANION_STATUS_H
#define COMPANION_STATUS_H
#include <stdbool.h>
#include <stdint.h>
#ifdef CONFIG_COMPANION_STATUS
void companion_status_start(void);
void companion_status_controller_result(int result);
void companion_status_stopping(void);
void companion_status_keyboard(unsigned index, bool present);
/* Platform sends one frame under the shared lock, including 1 us idle on
 * each side. False means lock busy; never interleave or steal ownership. */
bool mcu_hw_status_send(const uint8_t *frame, unsigned length);
#else
static inline void companion_status_start(void) {}
static inline void companion_status_controller_result(int result) {(void)result;}
static inline void companion_status_stopping(void) {}
static inline void companion_status_keyboard(unsigned index, bool present) {(void)index;(void)present;}
#endif
#endif
