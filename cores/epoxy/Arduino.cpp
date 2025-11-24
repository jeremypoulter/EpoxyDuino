/*
 * Copyright (c) 2019 Brian T. Park
 *
 * Parts derived from the Arduino SDK
 * Copyright (c) 2005-2013 Arduino Team
 *
 * Parts inspired by [Entering raw
 * mode](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html).
 *
 * Parts inspired by [ESP8266 Host
 * Emulation](https://github.com/esp8266/Arduino/tree/master/tests/host).
 *
 */

#include <inttypes.h>
#include <unistd.h> // usleep()
#include <time.h> // clock_gettime()
#include <atomic>
#include "Arduino.h"
#include <epoxy_test/Injection/Injection.h>

#include "epoxy_test/ArduinoTest.inc"

std::atomic<unsigned long> epoxy_micros;
unsigned long epoxy_start_time = 0;
bool epoxy_real_time = true;

// -----------------------------------------------------------------------
// Arduino methods emulated in Unix
// -----------------------------------------------------------------------

static uint32_t digitalReadPinValues = 0;
static uint32_t digitalWritePinValues = 0;

void yield() {
  usleep(1000); // prevents program from consuming 100% CPU
}

void pinMode(uint8_t /*pin*/, uint8_t /*mode*/) {}

void digitalWrite(uint8_t pin, uint8_t val) {
  if (pin >= 32) return;

  if (val == 0) {
    digitalWritePinValues &= ~(((uint32_t)0x1) << pin);
  } else {
    digitalWritePinValues |= ((uint32_t)0x1) << pin;
  }
}

uint8_t digitalWriteValue(uint8_t pin) {
  if (pin >= 32) return 0;

  return (digitalWritePinValues & (((uint32_t)0x1) << pin)) != 0;
}

int digitalRead(uint8_t pin) {
  if (pin >= 32) return 0;

  return (digitalReadPinValues & (((uint32_t)0x1) << pin)) != 0;
}

void digitalReadValue(uint8_t pin, uint8_t val) {
  if (pin >= 32) return;

  if (val == 0) {
    digitalReadPinValues &= ~(((uint32_t)0x1) << pin);
  } else {
    digitalReadPinValues |= ((uint32_t)0x1) << pin;
  }
}

int analogRead(uint8_t /*pin*/) { return 0; }

void analogWrite(uint8_t /*pin*/, int /*val*/) {}

unsigned long millis() {
  if (epoxy_real_time)
  {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    unsigned long ms = spec.tv_sec * 1000U + spec.tv_nsec / 1000000UL;
    return ms - epoxy_start_time / 1000;
  }
  else
  {
    return epoxy_micros / 1000;
  }
}

unsigned long micros() {
  if (epoxy_real_time)
  {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    unsigned long us = spec.tv_sec * 1000000UL + spec.tv_nsec / 1000U;
    return us - epoxy_start_time;
  }
  else
  {
    return epoxy_micros;
  }
}

void tone(uint8_t /*_pin*/, unsigned int /*frequency*/, unsigned long /*duration*/) {}

void noTone(uint8_t /*_pin*/) {}

void delay(unsigned long ms) {
  if (epoxy_real_time)
  {
    usleep(ms * 1000);
  }
  else
  {
    EpoxyInjection::Injector::delay_us(1000 * ms);
  }
}

void delayMicroseconds(unsigned int us) {
  if (epoxy_real_time)
  {
    usleep(us);
  }
  else
  {
    delay(1000*us);
  }
}

unsigned long pulseIn(
    uint8_t /*pin*/, uint8_t /*state*/, unsigned long /*timeout*/) {
  return 0;
}

unsigned long pulseInLong(
    uint8_t /*pin*/, uint8_t /*state*/, unsigned long /*timeout*/) {
  return 0;
}

void shiftOut(
    uint8_t /*dataPin*/, uint8_t /*clockPin*/, uint8_t /*bitOrder*/,
    uint8_t /*val*/) {}

uint8_t shiftIn(
    uint8_t /*dataPin*/, uint8_t /*clockPin*/, uint8_t /*bitOrder*/) {
  return 0;
}

// -----------------------------------------------------------------------
// Interrupt support
// -----------------------------------------------------------------------

#define MAX_INTERRUPTS 32

struct InterruptHandler {
  void (*userFunc)(void);
  void (*userFuncArg)(void*);
  void* arg;
  uint8_t pin;
  int mode;
  bool enabled;
  uint8_t lastPinState;
};

static InterruptHandler interruptHandlers[MAX_INTERRUPTS];
static bool interruptsInitialized = false;

static void initInterrupts() {
  if (!interruptsInitialized) {
    for (int i = 0; i < MAX_INTERRUPTS; i++) {
      interruptHandlers[i].userFunc = nullptr;
      interruptHandlers[i].userFuncArg = nullptr;
      interruptHandlers[i].arg = nullptr;
      interruptHandlers[i].pin = 255;
      interruptHandlers[i].mode = 0;
      interruptHandlers[i].enabled = false;
      interruptHandlers[i].lastPinState = 0;
    }
    interruptsInitialized = true;
  }
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
  initInterrupts();
  
  if (interruptNum >= MAX_INTERRUPTS || userFunc == nullptr) {
    return;
  }
  
  interruptHandlers[interruptNum].userFunc = userFunc;
  interruptHandlers[interruptNum].userFuncArg = nullptr;
  interruptHandlers[interruptNum].arg = nullptr;
  interruptHandlers[interruptNum].pin = interruptNum;
  interruptHandlers[interruptNum].mode = mode;
  interruptHandlers[interruptNum].enabled = true;
  interruptHandlers[interruptNum].lastPinState = digitalRead(interruptNum);
}

void detachInterrupt(uint8_t interruptNum) {
  initInterrupts();
  
  if (interruptNum >= MAX_INTERRUPTS) {
    return;
  }
  
  interruptHandlers[interruptNum].userFunc = nullptr;
  interruptHandlers[interruptNum].userFuncArg = nullptr;
  interruptHandlers[interruptNum].arg = nullptr;
  interruptHandlers[interruptNum].enabled = false;
}

#if defined(EPOXY_CORE_ESP8266)
void attachInterruptArg(uint8_t pin, void (*userFunc)(void*), void* arg, int mode) {
  initInterrupts();
  
  if (pin >= MAX_INTERRUPTS || userFunc == nullptr) {
    return;
  }
  
  interruptHandlers[pin].userFunc = nullptr;
  interruptHandlers[pin].userFuncArg = userFunc;
  interruptHandlers[pin].arg = arg;
  interruptHandlers[pin].pin = pin;
  interruptHandlers[pin].mode = mode;
  interruptHandlers[pin].enabled = true;
  interruptHandlers[pin].lastPinState = digitalRead(pin);
}
#endif

/**
 * Check if any interrupts should be triggered based on pin state changes.
 * This function should be called periodically (e.g., from yield() or in the
 * main loop) to check for pin state changes and trigger interrupt handlers.
 * 
 * This is an EpoxyDuino-specific function for testing interrupt-driven code.
 */
void checkInterrupts() {
  initInterrupts();
  
  for (int i = 0; i < MAX_INTERRUPTS; i++) {
    if (!interruptHandlers[i].enabled) {
      continue;
    }
    
    uint8_t currentState = digitalRead(interruptHandlers[i].pin);
    uint8_t lastState = interruptHandlers[i].lastPinState;
    bool shouldTrigger = false;
    
    switch (interruptHandlers[i].mode) {
#if defined(EPOXY_CORE_AVR)
      case CHANGE:
        shouldTrigger = (currentState != lastState);
        break;
      case FALLING:
        shouldTrigger = (lastState == HIGH && currentState == LOW);
        break;
      case RISING:
        shouldTrigger = (lastState == LOW && currentState == HIGH);
        break;
#elif defined(EPOXY_CORE_ESP8266)
      case RISING:
        shouldTrigger = (lastState == LOW && currentState == HIGH);
        break;
      case FALLING:
        shouldTrigger = (lastState == HIGH && currentState == LOW);
        break;
      case CHANGE:
        shouldTrigger = (currentState != lastState);
        break;
      case ONLOW:
        shouldTrigger = (currentState == LOW);
        break;
      case ONHIGH:
        shouldTrigger = (currentState == HIGH);
        break;
      case ONLOW_WE:
        shouldTrigger = (currentState == LOW);
        break;
      case ONHIGH_WE:
        shouldTrigger = (currentState == HIGH);
        break;
#else
      case CHANGE:
        shouldTrigger = (currentState != lastState);
        break;
      case FALLING:
        shouldTrigger = (lastState == HIGH && currentState == LOW);
        break;
      case RISING:
        shouldTrigger = (lastState == LOW && currentState == HIGH);
        break;
#endif
    }
    
    if (shouldTrigger) {
      if (interruptHandlers[i].userFunc != nullptr) {
        interruptHandlers[i].userFunc();
      } else if (interruptHandlers[i].userFuncArg != nullptr) {
        interruptHandlers[i].userFuncArg(interruptHandlers[i].arg);
      }
    }
    
    interruptHandlers[i].lastPinState = currentState;
  }
}

/**
 * Manually trigger an interrupt for testing purposes. This simulates an
 * interrupt occurring on the specified pin.
 * 
 * This is an EpoxyDuino-specific function for testing interrupt-driven code.
 */
void triggerInterrupt(uint8_t interruptNum) {
  initInterrupts();
  
  if (interruptNum >= MAX_INTERRUPTS || !interruptHandlers[interruptNum].enabled) {
    return;
  }
  
  if (interruptHandlers[interruptNum].userFunc != nullptr) {
    interruptHandlers[interruptNum].userFunc();
  } else if (interruptHandlers[interruptNum].userFuncArg != nullptr) {
    interruptHandlers[interruptNum].userFuncArg(interruptHandlers[interruptNum].arg);
  }
}
