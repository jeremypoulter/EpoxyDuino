#pragma once

// Native-build shim for ESP32 Arduino WiFi event constants + event info struct.
// Used only for PlatformIO `native` builds.

#include <stdint.h>

// Event constants are defined in WiFiEvent_t (see WiFi.h).
// This shim only provides the event info struct used by the firmware.

// Provide the subset of arduino_event_info_t used by NetManagerTask.
// This intentionally only models the fields accessed in net_manager.cpp.

typedef struct {
  struct {
    uint8_t ssid[33];
    uint8_t bssid[6];
    uint8_t channel;
  } wifi_sta_connected;

  struct {
    uint8_t ssid[33];
    uint8_t bssid[6];
    uint8_t reason;
  } wifi_sta_disconnected;

  struct {
    struct {
      struct { uint32_t addr; } ip;
      struct { uint32_t addr; } netmask;
      struct { uint32_t addr; } gw;
    } ip_info;
  } got_ip;

  struct {
    uint8_t mac[6];
    uint8_t aid;
  } wifi_ap_staconnected;

  struct {
    uint8_t mac[6];
    uint8_t aid;
  } wifi_ap_stadisconnected;

  struct {
    uint32_t number;
  } wifi_scan_done;
} arduino_event_info_t;
