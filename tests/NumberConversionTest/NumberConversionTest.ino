#include <Arduino.h>
#include <AUnit.h>
#include <aunit/Test.h>
#include <limits.h>

using aunit::TestRunner;

//---------------------------------------------------------------------------
// itoa()/utoa()/ltoa()/ultoa() used to divide their unsigned accumulator with
// the signed div()/ldiv(). Anything above the signed maximum came back with a
// negative remainder, so `rem + '0'` landed below '0' and the digits came out
// as punctuation -- ultoa(0xB699AE2E81C34B07, s, 16) gave ",'**+/#/)\"-$%,!'"
// rather than "b699ae2e81c34b07".
//---------------------------------------------------------------------------

test(NumberConversionTest, ultoa_above_signed_max) {
  char buf[80];
  assertEqual(ultoa(0xB699AE2E81C34B07UL, buf, 16), "B699AE2E81C34B07");
  assertEqual(ultoa(ULONG_MAX, buf, 10), "18446744073709551615");
}

test(NumberConversionTest, utoa_above_signed_max) {
  char buf[80];
  assertEqual(utoa(0xDEADBEEFU, buf, 16), "DEADBEEF");
  assertEqual(utoa(UINT_MAX, buf, 10), "4294967295");
}

test(NumberConversionTest, base2_does_not_overrun_the_buffer) {
  // One character per bit. BUFSIZE was sized for an int, so the widest of these
  // wrote past the end of the scratch buffer.
  char buf[80];
  assertEqual(ultoa(ULONG_MAX, buf, 2), "1111111111111111111111111111111111111111111111111111111111111111");
}

test(NumberConversionTest, negative_and_most_negative) {
  char buf[80];
  assertEqual(itoa(-42, buf, 10), "-42");
  assertEqual(ltoa(-42L, buf, 10), "-42");
  // Negating the most negative value as a signed type is undefined behaviour.
  assertEqual(itoa(INT_MIN, buf, 10), "-2147483648");
  assertEqual(ltoa(LONG_MIN, buf, 10), "-9223372036854775808");
}

test(NumberConversionTest, still_right_for_ordinary_values) {
  char buf[80];
  assertEqual(itoa(0, buf, 10), "0");
  assertEqual(utoa(0, buf, 16), "0");
  assertEqual(itoa(255, buf, 16), "FF");
  assertEqual(ltoa(123456789L, buf, 10), "123456789");
  assertEqual(ultoa(255UL, buf, 2), "11111111");
}

//---------------------------------------------------------------------------

void setup() {
  aunit::Test::displayMinPosition(50);
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (! Serial);
}

void loop() {
  TestRunner::run();
}
