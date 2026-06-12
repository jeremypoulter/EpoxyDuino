#pragma once

// Minimal host-build shim for ESP-IDF netif DNS APIs used by ArduinoMongoose.
// Used only for PlatformIO `native` builds.

#include <stdint.h>

#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) \
  (((major) * 10000) + ((minor) * 100) + (patch))
#endif

#ifndef ESP_ARDUINO_VERSION
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int esp_err_t;

#ifndef ESP_OK
#define ESP_OK 0
#endif

typedef struct esp_netif_obj esp_netif_t;

typedef enum {
  ESP_NETIF_DNS_MAIN = 0,
  ESP_NETIF_DNS_BACKUP = 1,
} esp_netif_dns_type_t;

typedef enum {
  ESP_IPADDR_TYPE_V4 = 0,
  ESP_IPADDR_TYPE_V6 = 6,
} esp_ipaddr_type_t;

typedef struct {
  uint32_t addr;
} esp_ip4_addr_t;

typedef struct {
  uint32_t addr[4];
} esp_ip6_addr_t;

typedef struct {
  esp_ipaddr_type_t type;
  union {
    esp_ip4_addr_t ip4;
    esp_ip6_addr_t ip6;
  } u_addr;
} esp_ip_addr_t;

typedef struct {
  esp_ip_addr_t ip;
} esp_netif_dns_info_t;

static inline esp_netif_t* esp_netif_get_handle_from_ifkey(const char* /*if_key*/) {
  return (esp_netif_t*) 0;
}

static inline esp_err_t esp_netif_get_dns_info(
    esp_netif_t* /*esp_netif*/,
    esp_netif_dns_type_t /*type*/,
    esp_netif_dns_info_t* /*dns*/) {
  return -1;
}

#ifdef __cplusplus
}

#include "WString.h"

class IPv6Address {
 public:
  explicit IPv6Address(const uint32_t* /*addr*/) {}

  String toString() const {
    return String("::");
  }
};
#endif
