//src/srclib/my_MAX30102.h
#ifndef MY_MAX30102_H
#define MY_MAX30102_H

#include <avr/io.h>

#define MAX30102_ADDR 0xAE  // 7비트 주소 0x57 + Write(0) = 0xAE / Read(1) = 0xAF

// 주요 레지스터 주소
#define REG_INTR_STATUS_1 0x00
#define REG_MODE_CONFIG   0x09
#define REG_SPO2_CONFIG   0x0A
#define REG_LED1_PA       0x0C // Red LED
#define REG_LED2_PA       0x0D // IR LED
#define REG_FIFO_DATA     0x07

void I2C_Init(void);
void MAX30102_Write(uint8_t reg, uint8_t data);
uint8_t MAX30102_Read(uint8_t reg);
uint8_t MAX30102_Read_Interrupt_Status(void); //인터럽트 추가
void MAX30102_Init(void);
void MAX30102_Read_FIFO(uint32_t *red, uint32_t *ir);

#endif