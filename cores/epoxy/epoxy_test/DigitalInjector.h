#pragma once
#include "Injection.h"
#include <Arduino.h>

namespace EpoxyInjection
{

class DigitalInjectorEvent : public Event
{
  public:
    DigitalInjectorEvent(unsigned long us, uint8_t pin, uint8_t val)
    :
    Event(us),
    pin_(pin),
    val_(val) {}

    unsigned long raise() override
    {
      digitalReadValue(pin_, val_);
      return 0;
    }

  private:
    uint8_t pin_;
    uint8_t val_;
};

}
