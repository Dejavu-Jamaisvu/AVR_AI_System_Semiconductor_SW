#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

// 1. Timer0 (8비트) Fast PWM 초기화 함수
void Timer0_Fast_PWM_Init(void) {
    // 1-1. 출력 핀 설정: PD6 (OC0A, 아두이노 디지털 6번 핀) ★ 변경
    DDRD |= (1 << DDD6); 

    // 1-2. TCCR0A 설정 (8비트 타이머용 레지스터 이름임)
    // - COM0A1 = 1: 비반전 모드
    // - WGM01 = 1, WGM00 = 1: Fast PWM 모드 (TOP = 255) ★ 8비트 설정
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);

    // 1-3. TCCR0B 설정
    // - CS01, CS00 = 1: 프리스케일러 64 설정
    // 주파수 계산: 16MHz / (64 * 256) = 약 976Hz (매우 빠름)
    TCCR0B = (1 << CS01) | (1 << CS00);

    // 1-4. 초기 듀티비 (0~255)
    OCR0A = 0;
}

int main(void) {
    Timer0_Fast_PWM_Init();

    // 8비트이므로 최대값은 255입니다.
    uint16_t brightness = 0; 
    int8_t fadeAmount = 2; // 255단계이므로 변화량을 조금 줄여야 부드럽습니다.

    while (1) {
        // 2-1. 8비트 레지스터 OCR0A에 값 쓰기
        OCR0A = (uint8_t)brightness;

        // 2-2. 밝기 계산
        brightness += fadeAmount;

        // 2-3. 8비트 경계값(255) 처리
        if (brightness >= 255) {
            brightness = 255;
            fadeAmount = -fadeAmount;
        } 
        else if (brightness <= 0 || brightness > 255) {
            brightness = 0;
            fadeAmount = -fadeAmount;
        }

        // 2-4. 8비트 고속 PWM은 주기가 매우 빠르므로 딜레이를 조절하세요.
        _delay_ms(15); 
    }
    return 0;
}