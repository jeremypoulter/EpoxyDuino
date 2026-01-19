/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/select.h>
#include "PtySerial.h"

// Map baud rate integers to termios speed constants
static speed_t baudToSpeed(unsigned long baud) {
  switch (baud) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
#ifdef B500000
    case 500000: return B500000;
#endif
#ifdef B576000
    case 576000: return B576000;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
#ifdef B1000000
    case 1000000: return B1000000;
#endif
#ifdef B1152000
    case 1152000: return B1152000;
#endif
#ifdef B1500000
    case 1500000: return B1500000;
#endif
#ifdef B2000000
    case 2000000: return B2000000;
#endif
#ifdef B2500000
    case 2500000: return B2500000;
#endif
#ifdef B3000000
    case 3000000: return B3000000;
#endif
#ifdef B3500000
    case 3500000: return B3500000;
#endif
#ifdef B4000000
    case 4000000: return B4000000;
#endif
    default: return B115200;  // Default to 115200
  }
}

PtySerial::PtySerial(const std::string& path)
    : portPath(path), portFd(-1), bufch(-1) {
}

PtySerial::~PtySerial() {
  end();
}

void PtySerial::setPortPath(const std::string& path) {
  if (portFd < 0) {
    portPath = path;
  }
}

bool PtySerial::configurePort(unsigned long baud) {
  struct termios tty;
  
  // Get current terminal settings
  if (tcgetattr(portFd, &tty) != 0) {
    perror("tcgetattr failed");
    return false;
  }

  // Set baud rate
  speed_t speed = baudToSpeed(baud);
  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  // Set 8 data bits, 1 stop bit, no parity
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;

  // Raw mode: disable canonical input and echo
  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_oflag &= ~OPOST;

  // Set read timeout to non-blocking (0 bytes minimum, 0 timeout)
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  // Apply settings
  if (tcsetattr(portFd, TCSANOW, &tty) != 0) {
    perror("tcsetattr failed");
    return false;
  }

  // Flush any existing data
  tcflush(portFd, TCIOFLUSH);

  return true;
}

bool PtySerial::begin(unsigned long baud) {
  if (portFd >= 0) {
    return true;  // Already open
  }

  // Open the PTY port
  portFd = open(portPath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  
  if (portFd < 0) {
    perror("Failed to open PTY port");
    return false;
  }

  // Configure the port
  if (!configurePort(baud)) {
    close(portFd);
    portFd = -1;
    return false;
  }

  bufch = -1;
  return true;
}

void PtySerial::end() {
  if (portFd >= 0) {
    close(portFd);
    portFd = -1;
    bufch = -1;
  }
}

size_t PtySerial::write(uint8_t c) {
  if (portFd < 0) {
    return 0;
  }

  ssize_t status = ::write(portFd, &c, 1);
  return (status <= 0) ? 0 : 1;
}

int PtySerial::read() {
  int ch = peek();
  bufch = -1;
  return ch;
}

int PtySerial::peek() {
  if (portFd < 0) {
    return -1;
  }

  if (bufch == -1) {
    unsigned char c;
    ssize_t status = ::read(portFd, &c, 1);
    bufch = (status <= 0) ? -1 : c;
  }
  
  return bufch;
}

int PtySerial::available() {
  if (portFd < 0) {
    return 0;
  }

  // Check if there's data in our buffer
  if (bufch != -1) {
    return 1;
  }

  // Use select() to check if data is available
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(portFd, &readfds);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  int result = select(portFd + 1, &readfds, nullptr, nullptr, &timeout);
  
  if (result > 0 && FD_ISSET(portFd, &readfds)) {
    return 1;  // Data available
  }

  return 0;
}
