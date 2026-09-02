#if defined(EPOXY_DUINO) \
    && (defined(EPOXY_CORE_ESP32) || defined(EPOXY_CORE_ESP8266))

// Update.h and ArduinoOTA.h are ESP-only concepts. Guarding on EPOXY_DUINO
// alone made this translation unit compile under every core, so selecting
// EPOXY_CORE_AVR failed here on a missing Update.h.

// Storage for extern globals declared by native shim headers.

#include <Update.h>
#include <ArduinoOTA.h>

UpdateClass Update;
ArduinoOTAClass ArduinoOTA;

#endif
