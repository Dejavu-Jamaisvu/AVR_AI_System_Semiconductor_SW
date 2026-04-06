#ifndef MY_LCD_H_
#define MY_LCD_H_

#include <avr/io.h>
#include <util/delay.h>

#define LCD_ADDR 0x27  // 또는 0x3F (스캐너로 확인한 주소)

// PCF8574 -> LCD 핀 매핑 (비트 위치)
#define RS 0x01  // P0
#define RW 0x02  // P1
#define EN 0x04  // P2
#define BL 0x08  // P3 (Backlight)

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_print(char *str);
void lcd_clear(void);
void lcd_display_on(void);
void lcd_display_off(void);

#endif