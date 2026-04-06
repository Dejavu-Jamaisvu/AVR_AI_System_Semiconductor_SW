//my_rfid.c
#include <avr/io.h>
#include <util/delay.h>
#include "my_rfid.h"

static spi_cs_t _rf_cs;

// RC522 레지스터 쓰기
void rc522_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { (reg << 1) & 0x7E, val };
    spi_transaction(_rf_cs, buf, NULL, 2);
}

// RC522 레지스터 읽기
uint8_t rc522_read(uint8_t reg) {
    uint8_t tx[2] = { ((reg << 1) & 0x7E) | 0x80, 0x00 };
    uint8_t rx[2];
    spi_transaction(_rf_cs, tx, rx, 2);
    return rx[1];
}

// 버전 확인용 함수
uint8_t rfid_get_version(void) {
    return rc522_read(0x37); // VersionReg
}

void rfid_init(spi_cs_t cs) {
    _rf_cs = cs;
    spi_cs_init(_rf_cs);

    // 1. 하드웨어 리셋 (D9 / PB1)
    DDRB |= (1 << PB1);
    PORTB &= ~(1 << PB1); _delay_ms(50);
    PORTB |= (1 << PB1);  _delay_ms(50);

    // 2. 소프트웨어 리셋
    rc522_write(0x01, 0x0F); // CommandReg -> SoftReset
    _delay_ms(50);

    // 3. 타이머 설정 (안정적인 통신을 위한 기본값)
    rc522_write(0x2A, 0x8D); // TModeReg
    rc522_write(0x2B, 0x3E); // TPrescalerReg
    rc522_write(0x2D, 30);   // TReloadRegL
    rc522_write(0x2C, 0);    // TReloadRegH
    
    rc522_write(0x15, 0x40); // TxASKReg (100% ASK)
    rc522_write(0x11, 0x3D); // ModeReg (CRC 초기값 0x6363)
    
    // 4. 수신 감도 설정 (Gain)
    rc522_write(0x26, 0x70); // RFCfgReg (최대 감도)
    
    // 5. 안테나 ON
    uint8_t current = rc522_read(0x14); // TxControlReg
    rc522_write(0x14, current | 0x03);
    _delay_ms(10);
}

int rfid_read_uid(rfid_uid_t *uid) {
    // --- 1단계: REQA (카드가 있는지 호출) ---
    rc522_write(0x01, 0x00);      // CommandReg -> Idle
    rc522_write(0x0A, 0x80);      // FIFOLevelReg -> Flush FIFO
    rc522_write(0x04, 0x7F);      // ComIrqReg -> Clear IRQs
    
    rc522_write(0x0D, 0x07);      // BitFramingReg -> 7비트 준비
    rc522_write(0x09, 0x26);      // FIFODataReg -> PICC_REQA
    rc522_write(0x01, 0x0C);      // CommandReg -> Transceive
    rc522_write(0x0D, 0x87);      // Start 전송 시작 (7비트)
    _delay_ms(10);

    // 응답이 없으면 종료 (카드가 근처에 없음)
    if (rc522_read(0x0A) == 0) return 0;

    // --- 2단계: Anticollision (충돌 방지 및 UID 읽기) ---
    rc522_write(0x01, 0x00);      // Idle
    rc522_write(0x0A, 0x80);      // Flush FIFO
    rc522_write(0x04, 0x7F);      // Clear IRQs
    
    rc522_write(0x0D, 0x00);      // BitFramingReg -> 다시 8비트(0번 비트까지) 설정
    rc522_write(0x09, 0x93);      // FIFODataReg -> PICC_ANTICOLL
    rc522_write(0x09, 0x20);      // NVB (2바이트 전송)
    
    rc522_write(0x01, 0x0C);      // CommandReg -> Transceive
    rc522_write(0x0D, 0x80);      // Start 전송 시작
    _delay_ms(10);

    // --- 3단계: 결과 처리 ---
    uint8_t n = rc522_read(0x0A); // FIFO에 몇 바이트 들어왔나?
    
    // 정상적인 UID는 4바이트 + 체크섬 1바이트 = 총 5바이트가 들어옴
    if (n >= 5) {
        uid->size = 4;
        for (uint8_t i = 0; i < 4; i++) {
            uid->bytes[i] = rc522_read(0x09); // FIFO에서 UID 4바이트 추출
        }
        rc522_read(0x09); // 마지막 체크섬 바이트는 읽어서 FIFO 비우기
        return 1; // 읽기 성공
    }
    
    return 0; // 실패
}