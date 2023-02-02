#include <epoxy_test/Script/Script.h>
#include <Arduino.h>
#include <AUnit.h>

using aunit::TestRunner;
using namespace EpoxyTest;

#define assertEqualString(s1,s2) assertEqual(std::string(s1).c_str(), std::string(s2).c_str())

//---------------------------------------------------------------------------
test(ScriptParser, getWord)
{
  ScriptParser parser("w1  w2  ", 0);

  assertEqualString(parser.getLine(), "w1  w2  ");
  assertEqualString(parser.getWord(), "w1");
  assertEqualString(parser.getWord(), "w2");
  assertEqualString(parser.getLine(), "");
}

test(ScriptParser, getNumber)
{
  ScriptParser parser("10 nan 30kHz", 10, false);

  int integer;
  assertEqual(parser.getNumber(integer), true);
  assertEqual(integer, 10);

  assertEqual(parser.getNumber(integer), false);
  assertEqual(parser.errorsCount(), 1);
  parser.getWord(); // remove 'nan'

  assertEqual(parser.getNumber(integer), true);
  assertEqual(integer, 30);

  assertEqualString(parser.getLine(), "kHz");
}

test(ScriptParser, getPinNumber)
{
  pins_arduino["dummy"] = 50;
  ScriptParser parser("dummy 14", 0);
  
  assertEqual(parser.getPinNumber(), 50);
  assertEqual(parser.getPinNumber(), 14);
}

test(ScriptParser, getDuration)
{
  ScriptParser parser("10us 10ms 3s 1mn 100", 0, false);

  unsigned long d;
  assertEqual(parser.getDuration(d), true); // 10us
  assertEqual((int)d, 10);

  assertEqual(parser.getDuration(d), true); // 10ms
  assertEqual((int)d, 10000);

  assertEqual(parser.getDuration(d), true); // 3s
  assertEqual((int)d, 3000000);

  assertEqual(parser.getDuration(d), true); // 1mn
  assertEqual((int)d, 60000000);

  assertEqual(parser.getDuration(d), false);  // 100 ?

  assertEqual(parser.errorsCount(), 1);
}

test(ScriptParser, getPeriodUs)
{
  ScriptParser parser("1us 1ms 2Hz 1day", 0, false);

  assertEqual(parser.getPeriodUs(), 1);
  assertEqual(parser.getPeriodUs(), 1000);
  assertEqual(parser.getPeriodUs(), 2000000);
  assertEqual(parser.getPeriodUs(), 0); // 1day ?
  assertEqual(parser.errorsCount(), 1);
}

test(ScriptParser, ratioUs)
{
  ScriptParser parser("us ms s user", 0, false);

  assertEqual(parser.ratioUs(), 1);
  assertEqual(parser.ratioUs(), 1000);
  assertEqual(parser.ratioUs(), 1000000);
  assertEqual(parser.ratioUs(), 0); // user ?
}

test(ScriptParser, periodUs)
{
  ScriptParser parser("Hz kHz Mhz kilo", 0, false);

  assertEqual(parser.periodUs(), 1000000);
  assertEqual(parser.periodUs(), 1000);
  assertEqual(parser.periodUs(), 1);
  assertEqual(parser.periodUs(), 0); // kilo ?
}

test(ScriptParser, startsWith)
{
  std::string line("bitstream");;

  assertEqual(EpoxyTest::startsWith(line, "bitstream"), true);
  assertEqual(EpoxyTest::startsWith(line, "bitstreamer"), false);
  assertEqual(EpoxyTest::startsWith(line, "bitstreap"), false);
  assertEqual(EpoxyTest::startsWith(line, "bitstre"), false);

  line = "bitstream "; // with space
  assertEqual(EpoxyTest::startsWith(line, "bitstream"), true);

  line = "bitstream\t"; // with space
  assertEqual(EpoxyTest::startsWith(line, "bitstream"), true);
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
