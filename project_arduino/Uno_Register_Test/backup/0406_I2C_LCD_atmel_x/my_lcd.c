#include "my_LCD.h"
#include "my_I2C.h"

// I2C를 통해 PCF8574로 1바이트 전송
void lcd_send_i2c(uint8_t val) {
    i2c_start(LCD_ADDR << 1);
    i2c_write(val | BL); // 항상 백라이트 비트를 1로 유지
    i2c_stop();
}

// EN(Enable) 핀에 펄스를 주어 데이터를 LCD로 래치(Latch)함
void lcd_pulse(uint8_t val) {
    lcd_send_i2c(val | EN);  // EN High
    _delay_us(1);
    lcd_send_i2c(val & ~EN); // EN Low
    _delay_us(40);
}

// 4비트 단위로 전송 (상위 4비트 데이터가 P4~P7에 위치함)
void lcd_send_4bit(uint8_t val, uint8_t mode) {
    uint8_t high_nibble = (val & 0xF0) | mode;
    uint8_t low_nibble = ((val << 4) & 0xF0) | mode;
    
    lcd_send_i2c(high_nibble);
    lcd_pulse(high_nibble);
    
    lcd_send_i2c(low_nibble);
    lcd_pulse(low_nibble);
}

void lcd_command(uint8_t cmd) { lcd_send_4bit(cmd, 0); } // RS = 0
void lcd_data(uint8_t data) { lcd_send_4bit(data, RS); } // RS = 1

void lcd_init(void) {
    _delay_ms(50); // 전원 안정화 대기
    
    // 4비트 모드 초기화 시퀀스 (HD44780U 매뉴얼 기준)
    lcd_send_i2c(0x30 | 0x04); lcd_send_i2c(0x30); _delay_ms(5);
    lcd_send_i2c(0x30 | 0x04); lcd_send_i2c(0x30); _delay_us(150);
    lcd_send_i2c(0x30 | 0x04); lcd_send_i2c(0x30); _delay_ms(1);
    
    // 진짜 4비트 모드 설정
    lcd_send_i2c(0x20 | 0x04); lcd_send_i2c(0x20); _delay_ms(1);

    lcd_command(0x28); // Function Set: 4-bit, 2-line, 5x8 dots
    lcd_display_on();  // Display ON
    lcd_clear();       // Clear Screen
    lcd_command(0x06); // Entry Mode: Increment
}

void lcd_clear(void) {
    lcd_command(0x01);
    _delay_ms(2); // Clear 명령은 처리가 느림
}

void lcd_display_on(void) { lcd_command(0x0C); } // 0000 1100 (D=1, C=0, B=0)
void lcd_display_off(void) { lcd_command(0x08); } // 0000 1000

void lcd_print(char *str) {
    while(*str) lcd_data(*str++);
}