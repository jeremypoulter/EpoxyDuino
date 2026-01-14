/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/eeprom.cpp
 *
 * Implementation of AVR EEPROM emulation using file-backed storage.
 */

#include "avr/eeprom.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// EEPROM storage size
#define EEPROM_SIZE (E2END + 1)

// Static buffer for EEPROM data
static uint8_t eeprom_data[EEPROM_SIZE];
static bool eeprom_initialized = false;
static const char* eeprom_file = "epoxyeepromdata";

// Initialize EEPROM from file
static void eeprom_init() {
  if (eeprom_initialized) return;
  
  // Try to load from file
  FILE* f = fopen(eeprom_file, "rb");
  if (f) {
    size_t bytes_read = fread(eeprom_data, 1, EEPROM_SIZE, f);
    fclose(f);
    // Fill remaining bytes with 0xFF (default EEPROM state)
    if (bytes_read < EEPROM_SIZE) {
      memset(eeprom_data + bytes_read, 0xFF, EEPROM_SIZE - bytes_read);
    }
  } else {
    // Initialize with 0xFF (default EEPROM state)
    memset(eeprom_data, 0xFF, EEPROM_SIZE);
  }
  
  eeprom_initialized = true;
}

// Save EEPROM to file
static void eeprom_save() {
  FILE* f = fopen(eeprom_file, "wb");
  if (f) {
    fwrite(eeprom_data, 1, EEPROM_SIZE, f);
    fclose(f);
  }
}

// Convert pointer to offset
static size_t ptr_to_offset(const void* ptr) {
  return (size_t)ptr;
}

extern "C" {

uint8_t eeprom_read_byte(const uint8_t *addr) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset >= EEPROM_SIZE) return 0xFF;
  return eeprom_data[offset];
}

void eeprom_write_byte(uint8_t *addr, uint8_t value) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset >= EEPROM_SIZE) return;
  eeprom_data[offset] = value;
  eeprom_save();
}

uint16_t eeprom_read_word(const uint16_t *addr) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset + 1 >= EEPROM_SIZE) return 0xFFFF;
  return (uint16_t)eeprom_data[offset] | ((uint16_t)eeprom_data[offset + 1] << 8);
}

void eeprom_write_word(uint16_t *addr, uint16_t value) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset + 1 >= EEPROM_SIZE) return;
  eeprom_data[offset] = (uint8_t)(value & 0xFF);
  eeprom_data[offset + 1] = (uint8_t)((value >> 8) & 0xFF);
  eeprom_save();
}

uint32_t eeprom_read_dword(const uint32_t *addr) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset + 3 >= EEPROM_SIZE) return 0xFFFFFFFF;
  return (uint32_t)eeprom_data[offset] |
         ((uint32_t)eeprom_data[offset + 1] << 8) |
         ((uint32_t)eeprom_data[offset + 2] << 16) |
         ((uint32_t)eeprom_data[offset + 3] << 24);
}

void eeprom_write_dword(uint32_t *addr, uint32_t value) {
  eeprom_init();
  size_t offset = ptr_to_offset(addr);
  if (offset + 3 >= EEPROM_SIZE) return;
  eeprom_data[offset] = (uint8_t)(value & 0xFF);
  eeprom_data[offset + 1] = (uint8_t)((value >> 8) & 0xFF);
  eeprom_data[offset + 2] = (uint8_t)((value >> 16) & 0xFF);
  eeprom_data[offset + 3] = (uint8_t)((value >> 24) & 0xFF);
  eeprom_save();
}

void eeprom_read_block(void *dst, const void *src, size_t n) {
  eeprom_init();
  size_t offset = ptr_to_offset(src);
  if (offset >= EEPROM_SIZE) return;
  if (offset + n > EEPROM_SIZE) {
    n = EEPROM_SIZE - offset;
  }
  memcpy(dst, eeprom_data + offset, n);
}

void eeprom_write_block(const void *src, void *dst, size_t n) {
  eeprom_init();
  size_t offset = ptr_to_offset(dst);
  if (offset >= EEPROM_SIZE) return;
  if (offset + n > EEPROM_SIZE) {
    n = EEPROM_SIZE - offset;
  }
  memcpy(eeprom_data + offset, src, n);
  eeprom_save();
}

void eeprom_update_byte(uint8_t *addr, uint8_t value) {
  uint8_t current = eeprom_read_byte(addr);
  if (current != value) {
    eeprom_write_byte(addr, value);
  }
}

void eeprom_update_word(uint16_t *addr, uint16_t value) {
  uint16_t current = eeprom_read_word(addr);
  if (current != value) {
    eeprom_write_word(addr, value);
  }
}

void eeprom_update_dword(uint32_t *addr, uint32_t value) {
  uint32_t current = eeprom_read_dword(addr);
  if (current != value) {
    eeprom_write_dword(addr, value);
  }
}

void eeprom_update_block(const void *src, void *dst, size_t n) {
  eeprom_init();
  size_t offset = ptr_to_offset(dst);
  if (offset >= EEPROM_SIZE) return;
  if (offset + n > EEPROM_SIZE) {
    n = EEPROM_SIZE - offset;
  }
  
  const uint8_t* src_bytes = (const uint8_t*)src;
  bool changed = false;
  
  for (size_t i = 0; i < n; i++) {
    if (eeprom_data[offset + i] != src_bytes[i]) {
      eeprom_data[offset + i] = src_bytes[i];
      changed = true;
    }
  }
  
  if (changed) {
    eeprom_save();
  }
}

} // extern "C"
