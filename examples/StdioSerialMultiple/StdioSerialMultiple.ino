/*
 * Test emulation of multiple Serial ports.
 *
 * $ ./StdioSerialMultiple.out
 * Printing to STDOUT
 * Printing to STDERR
 *
 * $ ./StdioSerialMultiple.out > /dev/null
 * Printing to STDERR
 */

#include <Arduino.h>

//-----------------------------------------------------------------------------

#if defined(EPOXY_DUINO)
StdioSerial Serial1(STDOUT_FILENO);
StdioSerial Serial2(STDERR_FILENO);
#endif


void setup(void) {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  Serial1.begin(115200);
  Serial2.begin(115200);
  while (!Serial1);
  while (!Serial2);

#if defined(EPOXY_DUINO)
  Serial1.setLineModeUnix();
  Serial2.setLineModeUnix();
#endif

  Serial1.println(F("Printing to STDOUT"));
  Serial2.println(F("Printing to STDERR"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop(void) {}
