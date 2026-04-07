
//src/main.c

//#define UART_BAUD 115200UL  // 데이터 양이 많으므로 115200 권장
//platformio.ini 파일에 monitor_speed = 115200 작성필요

//============인터럽트 사용===============
#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "srclib/my_uart.h"
#include "srclib/my_MAX30102.h"

// --- [1. 구조체 및 전역 변수] ---
typedef struct {
    uint32_t red; uint32_t ir;
    float ir_avg; float red_avg;
    int bpm; float spo2;
} HealthData;

typedef struct { float w; float alpha; } HighPassFilter;
typedef struct { uint32_t last_beat_tick; bool rising; } BeatDetector;

#define LED_PIN PB5
volatile bool data_ready = false; 
uint32_t global_tick = 0;

ISR(INT0_vect) { data_ready = true; }

// --- [2. 알고리즘 모듈] ---

// 필터 계수를 0.90으로 조정하여 빠른 변화에 대응
float apply_filter(HighPassFilter *f, uint32_t raw_val) {
    float prev_w = f->w;
    f->w = (float)raw_val + (f->alpha * prev_w);
    return f->w - prev_w;
}

void update_health_stats(HealthData *d) {
    // 평균값 추적을 더 부드럽게 (0.95 : 0.05)
    d->red_avg = (d->red_avg * 0.98) + (d->red * 0.02);
    d->ir_avg = (d->ir_avg * 0.98) + (d->ir * 0.02);
    
    float red_ac = (float)d->red - d->red_avg;
    float ir_ac = (float)d->ir - d->ir_avg;
    
    if (d->ir_avg > 0) {
        float R = (fabs(red_ac) / d->red_avg) / (fabs(ir_ac) / d->ir_avg);
        float s = 110.0 - (25.0 * R);
        if (s > 100.0) s = 100.0; if (s < 88.0) s = 88.0; // 하한선을 88로 상향
        
        // SpO2 값의 급격한 변화 방지 (느리게 반영)
        d->spo2 = (d->spo2 * 0.9) + (s * 0.1);
    }
}

// 박동 분석 엔진 (문턱값 및 범위 최적화)
bool process_pulse_logic(HealthData *d, HighPassFilter *f, BeatDetector *bd, float *last_f) {
    float cur_f = apply_filter(f, d->ir);
    bool detected = false;

    if (cur_f > *last_f) bd->rising = true;
    else if (cur_f < *last_f && bd->rising) {
        // 문턱값을 30으로 높여 미세 노이즈 무시
        if (cur_f > 30) { 
            bd->rising = false;
            uint32_t dur = global_tick - bd->last_beat_tick;
            
            // 정상적인 인간의 맥박 범위 (40~130 BPM)에 집중
            if (dur >= 45 && dur <= 150) { 
                int raw = 6000 / dur;
                // BPM 튀는 현상 방지 (이전 값 비중 확대)
                d->bpm = (d->bpm == 0) ? raw : (d->bpm * 0.85) + (raw * 0.15);
                update_health_stats(d);
                detected = true;
            }
            bd->last_beat_tick = global_tick;
        }
    }
    *last_f = cur_f;
    return detected;
}

// --- [3. 메인 로직] ---

void init_all(void) {
    DDRB |= (1 << LED_PIN);
    uart_init();
    stdout = uart_get_stdout();
    _delay_ms(200);

    DDRD &= ~(1 << PD2); 
    PORTD |= (1 << PD2); 
    EICRA |= (1 << ISC01); 
    EIMSK |= (1 << INT0);  
    sei();

    MAX30102_Init();
    MAX30102_Read_Interrupt_Status(); 
    printf("\n[ SYSTEM CALIBRATED ]\n");
}

int main(void) {
    init_all();

    HealthData h_data = {0, 0, 0, 0, 0, 98.0};
    HighPassFilter ir_hpf = {0, 0.90}; // 필터 알파값 0.90으로 조정
    BeatDetector detector = {0, false};
    float last_v = 0;
    bool is_on = false;
    uint16_t watchdog = 0;

    while (1) {
        if (data_ready) {
            data_ready = false;
            watchdog = 0;

            MAX30102_Read_FIFO(&h_data.red, &h_data.ir);
            MAX30102_Read_Interrupt_Status();

            // 히스테리시스 구간 (30000 ~ 22000)
            if (h_data.ir > 30000) {
                if (!is_on) {
                    detector.last_beat_tick = global_tick;
                    is_on = true;
                    printf("\n>>> Measuring...\n");
                }

                if (process_pulse_logic(&h_data, &ir_hpf, &detector, &last_v)) {
                    // SpO2가 너무 낮게 나오면 출력 시 보정 (디스플레이용)
                    printf("BPM: %d | SpO2: %d%%\n", h_data.bpm, (int)h_data.spo2);
                    PORTB |= (1 << LED_PIN); _delay_ms(15); PORTB &= ~(1 << LED_PIN);
                }
            } 
            else if (h_data.ir < 22000) {
                if (is_on) {
                    printf("\n>>> Stop. Resetting...\n");
                    ir_hpf.w = 0; h_data.bpm = 0; h_data.spo2 = 98.0;
                    is_on = false;
                }
                if (global_tick % 400 == 0) printf("Ready for Finger...\r");
            }
            global_tick++;
        } 
        else {
            _delay_ms(1);
            if (++watchdog > 3000) {
                MAX30102_Read_Interrupt_Status();
                watchdog = 0;
            }
        }
    }
}

//============인터럽트 사용전===============
// #ifndef UART_BAUD
// #define UART_BAUD 9600UL
// #endif
// #ifndef F_CPU
// #define F_CPU 16000000UL
// #endif

// #include <avr/io.h>
// #include <util/delay.h>
// #include <stdio.h>
// #include <stdbool.h>
// #include <math.h>
// #include "srclib/my_uart.h"
// #include "srclib/my_MAX30102.h"

// // --- 구조체 정의 ---
// typedef struct {
//     uint32_t red;
//     uint32_t ir;
//     float ir_avg;
//     float red_avg;
//     int bpm;
//     float spo2;
// } HealthData;

// typedef struct {
//     float w;
//     float alpha;
// } HighPassFilter;

// typedef struct {
//     int bpm_buffer[5];
//     int idx;
//     uint32_t last_beat_tick;
//     bool rising;
// } BeatDetector;

// #define LED_PIN PB5
// uint32_t current_tick = 0;

// // --- 핵심 보조 함수들 ---

// float update_high_pass(HighPassFilter *f, uint32_t x) {
//     float prev_w = f->w;
//     f->w = (float)x + (f->alpha * prev_w);
//     return f->w - prev_w;
// }

// float process_health_data(HealthData *h_data, HighPassFilter *ir_hpf) {
//     h_data->red_avg = (h_data->red_avg * 0.95) + (h_data->red * 0.05);
//     h_data->ir_avg = (h_data->ir_avg * 0.95) + (h_data->ir * 0.05);
//     return update_high_pass(ir_hpf, h_data->ir);
// }

// int get_averaged_bpm(BeatDetector *bd, int new_bpm) {
//     bd->bpm_buffer[bd->idx] = new_bpm;
//     bd->idx = (bd->idx + 1) % 5;
//     int sum = 0;
//     for(int i=0; i<5; i++) sum += bd->bpm_buffer[i];
//     return sum / 5;
// }

// void calculate_health_metrics(HealthData *data) {
//     float red_ac = (float)data->red - data->red_avg;
//     float ir_ac = (float)data->ir - data->ir_avg;
//     if (data->ir_avg > 0 && ir_ac != 0) {
//         float R = (fabs(red_ac) / data->red_avg) / (fabs(ir_ac) / data->ir_avg);
//         float spo2_val = 110.0 - (25.0 * R);
//         if (spo2_val > 100.0) spo2_val = 100.0;
//         if (spo2_val < 85.0) spo2_val = 85.0;
//         data->spo2 = (data->spo2 * 0.8) + (spo2_val * 0.2);
//     }
// }

// // --- 메인 모니터링 함수 ---

// void run_test2_bpm_stable_monitoring(void) {
//     HealthData h_data = {0, 0, 0, 0, 0, 98.0};
//     HighPassFilter ir_hpf = {0, 0.94}; 
//     BeatDetector detector = {{0}, 0, 0, false};
//     float last_filtered_ir = 0;
//     bool is_finger_on = false; // 손가락 접촉 상태 플래그

//     MAX30102_Init();
//     printf("\n[ SYSTEM ACTIVE ]\n");

//     while (1) {
//         MAX30102_Read_FIFO(&h_data.red, &h_data.ir);
//         float filtered_ir = process_health_data(&h_data, &ir_hpf);

//         // 1. 손가락 감지 (30000 기준)
//         if (h_data.ir > 30000) { 
//             if (!is_finger_on) {
//                 // 손가락을 새로 댄 경우: 타이밍 초기화
//                 detector.last_beat_tick = current_tick;
//                 is_finger_on = true;
//             }

//             // 2. 박동 검출 로직
//             if (filtered_ir > last_filtered_ir) {
//                 detector.rising = true;
//             } 
//             else if (filtered_ir < last_filtered_ir && detector.rising) {
//                 if (filtered_ir > 15) { // 문턱값
//                     detector.rising = false;
//                     uint32_t duration = current_tick - detector.last_beat_tick;
                    
//                     // 3. 로그 기반 황금 간격 (45~120 Tick)
//                     if (duration >= 45 && duration <= 120) {
//                         int raw_bpm = 6000 / duration; 
                        
//                         // 이동 평균으로 BPM 안정화
//                         h_data.bpm = (h_data.bpm == 0) ? raw_bpm : (h_data.bpm * 0.7) + (raw_bpm * 0.3);
//                         calculate_health_metrics(&h_data);

//                         // 결과 출력
//                         printf("BPM: %d | SpO2: %d%%\n", h_data.bpm, (int)h_data.spo2);
                        
//                         // LED 피드백
//                         PORTB |= (1 << LED_PIN); _delay_ms(15); PORTB &= ~(1 << LED_PIN);
//                         detector.last_beat_tick = current_tick;
//                     }
//                 }
//             }
//         } 
//         else {
//             // 4. 손가락을 뗀 경우: 센서 및 필터 초기화
//             if (is_finger_on) {
//                 printf("Sensor Reset...\n");
//                 MAX30102_Init(); // 센서 재시작 (버퍼 비우기)
//                 ir_hpf.w = 0;     // 필터 잔상 제거
//                 detector.rising = false;
//                 h_data.bpm = 0;
//                 is_finger_on = false;
//             }
//             if (current_tick % 500 == 0) printf("Place Finger...\n");
//         }

//         last_filtered_ir = filtered_ir;
//         current_tick++;
//         _delay_ms(10); 
//     }
// }

// int main(void) {
//     DDRB |= (1 << LED_PIN); 
//     uart_init();
//     stdout = uart_get_stdout();
//     _delay_ms(500);

//     run_test2_bpm_stable_monitoring(); 

//     while (1); 
//     return 0;
// }

//============이동 평균 필터===============


// --- 내부 계산 함수 (모듈화) ---
//void calculate_health_metrics(HealthData *data, uint32_t last_beat_tick);


// /**
//  * [TEST 2] 통합 모니터링 모드 (BPM + SpO2)
//  * 한 번 실행하면 무한 루프로 정밀 측정을 수행합니다.
//  */
// void run_test2_bpm_stable_monitoring(void) {
//     HealthData current = {0, 0, 0, 0, 0, 98.0};
//     uint32_t last_beat_tick = 0;
//     bool rising = false;
//     float last_ir_avg = 0;

//     int bpm_buf[5] = {0};
//     int b_idx = 0;

//     printf("\n--- [TEST 2] STABLE MONITORING ---\n");
//     printf("TIME(ms), IR_AVG, BPM, SpO2(%%)\n");
    
//     MAX30102_Init();

//     while (1) {
//         MAX30102_Read_FIFO(&current.red, &current.ir);
        
//         // 필터 계수 조정 (조금 더 부드럽게)
//         current.red_avg = (current.red_avg * 0.9) + (current.red * 0.1);
//         current.ir_avg = (current.ir_avg * 0.9) + (current.ir * 0.1);

//         // 손가락 감지 임계값을 20000으로 하향 조정
//         if (current.ir > 20000) {
//             if (current.ir_avg > last_ir_avg) {
//                 rising = true;
//             } 
//             else if (current.ir_avg < last_ir_avg && rising) {
//                 rising = false; 
//                 uint32_t duration = current_tick - last_beat_tick;
                
//                 if (duration >= 40 && duration <= 150) {
//                     int raw_bpm = 6000 / duration;
//                     bpm_buf[b_idx] = raw_bpm;
//                     b_idx = (b_idx + 1) % 5;
                    
//                     int sum = 0;
//                     for(int i=0; i<5; i++) sum += bpm_buf[i];
//                     current.bpm = sum / 5;

//                     calculate_health_metrics(&current, last_beat_tick);

//                     // [%] 출력을 위해 float 대신 (정수.소수점) 방식으로 안전하게 출력
//                     int spo2_int = (int)current.spo2;
//                     int spo2_dec = (int)((current.spo2 - spo2_int) * 10);

//                     printf("%lu, %lu, %d, %d.%d%%\n", 
//                             current_tick * 10, (uint32_t)current.ir_avg, current.bpm, spo2_int, spo2_dec);
                    
//                     PORTB |= (1 << LED_PIN); _delay_ms(10); PORTB &= ~(1 << LED_PIN);
//                     last_beat_tick = current_tick;
//                 }
//             }
//         } else {
//             // 메시지가 너무 자주 뜨지 않게 조절
//             if (current_tick % 200 == 0) printf(">> Place Finger...\n");
//             last_beat_tick = current_tick;
//         }

//         last_ir_avg = current.ir_avg;
//         current_tick++;
//         _delay_ms(10); 
//     }
// }

// /**
//  * 산소포화도(SpO2) 상세 계산 모듈
//  */
// void calculate_health_metrics(HealthData *data, uint32_t last_beat_tick) {
//     // AC 성분(변화량)과 DC 성분(평균값)의 비율 계산
//     float red_ac = (float)data->red - data->red_avg;
//     float ir_ac = (float)data->ir - data->ir_avg;
    
//     // R = (Red_AC / Red_DC) / (IR_AC / IR_DC)
//     float R = (fabs(red_ac) / data->red_avg) / (fabs(ir_ac) / data->ir_avg);
    
//     // SpO2 근사식 적용
//     float spo2_val = 110.0 - (25.0 * R);
    
//     // 수치 안정화 (보간법)
//     if (spo2_val > 100.0) spo2_val = 100.0;
//     if (spo2_val < 85.0) spo2_val = 85.0;
    
//     data->spo2 = (data->spo2 * 0.7) + (spo2_val * 0.3);
// }












//=========================================================


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
