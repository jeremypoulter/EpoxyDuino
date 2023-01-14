#pragma once

#include <map>
#include <string>

#if defined(EPOXY_CORE_ESP8266)
  #include "pins_arduino_esp8266.h"
#endif
#if defined(EPOXY_CORE_AVR)
  #include "pins_arduino_avr.h"
#endif

namespace EpoxyTest
{
	extern std::map<std::string, int> pins_arduino;
}

