/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file wiring_private.h
 *
 * Private definitions for Arduino wiring functions.
 * Provides stubs for internal Arduino definitions.
 */

#ifndef WIRING_PRIVATE_H
#define WIRING_PRIVATE_H

#include <avr/io.h>
#include "Arduino.h"

// ADC reference modes
#ifndef DEFAULT
#define DEFAULT 1
#endif

#ifndef EXTERNAL
#define EXTERNAL 0
#endif

#ifndef INTERNAL
#define INTERNAL 3
#endif

// Timer prescaler bits
#define TIMER0_PRESCALE_VALUE 64
#define TIMER1_PRESCALE_VALUE 64
#define TIMER2_PRESCALE_VALUE 64

// Macros for manipulating special function registers
#ifndef _SFR_BYTE
#define _SFR_BYTE(addr) (*(volatile uint8_t*)(addr))
#endif

// Macros for timer control
#ifndef sbi
#define sbi(sfr, bit) (sfr |= _BV(bit))
#endif

#ifndef cbi
#define cbi(sfr, bit) (sfr &= ~_BV(bit))
#endif

#endif // WIRING_PRIVATE_H
