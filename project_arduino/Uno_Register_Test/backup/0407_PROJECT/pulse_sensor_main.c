//src/pulse_sensor_main.c

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "srclib/my_pulse_sensor.h"

// UART 초기화 (9600 baud 기준)
void UART_Init() {
    UBRR0H = 0;
    UBRR0L = 103; 
    UCSR0B = (1 << TXEN0); // 송신 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8비트 데이터
}

void UART_Transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_PrintString(const char* str) {
    while (*str) UART_Transmit(*str++);
}

int main(void) {
    UART_Init();
    
    PulseSensor mySensor;
    // ADC0(A0핀), 임계값 550으로 설정
    PulseSensor_Init(&mySensor, 0, 550);
    
    UART_PrintString("Atmel C Pulse Sensor Start\r\n");

    while (1) {
        if (PulseSensor_IsBeatDetected(&mySensor)) {
            UART_PrintString("Heart Beat! <3\r\n");
        }
        _delay_ms(20); // 샘플링 간격
    }
}