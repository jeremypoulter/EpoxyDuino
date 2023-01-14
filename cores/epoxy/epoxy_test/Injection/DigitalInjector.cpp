#include "DigitalInjector.h"
#include <Arduino.h>
#include <iostream>

namespace EpoxyInjection
{
  __attribute__((constructor))
  static void resetPins()
  {
    EpoxyTest::registerReset([]() -> void
    {
      for(int i=0; i<32; i++)
        digitalReadValue(i, 0);
    });
  };

  unsigned long DigitalInjectorEvent::raise()
  {
    digitalReadValue(pin_, val_);
    return 0;
  }
}
