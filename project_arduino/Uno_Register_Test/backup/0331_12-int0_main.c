#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PIN PB5 // 아두이노 우노 내장 LED (D13)
#define BTN_PIN PD2 // 외부 인터럽트 0 (D2)

volatile int state = 0;			// 현재 LED의 상태
void int0_init(void) {
    // 1. PD2 핀을 입력으로 설정 및 내부 풀업 저항 활성화
    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    // 2. 인터럽트 감지 조건 설정 (EICRA 레지스터)
    // ISC01 = 1, ISC00 = 0 : 하강 에지(Falling Edge) 감지
    // 스위치를 눌러 전압이 GND로 떨어지는 순간을 포착함
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    // 3. INT0 외부 인터럽트 허용 (EIMSK 레지스터)
    EIMSK |= (1 << INT0);

    // 4. 전역 인터럽트 활성화
    sei(); 
}

// INT0 외부 인터럽트 서비스 루틴(ISR)
ISR(INT0_vect) {
    // LED 상태 반전 (Toggle)
    PORTB ^= (1 << LED_PIN); 
}

int main(void) {
    // LED 핀을 출력으로 설정
    DDRB |= (1 << LED_PIN);

    // 인터럽트 초기화
    int0_init();

    while(1) {
        // 메인 루프는 비워둠. 
        // CPU는 다른 작업을 하다가 버튼이 눌리면 즉각 ISR을 실행함.
    }
    return 0;
}