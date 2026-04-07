//src/srclib/my_MAX30102.c
#include "my_MAX30102.h"
#include <util/delay.h>

void I2C_Init(void) {
    TWSR = 0x00; // 분주비 1
    TWBR = 72;   // 16MHz 클록에서 100kHz SCL 속도 설정
    TWCR = (1 << TWEN);
}

void I2C_Start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void I2C_Stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void I2C_Write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t I2C_Read_ACK(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C_Read_NACK(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void MAX30102_Write(uint8_t reg, uint8_t data) {
    I2C_Start();
    I2C_Write(MAX30102_ADDR);
    I2C_Write(reg);
    I2C_Write(data);
    I2C_Stop();
}

uint8_t MAX30102_Read(uint8_t reg) {
    uint8_t data;
    I2C_Start();
    I2C_Write(MAX30102_ADDR);     // Write Mode
    I2C_Write(reg);               // 레지스터 선택
    
    I2C_Start();                  // Repeated Start
    I2C_Write(MAX30102_ADDR | 1); // Read Mode
    data = I2C_Read_NACK();       // 1바이트 읽고 종료
    I2C_Stop();
    
    return data;
}

// void MAX30102_Init(void) {
//     I2C_Init();
//     MAX30102_Write(0x09, 0x40); // Reset
//     MAX30102_Write(REG_MODE_CONFIG, 0x03); // SpO2 mode (Red + IR)
//     MAX30102_Write(REG_SPO2_CONFIG, 0x27); // 411us pulse width, 100sps
//     MAX30102_Write(REG_LED1_PA, 0x24);     // Red LED 전류 설정
//     MAX30102_Write(REG_LED2_PA, 0x24);     // IR LED 전류 설정
// }

void MAX30102_Init(void) {
    I2C_Init();
    MAX30102_Write(0x09, 0x40); // Reset
    _delay_ms(100);             // 리셋 후 안정화 시간 필요

    // FIFO 포인터 초기화 (추가 권장)
    MAX30102_Write(0x04, 0x00); // FIFO_WRITE_PTR
    MAX30102_Write(0x05, 0x00); // OVERFLOW_COUNTER
    MAX30102_Write(0x06, 0x00); // FIFO_READ_PTR

    MAX30102_Write(REG_MODE_CONFIG, 0x03); // SpO2 mode (Red + IR)
    MAX30102_Write(REG_SPO2_CONFIG, 0x27); // 411us pulse width, 100sps
    MAX30102_Write(REG_LED1_PA, 0x24);     // Red LED 전류 설정
    MAX30102_Write(REG_LED2_PA, 0x24);     // IR LED 전류 설정
}

void MAX30102_Read_FIFO(uint32_t *red, uint32_t *ir) {
    I2C_Start();
    I2C_Write(MAX30102_ADDR);
    I2C_Write(REG_FIFO_DATA);
    
    I2C_Start(); // Repeated Start
    I2C_Write(MAX30102_ADDR | 1); // Read mode
    
    uint8_t data[6];
    for(int i=0; i<5; i++) data[i] = I2C_Read_ACK();
    data[5] = I2C_Read_NACK();
    I2C_Stop();

    *red = ((uint32_t)data[0] << 16 | (uint32_t)data[1] << 8 | data[2]) & 0x3FFFF;
    *ir = ((uint32_t)data[3] << 16 | (uint32_t)data[4] << 8 | data[5]) & 0x3FFFF;
}