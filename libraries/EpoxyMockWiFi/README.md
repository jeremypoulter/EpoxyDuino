# EpoxyMockWiFi Library

A mock implementation of the Arduino WiFi libraries to allow code written against
ESP8266, ESP32, and other Arduino WiFi APIs to compile under EpoxyDuino. This
library provides stub implementations of common WiFi functionality.

This library is designed to be extensible, allowing future support for additional
Arduino-compatible WiFi implementations.

## Supported Platforms

Currently provides mock APIs compatible with:

* **ESP8266 WiFi** - ESP8266WiFi library
* **ESP32 WiFi** - WiFi library for ESP32
* **Generic Arduino WiFi** - Standard Arduino WiFi API

## Features

This mock library provides:

* WiFi connection management (begin, disconnect, status)
* Network scanning
* Access Point (AP) mode
* Station (STA) mode
* IP address configuration
* MAC address access
* WiFi event handling (stubs)
* WiFiClient and WiFiServer classes
* DNS and mDNS stubs

## Usage

### Code Changes

Code written against the Arduino WiFi libraries should compile with minimal
changes:

```C++
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.begin("MySSID", "MyPassword");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());
}
```

### Platform-Specific Code

For code that needs to distinguish between platforms:

```C++
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#elif defined(EPOXY_DUINO)
  #include <WiFi.h>
#endif
```

### Makefile

Add `EpoxyMockWiFi` to the `ARDUINO_LIBS` in the EpoxyDuino `Makefile`:

```make
APP_NAME := MyApp
ARDUINO_LIBS := EpoxyMockWiFi ...
include ../../../../EpoxyDuino.mk
```

## Simulating WiFi State

The mock library provides functions to simulate WiFi behavior for testing:

```C++
#if defined(EPOXY_DUINO)
  // Simulate successful connection
  WiFi.mockSetStatus(WL_CONNECTED);
  WiFi.mockSetLocalIP(IPAddress(192, 168, 1, 100));
#endif
```

## Limitations

This is a mock library for compilation testing. It does not:

* Actually connect to WiFi networks
* Send or receive network data
* Perform DNS resolution
* Handle real network events

For actual network functionality on desktop systems, consider using native
socket programming or libraries like libcurl.

## Future Extensions

Planned additions:

* WiFiUDP mock
* More detailed event simulation
* Network traffic simulation for testing
* Support for additional Arduino WiFi variants

## License

MIT License
