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

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include "srclib/my_uart.h"
#include "srclib/my_MAX30102.h"

// --- 전역 설정 ---
#define LED_PIN   PB5   // 아두이노 13번 LED
uint32_t current_tick = 0;

// --- 함수 선언 ---
void run_test0_hw_i2c_check(void);
void run_test1_uart_raw_streaming(void);
void run_test2_bpm_stable_monitoring(void);

int main(void) {
    // 기본 초기화
    DDRB |= (1 << LED_PIN); 
    uart_init();
    stdout = uart_get_stdout();
    _delay_ms(500);

    // [선택] 실행하고 싶은 테스트 하나만 주석을 해제하세요.
    
    // run_test0_hw_i2c_check();          // 연결 확인 (LED 깜빡임)
    // run_test1_uart_raw_streaming();     // 원시 데이터 무한 출력
    run_test2_bpm_stable_monitoring();  // 평균 필터 적용 BPM 무한 측정

    while (1); 
    return 0;
}

/**
 * [TEST 0] 하드웨어 연결 체크 (LED 신호)
 */
void run_test0_hw_i2c_check(void) {
    I2C_Init();
    MAX30102_Write(0x09, 0x40); // 리셋
    _delay_ms(100);

    while(1) { // 무한 반복 체크
        uint8_t id = MAX30102_Read(0xFF);
        if (id == 0x15) {
            PORTB |= (1 << LED_PIN); _delay_ms(200);
            PORTB &= ~(1 << LED_PIN); _delay_ms(200);
        } else {
            PORTB |= (1 << LED_PIN); // 에러 시 계속 켜짐
        }
    }
}

/**
 * [TEST 1] 원시 데이터 무한 스트리밍 (시리얼 플로터용)
 */
void run_test1_uart_raw_streaming(void) {
    printf("\n--- [TEST 1] RAW DATA STREAMING START ---\n");
    MAX30102_Init();
    
    uint32_t r, ir;
    while(1) {
        MAX30102_Read_FIFO(&r, &ir);
        // 시리얼 플로터에서 RED와 IR을 동시에 그리기 위한 형식
        printf("%lu,%lu\n", r, ir);
        _delay_ms(10);
    }
}

// --- SpO2 및 BPM 계산을 위한 구조체 (데이터 관리용) ---
typedef struct {
    uint32_t red;
    uint32_t ir;
    float ir_avg;
    float red_avg;
    int bpm;
    float spo2;
} HealthData;

// --- 내부 계산 함수 (모듈화) ---
void calculate_health_metrics(HealthData *data, uint32_t last_beat_tick);

/**
 * [TEST 2] 통합 모니터링 모드 (BPM + SpO2)
 * 한 번 실행하면 무한 루프로 정밀 측정을 수행합니다.
 */
void run_test2_bpm_stable_monitoring(void) {
    HealthData current = {0, 0, 0, 0, 0, 98.0};
    uint32_t last_beat_tick = 0;
    bool rising = false;
    float last_ir_avg = 0;

    int bpm_buf[5] = {0};
    int b_idx = 0;

    printf("\n--- [TEST 2] STABLE MONITORING ---\n");
    printf("TIME(ms), IR_AVG, BPM, SpO2(%%)\n");
    
    MAX30102_Init();

    while (1) {
        MAX30102_Read_FIFO(&current.red, &current.ir);
        
        // 필터 계수 조정 (조금 더 부드럽게)
        current.red_avg = (current.red_avg * 0.9) + (current.red * 0.1);
        current.ir_avg = (current.ir_avg * 0.9) + (current.ir * 0.1);

        // 손가락 감지 임계값을 20000으로 하향 조정
        if (current.ir > 20000) {
            if (current.ir_avg > last_ir_avg) {
                rising = true;
            } 
            else if (current.ir_avg < last_ir_avg && rising) {
                rising = false; 
                uint32_t duration = current_tick - last_beat_tick;
                
                if (duration >= 40 && duration <= 150) {
                    int raw_bpm = 6000 / duration;
                    bpm_buf[b_idx] = raw_bpm;
                    b_idx = (b_idx + 1) % 5;
                    
                    int sum = 0;
                    for(int i=0; i<5; i++) sum += bpm_buf[i];
                    current.bpm = sum / 5;

                    calculate_health_metrics(&current, last_beat_tick);

                    // [%] 출력을 위해 float 대신 (정수.소수점) 방식으로 안전하게 출력
                    int spo2_int = (int)current.spo2;
                    int spo2_dec = (int)((current.spo2 - spo2_int) * 10);

                    printf("%lu, %lu, %d, %d.%d%%\n", 
                            current_tick * 10, (uint32_t)current.ir_avg, current.bpm, spo2_int, spo2_dec);
                    
                    PORTB |= (1 << LED_PIN); _delay_ms(10); PORTB &= ~(1 << LED_PIN);
                    last_beat_tick = current_tick;
                }
            }
        } else {
            // 메시지가 너무 자주 뜨지 않게 조절
            if (current_tick % 200 == 0) printf(">> Place Finger...\n");
            last_beat_tick = current_tick;
        }

        last_ir_avg = current.ir_avg;
        current_tick++;
        _delay_ms(10); 
    }
}

/**
 * 산소포화도(SpO2) 상세 계산 모듈
 */
void calculate_health_metrics(HealthData *data, uint32_t last_beat_tick) {
    // AC 성분(변화량)과 DC 성분(평균값)의 비율 계산
    float red_ac = (float)data->red - data->red_avg;
    float ir_ac = (float)data->ir - data->ir_avg;
    
    // R = (Red_AC / Red_DC) / (IR_AC / IR_DC)
    float R = (fabs(red_ac) / data->red_avg) / (fabs(ir_ac) / data->ir_avg);
    
    // SpO2 근사식 적용
    float spo2_val = 110.0 - (25.0 * R);
    
    // 수치 안정화 (보간법)
    if (spo2_val > 100.0) spo2_val = 100.0;
    if (spo2_val < 85.0) spo2_val = 85.0;
    
    data->spo2 = (data->spo2 * 0.7) + (spo2_val * 0.3);
}

/**
 * [TEST 2] 평균 필터 기반 안정적 BPM 측정 (무한 루프)
 */

//"이동 평균 (Moving Average)" 한 번 더!
//지금은 한 번 박동할 때마다 바로 BPM을 계산 -> 최근 5번의 BPM 평균값 
// void run_test2_bpm_stable_monitoring(void) {
//     uint32_t red, ir;
//     uint32_t ir_avg = 0, last_ir_avg = 0;
//     uint32_t last_beat_tick = 0;
//     bool rising = false;
    
//     int bpm_buffer[5] = {0};
//     int bpm_idx = 0;
//     int stable_bpm = 0;

//     printf("\n--- [TEST 2] STABLE BPM MONITORING START ---\n");
//     printf("IR_AVG, RAW_BPM, STABLE_BPM\n");
    
//     MAX30102_Init();

//     while (1) {
//         MAX30102_Read_FIFO(&red, &ir);
        
//         // 지수 이동 평균 필터 (데이터 부드럽게)
//         ir_avg = (last_ir_avg * 7 + ir * 3) / 10;

//         if (ir > 30000) { // 손가락 감지 임계값
//             if (ir_avg > last_ir_avg) {
//                 rising = true;
//             } 
//             else if (ir_avg < last_ir_avg && rising) {
//                 rising = false; 
//                 uint32_t duration = current_tick - last_beat_tick;
                
//                 // 현실적인 심박 간격 (0.5초 ~ 1.2초 사이)
//                 if (duration >= 50 && duration <= 120) {
//                     int raw_bpm = 6000 / duration;
                    
//                     // 최근 5개 값 평균화 (정확도 향상)
//                     bpm_buffer[bpm_idx] = raw_bpm;
//                     bpm_idx = (bpm_idx + 1) % 5;
                    
//                     int sum = 0;
//                     for(int i=0; i<5; i++) sum += bpm_buffer[i];
//                     stable_bpm = sum / 5;

//                     printf("%lu, %d, %d\n", ir_avg, raw_bpm, stable_bpm);
                    
//                     // 심박 시 LED 반짝임
//                     PORTB |= (1 << LED_PIN); _delay_ms(10); PORTB &= ~(1 << LED_PIN);
//                     last_beat_tick = current_tick;
//                 }
//             }
//         } else {
//             // 손가락 없을 때 1초마다 대기 메시지
//             if (current_tick % 100 == 0) printf("Waiting for finger...\n");
//             last_beat_tick = current_tick; // 시간 초기화
//         }

//         last_ir_avg = ir_avg;
//         current_tick++;
//         _delay_ms(10); 
//     }
// }

/**
 * [TEST 2] 최종 BPM 계산 및 그래프 출력 모드
 */
// void run_test2_bpm_calculation(void) {
//     uint32_t red, ir, last_ir = 0, last_beat_tick = 0;
//     bool rising = false;

//     printf("--- [TEST 2] BPM MONITORING MODE ---\n");
//     printf("Format: [RED, IR, BPM]\n");

//     while (1) {
//         MAX30102_Read_FIFO(&red, &ir);

//         if (ir > 50000) { // 손가락 감지 시
//             if (ir > last_ir) rising = true;
//             else if (ir < last_ir && rising) {
//                 rising = false;
//                 uint32_t duration = current_tick - last_beat_tick;
                
//                 if (duration > 40) { // 노이즈 필터링
//                     int bpm = 6000 / duration;
//                     // 시리얼 플로터용 출력
//                     printf("%lu, %lu, %d\n", red, ir, bpm);
//                     last_beat_tick = current_tick;
                    
//                     // 비트 감지 시 LED 짧게 깜빡
//                     PORTB |= (1 << LED_PIN); _delay_ms(5); PORTB &= ~(1 << LED_PIN);
//                 }
//             }
//         } else {
//             if (current_tick % 100 == 0) printf("Waiting for Finger...\n");
//         }

//         last_ir = ir;
//         current_tick++;
//         _delay_ms(10);
//     }
// }

//임계값을 낮추고, 9600bps의 느린 속도 때문에 밀리는 데이터를 처리하기 위해 샘플링 속도를 조정
// void run_test2_bpm_calculation(void) {
//     uint32_t red, ir;
//     uint32_t ir_avg = 0, last_ir_avg = 0;
//     uint32_t last_beat_tick = 0;
//     bool rising = false;
//     int beat_count = 0;

//     printf("--- [TEST 2] STABILIZED MODE ---\n");
//     // 시리얼 플로터에서 보기 편하게 헤더 출력
//     printf("RED,IR,BPM\n"); 

//     while (1) {
//         MAX30102_Read_FIFO(&red, &ir);

//         // 1. 노이즈를 줄이기 위한 지수 이동 평균 (Exponential Moving Average)
//         // 이전 값 70% + 새 값 30%를 섞어 부드럽게 만듭니다.
//         ir_avg = (last_ir_avg * 7 + ir * 3) / 10;

//         // 2. 손가락 감지 기준 하향 (30000 이상이면 일단 잡힌 것으로 간주)
//         if (ir > 30000) {
//             // 3. 피크 검출
//             if (ir_avg > last_ir_avg) {
//                 rising = true;
//             } 
//             else if (ir_avg < last_ir_avg && rising) {
//                 rising = false; 
                
//                 uint32_t duration = current_tick - last_beat_tick;
                
//                 // 4. 현실적인 BPM 필터 (60~120 BPM 사이일 때만 출력)
//                 // 10ms tick 기준: 100(60BPM) ~ 50(120BPM)
//                 if (duration >= 50 && duration <= 100) {
//                     int bpm = 6000 / duration;
                    
//                     // 데이터가 너무 튀는 것을 방지하기 위해 5번 중 1번만 출력하거나 
//                     // 혹은 확실한 값만 출력
//                     printf("%lu,%lu,%d\n", red, ir_avg, bpm);
                    
//                     PORTB |= (1 << LED_PIN); _delay_ms(20); PORTB &= ~(1 << LED_PIN);
//                     last_beat_tick = current_tick;
//                 }
//             }
//         } 
//         // 'Waiting' 메시지가 너무 자주 뜨면 그래프를 가리므로 2초에 한 번만 출력
//         else if (current_tick % 200 == 0) {
//             // printf("Waiting...\n"); // 주석 처리하면 그래프가 더 깔끔하게 보입니다.
//         }

//         last_ir_avg = ir_avg;
//         current_tick++;
//         _delay_ms(10); 
//     }
// }
