#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  size_t size;
} esp_partition_t;

// Native host builds do not model OTA partitions. Return a stub descriptor so
// callers can safely read a nominal size value.
static inline const esp_partition_t* esp_ota_get_running_partition(void) {
  static const esp_partition_t partition = {0};
  return &partition;
}

#ifdef __cplusplus
}
#endif
