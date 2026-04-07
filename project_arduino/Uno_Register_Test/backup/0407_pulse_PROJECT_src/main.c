#include <util/delay.h>
#include "max30102_i2c.h"
//#include "UART0.h"
#include "lcd1602_i2c.h"

volatile uint32_t red_led = 0;
volatile uint32_t ir_led = 0;
volatile float heart_bps = 0;
volatile float spo2_data = 0;

volatile int8_t temp_int = 0;
volatile uint8_t temp_frac = 0;

volatile char buffer[40] = { 0 };

int main() {
    max30102_init();
    //UART0_init(); //나중에 필요시 활용
    lcd_init();

    uint8_t status;

    while (1) {
        max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);

        // if data exists
        if (status & 0xC0) {
            max30102_read_fifo(&red_led, &ir_led);
            max30102_calculate_heart_and_spo2(red_led, ir_led, &heart_bps, &spo2_data);

            static uint16_t display_count = 0;
            if (++display_count >= 25) {
                max30102_read_temperature(&temp_int, &temp_frac);

                lcd_clear();

                lcd_gotoxy(0, 0);
                sprintf(buffer, "BPS:%3d SPO2:%3d", (int)heart_bps, (int)spo2_data);
                lcd_print(buffer);

                lcd_gotoxy(0, 1);
                sprintf(buffer, "TEMP:%d.%d C", temp_int, temp_frac);
                lcd_print(buffer);

                display_count = 0;
            }
        }
    }
}