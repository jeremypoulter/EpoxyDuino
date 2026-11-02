#include <Arduino.h>
#include <AUnit.h>
#include <EpoxyEepromEsp.h>

using aunit::TestRunner;

#define EEPROM EpoxyEepromEspInstance

//---------------------------------------------------------------------------

#if !defined(EPOXY_CORE_ESP8266)
// begin() returns bool on the ESP32 core (the default here), void on ESP8266
test(beginReturnsFalseForZeroSize) {
  // Refused even before any begin(), when the size already matches (0)
  EpoxyEepromEsp fresh;
  assertFalse(fresh.begin(0));
}
#endif

test(writeAndReadTest) {
#if defined(EPOXY_CORE_ESP8266)
  EEPROM.begin(1024);
#else
  assertTrue(EEPROM.begin(1024));
  assertFalse(EEPROM.begin(0)); // a zero size is refused and the 1024 bytes stay
#endif
  assertEqual((size_t) 1024, (size_t) EEPROM.length());

  // Write using put()
  uint32_t output = 0x04030201;
  EEPROM.put(0, output);

  // Write at the end of the EEPROM space, using write() and operator[].
  EEPROM.write(1020, 1);
  EEPROM.write(1021, 2);
  EEPROM[1022] = 3;
  EEPROM[1023] = 4;

  // Save to disk
  EEPROM.commit();
  EEPROM.end();

  // Read back again.
  EEPROM.begin(1024);

  // Read using get()
  uint32_t input = 0;
  EEPROM.get(0, input);
  assertEqual(output, input);

  // Read using read() and operator[].
  assertEqual(1, EEPROM.read(1020));
  assertEqual(2, EEPROM.read(1021));
  assertEqual(3, EEPROM[1022]);
  assertEqual(4, EEPROM[1023]);

  EEPROM.end();
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // needed for Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
