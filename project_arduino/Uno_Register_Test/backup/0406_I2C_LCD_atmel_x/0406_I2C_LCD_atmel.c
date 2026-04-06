#include "my_I2C.h"
#include "my_LCD.h"

int main(void) {
    i2c_init();
    lcd_init();

    lcd_print("Hello Professor");
    _delay_ms(2000);

    while(1) {
        lcd_clear();
        lcd_print("LCD 1602 Test");
        _delay_ms(2000);

        lcd_display_off(); // 화면 끄기 테스트
        _delay_ms(1000);
        
        lcd_display_on();  // 화면 켜기 테스트
        _delay_ms(1000);
    }
}