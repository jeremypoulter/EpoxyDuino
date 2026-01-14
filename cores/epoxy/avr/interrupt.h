/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/interrupt.h
 *
 * Emulation of AVR interrupt handling for native Linux builds.
 * Provides stub implementations for interrupt control functions.
 */

#ifndef AVR_INTERRUPT_H
#define AVR_INTERRUPT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Status register - stub implementation
extern volatile uint8_t _epoxy_sreg;
#define SREG _epoxy_sreg

// Interrupt enable/disable functions (stubs - do nothing on native platform)
static inline void cli(void) {
  // Disable global interrupts (stub)
}

static inline void sei(void) {
  // Enable global interrupts (stub)
}

// ISR macro for defining interrupt service routines (not used in native builds)
#define ISR(vector) void vector(void)

// Interrupt vector names (stubs)
#define TIMER0_OVF_vect     void
#define TIMER1_OVF_vect     void
#define TIMER2_OVF_vect     void
#define INT0_vect           void
#define INT1_vect           void
#define PCINT0_vect         void
#define PCINT1_vect         void
#define PCINT2_vect         void

#ifdef __cplusplus
}
#endif

#endif // AVR_INTERRUPT_H
