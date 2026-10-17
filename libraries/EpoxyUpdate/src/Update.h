#pragma once

// Host-build implementation of the ESP32 Arduino Update API, for PlatformIO
// `native` builds. Every call succeeds, so the firmware runs its normal update
// code without a flash partition behind it.
//
// Set EPOXY_UPDATE_SINK_DIR to a directory and the update is also recorded
// there, so a test can check what the firmware handed the updater:
//   ota_begin.json  {"source":"...","begin_size":N}  written by begin()
//   ota_image.bin   every byte passed to write(), in order
//   ota_end.json    {"ok":true,"written":N}          written by end() or abort()
// "source" is the URL set with EpoxyUpdateSetSource(), or empty for an upload.

#ifdef __cplusplus

#include <stddef.h>
#include <stdint.h>

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

class UpdateClass {
 public:
  bool isRunning() const { return running; }
  bool isFinished() const { return true; }
  bool begin(size_t size = 0, int cmd = 0);
  size_t write(const uint8_t* data, size_t len);
  bool end(bool evenIfRemaining = false);
  bool hasError() const { return false; }
  int getError() const { return 0; }
  void printError(void* /*out*/) const {}
  void abort();
  const char* errorString() const { return "No error"; }

 private:
  // True from begin() until end() or abort(), as on the device: the firmware
  // starts a download only when no update is running.
  bool running = false;
};

extern UpdateClass Update;

// Host builds only: names the URL the next begin() downloads from, so the
// record can show where an update came from. begin() clears it.
void EpoxyUpdateSetSource(const char* source);

#endif
