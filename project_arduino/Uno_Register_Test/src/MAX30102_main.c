#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//#define UART_BAUD 115200UL  // 데이터 양이 많으므로 115200 권장
//platformio.ini 파일에 monitor_speed = 115200 작성필요

#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "srclib/my_uart.h"
#include "srclib/my_MAX30102.h"

int main(void) {
    uint32_t red_led = 0;
    uint32_t ir_led = 0;


    
    // 1. UART 초기화 및 printf 연결
    uart_init();
    stdout = uart_get_stdout();
    

    // [추가] 첫 번째 확인 포인트: UART 자체가 작동하는가?
    _delay_ms(1000); // 안정화를 위해 1초 대기
    printf("\n[Step 1] UART 9600 Test Start\n");


    // 2. MAX30102 센서 초기화 (I2C 설정 포함)
    printf("[Step 2] Initializing MAX30102...\n");
    MAX30102_Init();


    // 두 번째 확인 포인트: I2C 통신에서 멈추지 않았는가?
    printf("[Step 3] I2C Init Success!\n");
    
    printf("\n--- MAX30102 Pulse Oximeter Test Start ---\n");
    printf("[Red_Value, IR_Value]\n"); // 시리얼 플로터 헤더

    while (1) {
        // 3. FIFO에서 데이터 읽기 (Red와 IR 데이터 3바이트씩 총 6바이트 읽음)
        MAX30102_Read_FIFO(&red_led, &ir_led);
        
        // 4. 데이터 출력
        // 손가락이 닿았을 때만 출력하거나, 실시간 그래프를 위해 계속 출력
        if (red_led > 10000) { 
            // 시리얼 플로터용 포맷: "값1, 값2"
            printf("[%lu, %lu]\n", red_led, ir_led);
        } else {
            // 손가락이 없을 때 메시지 (필요 시 주석 해제)
            // printf("No Finger Detected\n");
        }
        
        // 5. 샘플링 속도 조절 (10ms = 100Hz)
        _delay_ms(10); 
    }
    
    return 0;
}
            // 여기에 심박수 계산 알고리즘 추가 가능
            // 현재는 원시 데이터만 읽어옴