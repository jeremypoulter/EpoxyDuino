#if defined(EPOXY_DUINO)

#include <Update.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>

UpdateClass Update;

namespace {

FILE* image = nullptr;
size_t written = 0;
std::string source;

// Empty when the sink is off, so each record is written only when asked for.
std::string sinkPath(const char* name) {
  const char* dir = getenv("EPOXY_UPDATE_SINK_DIR");
  if (!dir || !*dir) {
    return std::string();
  }
  return std::string(dir) + "/" + name;
}

void closeImage() {
  if (image) {
    fclose(image);
    image = nullptr;
  }
}

void recordEnd(bool ok) {
  closeImage();
  std::string path = sinkPath("ota_end.json");
  if (path.empty()) {
    return;
  }
  FILE* f = fopen(path.c_str(), "w");
  if (f) {
    fprintf(f, "{\"ok\":%s,\"written\":%zu}\n", ok ? "true" : "false", written);
    fclose(f);
  }
}

}  // namespace

void EpoxyUpdateSetSource(const char* url) {
  source = url ? url : "";
}

bool UpdateClass::begin(size_t size, int /*cmd*/) {
  closeImage();
  written = 0;

  std::string path = sinkPath("ota_begin.json");
  if (!path.empty()) {
    FILE* f = fopen(path.c_str(), "w");
    if (f) {
      // The source is a URL from the request: leave quotes and backslashes out of the JSON.
      fprintf(f, "{\"source\":\"");
      for (char c : source) {
        if (c != '"' && c != '\\') fputc(c, f);
      }
      fprintf(f, "\",\"begin_size\":%zu}\n", size);
      fclose(f);
    }
    image = fopen(sinkPath("ota_image.bin").c_str(), "wb");
  }

  source.clear();
  return true;
}

size_t UpdateClass::write(const uint8_t* data, size_t len) {
  if (image && len > 0) {
    fwrite(data, 1, len, image);
    fflush(image);
  }
  written += len;
  return len;
}

bool UpdateClass::end(bool /*evenIfRemaining*/) {
  recordEnd(true);
  return true;
}

void UpdateClass::abort() {
  recordEnd(false);
}

#endif  // EPOXY_DUINO
