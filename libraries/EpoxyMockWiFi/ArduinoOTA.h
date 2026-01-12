#pragma once

// Minimal host-build stub for ArduinoOTA.
// Used only for PlatformIO `native` builds.

#include "Update.h"

typedef int ota_error_t;

class ArduinoOTAClass {
 public:
  ArduinoOTAClass() = default;

  template <typename Fn>
  ArduinoOTAClass& onStart(Fn /*fn*/) { return *this; }
  template <typename Fn>
  ArduinoOTAClass& onEnd(Fn /*fn*/) { return *this; }
  template <typename Fn>
  ArduinoOTAClass& onProgress(Fn /*fn*/) { return *this; }
  template <typename Fn>
  ArduinoOTAClass& onError(Fn /*fn*/) { return *this; }

  void setHostname(const char* /*hostname*/) {}
  void setPassword(const char* /*password*/) {}
  void setRebootOnSuccess(bool /*reboot*/) {}

  void begin() {}
  void handle() {}

  int getCommand() const { return 0; }

  static ArduinoOTAClass& getInstance() {
    static ArduinoOTAClass instance;
    return instance;
  }
};

extern ArduinoOTAClass ArduinoOTA;
