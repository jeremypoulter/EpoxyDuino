#if defined(EPOXY_DUINO)

// Storage for extern globals declared by native shim headers.

#include <Update.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

MDNSResponder MDNS;
UpdateClass Update;
ArduinoOTAClass ArduinoOTA;

#endif
