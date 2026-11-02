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

#ifndef EPOXY_EEPROM_ESP_H
#define EPOXY_EEPROM_ESP_H

#include <stdint.h>
#include <string.h> // memcpy()

/** Use this macro to distinguish between EpoxyEepromEsp or EpoxyEepromAvr. */
#define EPOXY_DUINO_EPOXY_EEPROM_ESP 1

/**
 * A EEPROM library that works on POSIX-like systems using EpoxyDuino. This
 * version implements the EEPROM API found on ESP8266 and ESP32 Arduino Cores.
 *
 * The two cores differ in the return types of begin(), end() and length(), so
 * those follow the core chosen with EPOXY_CORE_ESP8266 or EPOXY_CORE_ESP32.
 * Without either (the default AVR core), the ESP32 signatures are used.
 */
class EpoxyEepromEsp {
  public:
    EpoxyEepromEsp() = default;

#if defined(EPOXY_CORE_ESP8266)
    // The ESP8266 core's begin() returns void. The same checks run as on the
    // ESP32, so a failed begin() still keeps the previous contents.
    void begin(size_t size) { open(size); }
#else
    /**
     * ESP32 core: returns false if the size is zero or the buffer can't be
     * allocated. The previous contents are kept in that case.
     */
    bool begin(size_t size) { return open(size); }
#endif

    uint8_t read(int address) {
      return data_[address];
    }

    void write(int address, uint8_t val) {
      uint8_t prev = data_[address];
      if (val != prev) {
        data_[address] = val;
        dirty_ = true;
      }
    }

    bool commit();

#if defined(EPOXY_CORE_ESP8266)
    // The ESP8266 core's end() returns the result of commit().
    bool end() {
      bool saved = commit();
      close();
      return saved;
    }

    size_t length() { return size_; }
#else
    void end() {
      commit();
      close();
    }

    // The ESP32 core returns uint16_t.
    uint16_t length() { return (uint16_t) size_; }
#endif

    uint8_t& operator[](int address) { return data_[address]; }

    uint8_t const & operator[](int address) const { return data_[address]; }

    /**
     * Unlike the version on ESP8266 or ESP32, this implementation does not
     * perform bounds checking. It didn't seem to make sense since operator[]
     * does not do bounds checking either, and it seems better to catch out of
     * bounds problem on desktop systems with a Segmentation Fault.
     */
    template<typename T>
    T &get(int address, T &t) {
      memcpy((uint8_t*) &t, data_ + address, sizeof(T));
      return t;
    }

    /**
     * Unlike the version on ESP8266 or ESP32, this implementation does not
     * perform bounds checking. It didn't seem to make sense since operator[]
     * does not do bounds checking either, and it seems better to catch out of
     * bounds problem on desktop systems with a Segmentation Fault.
     */
    template<typename T>
    const T &put(int address, const T &t) {
      if (memcmp(data_ + address, (const uint8_t*)&t, sizeof(T)) != 0) {
        dirty_ = true;
        memcpy(data_ + address, (const uint8_t*)&t, sizeof(T));
      }

      return t;
    }

  private:
    static const char* getDataPath();

    bool open(size_t size);

    void close() {
      if (data_) {
        delete[] data_;
        data_ = nullptr;
      }
      size_ = 0;
      dirty_ = false;
    }

    uint8_t* data_ = nullptr;
    size_t size_ = 0;
    bool dirty_ = false;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern EpoxyEepromEsp EpoxyEepromEspInstance;
#endif

#endif
