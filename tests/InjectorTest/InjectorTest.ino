#line 2 "DigitalReadTest"

#include <epoxy_test/injection>
#include <Arduino.h>
#include <AUnit.h>

using aunit::TestRunner;

//---------------------------------------------------------------------------
test(InjectorTest, digital_pins_reset_after_reset)
{
  for(int i=0; i<32; i++)
    digitalReadValue(i, 1);

  for(int i=0; i<32; i++)
    assertEqual(digitalRead(i), 1);

  EpoxyTest::reset();

  for(int i=0; i<32; i++)
    assertEqual(digitalRead(i), 0);
}

test(InjectorTest, events_cancelled)
{
  digitalReadValue(0, 0);
  auto event = std::unique_ptr<EpoxyInjection::DigitalInjectorEvent>(new EpoxyInjection::DigitalInjectorEvent(500, 0, 1));  // digital set to 1 at 500us
  EpoxyInjection::Injector::addEvent(std::move(event));
  // After 500us, pin will be set to 1 but...
  assertEqual(EpoxyInjection::Injector::eventsSize(), (size_t)1);
  EpoxyTest::reset();

  assertEqual(EpoxyInjection::Injector::eventsSize(), (size_t)0);
}

test(InjectorTest, wait_and_check_real_time)
{
  EpoxyTest::reset();
  // EpoxyTest::set_millis(0);

  // Set the pin to 0
  digitalReadValue(0, 0);
  // Verify that pins return 0 initially.
  assertEqual(digitalRead(0), 0);

  auto event = std::unique_ptr<EpoxyInjection::DigitalInjectorEvent>(new EpoxyInjection::DigitalInjectorEvent(500, 0, 1));  // digital set to 1 at 500us
  EpoxyInjection::Injector::addEvent(std::move(event));

  for(int i=0; i<100; i++)  // Wait max for 2 sec (test may failed under very high cpu load)
  {
    delay(20);
    if(digitalRead(0), 1) break;
  }

  // Check that the pin has changed
  assertEqual(digitalRead(0), 1);

  // Reset the pin
  digitalReadValue(0, 0);
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
