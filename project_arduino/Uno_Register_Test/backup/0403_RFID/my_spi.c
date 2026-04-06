//my_spi.c
#include <avr/io.h>
#include "my_spi.h"

void spi_init(void) {
    // SCK(PB5), MOSI(PB3), SS(PB2)를 출력으로 설정
    DDRB |= (1 << PB5) | (1 << PB3) | (1 << PB2);
    // MISO(PB4)를 입력으로 설정
    DDRB &= ~(1 << PB4);
    // SPI 활성화, 마스터 모드, 클럭 분주 1/4
    SPCR = (1 << SPE) | (1 << MSTR);
}

void spi_cs_init(spi_cs_t cs) {
    *(cs.ddr) |= (1 << cs.pin);
    *(cs.port) |= (1 << cs.pin); // Idle 상태는 High
}

uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF))); // 전송 완료 대기
    return SPDR;
}

void spi_transaction(spi_cs_t cs, const uint8_t *tx, uint8_t *rx, size_t len) {
    *(cs.port) &= ~(1 << cs.pin); // CS Low
    for (size_t i = 0; i < len; i++) {
        uint8_t b = spi_transfer(tx[i]);
        if (rx) rx[i] = b;
    }
    *(cs.port) |= (1 << cs.pin);  // CS High
}