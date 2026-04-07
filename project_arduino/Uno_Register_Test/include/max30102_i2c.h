//include/max30102_i2c.h
//
// Created by hiimseoll on 26. 4. 7..
//

#ifndef UNTITLED3_MAX30102_I2C_H

#include "i2c.h"
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h> // ltoa 사용
#include <string.h>

#define UNTITLED3_MAX30102_I2C_H

// R/W ADDRESS========================================================
#define MAX30102_I2C_ADDR 0x57
#define MAX30102_I2C_WRITE_ADDR (MAX30102_I2C_ADDR << 1)        // 0xAE
#define MAX30102_I2C_READ_ADDR  ((MAX30102_I2C_ADDR << 1) | 1)  // 0xAF
// ===================================================================

// REGISTER ADDRESS===================================================
#define MAX30102_REG_INTR_STATUS_1 0x00
#define MAX30102_REG_INTR_STATUS_2 0x01
#define MAX30102_REG_INTR_ENABLE_1 0x02
#define MAX30102_REG_INTR_ENABLE_2 0x03
#define MAX30102_REG_FIFO_WR_PTR 0x04
#define MAX30102_REG_OVF_COUNTER 0x05
#define MAX30102_REG_FIFO_RD_PTR 0x06
#define MAX30102_REG_FIFO_DATA 0x07
#define MAX30102_REG_FIFO_CONFIG 0x08
#define MAX30102_REG_MODE_CONFIG 0x09
#define MAX30102_REG_SPO2_CONFIG 0x0A
#define MAX30102_REG_LED1_PA 0x0C
#define MAX30102_REG_LED2_PA 0x0D
#define MAX30102_REG_PILOT_PA 0x10
#define MAX30102_REG_MULTI_LED_CTRL1 0x11
#define MAX30102_REG_MULTI_LED_CTRL2 0x12
#define MAX30102_REG_TEMP_INTR 0x1F
#define MAX30102_REG_TEMP_FRAC 0x20
#define MAX30102_REG_TEMP_CONFIG 0x21
#define MAX30102_REG_PROX_INT_THRESH 0x30
#define MAX30102_REG_REV_ID 0xFE
#define MAX30102_REG_PART_ID 0xFF
// ===================================================================

// COMMAND============================================================
#define MAX30102_CONFIG_RESET 0x40
// ===================================================================

#define AVG_SIZE            8
#define FINGER_THRESHOLD    25000
#define PEAK_THRESHOLD      15.0f
#define MIN_PEAK_INTERVAL   400

typedef struct _dc{
    float alpha;
    float dc_val;
    float prev_raw;
    float ac_val;
} dc_t;

typedef struct _pulse_time{
    volatile uint32_t current_ms;
    uint32_t last_peak_ms;
    uint32_t interval_ms;
} pulse_time_t;

extern volatile pulse_time_t p_time;

void timer0_init(void);
uint8_t max30102_init();
uint8_t max30102_write_reg(uint8_t reg_addr, uint8_t data);
uint8_t max30102_read_reg(uint8_t reg_addr, uint8_t* data);
uint8_t max30102_read_fifo(uint32_t* red_led, uint32_t* ir_led);
void max30102_calculate_heart_and_spo2(uint32_t red_raw, uint32_t ir_raw, float* heart_bps, float* spo2_data);
uint8_t max30102_read_temperature(int8_t* integer_part, uint8_t* fractional_part);
uint8_t max30102_reset();
void max30102_uart_plot_pulse(char* buffer);

#endif //UNTITLED3_MAX30102_I2C_H