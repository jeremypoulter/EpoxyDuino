// Selects the EEPROM flavour for the core chosen with EPOXY_CORE. The ESP8266
// and ESP32 cores share the API in EpoxyEepromEsp.h. The AVR core has a
// different API, in EpoxyEepromAvr.h, with no begin(size), commit() or end().
#if defined(EPOXY_CORE_AVR)
#include <EpoxyEepromAvr.h>
#define EEPROM EpoxyEepromAvrInstance
#else
#include <EpoxyEepromEsp.h>
#define EEPROM EpoxyEepromEspInstance
#endif
