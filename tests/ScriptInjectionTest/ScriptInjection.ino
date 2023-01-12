#line 2 "DigitalReadTest"

#include <epoxy_test/Script/Script.h>
#include <epoxy_test/Script/Commands/BitStream.h>
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

test(ScriptInjection, bitstream)
{
  EpoxyTest::reset();
  EpoxyTest::Script bitstream("scripts/bitstream.txt");

  struct streamRead
  {
    int pin;              // pin of bitstream
    int period;           // period
    unsigned long peek;   // next time to sample bit
    size_t size;          // Number of bits to read
    std::string stream;   // rebuilt of bitstream

    unsigned long end_sampling()
    {
      return (size+1) * period;
    }

    streamRead(int pin, int period, size_t size)
      : pin(pin), period(period)
      , peek(0)
      , size(size)
    {}

    // Note: this is a kind of software serial emulation that could
    // be re-used
    void sample()
    {
      if (stream.length() == size) return;
      if (peek == 0)
      {
        if (digitalRead(pin) == 0) return;  // wait start bit
        peek = micros() + period + period/2;
      }
      else if (peek > micros())
      {
        return;
      }
      else
      {
        peek += period;
      }

      if (digitalRead(pin) == 1)
        stream +='1';
      else
        stream +='0';
    }

  };

  // see bitstream.txt for values
  streamRead stream_1(5, 100000, 16);
  streamRead stream_2(6, 150000, 16);

  auto end_sampling = std::max(stream_1.end_sampling(), stream_2.end_sampling());

  while(micros() < end_sampling)
  {
    stream_1.sample();
    stream_2.sample();
    delayMicroseconds(10);
  }

  // Disable test if any sync problem has occured (machine to slow ?)
  if (EpoxyTest::BitStream::outOfSyncCount())
  {
    std::cerr << "*** Warning: test cancelled due to outOfSync errors" << std::endl;
    return;
  }

  assertEqual(stream_1.stream.c_str(), "1001110111010110");
  assertEqual(stream_2.stream.c_str(), "1101001010010101");
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
