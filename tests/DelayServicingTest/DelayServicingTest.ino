// Regression tests for delay()/delayMicroseconds():
//
//   1. In simulated time they must advance the clock by exactly the amount
//      requested. delayMicroseconds() used to call delay(1000*us) -- delay()
//      takes milliseconds, so the microsecond count was scaled by 10^6.
//
//   2. They must service interrupts and yield callbacks while waiting. On
//      real hardware an interrupt fires during a delay; on the host nothing
//      runs unless something polls for it, so a delay that blocked straight
//      through would stall every registered handler for its whole duration.
//      Code that waits on an interrupt-driven flag inside a delay loop would
//      then never observe it -- exactly the shape of the OpenEVSE GFI
//      self-test, which pulses a pin and waits for the trip to come back.

#include <epoxy_test/Script/Script.h>
#include <Arduino.h>
#include <AUnit.h>
#include <aunit/Test.h>

using aunit::TestRunner;
using namespace EpoxyTest;

//---------------------------------------------------------------------------
// simulated-time duration
//---------------------------------------------------------------------------

test(DelayServicing, delay_advances_exactly)
{
  EpoxyTest::reset();
  set_micros(0);
  delay(10);
  assertEqual(micros(), 10000UL);
}

test(DelayServicing, delayMicroseconds_advances_exactly)
{
  EpoxyTest::reset();
  set_micros(0);
  delayMicroseconds(8333); // GFI_PULSE_ON_US, ~1/2 cycle of 60Hz
  assertEqual(micros(), 8333UL);
}

test(DelayServicing, delayMicroseconds_sub_slice)
{
  EpoxyTest::reset();
  set_micros(0);
  delayMicroseconds(250); // shorter than the 1ms service slice
  assertEqual(micros(), 250UL);
}

test(DelayServicing, delay_zero_still_advances_nothing)
{
  EpoxyTest::reset();
  set_micros(0);
  delay(0);
  assertEqual(micros(), 0UL);
}

//---------------------------------------------------------------------------
// servicing while waiting
//---------------------------------------------------------------------------

namespace {

volatile int g_callbackCount = 0;
volatile int g_isrCount = 0;

constexpr uint8_t kIsrPin = 4;

void countingCallback(void* /*context*/) {
  ++g_callbackCount;
}

void countingIsr() {
  ++g_isrCount;
}

// Raises kIsrPin, so the following service pass sees a rising edge. Stands in
// for hardware asserting an input line mid-delay. n.b. digitalReadValue(), not
// digitalWrite(): EpoxyDuino keeps input and output pin state separate, and
// checkInterrupts() polls the input side.
void raisePinCallback(void* /*context*/) {
  digitalReadValue(kIsrPin, HIGH);
}

// Deliberately calls delay() from inside a service pass.
void delayingCallback(void* /*context*/) {
  ++g_callbackCount;
  delay(1);
}

} // namespace

test(DelayServicing, delay_services_yield_callbacks)
{
  EpoxyTest::reset();
  g_callbackCount = 0;
  assertTrue(epoxyRegisterYieldServiceCallback(countingCallback, nullptr));

  delay(5);

  epoxyUnregisterYieldServiceCallback(countingCallback, nullptr);
  assertMore(g_callbackCount, 0);
}

test(DelayServicing, delayMicroseconds_services_yield_callbacks)
{
  EpoxyTest::reset();
  g_callbackCount = 0;
  assertTrue(epoxyRegisterYieldServiceCallback(countingCallback, nullptr));

  delayMicroseconds(2000);

  epoxyUnregisterYieldServiceCallback(countingCallback, nullptr);
  assertMore(g_callbackCount, 0);
}

// The end-to-end case: a line goes high while the firmware sits in delay(),
// and the interrupt handler must run before the delay returns.
test(DelayServicing, delay_delivers_interrupt)
{
  EpoxyTest::reset();
  g_isrCount = 0;
  digitalReadValue(kIsrPin, LOW);
  attachInterrupt(kIsrPin, countingIsr, RISING);
  assertTrue(epoxyRegisterYieldServiceCallback(raisePinCallback, nullptr));

  delay(5);

  epoxyUnregisterYieldServiceCallback(raisePinCallback, nullptr);
  detachInterrupt(kIsrPin);
  assertMore(g_isrCount, 0);
}

// A callback that itself delays must not recurse back into servicing.
// n.b. a named function, not a lambda: register and unregister must be the
// same function pointer, and two lambda expressions are two distinct ones.
test(DelayServicing, reentrant_callback_terminates)
{
  EpoxyTest::reset();
  g_callbackCount = 0;
  assertTrue(epoxyRegisterYieldServiceCallback(delayingCallback, nullptr));

  delay(3);

  assertTrue(epoxyUnregisterYieldServiceCallback(delayingCallback, nullptr));
  assertMore(g_callbackCount, 0);
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
