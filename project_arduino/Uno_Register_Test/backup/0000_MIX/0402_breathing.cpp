#ifndef F_CPU
#define F_CPU 16000000UL // 시스템 클럭 정의 (16MHz)
#endif

#include <avr/io.h>
#include <util/delay.h>

// 1. Timer1 PWM 초기화 함수
void Timer1_PWM_Breathing_Init(void) {
    // 1-1. 출력 핀 설정: PB1 (OC1A)
    DDRB |= (1 << PB1); //PB1 디지털 9번 핀 //출력모드

    // 1-2. TCCR1A 레지스터 설정
    // - COM1A1 = 1: 비교 일치 시 OC1A 핀을 LOW로, BOTTOM 도달 시 HIGH로 (비반전 모드)
    // - WGM11 = 1, WGM10 = 1: 위상 교정 PWM 모드 (10비트 해상도, TOP = 0x03FF)
    TCCR1A = (1 << COM1A1) | (1 << WGM11) | (1 << WGM10);

    // 1-3. TCCR1B 레지스터 설정
    // - WGM13 = 0, WGM12 = 0: 위상 교정 PWM 모드 (10비트) 유지
    // - CS11 = 1, CS10 = 1: 프리스케일러 64 설정 (122Hz flicker-free 주파수)
    TCCR1B = (1 << CS11) | (1 << CS10);

    // 1-4. 초기 듀티비 설정: 0% (OCR1A = 0)
    OCR1A = 0;
}

// 2. 메인 함수
int main(void) {
    Timer1_PWM_Breathing_Init(); // PWM 및 핀 초기화

    uint16_t brightness = 0;     // 현재 밝기 값 (OCR1A 대입용, 0~1023)
    int8_t fadeAmount = 1;       // 밝기 변화량 및 방향 (양수: 밝아짐, 음수: 어두워짐)

    while (1) {
        // 2-1. OCR1A 레지스터 업데이트 (듀티비 제어)
        OCR1A = brightness;

        // 2-2. 다음 루프를 위한 밝기 값 계산
        brightness += fadeAmount;

        // 2-3. 밝기 경계값 처리 및 방향 반전
        // 10비트 해상도이므로 TOP 값은 1023 (0x03FF)
        if (brightness <= 0 || brightness >= 1023) {
            fadeAmount = -fadeAmount; // 방향 전환
            
            // 극단적인 값에서의 오버플로/언더플로 방지 및 부드러운 전환을 위한 보정
            if(brightness > 1023) brightness = 1023;
            if(brightness < 0)    brightness = 0;
        }

        // 2-4. 숨쉬기 속도 조절을 위한 미세 지연 (블로킹 방식)
        // 이 지연 값에 따라 숨쉬기 주기가 결정됨 (너무 크면 끊겨 보임)
        _delay_ms(3);
    }
    return 0; // 이 줄에 도달하지 않음
}