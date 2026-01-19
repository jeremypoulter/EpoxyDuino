/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#ifndef EPOXY_DUINO_PTY_SERIAL_H
#define EPOXY_DUINO_PTY_SERIAL_H

#include <string>
#include "Print.h"
#include "Stream.h"

/**
 * A version of Serial that reads from and writes to a PTY (pseudo-terminal)
 * port. This allows communication with virtual or real PTY devices for
 * emulation and testing purposes.
 */
class PtySerial: public Stream {
  public:
    /**
     * Construct an instance with a PTY port path.
     *
     * @param portPath Path to the PTY device (e.g., "/dev/ttyUSB0" or "/tmp/vpty")
     */
    explicit PtySerial(const std::string& portPath);

    /**
     * Destructor. Closes the PTY port if open.
     */
    ~PtySerial();

    /**
     * Open and configure the PTY port.
     *
     * @param baud Baud rate (standard rates like 9600, 115200, etc.)
     * @return true if port opened successfully, false otherwise
     */
    bool begin(unsigned long baud = 115200);

    /**
     * Close the PTY port.
     */
    void end();

    /**
     * Check if the port is open.
     */
    operator bool() { return portFd >= 0; }

    /**
     * Write a single byte to the PTY port.
     *
     * @param c The byte to write
     * @return Number of bytes written (0 or 1)
     */
    size_t write(uint8_t c) override;

    // Pull in all other overloaded versions of the write() function from the
    // Print parent class.
    using Print::write;

    /**
     * Check how many bytes are available to read.
     *
     * @return Number of bytes available, or -1 on error
     */
    int available() override;

    /**
     * Read a single byte from the PTY port.
     *
     * @return The byte read, or -1 if no data available or error
     */
    int read() override;

    /**
     * Peek at the next byte without consuming it.
     *
     * @return The next byte, or -1 if no data available or error
     */
    int peek() override;

    /**
     * Set the port path after construction.
     *
     * @param portPath Path to the PTY device
     */
    void setPortPath(const std::string& portPath);

  private:
    /**
     * Configure PTY port settings (baud rate, data bits, parity, etc.)
     *
     * @param baud Baud rate
     * @return true if configuration successful, false otherwise
     */
    bool configurePort(unsigned long baud);

    std::string portPath;
    int portFd;
    int bufch;
};

#endif
