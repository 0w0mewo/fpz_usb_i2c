#ifndef _I2C_HELPERS_H_H
#define _I2C_HELPERS_H_H

#include <stdint.h>

#define I2C_TIMEOUT 6

uint8_t i2c_read(uint8_t dev_addr, uint8_t* buf, uint16_t len);
uint8_t i2c_write(uint8_t dev_addr, uint8_t* buf, uint16_t len);
uint8_t i2c_device_ready(uint8_t dev_addr);

void log16bits(uint16_t data);
void log8bits(uint8_t data);
#endif
