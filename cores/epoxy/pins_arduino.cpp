#ifdef EPOXY_TEST

#include "pins_arduino.h"

namespace EpoxyTest
{

std::map<std::string, int> pins_arduino = 
{
  { "LED_BUILTINT", LED_BUILTIN },
  { "D0"  , D0  },
  { "D1"  , D1  },
  { "D2"  , D2  },
  { "D3"  , D3  },
  { "D4"  , D4  },
  { "D5"  , D5  },
  { "D6"  , D6  },
  { "D7"  , D7  },
  { "D8"  , D8  },
  { "D9"  , D9  },
  { "D10" , D10 },
  { "D11" , D11 },
  { "D12" , D12 },
  { "D13" , D13 },
  { "D14" , D14 },
  { "D15" , D15 },

  { "A0"  , A0  },
  { "A1"  , A1  },
  { "A2"  , A2  },
  { "A3"  , A3  },
  { "A4"  , A4  },
  { "A5"  , A5  },
  { "A6"  , A6  },
  { "A7"  , A7  },
  { "A8"  , A8  },
  { "A9"  , A9  },

#ifdef EPOXY_CORE_ESP8266
  { "SDA", PIN_WIRE_SDA },
  { "SCL", PIN_WIRE_SCL },
#endif

#ifdef EPOXY_CORE_AVR
	{ "SS"   , 1  },
	{ "MOSI" , 2  },
	{ "MISO" , 3  },
	{ "SCK"  , 4  },
	{ "SDA"  , 5  },
	{ "SCL"  , 6  },

#endif

};

} // ns

#endif
