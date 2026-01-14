/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/wdt.h
 *
 * Emulation of AVR watchdog timer for native Linux builds.
 * Provides stub implementations that allow code to compile but don't
 * actually implement watchdog functionality.
 */

#ifndef AVR_WDT_H
#define AVR_WDT_H

#include <stdint.h>

// Watchdog timer prescaler values
#define WDTO_15MS   0
#define WDTO_30MS   1
#define WDTO_60MS   2
#define WDTO_120MS  3
#define WDTO_250MS  4
#define WDTO_500MS  5
#define WDTO_1S     6
#define WDTO_2S     7
#define WDTO_4S     8
#define WDTO_8S     9

// Watchdog timer control register bits
#define WDCE  4
#define WDE   3
#define WDP3  5
#define WDP2  2
#define WDP1  1
#define WDP0  0
#define WDIE  6
#define WDIF  7

// Stub functions for watchdog control
#ifdef __cplusplus
extern "C" {
#endif

// Enable watchdog timer (stub - does nothing on native platform)
static inline void wdt_enable(uint8_t timeout) {
  (void)timeout; // Suppress unused parameter warning
}

// Disable watchdog timer (stub - does nothing on native platform)
static inline void wdt_disable(void) {
}

// Reset watchdog timer (stub - does nothing on native platform)
static inline void wdt_reset(void) {
}

#ifdef __cplusplus
}
#endif

#endif // AVR_WDT_H
