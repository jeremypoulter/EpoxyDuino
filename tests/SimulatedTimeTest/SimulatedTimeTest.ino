#include <epoxy_test/Script/Script.h>
#include <Arduino.h>
#include <AUnit.h>
#include <aunit/Test.h>
#include <unistd.h>

using aunit::TestRunner;
using namespace EpoxyTest;

//---------------------------------------------------------------------------

test(SimulatedTime, back_to_real_after_reset)
{
  EpoxyTest::reset();
  set_seconds(1);
  assertEqual(millis(), 1000UL);

  EpoxyTest::reset();
  auto m = millis();
  usleep(10000);
  assertFalse(m == millis());
}

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
  EpoxyTest::reset();
  set_seconds(1);
  add_seconds(1);
  assertEqual(millis(), 2000UL);

  add_millis(1000);
  assertEqual(millis(), 3000UL);

  add_micros(1000000);
  assertEqual(millis(), 4000UL);
}

test(SimulatedTime, multiple_direct_scripts)
{
  EpoxyTest::reset();
  set_seconds(0);
  EpoxyTest::Script s1("script:pin D1 0");
  EpoxyTest::Script s2("script:at 100ms pin D1 1");
  EpoxyTest::Script s3("script:at 200ms pin D1 0");

  assertEqual(digitalRead(1), 0);
  while(millis() <= 110)
  {
    delay(10);
    usleep(2);
  }
  assertEqual(digitalRead(1), 1);
  while(millis() <= 210) delay(10);
  assertEqual(digitalRead(1), 0);
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
