#pragma once

// Minimal host-build shim for ESP-IDF random API used by native builds.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <random>

#ifdef __cplusplus
extern "C" {
#endif

static inline void esp_fill_random(void* buf, size_t len) {
  if (!buf || len == 0) return;

  static std::random_device rd;
  uint8_t* out = static_cast<uint8_t*>(buf);

  while (len >= sizeof(uint32_t)) {
    uint32_t value = static_cast<uint32_t>(rd());
    memcpy(out, &value, sizeof(value));
    out += sizeof(value);
    len -= sizeof(value);
  }

  if (len > 0) {
    uint32_t value = static_cast<uint32_t>(rd());
    memcpy(out, &value, len);
  }
}

static inline uint32_t esp_random(void) {
  uint32_t value = 0;
  esp_fill_random(&value, sizeof(value));
  return value;
}

#ifdef __cplusplus
}
#endif
