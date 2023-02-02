#include <epoxy_test/Script/Script.h>
#include <Arduino.h>
#include <AUnit.h>
#include <aunit/Test.h>

using aunit::TestRunner;
using namespace EpoxyTest;

//---------------------------------------------------------------------------

test(SimulatedTime, setters)
{
  EpoxyTest::reset();
  set_seconds(1);
  assertEqual(millis(), 1000UL);
  set_millis(1000);
  assertEqual(millis(), 1000UL);
  set_micros(1000000);
  assertEqual(micros(), 1000000UL);
}

test(SimulatedTime, adders)
{
  set_seconds(1);
  add_seconds(1);
  assertEqual(millis(), 2000UL);

  add_millis(1000);
  assertEqual(millis(), 3000UL);

  add_micros(1000000);
  assertEqual(millis(), 4000UL);
}



//---------------------------------------------------------------------------

void setup() {
  aunit::Test::displayMinPosition(50);
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // needed for Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
