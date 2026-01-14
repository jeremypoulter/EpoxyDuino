#if defined(EPOXY_DUINO) && (defined(EPOXY_CORE_ESP8266) || defined(EPOXY_CORE_ESP32))

// Storage for extern globals declared by native shim headers.

#include <Update.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

MDNSResponder MDNS;
UpdateClass Update;
ArduinoOTAClass ArduinoOTA;

#endif
