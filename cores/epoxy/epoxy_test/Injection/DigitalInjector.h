#pragma once
#include <Arduino.h>

#include "Event.h"

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

    unsigned long raise() override;

  private:
    uint8_t pin_;
    uint8_t val_;
};

}
