/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file avr/boot.h
 *
 * Bootloader support for AVR emulation.
 * Provides stubs for bootloader-related functions.
 */

#ifndef AVR_BOOT_H
#define AVR_BOOT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bootloader functions (stubs - do nothing on native platform)
static inline void boot_rww_enable(void) {
  // Enable read-while-write section (stub)
}

static inline void boot_spm_busy_wait(void) {
  // Wait for SPM operation to complete (stub)
}

// Signature row access (stub - returns dummy values)
static inline uint8_t boot_signature_byte_get(uint16_t addr) {
  (void)addr;
  return 0xFF;
}

#ifdef __cplusplus
}
#endif

#endif // AVR_BOOT_H
