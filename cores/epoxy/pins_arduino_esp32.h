/*
  Minimal ESP32 pin definitions for EpoxyDuino.

  EpoxyDuino runs on a Unix host, so these values exist primarily to satisfy
  compilation of libraries that expect common Arduino-ESP32 symbols.

  The actual numeric values are not important for EpoxyDuino's GPIO stubs.
*/

#ifndef EPOXY_DUINO_PINS_ARDUINO_ESP32_H
#define EPOXY_DUINO_PINS_ARDUINO_ESP32_H

#include <stdint.h>

// Common defaults on many ESP32 dev boards (e.g. WROOM/WROVER).
#define LED_BUILTIN 2

#define PIN_WIRE_SDA (21)
#define PIN_WIRE_SCL (22)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

// Common SPI defaults.
static const uint8_t SS   = 5;
static const uint8_t MOSI = 23;
static const uint8_t MISO = 19;
static const uint8_t SCK  = 18;

// Common UART pins.
static const uint8_t RX = 3;
static const uint8_t TX = 1;

// Some libraries (and EpoxyDuino's own pin name mapping) expect D0..D15.
// Provide simple aliases.
static const uint8_t D0  = 0;
static const uint8_t D1  = 1;
static const uint8_t D2  = 2;
static const uint8_t D3  = 3;
static const uint8_t D4  = 4;
static const uint8_t D5  = 5;
static const uint8_t D6  = 6;
static const uint8_t D7  = 7;
static const uint8_t D8  = 8;
static const uint8_t D9  = 9;
static const uint8_t D10 = 10;
static const uint8_t D11 = 11;
static const uint8_t D12 = 12;
static const uint8_t D13 = 13;
static const uint8_t D14 = 14;
static const uint8_t D15 = 15;

// Define some arbitrary analog pins for compilation.
static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;
static const uint8_t A8 = 8;
static const uint8_t A9 = 9;

#endif
