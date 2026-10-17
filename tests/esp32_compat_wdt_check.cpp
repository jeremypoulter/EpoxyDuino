// Self-check for the host loop-watchdog stubs in cores/epoxy/esp32_compat.h.
// Build and run:
//   g++ -std=c++11 -I cores/epoxy tests/esp32_compat_wdt_check.cpp -o /tmp/wdt && /tmp/wdt
#include <assert.h>
#include "esp32_compat.h"

int main()
{
  assert(loopWDTArmed());        // armed by default, like the ESP32 core
  disableLoopWDT();
  assert(!loopWDTArmed());       // OTA handshake suspends it
  feedLoopWDT();                 // feeding while disarmed is harmless
  assert(!loopWDTArmed());
  enableLoopWDT();
  assert(loopWDTArmed());        // resume re-arms it
  return 0;
}
