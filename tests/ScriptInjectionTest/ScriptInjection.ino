#line 2 "DigitalReadTest"

#include <epoxy_test/Script/Script.h>
#include <Arduino.h>
#include <AUnit.h>

using aunit::TestRunner;

//---------------------------------------------------------------------------
test(ScriptInjection, pin)
{
  EpoxyTest::reset();
  EpoxyTest::Script script("scripts/script.txt");

  assertEqual(digitalRead(1), 0);
  while(millis() <= 110) delay(10);
  assertEqual(digitalRead(1), 1);
}

test(ScriptInjection, pin_mapping)
{
  EpoxyTest::reset();
  EpoxyTest::Script script("scripts/mapping.txt");

  assertEqual(digitalRead(1), 0);
  while(millis() <= 110) delay(10);
  assertEqual(digitalRead(SS), 1);
  assertEqual(digitalRead(1), 1);
}

test(ScriptInjection, at)
{
  EpoxyTest::reset();
  EpoxyTest::Script script("scripts/at.txt");

  assertEqual(digitalRead(4), 0);
  while(millis() <= 550) delay(10);
  assertEqual(digitalRead(4), 1);
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
