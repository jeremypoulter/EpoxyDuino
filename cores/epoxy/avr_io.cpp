/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr_io.cpp
 *
 * Implementation of AVR I/O register emulation.
 * Defines all the stub registers declared in avr/io.h and avr/interrupt.h
 */

#include "avr/io.h"
#include "avr/interrupt.h"

// Status register from interrupt.h
volatile uint8_t _epoxy_sreg = 0;

// Port B registers
volatile uint8_t _epoxy_PINB = 0;
volatile uint8_t _epoxy_DDRB = 0;
volatile uint8_t _epoxy_PORTB = 0;

// Port C registers
volatile uint8_t _epoxy_PINC = 0;
volatile uint8_t _epoxy_DDRC = 0;
volatile uint8_t _epoxy_PORTC = 0;

// Port D registers
volatile uint8_t _epoxy_PIND = 0;
volatile uint8_t _epoxy_DDRD = 0;
volatile uint8_t _epoxy_PORTD = 0;

#ifdef AVR_LARGE_PORTS
// Port A registers
volatile uint8_t _epoxy_PINA = 0;
volatile uint8_t _epoxy_DDRA = 0;
volatile uint8_t _epoxy_PORTA = 0;

// Port E registers
volatile uint8_t _epoxy_PINE = 0;
volatile uint8_t _epoxy_DDRE = 0;
volatile uint8_t _epoxy_PORTE = 0;

// Port F registers
volatile uint8_t _epoxy_PINF = 0;
volatile uint8_t _epoxy_DDRF = 0;
volatile uint8_t _epoxy_PORTF = 0;
#endif

// Timer 0 registers
volatile uint8_t _epoxy_TCCR0A = 0;
volatile uint8_t _epoxy_TCCR0B = 0;
volatile uint8_t _epoxy_TCNT0 = 0;
volatile uint8_t _epoxy_OCR0A = 0;
volatile uint8_t _epoxy_OCR0B = 0;
volatile uint8_t _epoxy_TIMSK0 = 0;
volatile uint8_t _epoxy_TIFR0 = 0;

// Timer 1 registers
volatile uint8_t _epoxy_TCCR1A = 0;
volatile uint8_t _epoxy_TCCR1B = 0;
volatile uint8_t _epoxy_TCCR1C = 0;
volatile uint16_t _epoxy_TCNT1 = 0;
volatile uint16_t _epoxy_OCR1A = 0;
volatile uint16_t _epoxy_OCR1B = 0;
volatile uint16_t _epoxy_ICR1 = 0;
volatile uint8_t _epoxy_TIMSK1 = 0;
volatile uint8_t _epoxy_TIFR1 = 0;

// Timer 2 registers
volatile uint8_t _epoxy_TCCR2A = 0;
volatile uint8_t _epoxy_TCCR2B = 0;
volatile uint8_t _epoxy_TCNT2 = 0;
volatile uint8_t _epoxy_OCR2A = 0;
volatile uint8_t _epoxy_OCR2B = 0;
volatile uint8_t _epoxy_TIMSK2 = 0;
volatile uint8_t _epoxy_TIFR2 = 0;

// ADC registers
volatile uint8_t _epoxy_ADMUX = 0;
volatile uint8_t _epoxy_ADCSRA = 0;
volatile uint8_t _epoxy_ADCSRB = 0;
volatile uint16_t _epoxy_ADC = 0;

// TWI/I2C registers
volatile uint8_t _epoxy_TWBR = 0;
volatile uint8_t _epoxy_TWSR = 0;
volatile uint8_t _epoxy_TWAR = 0;
volatile uint8_t _epoxy_TWDR = 0;
volatile uint8_t _epoxy_TWCR = 0;
volatile uint8_t _epoxy_TWAMR = 0;

// MCU Status Register
volatile uint8_t _epoxy_MCUSR = 0;
