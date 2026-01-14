/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/eeprom.h
 *
 * Emulation of AVR EEPROM for native Linux builds.
 * Uses a file-backed implementation to persist data between runs.
 */

#ifndef AVR_EEPROM_H
#define AVR_EEPROM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// EEPROM size for emulation (match typical AVR sizes)
#ifndef E2END
#define E2END 1023  // 1KB EEPROM (can be overridden)
#endif

// Read a byte from EEPROM
uint8_t eeprom_read_byte(const uint8_t *addr);

// Write a byte to EEPROM
void eeprom_write_byte(uint8_t *addr, uint8_t value);

// Read a word (16-bit) from EEPROM
uint16_t eeprom_read_word(const uint16_t *addr);

// Write a word (16-bit) to EEPROM
void eeprom_write_word(uint16_t *addr, uint16_t value);

// Read a dword (32-bit) from EEPROM
uint32_t eeprom_read_dword(const uint32_t *addr);

// Write a dword (32-bit) to EEPROM
void eeprom_write_dword(uint32_t *addr, uint32_t value);

// Read a block of data from EEPROM
void eeprom_read_block(void *dst, const void *src, size_t n);

// Write a block of data to EEPROM
void eeprom_write_block(const void *src, void *dst, size_t n);

// Update a byte in EEPROM (only write if different)
void eeprom_update_byte(uint8_t *addr, uint8_t value);

// Update a word in EEPROM (only write if different)
void eeprom_update_word(uint16_t *addr, uint16_t value);

// Update a dword in EEPROM (only write if different)
void eeprom_update_dword(uint32_t *addr, uint32_t value);

// Update a block in EEPROM (only write bytes that differ)
void eeprom_update_block(const void *src, void *dst, size_t n);

// Check if EEPROM is ready (always true for emulation)
static inline uint8_t eeprom_is_ready(void) {
  return 1;
}

#ifdef __cplusplus
}
#endif

#endif // AVR_EEPROM_H
