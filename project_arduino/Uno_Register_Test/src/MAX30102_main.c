#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//#define UART_BAUD 115200UL  // 데이터 양이 많으므로 115200 권장
//platformio.ini 파일에 monitor_speed = 115200 작성필요

#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include "srclib/my_uart.h"
#include "srclib/my_MAX30102.h"

// --- 전역 설정 ---
#define LED_PIN   PB5   // 아두이노 우노 내장 LED (13번 핀)
uint32_t current_tick = 0;

// --- 테스트 함수 선언 ---
void run_test0_hw_i2c_check(void);    // [TEST 0] UART 없이 하드웨어/I2C 체크
void run_test1_uart_raw_data(void);   // [TEST 1] UART 출력 및 원시 데이터 확인
void run_test2_bpm_calculation(void); // [TEST 2] 최종 BPM 계산 모드

int main(void) {
    // 0. 기본 포트 설정 (LED 출력용)
    DDRB |= (1 << LED_PIN); 

    // [TEST 0] 하드웨어 연결 상태 확인 (가장 먼저 실행)
    // UART가 안 나와도 LED 깜빡임으로 상태를 알 수 있음
    run_test0_hw_i2c_check();

    // [TEST 1] UART 통신 및 데이터 스트리밍 확인
    // 여기서부터는 시리얼 모니터(9600)가 필요함
    run_test1_uart_raw_data();

    // [TEST 2] 심박수(BPM) 계산 모드 진입
    // 실시간 파형과 계산된 수치를 출력
    run_test2_bpm_calculation();

    return 0;
}

/**
 * [TEST 0] 하드웨어 및 I2C 기초 연결 체크
 * UART 없이 LED 상태만으로 연결 성공 여부 판단
 */
void run_test0_hw_i2c_check(void) {
    I2C_Init();
    
    // 센서 리셋 시도
    MAX30102_Write(0x09, 0x40); 
    _delay_ms(100);

    // ID 읽기 시도 (0xFF 주소의 값이 0x15인지 확인)
    uint8_t id = MAX30102_Read(0xFF);

    if (id == 0x15) {
        // 성공: LED를 3번 빠르게 깜빡임
        for(int i=0; i<3; i++) {
            PORTB |= (1 << LED_PIN); _delay_ms(100);
            PORTB &= ~(1 << LED_PIN); _delay_ms(100);
        }
    } else {
        // 실패: LED를 계속 켜둠 (배선 확인 필요)
        PORTB |= (1 << LED_PIN); 
        while(1); 
    }
}

/**
 * [TEST 1] UART 출력 및 센서 원시 데이터 확인
 */
void run_test1_uart_raw_data(void) {
    uart_init();
    stdout = uart_get_stdout();
    _delay_ms(500);

    printf("\n--- [TEST 1] UART & RAW DATA OK ---\n");
    MAX30102_Init(); // 센서 설정 초기화
    
    printf("Reading 50 samples of Raw Data...\n");
    for(int i=0; i<50; i++) {
        uint32_t r, ir;
        MAX30102_Read_FIFO(&r, &ir);
        printf("Sample %d -> R:%lu, I:%lu\n", i, r, ir);
        _delay_ms(20);
    }
    printf("--- [TEST 1] PASSED ---\n\n");
}

/**
 * [TEST 2] 최종 BPM 계산 및 그래프 출력 모드
 */
void run_test2_bpm_calculation(void) {
    uint32_t red, ir, last_ir = 0, last_beat_tick = 0;
    bool rising = false;

    printf("--- [TEST 2] BPM MONITORING MODE ---\n");
    printf("Format: [RED, IR, BPM]\n");

    while (1) {
        MAX30102_Read_FIFO(&red, &ir);

        if (ir > 50000) { // 손가락 감지 시
            if (ir > last_ir) rising = true;
            else if (ir < last_ir && rising) {
                rising = false;
                uint32_t duration = current_tick - last_beat_tick;
                
                if (duration > 40) { // 노이즈 필터링
                    int bpm = 6000 / duration;
                    // 시리얼 플로터용 출력
                    printf("%lu, %lu, %d\n", red, ir, bpm);
                    last_beat_tick = current_tick;
                    
                    // 비트 감지 시 LED 짧게 깜빡
                    PORTB |= (1 << LED_PIN); _delay_ms(5); PORTB &= ~(1 << LED_PIN);
                }
            }
        } else {
            if (current_tick % 100 == 0) printf("Waiting for Finger...\n");
        }

        last_ir = ir;
        current_tick++;
        _delay_ms(10);
    }
}