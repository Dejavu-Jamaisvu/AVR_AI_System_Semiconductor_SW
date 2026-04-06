//main.c
#include <avr/io.h>
#include <util/delay.h>
#include "lcd1602_i2c.h"

int main(void) {
    // I2C 하드웨어(TWI) 초기화 함수 필요
    i2c_init();
    // LCD 초기화
    lcd_init();


    char buffer[16];
    setup_custom_char(); // 하트 등록

    // // 메시지 출력
    // lcd_gotoxy(0, 0);
    // lcd_print("Hello, Andrew!");

    // lcd_gotoxy(0, 1);
    // lcd_print("AVR I2C System");



    // 1. 10초 타이머 시작
    for (int i = 10; i >= 0; i--) {
        lcd_clear();
        lcd_gotoxy(0, 0);
        sprintf(buffer, "Timer: %02d sec", i);
        lcd_print(buffer);

        // 두 번째 줄에 로딩 바 출력 (# 표시)
        lcd_gotoxy(0, 1);
        for(int j=0; j<i; j++) lcd_data('#');

        // 5초 전부터 백라이트(화면) 깜빡이기
        if (i <= 5 && i > 0) {
            lcd_display_off();
            _delay_ms(200);
            lcd_display_on();
            _delay_ms(800);
        } else {
            _delay_ms(1000);
        }
    }

    // 2. 하트와 함께 두 줄이 흐르는 전광판 메시지
    lcd_clear();
    lcd_gotoxy(0, 0);
    lcd_data(0); lcd_print(" Timer Finished! "); lcd_data(0);
    lcd_gotoxy(0, 1);
    lcd_print("  Great Job, Andrew!  ");

    while (1) {
        // 이 함수는 1, 2층 전체를 왼쪽으로 한 칸 밀어버립니다.
        lcd_scroll_left(); 
        _delay_ms(400); // 흐르는 속도
    }

    return 0;
}