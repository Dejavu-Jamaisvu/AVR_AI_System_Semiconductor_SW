#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// 1. LED 상태 관리를 위한 구조체 정의
typedef struct {
    uint8_t pin;               // 핀 번호
    uint16_t onTime;           // 켜져 있을 시간 (ms)
    uint16_t offTime;          // 꺼져 있을 시간 (ms)
    volatile uint16_t counter; // 1ms마다 증가할 카운터 (ISR 내부 변경 변수이므로 volatile 필수)
    volatile uint8_t state;    // 현재 상태 (HIGH/LOW)
} TimerFlasher;

// 2. 3개의 LED를 배열로 묶어서 관리 (확장성 극대화)
#define NUM_LEDS 3
TimerFlasher leds[NUM_LEDS] = {
    {13, 100, 400, 0, LOW},  // LED 1: 13번 핀, 100ms ON / 400ms OFF
    {12, 500, 500, 0, LOW},  // LED 2: 12번 핀, 500ms ON / 500ms OFF
    {11, 50,  950, 0, LOW}   // LED 3: 11번 핀, 50ms ON / 950ms OFF
};

// 3. Timer1 하드웨어 초기화 (정확히 1ms마다 인터럽트 발생)
void Timer1_Init() {
    cli(); // 전역 인터럽트 중지 (설정 중 방해 방지)

    TCCR1A = 0; // 레지스터 초기화
    TCCR1B = 0; 
    TCNT1  = 0; // 타이머 카운터 초기화

    // 목표 카운트 값 설정 (1ms 기준)
    // 공식: (16MHz / (프리스케일러 * 목표 주파수)) - 1
    // (16,000,000 / (64 * 1000)) - 1 = 249
    OCR1A = 249; 

    // CTC 모드 설정 (WGM12 비트 1로 설정)
    TCCR1B |= (1 << WGM12); 

    // 프리스케일러 64 설정 (CS11, CS10 비트 1로 설정)
    TCCR1B |= (1 << CS11) | (1 << CS10); 

    // Timer1 비교 일치 A 인터럽트 활성화    
    TIMSK1 |= (1 << OCIE1A); 

    sei(); // 전역 인터럽트 재개
}

// 4. Timer1 인터럽트 서비스 루틴 (정확히 1ms마다 자동 실행됨)
ISR(TIMER1_COMPA_vect) {
    // 등록된 모든 LED의 카운터를 확인하고 제어
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i].counter++; // 1ms 지났으므로 카운터 1 증가

        // LED가 켜져 있고, 끄는 시간이 되었을 때
        if (leds[i].state == HIGH && leds[i].counter >= leds[i].onTime) {
            leds[i].state = LOW;
            leds[i].counter = 0; // 카운터 초기화
            digitalWrite(leds[i].pin, leds[i].state);
        }
        // LED가 꺼져 있고, 켜는 시간이 되었을 때
        else if (leds[i].state == LOW && leds[i].counter >= leds[i].offTime) {
            leds[i].state = HIGH;
            leds[i].counter = 0; // 카운터 초기화
            digitalWrite(leds[i].pin, leds[i].state);
        }
    }
}

void setup() {
    // 모든 핀을 출력으로 설정
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        pinMode(leds[i].pin, OUTPUT);
    }
    
    // 타이머 인터럽트 시작
    Timer1_Init(); 
}

void loop() {
    // 메인 루프는 완전히 비어 있습니다!
    // LED 제어는 백그라운드(ISR)에서 하드웨어적으로 알아서 처리됩니다.
    // 여기에 시리얼 통신이나 복잡한 센서 계산 코드를 자유롭게 넣어도 LED는 절대 밀리지 않습니다.
}