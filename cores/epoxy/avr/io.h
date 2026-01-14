/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/io.h
 *
 * Emulation of AVR I/O registers for native Linux builds.
 * Provides stub implementations for port and register access.
 */

#ifndef AVR_IO_H
#define AVR_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// AVR I/O port emulation
// We provide stub registers for common AVR ports

// Port B registers
extern volatile uint8_t _epoxy_PINB;
extern volatile uint8_t _epoxy_DDRB;
extern volatile uint8_t _epoxy_PORTB;
#define PINB  _epoxy_PINB
#define DDRB  _epoxy_DDRB
#define PORTB _epoxy_PORTB

// Port C registers
extern volatile uint8_t _epoxy_PINC;
extern volatile uint8_t _epoxy_DDRC;
extern volatile uint8_t _epoxy_PORTC;
#define PINC  _epoxy_PINC
#define DDRC  _epoxy_DDRC
#define PORTC _epoxy_PORTC

// Port D registers
extern volatile uint8_t _epoxy_PIND;
extern volatile uint8_t _epoxy_DDRD;
extern volatile uint8_t _epoxy_PORTD;
#define PIND  _epoxy_PIND
#define DDRD  _epoxy_DDRD
#define PORTD _epoxy_PORTD

// Additional ports for larger AVR chips
#ifdef AVR_LARGE_PORTS
extern volatile uint8_t _epoxy_PINA;
extern volatile uint8_t _epoxy_DDRA;
extern volatile uint8_t _epoxy_PORTA;
#define PINA  _epoxy_PINA
#define DDRA  _epoxy_DDRA
#define PORTA _epoxy_PORTA

extern volatile uint8_t _epoxy_PINE;
extern volatile uint8_t _epoxy_DDRE;
extern volatile uint8_t _epoxy_PORTE;
#define PINE  _epoxy_PINE
#define DDRE  _epoxy_DDRE
#define PORTE _epoxy_PORTE

extern volatile uint8_t _epoxy_PINF;
extern volatile uint8_t _epoxy_DDRF;
extern volatile uint8_t _epoxy_PORTF;
#define PINF  _epoxy_PINF
#define DDRF  _epoxy_DDRF
#define PORTF _epoxy_PORTF
#endif

// Timer/Counter registers (stubs)
extern volatile uint8_t _epoxy_TCCR0A;
extern volatile uint8_t _epoxy_TCCR0B;
extern volatile uint8_t _epoxy_TCNT0;
extern volatile uint8_t _epoxy_OCR0A;
extern volatile uint8_t _epoxy_OCR0B;
extern volatile uint8_t _epoxy_TIMSK0;
extern volatile uint8_t _epoxy_TIFR0;

#define TCCR0A  _epoxy_TCCR0A
#define TCCR0B  _epoxy_TCCR0B
#define TCNT0   _epoxy_TCNT0
#define OCR0A   _epoxy_OCR0A
#define OCR0B   _epoxy_OCR0B
#define TIMSK0  _epoxy_TIMSK0
#define TIFR0   _epoxy_TIFR0

extern volatile uint8_t _epoxy_TCCR1A;
extern volatile uint8_t _epoxy_TCCR1B;
extern volatile uint8_t _epoxy_TCCR1C;
extern volatile uint16_t _epoxy_TCNT1;
extern volatile uint16_t _epoxy_OCR1A;
extern volatile uint16_t _epoxy_OCR1B;
extern volatile uint16_t _epoxy_ICR1;
extern volatile uint8_t _epoxy_TIMSK1;
extern volatile uint8_t _epoxy_TIFR1;

#define TCCR1A  _epoxy_TCCR1A
#define TCCR1B  _epoxy_TCCR1B
#define TCCR1C  _epoxy_TCCR1C
#define TCNT1   _epoxy_TCNT1
#define OCR1A   _epoxy_OCR1A
#define OCR1B   _epoxy_OCR1B
#define ICR1    _epoxy_ICR1
#define TIMSK1  _epoxy_TIMSK1
#define TIFR1   _epoxy_TIFR1

// Timer1 control register bits
#define WGM10   0
#define WGM11   1
#define WGM12   3
#define WGM13   4
#define COM1A0  6
#define COM1A1  7
#define COM1B0  4
#define COM1B1  5
#define CS10    0
#define CS11    1
#define CS12    2

// Timer2 PWM pin (AVR port mapping)
#define PORTB2  2

extern volatile uint8_t _epoxy_TCCR2A;
extern volatile uint8_t _epoxy_TCCR2B;
extern volatile uint8_t _epoxy_TCNT2;
extern volatile uint8_t _epoxy_OCR2A;
extern volatile uint8_t _epoxy_OCR2B;
extern volatile uint8_t _epoxy_TIMSK2;
extern volatile uint8_t _epoxy_TIFR2;

#define TCCR2A  _epoxy_TCCR2A
#define TCCR2B  _epoxy_TCCR2B
#define TCNT2   _epoxy_TCNT2
#define OCR2A   _epoxy_OCR2A
#define OCR2B   _epoxy_OCR2B
#define TIMSK2  _epoxy_TIMSK2
#define TIFR2   _epoxy_TIFR2

// ADC registers (stubs)
extern volatile uint8_t _epoxy_ADMUX;
extern volatile uint8_t _epoxy_ADCSRA;
extern volatile uint8_t _epoxy_ADCSRB;
extern volatile uint16_t _epoxy_ADC;

#define ADMUX   _epoxy_ADMUX
#define ADCSRA  _epoxy_ADCSRA
#define ADCSRB  _epoxy_ADCSRB
#define ADC     _epoxy_ADC

// TWI/I2C registers (stubs)
extern volatile uint8_t _epoxy_TWBR;
extern volatile uint8_t _epoxy_TWSR;
extern volatile uint8_t _epoxy_TWAR;
extern volatile uint8_t _epoxy_TWDR;
extern volatile uint8_t _epoxy_TWCR;
extern volatile uint8_t _epoxy_TWAMR;

#define TWBR  _epoxy_TWBR
#define TWSR  _epoxy_TWSR
#define TWAR  _epoxy_TWAR
#define TWDR  _epoxy_TWDR
#define TWCR  _epoxy_TWCR
#define TWAMR _epoxy_TWAMR

// MCU Status Register
extern volatile uint8_t _epoxy_MCUSR;
#define MCUSR _epoxy_MCUSR

// MCUSR bits
#define PORF   0  // Power-on reset flag
#define EXTRF  1  // External reset flag  
#define BORF   2  // Brown-out reset flag
#define WDRF   3  // Watchdog reset flag

// CPU frequency (default to 16MHz like Arduino Uno)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Common bit definitions
#define _BV(bit) (1 << (bit))

// Special function register access macros
#define _SFR_BYTE(addr) (addr)

// Pin number definitions for Port B
#define PB0 0
#define PB1 1
#define PB2 2
#define PB3 3
#define PB4 4
#define PB5 5
#define PB6 6
#define PB7 7

// Pin number definitions for Port C
#define PC0 0
#define PC1 1
#define PC2 2
#define PC3 3
#define PC4 4
#define PC5 5
#define PC6 6
#define PC7 7

// Pin number definitions for Port D
#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7

// Power management (stubs - do nothing)
#define sleep_enable()
#define sleep_disable()
#define sleep_cpu()

#ifdef __cplusplus
}
#endif

#endif // AVR_IO_H
