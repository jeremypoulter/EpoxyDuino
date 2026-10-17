#pragma once

// Misc ESP32-only helpers referenced by the firmware.
// Used only for PlatformIO `native` builds.

// Pull in system time headers before defining any compatibility macros.
#include <time.h>
#include <sys/time.h>

// Loop watchdog (the ESP32 Arduino core's loopTask WDT). The host has no
// hardware timer, so a stalled loop is never reset. We only track whether the
// firmware armed or disarmed it, so callers can check that they pair the calls
// (e.g. OTA disables it for the TLS handshake and re-enables it after).
// ponytail: no timeout is enforced. Upgrade path: a watchdog thread that calls
// abort() when feedLoopWDT() has not run within 5 s while armed.
static inline bool &loopWDTArmedFlag()
{
  static bool armed = true;
  return armed;
}
static inline void enableLoopWDT() { loopWDTArmedFlag() = true; }
static inline void disableLoopWDT() { loopWDTArmedFlag() = false; }
static inline void feedLoopWDT() {}
static inline bool loopWDTArmed() { return loopWDTArmedFlag(); }

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
