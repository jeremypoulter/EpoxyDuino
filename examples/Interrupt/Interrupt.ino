/*
 * Interrupt Example - Demonstrates interrupt handling in EpoxyDuino
 * 
 * This example shows how to use interrupts with EpoxyDuino. It demonstrates:
 * 1. Attaching interrupt handlers to pins
 * 2. Different interrupt modes (RISING, FALLING, CHANGE)
 * 3. Using digitalReadValue() to simulate pin changes
 * 4. Using checkInterrupts() to process interrupt events
 * 
 * On Linux or Mac, type:
 *  * $ make
 *  * $ ./Interrupt.out
 */

#include <Arduino.h>

// Pin definitions
#define BUTTON_PIN 2
#define LED_PIN 13

// Interrupt counters
volatile int risingCount = 0;
volatile int fallingCount = 0;
volatile int changeCount = 0;

// Interrupt handler for RISING edge
void risingInterrupt() {
  risingCount++;
  SERIAL_PORT_MONITOR.println(F("RISING interrupt triggered!"));
}

// Interrupt handler for FALLING edge
void fallingInterrupt() {
  fallingCount++;
  SERIAL_PORT_MONITOR.println(F("FALLING interrupt triggered!"));
}

// Interrupt handler for CHANGE (any edge)
void changeInterrupt() {
  changeCount++;
  SERIAL_PORT_MONITOR.print(F("CHANGE interrupt triggered! Pin state: "));
  SERIAL_PORT_MONITOR.println(digitalRead(LED_PIN));
}

void setup() {
  SERIAL_PORT_MONITOR.begin(115200);
  SERIAL_PORT_MONITOR.println(F("=== EpoxyDuino Interrupt Example ==="));
  SERIAL_PORT_MONITOR.println();
  
  // Configure pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Attach interrupt handlers
  attachInterrupt(BUTTON_PIN, risingInterrupt, RISING);
  attachInterrupt(LED_PIN, changeInterrupt, CHANGE);
  
  SERIAL_PORT_MONITOR.println(F("Interrupts attached:"));
  SERIAL_PORT_MONITOR.print(F("  Pin "));
  SERIAL_PORT_MONITOR.print(BUTTON_PIN);
  SERIAL_PORT_MONITOR.println(F(": RISING"));
  SERIAL_PORT_MONITOR.print(F("  Pin "));
  SERIAL_PORT_MONITOR.print(LED_PIN);
  SERIAL_PORT_MONITOR.println(F(": CHANGE"));
  SERIAL_PORT_MONITOR.println();
}

void loop() {
  SERIAL_PORT_MONITOR.println(F("--- Test 1: Simulating BUTTON_PIN HIGH (RISING edge) ---"));
  digitalReadValue(BUTTON_PIN, HIGH);
  checkInterrupts();
  delay(500);
  
  SERIAL_PORT_MONITOR.println(F("--- Test 2: Simulating BUTTON_PIN LOW (no interrupt expected) ---"));
  digitalReadValue(BUTTON_PIN, LOW);
  checkInterrupts();
  delay(500);
  
  SERIAL_PORT_MONITOR.println(F("--- Test 3: Simulating LED_PIN HIGH (CHANGE interrupt) ---"));
  digitalWrite(LED_PIN, HIGH);
  digitalReadValue(LED_PIN, HIGH);
  checkInterrupts();
  delay(500);
  
  SERIAL_PORT_MONITOR.println(F("--- Test 4: Simulating LED_PIN LOW (CHANGE interrupt) ---"));
  digitalWrite(LED_PIN, LOW);
  digitalReadValue(LED_PIN, LOW);
  checkInterrupts();
  delay(500);
  
  SERIAL_PORT_MONITOR.println(F("--- Test 5: Testing detachInterrupt ---"));
  detachInterrupt(BUTTON_PIN);
  SERIAL_PORT_MONITOR.println(F("Detached BUTTON_PIN interrupt"));
  digitalReadValue(BUTTON_PIN, HIGH);
  checkInterrupts();
  SERIAL_PORT_MONITOR.println(F("No interrupt should have triggered"));
  delay(500);
  
  SERIAL_PORT_MONITOR.println(F("--- Test 6: Reattaching with FALLING mode ---"));
  attachInterrupt(BUTTON_PIN, fallingInterrupt, FALLING);
  digitalReadValue(BUTTON_PIN, LOW);
  checkInterrupts();
  delay(500);
  
  // Print statistics
  SERIAL_PORT_MONITOR.println();
  SERIAL_PORT_MONITOR.println(F("=== Interrupt Statistics ==="));
  SERIAL_PORT_MONITOR.print(F("RISING interrupts:  "));
  SERIAL_PORT_MONITOR.println(risingCount);
  SERIAL_PORT_MONITOR.print(F("FALLING interrupts: "));
  SERIAL_PORT_MONITOR.println(fallingCount);
  SERIAL_PORT_MONITOR.print(F("CHANGE interrupts:  "));
  SERIAL_PORT_MONITOR.println(changeCount);
  SERIAL_PORT_MONITOR.println();
  
  SERIAL_PORT_MONITOR.println(F("=== Test Complete - Restarting in 2 seconds ==="));
  SERIAL_PORT_MONITOR.println();
  delay(2000);
  
  // Reset counters for next iteration
  risingCount = 0;
  fallingCount = 0;
  changeCount = 0;
  
  // Reset pin states
  digitalReadValue(BUTTON_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalReadValue(LED_PIN, LOW);
  
  // Reattach interrupts for next iteration
  attachInterrupt(BUTTON_PIN, risingInterrupt, RISING);
  attachInterrupt(LED_PIN, changeInterrupt, CHANGE);
}
