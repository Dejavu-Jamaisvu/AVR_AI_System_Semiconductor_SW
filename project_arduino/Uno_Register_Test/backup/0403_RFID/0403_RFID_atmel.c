//main.c

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "my_uart.h"
#include "my_spi.h"
#include "my_rfid.h"

// PlatformIO 빌드 호환용
void setup() {
    uart_init();
    stdout = uart_get_stdout();

    printf("\r\n============================\r\n");
    printf("   RFID Reader Booting...   \r\n");
    printf("============================\r\n");

    spi_init();
    spi_cs_t rfid_cs = { &DDRB, &PORTB, PB2 }; 
    rfid_init(rfid_cs);

    uint8_t ver = rfid_get_version();
    printf("RC522 Firmware Ver: 0x%02X\n", ver);
    printf("Ready! Please scan your card...\n");
}

void loop() {
    rfid_uid_t my_uid;
    if (rfid_read_uid(&my_uid)) {
        // 문구 앞에 \r\n을 넣어 이전 줄의 잔상을 확실히 밀어냅니다.
        printf("\r\n >>> CARD DETECTED! <<<"); 
        printf("\r\n ID: %02X %02X %02X %02X", 
               my_uid.bytes[0], my_uid.bytes[1], 
               my_uid.bytes[2], my_uid.bytes[3]);
        printf("\r\n ------------------------ \r\n");
        
        // fflush(stdout); // AVR 환경에 따라 필요할 수 있음
        _delay_ms(800); 
    }
}

int main(void) {
    setup();
    while (1) {
        loop();
    }
    return 0;
}