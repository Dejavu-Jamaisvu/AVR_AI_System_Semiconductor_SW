//main.c
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "my_i2C.h"


// [추가] 외부 함수 선언
extern void uart_init(void);
extern void uart_print(const char* s);


int main(void) {
    uart_init(); // UART 초기화 (9600bps 등)
    i2c_init();
    
    char buf[32];
    uart_print("\n--- I2C Address Scanner Start ---\n");

    while (1) {
        uint8_t devices_found = 0;

        for (uint8_t addr = 1; addr < 127; addr++) {
            // SLA+W 형태로 주소 생성 (7비트 주소 + Write비트 0)
            uint8_t res = i2c_start(addr << 1);
            
            if (res == 0) { // ACK 응답이 온 경우
                sprintf(buf, "Device found at: 0x%02X\n", addr);
                uart_print(buf);
                devices_found++;
            }
            i2c_stop();
            _delay_ms(10);
        }

        if (devices_found == 0) {
            uart_print("No I2C devices found.\n");
        }
        
        uart_print("Scan Finished. Retrying in 5 seconds...\n");
        _delay_ms(5000);
    }
}