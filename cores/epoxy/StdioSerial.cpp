/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <stdio.h>
#include <unistd.h>
#include "StdioSerial.h"

#ifndef SERIAL_OUTFD
#define SERIAL_OUTFD STDOUT_FILENO
#endif

size_t StdioSerial::write(uint8_t c) {
  ssize_t status = ::write(SERIAL_OUTFD, &c, 1);
  return (status <= 0) ? 0 : 1;
}

int StdioSerial::read() {
  int ch = peek();
  bufch = -1;
  return ch;
}

int StdioSerial::peek() {
  if (bufch == -1) {
    // 'c' must be unsigned to avoid ambiguity with -1 in-band error condition
    unsigned char c;
    ssize_t status = ::read(STDIN_FILENO, &c, 1);
    bufch = (status <= 0) ? -1 : c;
  }
  return bufch;
}

int StdioSerial::available() {
  int ch = peek();
  return (int) (ch != -1);
}

StdioSerial Serial;
