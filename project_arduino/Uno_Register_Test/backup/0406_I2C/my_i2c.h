//my_i2c.h

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdint.h> //[추가] uint8_t 사용을 위해 필요

// I2C 클록 설정 (100kHz @ 16MHz CPU)
#define SCL_CLOCK 100000L

void i2c_init(void);
uint8_t i2c_start(uint8_t address);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

#endif