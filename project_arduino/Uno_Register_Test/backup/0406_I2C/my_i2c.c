//my_i2c.c
#include <avr/io.h>
#include "my_i2c.h"

void i2c_init(void) {
    TWBR = 72; // 100kHz (CPU 16MHz 기준)
    TWSR = 0;  // Prescaler = 1
}

uint8_t i2c_start(uint8_t address) {
    // 1. START 전송
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    if ((TWSR & 0xF8) != 0x08) return 1; // START 실패

    // 2. 주소 전송 (Write 모드)
    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // 3. ACK 확인 (0x18: SLA+W 전송 후 ACK 수신 성공)
    if ((TWSR & 0xF8) == 0x18) return 0; // 장치 발견
    else return 2; // 응답 없음
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    // STOP은 대기하지 않음
}