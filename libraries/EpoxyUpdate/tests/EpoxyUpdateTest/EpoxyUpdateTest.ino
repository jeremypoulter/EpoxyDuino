#line 2 "EpoxyUpdateTest"

#include <Arduino.h>
#include <AUnit.h>
#include <Update.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

using aunit::TestOnce;
using aunit::TestRunner;

static const char SINK_DIR[] = "epoxyupdate_sink";
static const char BEGIN_PATH[] = "epoxyupdate_sink/ota_begin.json";
static const char IMAGE_PATH[] = "epoxyupdate_sink/ota_image.bin";
static const char END_PATH[] = "epoxyupdate_sink/ota_end.json";

static const uint8_t IMAGE[] = {0xe9, 0x01, 0x02, 0x03, 0x04};

// Returns the file contents as a NUL-terminated string, or "" when it is missing.
static const char* readFile(const char* path, char* buf, size_t size) {
  buf[0] = '\0';
  FILE* f = fopen(path, "rb");
  if (f) {
    size_t n = fread(buf, 1, size - 1, f);
    buf[n] = '\0';
    fclose(f);
  }
  return buf;
}

static void clearSink() {
  remove(BEGIN_PATH);
  remove(IMAGE_PATH);
  remove(END_PATH);
}

class UpdateSinkTest : public TestOnce {
 protected:
  void setup() override {
    TestOnce::setup();
    mkdir(SINK_DIR, 0755);
    setenv("EPOXY_UPDATE_SINK_DIR", SINK_DIR, 1);
    clearSink();
  }
};

testF(UpdateSinkTest, recordsSourceSizeAndImage) {
  EpoxyUpdateSetSource("https://github.com/OpenEVSE/openevse_esp32_firmware/releases/download/v1/a.bin");
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertEqual(sizeof(IMAGE), Update.write(IMAGE, sizeof(IMAGE)));
  assertTrue(Update.end());

  char buf[256];
  assertTrue(strstr(readFile(BEGIN_PATH, buf, sizeof(buf)),
      "\"source\":\"https://github.com/OpenEVSE/openevse_esp32_firmware/releases/download/v1/a.bin\"") != nullptr);
  assertTrue(strstr(buf, "\"begin_size\":5") != nullptr);

  char image[16];
  FILE* f = fopen(IMAGE_PATH, "rb");
  assertTrue(f != nullptr);
  assertEqual(sizeof(IMAGE), fread(image, 1, sizeof(image), f));
  fclose(f);
  assertEqual(0, memcmp(IMAGE, image, sizeof(IMAGE)));

  assertTrue(strstr(readFile(END_PATH, buf, sizeof(buf)), "\"ok\":true,\"written\":5") != nullptr);
}

testF(UpdateSinkTest, sourceIsEscapedAsJson) {
  EpoxyUpdateSetSource("https://example.com/a\"b\\c.bin");
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertTrue(Update.end());

  char buf[256];
  assertTrue(strstr(readFile(BEGIN_PATH, buf, sizeof(buf)),
      "\"source\":\"https://example.com/a\\\"b\\\\c.bin\"") != nullptr);
}

testF(UpdateSinkTest, uploadHasEmptySource) {
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertEqual(sizeof(IMAGE), Update.write(IMAGE, sizeof(IMAGE)));
  assertTrue(Update.end());

  char buf[256];
  assertTrue(strstr(readFile(BEGIN_PATH, buf, sizeof(buf)), "\"source\":\"\"") != nullptr);
}

testF(UpdateSinkTest, sourceIsClearedByBegin) {
  EpoxyUpdateSetSource("https://github.com/OpenEVSE/openevse_esp32_firmware/releases/download/v1/a.bin");
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertTrue(Update.end());

  assertTrue(Update.begin(sizeof(IMAGE)));
  assertTrue(Update.end());

  char buf[256];
  assertTrue(strstr(readFile(BEGIN_PATH, buf, sizeof(buf)), "\"source\":\"\"") != nullptr);
}

testF(UpdateSinkTest, abortRecordsFailure) {
  assertTrue(Update.begin(sizeof(IMAGE)));
  Update.abort();

  char buf[256];
  assertTrue(strstr(readFile(END_PATH, buf, sizeof(buf)), "\"ok\":false") != nullptr);
}

testF(UpdateSinkTest, isRunningBetweenBeginAndEnd) {
  assertFalse(Update.isRunning());
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertTrue(Update.isRunning());
  assertEqual(sizeof(IMAGE), Update.write(IMAGE, sizeof(IMAGE)));
  assertTrue(Update.isRunning());
  assertTrue(Update.end());
  assertFalse(Update.isRunning());

  assertTrue(Update.begin(sizeof(IMAGE)));
  Update.abort();
  assertFalse(Update.isRunning());
}

testF(UpdateSinkTest, imageThatCannotBeOpenedIsRecordedAsFailed) {
  // A directory where the image should go makes fopen() fail. The update still
  // succeeds, but the record must not say the image is good.
  mkdir(IMAGE_PATH, 0755);
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertEqual(sizeof(IMAGE), Update.write(IMAGE, sizeof(IMAGE)));
  assertTrue(Update.end());

  char buf[256];
  assertTrue(strstr(readFile(END_PATH, buf, sizeof(buf)), "\"ok\":false") != nullptr);
}

testF(UpdateSinkTest, nothingRecordedWhenSinkUnset) {
  unsetenv("EPOXY_UPDATE_SINK_DIR");
  assertTrue(Update.begin(sizeof(IMAGE)));
  assertEqual(sizeof(IMAGE), Update.write(IMAGE, sizeof(IMAGE)));
  assertTrue(Update.end());

  assertTrue(fopen(BEGIN_PATH, "rb") == nullptr);
  assertTrue(fopen(IMAGE_PATH, "rb") == nullptr);
  assertTrue(fopen(END_PATH, "rb") == nullptr);
}

void setup() {
  delay(1000);
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR);
}

void loop() {
  TestRunner::run();
}
