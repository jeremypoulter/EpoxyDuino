#if defined(EPOXY_DUINO)

#include <Update.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>

UpdateClass Update;

namespace {

FILE* image = nullptr;
size_t written = 0;
// Set when the sink fails to open, write, flush or close a record. The update
// itself still succeeds (the host API always does); the record says so.
bool sinkFailed = false;
std::string source;

// Empty when the sink is off, so each record is written only when asked for.
std::string sinkPath(const char* name) {
  const char* dir = getenv("EPOXY_UPDATE_SINK_DIR");
  if (!dir || !*dir) {
    return std::string();
  }
  return std::string(dir) + "/" + name;
}

// Escapes a string for use between the quotes of a JSON string.
std::string jsonEscape(const std::string& text) {
  std::string out;
  for (unsigned char c : text) {
    if (c == '"' || c == '\\') {
      out += '\\';
      out += static_cast<char>(c);
    } else if (c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", c);
      out += buf;
    } else {
      out += static_cast<char>(c);
    }
  }
  return out;
}

void closeImage() {
  if (image) {
    if (fclose(image) != 0) sinkFailed = true;
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
    bool recorded = ok && !sinkFailed;
    fprintf(f, "{\"ok\":%s,\"written\":%zu}\n", recorded ? "true" : "false", written);
    if (fclose(f) != 0) sinkFailed = true;
  }
}

}  // namespace

void EpoxyUpdateSetSource(const char* url) {
  source = url ? url : "";
}

bool UpdateClass::begin(size_t size, int /*cmd*/) {
  closeImage();
  written = 0;
  sinkFailed = false;
  running = true;

  std::string path = sinkPath("ota_begin.json");
  if (!path.empty()) {
    // A record of the previous update must not outlive the start of this one.
    remove(sinkPath("ota_end.json").c_str());

    FILE* f = fopen(path.c_str(), "w");
    if (f) {
      fprintf(f, "{\"source\":\"%s\",\"begin_size\":%zu}\n",
          jsonEscape(source).c_str(), size);
      if (fclose(f) != 0) sinkFailed = true;
    } else {
      sinkFailed = true;
    }

    image = fopen(sinkPath("ota_image.bin").c_str(), "wb");
    if (!image) sinkFailed = true;
  }

  source.clear();
  return true;
}

size_t UpdateClass::write(const uint8_t* data, size_t len) {
  if (image && len > 0) {
    if (fwrite(data, 1, len, image) != len) sinkFailed = true;
    if (fflush(image) != 0) sinkFailed = true;
  }
  written += len;
  return len;
}

bool UpdateClass::end(bool /*evenIfRemaining*/) {
  running = false;
  recordEnd(true);
  return true;
}

void UpdateClass::abort() {
  running = false;
  recordEnd(false);
}

#endif  // EPOXY_DUINO
