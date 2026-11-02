/*
  Copyright (c) 2021 Brian T. Park.

  API derived from:
  EEPROM.cpp - esp8266 EEPROM emulation

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <errno.h> // errno, ENOENT
#include <stdlib.h> // getenv()
#include <stdio.h> // FILE, fopen(), fwrite(), fread()
#include <string.h> // memset()
#include <new> // std::nothrow
#include "EpoxyEepromEsp.h"

bool EpoxyEepromEsp::open(size_t size) {
#if defined(EPOXY_CORE_ESP8266)
  // The ESP8266 core clamps to one flash sector and rounds up to 4 bytes. The
  // ESP32 core keeps the size it was given.
  if (size > 4096) size = 4096;
  size = (size + 3) & (~3);
#endif

  if (size == 0) return false;
#if defined(EPOXY_CORE_ESP32)
  // ESP32 length() returns uint16_t, so a larger size could not be reported.
  if (size > 0xFFFF) return false;
#endif
  if (size == size_) return true;

  uint8_t* data = new (std::nothrow) uint8_t[size];
  if (!data) return false;

  // Erased EEPROM reads as 0xFF, so any part of the file that is shorter than
  // the EEPROM keeps that value, rather than whatever new[] left behind.
  memset(data, 0xFF, size);

  FILE* fp = fopen(getDataPath(), "rb");
  if (fp) {
    fread(data, 1, size, fp);
    bool readFailed = ferror(fp) != 0;
    fclose(fp);
    if (readFailed) {
      delete[] data;
      return false;
    }
  } else if (errno != ENOENT) {
    // A file that exists but can't be opened is not a blank EEPROM.
    delete[] data;
    return false;
  }

  if (data_) {
    delete[] data_;
  }
  data_ = data;
  size_ = size;
  dirty_ = false;
  return true;
}

bool EpoxyEepromEsp::commit() {
  if (!size_ || !dirty_ || !data_) return true;
  FILE* fp = fopen(getDataPath(), "wb");
  if (!fp) return false;
  bool written = fwrite(data_, 1, size_, fp) == size_;
  bool closed = fclose(fp) == 0;
  if (written && closed) {
    dirty_ = false;
  }
  return written && closed;
}

const char* EpoxyEepromEsp::getDataPath() {
  const char* dataPath = getenv("EPOXY_EEPROM_DATA");
  if (!dataPath) {
    dataPath = "epoxyeepromdata";
  }
  return dataPath;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
EpoxyEepromEsp EpoxyEepromEspInstance;
#endif
