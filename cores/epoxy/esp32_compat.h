#pragma once

// Misc ESP32-only helpers referenced by the firmware.
// Used only for PlatformIO `native` builds.

// Pull in system time headers before defining any compatibility macros.
#include <time.h>
#include <sys/time.h>

// Loop watchdog (the ESP32 Arduino core's loopTask WDT), implemented in
// esp32_wdt.cpp. Off until enableLoopWDT(); then the process is terminated,
// with a dump of the loop task's stack, if feedLoopWDT() is not called within
// the timeout. epoxyduino_main() feeds it after every loop().
void enableLoopWDT();
void disableLoopWDT();
void feedLoopWDT();
bool loopWDTArmed();
// Timeout in ms (default 5000, or EPOXY_LOOP_WDT_MS); 0 never expires.
void loopWDTSetTimeout(unsigned ms);
// Replaces terminate-and-dump on expiry, e.g. so tests can observe it.
void loopWDTSetExpiryHandler(void (*handler)());

// Arduino pin->interrupt mapping helper (used by MicroTasks). For host builds
// we treat the pin number as the interrupt number.
#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(p) (p)
#endif

// ESP32 Arduino code often uses `timezone` as a typedef. On Linux/glibc this
// is typically a variable, while the type is `struct timezone`.
// Map `timezone` token to `struct timezone` for host builds.
#ifndef timezone
#define timezone struct timezone
#endif
