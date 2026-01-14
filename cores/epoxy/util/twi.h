/*
 * Copyright (c) 2025
 * MIT License
 */

/**
 * @file util/twi.h
 *
 * TWI (Two Wire Interface / I2C) register definitions for AVR emulation.
 */

#ifndef UTIL_TWI_H
#define UTIL_TWI_H

// TWI Status Codes
#define TW_START                0x08
#define TW_REP_START            0x10

// Master Transmitter
#define TW_MT_SLA_ACK           0x18
#define TW_MT_SLA_NACK          0x1C
#define TW_MT_DATA_ACK          0x28
#define TW_MT_DATA_NACK         0x30
#define TW_MT_ARB_LOST          0x38

// Master Receiver
#define TW_MR_ARB_LOST          0x38
#define TW_MR_SLA_ACK           0x40
#define TW_MR_SLA_NACK          0x44
#define TW_MR_DATA_ACK          0x50
#define TW_MR_DATA_NACK         0x58

// Slave Transmitter
#define TW_ST_SLA_ACK           0xA8
#define TW_ST_ARB_LOST_SLA_ACK  0xB0
#define TW_ST_DATA_ACK          0xB8
#define TW_ST_DATA_NACK         0xC0
#define TW_ST_LAST_DATA         0xC8

// Slave Receiver
#define TW_SR_SLA_ACK           0x60
#define TW_SR_ARB_LOST_SLA_ACK  0x68
#define TW_SR_GCALL_ACK         0x70
#define TW_SR_ARB_LOST_GCALL_ACK 0x78
#define TW_SR_DATA_ACK          0x80
#define TW_SR_DATA_NACK         0x88
#define TW_SR_GCALL_DATA_ACK    0x90
#define TW_SR_GCALL_DATA_NACK   0x98
#define TW_SR_STOP              0xA0

// Misc
#define TW_NO_INFO              0xF8
#define TW_BUS_ERROR            0x00

// TWI prescaler values
#define TW_FREQ_100K            100000UL
#define TW_FREQ_250K            250000UL
#define TW_FREQ_400K            400000UL

// Status register mask
#define TW_STATUS_MASK          0xF8
#define TW_STATUS               (TWSR & TW_STATUS_MASK)

// Control register bits
#define TWINT   7
#define TWEA    6
#define TWSTA   5
#define TWSTO   4
#define TWWC    3
#define TWEN    2
#define TWIE    0

// Prescaler bits
#define TWPS0   0
#define TWPS1   1

// Read/Write bits
#define TW_READ   1
#define TW_WRITE  0

// TWI registers (defined in avr/io.h but declared here for reference)
#ifndef TWBR
extern volatile uint8_t _epoxy_TWBR;
extern volatile uint8_t _epoxy_TWSR;
extern volatile uint8_t _epoxy_TWAR;
extern volatile uint8_t _epoxy_TWDR;
extern volatile uint8_t _epoxy_TWCR;
extern volatile uint8_t _epoxy_TWAMR;

#define TWBR  _epoxy_TWBR
#define TWSR  _epoxy_TWSR
#define TWAR  _epoxy_TWAR
#define TWDR  _epoxy_TWDR
#define TWCR  _epoxy_TWCR
#define TWAMR _epoxy_TWAMR
#endif

#endif // UTIL_TWI_H
