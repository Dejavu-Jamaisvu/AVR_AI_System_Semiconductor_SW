//src/main_lie_detector.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "max30102_i2c.h"
#include "lcd1602_i2c.h"
#include "my_servo.h"

volatile uint16_t timer_ticks = 0;
float base_bpm = 0;
#define BUTTON_PIN PB0 

ISR(TIMER1_OVF_vect) {
    timer_ticks++;
}

uint8_t is_button_pressed() {
    if (!(PINB & (1 << BUTTON_PIN))) {
        _delay_ms(50);
        if (!(PINB & (1 << BUTTON_PIN))) return 1;
    }
    return 0;
}

int main(void) {
    // 초기화
    i2c_init();
    lcd_init();
    max30102_init();
    Servo_Init();
    
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB |= (1 << BUTTON_PIN);

    sei();

    uint32_t red_led, ir_led;
    float current_bpm = 0, spo2 = 0;
    char buffer[16];
    uint8_t status;

    // 초기 상태 설정
    Servo_Write(90);
    lcd_clear();
    lcd_print("1. Push D8 Calib");

    while (1) {
        // [핵심] 버튼 누름 여부와 관계없이 센서 데이터는 항상 읽음
        max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);
        if (status & 0xC0) {
            max30102_read_fifo(&red_led, &ir_led);
            max30102_calculate_heart_and_spo2(red_led, ir_led, &current_bpm, &spo2);
        }

        // --- 1단계: 보정 시작 대기 (IDLE) ---
        if (base_bpm == 0) {
            if (is_button_pressed()) {
                lcd_clear();
                lcd_print("Calibrating...");
                
                float sum_bpm = 0;
                int count = 0;
                // 이미 센서가 돌고 있으므로 즉시 10초간 샘플링
                for (int i = 0; i < 100; i++) {
                    max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);
                    if (status & 0xC0) {
                        max30102_read_fifo(&red_led, &ir_led);
                        max30102_calculate_heart_and_spo2(red_led, ir_led, &current_bpm, &spo2);
                        if (current_bpm > 40) {
                            sum_bpm += current_bpm;
                            count++;
                        }
                    }
                    sprintf(buffer, "Progress: %d%%", i * 2);
                    lcd_gotoxy(0, 1);
                    lcd_print(buffer);
                    _delay_ms(100);
                }
                base_bpm = (count > 0) ? (sum_bpm / count) : 75.0;
                lcd_clear();
                lcd_gotoxy(0, 0);
                sprintf(buffer, "Ready! Base:%d", (int)base_bpm);
                lcd_print(buffer);
                lcd_gotoxy(0, 1);
                lcd_print("Push to Test");
            }
        } 
        
        // --- 2단계: 테스트 실행 (READY & TEST) ---
        else {
            if (is_button_pressed()) {
                lcd_clear();
                lcd_print("Analyzing...");
                
                // 버튼을 누른 즉시 현재 계산된 BPM으로 판정 (대기 시간 대폭 단축)
                // 센서가 이미 백그라운드에서 돌고 있었으므로 즉시 값을 얻을 수 있음
                _delay_ms(500); // 아주 짧은 안정화 시간
                
                max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);
                if (status & 0xC0) {
                    max30102_read_fifo(&red_led, &ir_led);
                    max30102_calculate_heart_and_spo2(red_led, ir_led, &current_bpm, &spo2);
                }

                lcd_gotoxy(0, 1);
                sprintf(buffer, "BPM: %d", (int)current_bpm);
                lcd_print(buffer);

                if (current_bpm > (base_bpm + 12)) {
                    lcd_gotoxy(0, 0);
                    lcd_print("RESULT: LIE!!  ");
                    Servo_Write(135);
                } else {
                    lcd_gotoxy(0, 0);
                    lcd_print("RESULT: TRUTH  ");
                    Servo_Write(45);
                }
                
                _delay_ms(4000); // 결과 확인 시간
                Servo_Write(90); // 원위치
                
                // 다시 초기 화면으로
                lcd_clear();
                lcd_gotoxy(0, 0);
                sprintf(buffer, "Base:%d Push Test", (int)base_bpm);
                lcd_print(buffer);
            }
        }
        
        // 루프 속도 조절 (너무 빠르면 I2C 버하가 걸림)
        _delay_ms(10);
    }
}


// #define F_CPU 16000000UL
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <util/delay.h>
// #include <stdio.h>
// #include "max30102_i2c.h"
// #include "lcd1602_i2c.h"
// #include "my_servo.h"

// // 전역 변수 및 설정
// volatile uint16_t timer_ticks = 0; // 서보 제어용 틱
// float base_bpm = 0;
// #define BUTTON_PIN PB0 // D8 핀

// // 서보 제어용 타이머1 오버플로 인터럽트
// ISR(TIMER1_OVF_vect) {
//     timer_ticks++;
// }

// // 버튼 입력 확인 (내부 풀업 사용, 눌리면 0)
// uint8_t is_button_pressed() {
//     if (!(PINB & (1 << BUTTON_PIN))) {
//         _delay_ms(50); // 디바운스
//         if (!(PINB & (1 << BUTTON_PIN))) return 1;
//     }
//     return 0;
// }

// int main(void) {
//     // 1. 초기화 (순서가 중요합니다)
//     i2c_init();           // I2C 먼저
//     lcd_init();           // 그 다음 LCD
//     max30102_init();      // 센서 (내부에서 timer0_init() 호출됨)
//     Servo_Init();         // 서보 (Timer1 설정)
    
//     // 버튼 핀 D8(PB0) 설정
//     DDRB &= ~(1 << BUTTON_PIN);
//     PORTB |= (1 << BUTTON_PIN);

//     sei(); // 모든 인터럽트(Timer0, Timer1) 허용

//     uint32_t red_led, ir_led;
//     float current_bpm = 0, spo2 = 0;
//     char buffer[16];
//     uint8_t status;

//     // --- 1단계: IDLE ---
//     lcd_clear();
//     lcd_print("1. Push D8 Calib");
//     Servo_Write(90); // 중간 대기

//     while (!is_button_pressed()); // 버튼 누를 때까지 대기

//     // --- 2단계: CALIBRATION ---
//     lcd_clear();
//     lcd_print("Calibrating...");
//     float sum_bpm = 0;
//     int count = 0;

//     for (int i = 0; i < 100; i++) { // 약 10초간 측정
//         max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);
//         if (status & 0xC0) {
//             max30102_read_fifo(&red_led, &ir_led);
//             max30102_calculate_heart_and_spo2(red_led, ir_led, &current_bpm, &spo2);
//             if (current_bpm > 40) {
//                 sum_bpm += current_bpm;
//                 count++;
//             }
//         }
//         sprintf(buffer, "Progress: %d%%", i);
//         lcd_gotoxy(0, 1);
//         lcd_print(buffer);
//         _delay_ms(100);
//     }
//     base_bpm = (count > 0) ? (sum_bpm / count) : 75.0;

//     // --- 3단계: READY & TEST ---
//     while (1) {
//         lcd_clear();
//         lcd_gotoxy(0, 0);
//         sprintf(buffer, "Ready! Base:%d", (int)base_bpm);
//         lcd_print(buffer);
//         lcd_gotoxy(0, 1);
//         lcd_print("Push to Test");

//         while (!is_button_pressed()); // 버튼 누르면 테스트 시작

//         lcd_clear();
//         lcd_print("Analyzing...");
        
//         // 3초간 실시간 심박 측정
//         for (int i = 0; i < 50; i++) {
//             max30102_read_reg(MAX30102_REG_INTR_STATUS_1, &status);
//             if (status & 0xC0) {
//                 max30102_read_fifo(&red_led, &ir_led);
//                 max30102_calculate_heart_and_spo2(red_led, ir_led, &current_bpm, &spo2);
//             }
//             _delay_ms(100);
//         }

//         // --- 4단계: 결과 판정 ---
//         lcd_gotoxy(0, 1);
//         sprintf(buffer, "BPM: %d", (int)current_bpm);
//         lcd_print(buffer);

//         if (current_bpm > (base_bpm + 5)) {
//             lcd_gotoxy(0, 0);
//             lcd_print("RESULT: LIE!!  ");
//             Servo_Write(135); // 거짓 (135도)
//         } else {
//             lcd_gotoxy(0, 0);
//             lcd_print("RESULT: TRUTH  ");
//             Servo_Write(45);   // 진실 (45도)
//         }
        
//         _delay_ms(5000); // 5초간 결과 보여줌
//         Servo_Write(90); // 다시 대기 상태로
//     }
// }